#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <endian.h>
#include "dungeonGen.h"

int main(int argc, char *argv[])
{
  //creating path dir
  char * path;
  char * home = getenv("HOME");
  char * dungeonPath = "/.rlg327/dungeons";
  path = malloc((sizeof(home) + sizeof(dungeonPath)+1) * sizeof(char));
  sprintf(path, "%s%s", home, dungeonPath);
  //reading the arguments
  int save = 0;
  int load = 0;
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[1], "--save") == 0) {
      save = 1;
    } else if (strcmp(argv[1], "--load") == 0) {
      load = 1;
    } else {
      printf("There are no arguments");
      return 0;
    }
  }
  readFile(path);


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

int save(FILE * file, char* path){
  fopen(path, "wb+");
  if(file == NULL) {
		fprintf(stderr, "FILE ERROR: Cannot write dungeon at %s\n", path);
        exit(1);
	}
}

void readFile(char * path){
  FILE * f;
  f = fopen(path, "rb+");
  if(f = NULL){
    printf("Could not open FILE %s", path);
    exit(1);
  }
  fread(bin.file_type, 12, 1, f);
  fread(&bin.version, 4, 1, f);
  bin.version = htobe32(bin.version);
  fread(&bin.size, 4, 1, f);
  bin.size = htobe32(bin.size);
  fread(&bin.xPC, 1, 1, f);
  bin.xPC = htobe8(bin.xPC);
  fread(&bin.yPC, 1, 1, f);
  bin.yPC = htobe8(bin.yPC);
  //reads the hardness from the file
  //char * tempHardness = malloc((sizeof(bin.hardness) * sizeof(char)));
  //fread(tempHardness, 1680, 1, f);
  for (int i = 0; i < 21; i++)
  {
    for (int j = 0; j < 80; j++)
    {
      fread(&bin.hardness[i][j], 1, 1, f);
      bin.hardness[i][j] = htobe8(bin.hardness[i][j]);
    }
  }
  fread(&bin.r, 2 ,1 , f);
  bin.r = htobe16(bin.r);
  bin.rPos = malloc(bin.r * 4 * sizeof(uint8_t));
  //reading room positions NEEDS TO BE CHECKED
  for(int j = 0; j < bin.r; j++){
    for(int i = 0; i < 4; i++){
      fread(bin.rPos+i*j+4, 1, 1, f);
      *(bin.rPos + i * (j+4)) = htobe8(*(bin.rPos + i * (j+4)));
    }
  }
  fread(&bin.numUpStairs, 1, 1, f);
  bin.numUpStairs = htobe16(bin.numUpStairs);
  uint8_t tempUpStairs[bin.numUpStairs];
  bin.xUpStairs = tempUpStairs;
  for(int i = 0; i < bin.numUpStairs; i++){
    fread(&bin.xUpStairs[i], 1, 1, f);
    bin.xUpStairs = htobe8(bin.xUpStairs);
    fread(&bin.yUpStairs[i], 1, 1, f);
    bin.yUpStairs = htobe8(bin.yUpStairs);
  }
  fread(&bin.numDownStairs, 1, 1, f);
  bin.numDownStairs = htobe16(bin.numDownStairs);
  uint8_t tempDownStairs[bin.numDownStairs];
  bin.xDownStairs = tempDownStairs;
  for(int i = 0; i < bin.numDownStairs; i++){
    fread(&bin.xDownStairs[i], 1, 1, f);
    bin.xDownStairs = htobe8(bin.xDownStairs);
    fread(&bin.yDownStairs[i], 1, 1, f);
    bin.yDownStairs = htobe8(bin.yDownStairs);
  }
  fclose(f);

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
  rooms[6];
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
