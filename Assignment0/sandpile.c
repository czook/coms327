// Based off -- https://en.wikipedia.org/wiki/Abelian_sandpile_model
const int prim =23;
void inputVals(int grid[prim][prim],int x, int y, int h);
void worldGen(int grid[prim][prim]);
void printWorld(int grid[prim][prim]);
int pilefall(int x, int y, int grid[prim][prim]);
#include <stdio.h>
#include <stdlib.h>
 int main(int argc, char* argv[]){
    int grid[prim][prim];
    worldGen(grid);
    for (int i = 1; i< argc-1; i++){
        printf("%d",argc);
        inputVals(grid, atoi(argv[i]), atoi(argv[i+1]), atoi(argv[i+2]));
        i+=2;
    }

    //printWorld(grid);
    while(1){
        //add sand in the middle
        grid[11][11]++;
        pilefall(11,11, grid);
        printWorld(grid);
    }
    return 0;
 }
//use this method after worldGen
 void inputVals(int grid[prim][prim],int x, int y, int h){
    if (x<22 || y<22|| x>0 || y>0){
        if (x==11 && y==11){
            printf("A sink at the center is invalid");
        }
        if (h==-1){}
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
            if(grid[i][j]==-1){
                printf("# ");
            }else{
                printf("%d ", grid[i][j]);
            }
        }
        printf("\n");
    }
    printf("\n");
 }

//recursive
 int pilefall(int x, int y, int grid[prim][prim]){
    if(grid[x][y]<=8){
         //base case
         return 0;
    }
    grid[x][y]= grid[x][y] - 8;
     //top left
    if(x-1>=0){
        if(grid[x-1][y]!=-1){
            grid[x-1][y]++;
            pilefall(x-1, y, grid);
        }
    }
    if(y-1>=0&&x-1>=0){
        if(grid[x-1][y-1]!=-1){
            grid[x-1][y-1]++;
            pilefall(x-1, y-1, grid);
        }
    }
    if(y+1<prim&&x-1>=0){
        if(grid[x-1][y+1]!=-1){
            grid[x-1][y+1]++;
            pilefall(x-1, y+1, grid);
        }
    }
    if(x+1<prim){
        if(grid[x+1][y]!=-1){
            grid[x+1][y]++;
            pilefall(x+1, y, grid);
        }
    }
    if(y-1>=0&&x+1<prim){
        if(grid[x+1][y-1]!=-1){
            grid[x+1][y-1]++;
            pilefall(x+1, y-1, grid);
        }
    }
    if(y+1<prim&&x+1<prim){
        if(grid[x+1][y+1]!=-1){
            grid[x+1][y+1]++;
            pilefall(x+1, y+1, grid);
        }
    }
    if(y-1>=0){
        if(grid[x][y-1]!=-1){
            grid[x][y-1]++;
            pilefall(x, y-1, grid);
        }
    }
    if(y+1<prim){
        if(grid[x][y+1]!=-1){
            grid[x][y+1]++;
            pilefall(x, y+1, grid);
        }
    }
    if(grid[x][y] > 8){
        pilefall(x,y,grid);
    }
    
 }
 