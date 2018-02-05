/*
 * I DO NOT OWN THIS! Credit to Joel Castellanos for providing the code. 
*/

#ifndef BMP_IMG_WRITER_H
#define BMP_IMG_WRITER_H

typedef void(*drawFunc)(unsigned char *, int, int, int, int);

extern const drawFunc D_FUNCS[4];

void copyIntToAddress(int n, unsigned char bytes[]);
void setRGB(unsigned char data[], int x, int y, int rowSize,
  int pixelHeight,
  unsigned char r, unsigned char g, unsigned char b);
void getRGB(unsigned char *data, int x, int y, int rowSize, int pixelHeight,
  unsigned char *r, unsigned char *g, unsigned char *b);
void drawNorth(unsigned char *img, int rowSize, int pixelHeight, int offsetX, int offsetY);
void drawSouth(unsigned char *img, int rowSize, int pixelHeight, int offsetX, int offsetY);
void drawWest(unsigned char *img, int rowSize, int pixelHeight, int offsetX, int offsetY);
void drawEast(unsigned char *img, int rowSize, int pixelHeight, int offsetX, int offsetY);
void drawEast(unsigned char *img, int rowSize, int pixelHeight, int offsetX, int offsetY);
void fixWalls(unsigned char *img, int rowSize, int pixelHeight, int offsetX, int offsetY);

#endif // BMP_IMG_WRITER_H
