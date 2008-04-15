// -*- c-file-style: "java" -*-
// $Id$
// This file is part of SmallBASIC
//
// user-defined structures
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2007 Chris Warren-Smith. [http://tinyurl.com/ja2ss]


#include "sys.h"
#include "var.h"
#include "smbas.h"

/**
 * Compare on UDS to another. see v_compare comments for return spec.
 */
int uds_compare(const var_p_t var_a, const var_p_t var_b)
{
  uds_field_s *next_a = var_a->v.uds;

  while (next_a) {
    uds_field_s *next_b = var_b->v.uds;
    while (next_b) {
      if (next_a->field_id == next_b->field_id) {
        int comp = v_compare(next_a->var, next_b->var);
        if (comp != 0) {
          return comp;
        }
      }
      next_b = next_b->next;
    }
    next_a = next_a->next;
  }

  // must have the same number of elements to fully match
  return uds_to_int(var_a) == uds_to_int(var_b) ? 0 : -1;
}

/**
 * Return true if the structure is empty
 */
int uds_is_empty(var_p_t var_p)
{
  return (var_p->v.uds == 0);
}

/**
 * Return the contents of the structure as an integer
 */
int uds_to_int(const var_p_t var_p)
{
  int n = 0;
  if (var_p->type == V_UDS) {
    uds_field_s *next = var_p->v.uds;
    while (next) {
      n += uds_to_int(next->var) + 1;
      next = next->next;
    }
  }
  return n;
}

/**
 * helper for uds_resolve_fields
 */
uds_field_s *uds_new_field(addr_t field_id)
{
  uds_field_s *field;
  field = tmp_alloc(sizeof(uds_field_s));
  field->next = 0;
  field->field_id = field_id;
  field->var = v_new();
  return field;
}

/**
 * Scan byte code for node kwTYPE_UDS_EL and attach as field elements
 * if they don't already exist.
 * returns the final element eg z in foo.x.y.z
 */
var_p_t uds_resolve_fields(const var_p_t var_p)
{
  var_p_t field = 0;            // for code "foo.x.y.z" return "z"

  if (code_peek() == kwTYPE_UDS_EL) {
    code_skipnext();
    addr_t field_id = code_getaddr();

    if (var_p->type != V_UDS) {
      v_free(var_p);
      var_p->type = V_UDS;
    }

    uds_field_s *next = var_p->v.uds;
    uds_field_s *last = 0;

    while (next) {
      last = next;              // save tail
      if (next->field_id == field_id) {
        field = next->var;
        break;
      }
      next = next->next;
    }
    if (field == 0) {
      // append field to list
      if (last != 0) {
        // append to existing list of elements
        last->next = uds_new_field(field_id);
        field = last->next->var;
      }
      else {
        // create first node on var_p->v.uds
        var_p->v.uds = uds_new_field(field_id);
        field = var_p->v.uds->var;
      }
    }
    field = uds_resolve_fields(field);
  }
  else {
    field = var_p;
  }

  return field;
}

/**
 * empty struct values
 */
void uds_clear(const var_p_t var)
{
  uds_field_s *next = var->v.uds;
  while (next) {
    v_free(next->var);
    next = next->next;
  }
}

/**
 * Helper for uds_free
 */
void uds_free_element(uds_field_s * element)
{
  if (element) {
    uds_free_element(element->next);
    v_free(element->var);
    tmp_free(element);
  }
}

/**
 * Delete the given structure
 */
void uds_free(var_p_t var_p)
{
  uds_free_element(var_p->v.uds);
  var_p->v.uds = 0;
}

/**
 * copy values from one structure to another
 */
void uds_set(var_p_t dest, const var_p_t src)
{
  v_free(dest);
  dest->type = V_UDS;

  uds_field_s *dest_node = 0;
  uds_field_s *src_node = src->v.uds;

  while (src_node) {
    if (dest->v.uds == 0) {
      // head of the list
      dest->v.uds = uds_new_field(src_node->field_id);
      v_set(dest->v.uds->var, src_node->var);
      dest_node = dest->v.uds;
    }
    else {
      // subsequent list item
      dest_node->next = uds_new_field(src_node->field_id);
      v_set(dest_node->next->var, src_node->var);
      dest_node = dest_node->next;
    }
    src_node = src_node->next;
  }
}

/**
 * Return the contents of the structure as a string
 */
void uds_to_str(const var_p_t var_p, char *out, int max_len)
{
  sprintf(out, "UDS:%d", uds_to_int(var_p));
}

/**
 * Print the contents of the structure
 */
void uds_write(const var_p_t var_p, int method, int handle)
{
  pv_write("(", method, handle);
  uds_field_s *next = var_p->v.uds;
  while (next) {
    pv_writevar(next->var, method, handle);
    next = next->next;
    if (next) {
      pv_write(",", method, handle);
    }
  }
  pv_write(")", method, handle);
}

// End of $Id$
