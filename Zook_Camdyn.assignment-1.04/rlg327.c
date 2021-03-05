#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/time.h>
#include <assert.h>
#include <errno.h>

#include "heap.h"
#include "rlg327.h"



static uint32_t in_room(dungeon_t *d, int16_t y, int16_t x)
{
  int i;

  for (i = 0; i < d->num_rooms; i++) {
    if ((x >= d->rooms[i].position[dim_x]) &&
        (x < (d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x])) &&
        (y >= d->rooms[i].position[dim_y]) &&
        (y < (d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y]))) {
      return 1;
    }
  }

  return 0;
}

static uint32_t adjacent_to_room(dungeon_t *d, int16_t y, int16_t x)
{
  return (mapxy(x - 1, y) == ter_floor_room ||
          mapxy(x + 1, y) == ter_floor_room ||
          mapxy(x, y - 1) == ter_floor_room ||
          mapxy(x, y + 1) == ter_floor_room);
}

static uint32_t is_open_space(dungeon_t *d, int16_t y, int16_t x)
{
  return !hardnessxy(x, y);
}

static int32_t corridor_path_cmp(const void *key, const void *with) {
  return ((corridor_path_t *) key)->cost - ((corridor_path_t *) with)->cost;
}

static void dijkstra_corridor(dungeon_t *d, pair_t from, pair_t to)
{
  static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized) {
    for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }
  
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from[dim_y]][from[dim_x]].cost = 0;

  heap_init(&h, corridor_path_cmp, NULL);

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (mapxy(x, y) != ter_wall_immutable) {
        path[y][x].hn = heap_insert(&h, &path[y][x]);
      } else {
        path[y][x].hn = NULL;
      }
    }
  }

  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
        if (mapxy(x, y) != ter_floor_room) {
          mapxy(x, y) = ter_floor_hall;
          hardnessxy(x, y) = 0;
        }
      }
      heap_delete(&h);
      return;
    }

    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x]    ].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] - 1].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] + 1].hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x]    ].hn);
    }
  }
}


void dijkstra_tunneling_monsters(dungeon_t *d)
{
	static corridor_path_t path[DUNGEON_Y][DUNGEON_X];
	static corridor_path_t *p;

	uint32_t init = 0;

	heap_t h;

	uint32_t x, y;

	if (!init)
	{
		for (y = 0; y < DUNGEON_Y; y++)
		{
			for (x = 0; x < DUNGEON_X; x++)
			{
				path[y][x].pos[dim_y] = y;
				path[y][x].pos[dim_x] = x;
			}
		}
		init = 1;
	}

	for (y = 0; y < DUNGEON_Y; y++){
		for (x = 0; x < DUNGEON_X; x++){
			path[y][x].cost = INT_MAX;
		}
	} 

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			d->tmap[y][x] = 0;
		}
	}

	path[d->pc[dim_y]][d->pc[dim_x]].cost = 0;

	heap_init(&h, corridor_path_cmp, NULL); 

	for (y = 0; y < DUNGEON_Y; y++)
	{
		for (x = 0; x < DUNGEON_X; x++)
		{
			if (d->map[y][x] != ter_wall_immutable)
			{ 
				path[y][x].hn = heap_insert(&h, &path[y][x]);	
			}
			else
			{
				path[y][x].hn = NULL;
			}
		}
	}

	while ((p = heap_remove_min(&h)))
	{
		d->tmap[p->pos[dim_y]][p->pos[dim_x]] = p->cost;
		if(( path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn) &&
			(path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn);
		}

		if(( path[p->pos[dim_y] - 1][p->pos[dim_x]].hn) &&
			(path[p->pos[dim_y] - 1][p->pos[dim_x]].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] - 1][p->pos[dim_x]].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x]].hn);
		}

		if(( path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn) &&
			(path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn);
		}

		if(( path[p->pos[dim_y]][p->pos[dim_x] - 1].hn) &&
			(path[p->pos[dim_y]][p->pos[dim_x] - 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y]][p->pos[dim_x] - 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y]][p->pos[dim_x] - 1].hn);
		}

		if(( path[p->pos[dim_y]][p->pos[dim_x] + 1].hn) &&
			(path[p->pos[dim_y]][p->pos[dim_x] + 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y]][p->pos[dim_x] + 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y]][p->pos[dim_x] + 1].hn);
		}

		if(( path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn) &&
			(path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn);

		}

		if(( path[p->pos[dim_y] + 1][p->pos[dim_x]].hn) &&
			(path[p->pos[dim_y] + 1][p->pos[dim_x]].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] + 1][p->pos[dim_x]].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x]].hn);

		}

		if(( path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn) &&
			(path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn);
		}
	}
	heap_delete(&h);
}

void dijkstra_non_tunneling_monsters(dungeon_t *d){
	static corridor_path_t path[DUNGEON_Y][DUNGEON_X];
	static corridor_path_t *p;

	uint32_t init = 0;

	heap_t h;

	uint32_t x, y;

	if (!init)
	{
		for (y = 0; y < DUNGEON_Y; y++)
		{ 
			for (x = 0; x < DUNGEON_X; x++)
			{
				path[y][x].pos[dim_y] = y;
				path[y][x].pos[dim_x] = x;
			}
		}
		init = 1;
	}

	for (y = 0; y < DUNGEON_Y; y++){
		for (x = 0; x < DUNGEON_X; x++){
			path[y][x].cost = INT_MAX;
		}
	} 

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			d->ntmap[y][x] = 0;
		}
	}

	path[d->pc[dim_y]][d->pc[dim_x]].cost = 0;

	heap_init(&h, corridor_path_cmp, NULL);

	for (y = 0; y < DUNGEON_Y; y++)
	{
		for (x = 0; x < DUNGEON_X; x++)
		{
			if(d->hardness[y][x] == 0){
				path[y][x].hn = heap_insert(&h, &path[y][x]);
			} else{
				path[y][x].hn = NULL;
			}
		}
	}

	while ((p = heap_remove_min(&h)))
	{
		d->ntmap[p->pos[dim_y]][p->pos[dim_x]] = p->cost;

		if(( path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn) &&
			(path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn);
		}

		if(( path[p->pos[dim_y] - 1][p->pos[dim_x]].hn) &&
			(path[p->pos[dim_y] - 1][p->pos[dim_x]].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] - 1][p->pos[dim_x]].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x]].hn);
		}

		if(( path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn) &&
			(path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn);
		}

		if(( path[p->pos[dim_y]][p->pos[dim_x] - 1].hn) &&
			(path[p->pos[dim_y]][p->pos[dim_x] - 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y]][p->pos[dim_x] - 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y]][p->pos[dim_x] - 1].hn);
		}

		if(( path[p->pos[dim_y]][p->pos[dim_x] + 1].hn) &&
			(path[p->pos[dim_y]][p->pos[dim_x] + 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y]][p->pos[dim_x] + 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y]][p->pos[dim_x] + 1].hn);
		}

		if(( path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn) &&
			(path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn);
		}

		if(( path[p->pos[dim_y] + 1][p->pos[dim_x]].hn) &&
			(path[p->pos[dim_y] + 1][p->pos[dim_x]].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] + 1][p->pos[dim_x]].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x]].hn);
		}

		if(( path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn) &&
			(path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost > (p->cost + distancepair(p->pos)))){
				path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost = p->cost + distancepair(p->pos);
				path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
				path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
				heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn);
		}
	}
	heap_delete(&h);
}

void populate_tunneling_monsters(dungeon_t *d)
{
	printf("\nDijkstra's Rendering for Tunneling Monsters.\n");
	int y, x;

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			switch (mapxy(x,y)) {
				case ter_wall_immutable:
						putchar(' ');
					break;
				default:
					if (x == d->pc[dim_x] && y == d->pc[dim_y]){
						putchar('@');
						break;
					}
					if(d->tmap[y][x] != 0){
						printf("%d", d->tmap[y][x] % 10);
						break;
					}else{
						putchar(' ');
					}
			}
    	}
    	putchar('\n');
	}
}

void populate_non_tunneling_monsters(dungeon_t *d)
{
	printf("\nDijkstra's Rendering for Non-Tunneling Monsters.\n");
	for (int y = 0; y < DUNGEON_Y; y++) {
		for (int x = 0; x < DUNGEON_X; x++) {
			switch (mapxy(x,y)) {
				case ter_wall_immutable:
						putchar(' ');
					break;
				default:	
					if (x == d->pc[dim_x] && y == d->pc[dim_y]){
						putchar('@');
						break;
					}
					if(d->ntmap[y][x] != 0){
						printf("%d", d->ntmap[y][x] % 10);
					}else
						putchar(' ');
			}
    	}
    	putchar('\n');
}
}

/* This is a cut-and-paste of the above.  The code is modified to  *
 * calculate paths based on inverse hardnesses so that we get a    *
 * high probability of creating at least one cycle in the dungeon. */
static void dijkstra_corridor_inv(dungeon_t *d, pair_t from, pair_t to)
{
  static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized) {
    for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }
  
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from[dim_y]][from[dim_x]].cost = 0;

  heap_init(&h, corridor_path_cmp, NULL);

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (mapxy(x, y) != ter_wall_immutable) {
        path[y][x].hn = heap_insert(&h, &path[y][x]);
      } else {
        path[y][x].hn = NULL;
      }
    }
  }

  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
        if (mapxy(x, y) != ter_floor_room) {
          mapxy(x, y) = ter_floor_hall;
          hardnessxy(x, y) = 0;
        }
      }
      heap_delete(&h);
      return;
    }

#define hardnesspair_inv(p) (is_open_space(d, p[dim_y], p[dim_x]) ? 127 :     \
                             (adjacent_to_room(d, p[dim_y], p[dim_x]) ? 191 : \
                              (255 - hardnesspair(p))))

    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x]    ].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] - 1].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] + 1].hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x]    ].hn);
    }
  }
}

/* Chooses a random point inside each room and connects them with a *
 * corridor.  Random internal points prevent corridors from exiting *
 * rooms in predictable locations.                                  */
static int connect_two_rooms(dungeon_t *d, room_t *r1, room_t *r2)
{
  pair_t e1, e2;

  e1[dim_y] = rand_range(r1->position[dim_y],
                         r1->position[dim_y] + r1->size[dim_y] - 1);
  e1[dim_x] = rand_range(r1->position[dim_x],
                         r1->position[dim_x] + r1->size[dim_x] - 1);
  e2[dim_y] = rand_range(r2->position[dim_y],
                         r2->position[dim_y] + r2->size[dim_y] - 1);
  e2[dim_x] = rand_range(r2->position[dim_x],
                         r2->position[dim_x] + r2->size[dim_x] - 1);

  /*  return connect_two_points_recursive(d, e1, e2);*/
  dijkstra_corridor(d, e1, e2);

  return 0;
}

static int create_cycle(dungeon_t *d)
{
  /* Find the (approximately) farthest two rooms, then connect *
   * them by the shortest path using inverted hardnesses.      */

  int32_t max, tmp, i, j, p, q;
  pair_t e1, e2;

  for (i = max = 0; i < d->num_rooms - 1; i++) {
    for (j = i + 1; j < d->num_rooms; j++) {
      tmp = (((d->rooms[i].position[dim_x] - d->rooms[j].position[dim_x])  *
              (d->rooms[i].position[dim_x] - d->rooms[j].position[dim_x])) +
             ((d->rooms[i].position[dim_y] - d->rooms[j].position[dim_y])  *
              (d->rooms[i].position[dim_y] - d->rooms[j].position[dim_y])));
      if (tmp > max) {
        max = tmp;
        p = i;
        q = j;
      }
    }
  }

  /* Can't simply call connect_two_rooms() because it doesn't *
   * use inverse hardnesses, so duplicate it here.            */
  e1[dim_y] = rand_range(d->rooms[p].position[dim_y],
                         (d->rooms[p].position[dim_y] +
                          d->rooms[p].size[dim_y] - 1));
  e1[dim_x] = rand_range(d->rooms[p].position[dim_x],
                         (d->rooms[p].position[dim_x] +
                          d->rooms[p].size[dim_x] - 1));
  e2[dim_y] = rand_range(d->rooms[q].position[dim_y],
                         (d->rooms[q].position[dim_y] +
                          d->rooms[q].size[dim_y] - 1));
  e2[dim_x] = rand_range(d->rooms[q].position[dim_x],
                         (d->rooms[q].position[dim_x] +
                          d->rooms[q].size[dim_x] - 1));

  dijkstra_corridor_inv(d, e1, e2);

  return 0;
}

static int connect_rooms(dungeon_t *d)
{
  uint32_t i;

  for (i = 1; i < d->num_rooms; i++) {
    connect_two_rooms(d, d->rooms + i - 1, d->rooms + i);
  }

  create_cycle(d);

  return 0;
}

int gaussian[5][5] = {
  {  1,  4,  7,  4,  1 },
  {  4, 16, 26, 16,  4 },
  {  7, 26, 41, 26,  7 },
  {  4, 16, 26, 16,  4 },
  {  1,  4,  7,  4,  1 }
};

typedef struct queue_node {
  int x, y;
  struct queue_node *next;
} queue_node_t;

static int smooth_hardness(dungeon_t *d)
{
  int32_t i, x, y;
  int32_t s, t, p, q;
  queue_node_t *head, *tail, *tmp;
#if DUMP_HARDNESS_IMAGES
  FILE *out;
#endif
  uint8_t hardness[DUNGEON_Y][DUNGEON_X];

  memset(&hardness, 0, sizeof (hardness));

  /* Seed with some values */
  for (i = 1; i < 255; i += 20) {
    do {
      x = rand() % DUNGEON_X;
      y = rand() % DUNGEON_Y;
    } while (hardness[y][x]);
    hardness[y][x] = i;
    if (i == 1) {
      head = tail = malloc(sizeof (*tail));
    } else {
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

#if DUMP_HARDNESS_IMAGES
  out = fopen("seeded.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
  fwrite(&hardness, sizeof (hardness), 1, out);
  fclose(out);
#endif
  
  /* Diffuse the vaules to fill the space */
  while (head) {
    x = head->x;
    y = head->y;
    i = hardness[y][x];

    if (x - 1 >= 0 && y - 1 >= 0 && !hardness[y - 1][x - 1]) {
      hardness[y - 1][x - 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y - 1;
    }
    if (x - 1 >= 0 && !hardness[y][x - 1]) {
      hardness[y][x - 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y;
    }
    if (x - 1 >= 0 && y + 1 < DUNGEON_Y && !hardness[y + 1][x - 1]) {
      hardness[y + 1][x - 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y + 1;
    }
    if (y - 1 >= 0 && !hardness[y - 1][x]) {
      hardness[y - 1][x] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y - 1;
    }
    if (y + 1 < DUNGEON_Y && !hardness[y + 1][x]) {
      hardness[y + 1][x] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y + 1;
    }
    if (x + 1 < DUNGEON_X && y - 1 >= 0 && !hardness[y - 1][x + 1]) {
      hardness[y - 1][x + 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y - 1;
    }
    if (x + 1 < DUNGEON_X && !hardness[y][x + 1]) {
      hardness[y][x + 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y;
    }
    if (x + 1 < DUNGEON_X && y + 1 < DUNGEON_Y && !hardness[y + 1][x + 1]) {
      hardness[y + 1][x + 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y + 1;
    }

    tmp = head;
    head = head->next;
    free(tmp);
  }

  /* And smooth it a bit with a gaussian convolution */
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      for (s = t = p = 0; p < 5; p++) {
        for (q = 0; q < 5; q++) {
          if (y + (p - 2) >= 0 && y + (p - 2) < DUNGEON_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < DUNGEON_X) {
            s += gaussian[p][q];
            t += hardness[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      d->hardness[y][x] = t / s;
    }
  }
  /* Let's do it again, until it's smooth like Kenny G. */
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      for (s = t = p = 0; p < 5; p++) {
        for (q = 0; q < 5; q++) {
          if (y + (p - 2) >= 0 && y + (p - 2) < DUNGEON_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < DUNGEON_X) {
            s += gaussian[p][q];
            t += hardness[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      d->hardness[y][x] = t / s;
    }
  }

#if DUMP_HARDNESS_IMAGES
  out = fopen("diffused.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
  fwrite(&hardness, sizeof (hardness), 1, out);
  fclose(out);

  out = fopen("smoothed.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
  fwrite(&d->hardness, sizeof (d->hardness), 1, out);
  fclose(out);
#endif

  return 0;
}

static int empty_dungeon(dungeon_t *d)
{
  uint8_t x, y;

  smooth_hardness(d);
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mapxy(x, y) = ter_wall;
      if (y == 0 || y == DUNGEON_Y - 1 ||
          x == 0 || x == DUNGEON_X - 1) {
        mapxy(x, y) = ter_wall_immutable;
        hardnessxy(x, y) = 255;
      }
    }
  }

  return 0;
}

static int place_rooms(dungeon_t *d)
{
  pair_t p;
  uint32_t i;
  int success;
  room_t *r;

  for (success = 0; !success; ) {
    success = 1;
    for (i = 0; success && i < d->num_rooms; i++) {
      r = d->rooms + i;
      r->position[dim_x] = 1 + rand() % (DUNGEON_X - 2 - r->size[dim_x]);
      r->position[dim_y] = 1 + rand() % (DUNGEON_Y - 2 - r->size[dim_y]);
      for (p[dim_y] = r->position[dim_y] - 1;
           success && p[dim_y] < r->position[dim_y] + r->size[dim_y] + 1;
           p[dim_y]++) {
        for (p[dim_x] = r->position[dim_x] - 1;
             success && p[dim_x] < r->position[dim_x] + r->size[dim_x] + 1;
             p[dim_x]++) {
          if (mappair(p) >= ter_floor) {
            success = 0;
            empty_dungeon(d);
          } else if ((p[dim_y] != r->position[dim_y] - 1)              &&
                     (p[dim_y] != r->position[dim_y] + r->size[dim_y]) &&
                     (p[dim_x] != r->position[dim_x] - 1)              &&
                     (p[dim_x] != r->position[dim_x] + r->size[dim_x])) {
            mappair(p) = ter_floor_room;
            hardnesspair(p) = 0;
          }
        }
      }
    }
  }

  return 0;
}

static void place_stairs(dungeon_t *d)
{
  pair_t p;
  do {
    while ((p[dim_y] = rand_range(1, DUNGEON_Y - 2)) &&
           (p[dim_x] = rand_range(1, DUNGEON_X - 2)) &&
           ((mappair(p) < ter_floor)                 ||
            (mappair(p) > ter_stairs)))
      ;
    mappair(p) = ter_stairs_down;
  } while (rand_under(1, 3));
  do {
    while ((p[dim_y] = rand_range(1, DUNGEON_Y - 2)) &&
           (p[dim_x] = rand_range(1, DUNGEON_X - 2)) &&
           ((mappair(p) < ter_floor)                 ||
            (mappair(p) > ter_stairs)))
      
      ;
    mappair(p) = ter_stairs_up;
  } while (rand_under(2, 4));
}

static int make_rooms(dungeon_t *d)
{
  uint32_t i;

  for (i = MIN_ROOMS; i < MAX_ROOMS && rand_under(5, 8); i++)
    ;
  d->num_rooms = i;
  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);
  
  for (i = 0; i < d->num_rooms; i++) {
    d->rooms[i].size[dim_x] = ROOM_MIN_X;
    d->rooms[i].size[dim_y] = ROOM_MIN_Y;
    while (rand_under(3, 5) && d->rooms[i].size[dim_x] < ROOM_MAX_X) {
      d->rooms[i].size[dim_x]++;
    }
    while (rand_under(3, 5) && d->rooms[i].size[dim_y] < ROOM_MAX_Y) {
      d->rooms[i].size[dim_y]++;
    }
  }

  return 0;
}

int gen_dungeon(dungeon_t *d)
{
  empty_dungeon(d);

  do {
    make_rooms(d);
  } while (place_rooms(d));
  connect_rooms(d);
  place_stairs(d);

  return 0;
}

void populate_dungeon(dungeon_t *d)
{
  pair_t p;

  for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
    for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
      if (d->pc[dim_x] == p[dim_x] && d->pc[dim_y] == p[dim_y]) {
        putchar('@');
      } else {
        switch (mappair(p)) {
        case ter_wall:
        case ter_wall_immutable:
          putchar(' ');
          break;
        case ter_floor:
        case ter_floor_room:
          putchar('.');
          break;
        case ter_floor_hall:
          putchar('#');
          break;
        case ter_debug:
          putchar('*');
          fprintf(stderr, "Debug character at %d, %d\n", p[dim_y], p[dim_x]);
          break;
        case ter_stairs_up:
          putchar('<');
          break;
        case ter_stairs_down:
          putchar('>');
          break;
        default:
          break;
        }
      }
    }
    putchar('\n');
  }
}

void delete_dungeon(dungeon_t *d)
{
  free(d->rooms);
}

void init_dungeon(dungeon_t *d)
{
  empty_dungeon(d);
}

int write_dungeon_map(dungeon_t *d, FILE *f)
{
  uint32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      fwrite(&d->hardness[y][x], sizeof (unsigned char), 1, f);
    }
  }

  return 0;
}

int write_rooms(dungeon_t *d, FILE *f)
{
  uint32_t i;
  uint16_t p;

  p = htobe16(d->num_rooms);
  fwrite(&p, 2, 1, f);
  for (i = 0; i < d->num_rooms; i++) {
    /* write order is xpos, ypos, width, height */
    p = d->rooms[i].position[dim_x];
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].position[dim_y];
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].size[dim_x];
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].size[dim_y];
    fwrite(&p, 1, 1, f);
  }

  return 0;
}

uint16_t count_up_stairs(dungeon_t *d)
{
  uint32_t x, y;
  uint16_t i;

  for (i = 0, y = 1; y < DUNGEON_Y - 1; y++) {
    for (x = 1; x < DUNGEON_X - 1; x++) {
      if (mapxy(x, y) == ter_stairs_up) {
        i++;
      }
    }
  }

  return i;
}

uint16_t count_down_stairs(dungeon_t *d)
{
  uint32_t x, y;
  uint16_t i;

  for (i = 0, y = 1; y < DUNGEON_Y - 1; y++) {
    for (x = 1; x < DUNGEON_X - 1; x++) {
      if (mapxy(x, y) == ter_stairs_down) {
        i++;
      }
    }
  }

  return i;
}

int write_stairs(dungeon_t *d, FILE *f)
{
  uint16_t num_stairs;
  uint8_t x, y;

  num_stairs = htobe16(count_up_stairs(d));
  fwrite(&num_stairs, 2, 1, f);
  for (y = 1; y < DUNGEON_Y - 1 && num_stairs; y++) {
    for (x = 1; x < DUNGEON_X - 1 && num_stairs; x++) {
      if (mapxy(x, y) == ter_stairs_up) {
        num_stairs--;
        fwrite(&x, 1, 1, f);
        fwrite(&y, 1, 1, f);
      }
    }
  }

  num_stairs = htobe16(count_down_stairs(d));
  fwrite(&num_stairs, 2, 1, f);
  for (y = 1; y < DUNGEON_Y - 1 && num_stairs; y++) {
    for (x = 1; x < DUNGEON_X - 1 && num_stairs; x++) {
      if (mapxy(x, y) == ter_stairs_down) {
        num_stairs--;
        fwrite(&x, 1, 1, f);
        fwrite(&y, 1, 1, f);
      }
    }
  }

  return 0;
}

uint32_t calculate_dungeon_size(dungeon_t *d)
{
  /* Per the spec, 1708 is 12 byte semantic marker + 4 byte file verion + *
   * 4 byte file size + 2 byte PC position + 1680 byte hardness array +   *
   * 2 byte each number of rooms, number of up stairs, number of down     *
   * stairs.                                                              */
  return (1708 + (d->num_rooms * 4) +
          (count_up_stairs(d) * 2)  +
          (count_down_stairs(d) * 2));
}

int makedirectory(char *dir)
{
  char *slash;

  for (slash = dir + strlen(dir); slash > dir && *slash != '/'; slash--)
    ;

  if (slash == dir) {
    return 0;
  }

  if (mkdir(dir, 0700)) {
    if (errno != ENOENT && errno != EEXIST) {
      fprintf(stderr, "mkdir(%s): %s\n", dir, strerror(errno));
      return 1;
    }
    if (*slash != '/') {
      return 1;
    }
    *slash = '\0';
    if (makedirectory(dir)) {
      *slash = '/';
      return 1;
    }

    *slash = '/';
    if (mkdir(dir, 0700) && errno != EEXIST) {
      fprintf(stderr, "mkdir(%s): %s\n", dir, strerror(errno));
      return 1;
    }
  }

  return 0;
}

int write_dungeon(dungeon_t *d, char *file)
{
  char *home;
  char *filename;
  FILE *f;
  size_t len;
  uint32_t be32;

  if (!file) {
    if (!(home = getenv("HOME"))) {
      fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
      home = ".";
    }

    len = (strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) +
           1 /* The NULL terminator */                                 +
           2 /* The slashes */);

    filename = malloc(len * sizeof (*filename));
    sprintf(filename, "%s/%s/", home, SAVE_DIR);
    makedirectory(filename);
    strcat(filename, DUNGEON_SAVE_FILE);

    if (!(f = fopen(filename, "w"))) {
      perror(filename);
      free(filename);

      return 1;
    }
    free(filename);
  } else {
    if (!(f = fopen(file, "w"))) {
      perror(file);
      exit(-1);
    }
  }

  /* The semantic, which is 6 bytes, 0-11 */
  fwrite(DUNGEON_SAVE_SEMANTIC, 1, sizeof (DUNGEON_SAVE_SEMANTIC) - 1, f);

  /* The version, 4 bytes, 12-15 */
  be32 = htobe32(DUNGEON_SAVE_VERSION);
  fwrite(&be32, sizeof (be32), 1, f);

  /* The size of the file, 4 bytes, 16-19 */
  be32 = htobe32(calculate_dungeon_size(d));
  fwrite(&be32, sizeof (be32), 1, f);

  /* The PC position, 2 bytes, 20-21 */
  fwrite(&d->pc[dim_x], 1, 1, f);
  fwrite(&d->pc[dim_y], 1, 1, f);

  /* The dungeon map, 1680 bytes, 22-1702 */
  write_dungeon_map(d, f);

  /* The rooms, num_rooms * 4 bytes, 1703-end */
  write_rooms(d, f);

  /* And the stairs */
  write_stairs(d, f);

  fclose(f);

  return 0;
}

int read_dungeon_map(dungeon_t *d, FILE *f)
{
  uint32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      fread(&d->hardness[y][x], sizeof (d->hardness[y][x]), 1, f);
      if (d->hardness[y][x] == 0) {
        /* Mark it as a corridor.  We can't recognize room cells until *
         * after we've read the room array, which we haven't done yet. */
        d->map[y][x] = ter_floor_hall;
      } else if (d->hardness[y][x] == 255) {
        d->map[y][x] = ter_wall_immutable;
      } else {
        d->map[y][x] = ter_wall;
      }
    }
  }


  return 0;
}

int read_stairs(dungeon_t *d, FILE *f)
{
  uint16_t num_stairs;
  uint8_t x, y;

  fread(&num_stairs, 2, 1, f);
  num_stairs = be16toh(num_stairs);
  for (; num_stairs; num_stairs--) {
    fread(&x, 1, 1, f);
    fread(&y, 1, 1, f);
    mapxy(x, y) = ter_stairs_up;
  }

  fread(&num_stairs, 2, 1, f);
  num_stairs = be16toh(num_stairs);
  for (; num_stairs; num_stairs--) {
    fread(&x, 1, 1, f);
    fread(&y, 1, 1, f);
    mapxy(x, y) = ter_stairs_down;
  }
  return 0;
}

int read_rooms(dungeon_t *d, FILE *f)
{
  uint32_t i;
  uint32_t x, y;
  uint16_t p;

  fread(&p, 2, 1, f);
  d->num_rooms = be16toh(p);
  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);

  for (i = 0; i < d->num_rooms; i++) {
    fread(&p, 1, 1, f);
    d->rooms[i].position[dim_x] = p;
    fread(&p, 1, 1, f);
    d->rooms[i].position[dim_y] = p;
    fread(&p, 1, 1, f);
    d->rooms[i].size[dim_x] = p;
    fread(&p, 1, 1, f);
    d->rooms[i].size[dim_y] = p;

    if (d->rooms[i].size[dim_x] < 1             ||
        d->rooms[i].size[dim_y] < 1             ||
        d->rooms[i].size[dim_x] > DUNGEON_X - 1 ||
        d->rooms[i].size[dim_y] > DUNGEON_X - 1) {
      fprintf(stderr, "Invalid room size in restored dungeon.\n");

      exit(-1);
    }

    if (d->rooms[i].position[dim_x] < 1                                       ||
        d->rooms[i].position[dim_y] < 1                                       ||
        d->rooms[i].position[dim_x] > DUNGEON_X - 1                           ||
        d->rooms[i].position[dim_y] > DUNGEON_Y - 1                           ||
        d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x] > DUNGEON_X - 1 ||
        d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x] < 0             ||
        d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y] > DUNGEON_Y - 1 ||
        d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y] < 0)             {
      fprintf(stderr, "Invalid room position in restored dungeon.\n");

      exit(-1);
    }
        

    /* After reading each room, we need to reconstruct them in the dungeon. */
    for (y = d->rooms[i].position[dim_y];
         y < d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y];
         y++) {
      for (x = d->rooms[i].position[dim_x];
           x < d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x];
           x++) {
        mapxy(x, y) = ter_floor_room;
      }
    }
  }

  return 0;
}

int read_dungeon(dungeon_t *d, char *file)
{
  char semantic[sizeof (DUNGEON_SAVE_SEMANTIC)];
  uint32_t be32;
  FILE *f;
  char *home;
  size_t len;
  char *filename;
  struct stat buf;

  if (!file) {
    if (!(home = getenv("HOME"))) {
      fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
      home = ".";
    }
    len = (strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) +
           1 /* The NULL terminator */                                 +
           2 /* The slashes */);

    filename = malloc(len * sizeof (*filename) + 3);
    sprintf(filename, "%s/%s/%s", home, SAVE_DIR, DUNGEON_SAVE_FILE);


    if (!(f = fopen(filename, "r"))) {
      perror(filename);
      free(filename);
      exit(-1);
    }

    if (stat(filename, &buf)) {
      perror(filename);
      exit(-1);
    }

    free(filename);
  } else {
    home = getenv("HOME");
    char * newfile = malloc(sizeof(home) * sizeof(SAVE_DIR) * sizeof(file) + 3);
    sprintf(newfile, "%s/%s/%s", home, SAVE_DIR, file);
    printf("%s", newfile);
    if (!(f = fopen(newfile, "r"))) {
      perror(newfile);
      exit(-1);
    }
    if (stat(newfile, &buf)) {
      perror(newfile);
      exit(-1);
    }
  }

  d->num_rooms = 0;

  fread(semantic, sizeof (DUNGEON_SAVE_SEMANTIC) - 1, 1, f);
  semantic[sizeof (DUNGEON_SAVE_SEMANTIC) - 1] = '\0';
  if (strncmp(semantic, DUNGEON_SAVE_SEMANTIC,
	      sizeof (DUNGEON_SAVE_SEMANTIC) - 1)) {
    fprintf(stderr, "Not an RLG327 save file.\n");
    exit(-1);
  }
  fread(&be32, sizeof (be32), 1, f);
  if (be32toh(be32) != 0) { /* Since we expect zero, be32toh() is a no-op. */
    fprintf(stderr, "File version mismatch.\n");
    exit(-1);
  }
  fread(&be32, sizeof (be32), 1, f);
  if (buf.st_size != be32toh(be32)) {
    fprintf(stderr, "File size mismatch.\n");
    exit(-1);
  }

  fread(&d->pc[dim_x], 1, 1, f);
  fread(&d->pc[dim_y], 1, 1, f);
  
  read_dungeon_map(d, f);

  read_rooms(d, f);

  read_stairs(d, f);

  fclose(f);

  return 0;
}

/* PGM dungeon descriptions do not support PC or stairs */
int read_pgm(dungeon_t *d, char *pgm)
{
  FILE *f;
  char s[80];
  uint8_t gm[DUNGEON_Y - 2][DUNGEON_X - 2];
  uint32_t x, y;
  uint32_t i;
  char size[8]; /* Big enough to hold two 3-digit values with a space between. */

  if (!(f = fopen(pgm, "r"))) {
    perror(pgm);
    exit(-1);
  }

  if (!fgets(s, 80, f) || strncmp(s, "P5", 2)) {
    fprintf(stderr, "Expected P5\n");
    exit(-1);
  }
  if (!fgets(s, 80, f) || s[0] != '#') {
    fprintf(stderr, "Expected comment\n");
    exit(-1);
  }
  snprintf(size, 8, "%d %d", DUNGEON_X - 2, DUNGEON_Y - 2);
  if (!fgets(s, 80, f) || strncmp(s, size, 5)) {
    fprintf(stderr, "Expected %s\n", size);
    exit(-1);
  }
  if (!fgets(s, 80, f) || strncmp(s, "255", 2)) {
    fprintf(stderr, "Expected 255\n");
    exit(-1);
  }

  fread(gm, 1, (DUNGEON_X - 2) * (DUNGEON_Y - 2), f);

  fclose(f);

  /* In our gray map, treat black (0) as corridor, white (255) as room, *
   * all other values as a hardness.  For simplicity, treat every white *
   * cell as its own room, so we have to count white after reading the  *
   * image in order to allocate the room array.                         */
  for (d->num_rooms = 0, y = 0; y < DUNGEON_Y - 2; y++) {
    for (x = 0; x < DUNGEON_X - 2; x++) {
      if (!gm[y][x]) {
        d->num_rooms++;
      }
    }
  }
  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);

  for (i = 0, y = 0; y < DUNGEON_Y - 2; y++) {
    for (x = 0; x < DUNGEON_X - 2; x++) {
      if (!gm[y][x]) {
        d->rooms[i].position[dim_x] = x + 1;
        d->rooms[i].position[dim_y] = y + 1;
        d->rooms[i].size[dim_x] = 1;
        d->rooms[i].size[dim_y] = 1;
        i++;
        d->map[y + 1][x + 1] = ter_floor_room;
        d->hardness[y + 1][x + 1] = 0;
      } else if (gm[y][x] == 255) {
        d->map[y + 1][x + 1] = ter_floor_hall;
        d->hardness[y + 1][x + 1] = 0;
      } else {
        d->map[y + 1][x + 1] = ter_wall;
        d->hardness[y + 1][x + 1] = gm[y][x];
      }
    }
  }

  for (x = 0; x < DUNGEON_X; x++) {
    d->map[0][x] = ter_wall_immutable;
    d->hardness[0][x] = 255;
    d->map[DUNGEON_Y - 1][x] = ter_wall_immutable;
    d->hardness[DUNGEON_Y - 1][x] = 255;
  }
  for (y = 1; y < DUNGEON_Y - 1; y++) {
    d->map[y][0] = ter_wall_immutable;
    d->hardness[y][0] = 255;
    d->map[y][DUNGEON_X - 1] = ter_wall_immutable;
    d->hardness[y][DUNGEON_X - 1] = 255;
  }

  return 0;
}

void usage(char *name)
{
  fprintf(stderr,
          "Usage: %s [-r|--rand <seed>] [-l|--load [<file>]]\n"
          "          [-s|--save [<file>]] [-i|--image <pgm file>]\n",
          name);

  exit(-1);
}

//NEW CODE
static uint32_t in_room(dungeon_t *d, int16_t y, int16_t x)
{
  int i;

  for (i = 0; i < d->num_rooms; i++) {
    if ((x >= d->rooms[i].position[dim_x]) &&
        (x < (d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x])) &&
        (y >= d->rooms[i].position[dim_y]) &&
        (y < (d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y]))) {
      return 1;
    }
  }

  return 0;
}

static uint32_t in_same_room(dungeon_t *d, int16_t y1, int16_t x1, int16_t y2, int16_t x2)
{
  int i;

  for (i = 0; i < d->num_rooms; i++) {
    if ((x1 >= d->rooms[i].position[dim_x]) &&
        (x1 < (d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x])) &&
        (y1 >= d->rooms[i].position[dim_y]) &&
        (y1 < (d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y])) &&
	(x2 >= d->rooms[i].position[dim_x]) &&
        (x2 < (d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x])) &&
        (y2 >= d->rooms[i].position[dim_y]) &&
        (y2 < (d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y]))) {
      return 1;
    }
  }

  return 0;
}

static int32_t non_tunnel_cmp(const void *key, const void *with) {
  return ((non_tunnel_t *) key)->cost - ((non_tunnel_t *) with)->cost;
}

static int32_t tunnel_cmp(const void *key, const void *with) {
  return ((non_tunnel_t *) key)->cost - ((non_tunnel_t *) with)->cost;
}

void generate_monsters(dungeon_t *dungeon, int numMonsters, char type){
  int i;
  for(i = 0; i < numMonsters; i++){
    //50/50 chance of each attribute
    int intelligence = rand() % 2;
    int telepathy = rand() % 2;
    int tunneling = rand() % 2;
    int erratic = rand() % 2;
    int placed = 0;
    int x, y;
    //Finds a place in a room where there isn't a character for new monster
    while(!placed){
      y = rand() % 105;
      x = rand() % 160;
      if(dungeon->map[y][x] == ter_floor_room && dungeon->monster[y][x].type == 'z'){
	placed = 1;
      }
    }
    //Based on attributes adds monster to the dungeon
    if(intelligence == 0 && telepathy == 0 && tunneling == 0 && erratic == 0){
      dungeon->monster[y][x].type = '0';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 0 && telepathy == 0 && tunneling == 0 && erratic == 1){
      dungeon->monster[y][x].type = '1';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 0 && telepathy == 0 && tunneling == 1 && erratic == 0){
      dungeon->monster[y][x].type = '2';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 0 && telepathy == 0 && tunneling == 1 && erratic == 1){
      dungeon->monster[y][x].type = '3';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 0 && telepathy == 1 && tunneling == 0 && erratic == 0){
      dungeon->monster[y][x].type = '4';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 0 && telepathy == 1 && tunneling == 0 && erratic == 1){
      dungeon->monster[y][x].type = '5';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 0 && telepathy == 1 && tunneling == 1 && erratic == 0){
      dungeon->monster[y][x].type = '6';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 0 && telepathy == 1 && tunneling == 1 && erratic == 1){
      dungeon->monster[y][x].type = '7';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 1 && telepathy == 0 && tunneling == 0 && erratic == 0){
      dungeon->monster[y][x].type = '8';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 1 && telepathy == 0 && tunneling == 0 && erratic == 1){
      dungeon->monster[y][x].type = '9';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 1 && telepathy == 0 && tunneling == 1 && erratic == 0){
      dungeon->monster[y][x].type = 'a';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 1 && telepathy == 0 && tunneling == 1 && erratic == 1){
      dungeon->monster[y][x].type = 'b';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 1 && telepathy == 1 && tunneling == 0 && erratic == 0){
      dungeon->monster[y][x].type = 'c';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 1 && telepathy == 1 && tunneling == 0 && erratic == 1){
      dungeon->monster[y][x].type = 'd';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else if(intelligence == 1 && telepathy == 1 && tunneling == 1 && erratic == 0){
      dungeon->monster[y][x].type = 'e';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }
    else{
      dungeon->monster[y][x].type = 'f';
      dungeon->monster[y][x].speed = rand() % 16 + 5;
    }

    if(intelligence == 1){
      dungeon->monster[y][x].lastKnown[0] = 0;
      dungeon->monster[y][x].lastKnown[1] = 0;
    }
    
    dungeon->monster[y][x].pos[0] = y;
    dungeon->monster[y][x].pos[1] = x;
    dungeon->monster[y][x].turn = 0;
    dungeon->monster[y][x].id = i+1;
    //Used for birthing specific monsters
    if(type != 'z'){
      dungeon->monster[y][x].type = type;
      dungeon->monster[y][x].speed = rand() % 16 + 5;
      dungeon->monster[y][x].pos[0] = y;
      dungeon->monster[y][x].pos[1] = x;
      dungeon->monster[y][x].turn = 0;
    }
  }
}

void moveStraight(monster_t *monster, int playerY, int playerX){
      if(monster->pos[1] < playerX){
	monster->next_pos[0] = monster->pos[0];
	monster->next_pos[1] = monster->pos[1] + 1;
      }
      else if(monster->pos[1] > playerX){
	monster->next_pos[0] = monster->pos[0];
	monster->next_pos[1] = monster->pos[1] - 1;
      }
      else if(monster->pos[0] < playerY){
	monster->next_pos[0] = monster->pos[0] + 1;
	monster->next_pos[1] = monster->pos[1];
      }
      else if(monster->pos[0] > playerY){
	monster->next_pos[0] = monster->pos[0] - 1;
	monster->next_pos[1] = monster->pos[1];
      }
}

void findShortest(dungeon_t *dungeon, monster_t *monster, int playerY, int playerX, int tunnel){
  int ny = 0, nx = 0, best = INT_MAX;
  int mony = monster->pos[0];
  int monx = monster->pos[1];
  //Finds closest move for tunneling monsters
  if(tunnel){
    if(dungeon->tunneling[mony - 1][monx    ].cost < best){
      ny = -1;
      best = dungeon->tunneling[mony - 1][monx    ].cost;
    }
    if(dungeon->tunneling[mony    ][monx + 1].cost < best){
      nx = 1;
      best = dungeon->tunneling[mony    ][monx + 1].cost;
    }
    if(dungeon->tunneling[mony + 1][monx    ].cost < best){
      ny = 1;
      best = dungeon->tunneling[mony + 1][monx    ].cost;
    }
    if(dungeon->tunneling[mony ][monx - 1].cost < best){
      nx = -1;
      best = dungeon->tunneling[mony    ][monx - 1].cost;
    }
  }
  //Finds closest move for non-tunneling monsters
  else{
    if(dungeon->non_tunneling[monster->pos[0] - 1][monster->pos[1]    ].cost < best){
      ny = -1;
      best = dungeon->tunneling[monster->pos[0] - 1][monster->pos[1]    ].cost;
    }
    if(dungeon->non_tunneling[monster->pos[0]    ][monster->pos[1] + 1].cost < best){
      nx = 1;
      best = dungeon->tunneling[monster->pos[0]    ][monster->pos[1] + 1].cost;
    }
    if(dungeon->non_tunneling[monster->pos[0] + 1][monster->pos[1]    ].cost < best){
      ny = 1;
      best = dungeon->tunneling[monster->pos[0] + 1][monster->pos[1]    ].cost;
    }
    if(dungeon->non_tunneling[monster->pos[0]    ][monster->pos[1] - 1].cost < best){
      nx = -1;
      best = dungeon->tunneling[monster->pos[0]    ][monster->pos[1] - 1].cost;
    }
  }
  monster->next_pos[0] = monster->pos[0] + ny;
  monster->next_pos[1] = monster->pos[1] + nx;
}

void generate_move(dungeon_t *dungeon, heap_t *heap, monster_t *monster, int playerY, int playerX){
  int erratic = rand() % 2;
  int sameRoom = in_same_room(dungeon, monster->pos[0], monster->pos[1], playerY, playerX);
    
  if(monster->type == '@'){
    //PC moves randomly
    int ny = rand_range(-1, 1);
    int nx = rand_range(-1, 1);
    monster->next_pos[0] = monster->pos[0] + ny;
    monster->next_pos[1] = monster->pos[1] + nx;
  }
  else if(monster->type == '0'){
    //Monster moves only if PC is in its line of sight
    if(sameRoom){
      moveStraight(monster, playerY, playerX);
    }
    else{
      monster->next_pos[0] = monster->pos[0];
      monster->next_pos[1] = monster->pos[1];
    }
  }
  else if(monster->type == '1'){
    //Monster moves either erratically or else if PC is in line of sight
    if(erratic){
      int ny = rand_range(-1, 1);
      int nx = rand_range(-1, 1);
      monster->next_pos[0] = monster->pos[0] + ny;
      monster->next_pos[1] = monster->pos[1] + nx;
    }
    else if(sameRoom){
      moveStraight(monster, playerY, playerX);
    }
    else{
      monster->next_pos[0] = monster->pos[0];
      monster->next_pos[1] = monster->pos[1];
    }
  }
  else if(monster->type == '2'){
    //Monster can tunnel and moves if PC is in line of sight (doesn't ever need to tunnel)
    if(sameRoom){
      moveStraight(monster, playerY, playerX);
    }
    else{
      monster->next_pos[0] = monster->pos[0];
      monster->next_pos[1] = monster->pos[1];
    }
  }
  else if(monster->type == '3'){
    //Monster can tunnel and either moves erratically or if PC is in line of sight
    if(erratic){
      int ny = rand_range(-1, 1);
      int nx = rand_range(-1, 1);
      monster->next_pos[0] = monster->pos[0] + ny;
      monster->next_pos[1] = monster->pos[1] + nx;
    }
    else if(sameRoom){
      moveStraight(monster, playerY, playerX);
    }
    else{
      monster->next_pos[0] = monster->pos[0];
      monster->next_pos[1] = monster->pos[1];
    }
  }
  else if(monster->type == '4'){
    //Monster knows where the PC is and moves towards it in straight line
    moveStraight(monster, playerY, playerX);
  }
  else if(monster->type == '5'){
    //Monster either moves errtically or knows where the PC is and moves towards it in straight line
    if(erratic){
      int ny = rand_range(-1, 1);
      int nx = rand_range(-1, 1);
      monster->next_pos[0] = monster->pos[0] + ny;
      monster->next_pos[1] = monster->pos[1] + nx;
    }
    else
      moveStraight(monster, playerY, playerX);
  }
  else if(monster->type == '6'){
    //Monster moves towards PC in straight line and can tunnel
    moveStraight(monster, playerY, playerX);
  }
  else if(monster->type =='7'){
    //Monster can tunnel and either moves erratically or in a straight line towards PC
    if(erratic){
      int ny = rand_range(-1, 1);
      int nx = rand_range(-1, 1);
      monster->next_pos[0] = monster->pos[0] + ny;
      monster->next_pos[1] = monster->pos[1] + nx;
    }
    else
      moveStraight(monster, playerY, playerX);
  }
  else if(monster->type == '8'){
    //Monster moves along shortest path towards the PC when in line of sight or if last known position is known
    if(sameRoom){
      findShortest(dungeon, monster, playerY, playerX, 0);
      monster->lastKnown[0] = playerY;
      monster->lastKnown[1] = playerX;
    }
    else if(monster->lastKnown[0] != 0 && monster->lastKnown[1] != 0){
      moveStraight(monster, monster->lastKnown[0], monster->lastKnown[1]);
      monster->lastKnown[0] = 0;
      monster->lastKnown[1] = 0;
    }
    else{
      monster->next_pos[0] = monster->pos[0];
      monster->next_pos[1] = monster->pos[1];
    }
  }
  else if(monster->type == '9'){
    //Monster either moves eratically or along shortest path when PC is in line of sight or last position is known
    if(erratic){
      int ny = rand_range(-1, 1);
      int nx = rand_range(-1, 1);
      monster->next_pos[0] = monster->pos[0] + ny;
      monster->next_pos[1] = monster->pos[1] + nx;
    }
    else if(sameRoom){
      findShortest(dungeon, monster, playerY, playerX, 0);
      monster->lastKnown[0] = playerY;
      monster->lastKnown[1] = playerX;
    }
    else if(monster->lastKnown[0] != 0 && monster->lastKnown[1] != 0){
      moveStraight(monster, monster->lastKnown[0], monster->lastKnown[1]);
      monster->lastKnown[0] = 0;
      monster->lastKnown[1] = 0;
    }
    else{
      monster->next_pos[0] = monster->pos[0];
      monster->next_pos[1] = monster->pos[1];
    }
  }
  else if(monster->type == 'a'){
    //Monster can tunnel and moves along shortest path when PC is in line of sight or is last position is known
    if(sameRoom){
      findShortest(dungeon, monster, playerY, playerX, 1);
      monster->lastKnown[0] = playerY;
      monster->lastKnown[1] = playerX;
    }
    else if(monster->lastKnown[0] != 0 && monster->lastKnown[1] != 1){
      moveStraight(monster, monster->lastKnown[0], monster->lastKnown[1]);
      monster->lastKnown[0] = 0;
      monster->lastKnown[1] = 0;
    }
    else{
      monster->next_pos[0] = monster->pos[0];
      monster->next_pos[1] = monster->pos[1];
    }
  }
  else if(monster->type == 'b'){
    //Monster can tunnel and either moves erratically of moves along shortest path when PC is in line of sight or is last position is known
    if(erratic){
      int ny = rand_range(-1, 1);
      int nx = rand_range(-1, 1);
      monster->next_pos[0] = monster->pos[0] + ny;
      monster->next_pos[1] = monster->pos[1] + nx;
    }
    else if(sameRoom){
      findShortest(dungeon, monster, playerY, playerX, 1);
      monster->lastKnown[0] = playerY;
      monster->lastKnown[1] = playerX;
    }
    else if(monster->lastKnown[0] != 0 && monster->lastKnown[1] != 1){
      moveStraight(monster, monster->lastKnown[0], monster->lastKnown[1]);
      monster->lastKnown[0] = 0;
      monster->lastKnown[1] = 0;
    }
    else{
      monster->next_pos[0] = monster->pos[0];
      monster->next_pos[1] = monster->pos[1];
    }
  }
  else if(monster->type == 'c'){
    //Monster moves along shortest path towards current position of PC
    findShortest(dungeon, monster, playerY, playerX, 0);
  }
  else if(monster->type == 'd'){
    //Monster either moves erratically or along shortest path towards current position of PC
    if(erratic){
      int ny = rand_range(-1, 1);
      int nx = rand_range(-1, 1);
      monster->next_pos[0] = monster->pos[0] + ny;
      monster->next_pos[1] = monster->pos[1] + nx;
    }
    else
      findShortest(dungeon, monster, playerY, playerX, 0);
  }
  else if(monster->type == 'e'){
    //Monster can tunnel and moves along shortest path towards current position of PC
    findShortest(dungeon, monster, playerY, playerX, 1);
  }
  else{
    //Monster can tunnel and either moves erratically or moves along shortest path towards current position of PC
    if(erratic){
      int ny = rand_range(-1, 1);
      int nx = rand_range(-1, 1);
      monster->next_pos[0] = monster->pos[0] + ny;
      monster->next_pos[1] = monster->pos[1] + nx;
    }
    else
      findShortest(dungeon, monster, playerY, playerX, 1);
  }
  
  
  if((monster->type == '@' || monster->type == '0' || monster->type == '1' || monster->type == '4' || monster->type =='5' || monster->type == '8' || monster->type == '9' || monster->type == 'c' || monster->type == 'd') &&
     dungeon->hardness[monster->next_pos[0]][monster->next_pos[1]] != 0){
    monster->next_pos[0] = monster->pos[0];
    monster->next_pos[1] = monster->pos[1];
    monster->turn = monster->turn + 1000 / monster->speed;
    monster->hn = heap_insert(heap, monster);
  }
  
  else{
    //printf("type: %c turn: %d cur y: %d cur x: %d in room?: %d\n", monster->type, monster->turn, monster->pos[0], monster->pos[1], sameRoom);
    monster->turn = monster->turn + 1000 / monster->speed;
    monster->hn = heap_insert(heap, monster);
  }
  
}

static int32_t move_cmp(const void *key, const void *with) {
  return ((monster_t *) key)->turn - ((monster_t *) with)->turn;
}


int main(int argc, char *argv[])
{
  dungeon_t d;
  time_t seed;
  struct timeval tv;
  uint32_t i;
  uint32_t do_load, do_save, do_seed, do_image, do_save_seed, do_save_image;
  uint32_t long_arg;
  char *save_file;
  char *load_file;
  char *pgm_file;

  UNUSED(in_room); /* Suppress warning */

  /* Default behavior: Seed with the time, generate a new dungeon, *
   * and don't write to disk.                                      */
  do_load = do_save = do_image = do_save_seed = do_save_image = 0;
  do_seed = 1;
  save_file = load_file = NULL;

  /* The project spec requires '--load' and '--save'.  It's common  *
   * to have short and long forms of most switches (assuming you    *
   * don't run out of letters).  For now, we've got plenty.  Long   *
   * forms use whole words and take two dashes.  Short forms use an *
    * abbreviation after a single dash.  We'll add '--rand' (to     *
   * specify a random seed), which will take an argument of it's    *
   * own, and we'll add short forms for all three commands, '-l',   *
   * '-s', and '-r', respectively.  We're also going to allow an    *
   * optional argument to load to allow us to load non-default save *
   * files.  No means to save to non-default locations, however.    *
   * And the final switch, '--image', allows me to create a dungeon *
   * from a PGM image, so that I was able to create those more      *
   * interesting test dungeons for you.                             */
  int nummon = 0; //0 if nummon is not specified, 1 if specified

 if (argc > 1) {
    for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0) {
      if (argv[i][0] == '-') { /* All switches start with a dash */
        if (argv[i][1] == '-') {
          argv[i]++;    /* Make the argument have a single dash so we can */
          long_arg = 1; /* handle long and short args at the same place.  */
        }
        switch (argv[i][1]) {
        case 'n':
            if ((!long_arg && argv[i][2]) ||
                (long_arg && strcmp(argv[i], "-nummon")) ||
                argc < ++i + 1 /* No more arguments */ ||
                !sscanf(argv[i], "%d", &d.numMonsters) /* Argument is not an integer */) {

                usage(argv[0]);
            }
            d.numMonsters = atoi(argv[i++]);
            nummon = 1;
            break;
        case 'r':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-rand")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%lu", &seed) /* Argument is not an integer */) {
            usage(argv[0]);
          }
          do_seed = 0;
          break;
        case 'l':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-load"))) {
            usage(argv[0]);
          }
          do_load = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            load_file = argv[++i];
          }
          break;
        case 's':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-save"))) {
            usage(argv[0]);
          }
          do_save = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll save to it.  If it is "seed", we'll save to    *
	     * <the current seed>.rlg327.  If it is "image", we'll  *
	     * save to <the current image>.rlg327.                  */
	    if (!strcmp(argv[++i], "seed")) {
	      do_save_seed = 1;
	      do_save_image = 0;
	    } else if (!strcmp(argv[i], "image")) {
	      do_save_image = 1;
	      do_save_seed = 0;
	    } else {
	      save_file = argv[i];
	    }
          }
          break;
        case 'i':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-image"))) {
            usage(argv[0]);
          }
          do_image = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            pgm_file = argv[++i];
          }
          break;
        default:
          usage(argv[0]);
        }
      } else { /* No dash */
        usage(argv[0]);
      }
    }
  }
 if (nummon == 0) {
     d.numMonsters = 13;
 }
  if (do_seed) {
    /* Allows me to generate more than one dungeon *
     * per second, as opposed to time().           */
    gettimeofday(&tv, NULL);
    seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  }

  printf("Seed is %ld.\n", seed);
  srand(seed);

  init_dungeon(&d);

  d.pc[dim_x] = d.pc[dim_y] = 0; /* Suppress false positive from Valgrind */

  if (do_load) {
    read_dungeon(&d, load_file);
  } else if (do_image) {
    read_pgm(&d, pgm_file);
  } else {
    gen_dungeon(&d);
  }

  if (!do_load) {
    /* Set a valid position for the PC */
    d.pc[dim_x] = d.rooms[0].position[dim_x];
    d.pc[dim_y] = d.rooms[0].position[dim_y];
  }
  place_pc(&d, &playerY, &playerX);

  //If monsters were put into the game
  if(numMonsters >= 0){
    generate_monsters(&d, numMonsters, 'z');

    int ids[numMonsters + 1];
    int y;
    for(y = 0; y < numMonsters + 1; y++){
      ids[y] = y;
    }

  populate_dungeon(&d);
  dijkstra_non_tunneling_monsters(&d);
  dijkstra_tunneling_monsters(&d);
  populate_non_tunneling_monsters(&d);
  populate_tunneling_monsters(&d);
  
heap_t heap;
    heap_init(&heap, move_cmp, NULL);
  
    int j, k;
    for(j = 0; j < 105; j++){
      for(k = 0; k < 160; k++){
	if(d.monster[j][k].type != 'z')
	  generate_move(&d, &heap, &d.monster[j][k], playerY, playerX);
	else
	  d.monster[j][k].hn = NULL;
      }
    }

    monster_t *move;
      
    int playing = 1;
    while(playing && (move = heap_remove_min(&heap))){

      if(ids[move->id] == -1){
	while(ids[move->id] == -1){
	  move->hn = NULL;
	  move = heap_remove_min(&heap);
	}
      }

      int ny = move->next_pos[0];
      int nx = move->next_pos[1];
      int y = move->pos[0];
      int x = move->pos[1];

      if(d.monster[move->next_pos[0]][move->next_pos[1]].type != 'z' && d.monster[move->next_pos[0]][move->next_pos[1]].type != d.monster[move->pos[0]][move->pos[1]].type){
	//If collision with player game ends
	if(move->next_pos[0] == playerY && move->next_pos[1] == playerX){
	  printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nGame Over\n");
	  playing = 0;
	  break;
	}
	//Else consumes monster in that space
	else{
	  int t;
	  for(t = 0; t < numMonsters + 1; t++){
	    if(d.monster[ny][nx].id == ids[t]){
	      ids[t] = -1;
	    }
	  }
	  d.monster[ny][nx] = *move;
 	  d.monster[ny][nx].pos[0] = ny;
 	  d.monster[ny][nx].pos[1] = nx;
 	  d.monster[ny][nx].hn = NULL;
	  d.monster[y][x].type = 'z';
	}
      }
      //If next space is rock
      else if(d.hardness[move->next_pos[0]][move->next_pos[1]] != 0){
	if(d.hardness[move->next_pos[0]][move->next_pos[1]] <= 85){
 	  //Makes space a hallway
 	  d.hardness[move->next_pos[0]][move->next_pos[1]] = 0;
 	  d.map[move->next_pos[0]][move->next_pos[1]] = ter_floor_hall;
	  //Moves monster into space
	  d.monster[ny][nx] = *move;
 	  d.monster[ny][nx].pos[0] = ny;
 	  d.monster[ny][nx].pos[1] = nx;
 	  d.monster[ny][nx].hn = NULL;
 	  d.monster[y][x].type = 'z';
	}
	else{
	  d.hardness[ny][nx] = d.hardness[ny][nx] - 85;
      	  //If monster doesn't make it through the wall it stays where it is
	  d.monster[y][x].next_pos[0] = y;
	  d.monster[y][x].next_pos[1] = x;
	  ny = y;
	  nx = x;
	  d.monster[y][x] = *move;
	  d.monster[y][x].hn = NULL;
	}
       	
      }
      //If monster moves to next space with no collision and no tunneling
      else{
	  d.monster[move->next_pos[0]][move->next_pos[1]] = *move;
 	  d.monster[move->next_pos[0]][move->next_pos[1]].pos[0] = move->next_pos[0];
 	  d.monster[move->next_pos[0]][move->next_pos[1]].pos[1] = move->next_pos[1];
 	  d.monster[move->next_pos[0]][move->next_pos[1]].hn = NULL;
	  
	  //If the monster moves out of its current space reset the space to have no monster
	  if(move->pos[0] != move->next_pos[0] || move->pos[1] != move->next_pos[1])
	    d.monster[move->pos[0]][move->pos[1]].type = 'z';
      }
    
      //If player moves, recalculate distances and re-render dungeon
      if(d.monster[ny][nx].type == '@'){
	playerY = ny;
	playerX = nx;
	
	non_tunneling(&d, playerY, playerX);
  
	tunneling(&d, playerY, playerX);

	render_dungeon(&d);

	usleep(100000);
      }
	generate_move(&d, &heap, &d.monster[ny][nx], playerY, playerX);
    }
  }
  else{
      non_tunneling(&d, playerY, playerX);
  
      tunneling(&d, playerY, playerX);

      render_dungeon(&d);
  }

  if (do_save) {
    if (do_save_seed) {
       /* 10 bytes for number, plus dot, extention and null terminator. */
      save_file = malloc(18);
      sprintf(save_file, "%ld.rlg327", seed);
    }
    if (do_save_image) {
      if (!pgm_file) {
	fprintf(stderr, "No image file was loaded.  Using default.\n");
	do_save_image = 0;
      } else {
	/* Extension of 3 characters longer than image extension + null. */
	save_file = malloc(strlen(pgm_file) + 4);
	strcpy(save_file, pgm_file);
	strcpy(strchr(save_file, '.') + 1, "rlg327");
      }
    }
    write_dungeon(&d, save_file);

    if (do_save_seed || do_save_image) {
      free(save_file);
    }
  }
  

  delete_dungeon(&d);

  return 0;
}
