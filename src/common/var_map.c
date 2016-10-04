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
#include "common/pproc.h"
#include "common/var_map.h"
#include "common/hashmap.h"
#include "lib/jsmn.h"

#define BUFFER_GROW_SIZE 64
#define BUFFER_PADDING   10
#define TOKEN_GROW_SIZE  16
#define ARRAY_GROW_SIZE  8

/**
 * Container for map_from_str
 */
typedef struct JsonTokens {
  const char *js;
  jsmntok_t *tokens;
  int num_tokens;
  int code_array;
} JsonTokens;

typedef struct cint_list {
  int *data;
  int length;
  int size;
  int grow_size;
} cint_list;

void cint_list_init(cint_list *cl, int size) {
  cl->length = 0;
  cl->size = size;
  cl->grow_size = size;
  cl->data = malloc(sizeof(int) * cl->grow_size);
}

void cint_list_append(cint_list *cl, int value) {
  if (cl->size - cl->length == 0) {
    cl->size += cl->grow_size;
    cl->data = realloc(cl->data, sizeof(int) * cl->size);
  }
  cl->data[cl->length++] = value;
}

struct array_node;

typedef struct array_node {
  var_t *v;
  int col;
  int row;
  struct array_node *next;
} array_node;

/**
 * Process the next token
 */
int map_read_next_token(var_p_t dest, JsonTokens *json, int index);

/**
 * initialise the variable as a map
 */
void map_init(var_p_t map) {
  hashmap_create(map, 0);
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
    result = var_p->v.m.count;
  } else {
    result = 0;
  }
  return result;
}

var_p_t map_get(var_p_t base, const char *name) {
  var_p_t result;
  if (base->type == V_MAP) {
    result = hashmap_get(base, name);
  } else {
    result = NULL;
  }
  return result;
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
  var_p_t var = map_get(base, name);
  return var != NULL ? v_igetval(var) : def;
}

const char *map_get_str(var_p_t base, const char *name) {
  char *result;
  var_p_t var = map_get(base, name);
  if (var != NULL && var->type == V_STR) {
    result = var->v.p.ptr;
  } else {
    result = NULL;
  }
  return result;
}

/**
 * return the element at the nth position
 */
int map_elem_cb(hashmap_cb *cb, var_p_t key, var_p_t value) {
  int result;
  if (cb->count++ == cb->index) {
    cb->var = key;
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

/**
 * return the element key at the nth position
 */
var_p_t map_elem_key(const var_p_t var_p, int index) {
  var_p_t result;
  if (var_p->type == V_MAP) {
    hashmap_cb cb;
    cb.count = 0;
    cb.index = index;
    cb.var = NULL;
    hashmap_foreach(var_p, map_elem_cb, &cb);
    result = cb.var;
  } else {
    result = NULL;
  }
  return result;
}

/**
 * Free the map data
 */
void map_free(var_p_t var_p) {
  if (var_p->type == V_MAP) {
    hashmap_destroy(var_p);
    v_init(var_p);
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
        hashmap_create(base, 0);
      }
    }

    // evaluate the variable 'key' name
    int len = code_getstrlen();
    const char *key = (const char *)&prog_source[prog_ip];
    prog_ip += len;
    field = hashmap_put(base, key, len);

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
  var_p_t key = v_new();
  v_setstr(key, name);
  var_p_t var = hashmap_putv(base, key);
  v_setint(var, value);
  return var;
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

    hashmap_create(base, 0);
    for (i = 0; i < clone->v.a.size; i++) {
      const var_t *element = v_elem(clone, i);
      var_p_t key = v_new();
      v_setint(key, i);
      var_p_t value = hashmap_putv(base, key);
      v_set(value, element);
    }

    // free the clone
    v_free(clone);
    v_detach(clone);
  } else if (base->type != V_MAP) {
    if (v_is_nonzero(base)) {
      err_typemismatch();
      return;
    } else {
      hashmap_create(base, 0);
    }
  }

  v_tostr(var_key);
  *result = hashmap_put(base, var_key->v.p.ptr, v_strlen(var_key));
}

/**
 * Traverse the root to copy into dest
 */
int map_set_cb(hashmap_cb *cb, var_p_t var_key, var_p_t value) {
  var_p_t key = v_new();
  v_set(key, var_key);
  var_p_t var = hashmap_putv(cb->var, key);
  v_set(var, value);
  if (var->type == V_FUNC) {
    var->v.fn.self = cb->var;
  }
  return 0;
}

/**
 * Copy values from one structure to another
 */
void map_set(var_p_t dest, const var_p_t src) {
  if (dest != src && src->type == V_MAP) {
    hashmap_cb cb;
    cb.var = dest;
    hashmap_create(dest, src->v.m.count);
    hashmap_foreach(src, map_set_cb, &cb);
    dest->v.m.count = src->v.m.count;
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
int map_to_str_cb(hashmap_cb *cb, var_p_t v_key, var_p_t v_var) {
  char *key = v_str(v_key);
  char *value = v_str(v_var);
  int required = strlen(cb->buffer) + strlen(key) + strlen(value) + BUFFER_PADDING;
  if (required >= cb->count) {
    cb->count = required + BUFFER_GROW_SIZE;
    cb->buffer = realloc(cb->buffer, cb->count);
  }
  if (!cb->start) {
    strcat(cb->buffer, ",");
  }
  cb->start = 0;
  strcat(cb->buffer, "\"");
  strcat(cb->buffer, key);
  strcat(cb->buffer, "\"");
  strcat(cb->buffer, ":");
  if (v_var->type == V_STR) {
    strcat(cb->buffer, "\"");
  }
  strcat(cb->buffer, value);
  if (v_var->type == V_STR) {
    strcat(cb->buffer, "\"");
  }
  free(key);
  free(value);
  return 0;
}

/**
 * Print the array element, growing the buffer as needed
 */
void array_append_elem(hashmap_cb *cb, var_t *elem) {
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
void array_to_str(hashmap_cb *cb, var_t *var) {
  strcpy(cb->buffer, "[");
  if (var->v.a.maxdim == 2) {
    // NxN
    int rows = ABS(var->v.a.ubound[0] - var->v.a.lbound[0]) + 1;
    int cols = ABS(var->v.a.ubound[1] - var->v.a.lbound[1]) + 1;
    int i, j;

    for (i = 0; i < rows; i++) {
      for (j = 0; j < cols; j++) {
        int pos = i * cols + j;
        var_t *elem = v_elem(var, pos);
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
      var_t *elem = v_elem(var, i);
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
  hashmap_cb cb;
  cb.count = BUFFER_GROW_SIZE;
  cb.buffer = malloc(cb.count);

  if (var_p->type == V_MAP) {
    cb.start = 1;
    strcpy(cb.buffer, "{");
    hashmap_foreach(var_p, map_to_str_cb, &cb);
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
 * Process the next primative value
 */
void map_set_primative(var_p_t dest, const char *s, int len) {
  int value = 0;
  int fract = 0;
  int text = 0;
  int sign = 1;
  int i;
  for (i = 0; i < len && !text; i++) {
    int n = s[i] - '0';
    if (n >= 0 && n <= 9) {
      value = value * 10 + n;
    } else if (!fract && s[i] == '.') {
      fract = 1;
    } else if (s[i] == '-' && sign) {
      sign = -1;
    } else {
      text = 1;
    }
  }
  if (text) {
    v_setstrn(dest, s, len);
  } else if (fract) {
    v_setreal(dest, atof(s));
  } else {
    v_setint(dest, sign * value);
  }
}

/**
 * Handle the semi-colon row separator character
 */
int map_read_next_array_token(JsonTokens *json, int index, var_p_t dest,
                              int count, int *new_elems, int *new_rows) {
  int next;
  jsmntok_t token = json->tokens[index];
  var_t *elem = v_elem(dest, count);

  if (token.type == JSMN_PRIMITIVE && json->tokens[0].type == JSMN_ARRAY) {
    int len = token.end - token.start;
    const char *str = json->js + token.start;
    const char *delim = memchr(str, ';', len);
    if (delim != NULL) {
      map_set_primative(elem, str, delim - str);
      while (delim != NULL) {
        len -= (delim - str) + 1;
        if (len > 0) {
          // text exists beyond ';'
          if (++count >= dest->v.a.size) {
            int size = dest->v.a.size + ARRAY_GROW_SIZE;
            v_resize_array(dest, size);
          }
          elem = v_elem(dest, count);
          str = ++delim;
          map_set_primative(elem, str, len);
          delim = memchr(str, ';', len);
          (*new_elems)++;
        } else {
          // no more text, just count the new row
          delim = NULL;
        }
        (*new_rows)++;
      }
    } else {
      map_set_primative(elem, json->js + token.start, token.end - token.start);
    }
    next = index + 1;
  } else {
    next = map_read_next_token(elem, json, index);
  }
  return next;
}

/**
 * change the grid dimensions when row separator found
 */
void map_resize_array(var_p_t dest, int size, int rows, int cols, cint_list *offs) {
  v_resize_array(dest, size);
  if (rows > 1) {
    int idx, row;
    var_t *var = v_new();
    v_tomatrix(var, rows, cols);
    for (idx = 0, row = -1; idx < size; idx++) {
      if (!offs->data[idx]) {
        // start of next row
        row++;
      }
      int pos = row * cols + offs->data[idx];
      v_set(v_elem(var, pos), v_elem(dest, idx));
    }
    v_set(dest, var);
    v_free(var);
    v_detach(var);
  }
}

/**
 * Creates an array variable
 */
int map_create_array(var_p_t dest, JsonTokens *json, int end_position, int index) {
  int count = 0;
  int i = index;
  int cols = 1;
  int rows = 1;
  int curcol = 0;
  cint_list offs;

  cint_list_init(&offs, ARRAY_GROW_SIZE);
  v_toarray1(dest, ARRAY_GROW_SIZE);
  while (i < json->num_tokens) {
    int new_elems = 0;
    int new_rows = 0;
    jsmntok_t token = json->tokens[i];
    if (token.start > end_position) {
      break;
    }
    if (count >= dest->v.a.size) {
      int size = dest->v.a.size + ARRAY_GROW_SIZE;
      v_resize_array(dest, size);
    }
    cint_list_append(&offs, curcol);
    i = map_read_next_array_token(json, i, dest, count, &new_elems, &new_rows);
    if (new_rows) {
      int j;
      for (j = 0; j < new_elems; j++) {
        cint_list_append(&offs, 0);
        count++;
      }
      rows += new_rows;
      // when no added cells, make curcol reset to zero
      curcol = !new_elems ? -1 : 0;
    }
    if (++curcol > cols) {
      cols = curcol;
    }
    count++;
  }
  map_resize_array(dest, count, rows, cols, &offs);
  free(offs.data);
  return i;
}

/**
 * Creates a map variable
 */
int map_create(var_p_t dest, JsonTokens *json, int end_position, int index) {
  hashmap_create(dest, 0);
  int i = index;
  while (i < json->num_tokens) {
    jsmntok_t token = json->tokens[i];
    if (token.start > end_position) {
      break;
    } else if (token.type == JSMN_STRING || token.type == JSMN_PRIMITIVE) {
      var_p_t key = v_new();
      map_set_primative(key, json->js + token.start, token.end - token.start);
      var_p_t value = hashmap_putv(dest, key);
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
  switch (token.type) {
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

void map_parse_str(const char *js, size_t len, var_p_t dest) {
  int num_tokens = TOKEN_GROW_SIZE;
  jsmntok_t *tokens = malloc(sizeof(jsmntok_t) * num_tokens);
  int result;
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
      map_parse_str(arg.v.p.ptr, arg.v.p.length, dest);
    }
  }
  v_free(&arg);
}

// array <- CODEARRAY(x1,y1...[;x2,y2...])
// dynamic arrays created with the [] operators
void map_from_codearray(var_p_t dest) {
  int count = 0;
  int cols = 0;
  int rows = 0;
  int curcol = 0;
  int ready = 0;
  array_node *node_head = NULL;
  array_node *node_tail = NULL;

  do {
    switch (code_peek()) {
    case kwTYPE_SEP:
      code_skipnext();
      if (code_peek() == ';') {
        // next row
        rows++;
        curcol = 0;
      } else {
        // next col
        if (++curcol > cols) {
          cols = curcol;
        }
      }
      code_skipnext();
      break;
    case kwTYPE_LEVEL_END:
      ready = 1;
      break;
    default:
      if (node_head == NULL) {
        node_head = malloc(sizeof(array_node));
        node_tail = node_head;
      } else {
        node_tail->next = malloc(sizeof(array_node));
        node_tail = node_tail->next;
      }
      node_tail->next = NULL;
      node_tail->col = curcol;
      node_tail->row = rows;
      node_tail->v = v_new();
      eval(node_tail->v);
      count++;
    }
  } while (!ready && !prog_error);

  // create the array
  rows++;
  cols++;
  if (rows > 1) {
    v_tomatrix(dest, rows, cols);
  } else {
    v_toarray1(dest, cols);
  }

  array_node *node_next = node_head;
  while (node_next) {
    int pos = node_next->row * cols + node_next->col;
    v_set(v_elem(dest, pos), node_next->v);
    v_free(node_next->v);
    v_detach(node_next->v);
    array_node *next = node_next->next;
    free(node_next);
    node_next = next;
  }
}
