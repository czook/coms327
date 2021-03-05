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

/* print hardness */
void print_hardnesS(Dungeon * dungeon) {
}

/* prints heatmap */
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

/* prints heatmap */
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

/* compare two ints used as costs ;; 0 if same, <0 if higher than key; >0 if lower than key */
int compare_int(const void *key, const void *with) {
	//printf("%d\n", *(const int *) key);
	return (const int) ((*(Tile_Node *) key).cost - (*(Tile_Node *) with).cost);
}

/* compare movement turns/priority for turns */
int compare_move(const void *key, const void *with) {
	Sprite k = *(Sprite *) key;
	Sprite w = *(Sprite *) with;

	return k.t-w.t;
	/*
	if(k.t < w.t)
		return -1;
	if(k.t > w.t)
		return 1;

	return 0;*/
}

/* returns the hardness cost of an int hardness */
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

/* djikstra's take 2; with tunnelling */
void map_dungeon_t(Dungeon * dungeon) {
	binheap_t h;
	Tile_Node tiles[dungeon->h][dungeon->w];

	binheap_init(&h, compare_int, NULL);

	/* starts from top left */
	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	int i;
	int j;

	/* set all indices and insert the default values */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			tiles[i][j].y = i;
			tiles[i][j].x = j;
			tiles[i][j].cost = INT_MAX;
			tiles[i][j].v = FALSE;
		}
	}

	/* set the player's cost as 0: */
	int px = dungeon->ss[dungeon->pc].p.x;
	int py = dungeon->ss[dungeon->pc].p.y;
	tiles[py][px].cost = 0;
	tiles[py][px].v = TRUE;
	binheap_insert(&h, &tiles[py][px]);

	/* primary cost calculation logic */

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

	/* copy the heatmap to the dungeon */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->cst[i][j] = tiles[i][j].cost;
		}
	}


	/* clean up the heap */
	binheap_delete(&h);
}

/* djikstra's take 2 */
void map_dungeon_nont(Dungeon * dungeon) {
	binheap_t h;
	Tile_Node tiles[dungeon->h][dungeon->w];

	binheap_init(&h, compare_int, NULL);

	/* starts from top left */
	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	int i;
	int j;

	/* set all indices and insert the default values */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			tiles[i][j].y = i;
			tiles[i][j].x = j;
			tiles[i][j].cost = INT_MAX;
			tiles[i][j].v = FALSE;
		}
	}

	/* set the player's cost as 0: */
	int px = dungeon->ss[dungeon->pc].p.x;
	int py = dungeon->ss[dungeon->pc].p.y;
	tiles[py][px].cost = 0;
	tiles[py][px].v = TRUE;
	binheap_insert(&h, &tiles[py][px]);

	/* primary cost calculation logic */

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

	/* copy the heatmap to the dungeon */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->csnt[i][j] = tiles[i][j].cost;
		}
	}


	/* clean up the heap */
	binheap_delete(&h);
}

/* reads from a dungeon file */
void read_dungeon(Dungeon * dungeon, char * path) {
	FILE * file;
	file = fopen(path, "rb+");
	if(file == NULL) {
		fprintf(stderr, "FILE ERROR: Could not open dungeon file at %s! read_dungeon()\n", path);
        exit(1);
	}

	/* read the file-type marker */
	fseek(file, 0, SEEK_SET);
	char marker[6];
	fread(marker, 1, 6, file);

	/* read the file version marker */
	fseek(file, 6, SEEK_SET);
	uint32_t file_version;
	uint32_t file_version_be;
	fread(&file_version_be, sizeof(uint32_t), 1, file);
	file_version = be32toh(file_version_be);
	dungeon->v = file_version;

	/* read the size of file */
	fseek(file, 10, SEEK_SET);
	uint32_t size;
	uint32_t size_be;
	fread(&size_be, sizeof(uint32_t), 1, file);
	size = be32toh(size_be);
	dungeon->s = size;

	/* read the hardness values in */
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

	/* read in rooms in dungeon */
	fseek(file, 1694, SEEK_SET);
	/* might want to make this just counted in 4's by the loop below, but w/e, math, amirite? */
	int room_i = 0;
	int room_count = (size - 1693) / 4;
	dungeon->nr = room_count;
	dungeon->r = calloc(room_count, sizeof(Room));
	/* could probably be replaced with a getpos() call for complete-ness */
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


	/* populate the rooms and corridors if not in rooms */
	/* add rooms to the dungeon buffer */
	int h;
	for(h = 0; h < dungeon->nr; h++) {
		for(i = dungeon->r[h].tl.y; i < dungeon->r[h].br.y+1; i++) {
			for(j = dungeon->r[h].tl.x; j < dungeon->r[h].br.x+1; j++) {
				dungeon->d[i][j].c = '.';
			}
		}
	}

	/* add corridors to the dungeon buffer */
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

/* writes the dungeon file to ~/.rlg327/dungeon */
void write_dungeon(Dungeon * dungeon, char * path) {
	FILE * file;

	/* folder creation logic */
	char * env_home = getenv("HOME");
	char * fdir_path;
	fdir_path = calloc(strlen(env_home) + 9, sizeof(char));
	strcpy(fdir_path, env_home);
	strcat(fdir_path, "/.rlg327");
	mkdir(fdir_path, S_IRWXU);
	/* mkdir will return -1 when it fails, but it will fail if the file exists so it doesn't especially matter to catch it as no output would be provided */


	file = fopen(path, "wb+");
	if(file == NULL) {
		fprintf(stderr, "FILE ERROR: Could not open dungeon file at %s! write_dungeon()\n", path);
        exit(1);
	}

	/* write the file-type marker */
	fseek(file, 0, SEEK_SET);
	char marker[7];
	strcpy(marker, "RLG327");
	fwrite(marker, sizeof(char), 6, file);

	/* write the file version marker */
	fseek(file, 6, SEEK_SET);
	uint32_t file_version = 0;
	uint32_t file_version_be = htobe32(file_version);
	fwrite(&file_version_be, sizeof(uint32_t), 1, file);

	/* write the size of the file ;; unsure how to properly calculate */
	fseek(file, 10, SEEK_SET);
 	uint32_t size = 1693 + (4 * dungeon->nr);
	uint32_t size_be = htobe32(size);
	fwrite(&size_be, sizeof(uint32_t), 1, file);

	/* row-major dungeon matrix */
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

	/* room positions ;; 4 bytes per room */
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

/* prints the dungeon */
void print_dungeon(Dungeon * dungeon, int nt, int t) {
	int i;
	int j;
	int h;

	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->p[i][j].c = ' ';
		}
	}

	/* add corridors to the print buffer */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			if(dungeon->d[i][j].p == 1 || dungeon->d[i][j].c == '#' || dungeon->d[i][j].h == 0) {
				dungeon->p[i][j].c = '#';
			}
		}
	}

	/* add rooms to the print buffer */
	for(h = 0; h < dungeon->nr; h++) {
		for(i = dungeon->r[h].tl.y; i < dungeon->r[h].br.y+1; i++) {
			for(j = dungeon->r[h].tl.x; j < dungeon->r[h].br.x+1; j++) {
				dungeon->p[i][j].c = '.';
			}
		}
	}

	/* add sprites to the print buffer */
	for(i = 0; i < dungeon->ns; i++) {
		//printf("%d, %d: %c speed: %d turn: %d\n", dungeon->ss[i].p.y, dungeon->ss[i].p.x, dungeon->ss[i].c, dungeon->ss[i].s.s, dungeon->ss[i].t);
		if(dungeon->ss[i].a == TRUE)
			dungeon->p[dungeon->ss[i].p.y][dungeon->ss[i].p.x].c = dungeon->ss[i].c;
	}

	/* print non-tunnelling dijkstra's */
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

	/* print tunnelling dijkstra's */
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

	/* print the print buffer */
	for(i = 0; i < dungeon->h; i++) {
		int j;
		for(j = 0; j < dungeon->w; j++) {
			printf("%c", (dungeon->p[i][j]).c);
		}
		printf("\n");
	}
}

/* (attempt to) place a room within a given dungeon */
int place_room(Dungeon * dungeon) {
	int x = (rand() % (dungeon->w-1)) +1;
	int y = (rand() % (dungeon->h-1)) +1;
	Room new_room;
	/*
	set top right to rng number; might be worth making a more detailed placer with a lower
		fail rate
	*/
	new_room.tl.x = x;
	new_room.tl.y = y;
	/* for RNG, maybe do a rando room width/height and re-set .br */

	HW: ;

	int we = (rand() % 4) + 4; /* width, expanded, up to 4 more */
	int he = (rand() % 4) + 3; /* height, expanded, up to 4 more */

	if(we == he) {
		/* if we have a square, re-generate */
		goto HW;
	}

	new_room.h = he;
	new_room.w = we;

	new_room.br.x = x + new_room.w-1;
	new_room.br.y = y + new_room.h-1;

	/* check for rooms loaded into the dungeon buffer already */
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

	/* return a failure if not all cells within the "Room" passed */
	if(passed < we*he) {
		return placed; /* should be -1 */
	}

	/* return a failure if part of the room is out of bounds */
	if(new_room.br.x >= dungeon->w || new_room.br.y >= dungeon->h) {
		return placed;
	}


	/* check for surrounding rooms */

	/* top row */
	for(i = new_room.tl.x-1; i < new_room.br.x+2 && new_room.tl.x-1 >= 0 && new_room.br.x+1 < dungeon->w && new_room.tl.y-1 >= 0; i++) {
		if((dungeon->p[new_room.tl.y-1][i]).c == '.') {
			return placed;
		}
	}

	/* bottom row */
	for(i = new_room.tl.x-1; i < new_room.br.x+2 && new_room.tl.x-1 >= 0 && new_room.br.x+1 < dungeon->w && new_room.br.y+1 < dungeon->h; i++) {
		if((dungeon->p[new_room.br.y+1][i]).c == '.') {
			return placed;
		}
	}

	/* left side */
	for(i = new_room.tl.y; i < new_room.br.y+1 && new_room.br.y+1 < dungeon->h && new_room.tl.x-1 >= 0; i++) {
		if((dungeon->p[i][new_room.tl.x-1]).c == '.') {
			return placed;
		}
	}

	/* right side */
	for(i = new_room.tl.y; i < new_room.br.y+1 && new_room.br.y+1 < dungeon->h && new_room.br.x+1 < dungeon->w; i++) {
		if((dungeon->p[i][new_room.br.x+1]).c == '.') {
			return placed;
		}
	}


	/* successful placement */
	placed = 0;

	/* fill the room into the dungeon buffer and add to room array */
	for(i = y; i < y+he; i++) {
		for(j = x; j < x+we; j++) {
			dungeon->p[i][j].c = '.';
			dungeon->d[i][j].h = 0;
		}
	}


	if(dungeon->nr < dungeon->mr) {
		dungeon->nr++;
		new_room.id = dungeon->nr-1; /* reflects position in the array */
		new_room.ctr.x = (new_room.w)/2 + new_room.tl.x;
		new_room.ctr.y = (new_room.h)/2 + new_room.tl.y;
		/* printf("%d: (%d, %d)\n", new_room.id, new_room.ctr.x, new_room.ctr.y); */
		dungeon->r[dungeon->nr-1] = new_room;
	} else {
		return -1;
	}


	return placed;
}

/* assistant function for gen_corridors() to check if all rooms are connected */
int all_connected(int * cnxns, Dungeon * dungeon) {
	int i;

	for(i = 0; i < dungeon->nr; i++) {
		if(cnxns[i] != 1 || dungeon->r[i].c != TRUE) {
			return FALSE;
		}
	}

	return TRUE;
}

/* generates and marks corridors */
void gen_corridors(Dungeon * dungeon) {
	int i;
	int connected[dungeon->nr];
	for(i = 0; i < dungeon->nr; i++) {
		connected[i] = 0;
	}
	//memset(connected, 0, dungeon->nr * sizeof(int));
	double dists[dungeon->nr];
	for(i = 0; i < dungeon->nr; i++) {
		dists[i] = 0;
	}
	//memset(dists, 0.0, dungeon->nr * sizeof(double));
	int max_paths = dungeon->nr * 3;
	Path paths[max_paths]; /* max paths is 3 * number of rooms */
	int path_cnt = 0;
	int	room_pos = 0; /* current room in use */

	for(i = 0; i < dungeon->nr; i++) {
		dists[i] = -1; /* infinite at -1 */
	}
	dists[0] = 0;

	/* ensure all rooms are disconnected */
	for(i = 0; i < dungeon->nr; i++) {
		dungeon->r[i].c = FALSE;
	}

	/* primary loop, goal is to connect all rooms; 0 means true */
	while(all_connected(connected, dungeon) == FALSE && path_cnt < max_paths) {
		int i;
		double d;
		Path new_path;

		/* populate dists from the current position */
		for(i = 0; i < dungeon->nr; i++) {
			/* calculate distance */
			d =  sqrt(pow(dungeon->r[i].ctr.x - dungeon->r[room_pos].ctr.x, 2) + pow(dungeon->r[i].ctr.y - dungeon->r[room_pos].ctr.y, 2));
			dists[i] = d;
		}

		/* find the room to path to ;; if not connected already and the distance is shorter and isn't our current position */

		int next = -1;
		for(i = 0; i < dungeon->nr; i++) {
			if(connected[i] != 1 && next == -1 && room_pos != i) {
				next = i;
			} else if(connected[i] != 1 && dists[i] < dists[next] && room_pos != i) {
				next = i;
			}
		}

		/** this would - in the future - be the point of adding extraneous paths **/
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

	/* populate the dungeon grid (draw the paths using x/y chasing/pathing) */

	/* draw dungeon paths in the dungeon grid; start at room 0 as per above */

	for(i = 0; i < path_cnt; i++) {
		int x = dungeon->r[paths[i].prev].ctr.x;
		int y = dungeon->r[paths[i].prev].ctr.y;

		/*printf("%d: (%d, %d)\n", i, x, y);*/

		while(x != dungeon->r[paths[i].next].ctr.x || y != dungeon->r[paths[i].next].ctr.y) {
			int dirx = 0; /* -1 for left, 1 for right */
			int diry = 0; /* -1 for down, 1 for up */

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
			/* don't place corridors in rooms */
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

/* generate a blank dungeon */
void gen_dungeon(Dungeon * dungeon) {
	/*** top 3 (0, 1, 2) are reserved for the pseudo-HUD ***/
	int i, j;

	/* set all slots to spaces originally */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			(dungeon->d[i][j]).c = ' ';	/* all basic rooms are spaces */
			int h = (rand() % 254) + 1;
			(dungeon->d[i][j]).h = h;
		}
	}

	/* immut-ify the outside rim */
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

	/* make p equal to d */
	for(i = 0; i < dungeon->h; i++) {
		for(j = 0; j < dungeon->w; j++) {
			dungeon->p[i][j] = dungeon->d[i][j];
		}
	}

	/* populate the rooms */
	int cnt = 0;
	int tst = 0;
	for(i = 0; dungeon->nr < dungeon->mr && cnt < 2000; i++) {
		tst = place_room(dungeon);
		if(tst < 0) {
			cnt++;
		}
	}


}

/* initializes the dungeon structure */
Dungeon init_dungeon(int h, int w, int mr) {
	Dungeon new_dungeon;
	new_dungeon.h	= h;
	new_dungeon.w	= w;
	new_dungeon.mr	= mr;
	new_dungeon.nr	= 0;
	new_dungeon.ns	= 0;
	new_dungeon.ms	= w*h; /* max sprites would be 1 per dungeon slot */
	new_dungeon.t	= 0;
	new_dungeon.go	= FALSE;

	/* dungeon buffer allocation+0'ing */
	new_dungeon.d = calloc(new_dungeon.h, sizeof(Tile *));

	int i;
	for(i = 0; i < new_dungeon.h; i++) {
		new_dungeon.d[i] = calloc(new_dungeon.w, sizeof(Tile));
	}

	/* dungeon visual buffer allocation+0'ing */
	new_dungeon.p = calloc(new_dungeon.h, sizeof(Tile *));

	for(i = 0; i < new_dungeon.h; i++) {
		new_dungeon.p[i] = calloc(new_dungeon.w, sizeof(Tile));
	}

	/* rooms allocation+0'ing */
	new_dungeon.r = calloc(new_dungeon.mr, sizeof(Room));

	/* sprites allocation */
	new_dungeon.ss = calloc(new_dungeon.ms, sizeof(Sprite));

	/* djikstra-based cost map allocation */
	new_dungeon.cst = calloc(w*h, sizeof(int *));
	for(i = 0; i < new_dungeon.h; i++) {
		new_dungeon.cst[i] = calloc(new_dungeon.w, sizeof(int));
	}

	/* djikstra-based cost map allocation */
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


/* Basic procedural dungeon generator */
int main(int argc, char * argv[]) {
	/*** process commandline arguments ***/
	int max_args = 7;
	int saving = FALSE;
	int loading = FALSE;
	int pathing = FALSE;
	int num_mon = 1;
	int custom_path = 0;
	if(argc > 2 && argc <= max_args) {
		/* both --save and --load */
		int i;
		for(i = 1; i < argc; i++) {
			test_args(argc, argv, i, &saving, &loading, &pathing, &custom_path, &num_mon);
		}
	} else if(argc == 2) {
		/* one arg */
		test_args(argc, argv, 1, &saving, &loading, &pathing, &custom_path, &num_mon);
	} else if(argc > max_args) {
		/* more than 2 commandline arguments, argv[0] is gratuitous */
		printf("Too many arguments!\n");
	} else {
		/* other; most likely 0 */
	}
	/*** end processing commandline arguments ***/


	/* init the dungeon with default dungeon size and a max of 12 rooms */
	srand(time(NULL));

	/* create 2 char pointers so as not to pollute the original HOME variable */
	char * env_path = getenv("HOME");
	/* char * path = calloc(strlen(env_path) + 17, sizeof(char)); */
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
	/*** dungeon is fully initiated ***/
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
	/*** dungeon is fully generated ***/

    //binheap_t h;
	//binheap_init(&h, compare_move, NULL);

	/* main loop */
	//Event nexts[dungeon.ns]

	for(i = 0; i < dungeon.ns; i++) {
		gen_move_sprite(&dungeon, i);
		//nexts[i] = next;
	}


	bool first = TRUE;
	bool run = TRUE;
	while(run == TRUE) {
		//Sprite *s = (Sprite*) binheap_remove_min(&h);
		int l = 0;
		for(i = 0; i < dungeon.ns; i++) {
			if(dungeon.ss[i].t < dungeon.ss[l].t) {
				l = i;
			}
		}
		//printf("sprite %d being worked on!\n", l);
		//dungeon.ss[s->sn];
		//printf("parsing move for %d at %d!\n", l, dungeon.ss[l].t);
		parse_move(&dungeon, l);
		gen_move_sprite(&dungeon, l);
		//printf("inserting move for %d at %d!\n", l, dungeon.ss[l].t);
		//binheap_insert(&h, (void *)s);
		//printf("binheap size: %d\n", h.size);
		if(l == dungeon.pc || first == TRUE) {
			//printf("num sprites: %d\n", dungeon.ns);
			map_dungeon_nont(&dungeon);
			map_dungeon_t(&dungeon);
			print_dungeon(&dungeon, 0, 0);
			//print_dungeon(&dungeon, 0, 1);
			//print_dungeon(&dungeon, 1, 0);
			sleep(2);
		}
		//print_dungeon(&dungeon, 1, 0); /* prints non-tunneling dijkstra's */
		//print_dungeon(&dungeon, 0, 1); /* prints tunneling dijkstra's */

		/* note: this will stop the game before the new world gets drawn since the monster will move to the player and thus kill him */
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

	/*** tear down sequence ***/
	//binheap_delete(&h);
	END: ;
	if(saving == TRUE) {
		write_dungeon(&dungeon, path);
	}

	/* free our arrays */
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
