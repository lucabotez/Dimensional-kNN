# Copyright @lucabotez

# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -Wshadow -Wpedantic -std=c99 -O0 -g

# define targets
TARGETS=kNN

#define object-files
OBJ=kNN.o kd_tree.o

build: $(TARGETS)

kNN: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -lm

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGETS) $(OBJ)

.PHONY: clean
