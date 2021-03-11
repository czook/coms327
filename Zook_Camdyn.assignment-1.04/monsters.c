#include <stdio.h>
#include <stdlib.h>
#include "dungeon_generator.h"
#include "binheap.h"

bool test_loc(Dungeon * dungeon, int x, int y, Sprite *s) {
	if(x > 0 && x < dungeon->w-1 && y > 0 && y < dungeon->h-1) {
		int hard = dungeon->d[y][x].h;
		if(dungeon->d[y][x].h < 255) {
			if(s->s.tu == FALSE && hard > 0)
				return FALSE;
			return TRUE;
		}
	}
	return FALSE;
}

void with_pc(Dungeon * dungeon, Sprite * s, bool *in) {
	int pc_rid	= -1;
	int s_rid	= -1;
	Sprite *pc = &(dungeon->ss[dungeon->pc]);

	int i;
	for(i = 0; i < dungeon->nr; i++) {
		if(s->p.x <= dungeon->r[i].br.x && s->p.y <= dungeon->r[i].br.y && s->p.x >= dungeon->r[i].tl.x && s->p.y >= dungeon->r[i].tl.y) {
			s_rid = i;
		}
		if(pc->p.x <= dungeon->r[i].br.x && pc->p.y <= dungeon->r[i].br.y && pc->p.x >= dungeon->r[i].tl.x && pc->p.y >= dungeon->r[i].tl.y) {
			pc_rid = i;
		}
	}
	if(pc_rid > 0 && s_rid > 0 && pc_rid == s_rid)
		*in = TRUE;
}

void gen_move_sprite(Dungeon * dungeon, int sn) {
	int sx = dungeon->ss[sn].p.x;
	int sy = dungeon->ss[sn].p.y;
	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

	Sprite *s = &(dungeon->ss[sn]);

	dungeon->ss[sn].t += (100 / s->s.s);

	Position new = {-1, -1};

	dungeon->d[s->p.y][s->p.x].h -= 85;
	if(dungeon->d[s->p.y][s->p.x].h < 0)
		dungeon->d[s->p.y][s->p.x].h = 0;

	if(s->a == TRUE) {
		int i;
		int j;
		int eb = rand() % 2; 
		for(i = 0; i < 8; i++) {
			int px = sx + xs[i];
			int py = sy + ys[i];

			if(px >= 0 && px < dungeon->w && py >= 0 && py < dungeon->h) {
				if(sn == dungeon->pc)
					goto PCEB;

				if(s->s.eb == FALSE || (s->s.eb == TRUE && eb)) {
					if(s->s.te == FALSE) {
						bool in_room = FALSE;
						with_pc(dungeon, s, &in_room);
						if(in_room == TRUE) {
							s->pc = dungeon->ss[dungeon->pc].p;

							IN: ;
							if(s->s.in == TRUE) {
								int k;
								int lowest = 0;
								bool set = FALSE;
								if(s->s.tu) {
									for(k = 0; k < 8; k++) {
										if(xs[k]+sx >= 0 && xs[k]+sx < dungeon->w && ys[k]+sy >= 0 && ys[k]+sy < dungeon->h) {
											if(dungeon->d[ys[k]+sy][xs[k]+sx].h < 255 && dungeon->cst[ys[k]+sy][xs[k]+sx] < dungeon->cst[ys[lowest]+sy][xs[lowest]+sx] && test_loc(dungeon, xs[k]+sx, ys[k]+sy, s) == TRUE && test_loc(dungeon, xs[lowest]+sx, ys[lowest]+sy, s) == TRUE) {
												lowest = k;
												set = TRUE;
											}
										}
									}
								} else {
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
							} else {
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
							goto PCEB;
						}
					} else {
						s->pc = dungeon->ss[dungeon->pc].p;
						goto IN;
					}

				} else {
					PCEB: ;
					j = 0;
					EB: ;
					int c = rand() % 9;
					px = xs[c] + sx;
					py = ys[c] + sy;
					if(test_loc(dungeon, px, py, s) == FALSE && j < 8) {
						j++;
						goto EB;
					}
					if(test_loc(dungeon, px, py, s) == TRUE) {
						new.x = px;
						new.y = py;
					}

					break;
				}
			}
		}
	}

	if(new.x < 0)
		new.x = sx;
	if(new.y < 0)
		new.y = sy;

	dungeon->ss[sn].to.x = new.x;
	dungeon->ss[sn].to.y = new.y;

	if(new.x == dungeon->ss[dungeon->pc].p.x && new.y == dungeon->ss[dungeon->pc].p.y)
		dungeon->go = TRUE;

	int i;
	for(i = 0; i < dungeon->ns; i++) {
		if(i != sn) {
			if(dungeon->ss[i].p.x == dungeon->ss[sn].to.x && dungeon->ss[i].p.y == dungeon->ss[sn].to.y && dungeon->ss[sn].a == TRUE)
				dungeon->ss[i].a = FALSE;
		}
	}

}

void parse_move(Dungeon * dungeon, int sn) {
	dungeon->ss[sn].p.x = dungeon->ss[sn].to.x;
	dungeon->ss[sn].p.y = dungeon->ss[sn].to.y;
}

Event gen_move_sprite_old(Dungeon * dungeon, int sn) {
	Event e;
	e.sn =0;
	e.speed =0;
	e.to.x =0;
	e.to.y = 0;
	e.turn =0;

	int xs[8] = {-1,0,1,1,1,0,-1,-1};
	int ys[8] = {-1,-1,-1,0,1,1,1,0};

    int sx = dungeon->ss[sn].p.x;
    int sy = dungeon->ss[sn].p.y;
	Sprite s = dungeon->ss[sn];
	int qp = -1;
	dungeon->ss[sn].t += (100 / s.s.s);

    int i;
    for(i = 0; i < 8; i++) {
        int x = sx + xs[i];
        int y = sy + ys[i];
        if(x > 0 && x < dungeon->w-1 && y > 0 && y < dungeon->h-1) {
            int hard = dungeon->d[y][x].h;
            if(hard < 255) {
				if(s.s.eb == TRUE) {
					int b = rand() % 2;
					if(b) {
						goto RP;
					}
				}

				if(hard > 0 && s.s.tu == FALSE) {
					continue;
				}

				if(s.s.te == FALSE) {
					if(dungeon->d[y][x].c == '.') {

					} else {
						goto RP;
					}

				} else {
					Sprite pc = dungeon->ss[dungeon->pc];
					if(sx < pc.p.x)
						x = sx + 1;
					if(sx > pc.p.x)
						x = sx - 1;
					if(sy < pc.p.y)
						y = sy + 1;
					if(sy > pc.p.y)
						y = sy - 1;

				}

				if(s.s.in == TRUE) {
					if(s.s.tu == TRUE) {
						if(dungeon->cst[ys[qp]+sy][xs[qp]+sx] > dungeon->cst[y][x] || qp == -1) {
							qp = i;
						}
					} else {
						if(dungeon->csnt[ys[qp]+sy][xs[qp]+sx] > dungeon->csnt[y][x] || qp == -1) {
							qp = i;
						}
					}
				} else {
					RP: ;
					qp = rand() % 9;
					break;
				}

            }
        }
    }
	return e;
}


void add_sprite(Dungeon * dungeon, Sprite s) {
	if(dungeon->ns < dungeon->ms) {
		dungeon->ns++;
	} else {
		goto END;
	}

	if(s.c == '@') {
		dungeon->pc = dungeon->ns - 1;
    }

	dungeon->ss[dungeon->ns - 1] = s;

	END: ;
}

Sprite gen_sprite(Dungeon * dungeon, char c, int x, int y, int r) {
	Sprite s;

	s.c = c;
	s.a = TRUE;

    if(s.c == '@') {
        s.s.s = 10;
        s.s.tu = FALSE;
		s.s.eb = FALSE;
		s.s.te = FALSE;
		s.s.in = FALSE;
	} else {
        s.s.s = (rand() % 16) + 5;
        s.s.in = rand() % 2;
        s.s.te = rand() % 2;
        s.s.tu = rand() % 2;
        s.s.eb = rand() % 2;

        if(s.s.in == FALSE && s.s.te == FALSE && s.s.tu == FALSE && s.s.eb == FALSE)
            s.c = '0';
        else if(s.s.in == FALSE && s.s.te == FALSE && s.s.tu == FALSE && s.s.eb == TRUE)
            s.c = '1';
        else if(s.s.in == FALSE && s.s.te == FALSE && s.s.tu == TRUE && s.s.eb == FALSE)
            s.c = '2';
        else if(s.s.in == FALSE && s.s.te == FALSE && s.s.tu == TRUE && s.s.eb == TRUE)
            s.c = '3';
        else if(s.s.in == FALSE && s.s.te == TRUE && s.s.tu == FALSE && s.s.eb == FALSE)
            s.c = '4';
        else if(s.s.in == FALSE && s.s.te == TRUE && s.s.tu == FALSE && s.s.eb == TRUE)
            s.c = '5';
        else if(s.s.in == FALSE && s.s.te == TRUE && s.s.tu == TRUE && s.s.eb == FALSE)
            s.c = '6';
        else if(s.s.in == FALSE && s.s.te == TRUE && s.s.tu == TRUE && s.s.eb == TRUE)
            s.c = '7';
        else if(s.s.in == TRUE && s.s.te == FALSE && s.s.tu == FALSE && s.s.eb == FALSE)
            s.c = '8';
        else if(s.s.in == TRUE && s.s.te == FALSE && s.s.tu == FALSE && s.s.eb == TRUE)
            s.c = '9';
        else if(s.s.in == TRUE && s.s.te == FALSE && s.s.tu == TRUE && s.s.eb == FALSE)
            s.c = 'a';
        else if(s.s.in == TRUE && s.s.te == FALSE && s.s.tu == TRUE && s.s.eb == TRUE)
            s.c = 'b';
        else if(s.s.in == TRUE && s.s.te == TRUE && s.s.tu == FALSE && s.s.eb == FALSE)
            s.c = 'c';
        else if(s.s.in == TRUE && s.s.te == TRUE && s.s.tu == FALSE && s.s.eb == TRUE)
            s.c = 'd';
        else if(s.s.in == TRUE && s.s.te == TRUE && s.s.tu == TRUE && s.s.eb == FALSE)
            s.c = 'e';
        else if(s.s.in == TRUE && s.s.te == TRUE && s.s.tu == TRUE && s.s.eb == TRUE)
            s.c = 'f';
    }

    if(s.s.tu == TRUE) {
		int t = 0;
		PRT: ;
        if(x < 0 || x > dungeon->w) {
            x = (rand() % (dungeon->w-2)) + 1;
        }
        if(y < 0 || y > dungeon->h) {
            y = (rand() % (dungeon->h-2)) + 1;
        }

		if(s.c != '@' && dungeon->nr > 1) {
			s.p.x = x;
		    s.p.y = y;

			bool w_pc = FALSE;
			with_pc(dungeon, &s, &w_pc);
			if(w_pc == TRUE && t < 8) {
				t++;
				goto PRT;
			}
		}
    }

    if(r > 0 || s.s.tu == FALSE) {
		int t = 0;
		PRNT: ;
        int r_id = rand() % dungeon->nr;
        x = (rand() % dungeon->r[r_id].w) + dungeon->r[r_id].tl.x;
        y = (rand() % dungeon->r[r_id].h) + dungeon->r[r_id].tl.y;

		if(s.c != '@' && dungeon->nr > 1) {
			s.p.x = x;
		    s.p.y = y;

			bool w_pc = FALSE;
			with_pc(dungeon, &s, &w_pc);
			if(w_pc == TRUE && t < 8) {
				t++;
				goto PRNT;
			}
		}
    } else {

    }

    s.p.x = x;
    s.p.y = y;
	s.t = 0;

	return s;
}

bool check_any_monsters(Dungeon * dungeon) {
	int i;
	for(i = 0; i < dungeon->ns; i++) {
		if(dungeon->ss[i].a == TRUE && i != 0)
			return TRUE;
	}

	return FALSE;
}
