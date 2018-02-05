/*************************************************************/
/*Justin Hall                                                */
/*                                                           */
/*The purpose of this file is to implement the recursive     */
/*  backtracking algorithm (using a version of Joel's        */
/*  Algorithm) in order to generate basic random mazes.      */
/*  Entry points to this file are prototyped inside of       */
/*  mazegen.h - always call mazeGenerate first, though not   */
/*  doing so will NOT crash the program.                     */
/*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mazegen.h"

#define MAZEIII

#ifdef MAZEIII
#include "BMP_ImageWriter.h"
#endif

#define MAX_STACK 173000
#define MAX_RAND_SETS 32
#define NUM_DIRECTIONS 4
#define INVALID -1
#define BMP_HEADER 54
#define MIN(x, y) (x < y ? x : y)
#define GOSTRAIGHT(pcent) (pcent > (double)rand()/(double)RAND_MAX)

const int DIRECTION_LIST[] = { NORTH, EAST, SOUTH, WEST };
const int DIRECTION_DX[] = { 0, 1, 0, -1 };
const int DIRECTION_DY[] = { -1, 0, 1, 0 };


const unsigned char pipeList[] =
{
  219, 208, 198, 200, 210, 186, 201, 204,
  181, 188, 205, 202, 187, 185, 203, 206
};

const int DIRECTION_MAP[] = { SOUTH, WEST, NORTH, EAST };
char rand_alleys[4];
char alleymap_X[4];
char alleymap_Y[4];

static uint8 randomSets[MAX_RAND_SETS];
static int numVisited = 0;
static int mWidth;
static int mHeight;
static short currX;
static short currY;
static int numCells;
static char alleyIndex = 0;
static int recursionLevel = 0;
static char bFoundWay = 0;
static char bFoundExit = 0;
static char bCallSolve = 1;

#ifdef MAZEIII
unsigned char header[54] =
{
  'B', 'M',  // magic number
  0, 0, 0, 0,  // size in bytes (set below)
  0, 0, 0, 0,  // reserved
  54, 0, 0, 0, // offset to start of pixel data
  40, 0, 0, 0, // info hd size
  0, 0, 0, 0,  // image width (set below)
  0, 0, 0, 0,  // image heigth (set below)
  1, 0,      // number color planes
  24, 0,     // bits per pixel
  0, 0, 0, 0,  // compression is none
  0, 0, 0, 0,  // image bits size
  0x13, 0x0B, 0, 0, // horz resoluition in pixel / m
  0x13, 0x0B, 0, 0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
  0, 0, 0, 0,  // #colors in pallete
  0, 0, 0, 0,  // #important colors
};

typedef struct
{
  uint8 *image; // for .bmp image
  int imgFileSize;
  int pixelWidth;
  int pixelHeight;
  int rowSize;
  int pixelDataSize;
} bmp_image_t;

static bmp_image_t *mazeImg = NULL;
#endif

static maze_t *maze = NULL;

/*************************************************************/
/*int width:                                                 */
/*  in,                                                      */
/*  width of the maze,                                       */
/*  can be any value - it will be checked for validity.      */
/*int height:                                                */
/*  in,                                                      */
/*  height of the maze,                                      */
/*  can be any value - it will be checked for validity.      */
/*int wayX:                                                  */
/*  in,                                                      */
/*  waypointX for the starting maze,                         */
/*  can be any value - it will be checked for validity.      */
/*int wayY:                                                  */
/*  in,                                                      */
/*  waypointY for the starting maze,                         */
/*  can be any value - it will be checked for validity.      */
/*Returns TRUE if there is something wrong with the inputs,  */
/*  FALSE otherwise.                                         */
/*This function takes some inputs required for the maze      */
/*  and makes sure none of them are invalid.                 */
/*Uses simple if/else if statements to check for             */
/*  valid inputs.                                            */
/*************************************************************/
int checkForNonsense(int width, int height, int wayX, int wayY, int wayLen,
  double wayDirPer, double straightProb, int printSteps)
{
  if (width < 3 || width > 1000) return TRUE;
  else if (height < 3 || height > 1000) return TRUE;
  else if (wayX < 1 || wayX > width) return TRUE;
  else if (wayY < 1 || wayY > height) return TRUE;
  else if (wayLen < 0 || wayLen > MIN(width, height) / 2) return TRUE;
  else if (wayDirPer < 0 || wayDirPer > 1.0) return TRUE;
  else if (straightProb < 0 || straightProb > 1.0) return TRUE;
  else if (printSteps != TRUE && printSteps != FALSE) return TRUE;

  return FALSE;
}

/*************************************************************/
/*int width:                                                 */
/*  in,                                                      */
/*  width of the maze,                                       */
/*  must be between 3 and 1000.                              */
/*int height:                                                */
/*  in,                                                      */
/*  height of the maze,                                      */
/*  must be between 3 and 1000.                              */
/*No return.                                                 */
/*This function allocates any dynamic memory the maze will   */
/*  need.                                                    */
/*Calls mazeFree() to make sure no leftover data is there to */
/*  prevent memory leaks. Then allocates the pointers that   */
/*  will point to each column of the 2D maze array. After    */
/*  this it loops through width number of times, allocating  */
/*  each column dynamically.                                 */
/*************************************************************/
maze_t *allocateMazeData(int width, int height)
{
  printf("w = %d, h = %d\n", width, height);
  mazeFree();
  int i, rowPadding;

  maze = (maze_t*)malloc(sizeof(maze_t));
  maze->data = (uint8**)malloc(sizeof(uint8*)*width);
  for (i = 0; i < width; i++)
  {
    maze->data[i] = (uint8*)malloc(sizeof(uint8)*height);
    memset(maze->data[i], 0, height);
  }
  maze->width = width;
  maze->height = height;

#ifdef MAZEIII
  mazeImg = malloc(sizeof(bmp_image_t));
  mazeImg->pixelWidth = 8;
  mazeImg->pixelHeight = 8;
  mazeImg->rowSize = mazeImg->pixelWidth * 3;
  rowPadding = (4 - (mazeImg->rowSize % 4)) % 4;
  //mazeImg->rowSize += rowPadding;
  mazeImg->pixelDataSize = mazeImg->rowSize*mazeImg->pixelHeight*width*height;
  mazeImg->imgFileSize = BMP_HEADER + mazeImg->pixelDataSize;

  mazeImg->image = malloc(sizeof(uint8) * mazeImg->pixelDataSize);
  memset(&mazeImg->image[0], 0xFF, sizeof(unsigned char) *
    mazeImg->pixelDataSize);

  printf("mazeImg->pixelWidth = %d\n", mazeImg->pixelWidth);
  printf("mazeImg->pixelHeight = %d\n", mazeImg->pixelHeight);
  printf("mazeImg->pixelDataSize = %d\n", mazeImg->pixelDataSize);
#endif

  return maze;
}

/*************************************************************/
/*short x:                                                   */
/*  in,                                                      */
/*  x position in the maze,                                  */
/*  can be any value - it will be checked for validity.      */
/*short y:                                                   */
/*  in,                                                      */
/*  y position in the maze,                                  */
/*  can be any value - it will be checked for validity.      */
/*Returns TRUE if valid, FALSE if not.                       */
/*This function takes x/y inputs for the maze                */
/*  and makes sure neither of them are invalid.              */
/*Uses simple if/else if statements to check for             */
/*  valid inputs.                                            */
/*************************************************************/
int checkBounds(short x, short y)
{
  if (x > maze->width - 1) return FALSE;
  else if (x < 0) return FALSE;
  else if (y > maze->height - 1) return FALSE;
  else if (y < 0) return FALSE;

  return TRUE;
}

/*************************************************************/
/*short x:                                                   */
/*  in,                                                      */
/*  x position in the maze,                                  */
/*  can be any value - should be a valid x-input.            */
/*short y:                                                   */
/*  in,                                                      */
/*  y position in the maze,                                  */
/*  can be any value - should be a valid y-input.            */
/*Returns INVALID if the inputs aren't on an edge of the maze*/
/*  and a specific value that maps to DIRECTION_LIST         */
/*  if either is on an edge.                                 */
/*This function takes x/y inputs for the maze                */
/*  and checks to see if either is on an edge.               */
/*Uses simple if/else if statements to check if either is on */
/*  an edge. The return value is designed to map to          */
/*  DIRECTION_LIST, so if it is on an edge using             */
/*  maze->data[x][y] |= DIRECTION_LIST[return_value] is not  */
/*  only valid but the intended use of this function (mostly */
/*  allows for the exit to be punched out when an edge is    */
/*  found).                                                  */
/*************************************************************/
int checkForEdge(short x, short y)
{
  if (x == 0 || x == mWidth) return x ? 1 : 3;
  else if (y == 0 || y == mHeight) return y ? 2 : 0;
  return INVALID;
}

/*************************************************************/
/*short x:                                                   */
/*  in,                                                      */
/*  x position in the maze,                                  */
/*  can be any value - should be a valid x-input.            */
/*short y:                                                   */
/*  in,                                                      */
/*  y position in the maze,                                  */
/*  can be any value - should be a valid y-input.            */
/*Returns TRUE if none of the 5 forward directions contain   */
/*  an active cell and they are not on an edge.              */
/*This function takes x/y inputs for the maze                */
/*  and sees if the forward 5 directions from the cell are   */
/*  already occupied.                                        */
/*Relies heavily the VISITED flag to have been set on the    */
/*  **previous** cell. This means that if you are coming from*/
/*  position (1, 1) and you are checking if (1, 2) has no    */
/*  adjacent cells already active, first call the markCell   */
/*  function on position (1, 1) and then call this function  */
/*  for position (1, 2).                                     */
/*************************************************************/
int checkAdjacent(short x, short y)
{
  char cx, cy;

  for (cy = -1; cy < 2; cy++)
  {
    for (cx = -1; cx < 2; cx++)
    {
      if (x + cx == x && y + cy == y) continue;
      else if (!checkBounds(x + cx, y + cy)) return FALSE;
      else if (maze->data[x + cx][y + cy] & VISITED) continue;
      else if (maze->data[x + cx][y + cy]) return FALSE;
    }
  }

  return TRUE;
}

/*************************************************************/
/*short x:                                                   */
/*  in,                                                      */
/*  x position in the maze,                                  */
/*  can be any value - should be a valid x-input.            */
/*short y:                                                   */
/*  in,                                                      */
/*  y position in the maze,                                  */
/*  can be any value - should be a valid y-input.            */
/*No return.                                                 */
/*This function takes an x/y position and creates a + sign   */
/*  outward from (and including) the x/y position in the     */
/*  way of marking each cell in the + with the VISITED flag. */
/*This function is built to work alongside the checkAdjacent */
/*  function. When this is called on an x, y position, it    */
/*  exclusive-ORs each cell going outward from the x, y      */
/*  position in the shape of a + (where the distance from the*/
/*  x, y position is always +1 or -1) with the VISITED flag. */
/*  This means that calling it twice will first mark the     */
/*  cells and then unmark them. So, to use this, first call  */
/*  this function on the x, y position, then call            */
/*  the checkAdjacent function, and then call this again     */
/*  on the same x, y position.                               */
/*************************************************************/
void markCell(short x, short y)
{
  int i;
  short cx, cy;
  maze->data[x][y] ^= VISITED; // mark current cell first

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    cx = x + DIRECTION_DX[i];
    cy = y + DIRECTION_DY[i];
    if (!checkBounds(cx, cy)) continue;
    maze->data[cx][cy] ^= VISITED;
  }
}

/*************************************************************/
/*short x:                                                   */
/*  in,                                                      */
/*  x position in the maze,                                  */
/*  can be any value - should be a valid x-input.            */
/*short y:                                                   */
/*  in,                                                      */
/*  y position in the maze,                                  */
/*  can be any value - should be a valid y-input.            */
/*char direction:                                            */
/*  in,                                                      */
/*  index of the direction to move,                          */
/*  should either directly map to DIRECTION_DX/DY or         */
/*  directly map to randomSets.                              */
/*Returns TRUE if going straight was fully successful,       */
/*  FALSE otherwise.                                         */
/*This function uses the GOSTRAIGHT macro along with         */
/*  the straight probability to help determine if continuing */
/*  in the same direction as the cell before is valid.       */
/*This function first uses GOSTRAIGHT to determine if it     */
/*  should attempt to continue going straight rather than    */
/*  choosing a new direction. If so, it then sets up currX   */
/*  and currY using the direction input. Next it checks the  */
/*  bounds of the next desired cell as well as whether or    */
/*  not it is occupied. Finally it marks the current cell,   */
/*  calls checkAdjacent for currX and currY, and if that     */
/*  succeeds it will carve a path straight into the new cell */
/*  and return TRUE to let you know it succeeded. Otherwise, */
/*  FALSE is returned to let you know it failed.             */
/*************************************************************/
int checkForStraight(short x, short y, char direction)
{
  if (GOSTRAIGHT(maze->straightProb))
  {
    currX = x;
    currY = y;
    if (direction < 4)
    {
      currX += DIRECTION_DX[direction];
      currY += DIRECTION_DY[direction];
    }
    else
    {
      currX += DIRECTION_DX[randomSets[direction]];
      currY += DIRECTION_DY[randomSets[direction]];
    }
    if (checkBounds(currX, currY) && !maze->data[currX][currY])
    {
      markCell(x, y);
      if (checkAdjacent(currX, currY))
      {
        markCell(x, y);
        maze->data[x][y] |= direction < 4 ? DIRECTION_LIST[direction] :
          DIRECTION_LIST[randomSets[direction]];
        maze->data[currX][currY] |= direction < 4 ? DIRECTION_MAP[direction] :
          DIRECTION_MAP[randomSets[direction]];

        return TRUE;
      }
      else markCell(x, y);
    }
  }

  return FALSE;
}

/*************************************************************/
/*short x:                                                   */
/*  in,                                                      */
/*  current x-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*short y:                                                   */
/*  in,                                                      */
/*  current y-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*char direction:                                            */
/*  in,                                                      */
/*  index of the direction to move,                          */
/*  should either directly map to DIRECTION_DX/DY or         */
/*  directly map to randomSets.                              */
/*No return.                                                 */
/*This function calls itself recursively to create the maze. */
/*After checking to see it hasn't hit the recursive depth    */
/*  cap, it selects a random index that corresponds to       */
/*  the randomSets array - this determines which element     */
/*  of DX and DY is called, thus randomizing the direction.  */
/*  Inside the loop, it checks each direction that it can    */
/*  potentially move and, if it determines it is valid,      */
/*  calls itself with the new x and y coordinates.           */
/*************************************************************/
void createMaze_Recursive(short x, short y, char direction)
{
  if (recursionLevel > MAX_STACK) return;

  char i, k;
  char cAdjacent = 1;
  char index = rand() % MAX_RAND_SETS / NUM_DIRECTIONS * NUM_DIRECTIONS;

  recursionLevel++;

  if (checkForStraight(x, y, direction))
  {
    createMaze_Recursive(currX, currY, direction);
  }

  for (i = 0; i < 2; i++)
  {
    for (k = 0; k < NUM_DIRECTIONS; k++)
    {
      currX = x + DIRECTION_DX[randomSets[index + k]];
      currY = y + DIRECTION_DY[randomSets[index + k]];

      if (currX < 0 || currX > mWidth || currY < 0 || currY > mHeight ||
        maze->data[currX][currY]) continue;

      if (cAdjacent)
      {
        markCell(x, y);
        if (!checkAdjacent(currX, currY))
        {
          markCell(x, y); // will unmark
          continue;
        }
        markCell(x, y); // will unmark
      }

      maze->data[x][y] |= DIRECTION_LIST[randomSets[index + k]];
      maze->data[currX][currY] |= DIRECTION_MAP[randomSets[index + k]];
      createMaze_Recursive(currX, currY, index + k);
    }
    cAdjacent = 0;
  }

  recursionLevel--;
}

/*************************************************************/
/*char *arr:                                                 */
/*  in,                                                      */
/*  array of characters to search,                           */
/*  should have at least 1 element.                          */
/*int indices:                                               */
/*  in,                                                      */
/*  number of indices in the array,                          */
/*  must not be negative or outside the bounds of *arr.      */
/*char num:                                                  */
/*  in,                                                      */
/*  the number to search the array for,                      */
/*  can be any number.                                       */
/*Returns TRUE if the array contains num, FALSE otherwise.   */
/*This function searches *arr for the number                 */
/*Loops through *arr and if it finds the number, returns     */
/*  TRUE. Otherwise FALSE is returned.                       */
/*************************************************************/
int containsNum(char *arr, int indices, char num)
{
  int i;

  for (i = 0; i < indices; i++) if (arr[i] == num) return TRUE;
  return FALSE;
}

/*************************************************************/
/*No inputs.                                                 */
/*No return.                                                 */
/*The purpose of this function is to limit *all* cells in    */
/*  the maze to 0x0F, eliminating all other bits.            */
/*This function loops through the entire maze and ANDs each  */
/*  cell with BITSLICE_0x0F.                                 */
/*************************************************************/
void sliceBits_0x0F()
{
  if (maze)
  {
    int x, y;
    for (y = 0; y < mHeight + 1; y++)
    {
      for (x = 0; x < mWidth + 1; x++)
      {
        maze->data[x][y] &= BITSLICE_0x0F;
      }
    }
  }
}

/*************************************************************/
/*uint8 *arr:                                                */
/*  in,                                                      */
/*  array to place the random values,                        */
/*  should have at least 2 elements.                         */
/*int indices:                                               */
/*  in,                                                      */
/*  number of indices in *arr,                               */
/*  must not be negative or exceed the length of *arr.       */
/*No return.                                                 */
/*The purpose of this is to generate an array of values      */
/*  on the range of [0, 3].                                  */
/*Sets index 0 to be a random number. Then, it loops through */
/*  and continuously gets new random numbers and checks to   */
/*  see if the array already contains that value. If not,    */
/*  the current index of the array is given the random value */
/*  and i is incremented.                                    */
/*************************************************************/
void randomizeArray(uint8 *arr, int indices, int limit)
{
  char i = 0;
  char index;
  arr[0] = rand() % limit;

  while (i < indices)
  {
    index = rand() % limit;
    if (!containsNum(arr, indices, index))
    {
      arr[i] = index;
      i++;
    }
  }
}

/*************************************************************/
/*No inputs.                                                 */
/*No return.                                                 */
/*This function initializes the arrays that are necessary    */
/*  to work with the alleys for Maze II.                     */
/*This will loop through 4 times, mapping each element       */
/*  of alleymap_X/Y with the elements of DIRECTION_LIST.     */
/*  It does this by adding the result of the multiplication  */
/*  between maze->alleyLen * DIRECTION_DX/DY[i], so there    */
/*  will either be the addition of maze->alleyLen to         */
/*  maze->wayX/wayY, the subtraction of maze->alleyLen,      */
/*  of no change. This results in two arrays that can be     */
/*  used in conjunction with DIRECTION_LIST for ease of use. */
/*  Finally, rand_alleys elements are set to -1 and          */
/*  randomizeArray is called so that the alley access points */
/*  are randomized.                                          */
/*************************************************************/
void initArrays()
{
  int i;

  for (i = 0; i < 4; i++)
  {
    alleymap_X[i] = maze->wayX + (maze->alleyLen * DIRECTION_DX[i]);
    alleymap_Y[i] = maze->wayY + (maze->alleyLen * DIRECTION_DY[i]);

    if (alleymap_X[i] < 0) alleymap_X[i] = 0;
    else if (alleymap_X[i] > mWidth + 1) alleymap_X[i] = mWidth;
    if (alleymap_Y[i] < 0) alleymap_Y[i] = 0;
    else if (alleymap_Y[i] > mHeight + 1) alleymap_Y[i] = mHeight;
  }

  memset(&rand_alleys[0], -1, 4);
  randomizeArray(&rand_alleys[0], 4, 4);
}

/*************************************************************/
/*short x:                                                   */
/*  in,                                                      */
/*  current x-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*short y:                                                   */
/*  in,                                                      */
/*  current y-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*char direction:                                            */
/*  in,                                                      */
/*  index of the direction to move,                          */
/*  should either directly map to DIRECTION_DX/DY or         */
/*  directly map to randomSets.                              */
/*Returns TRUE if it has hit the end of the run (when        */
/*  cellsLeft is less than 1). This lets the caller know     */
/*  that it is time to stop and call createMaze_Recursive    */
/*  (after all alleys are finished). FALSE is returned       */
/*  otherwise.                                               */
/*This function calls itself recursively to create the maze  */
/*  from the alley entry points.                             */
/*After checking to see it hasn't hit the recursive depth    */
/*  cap and that it still has cells left and that it should  */
/*  not continue going straight, it selects a random index   */
/*  that corresponds to the randomSets array - this          */
/*  determines which element of DX and DY is called, thus    */
/*  randomizing the direction. Inside the loop, it checks    */
/*  each direction that it can potentially move and, if it   */
/*  determines it is valid, (i.e. there are no active cells  */
/*  adjacent to it), calls itself with the new x and y       */
/*  coordinates. During the second loop through, the         */
/*  check to see if there are no active adjacent cells       */
/*  is turned off to ensure that all parts of the maze are   */
/*  filled.                                                  */
/*************************************************************/
int carveAlleys_Recursive(short x, short y, char direction)
{
  static int cellsLeft = 0;
  if (recursionLevel > MAX_STACK) return FALSE;
  if (cellsLeft < 1)
  {
    alleyIndex++;
    if (maze->printSteps) mazePrint();
    for (alleyIndex; alleyIndex < NUM_DIRECTIONS; alleyIndex++)
    {
      cellsLeft = numCells;
      if (maze->printSteps) mazePrint();
      carveAlleys_Recursive(alleymap_X[rand_alleys[alleyIndex]],
        alleymap_Y[rand_alleys[alleyIndex]],
        rand_alleys[alleyIndex]);
      if (maze->printSteps) mazePrint();
    }
    if (maze->printSteps) mazePrint();

    createMaze_Recursive(x, y, direction);

    return TRUE;
  }

  char i, k;
  char index = rand() % MAX_RAND_SETS / NUM_DIRECTIONS * NUM_DIRECTIONS;
  char cAdjacent = 1;

  recursionLevel++;

  if (checkForStraight(x, y, direction))
  {
    --cellsLeft;
    carveAlleys_Recursive(currX, currY, direction);
  }

  for (i = 0; i < 2; i++)
  {
    for (k = 0; k < NUM_DIRECTIONS; k++)
    {
      currX = x + DIRECTION_DX[randomSets[index + k]];
      currY = y + DIRECTION_DY[randomSets[index + k]];

      if (!checkBounds(currX, currY) || maze->data[currX][currY]) continue;

      if (cAdjacent)
      {
        markCell(x, y);
        if (!checkAdjacent(currX, currY))
        {
          markCell(x, y); // will unmark
          continue;
        }
        markCell(x, y); // will unmark
      }
      maze->data[x][y] |= DIRECTION_LIST[randomSets[index + k]];
      maze->data[currX][currY] |= DIRECTION_MAP[randomSets[index + k]];
      --cellsLeft;
      if (carveAlleys_Recursive(currX, currY, index + k))
      {
        createMaze_Recursive(x, y, direction);
        recursionLevel--;
        return TRUE;
      }
    }

    cAdjacent = 0; // set this to off for the next round
  }

  recursionLevel--;
  return FALSE;
}

/*************************************************************/
/*short x:                                                   */
/*  in,                                                      */
/*  current x-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*short y:                                                   */
/*  in,                                                      */
/*  current y-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*char index:                                                */
/*  in,                                                      */
/*  index of the direction to move,                          */
/*  should directly map to alleymap_X/Y.                     */
/*No return.                                                 */
/*This function calls itself recursively to create the maze  */
/*  alleys.                                                  */
/*Checks to see if the current x/y position is equal to      */
/*  the current alleymap_X/Y limits (and calls the next alley*/
/*  if so). Otherwise, it continues moving towards the       */
/*  alleymap_X/Y limits for its current index by using       */
/*  DIRECTION_DX/DY.                                         */
/*************************************************************/
void setupAlleys_Recursive(short x, short y, char index)
{
  if (index > 3) return;
  else if (x == alleymap_X[index] && y == alleymap_Y[index])
  {
    setupAlleys_Recursive(maze->wayX, maze->wayY, index + 1);
    return;
  }

  currX = x + DIRECTION_DX[index];
  currY = y + DIRECTION_DY[index];

  if (!checkBounds(currX, currY)) return;

  maze->data[x][y] |= DIRECTION_LIST[index];
  maze->data[currX][currY] |= DIRECTION_MAP[index];
  setupAlleys_Recursive(currX, currY, index);
}

/*************************************************************/
/*int width:                                                 */
/*  in,                                                      */
/*  width of the maze,                                       */
/*  must be between 3 and 1000.                              */
/*int height:                                                */
/*  in,                                                      */
/*  height of the maze,                                      */
/*  must be between 3 and 1000.                              */
/*int wayPointX:                                             */
/*  in,                                                      */
/*  x-location to start carving for the maze,                */
/*  must be from 1 to the width of the maze.                 */
/*int wayPointY:                                             */
/*  in,                                                      */
/*  x-location to start carving for the maze,                */
/*  must be from 1 to the height of the maze.                */
/*The other inputs are not used for this lab!                */
/*No FALSE if all went well - TRUE if not.                   */
/*This function is what generates the maze itself.           */
/*Calls checkForNonsense to make sure the inputs aren't      */
/*  invalid. If they aren't, it checks to see if the         */
/*  width and height are identical to the last maze,         */
/*  and if so it will zero out the old array and reuse it.   */
/*  Otherwise, allocateMazeData is called. After this,       */
/*  all variables in the maze pointer are set and            */
/*  carveAlleys_Recursive is called. At the end it will      */
/*  begin calling mazeSolve in order to determine if the     */
/*  created maze is actually valid. If not, the current maze */
/*  is dumped and restarted to try and create a valid maze.  */
/*************************************************************/
maze_t *mazeGenerate(int width, int height, // [3, 1000],  [3, 1000]
  int wayPointX, int wayPointY,       // [1, width],   [1, height]
  int wayPointAlleyLength,            // [0,  min(width, height)/2 ]
  double wayPointDirectionPercent,    // [0.0,  1.0]
  double straightProbability,         // [0.0,  1.0]
  int printAlgorithmSteps)            // [TRUE | FALSE]
{
  if (checkForNonsense(width, height, wayPointX, wayPointY,
    wayPointAlleyLength, wayPointDirectionPercent,
    straightProbability, printAlgorithmSteps))
  {
    printf("Error - invalid parameters to mazeGenerate.\n");
    return NULL;
  }
  int i;

  // Can we just reuse the old maze's data (just zero it out)?
  if (maze && maze->width == width && maze->height == height)
  {
    for (i = 0; i < maze->width; i++)
    {
      memset(maze->data[i], 0, height);
    }
  }
  else
  {
    allocateMazeData(width, height);
    memset(randomSets, -1, MAX_RAND_SETS);
    for (i = 0; i < MAX_RAND_SETS / NUM_DIRECTIONS; i++)
    {
      randomizeArray(&randomSets[i * NUM_DIRECTIONS], NUM_DIRECTIONS,
        NUM_DIRECTIONS);
    }
  }

  maze->startX = rand() % width;
  maze->startY = 0;
  //maze->endX = rand() % width;
  //maze->endY = height - 1;
  maze->wayX = wayPointX - 1;
  maze->wayY = wayPointY - 1;
  maze->alleyLen = wayPointAlleyLength;
  maze->dirPercent = wayPointDirectionPercent;
  maze->straightProb = straightProbability;
  maze->printSteps = printAlgorithmSteps;

  mWidth = width - 1;
  mHeight = height - 1;

  numCells = width * height * wayPointDirectionPercent;
  alleyIndex = 0;

  initArrays();
  //createMaze_Recursive(wayPointX - 1, wayPointY - 1);
  for (i = 0; i < 4; i++) setupAlleys_Recursive(maze->wayX, maze->wayY, i);
  //setupAlleys_Recursive(maze->wayX, maze->wayY, 0);
  for (alleyIndex; alleyIndex < NUM_DIRECTIONS; alleyIndex++)
  {
    if (maze->printSteps) mazePrint();
    carveAlleys_Recursive(alleymap_X[rand_alleys[alleyIndex]],
      alleymap_Y[rand_alleys[alleyIndex]],
      rand_alleys[alleyIndex]);
    if (maze->printSteps) mazePrint();
  }

  maze->data[maze->startX][maze->startY] |= NORTH;

  if (bCallSolve)
  {
    mazeSolve();
    sliceBits_0x0F();
  }

  return maze;
}

/*************************************************************/
/*short x:                                                   */
/*  in,                                                      */
/*  current x-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*short y:                                                   */
/*  in,                                                      */
/*  current y-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*Returns TRUE if the end has been found                     */
/*This function calls itself recursively to solve the maze.  */
/*Checks to see if the current cell is the end. If not       */
/*  it loops through all 4 possible directions and sees      */
/*  if there are unvisited cells next to them. If there      */
/*  are it calls itself with the new x and y coordinates.    */
/*  This continues until the whole maze is solved.           */
/*************************************************************/
int mazeSolve_Recursive(short x, short y, char direction)
{
  if (recursionLevel > MAX_STACK) return FALSE;

  recursionLevel++;
  char i;
  maze->data[x][y] |= VISITED; // set this cell to visited

  if ((i = checkForEdge(x, y)) != INVALID && bFoundWay && !bFoundExit)
  {
    bFoundExit = 1;
    maze->endX = x;
    maze->endY = y;
    maze->data[x][y] |= GOAL | DIRECTION_LIST[i];
    recursionLevel--;
    return TRUE;
  }
  else if (x == maze->wayX && y == maze->wayY && !bFoundWay)
  {
    bFoundWay = 1;
    maze->data[x][y] |= GOAL;
    mazeSolve_Recursive(maze->wayX, maze->wayY, -1);
    recursionLevel--;
    return TRUE;
  }

  for (i = 0; i < 4; i++)
  {
    currX = x + DIRECTION_DX[i];
    currY = y + DIRECTION_DY[i];

    if (currX < 0 || currX > maze->width - 1) continue;
    else if (currY < 0 || currY > maze->height - 1) continue;
    else if (maze->data[currX][currY] & VISITED) continue;
    else if (maze->data[x][y] & DIRECTION_LIST[i])
    {
      if (mazeSolve_Recursive(currX, currY, DIRECTION_LIST[i]))
      {
        maze->data[x][y] |= GOAL;
        recursionLevel--;
        return TRUE;
      }
    }
  }

  recursionLevel--;
  return FALSE;
}

/*************************************************************/
/*No inputs.                                                 */
/*No return.                                                 */
/*This function is called when the maze needs to be solved.  */
/*Makes sure there is actually an active maze available. If  */
/*  there is, it calls mazeSolve_Recursive with the starting */
/*  x and y coordinates of the maze.                         */
/*************************************************************/
void mazeSolve()
{
  const char MAX_TRIES = 10;
  char numTries = 0;
  bCallSolve = 0;
  if (maze)
  {
    bFoundExit = 0;

    while (!bFoundExit && numTries < MAX_TRIES)
    {
      bFoundWay = 0;
      bFoundExit = 0;
      mazeSolve_Recursive(maze->startX, maze->startY, -1);
      mazeSolve_Recursive(maze->wayX, maze->wayY, -1);

      if (!bFoundExit)
      {
        mazeGenerate(mWidth + 1, mHeight + 1, maze->wayX + 1, maze->wayY + 1,
          maze->alleyLen, maze->dirPercent, maze->straightProb,
          maze->printSteps);
      }
      numTries++;
    }

    if (!bFoundExit) printf("Joel's Algorithm failed\n");
  }
  bCallSolve = 1;
}

#ifdef MAZEIII
/*************************************************************/
/*int mazeX:                                                 */
/*  in,                                                      */
/*  current x-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*int mazeY:                                                 */
/*  in,                                                      */
/*  current y-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*No return.                                                 */
/*This function determines what walls it needs to draw       */
/*  for the current cell and writes them to an array.        */
/*Loops through, checking each cell against DIRECTION_LIST[i]*/
/*  and, if the cell contains that wall, calls the current   */
/*  function at index i from D_FUNCS (which are mapped       */
/*  to DIRECTION_LIST). Each function inside of D_FUNCS      */
/*  corresponds to a specific drawing function capable       */
/*  of drawing the desired walls. At the end, fixWalls is    */
/*  called to eliminate extra walls so that a path is        */
/*  established.                                             */
/*************************************************************/
void writePixelBlock(int mazeX, int mazeY)
{
  int i;

  for (i = 0; i < 4; i++)
  {
    if (maze->data[mazeX][mazeY] & DIRECTION_LIST[i])
    {
      D_FUNCS[i](&mazeImg->image[0], mazeImg->rowSize*(mWidth + 1),
        mazeImg->pixelHeight*(mHeight + 1), mazeX * 8, mazeY * 8);
    }
  }

  fixWalls(&mazeImg->image[0], mazeImg->rowSize*(mWidth + 1),
    mazeImg->pixelHeight*(mHeight + 1), mazeX * 8, mazeY * 8);
}

/*************************************************************/
/*int mazeX:                                                 */
/*  in,                                                      */
/*  current x-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*int mazeY:                                                 */
/*  in,                                                      */
/*  current y-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*unsigned char r:                                           */
/*  in,                                                      */
/*  red value for the current cell,                          */
/*  should be [0, 255].                                      */
/*unsigned char g:                                           */
/*  in,                                                      */
/*  green value for the current cell,                        */
/*  should be [0, 255].                                      */
/*unsigned char b:                                           */
/*  in,                                                      */
/*  blue value for the current cell,                         */
/*  should be [0, 255].                                      */
/*No return.                                                 */
/*This function colors inside the walls of the desired       */
/*  cell using the desired color.                            */
/*This uses a makeshift method of coloring where, when       */
/*  a wall of r/g/b values 0/0/0 is found, bShouldColor is   */
/*  inverted. When bShouldColor is 1, it colors. Otherwise   */
/*  it won't color. This produces decent colored cells but   */
/*  could definitely be better.                              */
/*************************************************************/
void setBlockColor(int mazeX, int mazeY,
  unsigned char r, unsigned char g, unsigned char b)
{
  int x, y, k, cx, cy;
  char bShouldColor = 0;
  unsigned char rd, gr, bl;

  for (k = 0; k < 2; k++)
  {
    for (y = 0; y < 8; y++)
    {
      for (x = 0; x < 8; x++)
      {
        cx = !k ? x : y;
        cy = !k ? y : x;

        getRGB(&mazeImg->image[0], cx + (mazeX * 8), cy + (mazeY * 8),
          mazeImg->rowSize*(mWidth + 1),
          mazeImg->pixelHeight*(mHeight + 1),
          &rd, &gr, &bl);
        if (!rd && !gr && !bl) // pixel is not just white
        {
          bShouldColor = !bShouldColor;
        }
        else if (bShouldColor)
        {
          setRGB(&mazeImg->image[0], cx + (mazeX * 8), cy + (mazeY * 8),
            mazeImg->rowSize*(mWidth + 1),
            mazeImg->pixelHeight*(mHeight + 1),
            r, g, b);
        }
      }
    }
  }
}
#endif

/*************************************************************/
/*int x:                                                     */
/*  in,                                                      */
/*  current x-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*int y:                                                     */
/*  in,                                                      */
/*  current y-value within the maze,                         */
/*  must not be out of bounds of the maze.                   */
/*Returns TRUE if the current cell is part of an alley,      */
/*  FALSE if not.                                            */
/*This function checks to see if the current cell is         */
/*  part of an alley.                                        */
/*Uses simple if statements to enforce constraints that,     */
/*  if true, indicate that the current cell is part of an    */
/*  alley. If none of the constraints result in an entirely  */
/*  true statement, the current cell is not part of an alley.*/
/*************************************************************/
int isAlley(int x, int y)
{
  if (x == maze->wayX && y >= alleymap_Y[0] && y <= alleymap_Y[2])
  {
    return TRUE;
  }
  else if (y == maze->wayY && x >= alleymap_X[3] && x <= alleymap_X[1])
  {
    return TRUE;
  }

  return FALSE;
}

/*************************************************************/
/*No inputs.                                                 */
/*No return.                                                 */
/*This function prints out the current color-coded maze.     */
/*Makes sure there is actually an active maze available. If  */
/*  there is, it loops through each cell, printing its data. */
/*  If it is either the waypointX and Y or if it is          */
/*  part of the goal, it will color them differently than    */
/*  the rest of the maze.                                    */
/*************************************************************/
void mazePrint()
{
  if (maze)
  {
    int i, k;

#ifdef MAZEIII
    memset(&mazeImg->image[0], 0xFF, sizeof(unsigned char) *
      mazeImg->pixelDataSize);

    copyIntToAddress(mazeImg->imgFileSize, &header[2]);
    copyIntToAddress(mazeImg->pixelWidth*(mWidth + 1), &header[18]);
    copyIntToAddress(mazeImg->pixelHeight*(mHeight + 1), &header[22]);
    copyIntToAddress(mazeImg->pixelDataSize, &header[34]);
#endif

    printf("\n\n");
    printf("========================\n");
    printf("Maze(%d x %d): (%d, %d)\n", maze->width, maze->height,
      maze->wayX + 1, maze->wayY + 1);
    printf("========================\n");

    for (i = 0; i < maze->height; i++)
    {
      for (k = 0; k < maze->width; k++)
      {
#ifdef MAZEIII
        writePixelBlock(k, i);
#endif
        if (maze->data[k][i] & GOAL)
        {
          textcolor(32);
#ifdef MAZEIII
          setBlockColor(k, i, 0, 255, 0);
#endif
        }
        else if (isAlley(k, i))
        {
          textcolor(31);
#ifdef MAZEIII
          setBlockColor(k, i, 255, 0, 0);
#endif
        }
        printf("%c", pipeList[maze->data[k][i] & BITSLICE_0x0F]);
        textcolor(37);
      }
      printf("\n");
    }

#ifdef MAZEIII
    FILE* f = fopen("maze.bmp", "wb");
    fwrite(header, 1, sizeof(header), f);
    fwrite(&mazeImg->image[0], 1,
      sizeof(unsigned char) * mazeImg->pixelDataSize, f);
    fclose(f);
#endif
  }
  printf("\n");
}

/*************************************************************/
/*No inputs.                                                 */
/*No return.                                                 */
/*This function is called when the maze needs to be freed.   */
/*Makes sure there is actually an active maze available. If  */
/*  there is, it loops through each column and frees the     */
/*  data. At the end, it frees the pointers to the old       */
/*  columns.                                                 */
/*************************************************************/
void mazeFree()
{
  if (maze)
  {
    int i;
    for (i = 0; i < maze->width; i++)
    {
      free(maze->data[i]);
    }
    free(maze->data);
    free(maze);
    maze = NULL;
#ifdef MAZEIII
    free(mazeImg->image);
    free(mazeImg);
    mazeImg = NULL;
#endif
  }
}

//===========================================================================
//Prints escape characters to change terminal foreground color.
void textcolor(int color)
{
  //30	Black
  //31	Red
  //32	Green
  //33	Yellow
  //34	Blue
  //35	Magenta
  //36	Cyan
  //37	White  
  printf("%c[%d;%d;%dm", 0x1B, 0, color, 40);
}