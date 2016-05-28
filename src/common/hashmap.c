// This file is part of SmallBASIC
//
// Support for dictionary/associative array variables
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2007-2016 Chris Warren-Smith.

#include "config.h"

#include "common/sys.h"
#include "common/var.h"
#include "common/smbas.h"
#include "common/hashmap.h"

/**
 * Our internal tree element node
 */
typedef struct TreeNode {
  var_p_t key;
  var_p_t value;
  struct TreeNode *left, *right;
} TreeNode;

/**
 * Returns a new tree node
 */
TreeNode *tree_create_node(var_p_t key) {
  TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
  node->key = key;
  node->value = NULL;
  node->left = NULL;
  node->right = NULL;
  return node;
}

/**
 * cleanup the given element
 */
void tree_delete_node(TreeNode *node) {
  // cleanup v_new
  v_free(node->key);
  free(node->key);

  // cleanup v_new
  if (node->value) {
    v_free(node->value);
    free(node->value);
  }

  // cleanup the node
  free(node);
}

/**
 * Callback to compare TreeNode's
 */
int tree_compare(var_p_t el_a, var_p_t el_b) {
  int result;
  if (el_a->type == V_STR && el_b->type == V_STR) {
    result = strcasecmp(el_a->v.p.ptr, el_b->v.p.ptr);
  } else {
    result = v_compare(el_a, el_b);
  }
  return result;
}

void tree_destroy(TreeNode *node) {
  if (node->left != NULL) {
    tree_destroy(node->left);
  }
  if (node->right != NULL) {
    tree_destroy(node->right);
  }
  tree_delete_node(node);
}

/* find or insert datum into search tree */
TreeNode *tree_search(TreeNode *node, TreeNode **rootp, int insert) {
  if (rootp == NULL) {
    return NULL;
  }
  while (*rootp != NULL) {
    int r;
    if ((r = tree_compare(node->key, (*rootp)->key)) == 0) {
      return *rootp;
    }
    rootp = (r < 0) ? &(*rootp)->left : &(*rootp)->right;
  }
  TreeNode *result;
  if (insert) {
    *rootp = node;
    result = node;
  } else {
    result = NULL;
  }
  return result;
}

int tree_foreach(const TreeNode *nodep, hashmap_foreach_func func, hashmap_cb *data) {
  if (nodep != NULL) {
    if (nodep->left != NULL && !tree_foreach(nodep->left, func, data)) {
      return 0;
    }

    if (func(data, nodep->key, nodep->value)) {
      return 0;
    }

    if (nodep->right != NULL && !tree_foreach(nodep->right, func, data)) {
      return 0;
    }
  }
  return 1;
}

/**
 * initialise the variable as a map
 */
void hashmap_create(var_p_t map) {
  v_free(map);
  map->type = V_MAP;
  map->v.m.map = NULL;
  map->v.m.size = 0;
}

int hashmap_destroy(var_p_t var_p) {
  if (var_p->type == V_MAP && var_p->v.m.map != NULL) {
    tree_destroy(var_p->v.m.map);
  }
  return 0;
}

var_p_t hashmap_put(var_p_t map, const var_p_t key) {
  var_p_t result;

  // hashmap_put takes ownership of key
  TreeNode *treeNode = tree_create_node(key);
  TreeNode *node = tree_search(treeNode, (TreeNode **)&map->v.m.map, 1);
  if (node != treeNode) {
    // found existing item, discard treeNode
    result = node->value;
    tree_delete_node(treeNode);
  } else {
    // not found, treeNode attached to tree
    result = treeNode->value = v_new();
    v_init(treeNode->value);
    map->v.m.size++;
  }
  return result;
}

var_p_t hashmap_get(var_p_t map, const var_p_t key) {
  var_p_t result;
  TreeNode treeNode;
  treeNode.key = key;

  TreeNode *node = tree_search(&treeNode, (TreeNode **)&map->v.m.map, 0);
  if (node != NULL) {
    result = node->value;
  } else {
    result = NULL;
  }
  return result;
}

void hashmap_foreach(var_p_t map, hashmap_foreach_func func, hashmap_cb *data) {
  if (map && map->type == V_MAP) {
    tree_foreach(map->v.m.map, func, data);
  }
}
