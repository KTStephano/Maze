#pragma once

#ifndef CM_UTILITY_H
#define CM_UTILITY_H

#include <stdio.h>
#include <stdlib.h>

typedef enum boolean_s
{
  _FALSE,
  _TRUE
} boolean_t;

typedef struct
{
  void			*data;
  int				size;
  int       reserved;
} array_t;

void appendus(array_t *arr, unsigned short data);
void appendi(array_t *arr, int data);
void appendf(array_t *arr, float data);
void reserveus(array_t *arr, int amount);
void reservei(array_t *arr, int amount);
void reservef(array_t *arr, int amount);
void initArray(array_t *arr);
void clearData(array_t *arr);
void clearAndFree(array_t *arr);

int geti(array_t *arr, int index);
float getf(array_t *arr, int index);

boolean_t compareStrings(const char *str1, const char *str2);

array_t copyi(array_t *from);
array_t copyf(array_t *from);

boolean_t checkCharValidity(int c);
boolean_t checkForAlpha(int c);
boolean_t checkForLower(int c);

char *arrayToChar(array_t *arr); // converts array_t to char *
array_t charToArray(char *str);

#ifdef linux
int _getch();
#endif

#endif // CM_UTILITY_H