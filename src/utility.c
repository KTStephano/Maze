#include "utility.h"
#include <string.h>
#ifdef linux
#include <termios.h>
#include <unistd.h>
#endif

void appendus(array_t *arr, unsigned short data)
{
  int newSize = arr->size + 1;
  unsigned short *temp;
  int i;

  if (arr->reserved)
  {
    ((unsigned short*)arr->data)[arr->size] = data;
    arr->size++;
    arr->reserved--;

    return;
  }

  if (arr->size == 0)
  {
    arr->data = malloc(sizeof(unsigned short));
    ((unsigned short*)arr->data)[0] = data;
    arr->size = 1;

    return;
  }

  temp = (unsigned short*)malloc(sizeof(unsigned short) * newSize);

  for (i = 0; i < arr->size; i++)
  {
    temp[i] = ((unsigned short*)arr->data)[i];
  }

  temp[newSize - 1] = data;

  clearData(arr);
  arr->data = (void*)temp;
  arr->size = newSize;
}

void appendi(array_t *arr, int data)
{
  int newSize = arr->size + 1;
  int *temp;
  int i;

  if (arr->reserved)
  {
    ((int*)arr->data)[arr->size] = data;
    arr->size++;
    arr->reserved--;

    return;
  }

  if (arr->size == 0)
  {
    arr->data = malloc(sizeof(int));
    ((int*)arr->data)[0] = data;
    arr->size = 1;

    return;
  }

  temp = (int*)malloc(sizeof(int) * newSize);

  for (i = 0; i < arr->size; i++)
  {
    temp[i] = ((int*)arr->data)[i];
  }

  temp[newSize - 1] = data;

  clearData(arr);
  arr->data = (void*)temp;
  arr->size = newSize;
}

void appendf(array_t *arr, float data)
{
  int newSize = arr->size + 1;
  float *temp;
  int i;

  if (arr->reserved)
  {
    ((float*)arr->data)[arr->size] = data;
    arr->size++;
    arr->reserved--;

    return;
  }

  if (arr->size == 0)
  {
    arr->data = malloc(sizeof(float));
    ((float*)arr->data)[0] = data;
    arr->size = 1;

    return;
  }

  temp = (float*)malloc(sizeof(float) * newSize);

  for (i = 0; i < arr->size; i++)
  {
    temp[i] = ((float*)arr->data)[i];
  }

  temp[newSize - 1] = data;

  clearData(arr);
  arr->data = (void*)temp;
  arr->size = newSize;
}

void reserveus(array_t *arr, int amount)
{
  unsigned short *temp;
  int newSize = arr->size + amount;
  int i;

  if (amount <= 0) return;

  if (arr->size == 0)
  {
    arr->reserved = amount;
    arr->data = (void*)malloc(sizeof(unsigned short) * amount);

    return;
  }
  else
  {
    temp = (unsigned short*)malloc(sizeof(unsigned short) * newSize);

    for (i = 0; i < arr->size; i++)
    {
      temp[i] = ((unsigned short*)arr->data)[i];
    }

    free(arr->data);
    arr->data = (void*)temp;
    arr->reserved = amount;
  }
}

void reservei(array_t *arr, int amount)
{
  int *temp;
  int newSize = arr->size + amount;
  int i;

  if (amount <= 0) return;

  if (arr->size == 0)
  {
    arr->reserved = amount;
    arr->data = (void*)malloc(sizeof(int) * amount);

    return;
  }
  else
  {
    temp = (int*)malloc(sizeof(int) * newSize);

    for (i = 0; i < arr->size; i++)
    {
      temp[i] = ((int*)arr->data)[i];
    }

    free(arr->data);
    arr->data = (void*)temp;
    arr->reserved = amount;
  }
}

void reservef(array_t *arr, int amount)
{
  float *temp;
  int newSize = arr->size + amount;
  int i;

  if (amount <= 0) return;

  if (arr->size == 0)
  {
    arr->reserved = amount;
    arr->data = (void*)malloc(sizeof(float) * amount);

    return;
  }
  else
  {
    temp = (float*)malloc(sizeof(float) * newSize);

    for (i = 0; i < arr->size; i++)
    {
      temp[i] = ((float*)arr->data)[i];
    }

    free(arr->data);
    arr->data = (void*)temp;
    arr->reserved = amount;
  }
}

void initArray(array_t *arr)
{
  arr->data = NULL;
  arr->size = 0;
  arr->reserved = 0;
}

void clearData(array_t *arr)
{
  if (arr->size <= 0) return;
  free(arr->data);
  arr->data = NULL;
  arr->size = 0;
  arr->reserved = 0;
}

void clearAndFree(array_t *arr)
{
  if (arr->size <= 0) return;
  free(arr->data);
  arr->data = NULL;
  arr->size = 0;
  arr->reserved = 0;

  free(arr);
}

int geti(array_t *arr, int index)
{
  if (index < 0 || index >= arr->size) return 0xFFFF;
  return ((int*)arr->data)[index];
}

float getf(array_t *arr, int index)
{
  if (index < 0 || index >= arr->size)
  {
    printf("OUT OF BOUNDS AT %d\n", index);
    return (float)0xFFFF;
  }
  return ((float*)arr->data)[index];
}

boolean_t compareStrings(const char *str1, const char *str2)
{
  int len1 = (int)strlen(str1);
  int len2 = (int)strlen(str2);
  int i;

  if (len1 != len2) return _FALSE;

  for (i = 0; i < len1; i++)
  {
    if (str1[i] != str2[i]) return _FALSE;
  }

  return _TRUE; // both were equal
}

array_t copyi(array_t *from)
{
  int i;
  array_t arrayi;
  arrayi.data = (void*)malloc(from->size);
  arrayi.size = from->size;

  for (i = 0; i < from->size; i++)
  {
    ((int*)arrayi.data)[i] = ((int*)from->data)[i];
  }

  return arrayi;
}

array_t copyf(array_t *from)
{
  int i;
  array_t arrayf;
  arrayf.data = (void*)malloc(from->size);
  arrayf.size = from->size;

  for (i = 0; i < from->size; i++)
  {
    ((float*)arrayf.data)[i] = ((float*)from->data)[i];
  }

  return arrayf;
}

boolean_t checkCharValidity(int c)
{
  return c >= 0 && c <= 127;
}

boolean_t checkForAlpha(int c)
{
  if (c >= 'a' && c <= 'z') return _TRUE;
  else if (c >= 'A' && c <= 'Z') return _TRUE;

  return _FALSE; // was not an alphabetical character
}

boolean_t checkForLower(int c)
{
  if (c >= 'a' && c <= 'z') return _TRUE;

  return _FALSE; // was not an alphabetical character
}

char *arrayToChar(array_t *arr)
{
  char *data = (char*)malloc(sizeof(char)*arr->size + 1);
  int i;

  for (i = 0; i < arr->size; i++)
  {
    data[i] = ((int*)arr->data)[i];
  }

  data[arr->size] = 0;
  return data;
}

array_t charToArray(char *str)
{
  array_t arr;
  initArray(&arr);
  int i, len;
  if (!str) return arr;

  len = strlen(str);
  for (i = 0; i < len; i++)
  {
    appendi(&arr, str[i]);
  }
  return arr;
}

#ifdef linux
int _getch()
{
  struct termios currTerminal, newTerminal;
  int ch;

  tcgetattr(STDIN_FILENO, &currTerminal);
  newTerminal = currTerminal;
  newTerminal.c_lflag &= ~(ICANON | ECHO);

  tcsetattr(STDIN_FILENO, TCSANOW, &newTerminal);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &currTerminal);

  return ch;
}
#endif