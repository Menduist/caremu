#ifndef CARS_H
#define CARS_H

#include "maps.h"
struct car {
	float rotation;
	float x;
	float y;
	float speed;

	float pedal;
	int reverse;
	float wheels;
	int blinking;
	int color;

	enum {
		CAR_CONTROL_NONE,
		CAR_CONTROL_IA,
		CAR_CONTROL_KEYBOARD,
		CAR_CONTROL_JOYSTICK
	} control;
	struct vector *way;
	struct node *target;

	struct vector *closed_set;
};

#include <SFML/Graphics.h>

void car_set_target(struct map *map, struct car *car, struct node *target);
void update_car(struct game *, struct map *map, struct car *car, double time);
void draw_car(struct graphics *graphics, struct car *car);
struct node *car_ia_get_random_node(struct map *map);

#endif
