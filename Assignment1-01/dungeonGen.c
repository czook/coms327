#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

//room object
struct Room { 
    int width;
    int height;
    int x; //x coordinate of top left corner of room
    int y; //y coordinate of top left corner of room
    int xEnd; // x coordinate of bottom right corner of room
    int yEnd; // y coordinate of bottom right corner of room
};

void genBorder();
void printBoard();
void roomGen();

//array to hold all room objects so they can be accessed
struct Room rooms[6];
//2D array of our game screen
char board[21][80];


int main(int argc, char* argv[]) {
    genBorder();
    roomGen();
    printBoard();
    
   
}
//generates border and initializes board with spaces
void genBorder() {
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 80; j++) {
            if (i == 0) {
                board[i][j] = '-';
            }
            if (i != 0 && i != 20 && (j == 0 || j == 79)) {
                board[i][j] = '|';
            }
            if (i == 20) {
                board[i][j] = '-';
            }
            if (i > 0 && i < 20 && j > 0 && j < 79) {
                board[i][j] = ' ';
            }
        }
    }
}
//prints all charcters on board array
void printBoard() {
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 80; j++) {
            printf("%c", board[i][j]);
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
        
        for (int j = rooms[i].y; j < rooms[i].yEnd; j++) { //places periods on board array for room locations
            for (int k = rooms[i].x; k < rooms[i].xEnd; k++) {
                
                board[j][k] = '.';
            }
        }
    }
}
