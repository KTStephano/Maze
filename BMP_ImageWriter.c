/**
 * I DO NOT OWN THIS! Credit to Joel Castellanos for providing the code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BMP_ImageWriter.h"

const drawFunc D_FUNCS[4] = { drawNorth, drawEast, drawSouth, drawWest };

//========================================================================
//This function copies 4 bytes from in int to an unsigned char array where
//  the least significant byte of the int is placed in the first element
//  of the array.
//========================================================================
void copyIntToAddress(int n, unsigned char bytes[])
{
  bytes[0] = n & 0xFF;
  bytes[1] = (n >> 8) & 0xFF;
  bytes[2] = (n >> 16) & 0xFF;
  bytes[3] = (n >> 24) & 0xFF;
}



//========================================================================
//Sets the RGB value of a single pixel at coordinates (x,y) in the
//   character array, data in bitmap format with three bytes per pixel.
//
//Bitmap format stores rows from bottom to top. Therefore, this function
//   needs the pixelHeight to calculate the offset into data.
//
//Bitmap format requires that each row is divisible by 4. Therefore, 
//   rowSize may need to be padded up to 3 bytes past the end of the data. 
//========================================================================
void setRGB(unsigned char data[], int x, int y, int rowSize,
  int pixelHeight,
  unsigned char r, unsigned char g, unsigned char b)
{
  y = (pixelHeight - y) - 1;
  int offset = (x * 3) + (y * rowSize);
  data[offset] = b;
  data[offset + 1] = g;
  data[offset + 2] = r;
}

void getRGB(unsigned char *data, int x, int y, int rowSize, int pixelHeight,
  unsigned char *r, unsigned char *g, unsigned char *b)
{
  y = (pixelHeight - y) - 1;
  int offset = (x * 3) + (y * rowSize);
  *b = data[offset];
  *g = data[offset + 1];
  *r = data[offset + 2];
}

void drawNorth(unsigned char *img, int rowSize, int pixelHeight, int offsetX, int offsetY)
{
  int i;
  unsigned char r, g, b;

  for (i = 0; i < 6; i++)
  {
    setRGB(&img[0], 2 + offsetX, i + offsetY, rowSize, pixelHeight, 0, 0, 0);
    setRGB(&img[0], 5 + offsetX, i + offsetY, rowSize, pixelHeight, 0, 0, 0);
  }

  for (i = 2; i < 6; i++)
  {
    setRGB(&img[0], i + offsetX, 5 + offsetY, rowSize, pixelHeight, 0, 0, 0);
  }
}

void drawSouth(unsigned char *img, int rowSize, int pixelHeight, int offsetX, int offsetY)
{
  int i;
  for (i = 3; i < 8; i++)
  {
    setRGB(&img[0], 2 + offsetX, i + offsetY, rowSize, pixelHeight, 0, 0, 0);
    setRGB(&img[0], 5 + offsetX, i + offsetY, rowSize, pixelHeight, 0, 0, 0);
  }

  for (i = 2; i < 6; i++)
  {
    setRGB(&img[0], i + offsetX, 2 + offsetY, rowSize, pixelHeight, 0, 0, 0);
  }
}

void drawWest(unsigned char *img, int rowSize, int pixelHeight, int offsetX, int offsetY)
{
  int i;
  for (i = 0; i < 6; i++)
  {
    setRGB(&img[0], i + offsetX, 2 + offsetY, rowSize, pixelHeight, 0, 0, 0);
    setRGB(&img[0], i + offsetX, 5 + offsetY, rowSize, pixelHeight, 0, 0, 0);
  }

  for (i = 2; i < 6; i++)
  {
    setRGB(&img[0], 5 + offsetX, i + offsetY, rowSize, pixelHeight, 0, 0, 0);
  }
}

void drawEast(unsigned char *img, int rowSize, int pixelHeight, int offsetX, int offsetY)
{
  int i;
  for (i = 0; i < 6; i++)
  {
    setRGB(&img[0], i + 2 + offsetX, 2 + offsetY, rowSize, pixelHeight, 0, 0, 0);
    setRGB(&img[0], i + 2 + offsetX, 5 + offsetY, rowSize, pixelHeight, 0, 0, 0);
  }

  for (i = 2; i < 6; i++)
  {
    setRGB(&img[0], 2 + offsetX, i + offsetY, rowSize, pixelHeight, 0, 0, 0);
  }
}

void fixWalls(unsigned char *img, int rowSize, int pixelHeight, int offsetX, int offsetY)
{
  int i, k, cx, cy;
  unsigned char r, g, b;
  // Check north, south, west, east
  const char INDICES_X[] = { 2, 2, 0, 7 };
  const char INDICES_Y[] = { 0, 7, 2, 2 };
  const char XVERSESY[] = { 1, 1, 0, 0 }; // 1 = x, 0 = y
  const char DEFAULT[] = { 2, 5, 2, 5 };

  for (i = 0; i < 4; i++)
  {
    getRGB(img, INDICES_X[i] + offsetX, INDICES_Y[i] + offsetY, rowSize, pixelHeight, &r, &g, &b);
    if (!r && !g && !b)
    {
      for (k = 3; k < 5; k++)
      {
        cx = XVERSESY[i] ? k : DEFAULT[i];
        cy = !XVERSESY[i] ? k : DEFAULT[i];

        setRGB(&img[0], cx + offsetX, cy + offsetY, rowSize, pixelHeight, 255, 255, 255);
      }
    }
  }
}
