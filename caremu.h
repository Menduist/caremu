#ifndef CAREMU_H
#define CAREMU_H

struct game {
	struct graphics *graphics;
	struct map *map;
	struct inputs *inputs;
	struct car *cars;
	double clock;
	double speed;
	int cars_count;
};

#endif
