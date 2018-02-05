#pragma once

#ifndef CM_COMMAND_H
#define CM_COMMAND_H

#include <string.h>
#include "utility.h"

typedef void(*cmd_t)(void);

typedef struct command_s
{
  struct command_s *next;
  array_t          id;
  cmd_t            cmd;
} command_t;

void cmd_Init(void);
void cmd_Shutdown(void);
void cmd_AddCommand(const char *id, cmd_t command);
void cmd_AddToBuffer(const char *id); // add a command to be executed
void cmd_ExecuteCommands(void);
void cmd_ExecuteSingle(const char *id); // executes single command immediately
void cmd_FlushBuffer(void);
void cmd_PrintCommands(void); // prints all logged commands by id
void cmd_PrintArray(array_t *arr);

int cmd_GetNumArgs(void);
char *cmd_GetArg(int index);

boolean_t cmd_ReadStrTo(array_t *out, const char *str);
boolean_t cmd_StrCmp(array_t *id, const char *str);
boolean_t cmd_ArrCmpi(array_t *arr1, array_t *arr2);

#endif // COMMAND_H