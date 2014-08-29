// This file is part of SmallBASIC
//
// Support for hash variables
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2007-2014 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

#include "common/var.h"

#ifndef VAR_HASH_H
#define VAR_HASH_H

#if defined(__cplusplus)
extern "C" {
#endif

int hash_compare(const var_p_t var_a, const var_p_t var_b);
int hash_is_empty(const var_p_t var_p);
int hash_to_int(const var_p_t var_p);
int hash_length(const var_p_t var_p);
var_p_t hash_elem(const var_p_t var_p, int index);
var_p_t hash_resolve_fields(const var_p_t base);
void hash_free(var_p_t var_p);
void hash_get_value(var_p_t base, var_p_t key, var_p_t *result);
void hash_set(var_p_t dest, const var_p_t src);
void hash_to_str(const var_p_t var_p, char *out, int max_len);
void hash_write(const var_p_t var_p, int method, int handle);

#if defined(__cplusplus)
}
#endif

#endif

