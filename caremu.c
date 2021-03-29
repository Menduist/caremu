#include "caremu.h"
#include "gpu.h"
#include "input.h"
#include "cars.h"
#include "maps.h"
#include <stdio.h>
#include <string.h>

#include <signal.h>

void sig_handler(int signo)
{
	(void)signo;
	signal(SIGINT, SIG_DFL);
	fprintf(stderr, "\n");
}

int init_display();

static void init_cars(struct game *game) {
	game->cars = calloc(game->cars_count, sizeof(struct car));
	int i = 0;
	while (i < game->cars_count) {
		game->cars[i].target = car_ia_get_random_node(game->map);
		game->cars[i].x = game->cars[i].target->x + 3 * i;
		game->cars[i].y = game->cars[i].target->y;
		game->cars[i].control = CAR_CONTROL_IA;
		i++;
	}
}

static void update_cars(struct game *game) {
	int i = 0;
	while (i < game->cars_count) {
		update_car(game, game->map, &game->cars[i], game->clock);
		i++;
	}
}

int main(int argc, char **argv)
{
	struct game game;
	char *target_map = strdup("maps/paris");

	game.cars_count = 4000;
	int opt;
	while ((opt = getopt(argc, argv, "m:c:")) != -1) {
		switch (opt) {
		case 'm':
			free(target_map);
			target_map = strdup(optarg);
			break;
		case 'c':
			game.cars_count = atoi(optarg);
			break;
		default:
			printf("Usage: %s [-m osm map file path (default %s)] [-c cars count (default %d)]\n",
				argv[0], target_map, game.cars_count);
			return -1;
		}
	}
	game.graphics = init_graphics();
	game.inputs = calloc(1, sizeof(struct inputs));
	game.map = map_parse_xml(target_map);
	game.speed = 1;
	init_cars(&game);

	sfClock *clock = sfClock_create();
	while (sfRenderWindow_isOpen(game.graphics->window)) {
		game.clock = sfTime_asMicroseconds(sfClock_restart(clock)) / 1000.0;
		update_inputs(&game);
		graphics_update(&game, game.graphics);
		game.clock *= game.speed;
		update_cars(&game);
		graphics_render(&game, game.graphics);
	}
}
