// Copyright @lucabotez

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

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

// implementarea bst din laborator, modificata la modul de retinere
// a datelor (void * -> int *) si la functia de comparatie

typedef struct kd_node_t kd_node_t;
struct  kd_node_t {
	kd_node_t *left;

	kd_node_t *right;

	int *data;
};

typedef struct kd_tree_t kd_tree_t;
struct kd_tree_t {
	kd_node_t  *root;

	int dim;

	int	(*cmp)(int *key1, int *key2, int dim);
};

int check_equal(int *key1, int *key2, int maxdim)
{
	for (int i = 0; i < maxdim; i++)
		if (key1[i] != key2[i])
			return 0;

	return 1;
}

static kd_node_t *__kd_node_create(int *data, int dim)
{
	kd_node_t *kd_node;

	kd_node = malloc(sizeof(*kd_node));
	DIE(!kd_node, "kd_node malloc");

	kd_node->left = NULL;
	kd_node->right = NULL;

	kd_node->data = malloc(dim * sizeof(int));
	DIE(!kd_node->data, "kd_node->data malloc");
	memcpy(kd_node->data, data, dim * sizeof(int));

	return kd_node;
}

kd_tree_t *kd_tree_create(int dim, int (*cmp_f)(int *, int *, int))
{
	kd_tree_t *kd_tree;

	kd_tree = malloc(sizeof(*kd_tree));
	DIE(!kd_tree, "kd_tree malloc");

	kd_tree->root  = NULL;
	kd_tree->dim = dim;
	kd_tree->cmp = cmp_f;

	return kd_tree;
}

void kd_tree_insert(kd_tree_t *kd_tree, int *data)
{
	int rc, level = 0;
	kd_node_t *parent = kd_tree->root;
	kd_node_t *node	= __kd_node_create(data, kd_tree->dim);

	if (!parent) {
		kd_tree->root = node;
		return;
	}

	while (1) {
		int dim = level % kd_tree->dim;
		rc = kd_tree->cmp(parent->data, data, dim);
		if (rc > 0) {
			if (!parent->left) {
				parent->left = node;
				return;
			}
			parent = parent->left;
		} else if (rc <= 0) {
			if (rc == 0 && check_equal(parent->data, data, kd_tree->dim) == 1) {
				free(node->data);
				free(node);
				return;
			}

			if (!parent->right) {
				parent->right = node;
				return;
			}
			parent = parent->right;
		}
		level++;
	}
}

static void __kd_tree_free(kd_node_t *kd_node, void (*free_data)(void *))
{
	if (!kd_node)
		return;

	__kd_tree_free(kd_node->left, free_data);
	__kd_tree_free(kd_node->right, free_data);

	free(kd_node->data);
	free_data(kd_node);
}

void kd_tree_free(kd_tree_t *kd_tree, void (*free_data)(void *))
{
	__kd_tree_free(kd_tree->root, free_data);
	free(kd_tree);
}

// functia compara doar coordonata unei dimensiuni dintre doua puncte
int kd_cmp_dim(int *key1, int *key2, int dim)
{
	if (key1[dim] < key2[dim])
		return -1;

	if (key1[dim] > key2[dim])
		return 1;

	if (key1[dim] == key2[dim])
		return 0;
}

// functia calculeaza distanta euclidiana dintre doua puncte in dim dimensiuni
float calculate_distance(int *point1, int *point2, int dim)
{
	float dist = 0;
	for (int i = 0; i < dim; i++)
		dist += (pow(point2[i] - point1[i], 2));

	dist = sqrt(dist);
	return dist;
}

// functia determina recursiv punctul / punctele din k-d tree aflate la
// distanta minima de punctul salvat in coords; acestea vor fi salvate in
// points_array de dimensiune array_dim, iar distanta in best_distance
void nearest_neighbor(kd_node_t *node, int *coords, int level, int *array_dim,
					  float *best_distance, int (*cmp_f)(int *, int *, int),
					  int dim, int ***points_array)
{
	if (!node)
		return;

	// copie temporara, scapam de dereferentieri multe
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

	// cazul in care vectorul de puncte este gol, salvam primul punct gasit
	if (*array_dim == 0) {
		array[0] = malloc(dim * sizeof(int));
		DIE(!array[0], "nn malloc");

		for (int i = 0; i < dim; i++)
			array[0][i] = node->data[i];

		*best_distance = calculate_distance(coords, node->data, dim);
		*array_dim = *array_dim + 1;
	} else {
		// distanta dintre punctul pe care ne aflam si cel dat
		float curr_distance = calculate_distance(coords, node->data, dim);

		// daca distantele sunt egale adaugam punctul in vectorul de puncte
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
		// daca distanta este mai mica resetam vectorul de puncte
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

	// continuam parcurgerea recursiva prin k-d tree
	nearest_neighbor(case1, coords, level + 1, array_dim, best_distance,
					 cmp_f, dim, points_array);

	array = *points_array;

	// distanta dintre planul level % dim punctului pe care ne aflam si
	// planul punctului dat
	float dim_distance = fabs(coords[level % dim] - node->data[level % dim]);

	// daca distanta dintre planuri este mai mica decat best_distance,
	// parcurgem si subarborele celalalt ce porneste din nodul pe care ne
	// aflam, radacina subarborelui fiind salvata in case2
	if (dim_distance < *best_distance)
		nearest_neighbor(case2, coords, level + 1, array_dim, best_distance,
						 cmp_f, dim, points_array);
}

// functia compara doar dimensiunea dim a doua puncte
int compare_points(int *point1, int *point2, int dim)
{
	for (int i = 0; i < dim; i++)
		if (point1[i] < point2[i])
			return -1;
		else if (point1[i] > point2[i])
			return 1;
}

// functia sorteaza crescatot vectorul de puncte
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

// functia verifica daca un punct apartine intervalului dat de
// coords1 si coords2 (verifica fiecare dimensiune in parte)
int check_interval(int *coords1, int *coords2, int *point, int dim)
{
	for (int i = 0; i < dim; i++)
		if (coords1[i] > point[i] || coords2[i] < point[i])
			return 0;

	return 1;
}

// functia determina recursiv punctele aflate in intervalul dat de punctele
// coords1 si coords2 si le salveaza in points_array
void range_search(kd_node_t *node, int *coords1, int *coords2, int level,
				  int *array_dim, int dim, int ***points_array)
{
	if (!node)
		return;

	// copie temporara, scapam de dereferentieri multe
	int **array = *points_array;

	// daca punctul pe care ne aflam este in intervalul dat il salvam
	if (check_interval(coords1, coords2, node->data, dim)) {
		// cazul in care vectorul de puncte este gol
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

	// daca valoarea corespunzatoare dimensiunii curr_dim a nodului curent
	// se afla in intervalul dat between ia valoarea 1; ne vom folosi de
	// aceasta variabila in recursivitate pentru a determina daca este nevoie
	// sa parcurgem si celalalt subarbore
	if (coords1[curr_dim] < node->data[curr_dim] &&
	    coords2[curr_dim] >= node->data[curr_dim])
		between = 1;

	// salvam cele doua cazuri in functie de valori
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

	// in intoarcerea recursiva se verifica valoarea variabilei between
	if (between == 1) {
		range_search(case2, coords1, coords2, level + 1, array_dim,
					 dim, points_array);
	}
}

// functia afiseaza punctele salvate in vectorul de puncte
void print_points(int **points_array, int array_dim, int dim)
{
	for (int i = 0; i < array_dim; i++) {
		for (int j = 0; j < dim; j++)
			printf("%d ", points_array[i][j]);

		printf("\n");
	}
}

// functia elibereaza memoria alocata pt vectorul de puncte
void free_points(int **points_array, int array_dim)
{
	for (int i = 0; i < array_dim; i++)
		free(points_array[i]);

	free(points_array);
}

int main(void)
{
	kd_tree_t *kd_tree;
	char command[WORD_SIZE]; // comanda citita

	while (1) {
		fscanf(stdin, "%s", command);
		if (strcmp(command, "LOAD") == 0) {
			char filename[WORD_SIZE];
			fscanf(stdin, "%s", filename);

			FILE *in = fopen(filename, "rt");
			DIE(!in, "failed to open file");

			int number, dim; // nr de puncte si de dim
			fscanf(in, "%d%d", &number, &dim);

			kd_tree = kd_tree_create(dim, kd_cmp_dim);
			while (number) { // citim numerele si le salvam in k-d tree
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

			if (array_dim > 1) // sortam doar daca avem mai mult de un punct
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

			if (array_dim > 1) // sortam doar daca avem mai mult de un punct
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
