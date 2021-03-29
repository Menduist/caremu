#include "maps.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <math.h>

#define pixel_per_meters 55

xmlNode *get_sibling(xmlNode * a_node, char *name) {
	xmlNode *cur_node = NULL;
	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (strcmp((char*)cur_node->name, name) == 0) return cur_node;
	}
	return 0;
}

struct node *map_get_closest_node(struct map *map, int x, int y) {
	unsigned int i = 0;
	long mindist = LONG_MAX;
	struct node *closest = NULL;
	while (i < map->ways.current) {
		struct way *way = map->ways.data[i++];
		if (way->type != ROAD) {
			continue;
		}
		unsigned int z = 0;
		while (z < way->nodes.current) {
			struct node *node = way->nodes.data[z++];
			long distance_squared = (node->x - x) * (node->x - x) + (node->y - y) * (node->y - y);
			if (distance_squared < mindist) {
				closest = node;
				mindist = distance_squared;
			}
		}
	}
	return closest;
}

char *get_attr(xmlNode * a_node, char *name) {
	xmlAttr *cur_attr = NULL;
	for (cur_attr = a_node->properties; cur_attr; cur_attr = cur_attr->next) {
		if (strcmp((char*)cur_attr->name, name) == 0) return (char*)cur_attr->children->content;
	}
	return 0;
}

/*static void print_element_names(int i, xmlNode * a_node)
{
	xmlNode *cur_node = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			printf("%d node type: Element, name: %s\n", i, cur_node->name);
		}
		else if (cur_node->type == XML_ATTRIBUTE_DECL) {
			printf("%d node type: Attribute, name: %s\n", i, cur_node->name);
		} else {
			printf("%d node type: %d, name: %s\n", i, cur_node->type, cur_node->name);
		}

		xmlAttr *cur_attr;
		for (cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {
			printf("%d attr %s val %s\n", i, cur_attr->name, (char *)cur_attr->children->content);
		}

		print_element_names(i+1, cur_node->children);
	}
}*/

sfVertexArray *create_map_vertex(struct map *map) {
	float meters_per_lane = 3;
	
	sfVertexArray *result = sfVertexArray_create();
	sfVertexArray_setPrimitiveType(result, sfQuads);
	unsigned int i = 0;
	while (i < map->ways.current) {
		struct way *way = map->ways.data[i];
		if (way->type == UNKNOWN) {
			i++;
			continue;
		}
		sfVertex vertex;
		memset(&vertex, 0, sizeof(sfVertex));
		if (way->type == ROAD) {
			vertex.color = sfBlack;
		} else if (way->type == RIVER) {
			vertex.color = sfBlue;
		}
		
		unsigned int y = 1;
		while (y < way->nodes.current) {
			struct node *last_node = way->nodes.data[y - 1];
			struct node *node = way->nodes.data[y];
			//sfVector2f node_pos = {(node->x - map->min_x - xoffset) * ratio, (node->y - map->min_y - yoffset) * ratio};
			//sfVector2f last_node_pos = {(last_node->x - map->min_x - xoffset) * ratio, (last_node->y - map->min_y - yoffset) * ratio};
			sfVector2f node_pos = {node->x, node->y};
			sfVector2f last_node_pos = {last_node->x, last_node->y};

			//double distance = sqrt((node_pos.x - last_node_pos.x) * (node_pos.x - last_node_pos.x) + (node_pos.y - last_node_pos.y) * (node_pos.y - last_node_pos.y));
			double angle_rad = atan2(node_pos.y - last_node_pos.y, node_pos.x - last_node_pos.x);
			double width = meters_per_lane * way->lane_count * pixel_per_meters;
			int xxx = -sin(angle_rad) * width / 2.0;
			int yyy = cos(angle_rad) * width / 2.0;

			if (y > 1) {
				vertex.position = (sfVector2f){last_node_pos.x - xxx, last_node_pos.y - yyy};
				sfVertexArray_append(result, vertex);
				vertex.position = (sfVector2f){last_node_pos.x + xxx, last_node_pos.y + yyy};
				sfVertexArray_append(result, vertex);
			}

			vertex.position = (sfVector2f){last_node_pos.x + xxx, last_node_pos.y + yyy};
			sfVertexArray_append(result, vertex);
			vertex.position = (sfVector2f){node_pos.x + xxx, node_pos.y + yyy};
			sfVertexArray_append(result, vertex);

			vertex.position = (sfVector2f){node_pos.x - xxx, node_pos.y - yyy};
			sfVertexArray_append(result, vertex);
			vertex.position = (sfVector2f){last_node_pos.x - xxx, last_node_pos.y - yyy};
			sfVertexArray_append(result, vertex);
			
			if (y != way->nodes.current - 1) {
				vertex.position = (sfVector2f){node_pos.x + xxx, node_pos.y + yyy};
				sfVertexArray_append(result, vertex);
				vertex.position = (sfVector2f){node_pos.x - xxx, node_pos.y - yyy};
				sfVertexArray_append(result, vertex);
			}
			y++;
		}
		i++;
	}
	return result;
}

void draw_map(sfRenderWindow *win, struct map *map) {
	static sfVertexArray *arr = NULL;
	if (!arr) arr = create_map_vertex(map);
	sfRenderWindow_drawVertexArray(win, arr, NULL);
/*
	float pixel_per_meters = 55;
	float meters_per_lane = 3;
	float ratio = 111111 * pixel_per_meters;
	float xoffset = (map->max_x - map->min_x) / 2;
	float yoffset = (map->max_y - map->min_y) / 2;

	unsigned int i = 0;
	sfRectangleShape *rect = sfRectangleShape_create();
	sfRectangleShape_setSize(rect, (sfVector2f){1, 1});
	sfRectangleShape_setOrigin(rect, (sfVector2f){0.5, 0.5});

	sfCircleShape *circle = sfCircleShape_create();
	sfCircleShape_setRadius(circle, 0.5);
	sfCircleShape_setOrigin(circle, (sfVector2f){0.5, 0.5});
	while (i < map->ways.current) {
		struct way *way = map->ways.data[i];
		if (way->type == UNKNOWN) {
			i++;
			continue;
		}
		unsigned int y = 1;
		if (way->type == RIVER) {
			sfRectangleShape_setFillColor(rect, sfBlue);
			sfCircleShape_setFillColor(circle, sfBlue);
		} else {
			sfRectangleShape_setFillColor(rect, sfYellow);
			sfCircleShape_setFillColor(circle, sfYellow);
		}
		while (y < way->nodes.current) {
			struct node *last_node = way->nodes.data[y - 1];
			struct node *node = way->nodes.data[y];
			sfVector2f node_pos = {(node->x - map->min_x - xoffset) * ratio, (node->y - map->min_y - yoffset) * ratio};
			sfVector2f last_node_pos = {(last_node->x - map->min_x - xoffset) * ratio, (last_node->y - map->min_y - yoffset) * ratio};

			float distance = sqrt((node_pos.x - last_node_pos.x) * (node_pos.x - last_node_pos.x) + (node_pos.y - last_node_pos.y) * (node_pos.y - last_node_pos.y));
			sfRectangleShape_setScale(rect, (sfVector2f){meters_per_lane * way->lane_count * pixel_per_meters, distance});
			sfRectangleShape_setPosition(rect, (sfVector2f){(node_pos.x + last_node_pos.x) / 2.0, (node_pos.y + last_node_pos.y) / 2.0});
			sfRectangleShape_setRotation(rect, atan2(node_pos.y - last_node_pos.y, node_pos.x - last_node_pos.x) / 3.14159265359 * 180.0 + 90);

			//sfRenderWindow_drawRectangleShape(win, rect, NULL);
			if (y != way->nodes.current - 1) {
				sfCircleShape_setScale(circle, (sfVector2f){meters_per_lane * way->lane_count * pixel_per_meters, meters_per_lane * way->lane_count * pixel_per_meters});
				sfCircleShape_setPosition(circle, node_pos);
				//sfRenderWindow_drawCircleShape(win, circle, NULL);
			}
			y++;
		}
		i++;
	}
*/
}

static void map_center_and_scale(struct map *map) {
	float ratio = 111111 * pixel_per_meters;
	float meanx = 0;
	float meany = 0;
	int node_count = 0;

	int i = 0;
	while (i < map->nodes->size) {
		struct hashtable_elem *ptr = &map->nodes->table[i];
		while (ptr && ptr->used) {
			struct node *node = ptr->value;
			if (node_count == 0) {
				meanx = node->x;
				meany = node->y;
			} else {
				meanx = meanx - (meanx / node_count) + node->x / node_count;
				meany = meany - (meany / node_count) + node->y / node_count;
			}
			node_count++;
			ptr = ptr->next;
		}
		i++;
	}

	//float xoffset = (map->max_x - map->min_x) / 2;
	//float yoffset = (map->max_y - map->min_y) / 2;
	float xoffset = (meanx);
	float yoffset = (meany);


	i = 0;
	while (i < map->nodes->size) {
		struct hashtable_elem *ptr = &map->nodes->table[i];
		while (ptr && ptr->used) {
			struct node *node = ptr->value;
			node->x = (node->x - xoffset) * ratio;
			node->y = (node->y - yoffset) * ratio;
			ptr = ptr->next;
		}
		i++;
	}
}

static void map_build_node_neighbors(struct map *map) {
	unsigned int i = 0;
	while (i < map->ways.current) {
		struct way *way = map->ways.data[i++];
		if (way->type != ROAD) {
			continue;
		}
		unsigned int z = 0;
		while (z < way->nodes.current) {
			struct node *node = way->nodes.data[z];
			if (!node->neighbors) node->neighbors = vector_new();
			if (z > 0 && !way->is_oneway) {
				if (node != way->nodes.data[z - 1]) {
					vector_push_back(node->neighbors, way->nodes.data[z - 1]);
				}
			}
			if (z != way->nodes.current - 1) {
				if (node != way->nodes.data[z + 1]) {
					vector_push_back(node->neighbors, way->nodes.data[z + 1]);
				}
			}
			z++;
		}
	}
}

int is_equal(const void *a, const void *b) {
	return a - b;
}

static bool are_node_equals(struct node *a, struct node *b) {
	return a == b;
}

static struct vector *get_node_reachable(struct node *node) {
	struct vector *all_reachable = vector_new();
	struct vector *open = vector_new();
	vector_push_back(open, node);
	while (open->current > 0) {
		struct node *current = vector_pop_back(open);
		vector_push_back(all_reachable, current);
		unsigned int z = 0;
		while (z < current->neighbors->current) {
			struct node *neigh = current->neighbors->data[z++];
			if (vector_find(open, (bool (*)(void *, void *))are_node_equals, neigh) != NULL) {
				continue;
			}
			if (vector_find(all_reachable, (bool (*)(void *, void *))are_node_equals, neigh) != NULL) {
				continue;
			}
			if (neigh->all_reachable) return neigh->all_reachable;
			else neigh->all_reachable = all_reachable;
			vector_push_back(open, neigh);
		}
	}
	free(open->data);
	free(open);
	return all_reachable;
}

static void map_build_node_reachable(struct map *map) {
	unsigned int i = 0;
	while (i < map->ways.current) {
		struct way *way = map->ways.data[i++];
		if (way->type != ROAD) {
			continue;
		}
		unsigned int z = 0;
		while (z < way->nodes.current) {
			struct node *node = way->nodes.data[z];
			if (node->all_reachable == 0)
				node->all_reachable = get_node_reachable(node);
			z++;
		}
	}
}

static int map_set_weights(struct map *map) {
	unsigned int i = 0;
	int node_count = 0;
	while (i < map->ways.current) {
		struct way *way = map->ways.data[i++];
		if (way->type != ROAD) {
			continue;
		}
		unsigned int z = 0;
		while (z < way->nodes.current) {
			struct node *node = way->nodes.data[z];

			unsigned int y = 0;
			node->weights = calloc(node->neighbors->current, sizeof(float));
			while (y < node->neighbors->current) {
				struct node *neighbor = node->neighbors->data[y];
				node->weights[y] = sqrt((node->x - neighbor->x) * (node->x - neighbor->x) + (node->y - neighbor->y) * (node->y - neighbor->y));
				y++;

			}
			z++;
			node_count++;
		}
	}
	return node_count;
}

int cmp_ptr(const void *a, const void *b) {
	return a - b;
}

static void map_build_nodes_of_way(struct map *map) {
	unsigned int i = 0;
	map->nodes_of_ways = vector_new();
	while (i < map->ways.current) {
		struct way *way = map->ways.data[i++];
		if (way->type != ROAD) {
			continue;
		}
		unsigned int z = 0;
		while (z < way->nodes.current) {
			struct node *node = way->nodes.data[z++];
			vector_push_back(map->nodes_of_ways, node);
		}
	}
	qsort(map->nodes_of_ways->data, map->nodes_of_ways->current, sizeof(void *), cmp_ptr);
	vector_uniq(map->nodes_of_ways);

}

static void map_build_all_ways(struct map *map) {
	int node_count = map_set_weights(map);
	int node_done = 0;

	unsigned int i = 0;
	while (i < map->ways.current) {
		struct way *way = map->ways.data[i++];
		if (way->type != ROAD) {
			continue;
		}
		unsigned int z = 0;
		while (z < way->nodes.current) {
			struct node *node = way->nodes.data[z];
			node_done++;
			//node->paths = get_way(node);
			z++;
		}
		printf("%d/%d\n", node_done, node_count);
	}

	node_done = 0;
	node_count = 0;
	i = 0;
	while (i < map->nodes_of_ways->current) {
		struct node *node = map->nodes_of_ways->data[i];
		if (node->way_count > 1) node_done++;
		i++;
	}
	printf("Kikoo %d/%d\n", node_done, i);
}

struct map *map_parse_xml(char *path) {
	struct map *result = calloc(1, sizeof(struct map));
	result->nodes = create_hashtable(65536);
	vector_init(&result->ways);

	result->min_x = result->min_y = 10000;
	result->max_x = result->max_y = -10000;

	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;

	doc = xmlReadFile(path, NULL, 0);
	root_element = xmlDocGetRootElement(doc);
	//print_element_names(0, root_element);

	printf("Parsing\n");
	xmlNode *osm = get_sibling(root_element, "osm");

	xmlNode *it;
	for (it = osm->children; it; it = it->next) {
		if (it->type == XML_ELEMENT_NODE) {
			if (strcmp((char*)it->name, "node") == 0) {
				struct node *node = calloc(1, sizeof(struct node));
				node->id = atol(get_attr(it, "id"));
				node->latitude = atof(get_attr(it, "lat"));
				node->longitude = atof(get_attr(it, "lon"));

				float earth_diameter = 6372 * cos(node->latitude/180*3.14159265359) * 2;
				float earth_circumference = earth_diameter * 3.14159265359;
				float earth_circumference_at_equator = 40075.017;
				node->y = -node->latitude;
				node->x = node->longitude / (earth_circumference_at_equator / earth_circumference);

				if (node->x > result->max_x) result->max_x = node->x;
				if (node->x < result->min_x) result->min_x = node->x;
				if (node->y > result->max_y) result->max_y = node->y;
				if (node->y < result->min_y) result->min_y = node->y;

				hashtable_insert(result->nodes, &node->id, sizeof(long), node);
			} else if (strcmp((char*)it->name, "way") == 0) {
				struct way *way = calloc(1, sizeof(struct way));
				vector_init(&way->nodes);
				way->lane_count = 1;

				xmlNode *child;
				for (child = it->children; child; child = child->next) {
					if (child->type == XML_ELEMENT_NODE) {
						if (strcmp((char*)child->name, "nd") == 0) {
							long ref = atol(get_attr(child, "ref"));
							struct node *node = hashtable_get(result->nodes, &ref, sizeof(long));
							node->way_count++;
							vector_push_back(&way->nodes, node);
						}
						else if (strcmp((char*)child->name, "tag") == 0) {
							char *key = get_attr(child, "k");
							char *value = get_attr(child, "v");
							if (strcmp(key, "highway") == 0) {
								if (strcmp(value, "footway") != 0 &&
								    strcmp(value, "bridleway") != 0 &&
								    strcmp(value, "steps") != 0 &&
								    strcmp(value, "pedestrian") != 0 &&
								    strcmp(value, "cycleway") != 0 &&
								    strcmp(value, "path") != 0)
									way->type = ROAD;
								if (strcmp(value, "motorway") == 0)
									way->lane_count = 3;
								else if (strcmp(value, "trunk") == 0)
									way->lane_count = 2;
								else if (strcmp(value, "primary") == 0)
									way->lane_count = 2;
								else if (strcmp(value, "service") == 0)
									way->lane_count = 0.5;
							} else if (strcmp(key, "waterway") == 0 || strcmp(key, "water") == 0) {
								way->type = RIVER;
							} else if (strcmp(key, "oneway") == 0 && strcmp(value, "yes") == 0) {
								way->is_oneway = 1;
							}
						}
					}
				}
				if (vector_get_first(&way->nodes) == vector_get_last(&way->nodes)) {
					way->is_closed = 1;
				}
				if (!way->is_oneway) {
					way->lane_count *= 2;
				}
				vector_push_back(&result->ways, way);
			}
		}
	}

	printf("Centering\n");
	map_center_and_scale(result);
	printf("Nodes of way\n");
	map_build_nodes_of_way(result);
	printf("Neighboring\n");
	map_build_node_neighbors(result);
	printf("Reachabling\n");
	map_build_node_reachable(result);
	printf("All ways\n");
	map_build_all_ways(result);
	printf("Map finished\n");
	xmlFreeDoc(doc);

	xmlCleanupParser();
	return result;
}
