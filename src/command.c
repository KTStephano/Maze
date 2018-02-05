#include "command.h"

#define MAX_CMDS 512

typedef struct
{
  array_t        buffer[MAX_CMDS];
  array_t        args[MAX_CMDS];
  int            used;
} cmdBuffer_t;

static command_t *cmds;
static cmdBuffer_t cmdBuffer;

static int argc = 0;
static array_t argv;
static char* currArg = NULL; // used for really basic garbage collection

//
// Functions specific to this file
//

// This counts the first leading to a space/EOL as the command,
// rest as input
char *cmd_TokenizeString(const char *str, array_t *args)
{
  int size = 0;
  int lastElement = 0;
  const char *tempStr = str;
  char *cmdStr = NULL; // stores first part of str
  boolean_t bFoundFirstSpace = _FALSE;

  if (!str[0]) return cmdStr; // empty string, return now

  // strip leading white space
  while (*tempStr == ' ') tempStr += 1;

  while (*tempStr)
  {
    if (*tempStr == ' ')
    {
      if (!bFoundFirstSpace)
      {
        cmdStr = (char*)malloc((sizeof(char) * size) + 1);
        memcpy(cmdStr, str, size);
        lastElement = size;
      }

      // ignore the rest
      while (*tempStr == ' ') tempStr += 1;
      if (*tempStr && bFoundFirstSpace) appendi(args, ' ');
      bFoundFirstSpace = _TRUE;
    }

    if (*tempStr && bFoundFirstSpace && *tempStr != ' ') appendi(args, *tempStr);

    if (*tempStr) tempStr += 1;
    size++;
  }

  // in case a space wasn't encountered
  if (!bFoundFirstSpace && !cmdStr)
  {
    cmdStr = (char*)malloc((sizeof(char) * size) + 1);
    memcpy(cmdStr, str, size);
    lastElement = size;
  }

  if (lastElement) cmdStr[lastElement] = 0;
  return cmdStr;
}

void cmd_GenerateNumArgs(int currCmdIndex)
{
  int i;

  if (cmdBuffer.args[currCmdIndex].size) argc++;

  for (i = 0; i < cmdBuffer.args[currCmdIndex].size; i++)
  {
    if (((int*)cmdBuffer.args[currCmdIndex].data)[i] == ' ') argc++;
  }

  argv = cmdBuffer.args[currCmdIndex];
}

//
// Functions available to the public
//
void cmd_Init(void)
{
  cmds = (command_t*)malloc(sizeof(command_t));
  cmds->next = NULL;
  cmds->cmd = NULL;

  cmdBuffer.used = 0;
}

void cmd_Shutdown(void)
{
  command_t *temp = cmds->next;
  if (cmds->cmd == NULL)
  {
    clearData(&cmds->id);
    free(cmds);
    return;
  }

  if (currArg != NULL) free(currArg);

  while (1)
  {
    clearData(&cmds->id);
    free(cmds);

    if (temp == NULL) break;

    cmds = temp; // there are still commands left to free
    temp = temp->next;
  }

  cmd_FlushBuffer();
}

void cmd_AddCommand(const char *id, cmd_t command)
{
  array_t cmdId;
  command_t *curr = cmds;

  if (cmds->cmd == NULL) // see if there are 0 commands
  {
    if (!cmd_ReadStrTo(&cmdId, id)) return;
    cmds->id = cmdId;
    cmds->cmd = command;
    return;
  }

  if (!cmd_ReadStrTo(&cmdId, id))
  {
    clearData(&cmdId);
    return;
  }

  while (1)
  {
    if (cmd_StrCmp(&curr->id, id)) return; // command already exists
    if (curr->next == NULL) break;

    curr = curr->next;
  }

  curr = cmds;

  while (1)
  {
    if (curr->next == NULL)
    {
      command_t *cmd = (command_t*)malloc(sizeof(command_t));
      curr->next = cmd;
      cmd->next = NULL;
      cmd->id = cmdId;
      cmd->cmd = command;

      break;
    }

    curr = curr->next;
  }
}

void cmd_FlushBuffer(void)
{
  int i;

  for (i = 0; i < cmdBuffer.used; i++)
  {
    clearData(&cmdBuffer.buffer[i]);
    clearData(&cmdBuffer.args[i]);
  }

  argc = 0; // make sure this gets reset
  cmdBuffer.used = 0;
}

void cmd_AddToBuffer(const char *id)
{
  array_t cmdId;
  array_t args;
  initArray(&args);
  char *str = cmd_TokenizeString(id, &args);
  cmd_PrintArray(&args);

  if (str == NULL) return;

  if (cmdBuffer.used >= MAX_CMDS)
  {
    if (str) free(str);
    return; // it can't log anymore commands
  }
  else if (!cmd_ReadStrTo(&cmdId, str))
  {
    if (str) free(str);
    return;
  }

  free(str);
  cmdBuffer.buffer[cmdBuffer.used] = cmdId;
  cmdBuffer.args[cmdBuffer.used] = args;
  cmdBuffer.used++;
}

void cmd_ExecuteCommands(void)
{
  int i;
  command_t *curr = cmds;
  char *str;

  if (cmds->cmd == NULL) return; // no commands logged

  for (i = 0; i < cmdBuffer.used; i++)
  {
    curr = cmds; // make sure to reset this each loop
    while (1)
    {
      // Try to find the command in the list
      if (cmd_ArrCmpi(&curr->id, &cmdBuffer.buffer[i]))
      {
        cmd_GenerateNumArgs(i);
        curr->cmd();
        argc = 0; // make sure this is reset
        break;
      }
      // Check to see if we're at the end of the list
      if (curr->next == NULL) break;

      curr = curr->next;
    }
  }

  cmd_FlushBuffer(); // clear the buffer since we're done
}

// Does NOT support a string like "<id> <param1> ..." like
// cmd_ExecuteCommands does
void cmd_ExecuteSingle(const char *id)
{
  array_t command;
  command_t *curr = cmds;
  initArray(&command);
  cmd_ReadStrTo(&command, id);

  if (cmds->cmd == NULL) return; // no commands logged

  while (1)
  {
    // Try to find the command in the list
    if (cmd_ArrCmpi(&curr->id, &command))
    {
      curr->cmd();
      break;
    }
    // Check to see if we're at the end of the list
    if (curr->next == NULL) break;

    curr = curr->next;
  }

  clearData(&command);

}

void cmd_PrintCommands(void)
{
  printf("Inside of cmd_PrintCommands\n");
  int i = 0;
  command_t *curr = cmds;

  if (cmds->cmd == NULL) return; // no commands logged

  printf("\n");
  while (1)
  {
    i++;
    cmd_PrintArray(&curr->id); // print the contents of curr->id

    if (i % 6 == 0) printf("\n"); // only print 6 commands per line
    else printf("\t");

    if (curr->next == NULL) break; // we are at the end

    curr = curr->next;
  }
  printf("\n");
}

// this will assume it is an array of integers that can be used as characters
void cmd_PrintArray(array_t *arr)
{
  int i;

  if (arr->size == 0) return; // it's empty

  for (i = 0; i < arr->size; i++)
  {
    if (checkCharValidity(((int*)arr->data)[i])) printf("%c", ((int*)arr->data)[i]);
    else printf("0");
  }
}

boolean_t cmd_ReadStrTo(array_t *out, const char *str)
{
  int i;
  initArray(out);

  if (!str[0]) return _FALSE; // don't do anything if the string is empty

  for (i = 0; i < strlen(str); i++)
  {
    appendi(out, str[i]);
  }

  return _TRUE;
}

boolean_t cmd_StrCmp(array_t *id, const char *str)
{
  int i;

  if (!str[0]) return _FALSE;
  else if (!id->size) return _FALSE;
  else if (id->size > strlen(str) || id->size < strlen(str)) return _FALSE;

  for (i = 0; i < id->size; i++)
  {
    if (((int*)id->data)[i] != str[i]) return _FALSE;
  }

  return 1; // they were the same
}

boolean_t cmd_ArrCmpi(array_t *arr1, array_t *arr2)
{
  int i;

  if (arr1->size < arr2->size || arr1->size > arr2->size) return _FALSE;

  for (i = 0; i < arr1->size; i++)
  {
    if (((int*)arr1->data)[i] != ((int*)arr2->data)[i]) return _FALSE;
  }

  return _TRUE; // they were the same
}

int cmd_GetNumArgs(void)
{
  return argc;
}

// Do not free the data this returns
char *cmd_GetArg(int index)
{
  array_t temp;
  int currIndex = 0;
  int i;
  char *arg = NULL;

  if (currArg != NULL) free(currArg);

  initArray(&temp);

  // check for invalid data
  if (index >= argc) return arg;
  else if (argc == 0) return arg;

  // When a space is encountered (separator), increment
  // currIndex. When currIndex equals the desired index, begin reading
  // in data from argv until the next space (then stop).
  for (i = 0; i < argv.size; i++)
  {
    if (currIndex == index && ((int*)argv.data)[i] != ' ')
    {
      appendi(&temp, ((int*)argv.data)[i]);
    }
    else if (currIndex == index && ((int*)argv.data)[i] == ' ') break;
    else if (((int*)argv.data)[i] == ' ') currIndex++;
  }

  arg = (char*)malloc((sizeof(char) * temp.size) + 1);
  for (i = 0; i < temp.size; i++) arg[i] = ((int*)temp.data)[i];

  arg[temp.size] = 0;
  currArg = arg;

  clearData(&temp);
  return arg;
}