#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

struct Room {
    int width;
    int height;
    int x;
    int y;
    int xEnd;
    int yEnd;
};

void genBorder();
void printBoard();
void roomGen();

struct Room rooms[6];
char board[21][80];


int main(int argc, char* argv[]) {
    genBorder();
    roomGen();
    printBoard();
    
   
}

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

void printBoard() {
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 80; j++) {
            printf("%c", board[i][j]);
        }
        printf("\n");
    }
}

void roomGen() {
    srand(time(NULL));
    for (int i = 0; i <= 6; i++) {
        rooms[i].width = rand() % (6 - 4 + 1) + 4; ;
        rooms[i].height = rand() % (5 - 3 + 1) + 3; ;
        rooms[i].x = rand() % (70 - 1 + 1) + 1; ;
        rooms[i].y = rand() % (11 - 1 + 1) + 1; ;
        rooms[i].xEnd = rooms[i].x + rooms[i].width;
        rooms[i].yEnd = rooms[i].y + rooms[i].height;
        
        for (int j = rooms[i].y; j < rooms[i].yEnd; j++) {
            for (int k = rooms[i].x; k < rooms[i].xEnd; k++) {
                
                board[j][k] = '.';
            }
        }
    }
}
