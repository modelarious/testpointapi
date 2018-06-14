#ifndef EOK
#define EOK 0
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define simple_point_generic(apiMessage) \
	va_list arg; \
	va_start (arg, str); \
	printf("__%s:\t", apiMessage); \
	vfprintf(stdout, str, arg); \
	printf("\n"); \
	va_end(arg)

int points, passes, fails, errors;

void simpleteststart(char * str, ...) {
	simple_point_generic("STRT");
	points = 0;
	passes = 0;
	fails = 0;
	errors = 0;
}

void simpletestnote(char * str, ...) {
	simple_point_generic("NOTE");
}

void simplepointstart(char * str, ...) {
	simple_point_generic("TEST");
	points++;
}

void simplepointpass(char * str, ...) {
	simple_point_generic("PASS");
	passes++;
}

void simplepointfail(char * str, ...) {
	simple_point_generic("FAIL");
	fails++;
}

void simplepointerrormsg(char *str, ...) {
	simple_point_generic("ERROR");
	errors++;
}

void simpletestend(char * str, ...) {
	simple_point_generic("END");
	printf("__Points: %d, Pass: %d, Fail: %d, Errors:%d\n",
		points,
		passes,
		fails,
		errors);
}
