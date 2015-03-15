// This file is part of SmallBASIC
//
// Support for dictionary/associative array variables
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2007-2014 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

#include "common/var.h"

#ifndef VAR_MAP_H
#define VAR_MAP_H

#if defined(__cplusplus)
extern "C" {
#endif

int map_compare(const var_p_t var_a, const var_p_t var_b);
int map_is_empty(const var_p_t var_p);
int map_to_int(const var_p_t var_p);
int map_length(const var_p_t var_p);
int map_get_bool(var_p_t base, const char *name);
int map_get_int(var_p_t base, const char *name, int def);
const char *map_get_str(var_p_t base, const char *name);
var_p_t map_get(var_p_t base, const char *name);
var_p_t map_elem_key(const var_p_t var_p, int index);
var_p_t map_resolve_fields(const var_p_t base);
var_p_t map_add_var(var_p_t base, const char *name, int value);
void map_init(var_p_t map);
void map_free(var_p_t var_p);
void map_get_value(var_p_t base, var_p_t key, var_p_t *result);
void map_set(var_p_t dest, const var_p_t src);
void map_set_int(var_p_t base, const char *name, var_int_t n);
char *map_to_str(const var_p_t var_p);
void map_write(const var_p_t var_p, int method, int handle);
void map_from_str(var_p_t var_p);

#if defined(__cplusplus)
}
#endif

#endif

