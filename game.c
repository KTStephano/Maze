#include "command.h"
#include "utility.h"
#include "input.h"
#include "mazegen.h"
#include "filesystem.h"
#include <time.h>
#ifdef linux
#include <unistd.h>
#endif

#define MAX_MENU_SELECTIONS 3
#define MAX_VIEW_DIST 2 // used for CHALLENGE_MODE
#define CHALLENGE_HEIGHT 26

typedef struct
{
  int currX, currY;
  int newX, newY; // kept in step with currX/Y
} player_t;

typedef struct
{
  char w_down, a_down;
  char s_down, d_down;
  char c_down;
} keyStates_t;

static boolean_t bShouldClose = _FALSE;
static boolean_t bNeedsUpdate = _FALSE;
static boolean_t bMenuIsActive = _TRUE; // starts off true
static boolean_t bEnterPressed = _FALSE;
static boolean_t bIsChallenge = _FALSE;
static char currSelection = 0;
static maze_t *maze = NULL;
static player_t player;
static keyStates_t keys;

void saveMaze();
void loadMaze();
void openConsole();
void closeConsole();
char *readInput();
void printMenu();
void endProgram();
void k_EnterDown();

// _TRUE if there is a pathing error
boolean_t checkForPathingError(int x, int y, char direction)
{
  if (!checkBounds(x, y))
  {
    //printf("Out of bounds!\n");
    return _TRUE;
  }
  else if (!(maze->data[player.currX][player.currY] &
             DIRECTION_LIST[direction]))
  {
    //printf("Path blocked!\n");
    return _TRUE;
  }

  return _FALSE;
}

void updateMenuSelection(char offset)
{
  currSelection += offset;
  if (currSelection < 0) currSelection = 0;
  else if (currSelection > MAX_MENU_SELECTIONS - 1)
  {
    currSelection = MAX_MENU_SELECTIONS - 1;
  }
  if (currSelection > MAX_MENU_SELECTIONS - 2 && bEnterPressed)
  {
    currSelection--; // this applies to New Game which only has 2 options
  }
  printMenu();
}

void k_WDown()
{
  if (!keys.w_down)
  {
    keys.w_down = TRUE;
    if (!bMenuIsActive)
    {
      int cx, cy;
      cx = player.currX;
      cy = player.currY - 1; // moves up

      if (checkForPathingError(cx, cy, 0)) return;
      player.newX = cx;
      player.newY = cy;
      //printf("NEW LOCATION: %d %d\n", cx, cy);
    }
    // -1 instead of 1 so it appears to move up
    else updateMenuSelection(-1);
  }
}

void k_WUp()
{
  keys.w_down = FALSE;
}

void k_ADown()
{
  if (!keys.a_down)
  {
    keys.a_down = TRUE;
    if (!bMenuIsActive)
    {
      int cx, cy;
      cx = player.currX - 1; // move left
      cy = player.currY;

      if (checkForPathingError(cx, cy, 3)) return;
      player.newX = cx;
      player.newY = cy;
      //printf("NEW LOCATION: %d %d\n", cx, cy);
    }
    else
    {
      bEnterPressed = _FALSE; // this will make it move back
      printMenu(); // refresh menu
    }
  }
}

void k_AUp()
{
  keys.a_down = FALSE;
}

void k_SDown()
{
  if (!keys.s_down)
  {
    keys.s_down = TRUE;
    if (!bMenuIsActive)
    {
      int cx, cy;
      cx = player.currX;
      cy = player.currY + 1; // moves down

      if (checkForPathingError(cx, cy, 2)) return;
      player.newX = cx;
      player.newY = cy;
      //printf("NEW LOCATION: %d %d\n", cx, cy);
    }
    // 1 instead of -1 so it appears to move down
    else updateMenuSelection(1);
  }
}

void k_SUp()
{
  keys.s_down = FALSE;
}

void k_DDown()
{
  if (!keys.d_down)
  {
    keys.d_down = TRUE;
    if (!bMenuIsActive)
    {
      int cx, cy;
      cx = player.currX + 1; // moves right
      cy = player.currY;

      if (checkForPathingError(cx, cy, 1)) return;
      player.newX = cx;
      player.newY = cy;
      //printf("NEW LOCATION: %d %d\n", cx, cy);
    }
    else k_EnterDown(); // enter selection
  }
}

void k_DUp()
{
  keys.d_down = FALSE;
}

void k_EnterDown()
{
  if (bEnterPressed && bMenuIsActive) // already pressed
  {
    switch (currSelection)
    {
    case 0:
      mazeFree();
      bIsChallenge = _FALSE;
      maze = mazeGenerate(25, 25, 12, 12, 4, 0.2, 0.5, FALSE);
      break;
    case 1: // serves as challenge mode
      mazeFree();
      bIsChallenge = _TRUE;
      // the height of CHALLENGE_HEIGHT is important - it lets the load
      // function determine if the game was in Challenge mode or not
      maze = mazeGenerate(25, CHALLENGE_HEIGHT, 12, 12, 4, 0.2, 0.5, FALSE);
      break;
    }
    player.newX = maze->startX;
    player.newY = maze->startY;

    bMenuIsActive = _FALSE;
    bEnterPressed = _FALSE;
  }
  else if (bMenuIsActive)
  {
    bEnterPressed = _TRUE;
    switch (currSelection)
    {
    case 0:
      printMenu(); // refresh the menu
      break;
    case 1: // skip 0 since it's New Game - handled above
      printf("Enter a file to load\n");
      char *str = readInput();
      char *temp = malloc(strlen("load ") + strlen(str) + 1);
      memcpy(temp, "load ", strlen("load "));
      strcpy(temp + strlen("load "), str);
      cmd_AddToBuffer(temp);
      free(str);
      free(temp);
      break;
    case 2:
      endProgram();
      break;
    }
  }
}

void endProgram()
{
  bShouldClose = _TRUE;
}

void gameInit()
{
  cmd_AddCommand("exit", endProgram);
  cmd_AddCommand("wdown", k_WDown);
  cmd_AddCommand("wup", k_WUp);
  cmd_AddCommand("adown", k_ADown);
  cmd_AddCommand("aup", k_AUp);
  cmd_AddCommand("sdown", k_SDown);
  cmd_AddCommand("sup", k_SUp);
  cmd_AddCommand("ddown", k_DDown);
  cmd_AddCommand("dup", k_DUp);
  cmd_AddCommand("save", saveMaze);
  cmd_AddCommand("load", loadMaze);
  cmd_AddCommand("cdown", openConsole);
  cmd_AddCommand("cup", closeConsole);
  cmd_AddCommand("enter", k_EnterDown);
  cmd_AddCommand("mazeSolve", mazeSolve);
  cmd_AddCommand("mazePrint", mazePrint);

  i_BindKey('e', "exit", "");
  i_BindKey('w', "wdown", "wup");
  i_BindKey('a', "adown", "aup");
  i_BindKey('s', "sdown", "sup");
  i_BindKey('d', "ddown", "dup");
  i_BindKey('c', "cdown", "cup");

  player.currX = 0;
  player.currY = 0;
  player.newX = -1;
  player.newY = -1;

  keys.w_down = FALSE;
  keys.a_down = FALSE;
  keys.s_down = FALSE;
  keys.d_down = FALSE;
  keys.c_down = FALSE;
}

void checkForUpdate()
{
  if (player.currX != player.newX)
  {
    player.currX = player.newX;
    bNeedsUpdate = _TRUE;
  }
  if (player.currY != player.newY)
  {
    player.currY = player.newY;
    bNeedsUpdate = _TRUE;
  }
  if (player.currX == maze->endX &&
      player.currY == maze->endY)
  {
    printf("You solved the maze!\n");
    endProgram();
    bNeedsUpdate = _TRUE;
  }
}

void saveMaze()
{
  if (cmd_GetNumArgs() != 1)
  {
    printf("Use format save maze.ext\n");
    return;
  }

  char *arg = cmd_GetArg(0);
  fileHandle_t file = fs_Open(arg, "w");
  array_t arr;
  initArray(&arr);
  if (file == -1)
  {
    printf("ERROR - could not open %s\n", arg);
    return;
  }
  FILE *f = fopen(fs_GetPath(file), "w");
  if (!f)
  {
    printf("ERROR - could not open %s\n", arg);
    return;
  }

  // capture the player's location first
  fprintf(f, "%c ", 'p');
  fprintf(f, "%d ", player.currX);
  fprintf(f, "%d", player.currY);
  fprintf(f, "\n");

  // store the width and height of the maze
  fprintf(f, "%c ", 'w');
  fprintf(f, "%d ", maze->width);
  fprintf(f, "%d", maze->height);
  fprintf(f, "\n");

  // store startX and startY
  fprintf(f, "%c ", 's');
  fprintf(f, "%d ", maze->startX);
  fprintf(f, "%d", maze->startY);
  fprintf(f, "\n");

  // store endX and endY
  fprintf(f, "%c ", 'e');
  fprintf(f, "%d ", maze->endX);
  fprintf(f, "%d", maze->endY);
  fprintf(f, "\n");

  int x, y;
  // now store the whole maze
  for (y = 0; y < maze->height; y++)
  {
    for (x = 0; x < maze->width; x++)
    {
      fprintf(f, "%c ", 'm');
      fprintf(f, "%d ", x); // x-coord
      fprintf(f, "%d ", y); // y-coord
      fprintf(f, "%d", maze->data[x][y] & BITSLICE_0x0F);
      fprintf(f, "\n");
    }
  }

  fs_WriteFile(file, &arr);
  fs_Close(file);
  fclose(f);
  clearData(&arr);
}

void loadMaze()
{
  if (cmd_GetNumArgs() != 1)
  {
    printf("Use format save maze.ext\n");
    return;
  }

  array_t buffer;
  initArray(&buffer);
  char *arg = cmd_GetArg(0);
  fileHandle_t file = fs_Open(arg, "r");
  if (file == INVALID)
  {
    printf("ERROR - could not open %s\n", arg);
    return;
  }
  fs_ReadFile(file, &buffer);
  mazeFree(); // prevent leaks
  bIsChallenge = _FALSE; // set this to _FALSE initially - may change

  int i;
  for (i = 0; i < buffer.size; i++)
  {
    int curr = geti(&buffer, i);
    int elem;
    if (curr == 'p' || curr == 's' || curr == 'e') // player data
    {
      i += 2; // start of the first element
      array_t temp;
      initArray(&temp);
      while (1)
      {
        // if space, set newX
        elem = geti(&buffer, i);
        if (elem == ' ')
        {
          char *str = arrayToChar(&temp);
          if (curr == 'p') player.newX = atoi(str);
          else if (curr == 's') maze->startX = atoi(str);
          else if (curr == 'e') maze->endX = atoi(str);
          free(str);
          clearData(&temp);
        }
        // if \n, set newY and break;
        else if (elem == '\n')
        {
          char *str = arrayToChar(&temp);
          if (curr == 'p') player.newY = atoi(str);
          else if (curr == 's') maze->startY = atoi(str);
          else if (curr == 'e') maze->endY = atoi(str);
          free(str);
          clearData(&temp);
          break;
        }
        appendi(&temp, geti(&buffer, i));
        i++;
      }
    }
    else if (curr == 'w') // width/height of maze
    {
      i += 2; // start of the first element
      array_t temp;
      initArray(&temp);
      int width, height;

      while (1)
      {
        elem = geti(&buffer, i);
        // if space, set newX
        if (elem == ' ')
        {
          char *str = arrayToChar(&temp);
          width = atoi(str);
          free(str);
          clearData(&temp);
        }
        // if \n, set newY and break;
        else if (elem == '\n')
        {
          char *str = arrayToChar(&temp);
          height = atoi(str);
          // check for challenge mode
          if (height == CHALLENGE_HEIGHT) bIsChallenge = _TRUE;
          free(str);
          clearData(&temp);
          break;
        }
        else appendi(&temp, geti(&buffer, i));
        i++;
      }
      maze = allocateMazeData(width, height);
    }
    // read in x/y/data for the maze (one point at a time)
    else if (curr == 'm') // width/height of maze
    {
      i += 2; // start of the first element
      array_t temp;
      initArray(&temp);
      int x, y, count = 0;

      while (1)
      {
        elem = geti(&buffer, i);
        appendi(&temp, geti(&buffer, i));
        // if space, set newX
        if (elem == ' ')
        {
          char *str = arrayToChar(&temp);
          if (!count) x = atoi(str);
          else
          {
            y = atoi(str);
            count = 0; // reset this
          }
          free(str);
          clearData(&temp);
          count++;
        }
        // if \n, set newY and break;
        else if (elem == '\n')
        {
          char *str = arrayToChar(&temp);
          char c = atoi(str); // should be safe by now
          maze->data[x][y] = c & BITSLICE_0x0F;
          free(str);
          clearData(&temp);
          break;
        }
        i++;
      }
    }
  }
  fs_Close(file);
  clearData(&buffer);
  bMenuIsActive = _FALSE;
}

void printMaze()
{
  int x, y;

  for (y = 0; y < maze->height; y++)
  {
    for (x = 0; x < maze->width; x++)
    {
#ifdef linux
      if (x == player.currX && y == player.currY) textcolor(32);
      else if (x == maze->endX && y == maze->endY) textcolor(31);
#endif
      // if it is challenge mode and the location is
      // further than MAX_VIEW_DIST, draw it as a space
      if (bIsChallenge && (x < player.currX - MAX_VIEW_DIST ||
        x > player.currX + MAX_VIEW_DIST ||
        y < player.currY - MAX_VIEW_DIST ||
        y > player.currY + MAX_VIEW_DIST))
      {
        printf(" ");
      }
      else printf("%c", pipeList[maze->data[x][y] & BITSLICE_0x0F]);
#ifdef linux
      textcolor(37);
#endif
    }
    printf("\n");
  }
  printf("\n");
}

void openConsole()
{
  if (!keys.c_down)
  {
    keys.c_down = TRUE;

    printf("Enter an executable command\n");
    char *str = readInput();
    cmd_AddToBuffer(str);
    free(str);
  }
}

void closeConsole()
{
  keys.c_down = FALSE;
}

char *readInput()
{
  array_t input;
  initArray(&input);
  int c = 0;
  char *str;

  while (1)
  {
    c = getc(stdin);
    if (c == '\n') break;
    appendi(&input, c);
  }
  str = arrayToChar(&input);
  clearData(&input);
  return str;
}

void printMenu()
{
  int i, k;
  char *items[] = { "New Game", "Load Game", "Quit" };
  char *newGame[] = { "Normal", "Challenge", "" };
  for (i = 0; i < 100; i++) printf("\n");
  printf("Use W and S to move up/down, A to move back and D to select\n");
  for (i = 0; i < MAX_MENU_SELECTIONS; i++)
  {
    if (i == currSelection)
    {
      // color current selection
      textcolor(32);
      // only two elements in newGame so break if necessary
      printf("%s", !bEnterPressed ? items[i] : newGame[i]);
      textcolor(37);
    }
    else printf("%s", !bEnterPressed ? items[i] : newGame[i]);
    printf("\n");
  }
}

int main(int argc, char** argv)
{
  int i = 0;
  char cwd[MAX_PATH];
  srand((unsigned)time(NULL)); // seed the RNG

  //maze = mazeGenerate(25, 25, 12, 12, 4, 0.2, 0.5, FALSE);
  //if (!maze) return -1;

  cmd_Init();
  i_Init();
  fs_Init();
  gameInit();
  //checkForUpdate();
#ifdef _WIN32
  fs_SetCWD(argv[0]);
#endif
#ifdef linux
  getcwd(&cwd[0], MAX_PATH);
  fs_SetCWD(&cwd[0]);
#endif
  fs_ProfileCWD();
  printMenu();

  while (!bShouldClose)
  {
    i_ProcessKeyInput();
    cmd_ExecuteCommands();
    if (!bMenuIsActive) checkForUpdate();

    if (bNeedsUpdate && !bMenuIsActive)
    {
      for (i = 0; i < 100; i++) printf("\n");
      printMaze();
      bNeedsUpdate = _FALSE;
    }
  }

  cmd_Shutdown();
  i_Shutdown();
  fs_Shutdown();
  mazeFree();

  return 0;
}