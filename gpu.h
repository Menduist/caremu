#ifndef GPU_G
#define GPU_G

#include <SDL/SDL.h>

#include "maps.h"
struct graphics {
	sfRenderWindow *window;
	sfView *game_view;
	sfView *ui_view;
	sfFont *font;
	sfFloatRect viewport;
	double zoomlevel;
	double lod;

	struct ui *ui;
};

struct ui {
	struct car *selected_car;
};

struct graphics *init_graphics();
void graphics_render(struct game *, struct graphics *);
void graphics_update(struct game *, struct graphics *);
int graphics_is_in_viewport(struct graphics *, sfVector2f, int size);

#endif
