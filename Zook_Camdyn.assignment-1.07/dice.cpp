#include "dungeon_generator.h"
#include <cstdio>
#include <stdlib.h>
#include <string>
#include <sstream>

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()


int	Dice::roll(void) {
	if(d == 0)
		return b;
	else
		return b + n * ((rand() % d) + 1);
}

std::string Dice::string(void) {
	return SSTR(b) + "+" + SSTR(n) + "d" + SSTR(d);
}

Dice::Dice(int base, int num, int die) {
	b = base;
	n = num;
	d = die;
}


