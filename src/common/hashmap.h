// This file is part of SmallBASIC
//
// Support for dictionary/associative array variables
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2007-2016 Chris Warren-Smith.

#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include "common/var.h"

/**
 * Callback structure for hashmap_foreach
 */
typedef struct hashmap_cb {
  int count;
  int index;
  int start;
  char *buffer;
  var_p_t var;
  var_p_t parent;
} hashmap_cb;

typedef int (*hashmap_foreach_func)(hashmap_cb *cb, var_p_t k, var_p_t v);

void hashmap_create(var_p_t map, int size);
int  hashmap_destroy(var_p_t map);
var_p_t hashmap_put(var_p_t map, const char *key, int length);
var_p_t hashmap_putc(var_p_t map, const char *key, int length);
var_p_t hashmap_putv(var_p_t map, const var_p_t key);
var_p_t hashmap_get(var_p_t map, const char *key);
void hashmap_foreach(var_p_t map, hashmap_foreach_func func, hashmap_cb *data);

#endif /* !_HASHMAP_H_ */

