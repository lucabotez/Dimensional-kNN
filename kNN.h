// Copyright @lucabotez

#ifndef kNN_H
#define kNN_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include "kd_tree.h"

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);					\
		}							\
	} while (0)

#define WORD_SIZE 256

// function headers
int check_equal(int *key1, int *key2, int maxdim);
int kd_cmp_dim(int *key1, int *key2, int dim);
float calculate_distance(int *point1, int *point2, int dim);
void nearest_neighbor(kd_node_t *node, int *coords, int level, int *array_dim,
					  float *best_distance, int (*cmp_f)(int *, int *, int),
					  int dim, int ***points_array);

int compare_points(int *point1, int *point2, int dim);
void sort_points(int ***points_array, int array_dim,
				 int (*cmp_f)(int *, int *, int), int dim);

int check_interval(int *coords1, int *coords2, int *point, int dim);
void range_search(kd_node_t *node, int *coords1, int *coords2, int level,
				  int *array_dim, int dim, int ***points_array);

void print_points(int **points_array, int array_dim, int dim);
void free_points(int **points_array, int array_dim);

#endif //kNN_H