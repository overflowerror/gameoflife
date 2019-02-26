#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "misc.h"
#include "field.h"
#include "cli.h"
#include "gif.h"

char behaviourMap[4] = {
	'd', 'c', 'k', 't'
};

void optionError(const char* string) {
	fprintf(stderr, "Error: %s\n", string);
	fprintf(stderr, "User -y for help.\n");
	exit(EXIT_USER_ERROR);
}

void rules2string(char* string, rules_t rules) {
	for (int i = 0; i < 9; i++) {
		string[i] = behaviourMap[rules.neighbors[i]];
	}
	string[10] = '\0';
}

rules_t string2rules(const char* string) {
	if (strlen(string) != 9) {
		optionError("Rules string malformed");
	}

	rules_t rules;

	for(int i = 0; i < strlen(string); i++) {
		bool okay = false;
		for (int j = 0; j < 4; j++) {
			if (string[i] == behaviourMap[j]) {
				rules.neighbors[i] = j;
				okay = true;
				break;
			}
		}
		if (!okay) {
			optionError("Rules string malformed");
		}
	}

	return rules;
}

void help() {
	printf("SYNOPSIS: %s [-y] [-c [-lbn] [-s TIME] [-t CHARSET]]\n"
	                 "%*s%s[-p LIMIT] [-r RULES] [-h HEIGHT -w WIDTH] FILE\n\n", getProgName(), strlen(getProgName()), "", 
		#ifdef GIF_SUPPORT
			"[-g [-s TIME] [{-o COLOR}]] "
		#else
			""
		#endif
	);
	printf("OPTIONS:\n");
	printf("  -y         show help\n");
	printf("  -c         use CLI output (default)\n");
	printf("  -g         use GIF output\n");
	printf("  -p LIMIT   the search limit for periodes (default: %i)\n", DEFAULT_PERIOD_THRESHOLD);
	char tmp[11];
	rules2string(tmp, DEFAULT_RULES);
	printf("  -r RULES   set custom rules (default: %s)\n", tmp);
	printf("  -h HEIGHT  override the height of the output\n");
	printf("  -w WIDTH   override the width of the output\n");
	printf("     FILE    the input file (- for stdin)\n");
	printf("\n");

	printf("CLI OPTIONS:\n");
	printf("  -l         loop the output even if periodes occure\n");
	printf("  -b         draw border (defaul)\n");
	printf("  -n         don't draw border\n");
	printf("  -s TIME    sleep in ms (default: 200)\n");
	printf("  -t CHARSET charset for drawing (default: \"  **\")\n");
	printf("\n");

	#ifdef GIF_SUPPORT
	printf("GIF OPTIONS:\n");
	printf("  -s TIME    sleep in 1/10 s (default: 20)\n");
	printf("  -o COLOR   colors to use\n");
	printf("             COLOR := DEAD|DIED|NEW|ALIVE:HEX_COLOR\n");
	printf("             HEX_COLOR := #[a-f0-9]{6}\n");
	printf("             (defaults: DEAD/DIED = #000000; NEW/ALIVE = #ffffff)\n");
	printf("\n");
	#endif

	printf("INPUT FORMAT:\n");
	printf("[#WIDTH,HEIGHT;]{X,Y;}\n\n");
	printf("     WDITH   the width of the output\n");
	printf("     HEIGHT  the width of the output\n");
	printf("     X       the x coordinate of a cell\n");
	printf("     Y       the y coordinate of a cell\n\n");
	printf("Example:\n");
	printf("#3,3;1,0;1,2;1,3;\n");
	exit(EXIT_SUCCESS);
}

int readCoordinates(FILE* file, int* x, int* y) {
	int c;
	char tmp[10];
	char* endptr;
	int position = 0;
	*x = -1;
	*y = -1;
	bool has = false;
	while((c = getc(file)) != EOF) {
		switch(c) {
			case ',':
				tmp[position] = '\0';
				position = 0;
				*x = strtol(tmp, &endptr, 10);
				if (*endptr != '\0') {
					return -1;
				}
				break;
			case ';':
				tmp[position] = '\0';
				position = 0;
				*y = strtol(tmp, &endptr, 10);
				if (*endptr != '\0') {
					return -1;
				}
				if ((*x < 0) || (*y < 0))
					return -1;
				return 0;
			case ' ':
			case '\n':
			case '\t':
				break;
			default:					
				has = true;
				if (position > 9) {
					return -1;
				}
				tmp[position++] = c;
				break;
		}
	}
	if (!has)
		return 1;
	return -1;
}

void setSize(FILE* file, int* width, int* height) {
	int c = getc(file);
	if (c != '#') {
		ungetc(c, file);
		return;
	}

	c = readCoordinates(file, width, height);

	if (c != 0) {
		error(EXIT_USER_ERROR, "setSize", "malformed file format");
	}
}

void setField(FILE* file, field_t field) {
	int r, x, y;
	
	while((r = readCoordinates(file, &x, &y)) == 0) {
		if ((x < 0) || (y < 0) || (x >= field.width) || (y >= field.height)) {
			error(EXIT_USER_ERROR, "setField", "coordinates out of bounds");
		}
		field.cells[x][y] = ALIVE;
	}

	if (r < 0) {
		error(EXIT_USER_ERROR, "setField", "malformed input format");
	}
};

void parseColor(gif_settings_t* settings, char* optarg) {
	char* color = NULL;
	int celltype = -1;
	for (int i = 0; i < strlen(optarg); i++) {
		if (optarg[i] == ':') {
			color = optarg + i + 1;
			optarg[i] = '\0';
			break;
		}
	}

	if (color == NULL) {
		optionError("color format malformed");
	}

	if (strcmp(optarg, "DEAD") == 0) {
		celltype = DEAD;
	} else if (strcmp(optarg, "DIED") == 0) {
		celltype = DIED;
	} else if (strcmp(optarg, "NEW") == 0) {
		celltype = NEW;
	} else if (strcmp(optarg, "ALIVE") == 0) {
		celltype = ALIVE;
	} else {
		optionError("unknown cell type");
	}

	hex2color(&(settings->colors[celltype][0]), color);
}

int main(int argc, char** argv) {
	initMisc(argv[0]);

	bool gif = false;
	bool loop = false;
	int width = -1;
	int height = -1;

	const char* filename;
	FILE* file;

	long sleep = -1;

	settings_t settings;
	settings.dimensions.width = -1;
	settings.dimensions.height = -1;
	settings.rules = DEFAULT_RULES;
	settings.periodThreshold = DEFAULT_PERIOD_THRESHOLD;

	cli_settings_t cli_settings;
	cli_settings.border = true;
	cli_settings.charset = "  **";

	gif_settings_t gif_settings;
	const char* gif_filename = "-";
	gif_setColor(&gif_settings, "#000000", "#000000", "#ffffff", "#ffffff");

	int opt;	
	char* endptr;

	while((opt = getopt(argc, argv, "ycgp:r:h:w:lbns:t:o:")) != -1) {
		switch(opt) {
			case 'y':
				help();
				break;
			case 'c':
				gif = false;
				break;
			case 'g':
				gif = true;
				break;
			case 'p':
				settings.periodThreshold = strtol(optarg, &endptr, 10);
				if (*endptr != '\0') {
					optionError("Argument has to be a number.");
				}
				break;
			case 'r':
				settings.rules = string2rules(optarg);
				break;
			case 'h':
				settings.dimensions.height = strtol(optarg, &endptr, 10);
				if (*endptr != '\0') {
					optionError("Argument has to be a number.");
				}
				break;
			case 'w':
				settings.dimensions.width = strtol(optarg, &endptr, 10);
				if (*endptr != '\0') {
					optionError("Argument has to be a number.");
				}
				break;

			case 'l':
				loop = true;
				break;
			case 'b':
				cli_settings.border = true;
				break;
			case 'n':
				cli_settings.border = false;
				break;
			case 's':
				sleep = strtol(optarg, &endptr, 10);
				if (*endptr != '\0') {
					optionError("Argument has to be a number.");
				}
				break;
			case 't':
				if (strlen(optarg) != 4) {
					optionError("Argument needs to be length 4");
				}
				cli_settings.charset = optarg;
				break;
			case 'o':
				parseColor(&gif_settings, optarg);
				break;
			default:
				optionError("unknown option");
				break;
		}
	}

	if (optind >= argc) {
		optionError("No file given.");
	}

	filename = argv[optind];

	if (strcmp("-", filename) == 0) {
		file = stdin;
	} else {
		file = fopen(filename, "r");
		if (file == NULL)
			error(EXIT_USER_ERROR, "main", NULL);
	}	

	setSize(file, &width, &height);

	if ((width < 0) || (height < 0)) {
		optionError("Width or height missing");
	}
	
	settings.dimensions.width = width;
	settings.dimensions.height = height;

	initField(settings);

	setField(file, getField());

	if (gif) {
		#ifndef GIF_SUPPORT
			error(EXIT_COMPILE_TIME_ERROR, "main", "no GIF support");
		#else

		if (sleep < 0)
			sleep = 20;

		if (strcmp(gif_filename, "-") == 0) {
			freopen(NULL, "wb", stdout);

			gif_settings.fh = fileno(stdout);
		} else {
			gif_settings.fh = open(gif_filename, O_WRONLY | O_CREAT | O_TRUNC);

			if (gif_settings.fh < 0) {
				error(EXIT_USER_ERROR, gif_filename, NULL);
			}
		}

		gif_settings.dimensions = settings.dimensions;
		gif_settings.sleep = sleep;

		gif_init(gif_settings);

		gif_output(getField());

		int period;
		while(((period = nextGeneration()) < 0)) {
			gif_output(getField());
		}
			
		gif_end();

		#endif
	} else {		
		if (sleep < 0)
			sleep = 200;

		cli_init(cli_settings);

		cli_output(getField());

		int period;
		while(((period = nextGeneration()) < 0) || loop) {
			usleep(sleep * 1000);
			cli_output(getField());
		}
		
		cli_output(getField());
		
		cli_end();
	}

	return EXIT_SUCCESS;
}
