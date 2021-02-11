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
void hallways(int num);
void insertLeft(int *x, int *y);
void insertRight(int *x, int *y);
void insertUp(int *x, int *y);
void insertDown(int *x, int *y);
void hardnessGen();
void placeStaircase();

//array to hold all room objects so they can be accessed
struct Room rooms[6];
//2D array of the matChars of the game screen
struct Grid playArea[21][80];
//counts floor tiles
int countTiles = 0;