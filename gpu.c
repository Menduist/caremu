#include "caremu.h"
#include "cars.h"
#include "maps.h"
#include "gpu.h"
#include "input.h"
#include <math.h>
#include <time.h>
#include <SFML/Graphics.h>

struct graphics *init_graphics()
{
	struct graphics *result = calloc(1, sizeof(struct graphics));
	srand(time(NULL));

	sfVideoMode mode = {1280, 720, 32};

	sfContextSettings settings;
	memset(&settings, 0, sizeof(settings));
	settings.antialiasingLevel = 16;

	result->window = sfRenderWindow_create(mode, "Caremu", sfResize | sfClose, &settings);

	sfRenderWindow_setFramerateLimit(result->window, 0);

	sfFloatRect visibleArea = {0.f, 0.f, 1280 * 10, 720 * 10};
	result->game_view = sfView_createFromRect(visibleArea);
	result->ui_view = sfView_createFromRect(visibleArea);
	result->font = sfFont_createFromFile("sansation.ttf");

	result->ui = calloc(1, sizeof(struct ui));

	return result;
}

void graphics_update(struct game *game, struct graphics *graphics) {
	if (game->inputs->key_down[sfKeyDash]) {
		sfView_zoom(graphics->game_view, 1 + .003 * game->clock);
	}
	if (game->inputs->key_down[sfKeyEqual]) {
		sfView_zoom(graphics->game_view, 1 - .003 * game->clock);
	}
	if (game->inputs->key_pressed[sfKeyEscape]) {
		exit(0);
	}
	float speed = sfView_getSize(graphics->game_view).x / 1000.0 * game->clock;
	if (game->inputs->key_down[sfKeyLeft]) {
		sfView_move(graphics->game_view, (sfVector2f){-speed, 0});
	}
	if (game->inputs->key_down[sfKeyRight]) {
		sfView_move(graphics->game_view, (sfVector2f){speed, 0});
	}
	if (game->inputs->key_down[sfKeyUp]) {
		sfView_move(graphics->game_view, (sfVector2f){0, -speed});
	}
	if (game->inputs->key_down[sfKeyDown]) {
		sfView_move(graphics->game_view, (sfVector2f){0, speed});
	}

	if (game->inputs->mouse_button_pressed[sfMouseLeft]) {
		graphics->ui->selected_car = 0;
		int i = 0;
		while (i < game->cars_count) {
			struct car *car = &game->cars[i];
			if ((car->x - game->inputs->mouse_world_x) * (car->x - game->inputs->mouse_world_x) +
				(car->y - game->inputs->mouse_world_y) * (car->y - game->inputs->mouse_world_y) < 200 * 200) {
				graphics->ui->selected_car = car;
				break;
			}
			i++;
		}
	}
}

int graphics_is_in_viewport(struct graphics *graphics, sfVector2f position, int size) {
	(void)size; //TODO use size
	return sfFloatRect_contains(&graphics->viewport, position.x, position.y);
}

void graphics_render(struct game *game, struct graphics *graphics) {
	/* Clear the screen */
	sfRenderWindow_clear(graphics->window, sfColor_fromRGB(11, 102, 35));
	sfRenderWindow_setView(graphics->window, graphics->game_view);

	sfVector2f center = sfView_getCenter(graphics->game_view);
	sfVector2f size = sfView_getSize(graphics->game_view);
	graphics->viewport.left = center.x - size.x / 2 - 300;
	graphics->viewport.top = center.y - size.y / 2 - 300;
	graphics->viewport.width = size.x + 600;
	graphics->viewport.height = size.y + 600;

	graphics->lod = 1.0 / ((sfRenderWindow_mapCoordsToPixel(graphics->window, (sfVector2f){100000.0, 0}, graphics->game_view).x - sfRenderWindow_mapCoordsToPixel(graphics->window, (sfVector2f){0, 0}, graphics->game_view).x) / 100000.0);


	draw_map(graphics->window, game->map);

	int i = 0;
	while (i < game->cars_count) {
		draw_car(graphics, &game->cars[i]);
		i++;
	}

	sfRenderWindow_setView(graphics->window, graphics->ui_view);

	/* Update the window */
	sfRenderWindow_display(graphics->window);
}
