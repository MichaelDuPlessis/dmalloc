#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct TreeNode {
  struct TreeNode *left;
  struct TreeNode *right;
  int value;
} TreeNode;

// Recursively allocates a binary tree of given depth
static TreeNode *allocate_tree(void *(*allocator)(size_t), size_t depth) {
  if (depth == 0)
    return NULL;

  TreeNode *node = (TreeNode *)allocator(sizeof(TreeNode));
  if (!node)
    return NULL;

  node->value = rand();
  node->left = allocate_tree(allocator, depth - 1);
  node->right = allocate_tree(allocator, depth - 1);
  return node;
}

// Recursively deallocates a binary tree
static void deallocate_tree(TreeNode *root, void (*deallocator)(void *)) {
  if (!root)
    return;
  deallocate_tree(root->left, deallocator);
  deallocate_tree(root->right, deallocator);
  deallocator(root);
}

// Benchmark: build and destroy trees repeatedly
void tree_allocs(void *(*allocator)(size_t), void (*deallocator)(void *),
                 size_t num_trees, size_t tree_depth, unsigned int seed) {
  srand(seed);

  for (size_t i = 0; i < num_trees; i++) {
    TreeNode *root = allocate_tree(allocator, tree_depth);
    deallocate_tree(root, deallocator);
  }
}
