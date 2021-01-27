// Based off -- https://en.wikipedia.org/wiki/Abelian_sandpile_model
const int prim =23;
void worldGen(int grid[prim][prim]);
void printWorld(int grid[prim][prim]);
void runWorld(int grid[prim][prim]);
void pilefall(int x, int y, int grid[prim][prim]);
#include <stdio.h>
 int main(int argc, char *argv[]){
    int grid[prim][prim]; 
    //printf("Debug");
    worldGen(grid);
    printWorld(grid);
    return 0;
 }
//use this method after worldGen
 void inputVals(int grid[prim][prim],int x, int y, int h){
    if (x>22 || y>22|| x<0 || y<0){
        if (x==11 && y==11){
            printf("A sink at the center is invalid");
        }
        grid[x][y] = h;
    }
    
 }


 void worldGen(int grid[prim][prim]){
    for(int i =0; i<23; i++){
        for(int j =0; j<23; j++){
                grid[i][j] = 0;
        }
    }
 }
//in the future will use ncurses
 void printWorld(int grid[prim][prim]){
     for(int i =0; i<23; i++){
        for(int j =0; j<23; j++){
            printf("%d ", grid[i][j]);
        }
        printf("\n");
    }
 }

 void runWorld(int grid[prim][prim]){
    while(){
        //add sand in the middle
        if(grid[11][11]==-1){
            printf("There is a sink on the center");
        }
        grid[11][11]++;
    }
    
 }
//recursive
 void pilefall(int x, int y, int grid[prim][prim]){
    if(grid[x][y]<8){
         //base case
         break;
    }
     //top left
    if(x-1>=0){
        grid[x-1][y]++;
        if(y-1>=0){
            grid[x-1][y-1]++;
        } else if(y+1<=prim){
            grid[x-1][y+1]++;
        }
    } else if(x+1<=prim){
        grid[x+1][y]++;
        if(y-1>=0){
            grid[x+1][y-1]++;
        } else if(y+1<=prim){
            grid[x+1][y+1]++;
        }
    } else if(y-1>=0){
        grid[x][y-1]++;
    } else if(x+1<=prim){
        grid[x][y+1]++;
    }
 }
 