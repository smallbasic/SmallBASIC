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
#include "lib/search.h"

/**
 * Our internal tree element node - node_t->key
 */
typedef struct TreeNode {
  var_p_t key;
  var_p_t value;
} TreeNode;

/**
 * Cast for node_t->key - node_t first field is a pointer to key
 */
typedef struct Node {
  TreeNode *treeNode;
} Node;

/**
 * Returns a new Element
 */
TreeNode *tree_create_node(var_p_t key) {
  TreeNode *treeNode = (TreeNode *)malloc(sizeof(TreeNode));
  treeNode->key = key;
  treeNode->value = NULL;
  return treeNode;
}

/**
 * cleanup the given element
 */
void tree_delete_node(TreeNode *treeNode) {
  // cleanup v_new
  v_free(treeNode->key);
  free(treeNode->key);

  // cleanup v_new
  if (treeNode->value) {
    v_free(treeNode->value);
    free(treeNode->value);
  }

  // cleanup the node
  free(treeNode);
}

/**
 * Callback to compare TreeNode's
 */
int tree_cmp_fn(const void *a, const void *b) {
  TreeNode *el_a = (TreeNode *)a;
  TreeNode *el_b = (TreeNode *)b;

  int result;
  if (el_a->key->type == V_STR && el_b->key->type == V_STR) {
    result = strcasecmp(el_a->key->v.p.ptr, el_b->key->v.p.ptr);
  } else {
    result = v_compare(el_a->key, el_b->key);
  }
  return result;
}

/**
 * Helper for map_free
 */
void tree_free_cb(void *node_key) {
  tree_delete_node((TreeNode *)node_key);
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
  if (var_p->type == V_MAP) {
    tdestroy(var_p->v.m.map, tree_free_cb);
  }
  return 0;
}

var_p_t hashmap_put(var_p_t map, const var_p_t key) {
  var_p_t result;

  // hashmap_put takes ownership of key
  TreeNode *treeNode = tree_create_node(key);

  Node *node = tfind(treeNode, &(map->v.m.map), tree_cmp_fn);
  if (node != NULL) {
    // item already exists
    result = node->treeNode->value;
    tree_delete_node(treeNode);
  }
  else {
    tsearch(treeNode, &(map->v.m.map), tree_cmp_fn);
    treeNode->value = result = v_new();
    v_init(treeNode->value);
    map->v.m.size++;
  }
  return result;
}

var_p_t hashmap_get(var_p_t map, const var_p_t key) {
  var_p_t result;
  TreeNode treeNode;
  treeNode.key = key;
  Node *node = tfind(&treeNode, &(map->v.m.map), tree_cmp_fn);
  if (node != NULL) {
    result = node->treeNode->value;
  } else {
    result = NULL;
  }
  return result;
}

int tree_foreach(const node_t *nodep, hashmap_foreach_func func, hashmap_cb *data) {
  if (nodep != NULL) {
    if (nodep->left != NULL && !tree_foreach(nodep->left, func, data)) {
      return 0;
    }

    Node *node = (Node *)nodep;
    if (func(data, node->treeNode->key, node->treeNode->value)) {
      return 0;
    }

    if (nodep->right != NULL && !tree_foreach(nodep->right, func, data)) {
      return 0;
    }
  }
  return 1;
}

void hashmap_foreach(var_p_t map, hashmap_foreach_func func, hashmap_cb *data) {
  if (map && map->type == V_MAP) {
    tree_foreach(map->v.m.map, func, data);
  }
}
