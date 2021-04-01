#ifndef dungeon_generator
#define dungeon_generator
	#include <vector>
	#include <string>

	typedef enum {RED, GREEN, BLUE, CYAN, YELLOW, MAGENTA, WHITE, BLACK} colour;
	typedef enum {WEAPON, OFFHAND, RANGED, ARMOR, HELMET, CLOAK, GLOVES, BOOTS, RING, AMULET, LIGHT, NONEQUIP, SCROLL, BOOK, FLASK, GOLD, AMMUNITION, FOOD, WAND, CONTAINER} itype;

	class PC;
	
	class Dice {
		int	b;	
		int	d;
		int	n;
	public:
		int	roll(void);
		Dice(int, int, int);
		std::string string(void);
	};
	
	class Item {
	public:
		std::string	n;	
		std::string* desc;
		int	dl;
		itype t;
		bool e;	
		colour c;
		Dice* d;
		int	hib;
		int	dob;
		int	deb;
		int	w;	
		int	spb;		
		int	sa;			
		int	v;			
	};

	class Memory {
	public:
		char c; 
		bool v; 
	};

	class Position {
	public:
		int	x; 
		int	y; 
	};

	class Stats {
	public:
		bool in; 
		bool te; 
		bool tu; 
		bool eb; 
		int s;  
		bool pa;	
		Dice* a;	
		int	hp;	
	};

	class Sprite {
	public:
		friend class PC;
		friend class Monster;
		Position p;	
		char c;	
		Stats s;	
		int	t;	
		Position to;	
		int	sn;	
		Position pc;	
		bool a;	
		std::string	n; 
		colour	color;	
		std::string* desc;	
		int	dl;	
		Sprite* thisSprite(void); 
		PC*  thisPC(void);
	};

	class Monster : public Sprite {
	public:
		friend class Sprite;
	};

	class PC : public Sprite {
	public:
		friend class Sprite;
		int	view;	
		Memory	**	mem;
	};

	#define	TRUE	1
	#define	FALSE	0
	typedef int Bool;

	typedef struct {
		int	h;	
		char c;	
		int	p;	
	} Tile;

	typedef struct {
		int prev; 
		int next; 
	} Path;

	typedef struct {
		Position* tl;	
		Position* br;	
		int	w;	
		int	h;	
		int	id;	
		int	p;	
		Position* ctr;	
		int	c;	
	} Room;

	typedef struct {
		Tile **d;		
		Tile **p;		
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
		int ** csnt;
		int	** cst;	
		int	pc;		
	    int t;		
		Bool go;		
		Position *su;		
		Position *sd;		
		PC * plyr;	
		
		int	mi;		
		int	di;		
		Item *	id;		
		int	mm;		
		int	dm;		
		Monster *md;		
	} Dungeon;

	typedef struct {
		int x;
		int y;
		int cost;
		int v;
	} Tile_Node;

	void add_sprite(Dungeon * dungeon, Sprite * s);
	Sprite * gen_sprite(Dungeon * dungeon, char c, int x, int y, int r);
	void gen_move_sprite(Dungeon * dungeon, int sn);
	void parse_move(Dungeon * dungeon, int sn);
	Bool check_any_monsters(Dungeon * dungeon);
	Bool test_loc(Dungeon * dungeon, int x, int y, Sprite *s);
	void with_pc(Dungeon * dungeon, Sprite * s, Bool *in);

	void gen_corridors(Dungeon * dungeon);
	void gen_dungeon(Dungeon * dungeon);
	Dungeon init_dungeon(int h, int w, int mr);

	void print_hardness(Dungeon * dungeon);
	void print_t_heatmap(Dungeon * dungeon);
	void print_nont_heatmap(Dungeon * dungeon);
	void print_dungeon(Dungeon * dungeon, int nt, int t);
	void print_dungeon_nnc(Dungeon * dungeon, int nt, int t);

	void map_dungeon_t(Dungeon * dungeon);


	Sprite * initSprite();
	Sprite * initSprites(int);

	void copySprite(Sprite *, Sprite *);
	void copyASprite(Sprite * to, int n, Sprite * from);
	Sprite * thisASprite(Sprite * arr, int i);

	int getSpritePX(Sprite *);
	int getSpritePY(Sprite *);
	char getSpriteC(Sprite *);
	bool getSpriteSIn(Sprite *);
	bool getSpriteSTe(Sprite *);
	bool getSpriteSTu(Sprite *);
	bool getSpriteSEb(Sprite *);
	int getSpriteSS(Sprite *);
	int getSpriteT(Sprite *);
	int getSpriteToX(Sprite *);
	int getSpriteToY(Sprite *);
	int getSpriteSn(Sprite *);
	int getSpritePcX(Sprite *);
	int getSpritePcY(Sprite *);
	bool getSpriteA(Sprite *);
	int getSpriteAPX(Sprite *, int);
	int getSpriteAPY(Sprite *, int);
	char getSpriteAC(Sprite *, int);
	bool getSpriteASIn(Sprite *, int);
	bool getSpriteASTe(Sprite *, int);
	bool getSpriteASTu(Sprite *, int);
	bool getSpriteASEb(Sprite *, int);
	int getSpriteASS(Sprite *, int);
	int getSpriteAT(Sprite *, int);
	int getSpriteAToX(Sprite *, int);
	int getSpriteAToY(Sprite *, int);
	int getSpriteASn(Sprite *, int);
	int getSpriteAPcX(Sprite *, int);
	int getSpriteAPcY(Sprite *, int);
	bool getSpriteAA(Sprite *, int);
	void setSpritePX(Sprite *, int);
	void setSpritePY(Sprite *, int);
	void setSpriteC(Sprite *, char);
	void setSpriteSIn(Sprite *, bool);
	void setSpriteSTe(Sprite *, bool);
	void setSpriteSTu(Sprite *, bool);
	void setSpriteSEb(Sprite *, bool);
	void setSpriteSS(Sprite *, int);
	void setSpriteT(Sprite *, int);
	void setSpriteToX(Sprite *, int);
	void setSpriteToY(Sprite *, int);
	void setSpriteSn(Sprite *, int);
	void setSpritePcX(Sprite *, int);
	void setSpritePcY(Sprite *, int);
	void setSpriteA(Sprite *, bool);
	void setSpriteAPX(Sprite *, int, int);
	void setSpriteAPY(Sprite *, int, int);
	void setSpriteAC(Sprite *, int, char c);
	void setSpriteASIn(Sprite *, int, bool);
	void setSpriteASTe(Sprite *, int, bool);
	void setSpriteASTu(Sprite *, int, bool);
	void setSpriteASEb(Sprite *, int, bool);
	void setSpriteASS(Sprite *, int, int);
	void setSpriteAT(Sprite *, int, int);
	void setSpriteAToX(Sprite *, int, int);
	void setSpriteAToY(Sprite *, int, int);
	void setSpriteASn(Sprite *, int, int);
	void setSpriteAPcX(Sprite *, int, int);
	void setSpriteAPcY(Sprite *, int, int);
	void setSpriteAA(Sprite *, int, bool);

	PC * initPC(Dungeon *);

	int getPosX(Position * p);
	int getPosY(Position * p);
	void setPosX(Position * p, int n);
	void setPosY(Position * p, int n);
	Position * initPos(void);

	void updateMemory(Dungeon *);
	char getMem(Dungeon *, int, int);
	PC * getPC(Sprite * arr);
	PC * thisAPC(Sprite * arr, int i);

#endif
