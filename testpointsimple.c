#ifndef EOK
#define EOK 0
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int points, passes, fails, errors;
	
void simpletestnote(char * str) {
	printf("NOTE:\t%s\n", str);
}

void simpleteststart(char * str) {
	printf("START:\t%s\n", str);
	points = 0;
	passes = 0;
	fails = 0;
	errors = 0;
}

void simplepointstart(char * str) {
	printf("TEST:\t%s\n", str);
	points++;
}

void simplepointpass(char * str) {
	printf("PASS:\t%s\n", str);
	passes++;
}

void simplepointfail(char * str) {
	printf("FAIL:\t%s\n", str);
	fails++;
}

void simplepointerrormsg(char *str) {
	printf("ERROR:\t%s\n", str);
	errors++;
}

void simpletestend(char * str) {
	printf("END:\t%s\n", str);
	printf("Points: %d, Pass: %d, Fail: %d, Errors:%d\n",
		points,
		passes,
		fails,
		errors);
}
