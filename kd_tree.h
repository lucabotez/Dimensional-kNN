// Copyright @lucabotez

#ifndef KD_TREE_H
#define KD_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// kd_tree node structure

typedef struct kd_node_t kd_node_t;
struct  kd_node_t {
	kd_node_t *left;

	kd_node_t *right;

	int *data;
};

// kd tree structure
typedef struct kd_tree_t kd_tree_t;
struct kd_tree_t {
	kd_node_t  *root;

	int dim;

	int	(*cmp)(int *key1, int *key2, int dim);
};

// function headers
static kd_node_t *__kd_node_create(int *data, int dim);
kd_tree_t *kd_tree_create(int dim, int (*cmp_f)(int *, int *, int));
void kd_tree_insert(kd_tree_t *kd_tree, int *data);
static void __kd_tree_free(kd_node_t *kd_node, void (*free_data)(void *));
void kd_tree_free(kd_tree_t *kd_tree, void (*free_data)(void *));

#endif //KD_TREE_H