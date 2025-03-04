# Copyright @lucabotez

# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -Wshadow -Wpedantic -std=c99 -O0 -g

# define targets
TARGETS=kNN

#define object-files
OBJ=kNN.o

build: $(TARGETS)

kNN: kNN.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGETS) $(OBJ)

.PHONY: clean
