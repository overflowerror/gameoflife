#ifndef FIELD_H
#define FIELD_H

#include <stdbool.h>

typedef enum {
	DEAD = 0, DIED = 1, NEW = 2, ALIVE = 3, UNDEFINED = -1
} cell_t;

typedef enum {
	DIE = 0, CREATE = 1, KEEP = 2, TOGGLE = 3
} behaviour_t;

typedef struct {
	behaviour_t neighbors[9];
} rules_t;

typedef struct {
	int height;
	int width;
} dimensions_t;

typedef struct {
	rules_t rules;
	dimensions_t dimensions;
	int periodThreshold;
} settings_t;

typedef struct {
	int height;
	int width;
	cell_t** cells;
} field_t;

#define DEFAULT_RULES ((rules_t) { .neighbors = {DIE, DIE, KEEP, CREATE, DIE, DIE, DIE, DIE, DIE}})
#define DEFAULT_PERIOD_THRESHOLD (30)

void initField(settings_t);
int nextGeneration();
field_t getField();
cell_t normalize(cell_t);

#endif
