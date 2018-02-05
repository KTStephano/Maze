#include "filesystem.h"
#include "utility.h"
#include "command.h"
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef linux
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#define MAX_HANDLES 64

typedef struct
{
  char *path;
  char *fileName; // so you can do quick comparisons of filenames
  fileHandle_t handle;
  FILE *file;
} file_t;

typedef struct
{
  char *cwd; // current working directory
  boolean_t bDirectoryNeedsProfiling;
  file_t activeFiles[MAX_HANDLES]; // open files with handles
  array_t directory;
} fileSystem_t;

static fileSystem_t fs;

// function prototypes
char *fs_StripExtension(const char *path);
void fs_cmdSetCWD(void);
void fs_SeekFile(void);
void fs_ListKnownFiles(void);

void appendFile(array_t *arr, file_t file)
{
  int newSize = arr->size + 1;
  file_t *temp;
  int i;

  if (arr->size == 0)
  {
    arr->data = malloc(sizeof(file_t));
    ((file_t*)arr->data)[0] = file;
    arr->size = 1;

    return;
  }

  temp = (file_t*)malloc(sizeof(file_t) * newSize);

  for (i = 0; i < arr->size; i++)
  {
    temp[i] = ((file_t*)arr->data)[i];
  }

  temp[newSize - 1] = file;

  clearData(arr);
  arr->data = (void*)temp;
  arr->size = newSize;
}

// clears out all active data
void fs_FlushFileSystem(void)
{
  int i;

  if (fs.cwd)
  {
    free(fs.cwd);
    for (i = 0; i < fs.directory.size; i++)
    {
      free(((file_t*)fs.directory.data)[i].path);
      free(((file_t*)fs.directory.data)[i].fileName);
    }
    clearData(&fs.directory);

    for (i = 0; i < MAX_HANDLES; i++)
    {
      fs_Close(fs.activeFiles[i].handle);
    }
  }
}

void fs_Init(void)
{
  int i;

  for (i = 0; i < MAX_HANDLES; i++)
  {
    fs.activeFiles[i].handle = i;
    fs.activeFiles[i].file = NULL;
    fs.activeFiles[i].path = NULL;
    fs.activeFiles[i].fileName = NULL;
  }

  fs.cwd = NULL;
  fs.bDirectoryNeedsProfiling = _TRUE;

  initArray(&fs.directory);

  // init commands here
  cmd_AddCommand("SetCWD", fs_cmdSetCWD);
  cmd_AddCommand("FindFile", fs_SeekFile);
  cmd_AddCommand("ListFiles", fs_ListKnownFiles);
}

void fs_Shutdown(void)
{
  fs_FlushFileSystem();
}

// sets the current working directory
void fs_SetCWD(const char *path)
{
  int len;
  char *newPath = fs_StripExtension(path);
  len = strlen(newPath) + 1;

  if (fs.cwd)
  {
    fs_FlushFileSystem();
  }

  fs.bDirectoryNeedsProfiling = _TRUE;
  fs.cwd = (char*)malloc(sizeof(char)*len);
  memcpy(fs.cwd, newPath, len);
  free(newPath);

  printf("CWD = %s\n", fs.cwd);
}

// strips the extension and the name associated with it
// ex: C:\\Users\\Documents\\file.exe becomes
// C:\\Users\\Documents\\ ** (minus **)
char *fs_StripExtension(const char *path)
{
  int i;
  int index = 0;
  const char *temp = path;
  char *newPath;
  char extra = 1;

  while (*temp)
  {
    if (*temp == '.')
    {
      while (index > 0 && *temp != '\\')
      {
        temp--;
        index--;
      }

      index++; // lets it capture the last '\'
      break;
    }

    temp++;
    index++;
  }
#ifdef linux
  temp--; // backtrack 1
  if (*temp != '/') extra = 2;
#endif

  newPath = (char*)malloc(sizeof(char)*index + extra);
  newPath[index] = 0;

#ifdef linux
  if (*temp != '/')
  {
    newPath[index] = '/';
    newPath[index + 1] = 0;
  }
#endif

  for (i = 0; i < index; i++)
  {
    newPath[i] = path[i];
  }

  return newPath;
}

boolean_t IsDirectory(const char *path)
{
#ifdef _WIN32
  LPCTSTR sPath = path;
  DWORD attrib = GetFileAttributesA(sPath);

  // the mask results in 0 if not a directory
  return (attrib & FILE_ATTRIBUTE_DIRECTORY);
#endif
#ifdef linux
  struct stat attrib;
  if (!stat(path, &attrib)) return _FALSE; // invalid
  return S_ISDIR(attrib.st_mode);
#endif
  return _FALSE;
}

void fs_Recursive_CheckDirectories(const char *path)
{
#ifdef _WIN32
  HANDLE search_handle;
  WIN32_FIND_DATA file;
#endif
#ifdef linux
  DIR *directory;
  struct dirent *direntry;
#endif
  int len = strlen(path);

  char *fpath = (char*)malloc(sizeof(char)*len + 3);
  memcpy(fpath, path, len);
#ifdef _WIN32
  fpath[len] = '\\';
  fpath[len + 1] = '*';
  fpath[len + 2] = 0;
#endif
#ifdef linux
  fpath[len] = '/';
  fpath[len + 1] = 0;
#endif

  //printf("Recursive fpath = %s\n", fpath);

#ifdef _WIN32
  search_handle = FindFirstFileA(fpath, &file);
  //printf("first file recursive = %s\n", file.cFileName);

  if (search_handle == INVALID_HANDLE_VALUE) return;

  do
  {
    if (!(file.cFileName[0] == '.') && !(file.cFileName[0] == '..'))
    {
      len = (strlen(fpath) - 1) + strlen(file.cFileName);
      char *temp = (char*)malloc(sizeof(char)*len + 1);

      memcpy(temp, fpath, strlen(fpath) - 1);
      memcpy(temp + (strlen(fpath) - 1), file.cFileName, strlen(file.cFileName));
      temp[len] = 0;

      //printf("recursive temp = %s\n", temp);

      if (IsDirectory(temp))
      {
        //printf("Recursive temp (%s) was a directory.\n", temp);
        fs_Recursive_CheckDirectories(temp);
        free(temp);
      }
      else
      {
        file_t f;
        //len = strlen(file.cFileName);
        len = strlen(temp);

        f.path = (char*)malloc(sizeof(char)*len + 1);
        f.fileName = (char*)malloc(sizeof(char)*strlen(file.cFileName) + 1);
        memcpy(f.path, temp, len + 1);
        memcpy(f.fileName, file.cFileName, strlen(file.cFileName) + 1);

        appendFile(&fs.directory, f);
      }
    }
  } while (FindNextFileA(search_handle, &file));
  FindClose(search_handle);
#endif
#ifdef linux
  directory = opendir(path);
  if (!directory) return;
  direntry = readdir(directory);

  do
  {
    if (!(direntry->d_name[0] == '.'))// && !(direntry->d_name[0] == '..'))
    {
      len = (strlen(fpath) - 1) + strlen(direntry->d_name);
      char *temp = (char*)malloc(sizeof(char)*len + 1);

      memcpy(temp, fpath, strlen(fpath) - 1);
      memcpy(temp + (strlen(fpath) - 1), direntry->d_name, strlen(direntry->d_name));
      temp[len] = 0;

      //printf("recursive temp = %s\n", temp);

      if (IsDirectory(temp))
      {
        //printf("Recursive temp (%s) was a directory.\n", temp);
        fs_Recursive_CheckDirectories(temp);
      }
      else
      {
        file_t f;
        //len = strlen(file.cFileName);
        len = strlen(temp);

        f.path = (char*)malloc(sizeof(char)*len + 1);
        f.fileName = (char*)malloc(sizeof(char)*strlen(direntry->d_name) + 1);
        memcpy(f.path, temp, len + 1);
        memcpy(f.fileName, direntry->d_name, strlen(direntry->d_name) + 1);

        appendFile(&fs.directory, f);
      }
      free(temp);
    }
  } while ((direntry = readdir(directory)));
  closedir(directory);
#endif
}

void fs_ProfileCWD(void)
{
  if (!fs.cwd || !fs.bDirectoryNeedsProfiling) return;
  printf("Profiling...\n");
#ifdef _WIN32
  HANDLE search_handle;
  WIN32_FIND_DATA file;
#endif
#ifdef linux
  DIR *directory;
  struct dirent *direntry;
#endif
  int len = strlen(fs.cwd);

  char *path = (char*)malloc(sizeof(char)*len + 2);
  memcpy(path, fs.cwd, len);
#ifdef _WIN32
  path[len] = '*';
  path[len + 1] = 0;
#endif
#ifdef linux
  path[len] = 0;
#endif

  fs.bDirectoryNeedsProfiling = _FALSE;

#ifdef _WIN32
  search_handle = FindFirstFileA(path, &file);
  free(path);

  if (search_handle == INVALID_HANDLE_VALUE) return;

  //printf("file = %s\n", file.cFileName);

  do
  {
    //if (!(file.cFileName[0] == '.'))
    //{
    len = strlen(fs.cwd) + strlen(file.cFileName);
    char *temp = (char*)malloc(sizeof(char)*len + 1);

    memcpy(temp, fs.cwd, strlen(fs.cwd));
    memcpy(temp + strlen(fs.cwd), file.cFileName, strlen(file.cFileName) + 1);

    //printf("temp = %s\n", temp);
    if (IsDirectory(temp))
    {
      fs_Recursive_CheckDirectories(temp);
      printf("%s\n", temp);
      free(temp);
    }
    else
    {
      file_t f;
      //len = strlen(file.cFileName);
      len = strlen(temp);

      f.path = (char*)malloc(sizeof(char)*len + 1);
      f.fileName = (char*)malloc(sizeof(char)*strlen(file.cFileName) + 1);
      memcpy(f.path, temp, len + 1);
      memcpy(f.fileName, file.cFileName, strlen(file.cFileName) + 1);

      appendFile(&fs.directory, f);
      free(temp);
    }
    //}
  } while (FindNextFileA(search_handle, &file));
  FindClose(search_handle);
#endif
#ifdef linux
  directory = opendir(path);
  free(path);
  if (!directory) return;
  direntry = readdir(directory);

  do
  {
    printf("File = %s\n", direntry->d_name);
    len = strlen(fs.cwd) + strlen(direntry->d_name);
    char *temp = (char*)malloc(sizeof(char)*len + 1);

    memcpy(temp, fs.cwd, strlen(fs.cwd));
    memcpy(temp + strlen(fs.cwd), direntry->d_name, strlen(direntry->d_name) + 1);

    //printf("temp = %s\n", temp);
    if (IsDirectory(temp))
    {
      fs_Recursive_CheckDirectories(temp);
      free(temp);
    }
    else
    {
      file_t f;
      len = strlen(temp);

      f.path = (char*)malloc(sizeof(char)*len + 1);
      f.fileName = (char*)malloc(sizeof(char)*strlen(direntry->d_name) + 1);
      memcpy(f.path, temp, len + 1);
      memcpy(f.fileName, direntry->d_name, strlen(direntry->d_name) + 1);

      appendFile(&fs.directory, f);
      free(temp);
    }
  } while ((direntry = readdir(directory)));
  closedir(directory);
#endif
}

// Always check that the return is not NULL!
char *fs_FindFile(const char *file)
{
  char *path = NULL;

  if (!fs.cwd) return path;
  else if (fs.bDirectoryNeedsProfiling) fs_ProfileCWD();

  int i;

  for (i = 0; i < fs.directory.size; i++)
  {
    if (compareStrings(((file_t*)fs.directory.data)[i].fileName, file))
    {
      int len = strlen(((file_t*)fs.directory.data)[i].path);
      path = (char*)malloc(sizeof(char)*len + 1);
      memcpy(path, ((file_t*)fs.directory.data)[i].path, len + 1);
      break;
    }
  }

  return path;
}

// always check if the return is INVALID!
fileHandle_t fs_Open(const char *file, const char *tag)
{
  int index;
  char *path;

  for (index = 0; index < MAX_HANDLES; index++)
  {
    if (!fs.activeFiles[index].file) break;
  }

  if (index == MAX_HANDLES) return INVALID;

  path = fs_FindFile(file);

  switch (*tag)
  {
  case 'r':
    if (!path) return INVALID; // file not found
    if (*(tag + 1) && *(tag + 1) == 'b')
    {
      fs.activeFiles[index].file = fopen(path, "rb");
    }
    else fs.activeFiles[index].file = fopen(path, "r");
    break;
  case 'w':
    if (!path)
    {
      path = (char*)malloc(sizeof(char)*strlen(file) + 1);
      path = strcpy(path, file);
      printf("%s\n", path);
    }
    fs.activeFiles[index].file = fopen(path, "w");
    break;
  case 'a':
    if (!path)
    {
      path = (char*)malloc(sizeof(char)*strlen(file) + 1);
      path = strcpy(path, file);
    }
    fs.activeFiles[index].file = fopen(path, "a");
    break;
  default: return INVALID;
  }

  if (!fs.activeFiles[index].file) return INVALID;

  fs.activeFiles[index].fileName = (char*)malloc(sizeof(char)*strlen(file) + 1);
  fs.activeFiles[index].fileName = strcpy(fs.activeFiles[index].fileName, file);
  fs.activeFiles[index].path = path;

  return fs.activeFiles[index].handle;
}

// Tries to close the file associated with the handle
void fs_Close(fileHandle_t handle)
{
  if (handle < 0 || handle > MAX_HANDLES) return;
  else if (!fs.activeFiles[handle].file) return;

  fclose(fs.activeFiles[handle].file);
  fs.activeFiles[handle].file = NULL;

  free(fs.activeFiles[handle].path);
  free(fs.activeFiles[handle].fileName);

  fs.activeFiles[handle].path = NULL;
  fs.activeFiles[handle].fileName = NULL;
}

// Always check if the return is NULL!
char *fs_GetPath(fileHandle_t handle)
{
  if (handle < 0 || handle > MAX_HANDLES) return NULL;
  else if (!fs.activeFiles[handle].file) return NULL;

  return fs.activeFiles[handle].path;
}

int fs_GetC(fileHandle_t handle)
{
  if (handle < 0 || handle >= MAX_HANDLES) return EOF;
  else if (!fs.activeFiles[handle].file) return EOF;

  return fgetc(fs.activeFiles[handle].file);
}

// Always check if the return is NULL!
void fs_ReadFile(fileHandle_t handle, array_t *buffer)
{
  char c;
  long fileSize;
  initArray(buffer);

  if (handle < 0 || handle >= MAX_HANDLES)
  {
    buffer->data = NULL;
    return;
  }
  else if (!fs.activeFiles[handle].file)
  {
    buffer->data = NULL;
    return;
  }

  // try to reserve enough for the file before hand
  // (improves performance immensely for larger files, especially)
  FILE *f = fopen(fs.activeFiles[handle].path, "rb"); // ftell requires binary read
  if (f)
  {
    fseek(f, 0L, SEEK_END); // find the end
    fileSize = ftell(f);
    fseek(f, 0L, SEEK_SET); // set to beginning
    reservei(buffer, fileSize);
    fclose(f);
  }

  while ((c = fs_GetC(handle)) != EOF)
  {
    appendi(buffer, c);
  }
}

void fs_WriteFile(fileHandle_t handle, array_t *input)
{
  // make sure it is a valid file
  if (handle < 0 || handle >= MAX_HANDLES) return;
  else if (!fs.activeFiles[handle].file) return;

  int i;
//  char *str = arrayToChar(input);
  for (i = 0; i < input->size; i++)
  {
    //fprintf(fs.activeFiles[handle].file, "%c", str[i]);
    fprintf(fs.activeFiles[handle].file, "%c", geti(input, i));
  }
//  free(str);
}

//
// Begin command line functions and their helpers
//
void fs_cmdSetCWD(void)
{
  char *arg;
  char *path;
  int i;
  array_t tempBuff;
  initArray(&tempBuff);

#ifdef _WIN32
  HANDLE search_handle;
  WIN32_FIND_DATA file;
#endif

  if (cmd_GetNumArgs() != 1)
  {
    printf("Invalid! Use: SetCWD path\\to\\dir\n");
    return;
  }

  arg = cmd_GetArg(0);

  for (i = 0; i < strlen(arg); i++)
  {
    if (arg[i] == '\\')
    {
      appendi(&tempBuff, '\\'); // append an extra
    }

    if (arg[i] != ' ')
    {
      appendi(&tempBuff, arg[i]);
    }
  }

  path = (char*)malloc(sizeof(char)*tempBuff.size + 2);
  memcpy(path, arrayToChar(&tempBuff), tempBuff.size);
  path[tempBuff.size] = '*';
  path[tempBuff.size + 1] = 0;

#ifdef _WIN32
  search_handle = FindFirstFileA(path, &file);

  if (search_handle == INVALID_HANDLE_VALUE)
  {
    printf("Invalid directory path.\n");
  }
  else
  {
    printf("%s\n", file.cFileName);
    fs_SetCWD(arrayToChar(&tempBuff));
    FindClose(search_handle);
  }
#endif

  free(path);
  clearData(&tempBuff);
}

void fs_SeekFile(void)
{
  char *path;
  char *arg;

  if (cmd_GetNumArgs() != 1)
  {
    printf("Invalid. Use FindFile filename.ext.\n");
    return;
  }

  arg = cmd_GetArg(0);
  path = fs_FindFile(arg);

  if (!path)
  {
    printf("Could not find %s. Make sure to use the format: filename.ext\n", arg);
    return;
  }

  printf("Found file: %s\n", path);
  free(path);
}

void fs_ListKnownFiles(void)
{
  int i;

  if (fs.cwd && fs.bDirectoryNeedsProfiling) fs_ProfileCWD();
  else if (!fs.cwd)
  {
    printf("Current working directory not set. Type SetCWD path\\to\\dir to set.\n");
    return;
  }

  printf("---- File List ----\n");
  for (i = 0; i < fs.directory.size; i++)
  {
    printf("%s\n", ((file_t*)fs.directory.data)[i].fileName);
  }
}