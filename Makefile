flags=-Wall -Wextra -Wpedantic -Wnull-dereference -Wfloat-equal -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code
save=-save-temps=obj -c
loc=bin

all: apitests

inspect: inspect-apitests

apitests: apitests.c testpoint.c
	gcc $(flags) -o $(loc)/apitests apitests.c


inspect-apitests: apitests.c testpoint.c
	gcc $(flags) $(save) -o $(loc)/apitests apitests.c

clean:
	rm -rf $(loc)/*
