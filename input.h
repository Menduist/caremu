#ifndef INPUT_H
#define INPUT_H

#include <SFML/Window.h>

struct inputs {
	char key_pressed[sfKeyCount]; //Key pressed at this frame
	char key_down[sfKeyCount]; //Key currently pressed
	char key_released[sfKeyCount]; //Key released at this frame

	char mouse_button_pressed[sfMouseButtonCount]; //Button pressed at this frame
	char mouse_button_down[sfMouseButtonCount]; //Button currently pressed
	char mouse_button_released[sfMouseButtonCount]; //Button released at this frame

	int mouse_x;
	int mouse_y;
	int mouse_world_x;
	int mouse_world_y;
};

void update_inputs(struct game *game);

#endif
