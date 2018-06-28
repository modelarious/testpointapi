flags=-Wall -Wextra -Wpedantic -Wnull-dereference -Wfloat-equal -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code
save=-save-temps=obj -c
loc=bin
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	libs=-lpthread -lrt
endif
ifeq ($(UNAME_S),Darwin)
	libs=-lpthread
endif

all: apitests main

inspect: inspect-apitests

apitests: apitests.c testpoint.c
	gcc $(flags) -o $(loc)/apitests apitests.c $(libs)

main: main.c testpoint.c
	gcc $(flags) -o $(loc)/main main.c $(libs)

inspect-apitests: apitests.c testpoint.c
	gcc $(flags) $(save) -o $(loc)/apitests apitests.c $(libs)

clean:
	rm -rf $(loc)/*
