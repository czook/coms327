#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <ncurses.h>
#include "binheap.h"
#include "dungeon_generator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
using namespace std;


int compare_int(const void *key, const void *with) {
	return (const int) ((*(Tile_Node *) key).cost - (*(Tile_Node *) with).cost);
}


int h_calc(int h) {
	int hc = 0;

	if(h >= 0 && h < 85) {
		return 1;
	}
	if(h > 84 && h < 171) {
		return 2;
	}
	if(h > 170 && h < 255) {
		return 3;
	}

	return hc;
}


void map_dungeon_t(Dungeon * dungeon) {
	binheap_t h;
	Tile_Node tiles[dungeon->h][dungeon->w];

	binheap_init(&h, compare_int, NULL);

	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	int i;
	int j;

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			tiles[i][j].y = i;
			tiles[i][j].x = j;
			tiles[i][j].cost = INT_MAX;
			tiles[i][j].v = FALSE;
		}
	}

	int px = getSpriteAPX(dungeon->ss, dungeon->pc);
	int py = getSpriteAPY(dungeon->ss, dungeon->pc);
	
	tiles[py][px].cost = 0;
	tiles[py][px].v = TRUE;
	binheap_insert(&h, &tiles[py][px]);


	binheap_node_t	*p;

	while((p = (binheap_node_t*)binheap_remove_min(&h))) {
		int hx = ((Tile_Node *) p)->x;
		int hy = ((Tile_Node *) p)->y;
		int tc = ((Tile_Node *) p)->cost;

		int i;
		for(i = 0; i < 8; i++) {
			int x = hx + xs[i];
			int y = hy + ys[i];
			if(x > 0 && x < dungeon->w-1 && y > 0 && y < dungeon->h-1) {
				int hard = dungeon->d[y][x].h;
				if(hard < 255) {
						int trial_cost = tc + h_calc(hard);
						if((tiles[y][x].cost > trial_cost && tiles[y][x].v == TRUE) || tiles[y][x].v == FALSE) {
							tiles[y][x].cost = tc + h_calc(hard);
							tiles[y][x].v = TRUE;

							binheap_insert(&h, (void *) &tiles[y][x]);
						}
				}
			}
		}
	}

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->cst[i][j] = tiles[i][j].cost;
		}
	}


	binheap_delete(&h);
}

void map_dungeon_nont(Dungeon * dungeon) {
	binheap_t h;
	Tile_Node tiles[dungeon->h][dungeon->w];

	binheap_init(&h, compare_int, NULL);

	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	int i;
	int j;

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			tiles[i][j].y = i;
			tiles[i][j].x = j;
			tiles[i][j].cost = INT_MAX;
			tiles[i][j].v = FALSE;
		}
	}


	int px = getSpriteAPX(dungeon->ss, dungeon->pc);
	int py = getSpriteAPY(dungeon->ss, dungeon->pc);
	
	tiles[py][px].cost = 0;
	tiles[py][px].v = TRUE;
	binheap_insert(&h, &tiles[py][px]);


	binheap_node_t	*p;

	while((p = (binheap_node_t*)binheap_remove_min(&h))) {
		int hx = ((Tile_Node *) p)->x;
		int hy = ((Tile_Node *) p)->y;
		int tc = ((Tile_Node *) p)->cost;

		int i;
		for(i = 0; i < 8; i++) {
			int x = hx + xs[i];
			int y = hy + ys[i];
			if(x > 0 && x < dungeon->w-1 && y > 0 && y < dungeon->h-1) {
				int hard = dungeon->d[y][x].h;
				if(hard == 0) {
						int trial_cost = tc + h_calc(hard);
						if((tiles[y][x].cost > trial_cost && tiles[y][x].v == TRUE) || tiles[y][x].v == FALSE) {
							tiles[y][x].cost = tc + h_calc(hard);
							tiles[y][x].v = TRUE;

							binheap_insert(&h, (void *) &tiles[y][x]);
						}
				}
			}
		}

	}

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->csnt[i][j] = tiles[i][j].cost;
		}
	}


	binheap_delete(&h);
}

void read_dungeon(Dungeon * dungeon, char * path) {
	FILE * file;
	file = fopen(path, "rb+");
	if(file == NULL) {
		fprintf(stderr, "FILE ERROR: Could not open dungeon file at %s! read_dungeon()\n", path);
        exit(1);
	}

	
	fseek(file, 0, SEEK_SET);
	char marker[6];
	fread(marker, 1, 6, file);

	fseek(file, 6, SEEK_SET);
	uint32_t file_version;
	uint32_t file_version_be;
	fread(&file_version_be, sizeof(uint32_t), 1, file);
	file_version = be32toh(file_version_be);
	dungeon->v = file_version;

	fseek(file, 10, SEEK_SET);
	uint32_t size;
	uint32_t size_be;
	fread(&size_be, sizeof(uint32_t), 1, file);
	size = be32toh(size_be);
	dungeon->s = size;

	fseek(file, 14, SEEK_SET);
	int i;
	int j;
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			int h;
			int8_t h_8;
			fread(&h_8, sizeof(int8_t), 1, file);
			h = (int) h_8;
			dungeon->d[i][j].h = h;
		}
	}

	fseek(file, 1694, SEEK_SET);
	int room_i = 0;
	int room_count = (size - 1693) / 4;
	dungeon->nr = room_count;
	dungeon->r = (Room*)calloc(room_count, sizeof(Room));
	int pos;
	for(pos = 1694; pos < (int)size; pos += 4) {
		int x_8;
		int w_8;
		int y_8;
		int h_8;
		fread(&x_8, sizeof(int8_t), 1, file);
		fread(&w_8, sizeof(int8_t), 1, file);
		fread(&y_8, sizeof(int8_t), 1, file);
		fread(&h_8, sizeof(int8_t), 1, file);
		setPosX(dungeon->r[room_i].tl, (int8_t) x_8);
		dungeon->r[room_i].w = (int8_t) w_8;
		setPosY(dungeon->r[room_i].tl, (int8_t) y_8);
		dungeon->r[room_i].h = (int8_t) h_8;
		setPosX(dungeon->r[room_i].br, ((int8_t) x_8) + dungeon->r[room_i].w-1);
		setPosY(dungeon->r[room_i].br, ((int8_t) y_8) + dungeon->r[room_i].h-1);
		room_i++;
	}

	int h;
	for(h = 0; h < dungeon->nr; h++) {
		for(i = getPosY(dungeon->r[h].tl); i < getPosY(dungeon->r[h].br)+1; i++) {
			for(j = getPosX(dungeon->r[h].tl); j < getPosX(dungeon->r[h].br)+1; j++) {
				dungeon->d[i][j].c = '.';
			}
		}
	}

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			if(dungeon->d[i][j].c != '.' && dungeon->d[i][j].h == 0) {
				dungeon->d[i][j].c = '#';
				dungeon->d[i][j].p = 1;
			}
		}
	}


	fclose(file);
}

void write_dungeon(Dungeon * dungeon, char * path) {
	FILE * file;
	char * env_home = getenv("HOME");
	char * fdir_path;
	fdir_path = (char*)calloc(strlen(env_home) + 9, sizeof(char));
	strcpy(fdir_path, env_home);
	strcat(fdir_path, "/.rlg327");
	mkdir(fdir_path, S_IRWXU);
	file = fopen(path, "wb+");
	if(file == NULL) {
		fprintf(stderr, "FILE ERROR: Could not open dungeon file at %s! write_dungeon()\n", path);
        exit(1);
	}

	fseek(file, 0, SEEK_SET);
	char marker[7];
	strcpy(marker, "RLG327");
	fwrite(marker, sizeof(char), 6, file);

	fseek(file, 6, SEEK_SET);
	uint32_t file_version = 0;
	uint32_t file_version_be = htobe32(file_version);
	fwrite(&file_version_be, sizeof(uint32_t), 1, file);

	fseek(file, 10, SEEK_SET);
 	uint32_t size = 1693 + (4 * dungeon->nr);
	uint32_t size_be = htobe32(size);
	fwrite(&size_be, sizeof(uint32_t), 1, file);

	fseek(file, 14, SEEK_SET);
	int pos = 14;
	int i;
	int j;

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			fseek(file, pos, SEEK_SET);
			int8_t h;
			h = (int8_t)(dungeon->d[i][j].h);
			fwrite(&h, sizeof(int8_t), 1, file);
			pos++;
		}
	}

	fseek(file, 1694, SEEK_SET);
	for(i = 0; i < dungeon->nr; i++) {
		int8_t x = (int8_t) getPosX(dungeon->r[i].tl);
		int8_t w = (int8_t) dungeon->r[i].w;
		int8_t y = (int8_t) getPosY(dungeon->r[i].tl);
		int8_t h = (int8_t) dungeon->r[i].h;

		fwrite(&x, sizeof(int8_t), 1, file);
		fwrite(&w, sizeof(int8_t), 1, file);
		fwrite(&y, sizeof(int8_t), 1, file);
		fwrite(&h, sizeof(int8_t), 1, file);
	}

	free(fdir_path);
	fclose(file);
}

void test_args(int argc, char ** argv, int here, int * s, int * l, int *p, int *cp, int *nm, int *nnc) {
		if(strcmp(argv[here], "--save") == 0) {
			*s = TRUE;
		} else if(strcmp(argv[here], "--load") == 0) {
			*l = TRUE;
		} else if(strcmp(argv[here], "-f") == 0) {
			*p = TRUE;
			*cp = here+1;
			if(here+1 > argc-1) {
				printf("Invalid filename argument!\n");
				*p = FALSE;
			}
		} else if(strcmp(argv[here], "--nummon") == 0) {
			*nm = atoi(argv[here+1]);
		} else if(strcmp(argv[here], "--no-ncurses") == 0) {
			*nnc = TRUE;
		}
}

void monster_list(Dungeon * dungeon) {
	clear();

	char mons [dungeon->ns-1][30];
	int i;
	for(i = 1; i < dungeon->ns; i++) {
		char ns[6];
		char ew[5];

		int hd = getSpriteAPY(dungeon->ss, 0) - getSpriteAPY(dungeon->ss, i);
		int wd = getSpriteAPX(dungeon->ss, 0) - getSpriteAPX(dungeon->ss, i);

		if(hd > 0)
			strcpy(ns, "north");
		else
			strcpy(ns, "south");

		if(wd > 0)
			strcpy(ew, "west");
		else
			strcpy(ew, "east");

		sprintf(mons[i-1], "%c, %2d %s and %2d %s", getSpriteAC(dungeon->ss, i), abs(hd), ns, abs(wd), ew);
	}

	WINDOW *w;
	w = newwin(24, 80, 0, 0);
	Bool scroll = FALSE;
	int top = 0;
	int bot;
	if(24 < dungeon->ns -1) {
		scroll = TRUE;
		bot = 23;
	} else {
		bot = dungeon->ns -2;
	}

	int j;
	for(;;) {
		for(i = top, j = 0; i < dungeon->ns -1 && i <= bot && j < 24; i++, j++) {
			mvprintw(j, 0, mons[i]);
		}
		MLV: ;
		int32_t k;
		k = getch();

		switch(k) {
			case KEY_UP:
				if(scroll == FALSE)
					goto MLV;

				if(top-1 >= 0) {
					top--;
					bot--;
				}
				clear();

				break;
			case KEY_DOWN:
				if(scroll == FALSE)
					goto MLV;

				if(bot+1 < dungeon->ns-1) {
					bot++;
					top++;
				}
				clear();

				break;
			case 27:
				return;
				break;
			default:
				goto MLV;
		}

		wrefresh(w);
	}

	delwin(w);
	print_dungeon(dungeon, 0, 0);
}

void parse_pc(Dungeon * dungeon, Bool * run, Bool * regen) {
	GCH: ;
	int32_t k;
	k = getch();
	if(k == 'Q') {
		*run = FALSE;
		return;
	}

	switch(k) {
		case 'h':
			H: ;
			setSpriteAToX(dungeon->ss, dungeon->pc, getSpriteAPX(dungeon->ss, dungeon->pc) -1);
			break;
		case '4':
			goto H;
		case 'l':
			L: ;
			setSpriteAToX(dungeon->ss, dungeon->pc, getSpriteAPX(dungeon->ss, dungeon->pc) +1);
			break;
		case '6':
			goto L;
		case 'k':
			K: ;
			setSpriteAToY(dungeon->ss, dungeon->pc, getSpriteAPY(dungeon->ss, dungeon->pc) -1);
			break;
		case '8':
			goto K;
		case 'j':
			J: ;
			setSpriteAToY(dungeon->ss, dungeon->pc, getSpriteAPY(dungeon->ss, dungeon->pc) +1);
			break;
		case '2':
			goto J;
		case 'y':
			Y: ;
			setSpriteAToX(dungeon->ss, dungeon->pc, getSpriteAPX(dungeon->ss, dungeon->pc) -1);
			setSpriteAToY(dungeon->ss, dungeon->pc, getSpriteAPY(dungeon->ss, dungeon->pc) -1);
			break;
		case '7':
			goto Y;
		case 'u':
			U: ;
			setSpriteAToX(dungeon->ss, dungeon->pc, getSpriteAPX(dungeon->ss, dungeon->pc) -1);
			setSpriteAToY(dungeon->ss, dungeon->pc, getSpriteAPY(dungeon->ss, dungeon->pc) +1);
			break;
		case '9':
			goto U;
		case 'n':
			N: ;
			setSpriteAToX(dungeon->ss, dungeon->pc, getSpriteAPX(dungeon->ss, dungeon->pc) +1);
			setSpriteAToY(dungeon->ss, dungeon->pc, getSpriteAPY(dungeon->ss, dungeon->pc) +1);
			break;
		case '3':
			goto N;
		case 'b':
			B: ;
			setSpriteAToX(dungeon->ss, dungeon->pc, getSpriteAPX(dungeon->ss, dungeon->pc) +1);
			setSpriteAToY(dungeon->ss, dungeon->pc, getSpriteAPY(dungeon->ss, dungeon->pc) -1);
			break;
		case '1':
			goto B;
		case '<':
			if(getSpriteAPX(dungeon->ss, 0) == getPosX(dungeon->su) && getSpriteAPY(dungeon->ss, 0) == getPosY(dungeon->su))
				*regen = TRUE;
			break;
		case '>':
			if(getSpriteAPX(dungeon->ss, 0) == getPosX(dungeon->sd) && getSpriteAPY(dungeon->ss, 0) == getPosY(dungeon->sd))
				*regen = TRUE;
			break;
		case '5':
			break;
		case ' ':
			break;
		case 'm':
			monster_list(dungeon);
			print_dungeon(dungeon, 0, 0);
			goto GCH;
		default:
			goto GCH;
	}

	if(dungeon->d[getSpriteAToY(dungeon->ss, dungeon->pc)][getSpriteAToX(dungeon->ss, dungeon->pc)].h > 0) {
		setSpriteAToX(dungeon->ss, dungeon->pc, getSpriteAPX(dungeon->ss, dungeon->pc));
		setSpriteAToY(dungeon->ss, dungeon->pc, getSpriteAPY(dungeon->ss, dungeon->pc));
	} else {
		setSpriteAPX(dungeon->ss, dungeon->pc, getSpriteAToX(dungeon->ss, dungeon->pc));
		setSpriteAPY(dungeon->ss, dungeon->pc, getSpriteAToY(dungeon->ss, dungeon->pc));
	}
	setSpriteAT(dungeon->ss, 0, getSpriteAT(dungeon->ss, 0) + (100 / getSpriteASS(dungeon->ss, 0)));
    int sn = 0;
    int i;
	for(i = 1; i < dungeon->ns; i++) {
		if(i != sn) {
			if((getSpriteAToX(dungeon->ss, i) == getSpriteAToX(dungeon->ss, sn)) && (getSpriteAToY(dungeon->ss, i) == getSpriteAToY(dungeon->ss, sn)) && getSpriteAA(dungeon->ss, sn) == TRUE)
				setSpriteAA(dungeon->ss, i, FALSE);
        }
    }
}

int rolldie(std::string s)
{
	int b, n, d;
	char* str = new char [s.length()+1];
	std::strcpy(str, s.c_str());
	sscanf(str, "%d%*c%d%*c%d", &b, &n, &d);
	Dice* di = new Dice(b, n, d);
	return di->roll();
}

Dice* getdie(std::string s)
{
	int b, n, d;
	char* str = new char [s.length()+1];
	std::strcpy(str, s.c_str());
	sscanf(str, "%d%*c%d%*c%d", &b, &n, &d);
	Dice* di = new Dice(b, n, d);
	return di;
}

int parsemonsters(Dungeon * dungeon) {
	char * env_path = getenv("HOME");
	char * path = (char*)calloc(strlen(env_path) + 50, sizeof(char));
	strcpy(path, env_path);
	strcat(path, "/.rlg327/monster_desc.txt");
	
	int dm = 0;
	bool first = true;
	dungeon->mm = 100;
	vector <Monster> mons;
	
	
	string line;
	ifstream md(path);
	if(md.is_open()) {
		Monster b_mo;
		Monster mo;
		while(getline(md, line)) {
			if(first) {
				if(line != "RLG327 MONSTER DESCRIPTION 1") {
					cout << "Invalid file head!" << endl;
					return -2;
				}
				first = false;
			} else {
				size_t n = 0;
				if(line == "BEGIN MONSTER")
					mo = b_mo;
				else if((n = line.find("NAME")) != std::string::npos)
					mo.n = line.substr(5, 77);
				else if((n = line.find("ABIL")) != std::string::npos) {
					if((n = line.find("SMART")) != std::string::npos)
						mo.s.in = true;
					else
						mo.s.in = false;
					if((n = line.find("TELE")) != std::string::npos)
						mo.s.te = true;
					else
						mo.s.te = false;
					if((n = line.find("TUNNEL")) != std::string::npos)
						mo.s.tu = true;
					else
						mo.s.tu = false;
					if((n = line.find("ERRATIC")) != std::string::npos)
						mo.s.eb = true;
					else
						mo.s.eb = false;
					if((n = line.find("PASS")) != std::string::npos)
						mo.s.pa = true;
					else
						mo.s.pa = false;
						
					
				} else if((n = line.find("COLOR")) != std::string::npos) {
					if((n = line.find("RED")) != std::string::npos)
						mo.c = RED;
					else if((n = line.find("GREEN")) != std::string::npos)
						mo.c = GREEN;
					else if((n = line.find("BLUE")) != std::string::npos)
						mo.c = BLUE;
					else if((n = line.find("CYAN")) != std::string::npos)
						mo.c = CYAN;
					else if((n = line.find("YELLOW")) != std::string::npos)
						mo.c = YELLOW;
					else if((n = line.find("MAGENTA")) != std::string::npos)
						mo.c = MAGENTA;
					else if((n = line.find("WHITE")) != std::string::npos)
						mo.c = WHITE;
					else if((n = line.find("BLACK")) != std::string::npos)
						mo.c = BLACK;
					
				} else if((n = line.find("DAM")) != std::string::npos) {
					mo.s.a = getdie(line.substr(4, line.size()));
					
				} else if((n = line.find("DESC")) != std::string::npos) {
					vector <std::string> desc;
					
					while(getline(md, line)) {
						if(line == ".")
							break;

						desc.push_back(line.substr(0, 77));
					}
					
					mo.desc = new string[desc.size()];
					mo.dl = desc.size();
					
					int i = desc.size() -1;
					while(desc.size() > 0) {
						string s = desc.back();
						desc.pop_back();
						mo.desc[i] = s;
						i--;
					}
					
				} else if((n = line.find("SPEED")) != std::string::npos) { 
					mo.s.s = rolldie(line.substr(6, line.size()));
					
				} else if((n = line.find("HP")) != std::string::npos) { 
					mo.s.hp = rolldie(line.substr(3, line.size()));
					
				} else if(line == "END") {
						mons.push_back(mo);
						dm++;
				}
			}
			
		}
	}
	else
		return -1;
		
	dungeon->dm = dm;
	
	dungeon->md = new Monster[mons.size()];

	printf("SIZE MONS: %d\n", (int)mons.size());

	int i = 0;
	while(mons.size() > 0) {
		cout << "UNLOADED MONSTER" << endl;
		Monster tmp = mons.back();
		cout << "NAME HERE: " << tmp.n << endl;
		dungeon->md[i] = tmp;
		mons.pop_back();
		i++;
	}
	return 0;
}

int parseitems(Dungeon * dungeon) {
	char * env_path = getenv("HOME");
	char * path = (char*)calloc(strlen(env_path) + 50, sizeof(char));
	strcpy(path, env_path);
	strcat(path, "/.rlg327/object_desc.txt");
	
	int di = 0;
	bool first = true;
	dungeon->mi = 100;
	vector <Item> items;
	
	
	string line;
	ifstream od(path);
	if(od.is_open()) {
		Item b_it;
		Item it;
		while(getline(od, line)) {
			if(first) {
				if(line != "RLG327 OBJECT DESCRIPTION 1") {
					cout << "Invalid file head!" << endl;
					return -2;
				}
				first = false;
			} else {
				size_t n = 0;
				if(line == "BEGIN OBJECT")
					it = b_it;
				else if((n = line.find("NAME")) != std::string::npos)
					it.n = line.substr(5, 77);
				else if((n = line.find("TYPE")) != std::string::npos) {
					if((n = line.find("WEAPON")) != std::string::npos)
						it.t = WEAPON;
					else if((n = line.find("OFFHAND")) != std::string::npos)
						it.t = OFFHAND;
					else if((n = line.find("RANGED")) != std::string::npos)
						it.t = RANGED;
					else if((n = line.find("ARMOR")) != std::string::npos)
						it.t = ARMOR;
					else if((n = line.find("HELMET")) != std::string::npos)
						it.t = HELMET;
					else if((n = line.find("CLOAK")) != std::string::npos)
						it.t = CLOAK;
					else if((n = line.find("GLOVES")) != std::string::npos)
						it.t = GLOVES;
					else if((n = line.find("BOOTS")) != std::string::npos)
						it.t = BOOTS;
					else if((n = line.find("RING")) != std::string::npos)
						it.t = RING;
					else if((n = line.find("AMULET")) != std::string::npos)
						it.t = AMULET;
					else if((n = line.find("LIGHT")) != std::string::npos)
						it.t = LIGHT;
					else if((n = line.find("SCROLL")) != std::string::npos)
						it.t = SCROLL;
					else if((n = line.find("BOOK")) != std::string::npos)
						it.t = BOOK;
					else if((n = line.find("FLASK")) != std::string::npos)
						it.t = FLASK;
					else if((n = line.find("GOLD")) != std::string::npos)
						it.t = GOLD;
					else if((n = line.find("AMMUNITION")) != std::string::npos)
						it.t = AMMUNITION;
					else if((n = line.find("FOOD")) != std::string::npos)
						it.t = FOOD;
					else if((n = line.find("WAND")) != std::string::npos)
						it.t = WAND;
					else if((n = line.find("CONTAINER")) != std::string::npos)
						it.t = CONTAINER;
						
					
				} else if((n = line.find("COLOR")) != std::string::npos) {
					if((n = line.find("RED")) != std::string::npos)
						it.c = RED;
					else if((n = line.find("GREEN")) != std::string::npos)
						it.c = GREEN;
					else if((n = line.find("BLUE")) != std::string::npos)
						it.c = BLUE;
					else if((n = line.find("CYAN")) != std::string::npos)
						it.c = CYAN;
					else if((n = line.find("YELLOW")) != std::string::npos)
						it.c = YELLOW;
					else if((n = line.find("MAGENTA")) != std::string::npos)
						it.c = MAGENTA;
					else if((n = line.find("WHITE")) != std::string::npos)
						it.c = WHITE;
					else if((n = line.find("BLACK")) != std::string::npos)
						it.c = BLACK;
					
				} else if((n = line.find("WEIGHT")) != std::string::npos)
					it.w = rolldie(line.substr(7, line.size()));
				else if((n = line.find("HIT")) != std::string::npos)
					it.hib = rolldie(line.substr(4, line.size()));
				else if((n = line.find("DAM")) != std::string::npos) {
					it.d = getdie(line.substr(4, line.size()));
					
				} else if((n = line.find("ATTR")) != std::string::npos)
					it.sa = rolldie(line.substr(5, line.size()));
				else if((n = line.find("VAL")) != std::string::npos)
					it.v = rolldie(line.substr(4, line.size()));
				else if((n = line.find("DODGE")) != std::string::npos)
					it.dob = rolldie(line.substr(6, line.size()));
				else if((n = line.find("DEF")) != std::string::npos)
					it.deb = rolldie(line.substr(4, line.size()));
				else if((n = line.find("SPEED")) != std::string::npos) {
					it.spb = rolldie(line.substr(6, line.size()));
				} else if((n = line.find("DESC")) != std::string::npos) {
					vector <std::string> desc;
					
					while(getline(od, line)) {
						if(line.find(".") == 0)
							break;

						desc.push_back(line.substr(0, 77));
					}
					
					it.desc = new string[desc.size()];
					it.dl = desc.size();
					
					int i = desc.size() -1;
					while(desc.size() > 0) {
						string s = desc.back();
						desc.pop_back();
						it.desc[i] = s;
						i--;
					}
					
				} else if((n = line.find("END")) != std::string::npos) {
					items.push_back(it);
					di++;
				}
			}
		}
	}
	else
		return -1;
		
	dungeon->di = di;
	
	dungeon->id = new Item[items.size()];
	
	int i = 0;
	while(items.size() > 0) {
		Item tmp = items.back();
		dungeon->id[i] = tmp;
		items.pop_back();
		i++;
	}
	
	return 0;
}

void printmds(Dungeon * dungeon)
{
	int i;
	for(i = 0; i < dungeon->dm; i++)
	{
		cout << "Name: " << dungeon->md[i].n << endl;
		
		cout << "Description: " << endl;
		int j;
		for(j = 0; j < dungeon->md[i].dl; j++)
		{
			cout << dungeon->md[i].desc[j] << endl;
		}
		
		switch(dungeon->md[i].c)
		{
		case RED: cout << "Color: RED" << endl; break;
		case GREEN: cout << "Color: GREEN" << endl; break;
		case BLUE: cout << "Color: BLUE" << endl; break;
		case CYAN: cout << "Color: CYAN" << endl; break;
		case YELLOW: cout << "Color: YELLOW" << endl; break;
		case MAGENTA: cout << "Color: MAGENTA" << endl; break;
		case WHITE: cout << "Color: WHITE" << endl; break;
		case BLACK: cout << "Color: BLACK" << endl; break;
		}
		
		printf("Speed: %d\n", dungeon->md[i].s.s);
		
		cout << "Abilities: ";
		if(dungeon->md[i].s.in)
			cout << "SMART ";
		if(dungeon->md[i].s.te)
			cout << "TELE ";
		if(dungeon->md[i].s.tu)
			cout << "TUNNEL ";
		if(dungeon->md[i].s.eb)
			cout << "ERRATIC ";
		if(dungeon->md[i].s.pa)
			cout << "PASS ";
		cout << endl;
		
		printf("HP: %d\n", dungeon->md[i].s.hp);
		
		cout << "Damage: " << dungeon->md[i].s.a->string() << endl;

		cout << endl;
	}
}

void printids(Dungeon * dungeon)
{
	int i;
	for(i = 0; i < dungeon->di; i++)
	{
		cout << "Name: " << dungeon->id[i].n << endl;
		
		cout << "Description: " << endl;
		int j;
		for(j = 0; j < dungeon->id[i].dl; j++)
		{
			cout << dungeon->id[i].desc[j] << endl;
		}
		
		cout << "Type: ";
		switch(dungeon->id[i].t)
		{
		case WEAPON: cout << "WEAPON"; break;
		case OFFHAND: cout << "OFFHAND"; break;
		case RANGED: cout << "RANGED"; break;
		case ARMOR: cout << "ARMOR"; break;
		case HELMET: cout << "HELMET"; break;
		case CLOAK: cout << "CLOAK"; break;
		case GLOVES: cout << "GLOVES"; break;
		case BOOTS: cout << "BOOTS"; break;
		case RING: cout << "RING"; break;
		case AMULET: cout << "AMULET"; break;
		case LIGHT: cout << "LIGHT"; break;
		case SCROLL: cout << "SCROLL"; break;
		case BOOK: cout << "BOOK"; break;
		case FLASK: cout << "FLASK"; break;
		case GOLD: cout << "GOLD"; break;
		case AMMUNITION: cout << "AMMUNITION"; break;
		case FOOD: cout << "FOOD"; break;
		case WAND: cout << "WAND"; break;
		case CONTAINER: cout << "CONTAINER"; break;
		default: break;
		}
		cout << endl;
		
		switch(dungeon->id[i].c)
		{
		case RED: cout << "Color: RED" << endl; break;
		case GREEN: cout << "Color: GREEN" << endl; break;
		case BLUE: cout << "Color: BLUE" << endl; break;
		case CYAN: cout << "Color: CYAN" << endl; break;
		case YELLOW: cout << "Color: YELLOW" << endl; break;
		case MAGENTA: cout << "Color: MAGENTA" << endl; break;
		case WHITE: cout << "Color: WHITE" << endl; break;
		case BLACK: cout << "Color: BLACK" << endl; break;
		}
		
		printf("Hit bonus: %d\n", dungeon->id[i].hib);
		
		cout << "Damage bonus: " << dungeon->id[i].d->string() << endl;
		
		printf("Dodge bonus: %d\n", dungeon->id[i].dob);
		
		printf("Defense bonus: %d\n", dungeon->id[i].deb);
		
		printf("Weight: %d\n", dungeon->id[i].w);
		
		printf("Speed bonus: %d\n", dungeon->id[i].spb);
		
		printf("Special attribute: %d\n", dungeon->id[i].sa);
		
		printf("Value in Pesos de Ocho: %d\n", dungeon->id[i].v);
		
		cout << endl;
	}
}

int main(int argc, char * argv[]) {
	int max_args = 8;
	int saving = FALSE;
	int loading = FALSE;
	int pathing = FALSE;
	int nnc = FALSE;
	int num_mon = 1;
	int custom_path = 0;
	if(argc > 2 && argc <= max_args) {
		int i;
		for(i = 1; i < argc; i++) {
			test_args(argc, argv, i, &saving, &loading, &pathing, &custom_path, &num_mon, &nnc);
		}
	} else if(argc == 2) {
		test_args(argc, argv, 1, &saving, &loading, &pathing, &custom_path, &num_mon, &nnc);
	} else if(argc > max_args) {
		printf("Too many arguments!\n");
	} else {
	}
	srand(time(NULL));

	char * env_path = getenv("HOME");
	char * path = (char*)calloc(strlen(env_path) + 50, sizeof(char));
	strcpy(path, env_path);
	strcat(path, "/.rlg327");
	if(pathing == TRUE) {
		strcat(path, "/");
		strcat(path, argv[custom_path]);
	} else {
		strcat(path, "/dungeon");
	}
	
	Bool regen = FALSE;
	Sprite * p_pc = initSprite();

	DUNGEN: ;

	Dungeon dungeon = init_dungeon(21, 80, 12);
	
	parsemonsters(&dungeon);
	parseitems(&dungeon);
	printmds(&dungeon);
	printids(&dungeon);
	return 0;

	if(loading == FALSE) {
		gen_dungeon(&dungeon);
		gen_corridors(&dungeon);
	} else {
		read_dungeon(&dungeon, path);
	}
	Sprite * pc = gen_sprite(&dungeon, '@', -1, -1, 1);
	add_sprite(&dungeon, pc);

	int i;
	for(i = 0; i < num_mon; i++) {
		Sprite * m = gen_sprite(&dungeon,'m' , -1, -1, 1);
		setSpriteSn(m, i);
		add_sprite(&dungeon, m);
	}

	map_dungeon_nont(&dungeon);
	map_dungeon_t(&dungeon);

	if(regen == TRUE) {
		int px = getSpriteAPX(dungeon.ss, 0);
		int py = getSpriteAPY(dungeon.ss, 0);
		copyASprite(dungeon.ss, 0, p_pc);
		setSpriteAPX(dungeon.ss, 0, px);
		setSpriteAPY(dungeon.ss, 0, py);
		setSpriteAToX(dungeon.ss, 0, px);
		setSpriteAToY(dungeon.ss, 0, py);
	}


	for(i = 1; i < dungeon.ns; i++) {
		gen_move_sprite(&dungeon, i);
	}

	if(regen == TRUE)
		goto PNC;


	void (*printer)(Dungeon*, int, int);
	if(nnc == FALSE) {
		printer = &print_dungeon;
		initscr();
		raw();
		noecho();
		curs_set(0);
		set_escdelay(25);
		keypad(stdscr, TRUE);
	} else {
		printer = &print_dungeon_nnc;
	}

	PNC: ;
	regen = FALSE;


	print_dungeon(&dungeon, 0, 0);
	Bool first = TRUE;
	Bool run = TRUE;
	while(run == TRUE) {
		int l = 0;
		for(i = 0; i < dungeon.ns; i++) {
			if(getSpriteAT(dungeon.ss, i) < getSpriteAT(dungeon.ss, l)) {
				l = i;
			}
		}


		if(l == dungeon.pc || first == TRUE) {
			parse_pc(&dungeon, &run, &regen);
			if(regen == TRUE) {
				copySprite(p_pc, thisASprite(dungeon.ss, 0));
				goto DUNFREE;
			}
			map_dungeon_nont(&dungeon);
			map_dungeon_t(&dungeon);

			int sn = 0;
			for(i = 1; i < dungeon.ns; i++) {
				if(i != sn) {
					if((getSpriteAPX(dungeon.ss, i) == getSpriteAPX(dungeon.ss, sn)) && (getSpriteAPY(dungeon.ss, i) == getSpriteAPY(dungeon.ss, sn)) && getSpriteAA(dungeon.ss, sn) == TRUE)
						setSpriteAA(dungeon.ss, i, FALSE);
				}
			}

			print_dungeon(&dungeon, 0, 0);
		} else {
			parse_move(&dungeon, l);
			gen_move_sprite(&dungeon, l);
		}
		refresh();
		if(dungeon.go == TRUE || getSpriteAA(dungeon.ss, dungeon.pc) == FALSE)
			break;

		Bool any = check_any_monsters(&dungeon);
		if(any == FALSE) {
			goto END;
		}
		first = FALSE;
	}
	printer(&dungeon, 0, 0);
	END: ;
	delwin(stdscr);
	endwin();

	if(saving == TRUE) {
		write_dungeon(&dungeon, path);
	}

	DUNFREE: ;
	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.d[i]);
	}
	free(dungeon.d);
	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.p[i]);
	}
	free(dungeon.p);
	free(dungeon.r);
	free(dungeon.ss);
	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.csnt[i]);
	}
	free(dungeon.csnt);
	for(i = 0; i < dungeon.h; i++) {
		free(dungeon.cst[i]);
	}
	free(dungeon.cst);

	if(regen == TRUE)
		goto DUNGEN;

	free(path);
	return 0;
}
