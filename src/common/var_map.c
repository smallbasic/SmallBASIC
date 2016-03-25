// This file is part of SmallBASIC
//
// Support for dictionary/associative array variables
// eg: dim foo: key="blah": foo(key) = "something": ? foo(key)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2010-2014 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

#include "common/sys.h"
#include "common/var.h"
#include "common/smbas.h"
#include "common/sberr.h"
#include "common/pproc.h"
#include "common/var_map.h"
#include "lib/search.h"
#include "lib/jsmn.h"

#define BUFFER_GROW_SIZE 64
#define BUFFER_PADDING   10
#define TOKEN_GROW_SIZE  16
#define ARRAY_GROW_SIZE  16

/**
 * Our internal tree element node
 */
typedef struct Element {
  var_p_t key;
  var_p_t value;
} Element;

/**
 * Globals for callback access
 */
typedef struct CallbackData {
  int method;
  int handle;
  int index;
  int count;
  int firstElement;
  Element *el;
  char *buffer;
} CallbackData;

/**
 * Cast for node_t->key
 */
typedef struct Node {
  Element *element;
} Node;

/**
 * Container for map_from_str
 */
typedef struct JsonTokens {
  const char *js;
  jsmntok_t *tokens;
  int num_tokens;
} JsonTokens;

/**
 * Returns a new Element
 */
Element *create_element(var_p_t key) {
  Element *element = (Element *)malloc(sizeof(Element));
  element->key = v_new();
  v_set(element->key, key);
  element->value = NULL;
  return element;
}

/**
 * Returns a new Element using the integer based key
 */
Element *create_int_element(int key) {
  Element *element = (Element *)malloc(sizeof(Element));
  element->key = v_new();
  v_setint(element->key, key);
  element->value = NULL;
  return element;
}

/**
 * initialise the variable as a map
 */
void map_init(var_p_t map) {
  v_free(map);
  map->type = V_MAP;
  map->v.m.map = NULL;
  map->v.m.size = 0;
}

/**
 * Process the next token
 */
int map_read_next_token(var_p_t dest, JsonTokens *json, int index);

/**
 * cleanup the given element
 */
void delete_element(Element *element) {
  // cleanup v_new
  v_free(element->key);
  free(element->key);

  // cleanup v_new
  if (element->value) {
    v_free(element->value);
    free(element->value);
  }

  // cleanup create_element()
  free(element);
}

/**
 * Callback to compare Element's
 */
int cmp_fn(const void *a, const void *b) {
  Element *el_a = (Element *)a;
  Element *el_b = (Element *)b;

  int result;
  if (el_a->key->type == V_STR && el_b->key->type == V_STR) {
    result = strcasecmp(el_a->key->v.p.ptr, el_b->key->v.p.ptr);
  } else {
    result = v_compare(el_a->key, el_b->key);
  }
  return result;
}

/**
 * Compare handler for map_resolve_key
 */
int cmp_fn_var(const void *a, const void *b) {
  var_p_t key_a = (var_p_t)a;
  var_p_t key_b = ((Element *)b)->key;

  int result;
  if (key_a->type == V_STR && key_b->type == V_STR) {
    result = strcasecmp(key_a->v.p.ptr, key_b->v.p.ptr);
  } else {
    result = v_compare(key_a, key_b);
  }
  return result;
}

/**
 * Compare one MAP to another. see v_compare comments for return spec.
 */
int map_compare(const var_p_t var_a, const var_p_t var_b) {
  return 0;
}

/**
 * Return true if the structure is empty
 */
int map_is_empty(const var_p_t var_p) {
  return (var_p->v.m.map == NULL);
}

/**
 * Return the contents of the structure as an integer
 */
int map_to_int(const var_p_t var_p) {
  return map_length(var_p);
}

/**
 * Return the number of elements
 */
int map_length(const var_p_t var_p) {
  int result;
  if (var_p->type == V_MAP) {
    result = var_p->v.m.size;
  } else {
    result = 0;
  }
  return result;
}

void map_get_recurse(CallbackData *cb, const node_t *nodep, const char *name) {
  if (nodep->left != NULL) {
    map_get_recurse(cb, nodep->left, name);
  }
  Element *el = ((Node *)nodep)->element;
  if (el != NULL && el->key->type == V_STR &&
      strcasecmp(el->key->v.p.ptr, name) == 0) {
    cb->el = el;
  }
  if (nodep->right != NULL) {
    map_get_recurse(cb, nodep->right, name);
  }
}

var_p_t map_get(var_p_t base, const char *name) {
  CallbackData cb;
  cb.el = NULL;
  if (base->type == V_MAP) {
    map_get_recurse(&cb, base->v.m.map, name);
  }
  return cb.el != NULL ? cb.el->value : NULL;
}

int map_get_bool(var_p_t base, const char *name) {
  int result = 0;
  var_p_t var = map_get(base, name);
  if (var != NULL) {
    switch (var->type) {
    case V_INT:
      result = (var->v.i != 0);
      break;
    case V_NUM:
      result = (var->v.n != 0);
      break;
    case V_STR:
      result = (strncasecmp(var->v.p.ptr, "true", 4));
      break;
    }
  }
  return result;
}

int map_get_int(var_p_t base, const char *name, int def) {
  int result = def;
  var_p_t var = map_get(base, name);
  if (var != NULL) {
    result = v_igetval(var);
  }
  return result;
}

const char *map_get_str(var_p_t base, const char *name) {
  char *result = NULL;
  var_p_t var = map_get(base, name);
  if (var != NULL && var->type == V_STR) {
    result = var->v.p.ptr;
  }
  return result;
}

void map_elem_recurse(CallbackData *cb, const node_t *nodep) {
  if (nodep->left != NULL) {
    map_elem_recurse(cb, nodep->left);
  }

  if (cb->el == NULL) {
    Element *element = ((Node *)nodep)->element;
    if (cb->count++ == cb->index) {
      cb->el = element;
    }
  }

  if (nodep->right != NULL) {
    map_elem_recurse(cb, nodep->right);
  }
}

/**
 * return the element at the nth position
 */
Element *map_elem(const var_p_t var_p, int index) {
  CallbackData cb;
  cb.count = 0;
  cb.index = index;
  cb.el = NULL;
  if (var_p->type == V_MAP) {
    map_elem_recurse(&cb, var_p->v.m.map);
  }
  return cb.el;
}

/**
 * return the element key at the nth position
 */
var_p_t map_elem_key(const var_p_t var_p, int index) {
  Element *el = map_elem(var_p, index);
  return el != NULL ? el->key : NULL;
}

/**
 * Helper for map_free
 */
void map_free_cb(void *nodep) {
  Element *element = (Element *)nodep;
  delete_element(element);
}

/**
 * Free the map data
 */
void map_free(var_p_t var_p) {
  if (var_p->type == V_MAP) {
    tdestroy(var_p->v.m.map, map_free_cb);
    v_init(var_p);
  }
}

/**
 * Inserts the variable into the b-tree, set result to the key value
 */
void map_insert_key(var_p_t base, Element *key, var_p_t *result) {
  Node *node = tfind(key, &(base->v.m.map), cmp_fn);

  if (node != NULL) {
    // item already exists
    *result = node->element->value;
    delete_element(key);
  }
  else {
    key->value = *result = v_new();
    tsearch(key, &(base->v.m.map), cmp_fn);
    base->v.m.size++;
  }
}

/**
 * Resolve the variable into the b-tree without memory allocation
 */
void map_resolve_key(var_p_t base, var_p_t v_key, var_p_t *result) {
  Node *node = tfind(v_key, &(base->v.m.map), cmp_fn_var);

  if (node != NULL) {
    // item already exists
    *result = node->element->value;
  }
  else {
    Element *key = create_element(v_key);
    key->value = *result = v_new();
    tsearch(key, &(base->v.m.map), cmp_fn);
    base->v.m.size++;
  }
}

/**
 * Returns the final element eg z in foo.x.y.z
 *
 * Scan byte code for node kwTYPE_UDS_EL and attach as field elements
 * if they don't already exist.
 */
var_p_t map_resolve_fields(const var_p_t base) {
  var_p_t field = NULL;
  if (code_peek() == kwTYPE_UDS_EL) {
    code_skipnext();
    if (code_peek() != kwTYPE_STR) {
      err_stackmess();
      return NULL;
    } else {
      code_skipnext();
    }

    if (base->type != V_MAP) {
      if (v_is_nonzero(base)) {
        err_typemismatch();
        return NULL;
      } else {
        map_init(base);
      }
    }

    // evaluate the variable 'key' name
    var_t key;
    v_eval_str(&key);
    map_resolve_key(base, &key, &field);
    v_free(&key);

    // evaluate the next sub-element
    field = map_resolve_fields(field);
  } else {
    field = base;
  }
  return field;
}

/**
 * Adds a new variable onto the map
 */
var_p_t map_add_var(var_p_t base, const char *name, int value) {
  var_p_t result;
  Element *node = (Element *)malloc(sizeof(Element));
  node->key = v_new();
  node->value = NULL;
  v_setstr(node->key, name);
  map_insert_key(base, node, &result);
  v_setint(result, value);
  return result;
}

/**
 * Return the variable in base keyed by key, if not found then creates
 * an empty variable that will be returned in a further call
 */
void map_get_value(var_p_t base, var_p_t var_key, var_p_t *result) {
  if (base->type == V_ARRAY && base->v.a.size) {
    // convert the non-empty array to a map
    int i;
    var_t *clone = v_clone(base);

    map_init(base);
    for (i = 0; i < clone->v.a.size; i++) {
      const var_t *element = (var_t *)(clone->v.a.ptr + (sizeof(var_t) * i));
      Element *key = create_int_element(i);
      key->value = v_new();
      v_set(key->value, element);
      tsearch(key, &(base->v.m.map), cmp_fn);
    }

    // free the clone
    v_free(clone);
    free(clone);
  } else if (base->type != V_MAP) {
    if (v_is_nonzero(base)) {
      err_typemismatch();
      return;
    } else {
      map_init(base);
    }
  }
  map_resolve_key(base, var_key, result);
}

/**
 * Traverse the root to copy into dest
 */
void map_set_recurse(var_p_t dest, var_p_t root, int start, int end) {
  int count = end - start;
  int apex = start + (count / 2);

  Element *element = map_elem(root, apex);
  Element *key = create_element(element->key);
  key->value = v_new();
  v_set(key->value, element->value);
  tsearch(key, &(dest->v.m.map), cmp_fn);
  if (key->value->type == V_FUNC) {
    key->value->v.fn.self = dest;
  }

  if (apex > start) {
    map_set_recurse(dest, root, start, apex);
  }
  if (apex + 1 < end) {
    map_set_recurse(dest, root, apex + 1, end);
  }
}

/**
 * Copy values from one structure to another
 */
void map_set(var_p_t dest, const var_p_t src) {
  if (dest != src && src->type == V_MAP) {
    map_init(dest);
    dest->v.m.size = src->v.m.size;
    map_set_recurse(dest, src, 0, src->v.m.size);
  }
}

void map_set_int(var_p_t base, const char *name, var_int_t n) {
  var_p_t var = map_get(base, name);
  if (var != NULL) {
    v_setint(var, n);
  } else {
    map_add_var(base, name, n);
  }
}

/**
 * Helper for map_to_str
 */
void map_to_str_cb(CallbackData *cb, const node_t *nodep) {
  Element *element = ((Node *)nodep)->element;
  char *key = v_str(element->key);
  char *value = v_str(element->value);
  int required = strlen(cb->buffer) + strlen(key) + strlen(value) + BUFFER_PADDING;
  if (required >= cb->count) {
    cb->count = required + BUFFER_GROW_SIZE;
    cb->buffer = realloc(cb->buffer, cb->count);
  }
  if (!cb->firstElement) {
    strcat(cb->buffer, ",");
  }
  cb->firstElement = 0;
  strcat(cb->buffer, "\"");
  strcat(cb->buffer, key);
  strcat(cb->buffer, "\"");
  strcat(cb->buffer, ":");
  if (element->value->type == V_STR) {
    strcat(cb->buffer, "\"");
  }
  strcat(cb->buffer, value);
  if (element->value->type == V_STR) {
    strcat(cb->buffer, "\"");
  }
  free(key);
  free(value);
}

/**
 * Helper for map_to_str
 */
void map_to_str_recurse(CallbackData *cb, const node_t *nodep) {
  if (nodep->left != NULL) {
    map_to_str_recurse(cb, nodep->left);
  }
  map_to_str_cb(cb, nodep);
  if (nodep->right != NULL) {
    map_to_str_recurse(cb, nodep->right);
  }
}

/**
 * Print the array element, growing the buffer as needed
 */
void array_append_elem(CallbackData *cb, var_t *elem) {
  char *value = v_str(elem);
  int required = strlen(cb->buffer) + strlen(value) + BUFFER_PADDING;
  if (required >= cb->count) {
    cb->count = required + BUFFER_GROW_SIZE;
    cb->buffer = realloc(cb->buffer, cb->count);
  }
  strcat(cb->buffer, value);
  free(value);
}

/**
 * print the array variable
 */
void array_to_str(CallbackData *cb, var_t *var) {
  strcpy(cb->buffer, "[");
  if (var->v.a.maxdim == 2) {
    // NxN
    int rows = ABS(var->v.a.ubound[0] - var->v.a.lbound[0]) + 1;
    int cols = ABS(var->v.a.ubound[1] - var->v.a.lbound[1]) + 1;
    int i, j;

    for (i = 0; i < rows; i++) {
      for (j = 0; j < cols; j++) {
        int pos = i * cols + j;
        var_t *elem = (var_t *)(var->v.a.ptr + (sizeof(var_t) * pos));
        array_append_elem(cb, elem);
        if (j != cols - 1) {
          strcat(cb->buffer, ",");
        }
      }
      if (i != rows - 1) {
        strcat(cb->buffer, ";");
      }
    }
  } else {
    int i;
    for (i = 0; i < var->v.a.size; i++) {
      var_t *elem = (var_t *)(var->v.a.ptr + (sizeof(var_t) * i));
      array_append_elem(cb, elem);
      if (i != var->v.a.size - 1) {
        strcat(cb->buffer, ",");
      }
    }
  }
  strcat(cb->buffer, "]");
}

/**
 * Return the contents of the structure as a string
 */
char *map_to_str(const var_p_t var_p) {
  CallbackData cb;
  cb.count = BUFFER_GROW_SIZE;
  cb.buffer = malloc(cb.count);

  if (var_p->type == V_MAP) {
    cb.firstElement = 1;
    strcpy(cb.buffer, "{");
    map_to_str_recurse(&cb, var_p->v.m.map);
    strcat(cb.buffer, "}");
  } else if (var_p->type == V_ARRAY) {
    array_to_str(&cb, var_p);
  }
  return cb.buffer;
}

/**
 * Print the contents of the structure
 */
void map_write(const var_p_t var_p, int method, int handle) {
  if (var_p->type == V_MAP || var_p->type == V_ARRAY) {
    char *buffer = map_to_str(var_p);
    pv_write(buffer, method, handle);
    free(buffer);
  }
}

/**
 * Creates an array variable
 */
int map_create_array(var_p_t dest, JsonTokens *json, int end_position, int index) {
  int size = ARRAY_GROW_SIZE;
  int item_index = 0;
  v_toarray1(dest, size);
  int i = index;
  while (i < json->num_tokens) {
    jsmntok_t token = json->tokens[i];
    if (token.start > end_position) {
      break;
    }
    if (item_index >= size) {
      size += ARRAY_GROW_SIZE;
      v_resize_array(dest, size);
    }
    var_t *elem = (var_t *)(dest->v.a.ptr + (sizeof(var_t) * item_index));
    i = map_read_next_token(elem, json, i);
    item_index++;
  }
  v_resize_array(dest, item_index);
  return i;
}

/**
 * Process the next primative value
 */
void map_set_primative(var_p_t dest, const char *s, int len) {
  int value = 0;
  int fract = 0;
  int text = 0;
  int i;
  for (i = 0; i < len && !text; i++) {
    int n = s[i] - '0';
    if (n >= 0 && n <= 9) {
      value = value * 10 + n;
    } else if (!fract && s[i] == '.') {
      fract = 1;
    } else {
      text = 1;
    }
  }
  if (text) {
    v_setstrn(dest, s, len);
  } else if (fract) {
    v_setreal(dest, atof(s));
  } else {
    v_setint(dest, value);
  }
}

/**
 * Creates a map variable
 */
int map_create(var_p_t dest, JsonTokens *json, int end_position, int index) {
  map_init(dest);
  int i = index;
  while (i < json->num_tokens) {
    jsmntok_t token = json->tokens[i];
    if (token.start > end_position) {
      break;
    } else if (token.type == JSMN_STRING || token.type == JSMN_PRIMITIVE) {
      var_p_t value = NULL;
      Element *element = (Element *)malloc(sizeof(Element));
      element->key = v_new();
      element->value = NULL;
      map_set_primative(element->key, json->js + token.start, token.end - token.start);
      map_insert_key(dest, element, &value);
      i = map_read_next_token(value, json, i + 1);
    } else {
      err_array();
      break;
    }
  }
  return i;
}

/**
 * Process the next token
 */
int map_read_next_token(var_p_t dest, JsonTokens *json, int index) {
  int next;
  jsmntok_t token = json->tokens[index];
  switch(token.type) {
  case JSMN_OBJECT:
    next = map_create(dest, json, token.end, index + 1);
    break;
  case JSMN_ARRAY:
    next = map_create_array(dest, json, token.end, index + 1);
    break;
  case JSMN_PRIMITIVE:
    map_set_primative(dest, json->js + token.start, token.end - token.start);
    next = index + 1;
    break;
  case JSMN_STRING:
    v_setstrn(dest, json->js + token.start, token.end - token.start);
    next = index + 1;
    break;
  default:
    next = -1;
    break;
  }
  return next;
}

/**
 * Initialise a map from a string
 */
void map_from_str(var_p_t dest) {
  var_t arg;
  v_init(&arg);
  eval(&arg);
  if (!prog_error) {
    if (arg.type != V_STR) {
      v_set(dest, &arg);
    } else {
      int num_tokens = TOKEN_GROW_SIZE;
      jsmntok_t *tokens = malloc(sizeof(jsmntok_t) * num_tokens);
      const char *js = arg.v.p.ptr;
      size_t len = arg.v.p.size;
      jsmnerr_t result;
      jsmn_parser parser;

      jsmn_init(&parser);
      for (result = jsmn_parse(&parser, js, len, tokens, num_tokens);
           result == JSMN_ERROR_NOMEM;
           result = jsmn_parse(&parser, js, len, tokens, num_tokens)) {
        num_tokens += TOKEN_GROW_SIZE;
        tokens = realloc(tokens, sizeof(jsmntok_t) * num_tokens);
      }
      if (result < 0) {
        err_array();
      } else if (result > 0) {
        v_init(dest);

        JsonTokens json;
        json.tokens = tokens;
        json.js = js;
        json.num_tokens = parser.toknext;
        map_read_next_token(dest, &json, 0);
      }
      free(tokens);
    }
  }
  v_free(&arg);
}


