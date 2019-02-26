#include <stdio.h>
#include <stdbool.h>

#include "cli.h"
#include "field.h"

cli_settings_t cli_settings;

void cli_init(cli_settings_t _settings) {
	cli_settings = _settings;
}

int height;
void cli_output(field_t field) {
	height = field.height;
	if (cli_settings.border)
		for (int x = 0; x < field.width + 2; x++) {
			printf("#");
		}
	printf("\n");
	for (int y = 0; y < field.height; y++) {
		if (cli_settings.border)
			printf("#");
		for (int x = 0; x < field.width; x++) {
			cell_t cell = field.cells[x][y];
			if (cell == UNDEFINED)
				cell = DEAD;
			printf("%c", cli_settings.charset[cell]);
		}
		if (cli_settings.border)
			printf("#");
		printf("\n");
	}
	if (cli_settings.border)
		for (int x = 0; x < field.width + 2; x++) {
			printf("#");
		}
	printf("\n\033[%iA", height + (cli_settings.border ? 2 : 0));
}

void cli_end() {
	printf("\33[%iB", height + (cli_settings.border ? 2 : 0));
}
