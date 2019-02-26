#ifndef GIF_H
#define GIF_H

#include "field.h"

typedef struct {
	int fh;
	dimensions_t dimensions;
	unsigned char colors[4][3];
	int sleep;
} gif_settings_t;

void gif_init(gif_settings_t _settings);
void gif_output(field_t field);
void gif_setColor(gif_settings_t* settings, const char* dead, const char* died, const char* new, const char* alive);
void hex2color(unsigned char* color, const char* string);
void gif_end();

#endif
