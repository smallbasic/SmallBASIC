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

int tree_compare(const char *key, int length, var_p_t vkey) {
  return strcaselessn(key, vkey->v.p.ptr, I2MAX(length, vkey->v.p.size - 1));
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

TreeNode *tree_search(TreeNode **rootp, const char *key, int length) {
  while (*rootp != NULL) {
    int r = tree_compare(key, length, (*rootp)->key);
    if (r == 0) {
      return *rootp;
    }
    rootp = (r < 0) ? &(*rootp)->left : &(*rootp)->right;
  }
  TreeNode *result = tree_create_node(NULL);
  *rootp = result;
  return result;
}

TreeNode *tree_find(TreeNode **rootp, const char *key, int length) {
  while (*rootp != NULL) {
    int r = tree_compare(key, length, (*rootp)->key);
    if (r == 0) {
      return *rootp;
    }
    rootp = (r < 0) ? &(*rootp)->left : &(*rootp)->right;
  }
  return NULL;
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

var_p_t hashmap_put(var_p_t map, const char *key, int length) {
  TreeNode *node = tree_search((TreeNode **)&map->v.m.map, key, length);
  if (node->key == NULL) {
    node->key = v_new();
    node->value = v_new();
    v_setstrn(node->key, key, length);
    map->v.m.size++;
  }
  return node->value;
}

var_p_t hashmap_putv(var_p_t map, const var_p_t key) {
  // hashmap takes ownership of key
  if (key->type != V_STR) {
    // keys are always strings
    v_tostr(key);
  }

  TreeNode *node = tree_search((TreeNode **)&map->v.m.map, key->v.p.ptr, key->v.p.size);
  if (node->key == NULL) {
    node->key = key;
    node->value = v_new();
    map->v.m.size++;
  } else {
    // discard unused key
    v_free(key);
    free(key);
  }
  return node->value;
}

var_p_t hashmap_get(var_p_t map, const char *key) {
  var_p_t result;
  TreeNode *node = tree_find((TreeNode **)&map->v.m.map, key, strlen(key));
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
