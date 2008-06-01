// $Id$
// This file is part of SmallBASIC
//
// Support for hash variables
// eg: dim foo: key="blah": foo(key) = "something": ? foo(key)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

#include "sys.h"
#include "var.h"
#include "smbas.h"
#include "var_hash.h"

#ifdef HAVE_SEARCH_H
#include <search.h>
#endif

/**
 * Globals for callback access
 */
typedef struct CallbackData {
  int method;
  int handle;
  int count;
  int firstElement;
  void* hash;
} CallbackData;

CallbackData cb;

/**
 * Our internal tree element node
 */
typedef struct Element {
  char* key;
  var_p_t value;
} Element;

/**
 * Return value from search routines
 */
typedef struct Node {
  Element* element;
} Node;

/**
 * Returns a new Element 
 */
Element* create_element(const char* key) {
  Element* element = (Element*) tmp_alloc(sizeof (Element));
  element->key = strdup(key);
  element->value = NULL;
  return element;
}

/**
 * Callback to compare Element's
 */
int cmp_fn(const void *a, const void *b)
{
  Element* key_a = (Element*) a;
  Element* key_b = (Element*) b;
  return strcasecmp(key_a->key, key_b->key);
}

/**
 * Compare one HASH to another. see v_compare comments for return spec.
 */
int hash_compare(const var_p_t var_a, const var_p_t var_b)
{
  return 0;
}

/**
 * Return true if the structure is empty
 */
int hash_is_empty(const var_p_t var_p)
{
  return (var_p->v.hash == 0);
}

void hash_to_int_cb(const void *nodep, VISIT value, int level) {
  cb.count++;
}

/**
 * Return the contents of the structure as an integer
 */
int hash_to_int(const var_p_t var_p)
{
  cb.count = 0;
  if (var_p->type == V_HASH) {
    twalk(var_p->v.hash, hash_to_int_cb);
  }
  return cb.count;
}

/**
 * Empty struct values
 */
void hash_clear(const var_p_t var)
{
  hash_free(var);
}

/**
 * Helper for hash_free
 */
void hash_free_cb(void *nodep) {
  Element* element = (Element*) nodep;
  tmp_free(element->key); // cleanup strdup()
  tmp_free(element->value); // cleanup v_new
  tmp_free(element); // cleanup create_element() 
}

/**
 * Delete the given structure
 */
void hash_free(var_p_t var_p)
{
  if (var_p->type == V_HASH) {
    tdestroy(var_p->v.hash, hash_free_cb);
  }
}

/**
 * Return the variable in base keyed by key, if not found then creates
 * an empty variable that will be returned in a further call
 */
void hash_get_value(var_p_t base, var_p_t var_key, var_p_t *result)
{
  if (base->type != V_HASH) {
    // initialise as hash
    v_free(base);
    base->type = V_HASH;
    base->v.hash = 0;
  }

  // create a key which will hold our name and value pairs
  Element* key = create_element(var_key->v.p.ptr);
  Node* node = tfind(key, &base->v.hash, cmp_fn);
  if (node != NULL) {
    *result = node->element->value;
    tmp_free(key);
  }
  else {
    // add key to the tree
    key->value = *result = v_new();
    tsearch(key, &base->v.hash, cmp_fn);
  }
}

/**
 * Callback for hash_set()
 */
void hash_set_cb(const void *nodep, VISIT value, int level) {
  if (value == leaf || value == endorder) {
    // copy the element and insert it into the destination
    Element* element = ((Node*) nodep)->element;
    Element* key = create_element(element->key);
    key->value = v_new();
    v_set(key->value, element->value);
    tsearch(key, &cb.hash, cmp_fn);
  }
}

/**
 * Reference values from one structure to another
 */
void hash_set(var_p_t dest, const var_p_t src)
{
  v_free(dest);
  cb.hash = 0;

  //  dest->v.hash = hash = 0;
  if (src->type == V_HASH) {
    twalk(src->v.hash, hash_set_cb);
  }

  dest->type = V_HASH;
  dest->v.hash = cb.hash;
}

/**
 * Return the contents of the structure as a string
 */
void hash_to_str(const var_p_t var_p, char *out, int max_len)
{
  sprintf(out, "HASH:%d", hash_to_int(var_p));
}

/**
 * Callback for hash_write()
 */
void hash_write_cb(const void *nodep, VISIT value, int level) {
  if (value == leaf || value == endorder) {
    Element* element = ((Node*) nodep)->element;
    if (!cb.firstElement) {
      pv_write(",", cb.method, cb.handle);
    }
    cb.firstElement = 0;
    pv_write(element->key, cb.method, cb.handle);
    pv_write("=", cb.method, cb.handle);
    pv_writevar(element->value, cb.method, cb.handle);
  }
}

/**
 * Print the contents of the structure
 */
void hash_write(const var_p_t var_p, int method, int handle)
{
  if (var_p->type == V_HASH) {
    cb.method = method;
    cb.handle = handle;
    cb.firstElement = 1;
    pv_write("[", method, handle);
    twalk(var_p->v.hash, hash_write_cb);
    pv_write("]", method, handle);
  }
}

// End of $Id$
