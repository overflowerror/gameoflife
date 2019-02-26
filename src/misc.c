#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "misc.h"

const char* progName;

void initMisc(const char* _progName) {
	progName = _progName;
}

const char* getProgName() {
	return progName;
}

void error(int exitcode, const char* module, const char* error) {
	(void) fprintf(stderr, "%s: %s", progName, module);
	if (error != NULL) {
		(void) fprintf(stderr, ": %s", error);
	}
	if (errno != 0) {
		(void) fprintf(stderr, ": %s", strerror(errno));
	}
	(void) fprintf(stderr, "\n");
	exit(exitcode);
}
