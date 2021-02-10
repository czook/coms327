#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <math.h>

//room object
typedef struct Room { 
    int width;
    int height;
    int x; //x coordinate of top left corner of room
    int y; //y coordinate of top left corner of room
    int xEnd; // x coordinate of bottom right corner of room
    int yEnd; // y coordinate of bottom right corner of room
}Room;
typedef struct Grid{
    int hardness;
    char matChar;
}Grid;

void genBorder();
void printboard();
void roomGen();
int overlapChecker(int roomIndex);
void connect(int num);
void insertLeft(int *x, int *y);
void insertRight(int *x, int *y);
void insertUp(int *x, int *y);
void insertDown(int *x, int *y);
void hardnessGen();

//array to hold all room objects so they can be accessed
struct Room rooms[6];
//2D array of the matChars of the game screen
struct Grid playArea[21][80];



int main(int argc, char* argv[]) {
    hardnessGen();
    genBorder();
    roomGen();
    for(int i =0; i<6; i++){
        connect(i);
    }
    printboard();
   
}
//generates border and initializes matChar with spaces
void genBorder() {
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 80; j++) {
            if (i == 0) {
                playArea[i][j].matChar = '-';
            }
            if (i != 0 && i != 20 && (j == 0 || j == 79)) {
                playArea[i][j].matChar = '|';
            }
            if (i == 20) {
                playArea[i][j].matChar = '-';
            }
            if (i > 0 && i < 20 && j > 0 && j < 79) {
                playArea[i][j].matChar = ' ';
            }
        }
    }
}
//Generates the hardness of the matChars
void hardnessGen(){
    srand(time(NULL));

    //Random hardness for all rock in playArea
    int i, j;
    for(i = 0; i < 21; i++){
        for(j = 0; j < 80 ; j++){
            struct Grid matgrid;
            matgrid.matChar = ' ';
            playArea[i][j] = matgrid;
            if(i == 0 || i == 21 || j == 0 || j == 80)
                playArea[i][j].hardness = 255;
            else
                playArea[i][j].hardness = rand() % 254 + 1;
        }
    }
}
//prints all charcters on matChar array
void printboard() {
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 80; j++) {
            printf("%c", playArea[i][j].matChar);
        }
        printf("\n");
    }
}
//generates rooms
void roomGen() {
    srand(time(NULL));
    for (int i = 0; i <= 6; i++) { //iterates through rooms in rooms array and initializes attributes
        rooms[i].width = rand() % (6 - 4 + 1) + 4; ; //formula is rand() % (upperBound - lowerBound + 1) + lowerBound
        rooms[i].height = rand() % (5 - 3 + 1) + 3; ;
        rooms[i].x = rand() % (70) + 1; ;
        rooms[i].y = rand() % (11) + 1; ;
        rooms[i].xEnd = rooms[i].x + rooms[i].width;
        rooms[i].yEnd = rooms[i].y + rooms[i].height;
        if (overlapChecker(i) == 0) {
            i--;
            continue;
        }
        
        for (int j = rooms[i].y; j < rooms[i].yEnd; j++) { //places periods on matChar array for room locations
            for (int k = rooms[i].x; k < rooms[i].xEnd; k++) {
                playArea[j][k].matChar = '.';
                
            }
        }
        
    }
}

int overlapChecker(int roomIndex) { 
    for (int x = rooms[roomIndex].x; x < rooms[roomIndex].x + rooms[roomIndex].width; x++) {
        for (int y = rooms[roomIndex].y; y < rooms[roomIndex].y + rooms[roomIndex].height; y++) {
            if (playArea[y][x].matChar == '.') {
                return 0;
            }
        }
    }
    return 1;
}
void connect(int num){
  int i;
  double distance = 10000.0;
  struct Room closest;
  closest.x = rooms[num].x;
  closest.y = rooms[num].y;
  //Cycles through already connected rooms and finds closest
  for(i = 0; i < num; i++){
    int xDis = abs(rooms[num].x - rooms[i].x);
    int yDis = abs(rooms[num].x - rooms[i].y);
    //Calculates distance between two rooms, sets new closest point if distance is less
    if(sqrt(xDis * xDis + yDis * yDis) < distance){
      distance = sqrt(xDis * xDis + yDis * yDis);
      closest.x = rooms[i].x;
      closest.y = rooms[i].y;
    }
  }

  int xcpy = rooms[num].x;
  int ycpy = rooms[num].y;
  while(xcpy != closest.x || ycpy != closest.y){
    
    //Right
    if(xcpy < closest.x)
      insertRight(&xcpy, &ycpy);
    //Left
    else if(xcpy > closest.x)
      insertLeft(&xcpy, &ycpy);
    //Down
    else if(ycpy < closest.y)
      insertDown(&xcpy, &ycpy);
    //Up
    else if(ycpy> closest.y)
      insertUp(&xcpy, &ycpy);
    //Down and Right
    else if(xcpy < closest.x && ycpy < closest.y){
      if(playArea[ycpy][xcpy+1].hardness < playArea[ycpy+1][xcpy].hardness)
        insertRight(&xcpy, &ycpy);
      else
        insertDown(&xcpy, &ycpy);
    }
    //Down and left
    else if(xcpy > closest.x && ycpy < closest.y){
      if(playArea[ycpy][xcpy-1].hardness < playArea[ycpy+1][xcpy].hardness)
        insertLeft(&xcpy, &ycpy);
      else
        insertDown(&xcpy, &ycpy);
    }
    //Up and left
    else if(xcpy < closest.x && ycpy > closest.y){
      if(playArea[ycpy][xcpy+1].hardness < playArea[ycpy-1][xcpy].hardness)
        insertRight(&xcpy, &ycpy);
      else
        insertUp(&xcpy, &ycpy);
    }
    //Right and Up
    else if(xcpy > closest.x && ycpy > closest.y){
      if(playArea[ycpy][xcpy-1].hardness < playArea[ycpy-1][xcpy].hardness)
        insertLeft(&xcpy, &ycpy);
      else
	insertUp(&xcpy, &ycpy);
    }
    
  }
     
}

void insertLeft(int *x, int *y){
  *x = *x - 1;
  if(playArea[*y][*x].matChar != '.'){
    playArea[*y][*x].matChar = '#';
    playArea[*y][*x].hardness = 0;
  }
}

void insertRight(int *x, int *y){
*x = *x + 1;
 if(playArea[*y][*x].matChar != '.'){
    playArea[*y][*x].matChar = '#';
    playArea[*y][*x].hardness = 0;
  }
}

void insertUp(int *x, int *y){
  *y = *y - 1;
  if(playArea[*y][*x].matChar != '.'){
    playArea[*y][*x].matChar = '#';
    playArea[*y][*x].hardness = 0;
  }
}

void insertDown(int *x, int *y){
*y = *y + 1;
 if(playArea[*y][*x].matChar != '.'){
    playArea[*y][*x].matChar = '#';
    playArea[*y][*x].hardness = 0;
  }
}
