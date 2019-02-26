#ifndef MISC_H
#define MISC_H

#define EXIT_PANIC 42
#define EXIT_SUCCESS 0
#define EXIT_USER_ERROR 1
#define EXIT_COMPILE_TIME_ERROR 5

void initMisc(const char*);
const char* getProgName();
void error(int, const char*, const char*);

#endif
