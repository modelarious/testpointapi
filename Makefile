flags=-Wall -Wextra -Wpedantic -Wnull-dereference -Wfloat-equal -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code
save=-save-temps=obj -c
binLoc=bin
srcLoc=src/source
testLoc=src/tests
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	libs=-lpthread -lrt
endif
ifeq ($(UNAME_S),Darwin)
	libs=
endif

all: source tests

source: main
tests: apitests

inspect: inspect-apitests

apitests: $(testLoc)/apitests.c $(testLoc)/testpoint.c
	gcc $(flags) -o $(binLoc)/apitests $(testLoc)/apitests.c $(libs)

main: $(srcLoc)/main.c $(testLoc)/testpoint.c
	gcc $(flags) -o $(binLoc)/main $(srcLoc)/main.c $(libs)

inspect-apitests: $(testLoc)/apitests.c $(testLoc)/testpoint.c
	gcc $(flags) $(save) -o $(binLoc)/apitests $(testLoc)/apitests.c $(libs)

clean:
	rm -rf $(binLoc)/*