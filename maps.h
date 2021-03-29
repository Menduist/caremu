#ifndef MAPS_H
#define MAPS_H

#include "libtca/vector.h"
#include "libtca/hashtable.h"

struct path {
	struct node *target;
	struct vector *path;
};

struct node {
	long id;
	double latitude;
	double longitude;

	int way_count;

	double x;
	double y;

	struct vector *neighbors;
	float *weights;
	struct vector *all_reachable;

	long gscore; //For astar
	long fscore; //For astar
	struct node *came_from; //For astar

	struct vector *paths;
};

struct way {
	struct vector nodes;
	enum {
		UNKNOWN,
		RIVER,
		ROAD
	} type;
	int is_closed;
	int is_oneway;
	double lane_count;
};

struct map {
	struct vector ways;
	struct hashtable *nodes;
	struct vector *nodes_of_ways;

	double min_x;
	double min_y;
	double max_x;
	double max_y;
};

struct map *map_parse_xml(char *path);
#include <SFML/Graphics.h>
void draw_map(sfRenderWindow *win, struct map *map);

struct node *map_get_closest_node(struct map *map, int x, int y);

#endif
