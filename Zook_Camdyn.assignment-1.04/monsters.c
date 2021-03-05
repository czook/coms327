  
#include <stdio.h>
#include <stdlib.h>
#include "heap.h"
#include "rlg327.h"

void gen_move_sprite(Dungeon * dungeon, int sn) {
	//make ss[sn] when possible
	int sx = dungeon->ss[sn].p.x;
	int sy = dungeon->ss[sn].p.y;
	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	Sprite *s = &(dungeon->ss[sn]);

	/* increment the turn */
	dungeon->ss[sn].t += (100 / s->s.s);

	Position new = {-1, -1};

	dungeon->d[s->p.y][s->p.x].h -= 85;
	if(dungeon->d[s->p.y][s->p.x].h < 0)
		dungeon->d[s->p.y][s->p.x].h = 0;

	// make sure we're alive
	if(s->a == TRUE) {
		int i;
		int j;
		int eb = rand() % 2; /* we have a 50% chance to behave erratically */
		for(i = 0; i < 8; i++) {
			int px = sx + xs[i];
			int py = sy + ys[i];

			if(px >= 0 && px < dungeon->w && py >= 0 && py < dungeon->h) {
				/* drunken PC movement as per assignment 1.04 */
				if(sn == dungeon->pc)
					goto PCEB;

				/* check erratic behaviour */
				if(s->s.eb == FALSE || (s->s.eb == TRUE && eb)) {
					/** check if intelligent / telepathic **/
					//new.x = sx;
					//new.y = sy;
					if(s->s.te == FALSE) {
						/* see if you're in the same room */
						bool in_room = FALSE;
						with_pc(dungeon, s, &in_room);
						if(in_room == TRUE) {
							//cache PC location
							s->pc = dungeon->ss[dungeon->pc].p;

							IN: ;
							if(s->s.in == TRUE) {
								/* we are intelligent and can see our mark (tele or otherwise) */
								int k;
								int lowest = 0;
								bool set = FALSE;
								if(s->s.tu) {
									//tunneling
									for(k = 0; k < 8; k++) {
										if(xs[k]+sx >= 0 && xs[k]+sx < dungeon->w && ys[k]+sy >= 0 && ys[k]+sy < dungeon->h) {
											if(dungeon->d[ys[k]+sy][xs[k]+sx].h < 255 && dungeon->cst[ys[k]+sy][xs[k]+sx] < dungeon->cst[ys[lowest]+sy][xs[lowest]+sx] && test_loc(dungeon, xs[k]+sx, ys[k]+sy, s) == TRUE && test_loc(dungeon, xs[lowest]+sx, ys[lowest]+sy, s) == TRUE) {
												lowest = k;
												set = TRUE;
											}
										}
									}
								} else {
									//non-tunneling
									for(k = 0; k < 8; k++) {
										px = xs[k]+sx;
										py = ys[k]+sy;
										if(px >= 0 && px < dungeon->w && py >= 0 && py < dungeon->h) {
											if(dungeon->d[py][px].h == 0 && dungeon->csnt[py][px] <= dungeon->csnt[ys[lowest]+sy][xs[lowest]+sx]) {
												lowest = k;
												set = TRUE;
											}
										}
									}
										/*
										for(k = 0; k < 8; k++) {
											if(xs[k]+sx >= 0 && xs[k]+sx < dungeon->w && ys[k]+sy >= 0 && ys[k]+sy < dungeon->h) {
												if(dungeon->d[ys[k]+sy][xs[k]+sx].h < 255 && dungeon->csnt[ys[k]+sy][xs[k]+sx] < dungeon->csnt[ys[lowest]+sy][xs[lowest]+sx] && test_loc(dungeon, xs[k]+sx, ys[k]+sy, s) == TRUE && test_loc(dungeon, xs[lowest]+sx, ys[lowest]+sy, s) == TRUE) {
													lowest = k;
													set = TRUE;
												}
											}
										}
										*/
								}
								if(set == TRUE) {
									new.x = xs[lowest] + sx;
									new.y = ys[lowest] + sy;
									break;
								} else {
									new.x = sx;
									new.y = sy;
									break;
								}

									/*
									if(test_loc(dungeon, px, py, s) == TRUE) {
										//if we can move to the point
										if(s->s.tu) {
											//tunneling
											if(new.x > 0 && new.y > 0 && new.x != sx && new.y != sy) {
												if(dungeon->cst[py][px] < dungeon->cst[new.y][new.x]) {
													new.x = px;
													new.y = py;
												}
											} else {
												new.x = px;
												new.y = py;
											}
										} else {
											//non-tunneling
											if(new.x > 0 && new.y > 0 && new.x != sx && new.y != sy) {
												if(dungeon->csnt[py][px] < dungeon->csnt[new.y][new.x]) {
													new.x = px;
													new.y = py;
												}
											} else {
												new.x = px;
												new.y = py;
											}
										}
									}
									*/
							} else {
								//if not intelligent
								if(s->pc.x < sx)
									px = sx - 1;
								if(s->pc.x > sx)
									px = sx + 1;
								if(s->pc.y < sy)
									py = sy - 1;
								if(s->pc.y > sy)
									py = sy + 1;

								if(test_loc(dungeon, px, py, s) == TRUE) {
									new.x = px;
									new.y = py;
									break;
								}
							}
						} else {
							//not in the same room and not telepathic
							//randomize
							goto PCEB;
						}
					} else {
						//we know where the PC is
						s->pc = dungeon->ss[dungeon->pc].p;
						/**
						intelligence test still applies
						we just treat it as if we're always in the room " "
						**/
						goto IN;
					}

					//printf("%c in room? %d\n",s->c , in_room);
				} else {
					/* we are erratically behaving */
					PCEB: ;
					j = 0;
					EB: ;
					int c = rand() % 9;
					px = xs[c] + sx;
					py = ys[c] + sy;
					/* try to find a place to go up to n times */
					if(test_loc(dungeon, px, py, s) == FALSE && j < 8) {
						j++;
						goto EB;
					}
					if(test_loc(dungeon, px, py, s) == TRUE) {
						/* if the location is okay, commit it*/
						new.x = px;
						new.y = py;
					}

					break;
				}
			}
		}
	}

	/* safety net */
	if(new.x < 0)
		new.x = sx;
	if(new.y < 0)
		new.y = sy;

	dungeon->ss[sn].to.x = new.x;
	dungeon->ss[sn].to.y = new.y;

	if(new.x == dungeon->ss[dungeon->pc].p.x && new.y == dungeon->ss[dungeon->pc].p.y)
		dungeon->go = TRUE;

	/* check if we have to kill another sprite */
	int i;
	for(i = 0; i < dungeon->ns; i++) {
		if(i != sn) {
			if(dungeon->ss[i].p.x == dungeon->ss[sn].to.x && dungeon->ss[i].p.y == dungeon->ss[sn].to.y && dungeon->ss[sn].a == TRUE)
				dungeon->ss[i].a = FALSE;
			/*
			else if(dungeon->ss[i].p.x == dungeon->ss[sn].p.x && dungeon->ss[i].p.y == dungeon->ss[sn].p.y && dungeon->ss[i].a == TRUE)
				dungeon->ss[sn].a = FALSE;
			*/
		}
	}

	//return e;
}