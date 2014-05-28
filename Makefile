CC = gcc
CFLAGS = -ansi -pedantic -Wall -Werror -O3 -g -std=c99

TESTS = $(wildcard *.c)

all: 
	$(CC) $(CFLAGS) $(TESTS) -lm -o parsing
	./parsing
  
clean:
	rm parsing
