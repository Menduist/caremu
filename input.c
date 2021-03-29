#include "caremu.h"
#include "gpu.h"
#include "input.h"

void update_inputs(struct game *game) {
	struct inputs *inputs = game->inputs;

	memset(inputs->key_pressed, 0, sizeof(char) * sfKeyCount);
	memset(inputs->key_released, 0, sizeof(char) * sfKeyCount);
	memset(inputs->mouse_button_pressed, 0, sizeof(char) * sfMouseButtonCount);
	memset(inputs->mouse_button_released, 0, sizeof(char) * sfMouseButtonCount);

	sfEvent event;
	while (sfRenderWindow_pollEvent(game->graphics->window, &event))
	{
		if (event.type == sfEvtClosed)
			sfRenderWindow_close(game->graphics->window);
		else if (event.type == sfEvtKeyPressed) {
			inputs->key_pressed[event.key.code] = 1;
			inputs->key_down[event.key.code] = 1;
		}
		else if (event.type == sfEvtKeyReleased) {
			inputs->key_released[event.key.code] = 1;
			inputs->key_down[event.key.code] = 0;
		}
		else if (event.type == sfEvtMouseButtonPressed) {
			inputs->mouse_button_pressed[event.mouseButton.button] = 1;
			inputs->mouse_button_down[event.mouseButton.button] = 1;
		}
		else if (event.type == sfEvtMouseButtonPressed) {
			inputs->mouse_button_released[event.mouseButton.button] = 1;
			inputs->mouse_button_down[event.mouseButton.button] = 0;
		}
		else if (event.type == sfEvtResized) {
			sfVector2f current_size = sfView_getSize(game->graphics->game_view);
			double ratio = (double)event.size.width / (double)event.size.height;
			current_size.y = (double)current_size.x / ratio;
			sfView_setSize(game->graphics->game_view, current_size);
			current_size.x = event.size.width;
			current_size.y = event.size.height;
			sfView_setSize(game->graphics->ui_view, current_size);
		}
	}
	sfVector2i mousepos = sfMouse_getPositionRenderWindow(game->graphics->window);
	inputs->mouse_x = mousepos.x;
	inputs->mouse_y = mousepos.y;
	sfVector2f mousepos_world = sfRenderWindow_mapPixelToCoords(game->graphics->window, mousepos, game->graphics->game_view);
	inputs->mouse_world_x = mousepos_world.x;
	inputs->mouse_world_y = mousepos_world.y;
}
