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

#define MAP_SIZE 32

/**
 * Our internal tree element node
 */
typedef struct Node {
  var_p_t key;
  var_p_t value;
  struct Node *left, *right;
} Node;

/**
 * Returns a new tree node
 */
Node *tree_create_node(var_p_t key) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->key = key;
  node->value = NULL;
  node->left = NULL;
  node->right = NULL;
  return node;
}

/**
 * cleanup the given element
 */
void tree_delete_node(Node *node) {
  // cleanup v_new
  v_free(node->key);
  v_detach(node->key);

  // cleanup v_new
  if (node->value) {
    v_free(node->value);
    v_detach(node->value);
  }

  // cleanup the node
  free(node);
}

static inline int tree_compare(const char *key, int length, var_p_t vkey) {
  return strcaselessn(key, length, vkey->v.p.ptr, vkey->v.p.length - 1);
}

void tree_destroy(Node *node) {
  if (node->left != NULL) {
    tree_destroy(node->left);
  }
  if (node->right != NULL) {
    tree_destroy(node->right);
  }
  tree_delete_node(node);
}

Node *tree_search(Node **rootp, const char *key, int length) {
  while (*rootp != NULL) {
    int r = tree_compare(key, length, (*rootp)->key);
    if (r == 0) {
      return *rootp;
    }
    rootp = (r < 0) ? &(*rootp)->left : &(*rootp)->right;
  }
  Node *result = tree_create_node(NULL);
  *rootp = result;
  return result;
}

Node *tree_find(Node **rootp, const char *key, int length) {
  while (*rootp != NULL) {
    int r = tree_compare(key, length, (*rootp)->key);
    if (r == 0) {
      return *rootp;
    }
    rootp = (r < 0) ? &(*rootp)->left : &(*rootp)->right;
  }
  return NULL;
}

int tree_foreach(const Node *nodep, hashmap_foreach_func func, hashmap_cb *data) {
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
void hashmap_create(var_p_t map, int size) {
  v_free(map);
  map->type = V_MAP;
  map->v.m.count = 0;
  if (size == 0) {
    map->v.m.size = MAP_SIZE;
  } else {
    map->v.m.size = (size * 100) / 75;
  }
  map->v.m.map = calloc(map->v.m.size, sizeof(Node *));
}

int hashmap_destroy(var_p_t var_p) {
  if (var_p->type == V_MAP && var_p->v.m.map != NULL) {
    int i;
    for (i = 0; i < var_p->v.m.size; i++) {
      Node **table = (Node **)var_p->v.m.map;
      if (table[i] != NULL) {
        tree_destroy(table[i]);
      }
    }
    free(var_p->v.m.map);
  }
  return 0;
}

int hashmap_get_hash(const char *key, int length) {
  int hash = 1, i;
  for (i = 0; i < length && key[i] != '\0'; i++) {
    hash += to_lower(key[i]);
    hash <<= 3;
    hash ^= (hash >> 3);
  }
  return hash;
}

static inline Node *hashmap_search(var_p_t map, const char *key, int length) {
  int index = hashmap_get_hash(key, length) % map->v.m.size;
  Node **table = (Node **)map->v.m.map;
  Node *result = table[index];
  if (result == NULL) {
    // new entry
    result = table[index] = tree_create_node(NULL);
  } else {
    int r = tree_compare(key, length, result->key);
    if (r < 0) {
      result = tree_search(&result->left, key, length);
    } else if (r > 0) {
      result = tree_search(&result->right, key, length);
    }
  }
  return result;
}

static inline Node *hashmap_find(var_p_t map, const char *key) {
  int length = strlen(key);
  int index = hashmap_get_hash(key, length) % map->v.m.size;
  Node **table = (Node **)map->v.m.map;
  Node *result = table[index];
  if (result != NULL) {
    int r = tree_compare(key, length, result->key);
    if (r < 0 && result->left != NULL) {
      result = tree_find(&result->left, key, length);
    } else if (r > 0 && result->right != NULL) {
      result = tree_find(&result->right, key, length);
    } else if (r != 0) {
      result = NULL;
    }
  }
  return result;
}

var_p_t hashmap_put(var_p_t map, const char *key, int length) {
  Node *node = hashmap_search(map, key, length);
  if (node->key == NULL) {
    node->key = v_new();
    node->value = v_new();
    v_setstrn(node->key, key, length);
    map->v.m.count++;
  }
  return node->value;
}

var_p_t hashmap_putv(var_p_t map, const var_p_t key) {
  // hashmap takes ownership of key
  if (key->type != V_STR) {
    // keys are always strings
    v_tostr(key);
  }

  Node *node = hashmap_search(map, key->v.p.ptr, key->v.p.length);
  if (node->key == NULL) {
    node->key = key;
    node->value = v_new();
    map->v.m.count++;
  } else {
    // discard unused key
    v_free(key);
    v_detach(key);
  }
  return node->value;
}

var_p_t hashmap_get(var_p_t map, const char *key) {
  var_p_t result;
  Node *node = hashmap_find(map, key);
  if (node != NULL) {
    result = node->value;
  } else {
    result = NULL;
  }
  return result;
}

void hashmap_foreach(var_p_t map, hashmap_foreach_func func, hashmap_cb *data) {
  if (map && map->type == V_MAP) {
    int i;
    for (i = 0; i < map->v.m.size; i++) {
      Node **table = (Node **)map->v.m.map;
      if (table[i] != NULL) {
        tree_foreach(table[i], func, data);
      }
    }
  }
}
