#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include "binheap.h"
#include "dungeon_generator.h"

void print_t_heatmap(Dungeon * dungeon) {
	int i;
	int j;
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			int c = dungeon->cst[i][j];
			if(c >= 0 && c < 10) {
				printf("%d", c);
			} else if(c >= 10 && c < 36) {
				printf("%c", 'a' + (c - 10));
			} else if(c >= 36 && c < 62) {
				printf("%c", 'A' + (c - 36));
			} else {
				printf("%c", dungeon->d[i][j].c);
			}
		}
		printf("\n");
	}

}

void print_nont_heatmap(Dungeon * dungeon) {
	int i;
	int j;
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			int c = dungeon->csnt[i][j];
			if(c >= 0 && c < 10) {
				printf("%d", c);
			} else if(c >= 10 && c < 36) {
				printf("%c", 'a' + (c - 10));
			} else if(c >= 36 && c < 62) {
				printf("%c", 'A' + (c - 36));
			} else {
				printf("%c", dungeon->d[i][j].c);
			}
		}
		printf("\n");
	}

}

int compare_int(const void *key, const void *with) {
	return (const int) ((*(Tile_Node *) key).cost - (*(Tile_Node *) with).cost);
}

int compare_move(const void *key, const void *with) {
	Sprite k = *(Sprite *) key;
	Sprite w = *(Sprite *) with;

	return k.t-w.t;
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

	int px = dungeon->ss[dungeon->pc].p.x;
	int py = dungeon->ss[dungeon->pc].p.y;
	tiles[py][px].cost = 0;
	tiles[py][px].v = TRUE;
	binheap_insert(&h, &tiles[py][px]);


	binheap_node_t	*p;

	while((p = binheap_remove_min(&h))) {
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

	int px = dungeon->ss[dungeon->pc].p.x;
	int py = dungeon->ss[dungeon->pc].p.y;
	tiles[py][px].cost = 0;
	tiles[py][px].v = TRUE;
	binheap_insert(&h, &tiles[py][px]);


	binheap_node_t	*p;

	while((p = binheap_remove_min(&h))) {
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
	dungeon->r = calloc(room_count, sizeof(Room));
	int pos;
	for(pos = 1694; pos < size; pos += 4) {
		int x_8;
		int w_8;
		int y_8;
		int h_8;
		fread(&x_8, sizeof(int8_t), 1, file);
		fread(&w_8, sizeof(int8_t), 1, file);
		fread(&y_8, sizeof(int8_t), 1, file);
		fread(&h_8, sizeof(int8_t), 1, file);

		dungeon->r[room_i].tl.x = (int8_t) x_8;
		dungeon->r[room_i].w = (int8_t) w_8;
		dungeon->r[room_i].tl.y = (int8_t) y_8;
		dungeon->r[room_i].h = (int8_t) h_8;
		dungeon->r[room_i].br.x = ((int8_t) x_8) + dungeon->r[room_i].w-1;
		dungeon->r[room_i].br.y = ((int8_t) y_8) + dungeon->r[room_i].h-1;



		room_i++;
	}



	int h;
	for(h = 0; h < dungeon->nr; h++) {
		for(i = dungeon->r[h].tl.y; i < dungeon->r[h].br.y+1; i++) {
			for(j = dungeon->r[h].tl.x; j < dungeon->r[h].br.x+1; j++) {
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
	fdir_path = calloc(strlen(env_home) + 9, sizeof(char));
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
		int8_t x = (int8_t) dungeon->r[i].tl.x;
		int8_t w = (int8_t) dungeon->r[i].w;
		int8_t y = (int8_t) dungeon->r[i].tl.y;
		int8_t h = (int8_t) dungeon->r[i].h;

		fwrite(&x, sizeof(int8_t), 1, file);
		fwrite(&w, sizeof(int8_t), 1, file);
		fwrite(&y, sizeof(int8_t), 1, file);
		fwrite(&h, sizeof(int8_t), 1, file);
	}

	free(fdir_path);
	fclose(file);
}

void print_dungeon(Dungeon * dungeon, int nt, int t) {
	int i;
	int j;
	int h;

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->p[i][j].c = ' ';
		}
	}

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			if(dungeon->d[i][j].p == 1 || dungeon->d[i][j].c == '#' || dungeon->d[i][j].h == 0) {
				dungeon->p[i][j].c = '#';
			}
		}
	}

	for(h = 0; h < dungeon->nr; h++) {
		for(i = dungeon->r[h].tl.y; i < dungeon->r[h].br.y+1; i++) {
			for(j = dungeon->r[h].tl.x; j < dungeon->r[h].br.x+1; j++) {
				dungeon->p[i][j].c = '.';
			}
		}
	}

	for(i = 0; i < dungeon->ns; i++) {
		if(dungeon->ss[i].a == TRUE)
			dungeon->p[dungeon->ss[i].p.y][dungeon->ss[i].p.x].c = dungeon->ss[i].c;
	}

	if(nt > 0) {
		for(i = 0; i < dungeon->h; i++) {
			for(j = 0; j < dungeon->w; j++) {
				if(dungeon->d[i][j].h == 0) {
					int c = dungeon->csnt[i][j];
					if(c >= 0 && c < 10) {
						dungeon->p[i][j].c = '0' + c;
					} else if(c >= 10 && c < 36) {
						dungeon->p[i][j].c = 'a' + (c - 10);
					} else if(c >= 36 && c < 62) {
						dungeon->p[i][j].c = 'A' + (c - 36);
					}
				}
			}
		}
	}

	if(t > 0) {
		for(i = 0; i < dungeon->h; i++) {
			for(j = 0; j < dungeon->w; j++) {
				int c = dungeon->cst[i][j];
				if(c >= 0 && c < 10) {
					dungeon->p[i][j].c = '0' + c;
				} else if(c >= 10 && c < 36) {
					dungeon->p[i][j].c = 'a' + (c - 10);
				} else if(c >= 36 && c < 62) {
					dungeon->p[i][j].c = 'A' + (c - 36);
				}
			}
		}
	}

	for(i = 0; i < dungeon->h; i++) {
		int j;
		for(j = 0; j < dungeon->w; j++) {
			printf("%c", (dungeon->p[i][j]).c);
		}
		printf("\n");
	}
}


int place_room(Dungeon * dungeon) {
	int x = (rand() % (dungeon->w-1)) +1;
	int y = (rand() % (dungeon->h-1)) +1;
	Room new_room;
	new_room.tl.x = x;
	new_room.tl.y = y;

	HW: ;

	int we = (rand() % 4) + 4; 
	int he = (rand() % 4) + 3;

	if(we == he) {
		goto HW;
	}

	new_room.h = he;
	new_room.w = we;

	new_room.br.x = x + new_room.w-1;
	new_room.br.y = y + new_room.h-1;

	int i;
	int j;
	int placed = -1;
	int passed = 0;
	for(i = y; i < dungeon->h-1 && i < y+he; i++) {
		for(j = x; j < dungeon->w-1 && j < x+we; j++) {
			if(dungeon->p[i][j].c != '.') {
				passed++;
			}
		}
	}

	if(passed < we*he) {
		return placed; 
	}

	if(new_room.br.x >= dungeon->w || new_room.br.y >= dungeon->h) {
		return placed;
	}


	for(i = new_room.tl.x-1; i < new_room.br.x+2 && new_room.tl.x-1 >= 0 && new_room.br.x+1 < dungeon->w && new_room.tl.y-1 >= 0; i++) {
		if((dungeon->p[new_room.tl.y-1][i]).c == '.') {
			return placed;
		}
	}

	for(i = new_room.tl.x-1; i < new_room.br.x+2 && new_room.tl.x-1 >= 0 && new_room.br.x+1 < dungeon->w && new_room.br.y+1 < dungeon->h; i++) {
		if((dungeon->p[new_room.br.y+1][i]).c == '.') {
			return placed;
		}
	}

	for(i = new_room.tl.y; i < new_room.br.y+1 && new_room.br.y+1 < dungeon->h && new_room.tl.x-1 >= 0; i++) {
		if((dungeon->p[i][new_room.tl.x-1]).c == '.') {
			return placed;
		}
	}

	for(i = new_room.tl.y; i < new_room.br.y+1 && new_room.br.y+1 < dungeon->h && new_room.br.x+1 < dungeon->w; i++) {
		if((dungeon->p[i][new_room.br.x+1]).c == '.') {
			return placed;
		}
	}



	placed = 0;


	for(i = y; i < y+he; i++) {
		for(j = x; j < x+we; j++) {
			dungeon->p[i][j].c = '.';
			dungeon->d[i][j].h = 0;
		}
	}


	if(dungeon->nr < dungeon->mr) {
		dungeon->nr++;
		new_room.id = dungeon->nr-1; 
		new_room.ctr.x = (new_room.w)/2 + new_room.tl.x;
		new_room.ctr.y = (new_room.h)/2 + new_room.tl.y;
		dungeon->r[dungeon->nr-1] = new_room;
	} else {
		return -1;
	}


	return placed;
}

int all_connected(int * cnxns, Dungeon * dungeon) {
	int i;

	for(i = 0; i < dungeon->nr; i++) {
		if(cnxns[i] != 1 || dungeon->r[i].c != TRUE) {
			return FALSE;
		}
	}

	return TRUE;
}


void gen_corridors(Dungeon * dungeon) {
	int i;
	int connected[dungeon->nr];
	for(i = 0; i < dungeon->nr; i++) {
		connected[i] = 0;
	}
	double dists[dungeon->nr];
	for(i = 0; i < dungeon->nr; i++) {
		dists[i] = 0;
	}
	int max_paths = dungeon->nr * 3;
	Path paths[max_paths]; 
	int path_cnt = 0;
	int	room_pos = 0; 

	for(i = 0; i < dungeon->nr; i++) {
		dists[i] = -1; 
	}
	dists[0] = 0;

	for(i = 0; i < dungeon->nr; i++) {
		dungeon->r[i].c = FALSE;
	}

	
	while(all_connected(connected, dungeon) == FALSE && path_cnt < max_paths) {
		int i;
		double d;
		Path new_path;

		for(i = 0; i < dungeon->nr; i++) {
			d =  sqrt(pow(dungeon->r[i].ctr.x - dungeon->r[room_pos].ctr.x, 2) + pow(dungeon->r[i].ctr.y - dungeon->r[room_pos].ctr.y, 2));
			dists[i] = d;
		}


		int next = -1;
		for(i = 0; i < dungeon->nr; i++) {
			if(connected[i] != 1 && next == -1 && room_pos != i) {
				next = i;
			} else if(connected[i] != 1 && dists[i] < dists[next] && room_pos != i) {
				next = i;
			}
		}

		if(next != -1) {
			dungeon->r[room_pos].c = TRUE;
			dungeon->r[next].c = TRUE;
			connected[room_pos] = 1;
			new_path.prev = room_pos;
			new_path.next = next;
			paths[path_cnt] = new_path;
			room_pos = next;
			path_cnt++;
		} else {
			break;
		}

	}


	for(i = 0; i < path_cnt; i++) {
		int x = dungeon->r[paths[i].prev].ctr.x;
		int y = dungeon->r[paths[i].prev].ctr.y;


		while(x != dungeon->r[paths[i].next].ctr.x || y != dungeon->r[paths[i].next].ctr.y) {
			int dirx = 0; 
			int diry = 0; 

			if(x < dungeon->r[paths[i].next].ctr.x) {
				dirx = 1;
			} else if(x > dungeon->r[paths[i].next].ctr.x) {
				dirx = -1;
			}

			if(y < dungeon->r[paths[i].next].ctr.y) {
				diry = 1;
			} else if(y > dungeon->r[paths[i].next].ctr.y) {
				diry = -1;
			}

			dungeon->d[y][x].p = 1;
			if(dungeon->d[y][x].c != '.') {
				dungeon->d[y][x].c = '#';
				dungeon->d[y][x].h = 0;
			}

			if(dirx == -1) {
				x--;
			} else if(dirx == 1) {
				x++;
			} else if(diry == -1) {
				y--;
			} else if(diry == 1) {
				y++;
			}
		}

	}

}

void gen_dungeon(Dungeon * dungeon) {
	int i, j;

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			(dungeon->d[i][j]).c = ' ';	
			int h = (rand() % 254) + 1;
			(dungeon->d[i][j]).h = h;
		}
	}

	for(i = 0; i < dungeon->w; i++) {
		(dungeon->d[0][i]).h = 255;
	}
	for(i = 0; i < dungeon->w; i++) {
		(dungeon->d[dungeon->h-1][i]).h = 255;
	}
	for(i = 0; i < dungeon->h; i++) {
		(dungeon->d[i][0]).h = 255;
	}
	for(i = 0; i < dungeon->h; i++) {
		(dungeon->d[i][dungeon->w-1]).h = 255;
	}

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->p[i][j] = dungeon->d[i][j];
		}
	}

	int cnt = 0;
	int tst = 0;
	for(i = 0; dungeon->nr < dungeon->mr && cnt < 2000; i++) {
		tst = place_room(dungeon);
		if(tst < 0) {
			cnt++;
		}
	}


}

Dungeon init_dungeon(int h, int w, int mr) {
	Dungeon new_dungeon;
	new_dungeon.h	= h;
	new_dungeon.w	= w;
	new_dungeon.mr	= mr;
	new_dungeon.nr	= 0;
	new_dungeon.ns	= 0;
	new_dungeon.ms	= w*h; 
	new_dungeon.t	= 0;
	new_dungeon.go	= FALSE;

	new_dungeon.d = calloc(new_dungeon.h, sizeof(Tile *));

	int i;
	for(i = 0; i < new_dungeon.h; i++) {
		new_dungeon.d[i] = calloc(new_dungeon.w, sizeof(Tile));
	}

	new_dungeon.p = calloc(new_dungeon.h, sizeof(Tile *));

	for(i = 0; i < new_dungeon.h; i++) {
		new_dungeon.p[i] = calloc(new_dungeon.w, sizeof(Tile));
	}

	new_dungeon.r = calloc(new_dungeon.mr, sizeof(Room));

	new_dungeon.ss = calloc(new_dungeon.ms, sizeof(Sprite));

	new_dungeon.cst = calloc(w*h, sizeof(int *));
	for(i = 0; i < new_dungeon.h; i++) {
		new_dungeon.cst[i] = calloc(new_dungeon.w, sizeof(int));
	}

	new_dungeon.csnt = calloc(w*h, sizeof(int *));
	for(i = 0; i < new_dungeon.h; i++) {
		new_dungeon.csnt[i] = calloc(new_dungeon.w, sizeof(int));
	}

	return new_dungeon;
}

void test_args(int argc, char ** argv, int this, int * s, int * l, int *p, int *cp, int *nm) {
		if(strcmp(argv[this], "--save") == 0) {
			*s = TRUE;
		} else if(strcmp(argv[this], "--load") == 0) {
			*l = TRUE;
		} else if(strcmp(argv[this], "-f") == 0) {
			*p = TRUE;
			*cp = this+1;
			if(this+1 > argc-1) {
				printf("Invalid filename argument!\n");
				*p = FALSE;
			}
		} else if(strcmp(argv[this], "--nummon") == 0) {
			*nm = atoi(argv[this+1]);
		}
}



int main(int argc, char * argv[]) {
	
	int max_args = 7;
	int saving = FALSE;
	int loading = FALSE;
	int pathing = FALSE;
	int num_mon = 1;
	int custom_path = 0;
	if(argc > 2 && argc <= max_args) {
		int i;
		for(i = 1; i < argc; i++) {
			test_args(argc, argv, i, &saving, &loading, &pathing, &custom_path, &num_mon);
		}
	} else if(argc == 2) {
		test_args(argc, argv, 1, &saving, &loading, &pathing, &custom_path, &num_mon);
	} else if(argc > max_args) {
		printf("Too many arguments!\n");
	}

	srand(time(NULL));
	char * env_path = getenv("HOME");
	char * path = calloc(strlen(env_path) + 50, sizeof(char));
	strcpy(path, env_path);
	strcat(path, "/.rlg327");
	if(pathing == TRUE) {
		strcat(path, "/");
		strcat(path, argv[custom_path]);
	} else {
		strcat(path, "/dungeon");
	}

	Dungeon dungeon = init_dungeon(21, 80, 12);

	if(loading == FALSE) {
		gen_dungeon(&dungeon);
		gen_corridors(&dungeon);
	} else {
		read_dungeon(&dungeon, path);
	}
	Sprite pc = gen_sprite(&dungeon, '@', -1, -1, 1);
	add_sprite(&dungeon, pc);

	int i;
	for(i = 0; i < num_mon; i++) {
		Sprite m = gen_sprite(&dungeon,'m' , -1, -1, 1);
		m.sn = i;
		add_sprite(&dungeon, m);
	}

	map_dungeon_nont(&dungeon);
	map_dungeon_t(&dungeon);

	for(i = 0; i < dungeon.ns; i++) {
		gen_move_sprite(&dungeon, i);
	}


	bool first = TRUE;
	bool run = TRUE;
	while(run == TRUE) {
		int l = 0;
		for(i = 0; i < dungeon.ns; i++) {
			if(dungeon.ss[i].t < dungeon.ss[l].t) {
				l = i;
			}
		}
		parse_move(&dungeon, l);
		gen_move_sprite(&dungeon, l);
		if(l == dungeon.pc || first == TRUE) {
			map_dungeon_nont(&dungeon);
			map_dungeon_t(&dungeon);
			print_dungeon(&dungeon, 0, 0);
			sleep(2);
		}
		if(dungeon.go == TRUE || dungeon.ss[dungeon.pc].a == FALSE)
			break;

		bool any = check_any_monsters(&dungeon);
		if(any == FALSE) {
			printf("You win!\n");
			goto END;
		}
		first = FALSE;
	}
	print_dungeon(&dungeon, 0, 0);
	printf("Game Over!\n");

	END: ;
	if(saving == TRUE) {
		write_dungeon(&dungeon, path);
	}

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
	free(path);
	return 0;
}
