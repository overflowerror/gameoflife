#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

#include "field.h"

typedef struct {
	bool border;
	const char* charset;
} cli_settings_t;

void cli_init(cli_settings_t);
void cli_output(field_t);
void cli_end();

#endif
