// Based off -- https://en.wikipedia.org/wiki/Abelian_sandpile_model
const int prim =23;
#include <stdio.h>
 void main(){
    int grid[prim][prim]; 
    //printf("Debug");
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