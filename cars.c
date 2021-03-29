#include "gpu.h"
#include "caremu.h"
#include "cars.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_SPEED 3
#define ACCELERATION 0.004

struct node *car_ia_get_random_node(struct map *map) {
	struct way *way;
	do {
		way = map->ways.data[rand() % map->ways.current];
	} while (way->type != ROAD);
	return way->nodes.data[rand() % way->nodes.current];
}

static long node_distance_squared(struct node *a, struct node *b) {
	return (long)(a->x - b->x) * (long)(a->x - b->x) + (long)(a->y - b->y) * (long)(a->y - b->y);
}

static bool are_node_equals(struct node *a, struct node *b) {
	return a == b;
}

static struct vector *get_way(struct car *car, struct node *start, struct node *target) {
	//printf("From %ld to %ld\n", start->id, target->id);
	struct vector *closed_set = vector_new();

	struct vector open_set;
	vector_init(&open_set);
	vector_push_back(&open_set, start);
	start->gscore = 0;
	start->fscore = sqrt(node_distance_squared(start, target));

	while (open_set.current > 0 && closed_set->current < 5000) {
		struct node *current = NULL;
		int current_index = 0;
		unsigned int y = 0;
		while (y < open_set.current) {
			struct node *xxx = open_set.data[y];
			if (current == NULL || xxx->fscore < current->fscore) {
				current = xxx;
				current_index = y;
			}
			y++;
		}
		if (current == target) {
			struct vector *result = vector_new();
			struct node *it = current;
			while (it != start) {
				//printf("%ld, ", it->id);
				vector_push_back(result, it);
				it = it->came_from;
			}
			car->closed_set = closed_set;
			//printf("Found after %d\n", closed_set->current);
			return result;
		}
		vector_push_back(closed_set, current);
		vector_pop_index(&open_set, current_index);

		unsigned int neighbors_index = 0;
		while (neighbors_index < current->neighbors->current) {
			struct node *neigh = current->neighbors->data[neighbors_index++];
			if (vector_find(closed_set, (bool (*)(void *, void *))are_node_equals, neigh) != NULL) {
				continue;
			}

			int go_to_score = current->gscore + current->weights[neighbors_index];
			if (vector_find(&open_set, (bool (*)(void *, void *))are_node_equals, neigh) == NULL) {
				vector_push_back(&open_set, neigh);
			} else if (neigh->gscore >= go_to_score) {
				continue;
			}

			neigh->gscore = go_to_score;
			neigh->came_from = current;
			neigh->fscore = current->gscore + sqrt(node_distance_squared(neigh, target));
			if (neigh->fscore < 0) {
				printf("Setting neg\n");
			}
		}
	}
	printf("No way:( %d\n", closed_set->current);
	return NULL;
}

void car_set_target(struct map *map, struct car *car, struct node *target) {
	struct node *closest = map_get_closest_node(map, car->x, car->y);
	car->way = get_way(car, closest, target);
}

static void car_ia(struct map *map, struct car *car) {
	if (car->target == NULL) {
		if (car->way && car->way->current > 0) {
			car->target = vector_pop_back(car->way);
		} else {
			int i = 0;
			struct node *closest = map_get_closest_node(map, car->x, car->y);
			while ((closest->all_reachable && closest->all_reachable->current > 2) && ((car->way == NULL || car->way->current == 0) && i < 2)) {
				struct node *target = closest->all_reachable->data[rand() % closest->all_reachable->current];
				if (target != closest) {
					car->way = get_way(car, closest, closest->all_reachable->data[rand() % closest->all_reachable->current]);
					break;
				}
				i++;
			}
			if (car->way && car->way->current > 0) {
				car->target = vector_pop_back(car->way);
			} else {
				car->target = car_ia_get_random_node(map);
				car->x = car->target->x;
				car->y = car->target->y;
			}
		}
	}
	double distance = (car->x - car->target->x) * (car->x - car->target->x) + (car->y - car->target->y) * (car->y - car->target->y);
	car->pedal = 0.5;
	double target_angle = atan2(car->target->y - car->y, car->target->x - car->x) / 3.14159265359 * 180.;
	double angle_diff = target_angle - car->rotation;

	angle_diff += 3600;
	while (angle_diff > 360) angle_diff -= 360;

	if (angle_diff < 180) car->wheels = 1 * angle_diff / 30.0;
	else car->wheels = -1 * (360 - angle_diff) / 30.0;

	if (distance < 150 * 150) {
		car->target = NULL;
	}
	(void)map;
}

static void car_keyboard(struct map *map, struct car *car) {

}

void update_car(struct game *game, struct map *map, struct car *car, double time) {
	if (car->control == CAR_CONTROL_IA) car_ia(map, car);
	else if (car->control == CAR_CONTROL_KEYBOARD) car_keyboard(map, car);

	car->x += -sin((car->rotation - 90) / 180.0 * 3.14159265359) * car->speed * time;
	car->y += cos((car->rotation - 90) / 180.0 * 3.14159265359) * car->speed * time;

	if (car->pedal > 1) car->pedal = 1;
	if (car->pedal < -1) car->pedal = -1;
	if (car->wheels > 1) car->wheels = 1;
	if (car->wheels < -1) car->wheels = -1;
	car->rotation += car->wheels * car->speed * 0.3 * time;

	if (car->speed != 0.0) { //Friction
		if (car->speed > 0.0) {
			car->speed -= 0.001 * time;
		} else {
			car->speed += 0.001 * time;
		}
	}
	if (car->pedal > 0.0) { //Accelerating
		float wantedspeed = 0;
		if (car->reverse) {
			wantedspeed = MAX_SPEED * -car->pedal;
		} else {
			wantedspeed = MAX_SPEED * car->pedal;
		}
		float diff = wantedspeed - car->speed;
		if ((car->reverse && diff < 0) || (!car->reverse && diff > 0)) {
			if ((diff > 0.0 && diff > ACCELERATION * time)) {
				diff = ACCELERATION * time;
			} else if ((diff < 0.0 && diff < -ACCELERATION * time)) {
				diff = -ACCELERATION * time;
			}
			car->speed += diff;
		}
	}
	if (car->pedal < 0.0 && car->speed != 0.0) { //Braking
		if (car->speed > 0.0) {
			car->speed += car->pedal * 0.005 * time;
			if (car->speed < 0) car->speed = 0;
		} else {
			car->speed -= car->pedal * 0.005 * time;
			if (car->speed > 0) car->speed = 0;
		}
	}
}

void draw_car(struct graphics *graphics, struct car *car) {
	if (!graphics_is_in_viewport(graphics, (sfVector2f){car->x, car->y}, 200)) return;

	sfRenderWindow *win = graphics->window;
	static sfRectangleShape *body = 0;
	static sfRectangleShape *wheel = 0;
	static sfCircleShape *light = 0;
	static sfCircleShape *sidemirror = 0;
	static sfVertexArray *way = 0;
	static sfRectangleShape *windshield = 0;

	int base_size = 300;

	float car_width = base_size;
	float car_height = base_size / 2.1;

	float wheel_width = base_size / 5;
	float wheel_height = base_size / 10;
	if (!body) {
		body = sfRectangleShape_create();
		sfRectangleShape_setFillColor(body, sfColor_fromRGBA(255, 255, 255, 200));
		sfRectangleShape_setOrigin(body, (sfVector2f){car_width / 2, car_height / 2});
		sfRectangleShape_setSize(body, (sfVector2f){car_width, car_height});

		wheel = sfRectangleShape_create();
		sfRectangleShape_setSize(wheel, (sfVector2f){wheel_width, wheel_height});
		sfRectangleShape_setOrigin(wheel, (sfVector2f){wheel_width / 2, wheel_height / 2});
		sfRectangleShape_setFillColor(wheel, sfBlack);
		sfRectangleShape_setOutlineColor(wheel, sfBlack);
		sfRectangleShape_setOutlineThickness(wheel, -5);

		light = sfCircleShape_create();
		sfCircleShape_setRadius(light, 9);
		sfCircleShape_setScale(light, (sfVector2f){0.5, 1});

		windshield = sfRectangleShape_create();
		sfRectangleShape_setOrigin(windshield, (sfVector2f){car_width * 0.2 / 2, car_height * 0.8 / 2});
		sfRectangleShape_setSize(windshield, (sfVector2f){car_width * 0.2, car_height * 0.8});
		sfRectangleShape_setFillColor(windshield, sfColor_fromRGBA(0, 0, 255, 200));

		sidemirror = sfCircleShape_create();
		sfCircleShape_setRadius(sidemirror, 8);
		sfCircleShape_setOrigin(sidemirror, (sfVector2f){8, 8});
		sfCircleShape_setScale(sidemirror, (sfVector2f){0.5, 1});

		way = sfVertexArray_create();
		sfVertexArray_setPrimitiveType(way, sfLineStrip);
	}

	sfTransform transform = sfTransform_Identity;
	sfTransform_translate(&transform, car->x, car->y);
	sfTransform_rotate(&transform, car->rotation);

	if (graphics->lod < 10) {
		sfRectangleShape_setRotation(wheel, car->rotation + car->wheels * 40.0);

		sfRectangleShape_setPosition(wheel, sfTransform_transformPoint(&transform, (sfVector2f){car_width / 2 - wheel_width / 2, car_height / 2 - wheel_height / 2}));
		sfRenderWindow_drawRectangleShape(win, wheel, NULL);

		sfRectangleShape_setPosition(wheel, sfTransform_transformPoint(&transform, (sfVector2f){car_width / 2 - wheel_width / 2, -car_height / 2 + wheel_height / 2}));
		sfRenderWindow_drawRectangleShape(win, wheel, NULL);

		sfRectangleShape_setRotation(wheel, car->rotation + car->wheels * -5.0);
		sfRectangleShape_setPosition(wheel, sfTransform_transformPoint(&transform, (sfVector2f){-car_width / 2 + wheel_width / 2, car_height / 2 - wheel_height / 2}));
		sfRenderWindow_drawRectangleShape(win, wheel, NULL);

		sfRectangleShape_setPosition(wheel, sfTransform_transformPoint(&transform, (sfVector2f){-car_width / 2 + wheel_width / 2, -car_height / 2 + wheel_height / 2}));
		sfRenderWindow_drawRectangleShape(win, wheel, NULL);
	}

	//Body
	sfRectangleShape_setPosition(body, (sfVector2f){car->x, car->y});
	sfRectangleShape_setRotation(body, car->rotation);
	sfRenderWindow_drawRectangleShape(win, body, NULL);

	if (graphics->lod < 25) {
		//Rear light
		sfCircleShape_setRotation(light, car->rotation);
		if (car->pedal < 0) {
			sfCircleShape_setFillColor(light, sfColor_fromRGB(255, 0, 0));
		} else {
			sfCircleShape_setFillColor(light, sfColor_fromRGB(110, 0, 0));
		}

		sfCircleShape_setPosition(light, sfTransform_transformPoint(&transform, (sfVector2f){-car_width / 2, 5}));
		sfRenderWindow_drawCircleShape(win, light, NULL);

		if (car->reverse) {
			sfCircleShape_setFillColor(light, sfWhite);
		}
		sfCircleShape_setPosition(light, sfTransform_transformPoint(&transform, (sfVector2f){-car_width / 2, -5 - 20}));
		sfRenderWindow_drawCircleShape(win, light, NULL);
	}

	if (graphics->lod < 15) {
		//Side mirror
		sfCircleShape_setRotation(sidemirror, car->rotation + 30);
		sfCircleShape_setPosition(sidemirror, sfTransform_transformPoint(&transform, (sfVector2f){car_width / 2 * 0.6, car_height / 2 + 7}));
		sfRenderWindow_drawCircleShape(win, sidemirror, NULL);
		sfCircleShape_setRotation(sidemirror, car->rotation - 30);
		sfCircleShape_setPosition(sidemirror, sfTransform_transformPoint(&transform, (sfVector2f){car_width / 2 * 0.6, -car_height / 2 - 7}));
		sfRenderWindow_drawCircleShape(win, sidemirror, NULL);
	}

	//Windshield
	sfRectangleShape_setPosition(windshield, sfTransform_transformPoint(&transform, (sfVector2f){35, 0}));
	sfRectangleShape_setRotation(windshield, car->rotation);
	sfRenderWindow_drawRectangleShape(win, windshield, NULL);

	if (graphics->ui->selected_car == car) {
		sfVertex vert;
		vert.color = sfRed;
		sfVertexArray_clear(way);
		unsigned int y = 0;
		while (car->way && y < car->way->current) {
			struct node *node = car->way->data[y];
			vert.position.x = node->x;
			vert.position.y = node->y;
			sfVertexArray_append(way, vert);
			y++;
		}
		if (car->target) {
			vert.position.x = car->target->x;
			vert.position.y = car->target->y;
			sfVertexArray_append(way, vert);
		}
		vert.position.x = car->x;
		vert.position.y = car->y;
		sfVertexArray_append(way, vert);
		sfRenderWindow_drawVertexArray(win, way, NULL);
	}
}
