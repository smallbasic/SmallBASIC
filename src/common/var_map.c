// This file is part of SmallBASIC
//
// Support for dictionary/associative array variables
// eg: dim foo: key="blah": foo(key) = "something": ? foo(key)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2010-2019 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

#include "common/sys.h"
#include "common/pproc.h"
#include "common/hashmap.h"
#include "common/plugins.h"
#include "include/var_map.h"

#define BUFFER_GROW_SIZE 64
#define BUFFER_PADDING   10
#define TOKEN_GROW_SIZE  16
#define JSMN_STATIC

#include "lib/jsmn/jsmn.h"

//
// Container for map_from_str
//
typedef struct JsonTokens {
  const char *js;
  jsmntok_t *tokens;
  int num_tokens;
} JsonTokens;

struct ArrayNode;
typedef struct ArrayNode {
  var_t *v;
  int col;
  int row;
  struct ArrayNode *next;
} ArrayNode;

typedef struct ArrayList {
  ArrayNode *head;
  ArrayNode *tail;
} ArrayList;

//
// Process the next token
//
int map_read_next_token(var_p_t dest, JsonTokens *json, int index);

//
// initialise the variable as a map
//
void map_init(var_p_t map) {
  hashmap_create(map, 0);
}

//
// Compare one MAP to another. see v_compare comments for return spec.
//
int map_compare(const var_p_t var_a, const var_p_t var_b) {
  return 0;
}

//
// Return true if the structure is empty
//
int map_is_empty(const var_p_t var_p) {
  return (var_p->v.m.map == NULL);
}

//
// Return the contents of the structure as an integer
//
int map_to_int(const var_p_t var_p) {
  return map_length(var_p);
}

//
// Return the number of elements
//
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
  if (base != NULL && base->type == V_MAP) {
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

//
// return the element at the nth position
//
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

//
// return the element key at the nth position
//
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

//
// Free the map data
//
void map_free(var_p_t var_p) {
  if (var_p->type == V_MAP) {
    if (var_p->v.m.lib_id != -1 &&
        var_p->v.m.cls_id != -1 &&
        var_p->v.m.id != -1) {
      plugin_free(var_p->v.m.lib_id, var_p->v.m.cls_id, var_p->v.m.id);
    }
    hashmap_destroy(var_p);
    v_init(var_p);
  }
}

//
// callback for map_set_lib_id
//
int map_set_lib_id_cb(hashmap_cb *cb, var_p_t key, var_p_t value) {
  if (v_is_type(value, V_MAP)) {
    value->v.m.lib_id = cb->index;
  }
  return 0;
}

//
// sets the library-id onto the map parent and direct children
//
void map_set_lib_id(var_p_t var_p, int lib_id) {
  if (v_is_type(var_p, V_MAP)) {
    hashmap_cb cb;
    var_p->v.m.lib_id = lib_id;
    cb.index = lib_id;
    hashmap_foreach(var_p, map_set_lib_id_cb, &cb);
  }
}

//
// Returns the final element eg z in foo.x.y.z
// Scan byte code for node kwTYPE_UDS_EL and attach as field elements
// if they don't already exist.
//
var_p_t map_resolve_fields(var_p_t base, var_p_t *parent) {
  var_p_t field = NULL;
  if (code_peek() == kwTYPE_UDS_EL) {
    code_skipnext();
    if (code_peek() != kwTYPE_STR) {
      err_stackmess();
      return NULL;
    } else {
      code_skipnext();
    }

    if (base->type == V_REF) {
      base = base->v.ref;
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
    field = hashmap_putc(base, key, len);
    if (parent != NULL) {
      *parent = base;
    }

    // evaluate the next sub-element
    field = map_resolve_fields(field, parent);
  } else {
    field = base;
  }
  return field;
}

//
// Adds a new variable onto the map
//
var_p_t map_add_var(var_p_t base, const char *name, int value) {
  var_p_t key = v_new();
  v_setstr(key, name);
  var_p_t var = hashmap_putv(base, key);
  v_setint(var, value);
  return var;
}

//
// Return the variable in base keyed by key, if not found then creates
// an empty variable that will be returned in a further call
//
void map_get_value(var_p_t base, var_p_t var_key, var_p_t *result) {
  if (base->type == V_ARRAY && v_asize(base)) {
    // convert the non-empty array to a map
    var_t *clone = v_clone(base);

    hashmap_create(base, 0);
    for (int i = 0; i < v_asize(clone); i++) {
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
      *result = v_new();
      v_set(*result, base);
      return;
    } else {
      hashmap_create(base, 0);
    }
  }

  v_tostr(var_key);
  *result = hashmap_put(base, var_key->v.p.ptr, v_strlen(var_key));
}

//
// Traverse the root to copy into dest
//
int map_set_cb(hashmap_cb *cb, var_p_t var_key, var_p_t value) {
  if (var_key->type != V_STR || var_key->v.p.ptr[0] != MAP_TMP_FIELD[0]) {
    var_p_t key = v_new();
    v_set(key, var_key);
    var_p_t var = hashmap_putv(cb->var, key);
    v_set(var, value);
  }
  return 0;
}

//
// Copy values from one structure to another
//
void map_set(var_p_t dest, const var_p_t src) {
  if (dest != src && src->type == V_MAP) {
    hashmap_cb cb;
    cb.var = dest;
    hashmap_create(dest, src->v.m.count);
    hashmap_foreach(src, map_set_cb, &cb);
    dest->v.m.count = src->v.m.count;
    dest->v.m.id = src->v.m.id;
    dest->v.m.lib_id = -1;
    dest->v.m.cls_id = -1;
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

//
// Helper for map_to_str
//
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

//
// Print the array element, growing the buffer as needed
//
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

//
// print the array variable
//
void array_to_str(hashmap_cb *cb, var_t *var) {
  strcpy(cb->buffer, "[");
  if (v_maxdim(var) == 2) {
    // NxN
    int rows = ABS(v_ubound(var, 0) - v_lbound(var, 0)) + 1;
    int cols = ABS(v_ubound(var, 1) - v_lbound(var, 1)) + 1;

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
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
    for (int i = 0; i < v_asize(var); i++) {
      var_t *elem = v_elem(var, i);
      array_append_elem(cb, elem);
      if (i != v_asize(var) - 1) {
        strcat(cb->buffer, ",");
      }
    }
  }
  strcat(cb->buffer, "]");
}

//
// Return the contents of the structure as a string
//
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

//
// Print the contents of the structure
//
void map_write(const var_p_t var_p, int method, intptr_t handle) {
  if (var_p->type == V_MAP || var_p->type == V_ARRAY) {
    char *buffer = map_to_str(var_p);
    pv_write(buffer, method, handle);
    free(buffer);
  }
}

//
// Process the next primative value
//
void map_set_primative(var_p_t dest, const char *s, int len) {
  int value = 0;
  int fract = 0;
  int text = 0;
  int sign = 1;
  for (int i = 0; i < len && !text; i++) {
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
    if (len == 4 && strncasecmp(s, "true", len) == 0) {
      v_setint(dest, 1);
    } else if (len == 5 && strncasecmp(s, "false", len) == 0) {
      v_setint(dest, 0);
    } else {
      v_setstrn(dest, s, len);
    }
  } else if (fract) {
    v_setreal(dest, atof(s));
  } else {
    v_setint(dest, sign * value);
  }
}

//
// Adds a node to the array list
//
var_t *map_array_list_add(ArrayList *list, int row, int col) {
  if (list->head == NULL) {
    list->head = malloc(sizeof(ArrayNode));
    list->tail = list->head;
  } else {
    list->tail->next = malloc(sizeof(ArrayNode));
    list->tail = list->tail->next;
  }
  list->tail->next = NULL;
  list->tail->row = row;
  list->tail->col = col;
  list->tail->v = v_new();
  return list->tail->v;
}

//
// Builds the array from the ArrayNode list
//
void map_build_array(var_p_t dest, ArrayNode *node_next, int rows, int cols) {
  if (rows > 1) {
    v_tomatrix(dest, rows, cols);
  } else {
    v_toarray1(dest, cols);
  }
  while (node_next) {
    int pos = node_next->row * cols + node_next->col;
    v_set(v_elem(dest, pos), node_next->v);
    v_free(node_next->v);
    v_detach(node_next->v);
    ArrayNode *next = node_next->next;
    free(node_next);
    node_next = next;
  }
}

//
// Creates an array variable
//
int map_create_array(var_p_t dest, JsonTokens *json, int end_position, int index) {
  int i = index;
  int rows = 0;
  int cols = 0;
  int curcol = 0;
  ArrayList list;

  list.head = NULL;
  list.tail = NULL;

  while (i < json->num_tokens) {
    jsmntok_t token = json->tokens[i];
    if (token.start > end_position) {
      break;
    }
    var_t *elem = map_array_list_add(&list, rows, curcol++);
    int len = token.end - token.start;
    const char *str = json->js + token.start;
    const char *delim = memchr(str, ';', len);
    if (token.type == JSMN_PRIMITIVE && (delim != NULL || json->tokens[0].type == JSMN_ARRAY)) {
      if (delim != NULL) {
        if ((delim - str) > 0) {
          map_set_primative(elem, str, delim - str);
          if (curcol > cols) {
            cols = curcol;
          }
        }
        while (delim != NULL) {
          rows++;
          curcol = 0;
          len -= (delim - str) + 1;
          if (len > 0) {
            // text exists beyond ';'
            str = ++delim;
            if (*str != ';') {
              elem = map_array_list_add(&list, rows, curcol++);
              map_set_primative(elem, str, len);
            }
            delim = memchr(str, ';', len);
          } else {
            // no more text, just count the new row
            delim = NULL;
          }
        }
      } else {
        map_set_primative(elem, json->js + token.start, token.end - token.start);
      }
      i++;
    } else {
      i = map_read_next_token(elem, json, i);
    }
    if (curcol > cols) {
      cols = curcol;
    }
  }
  map_build_array(dest, list.head, rows + 1, cols);
  return i;
}

//
// Creates a map variable
//
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

//
// Process the next token
//
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

//
// Initialise a map from a string
//
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

//
// array <- CODEARRAY(x1,y1...[;x2,y2...])
// dynamic arrays created with the [] operators
//
void map_from_codearray(var_p_t dest) {
  int rows = 0;
  int cols = 0;
  int curcol = 0;
  int ready = 0;
  int seps = 0;
  ArrayList list;

  list.head = NULL;
  list.tail = NULL;

  do {
    switch (code_peek()) {
    case kwTYPE_SEP:
      seps++;
      code_skipnext();
      if (code_peek() == ';') {
        // next row
        rows++;
        curcol = 0;
      } else if (++curcol > cols) {
        // next col
        cols = curcol;
      }
      code_skipnext();
      break;
    case kwTYPE_LEVEL_END:
      ready = 1;
      break;
    default:
      eval(map_array_list_add(&list, rows, curcol));
    }
  } while (!ready && !prog_error);

  if (!seps && list.head == NULL) {
    v_toarray1(dest, 0);
  } else {
    map_build_array(dest, list.head, rows+1, cols+1);
  }
}
