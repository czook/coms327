#ifndef dungeon_generator
#define dungeon_generator
#include "binheap.h"
#define	TRUE	1
#define	FALSE	0
typedef int bool;

typedef struct {
	int	h;	
	char c;	
	int	 p;	
} Tile;

typedef struct {
	int	x; 
	int	y; 
} Position;

typedef struct {
	int prev; 
	int next; 
} Path;

typedef struct {
    bool in; 
    bool te; 
    bool tu; 
    bool eb; 
    int s; 
} Stats;

typedef struct {
	Position p;	
	char c;	
    Stats s;	
	int	t;	
	Position to;	
	int sn;	
	Position pc;
	bool a;	
} Sprite;

typedef struct {
	Position tl;
	Position br;
	int	w;
	int	h;
	int	id;
	int	p;
	Position ctr;
	int	c;
} Room;

typedef struct {
	Tile **	d;		
	Tile **	p;		
	int	h;		
	int	w;		
	int	nr; 	
	int	mr;		
	Room *r;		
	int	v;		
	int	s;		
	Sprite *ss;		
	int	ns;		
	int	ms;		
	int	**csnt;
	int	**cst;	
	int	pc;		
    int t;      
	bool go;
} Dungeon;

typedef struct {
	int x;
	int y;
	int cost;
	int v;
} Tile_Node;

typedef struct {
	int	sn;	
	int speed;	
	int	turn;	
	Position to;
} Event;


void add_sprite(Dungeon * dungeon, Sprite s);
Sprite gen_sprite(Dungeon * dungeon, char c, int x, int y, int r);
void gen_move_sprite(Dungeon * dungeon, int sn);
void parse_move(Dungeon * dungeon, int sn);
bool check_any_monsters(Dungeon * dungeon);

void map_dungeon_t(Dungeon * dungeon);

#endif
