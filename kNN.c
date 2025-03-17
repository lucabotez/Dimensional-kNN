// Copyright @lucabotez

#include "kNN.h"
#include "kd_tree.h"

// function that checks if two keys are equal
int check_equal(int *key1, int *key2, int maxdim)
{
	for (int i = 0; i < maxdim; i++)
		if (key1[i] != key2[i])
			return 0;

	return 1;
}

// function that compares only the given coordinate of two points
int kd_cmp_dim(int *key1, int *key2, int dim)
{
	if (key1[dim] < key2[dim])
		return -1;

	if (key1[dim] > key2[dim])
		return 1;

	if (key1[dim] == key2[dim])
		return 0;
}

// function that calculates the euclidian distance between two points in a
// dim-dimensional system
float calculate_distance(int *point1, int *point2, int dim)
{
	float dist = 0;
	for (int i = 0; i < dim; i++)
		dist += (pow(point2[i] - point1[i], 2));

	dist = sqrt(dist);
	return dist;
}

// function that recursively determines the minimum distance point / points
// from the k-d tree and a given point; they are saved in points_array, the
// distance in best_distance
void nearest_neighbor(kd_node_t *node, int *coords, int level, int *array_dim,
					  float *best_distance, int (*cmp_f)(int *, int *, int),
					  int dim, int ***points_array)
{
	if (!node)
		return;

	// temporary copy
	int **array = *points_array;

	int rc = cmp_f(coords, node->data, level % dim);
	kd_node_t *case1, *case2;

	if (rc < 0) {
		case1 = node->left;
		case2 = node->right;
	}

	if (rc >= 0) {
		case1 = node->right;
		case2 = node->left;
	}

	// no found points case
	if (*array_dim == 0) {
		array[0] = malloc(dim * sizeof(int));
		DIE(!array[0], "nn malloc");

		for (int i = 0; i < dim; i++)
			array[0][i] = node->data[i];

		*best_distance = calculate_distance(coords, node->data, dim);
		*array_dim = *array_dim + 1;
	} else {
		// the distance between the current and given point
		float curr_distance = calculate_distance(coords, node->data, dim);

		// if the distances are equal, the point is saved
		if (curr_distance == *best_distance) {
			*array_dim = *array_dim + 1;
			*points_array = realloc(*points_array,
									(*array_dim) * sizeof(int *));
			DIE(!points_array, "nn realloc");
			array = *points_array;
			array[*array_dim - 1] = malloc(dim * sizeof(int));
			DIE(!array[*array_dim - 1], "nn malloc");

			for (int i = 0; i < dim; i++)
				array[*array_dim - 1][i] = node->data[i];
		// if the distance is smaller, the array is reset
		} else if (curr_distance < *best_distance) {
			for (int i = 0; i < *array_dim; i++)
				free(array[i]);

			*array_dim = 1;
			array[0] = malloc(dim * sizeof(int));
			for (int i = 0; i < dim; i++)
				array[0][i] = node->data[i];

			*best_distance = curr_distance;
		}
	}

	// continue iterating through the k-d tree
	nearest_neighbor(case1, coords, level + 1, array_dim, best_distance,
					 cmp_f, dim, points_array);

	array = *points_array;

	// the distance between the current point plane and the given point
	// % dim
	float dim_distance = fabs(coords[level % dim] - node->data[level % dim]);

	// if the distance is smaller than best_distance, the other branch is
	// iterated through (the root of the branch is saved in case2)
	if (dim_distance < *best_distance)
		nearest_neighbor(case2, coords, level + 1, array_dim, best_distance,
						 cmp_f, dim, points_array);
}

// function that compares only a given dimension between 2 points
int compare_points(int *point1, int *point2, int dim)
{
	for (int i = 0; i < dim; i++)
		if (point1[i] < point2[i])
			return -1;
		else if (point1[i] > point2[i])
			return 1;
}

// function that sorts the points array
void sort_points(int ***points_array, int array_dim,
				 int (*cmp_f)(int *, int *, int), int dim)
{
	int **aux_array = *points_array;

	for (int i = 0; i < array_dim; i++)
		for (int j = i + 1; j < array_dim; j++) {
			int rc = cmp_f(aux_array[i], aux_array[j], dim);

			if (rc > 0)
				for (int k = 0; k < dim; k++) {
					int aux = aux_array[i][k];
					aux_array[i][k] = aux_array[j][k];
					aux_array[j][k] = aux;
				}
		}
}

// function that checks if a point is inside of a given interval
int check_interval(int *coords1, int *coords2, int *point, int dim)
{
	for (int i = 0; i < dim; i++)
		if (coords1[i] > point[i] || coords2[i] < point[i])
			return 0;

	return 1;
}

// function that recursively determines the points between a given interval
void range_search(kd_node_t *node, int *coords1, int *coords2, int level,
				  int *array_dim, int dim, int ***points_array)
{
	if (!node)
		return;

	// temporary copy
	int **array = *points_array;

	if (check_interval(coords1, coords2, node->data, dim)) {
		// empty array case
		if (*array_dim == 0) {
			array[0] = malloc(dim * sizeof(int));
			DIE(!array[0], "rs malloc");

			*array_dim = *array_dim + 1;

			for (int i = 0; i < dim; i++)
				array[0][i] = node->data[i];
		} else {
			*array_dim = *array_dim + 1;
			*points_array = realloc(*points_array,
									(*array_dim) * sizeof(int *));
			DIE(!points_array, "rs realloc");

			array = *points_array;

			array[*array_dim - 1] = malloc(dim * sizeof(int));
			DIE(!array[*array_dim - 1], "rs malloc");

			for (int i = 0; i < dim; i++)
				array[*array_dim - 1][i] = node->data[i];
		}
	}

	int between = 0;
	int curr_dim = level % dim;
	kd_node_t *case1, *case2;

	// if the value corresponding to the current node's dimension (curr_dim)
	// is within the given interval, 'between' will take the value 1;
	// this variable will be used in recursion to determine whether it is necessary
	// to also traverse the other subtree
	if (coords1[curr_dim] < node->data[curr_dim] &&
	    coords2[curr_dim] >= node->data[curr_dim])
		between = 1;

	// saving the two cases
	if (coords1[curr_dim] < node->data[curr_dim]) {
		case1 = node->left;
		case2 = node->right;
		range_search(case1, coords1, coords2, level + 1, array_dim,
					 dim, points_array);
	}

	else if (coords2[curr_dim] >= node->data[curr_dim]) {
		case1 = node->right;
		case2 = node->left;
		range_search(case1, coords1, coords2, level + 1, array_dim,
					 dim, points_array);
	}

	// the 'between' value is checked in the recursion return
	if (between == 1) {
		range_search(case2, coords1, coords2, level + 1, array_dim,
					 dim, points_array);
	}
}

// function that prints the points saved in the array
void print_points(int **points_array, int array_dim, int dim)
{
	for (int i = 0; i < array_dim; i++) {
		for (int j = 0; j < dim; j++)
			printf("%d ", points_array[i][j]);

		printf("\n");
	}
}

// function that frees all the allocated memory
void free_points(int **points_array, int array_dim)
{
	for (int i = 0; i < array_dim; i++)
		free(points_array[i]);

	free(points_array);
}

int main(void)
{
	kd_tree_t *kd_tree;
	char command[WORD_SIZE]; // given command

	while (1) {
		fscanf(stdin, "%s", command);
		if (strcmp(command, "LOAD") == 0) {
			char filename[WORD_SIZE];
			fscanf(stdin, "%s", filename);

			FILE *in = fopen(filename, "rt");
			DIE(!in, "failed to open file");

			int number, dim; // number of points and dimensions
			fscanf(in, "%d%d", &number, &dim);

			kd_tree = kd_tree_create(dim, kd_cmp_dim);
			while (number) { // saving the input in the k-d tree
				int *data = malloc(dim * sizeof(int));
				DIE(!data, "data malloc");

				for (int i = 0; i < dim; i++)
					fscanf(in, "%d", &data[i]);
				kd_tree_insert(kd_tree, data);
				free(data);

				number--;
			}
			fclose(in);
		} else if (strcmp(command, "NN") == 0) {
			int *coords = malloc(kd_tree->dim * sizeof(int));

			for (int i = 0; i < kd_tree->dim; i++)
				fscanf(stdin, "%d", &coords[i]);

			int **points_array, array_dim = 0;
			points_array = malloc(sizeof(int *));
			DIE(!points_array, "points_array malloc");

			float best_distance = 0;
			nearest_neighbor(kd_tree->root, coords, 0, &array_dim,
							 &best_distance, kd_cmp_dim, kd_tree->dim,
							 &points_array);

			if (array_dim > 1) // sorting only if there is more than one point
				sort_points(&points_array, array_dim,
							compare_points, kd_tree->dim);

			print_points(points_array, array_dim, kd_tree->dim);

			free_points(points_array, array_dim);
			free(coords);
		} else if (strcmp(command, "RS") == 0) {
			int *coords1 = malloc(kd_tree->dim * sizeof(int));
			int *coords2 = malloc(kd_tree->dim * sizeof(int));

			for (int i = 0; i < kd_tree->dim; i++)
				fscanf(stdin, "%d %d", &coords1[i], &coords2[i]);

			int **points_array, array_dim = 0;
			points_array = malloc(sizeof(int *));

			range_search(kd_tree->root, coords1, coords2, 0, &array_dim,
						 kd_tree->dim, &points_array);

			if (array_dim > 1) // sorting only if there is more than one point
				sort_points(&points_array, array_dim,
							compare_points, kd_tree->dim);

			print_points(points_array, array_dim, kd_tree->dim);

			free_points(points_array, array_dim);
			free(coords1);
			free(coords2);
		} else if (strcmp(command, "EXIT") == 0) {
			kd_tree_free(kd_tree, free);
			return 0;
		}
	}
	return 0;
}
