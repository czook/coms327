#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <endian.h>
#include <sys/stat.h>
#include "dungeonGen.h"

int main(int argc, char *argv[])
{
  //creating path dir
  char * path;
  //For Windows
  char * home = getenv("HOMEPATH");
  char * dungeonPath = "\\.rlg327\\";
  char * dungeon = "dungeon";
  
  
  //reading the arguments
  int save = 0;
  int load = 0;
  int both = 0;
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--save") == 0) {
      save = 1;
    } else if (strcmp(argv[i], "--load") == 0) {
      load = 1;
    }
    if ((save == 1 || load == 1) && (strcmp(argv[i-1], "--load") == 0 || strcmp(argv[i-1], "--save") == 0)){
      if(strcmp(argv[i], "--load")== 0 || strcmp(argv[i], "--save") == 0){
        if (argv[i+1] == NULL){
          dungeon = malloc((sizeof(argv[i+1])) * sizeof(char));
          dungeon = argv[i+1];
        }
        dungeon = dungeon;
      } else{
        dungeon = malloc((sizeof(argv[i])+1) * sizeof(char));
        dungeon = argv[i];
      }
      
    }
  }
  //genBorder();
  path = malloc((sizeof(home) + sizeof(dungeonPath) * sizeof(dungeon)+1) * sizeof(char));
  sprintf(path, "%s%s%s", home, dungeonPath, dungeon);
  printf("%s\n", path);
  if(load == 1){
    readFile(path);
  } else if(save == 1){
    saveFile(path);
  }
  if(load == 0){
    hardnessGen();
    genBorder();
    uint8_t rooms[6][4];
    roomGen(rooms);
    for (int i = 0; i < 6; i++)
    {
      hallways(i);
    }
    placeStaircase();
    if(both){
      saveFile(path);
      readFile(path);
    }
  }
  printboard();
  return 0;
}

void saveFile(char * path){
  FILE * f;
  f = fopen(path, "wb+");
  if(f == NULL) {
        fprintf(stderr, "FILE ERROR: Cannot write dungeon at %s\n", path);
    exit(1);
    }
  fseek(f, 0, SEEK_SET);
	char marker[7];
	strcpy(marker, "RLG327");
	fwrite(marker, sizeof(char), 6, f);

	/* write the file version marker */
	fseek(f, 6, SEEK_SET);
	uint32_t file_version = 0;
	uint32_t file_version_be = htobe32(file_version);
	fwrite(&file_version_be, sizeof(uint32_t), 1, f);

	/* write the size of the file ;; unsure how to properly calculate */
  //TODO
	fseek(f, 10, SEEK_SET);
  //TODO
 	uint32_t size = 1693 
	uint32_t size_be = htobe32(size);
	fwrite(&size_be, sizeof(uint32_t), 1, file);

	/* row-major dungeon matrix */
	fseek(file, 14, SEEK_SET);
	int pos = 14;
	int i;
	int j;

	for(i = 0; i < 21; i++) {
		for(j = 0; j < 80; j++) {
			fseek(file, pos, SEEK_SET);
			int8_t h;
			h = (int8_t)(playArea[i][j].matChar);
			fwrite(&h, sizeof(int8_t), 1, file);
			pos++;
		}
	}

	//Room writing
	fseek(f, 1694, SEEK_SET);
	for(i = 0; i < dungeon->nr; i++) {
		int8_t x = (int8_t) rooms[i].x;
		int8_t w = (int8_t) rooms[i].y;
		int8_t y = (int8_t) rooms[i].width;
		int8_t h = (int8_t) rooms[i].height;

		fwrite(&x, sizeof(int8_t), 1, f);
		fwrite(&w, sizeof(int8_t), 1, f);
		fwrite(&y, sizeof(int8_t), 1, f);
		fwrite(&h, sizeof(int8_t), 1, f);
	}
	fclose(f);
}
void readFile(char * path){
  FILE * f;
  f = fopen(path, "rb+");
  if(f == NULL){
    printf("Could not open FILE %s", path);
    exit(1);
  }
  fread(bin.file_type, 12, 1, f);
  fread(&bin.version, 4, 1, f);
  bin.version = htobe32(bin.version);
  fread(&bin.size, 4, 1, f);
  bin.size = htobe32(bin.size);
  fread(&bin.xPC, 1, 1, f);
  fread(&bin.yPC, 1, 1, f);
  //reads the hardness from the file
  //char * tempHardness = malloc((sizeof(bin.hardness) * sizeof(char)));
  //fread(tempHardness, 1680, 1, f);
  for (int i = 0; i < 21; i++)
  {
    for (int j = 0; j < 80; j++)
    {
      fread(&bin.hardness[i][j], sizeof(uint8_t), 1, f);
    }
  }
  fread(&bin.r, 2 ,1 , f);
  bin.r = htobe16(bin.r);
  uint8_t arr[bin.r][4];
  //reading room positions NEEDS TO BE CHECKED
  for(int j = 0; j < bin.r; j++){
    for(int i = 0; i < 4; i++){
      fread(&arr[j][i], 1, 1, f);
    }
  }
  
  fread(&bin.numUpStairs, 2, 1, f);
  bin.numUpStairs = htobe16(bin.numUpStairs);
  uint8_t tempUpStairs[bin.numUpStairs];
  uint8_t posUpStairs[bin.numUpStairs][2];
  for(int i = 0; i < bin.numUpStairs; i++){
    for(int j = 0; j < 2 ; j++){
      fread(&posUpStairs[i][j], 1, 1, f);
    }
  }
  fread(&bin.numDownStairs, 2, 1, f);
  bin.numDownStairs = htobe16(bin.numDownStairs);
  uint8_t posDownStairs[bin.numUpStairs][2];
  for(int i = 0; i < bin.numDownStairs; i++){
    for(int j = 0; j < 2 ; j++){
    fread(&posDownStairs[i][j], sizeof(uint8_t), 1, f);
    }
  }
  fclose(f);
  //using bin info and apply it to the playArea
  for (int i = 0; i < 21; i++){
    for (int j = 0; j < 80; j++){
      playArea[i][j].hardness = bin.hardness[i][j];
      if (playArea[i][j].hardness == 0){
        playArea[i][j].matChar = '#';
        for(int w = 0; w<bin.r;w++){
          int width = arr[w][2];
          int height = arr[w][3];
          int x = arr[w][0];
          int y = arr[w][1];
          int xEnd = x + width;
          int yEnd = y + height;
          for(int k = y; k < yEnd; k++){
            for(int l = x; l < xEnd; l++){
              if (i == k && j == l){
                playArea[k][l].matChar = '.';
              }
            }
          }
        }

      } else{
        playArea[i][j].matChar = ' ';
      }

    }
  }
  //upstaircase
  printf("%d", bin.numUpStairs);
  for(int i = 0; i<bin.numUpStairs;i++){
    playArea[posUpStairs[i][1]][posUpStairs[i][0]].matChar = '<';
  }
  
  //downstaircase
  for(int i = 0; i<bin.numDownStairs;i++){
    playArea[posDownStairs[i][1]][posDownStairs[i][0]].matChar = '>';
  }
  //PlayerCharacter
  playArea[bin.yPC][bin.xPC].matChar = '@';
  //place rooms
  //roomGen(arr);
}
//generates border and initializes matChar with spaces
void genBorder()
{
  for (int i = 0; i < 21; i++)
  {
    for (int j = 0; j <= 80; j++)
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
void roomGen(uint8_t arr[][4])
{
  int numRooms = 6;
  if(bin.r != 0){
    numRooms = bin.r;
  }
  srand(time(NULL));
  for (int i = 0; i < numRooms; i++)
  { //iterates through rooms in rooms array and initializes attributes
    if(bin.r == 0){
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
    } else{
      rooms[i].width = arr[i][2];
      ; //formula is rand() % (upperBound - lowerBound + 1) + lowerBound
      rooms[i].height = arr[i][3];
      ;
      rooms[i].x = arr[i][0];;
      ;
      rooms[i].y = arr[i][1];;
      ;
      rooms[i].xEnd = rooms[i].x + rooms[i].width;
      rooms[i].yEnd = rooms[i].y + rooms[i].height;
    }
    
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
