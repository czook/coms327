#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "dungeonGen.h"

int main(int argc, char *argv[])
{
  hardnessGen();
  genBorder();
  roomGen();
  for (int i = 0; i < 7; i++)
  {
    hallways(i);
  }
  placeStaircase();
  printboard();
}
//generates border and initializes matChar with spaces
void genBorder()
{
  for (int i = 0; i < 21; i++)
  {
    for (int j = 0; j < 80; j++)
    {
      if (i == 0)
      {
        playArea[i][j].matChar = '-';
      }
      if (i != 0 && i != 20 && (j == 0 || j == 79))
      {
        playArea[i][j].matChar = '|';
      }
      if (i == 20)
      {
        playArea[i][j].matChar = '-';
      }
      if (i > 0 && i < 20 && j > 0 && j < 79)
      {
        playArea[i][j].matChar = ' ';
      }
    }
  }
}
//Generates the hardness of the matChars
void hardnessGen()
{
  srand(time(NULL));

  //Random hardness for all rock in playArea
  int i, j;
  for (i = 0; i < 21; i++)
  {
    for (j = 0; j < 80; j++)
    {
      struct Grid matgrid;
      matgrid.matChar = ' ';
      playArea[i][j] = matgrid;
      if (i == 0 || i == 21 || j == 0 || j == 80)
        playArea[i][j].hardness = 255;
      else
        playArea[i][j].hardness = rand() % 254 + 1;
    }
  }
}
//prints all charcters on matChar array
void printboard()
{
  for (int i = 0; i < 21; i++)
  {
    for (int j = 0; j < 80; j++)
    {
      printf("%c", playArea[i][j].matChar);
    }
    printf("\n");
  }
}
//generates rooms
void roomGen()
{

  srand(time(NULL));
  for (int i = 0; i <= 6; i++)
  { //iterates through rooms in rooms array and initializes attributes
    rooms[i].width = rand() % (6 - 4 + 1) + 4;
    ; //formula is rand() % (upperBound - lowerBound + 1) + lowerBound
    rooms[i].height = rand() % (5 - 3 + 1) + 3;
    ;
    rooms[i].x = rand() % (70) + 1;
    ;
    rooms[i].y = rand() % (11) + 1;
    ;
    rooms[i].xEnd = rooms[i].x + rooms[i].width;
    rooms[i].yEnd = rooms[i].y + rooms[i].height;
    if (overlapChecker(i) == 0)
    {
      i--;
      continue;
    }

    for (int j = rooms[i].y; j < rooms[i].yEnd; j++)
    { //places periods on matChar array for room locations
      for (int k = rooms[i].x; k < rooms[i].xEnd; k++)
      {
        playArea[j][k].matChar = '.';
        countTiles++;
      }
    }
  }
}

int overlapChecker(int roomIndex)
{
  for (int x = rooms[roomIndex].x - 1; x < rooms[roomIndex].x + rooms[roomIndex].width + 1; x++)
  {
    for (int y = rooms[roomIndex].y - 1; y < rooms[roomIndex].y + rooms[roomIndex].height + 1; y++)
    {
      if (playArea[y][x].matChar == '.')
      {
        return 0;
      }
    }
  }
  return 1;
}
void hallways(int num)
{
  int i;
  double slope = 1000.0;
  struct Room next;
  next.x = rooms[num].x;
  next.y = rooms[num].y;
  for (i = 0; i < num; i++)
  {
    int dx = abs(rooms[num].x - rooms[i].x);
    int dy = abs(rooms[num].x - rooms[i].y);
    if (sqrt(dx * dx + dy * dy) < slope)
    {
      slope = sqrt(dx * dx + dy * dy);
      next.x = rooms[i].x;
      next.y = rooms[i].y;
    }
  }

  while (rooms[num].x != next.x || rooms[num].y != next.y)
  {

    //Right
    if (rooms[num].x < next.x)
      insertRight(&rooms[num].x, &rooms[num].y);
    //Left
    else if (rooms[num].x > next.x)
      insertLeft(&rooms[num].x, &rooms[num].y);
    //Down
    else if (rooms[num].y < next.y)
      insertDown(&rooms[num].x, &rooms[num].y);
    //Up
    else if (rooms[num].y > next.y)
      insertUp(&rooms[num].x, &rooms[num].y);
    //Down and Right
    else if (rooms[num].x < next.x && rooms[num].y < next.y)
    {
      if (playArea[rooms[num].y][rooms[num].x + 1].hardness < playArea[rooms[num].y + 1][rooms[num].x].hardness)
        insertRight(&rooms[num].x, &rooms[num].y);
      else
        insertDown(&rooms[num].x, &rooms[num].y);
    }
    //Down and left
    else if (rooms[num].x > next.x && rooms[num].y < next.y)
    {
      if (playArea[rooms[num].y][rooms[num].x - 1].hardness < playArea[rooms[num].y + 1][rooms[num].x].hardness)
        insertLeft(&rooms[num].x, &rooms[num].y);
      else
        insertDown(&rooms[num].x, &rooms[num].y);
    }
    //Up and left
    else if (rooms[num].x < next.x && rooms[num].y > next.y)
    {
      if (playArea[rooms[num].y][rooms[num].x + 1].hardness < playArea[rooms[num].y - 1][rooms[num].x].hardness)
        insertRight(&rooms[num].x, &rooms[num].y);
      else
        insertUp(&rooms[num].x, &rooms[num].y);
    }
    //Right and Up
    else if (rooms[num].x > next.x && rooms[num].y > next.y)
    {
      if (playArea[rooms[num].y][rooms[num].x - 1].hardness < playArea[rooms[num].y - 1][rooms[num].x].hardness)
        insertLeft(&rooms[num].x, &rooms[num].y);
      else
        insertUp(&rooms[num].x, &rooms[num].y);
    }
  }
}

void insertLeft(int *x, int *y)
{
  *x = *x - 1;
  if (playArea[*y][*x].matChar != '.')
  {
    playArea[*y][*x].matChar = '#';
    playArea[*y][*x].hardness = 0;
  }
}

void insertRight(int *x, int *y)
{
  *x = *x + 1;
  if (playArea[*y][*x].matChar != '.')
  {
    playArea[*y][*x].matChar = '#';
    playArea[*y][*x].hardness = 0;
  }
}

void insertUp(int *x, int *y)
{
  *y = *y - 1;
  if (playArea[*y][*x].matChar != '.')
  {
    playArea[*y][*x].matChar = '#';
    playArea[*y][*x].hardness = 0;
  }
}

void insertDown(int *x, int *y)
{
  *y = *y + 1;
  if (playArea[*y][*x].matChar != '.')
  {
    playArea[*y][*x].matChar = '#';
    playArea[*y][*x].hardness = 0;
  }
}

void placeStaircase()
{
  srand(time(NULL));
  int count = 0;
  int upCount = rand() % countTiles + 1;
  int downCount = rand() % countTiles + 1;
  while (upCount == downCount)
  {
    int downCount = rand() % countTiles + 1;
  }
  for (int i = 0; i < 21; i++)
  {
    for (int j = 0; j < 80; j++)
    {
      if (playArea[i][j].matChar == '.')
      {
        count++;
        if (count == upCount)
        {
          playArea[i][j].matChar = '<';
        }
        else if (count == downCount)
        {
          playArea[i][j].matChar = '>';
        }
      }
    }
  }
}
