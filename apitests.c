#include "testpointsimple.c"

int unit_test___printf() {
	return EXIT_SUCCESS;
}

int main() {
	simpleteststart("test point api test");
	
	unit_test___printf();
	
	simpletestend("test point api test");
}