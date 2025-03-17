// Copyright @lucabotez

#include "kd_tree.h"
#include "kNN.h"

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