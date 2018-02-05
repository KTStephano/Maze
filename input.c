#include "input.h"
#include "command.h"
#include "utility.h"
#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#endif

#define UNBOUND -1
#define MAX_CMD_LEN 64

static i_key_t keys[MAX_KEYS];
static int currKey = -1; // only one function will set this

void i_Init(void)
{
  int i;
  char str[2];
  str[1] = '\0';

  for (i = 0; i < MAX_KEYS; i++)
  {
    keys[i].keyID = UNBOUND;
    keys[i].bIsDown = _FALSE;
    keys[i].bKeyUp = _FALSE;
    keys[i].bNoEvent = _TRUE;

    str[0] = i;
    cmd_AddCommand(&str[0], i_KeyEvent);
  }
}

void i_Shutdown(void)
{
  int i;

  for (i = 0; i < MAX_KEYS; i++)
  {
    i_UnbindKey(i);
  }
}

void i_KeyEvent(void)
{
  char cmdBuf[MAX_CMD_LEN];

  if (currKey + 1 >= MAX_KEYS) currKey = -1; // make sure this gets reset
  currKey++; // increment the current key

  if (keys[currKey].keyID == UNBOUND) return; // not bound
  else if (keys[currKey].bNoEvent) return; // no event logged

  if (keys[currKey].bIsDown) // keydown event
  {
    if (keys[currKey].bindings[0] != "")
    {
      strcpy(cmdBuf, &keys[currKey].bindings[0][0]);
      //cmd_AddToBuffer(cmdBuf);
      cmd_ExecuteSingle(cmdBuf);
#ifdef linux
      // first set the key to bNoEvent
      keys[currKey].bIsDown = _FALSE;
      keys[currKey].bKeyUp = _FALSE;
      keys[currKey].bNoEvent = _TRUE;
      // queue keydown event ASAP for linux
      strcpy(cmdBuf, &keys[currKey].bindings[1][0]);
      //cmd_AddToBuffer(cmdBuf);
      cmd_ExecuteSingle(cmdBuf);
#endif
    }
  }
#ifdef _WIN32
  else if (keys[currKey].bKeyUp) // keyup event
  {
    if (keys[currKey].bindings[1] != "")
    {
      // first set the key to bNoEvent
      keys[currKey].bIsDown = _FALSE;
      keys[currKey].bKeyUp = _FALSE;
      keys[currKey].bNoEvent = _TRUE;
      strcpy(cmdBuf, &keys[currKey].bindings[1][0]);
      //cmd_AddToBuffer(cmdBuf);
      cmd_ExecuteSingle(cmdBuf);
    }
  }
#endif
}

void i_BindKey(int keyID, char *keyDown, char *keyUp)
{
  keyID = toupper(keyID);
  i_UnbindKey(keyID); // make sure nothing is bound

  //printf("i_BindKey; bindings[0] = %s ", keyDown);
  //printf("i_BindKey; bindings[1] = %s\n", keyUp);

  if (keyID >= 0 && keyID < MAX_KEYS)
  {
    keys[keyID].keyID = keyID;

    keys[keyID].bindings[0] = (char*)malloc(strlen(keyDown) + 1);
    keys[keyID].bindings[1] = (char*)malloc(strlen(keyUp) + 1);

    strcpy(keys[keyID].bindings[0], keyDown);
    strcpy(keys[keyID].bindings[1], keyUp);

    //printf("i_BindKey; keys[0] = %s ", keys[keyID].bindings[0]);
    //printf("i_BindKey; keys[1] = %s\n", keys[keyID].bindings[1]);
  }
}

void i_RebindKey(int oldKey, int newKey)
{
  if ((oldKey >= 0 && oldKey < MAX_KEYS) || (newKey >= 0 && oldKey < MAX_KEYS))
  {
    return; // invalid key num(s)
  }

  i_BindKey(newKey, keys[oldKey].bindings[0], keys[oldKey].bindings[1]);
  i_UnbindKey(oldKey);
}

void i_UnbindKey(int keyID)
{
  if (keyID >= 0 && keyID < MAX_KEYS)
  {
    if (keys[keyID].keyID == UNBOUND) return;
    else
    {
      keys[keyID].keyID = UNBOUND;
      free(keys[keyID].bindings[0]);
      free(keys[keyID].bindings[1]);
    }
  }
}

void i_ProcessKeyInput(void)
{
  int i;
  char str[2];
  str[1] = '\0';

#ifdef linux
  int c = 0;
  c = _getch();
  c = toupper(c);
#endif

  for (i = 0; i < MAX_KEYS; i++)
  {
    //if (checkForLower(i)) continue;
#ifdef _WIN32
    if (GetAsyncKeyState(i) != 0)
#endif
#ifdef linux
    if (c == i)
#endif
    {
      keys[i].bIsDown = _TRUE;
      keys[i].bKeyUp = _FALSE;
      keys[i].bNoEvent = _FALSE;
    }
#ifdef _WIN32
    else if (keys[i].bIsDown == _TRUE) // was down as of last call
    {
      //printf("ku %c\n", i);
      keys[i].bIsDown = _FALSE;
      keys[i].bKeyUp = _TRUE;
      keys[i].bNoEvent = _FALSE;
    }
#endif

    str[0] = i;
    cmd_AddToBuffer(&str[0]); // queue a key event
  }
}