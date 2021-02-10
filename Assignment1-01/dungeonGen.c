#include <stdio.h>
#include <stdlib.h>

struct Room {
    int width;
    int height;
    int x;
    int y;
};

void genBorder();
void printBoard();

struct Room rooms[6];
char board[21][80];


int main(int argc, char* argv[]) {
    
    rooms[0].width = 5;
    rooms[0].height = 5;
    rooms[0].x = 10;
    rooms[0].y = 10;
    genBorder();
    for (int i = rooms[0].x; i <= rooms[0].width + rooms[0].x; i++) {
        for (int j = rooms[0].y; j <= rooms[0].height + rooms[0].y; j++) {
            board[i][j] = '.';
        }

    }
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