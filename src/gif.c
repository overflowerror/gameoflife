#ifdef GIF_SUPPORT

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gif_lib.h>

#include "gif.h"
#include "field.h"
#include "misc.h"

gif_settings_t gif_settings;

GifFileType* image;

void putGraficsControlBlock() {
	int delay = gif_settings.sleep;

	static unsigned char
	ExtStr[4] = { 0x04, 0x00, 0x00, 0xff };


	ExtStr[0] = (false) ? 0x06 : 0x04;
	ExtStr[1] = delay % 256;
	ExtStr[2] = delay / 256;

	EGifPutExtension(image, GRAPHICS_EXT_FUNC_CODE, 4, ExtStr);
}

void enableLoop() {
	char nsle[12] = "NETSCAPE2.0";
	char subblock[3];
	if (EGifPutExtensionLeader(image, APPLICATION_EXT_FUNC_CODE) != GIF_OK) {
		error(EXIT_PANIC, "gif_init", "put extension leader");
	}
	if (EGifPutExtensionBlock(image, 11, nsle) != GIF_OK) {
		error(EXIT_PANIC, "gif_init", "put extension");
	}
	subblock[0] = 1;
	subblock[2] = 0; // loop count lo
	subblock[1] = 0; // loop count hi
	if (EGifPutExtensionBlock(image, 3, subblock) != GIF_OK) {
		error(EXIT_PANIC, "gif_init", "put extension");
	}
	if (EGifPutExtensionTrailer(image) != GIF_OK) {
		error(EXIT_PANIC, "gif_init", "put extension trailer");
	}
}

void gif_init(gif_settings_t _settings) {
	gif_settings = _settings;

	int errorCode = 0;
	image = EGifOpenFileHandle(gif_settings.fh, &errorCode);

	if (errorCode != E_GIF_SUCCEEDED) {
		error(EXIT_PANIC, "gif_init", "open file handler");
	}

	GifColorType colors[4];
	for (int i = 0; i < 4; i++) {
		colors[i].Red = gif_settings.colors[i][0];
		colors[i].Green = gif_settings.colors[i][1];
		colors[i].Blue = gif_settings.colors[i][2];
	}

	ColorMapObject* colorMap = GifMakeMapObject(4, &(colors[0]));

	EGifSetGifVersion(image, true);

	if (EGifPutScreenDesc(image, gif_settings.dimensions.width, gif_settings.dimensions.height, 4, 0, colorMap) != GIF_OK) {
		error(EXIT_PANIC, "gif_init", "put screen desc");
	}

	enableLoop();
}

void gif_output(field_t field) {
	putGraficsControlBlock();

	if (EGifPutImageDesc(image, 0, 0, field.width, field.height, false, NULL) != GIF_OK) {
		error(EXIT_PANIC, "git_output", "put image desc");
	}

	for (int y = 0; y < field.height; y++) {
		for(int x = 0; x < field.width; x++) {
			cell_t cell = field.cells[x][y];
			if (cell == UNDEFINED)
				cell = DEAD;
			if (EGifPutPixel(image, (GifPixelType) cell) != GIF_OK) {
				error(EXIT_PANIC, "gif_output", "put pixel");
			}
		}
	}
}

void gif_end() {
	int errorCode;
	EGifCloseFile(image, &errorCode);

	if (errorCode != E_GIF_SUCCEEDED) {
		fprintf(stderr, "%d\n", errorCode);
		error(EXIT_PANIC, "gif_end", "close file");
	}
}

unsigned char hex2byte(const char* string, int offset) {
	char tmp[3];
	memcpy(&(tmp[0]), string + offset, 2);
	tmp[3] = '\0';

	char* endptr;	
	unsigned char result = strtol(tmp, &endptr, 16);

	return result;
}

void hex2color(unsigned char* color, const char* string) {
	if (strlen(string) == 7) {
		color[0] = hex2byte(string, 1);
		color[1] = hex2byte(string, 3);
		color[2] = hex2byte(string, 5);
	} else {
	}
}

void gif_setColor(gif_settings_t* settings, const char* dead, const char* died, const char* new, const char* alive) {
	hex2color(&(settings->colors[DEAD][0]), dead);
	hex2color(&(settings->colors[DIED][0]), died);
	hex2color(&(settings->colors[NEW][0]), new);
	hex2color(&(settings->colors[ALIVE][0]), alive);
}
#endif
