#ifndef MAZEGEN_H
#define MAZEGEN_H

#define BITSLICE_0x0F 0x0F

#define TRUE 1
#define FALSE 0

//=======================================================================
//The codes below are not required to be used.
//They are included here only to be helpful.
#define NORTH 1             //0000 0001
#define EAST 2              //0000 0010
#define SOUTH 4             //0000 0100
#define WEST 8              //0000 1000
#define TOTAL_DIRECTIONS 4
#define ALL_DIRECTIONS 0x0F //0000 1111
#define NO_DIRECTIONS 0     //0000 0000
#define GOAL 16             //0001 0000 (useful when solving maze)
#define VISITED 32          //0010 0000 (useful when solving maze)
#define SPECIAL 64          //0100 0000

#define TEXTCOLOR_BLACK   30
#define TEXTCOLOR_RED     31
#define TEXTCOLOR_GREEN   32
#define TEXTCOLOR_YELLOW  33
#define TEXTCOLOR_BLUE    34
#define TEXTCOLOR_MAGENTA 35
#define TEXTCOLOR_CYAN    36
#define TEXTCOLOR_WHITE   37

typedef unsigned char uint8;

typedef struct
{
  uint8 **data;
  short width, height;
  short startX, startY;
  short endX, endY;
  short wayX, wayY;
  short alleyLen;
  double dirPercent;
  double straightProb;
  char printSteps;
} maze_t;

extern void textcolor(int color);

//=======================================================================


extern const int DIRECTION_LIST[];
extern const int DIRECTION_DX[];
extern const int DIRECTION_DY[];

extern const unsigned char pipeList[];


//=======================================================================
//Returns TRUE if one or more parameters are out of range. 
//  Otherwise, returns FALSE. 
maze_t *mazeGenerate(int width, int height, // [3, 1000],  [3, 1000]
  int wayPointX, int wayPointY,       // [1, width],   [1, height]
  int wayPointAlleyLength,            // [0,  min(width, height)/2 ]
  double wayPointDirectionPercent,    // [0.0,  1.0]
  double straightProbability,         // [0.0,  1.0]
  int printAlgorithmSteps);           // [TRUE | FALSE]
//=======================================================================

int checkBounds(short x, short y);

maze_t *allocateMazeData(int width, int height);

//void printStats();

void mazeSolve();

void mazePrint();

void mazeFree();
#endif
