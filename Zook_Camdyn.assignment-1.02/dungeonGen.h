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
    Room rooms[]; //TODO
}Grid;
typedef struct binary{
    char file_type[12];
    uint32_t version;
    uint32_t size;
    uint8_t xPC;
    uint8_t yPC;
    uint8_t hardness[21][80];
    uint16_t r;
    uint16_t numUpStairs;
    uint8_t * xUpStairs;
    uint8_t * yUpStairs;
    uint16_t numDownStairs;
    uint8_t * xDownStairs;
    uint8_t * yDownStairs;
    uint8_t (*rPos)[4];
}binary;

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
void saveFile(char * path);
void readFile(char * path);


//array to hold all room objects so they can be accessed
struct Room rooms[6];
//2D array of the matChars of the game screen
struct Grid playArea[21][80];
//counts floor tiles
int countTiles = 0;
//for reading binaries
struct binary bin;