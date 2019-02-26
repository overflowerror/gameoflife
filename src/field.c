#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "misc.h"
#include "field.h"

settings_t settings;

int generationCount;
cell_t*** generations = NULL;

void initField(settings_t _settings) {
	settings = _settings;

	generationCount = settings.periodThreshold;
	if (generationCount < 1)
		generationCount = 2;
	generations = malloc(generationCount * sizeof(cell_t**));
	if (generations == NULL)
		error(EXIT_PANIC, "field_setup", NULL);
	
	for (int g = 0; g < generationCount; g++) {
		generations[g] = malloc(settings.dimensions.width * sizeof(cell_t*));
		if (generations[g] == NULL)
			error(EXIT_PANIC, "field_setup", NULL);

		for (int x = 0; x < settings.dimensions.width; x++) {
			generations[g][x] = malloc(settings.dimensions.height * sizeof(cell_t));
			if (generations[g][x] == NULL)
				error(EXIT_PANIC, "field_setup", NULL);
			for (int y = 0; y < settings.dimensions.height; y++) {
				generations[g][x][y] = g == 0 ? DEAD : UNDEFINED;
			}
		}
	}
}

inline cell_t normalize(cell_t cell) {
	if (cell == NEW)
		return ALIVE;
	if (cell == DIED)
		return DEAD;
	if (cell == UNDEFINED)
		return DEAD;
	return cell;
}

static inline cell_t normalizeForPeriodCheck(cell_t cell) {
	if (cell == UNDEFINED)
		return cell;
	return normalize(cell);
}

static inline int getBoundarySafeState(int x, int y, cell_t** field) {
	if (x < 0)
		return 0;
	if (y < 0)
		return 0;
	if (x >= settings.dimensions.width)
		return 0;
	if (y >= settings.dimensions.height)
		return 0;
	return normalize(field[x][y]) == ALIVE ? 1 : 0;
}
static inline int countNeighbors(int x, int y, cell_t** field) {
	int count = 0;
	
	count += getBoundarySafeState(x + 1, y - 1, field);
	count += getBoundarySafeState(x + 1, y, field);
	count += getBoundarySafeState(x + 1, y + 1, field);
	count += getBoundarySafeState(x, y + 1, field);
	count += getBoundarySafeState(x - 1, y + 1, field);
	count += getBoundarySafeState(x - 1, y, field);
	count += getBoundarySafeState(x - 1, y - 1, field);
	count += getBoundarySafeState(x, y - 1, field);

	return count;
}

static inline cell_t nextState(int count, cell_t current) {
	current = normalize(current);

	if (current == ALIVE) {
		switch (settings.rules.neighbors[count]) {
			case DIE:
				return DIED;
			case CREATE:
				return ALIVE;
			case KEEP:
				return ALIVE;
			case TOGGLE:
				return DIED;
			default:
				error(EXIT_PANIC, "field_nextState", "unknown case");
		}
	} else {
		switch (settings.rules.neighbors[count]) {
			case DIE:
				return DEAD;
			case CREATE:
				return NEW;
			case KEEP:
				return DEAD;
			case TOGGLE:
				return NEW;
			default:
				error(EXIT_PANIC, "field_nextState", "unknown case");
		}
	}
}

static inline int getPeriod() {
	if (settings.periodThreshold < 0)
		return -1;

	for (int g = 1; g < generationCount; g++) {
		bool diff = false;
		for (int x = 0; x < settings.dimensions.width; x++) {
			for (int y = 0; y < settings.dimensions.height; y++) {
				//printf("x: %i, y: %i, g: %i, %i, %i\n", x, y, g, normalizeForPeriodCheck(generations[0][x][y]), normalizeForPeriodCheck(generations[g][x][y]));
				if (normalizeForPeriodCheck(generations[0][x][y]) != normalizeForPeriodCheck(generations[g][x][y])) {
					diff = true;
					break;
				}
			}
			if (diff)
				break;
		}
		//printf("diff: %i\n", diff);
		if (!diff)
			return g;
	}

	return -1;
}

int nextGeneration() {
	for (int x = 0; x < settings.dimensions.width; x++)
		free(generations[generationCount - 1][x]);
	for (int g = generationCount - 2; g >= 0; g--) {
		memcpy(generations[g + 1], generations[g], settings.dimensions.width * sizeof(cell_t*));
	}
	for (int x = 0; x < settings.dimensions.width; x++) {
		generations[0][x] = malloc(settings.dimensions.height * sizeof(cell_t));
		if (generations[0][x] == NULL)
			error(EXIT_PANIC, "field_nextGeneration", NULL);
	
		for (int y = 0; y < settings.dimensions.height; y++) {
			generations[0][x][y] = nextState(countNeighbors(x, y, generations[1]), generations[1][x][y]);
		}
	}

	return getPeriod();
}

field_t getField() {
	return (field_t) {
		.height = settings.dimensions.height,
		.width = settings.dimensions.width,
		.cells = generations[0]
	};
}
