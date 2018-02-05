#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include "utility.h"

#define MAX_PATH 256
#define INVALID -1

typedef int fileHandle_t;

void fs_Init(void);
void fs_SetCWD(const char *path);
void fs_Shutdown(void);
void fs_ProfileCWD(void);

fileHandle_t fs_Open(const char *file, const char *tag);
void fs_Close(fileHandle_t handle);
char *fs_GetPath(fileHandle_t handle);
char *fs_FindFile(const char *file);

void fs_ReadFile(fileHandle_t handle, array_t *buffer);
void fs_WriteFile(fileHandle_t handle, array_t *input);

#endif