// This file is part of SmallBASIC
//
// bc module. Bytecode manipulation routines (bytecode segments API)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2001 Nicholas Christopoulos

#include "common/bc.h"
#include "common/smbas.h"
#include "common/str.h"

/*
 * string escape codes
 */
const char escapes[][2] = {
  {'a', 0x07},
  {'b', 0x08},
  {'e', 0x1b},
  {'f', 0x0c},
  {'n', 0x0a},
  {'r', 0x0d},
  {'t', 0x09},
  {'v', 0x0b},
};

/*
 * whether the character is an escape
 */
int bc_is_escape(char c, char *value) {
  int len = sizeof(escapes) / sizeof(escapes[0]);
  int result = 0;
  for (int i = 0; i < len && !result; i++) {
    if (escapes[i][0] == c) {
      *value = escapes[i][1];
      result = 1;
    }
  }
  return result;
}

/*
 * whether the character is octal escape
 */
int bc_is_octal(char *str, char *output) {
  char *next = str + 1;
  int result = 0;
  int digits = 0;
  int value = 0;

  while (isdigit(*next) && digits < 4) {
    value = (value << 3) + (*next - '0');
    digits++;
    next++;
  }
  if (digits == 3 && value < 256) {
    *output = value;
    result = 1;
  }
  return result;
}

/*
 * Create a bytecode segment
 */
void bc_create(bc_t *bc) {
  bc->ptr = malloc(BC_ALLOC_INCR);
  bc->size = BC_ALLOC_INCR;
  bc->count = 0;
  bc->cp = 0;
  bc->eoc_position = 0;
  bc->line_position = 0;
}

/*
 * Destroy a bytecode segment
 */
void bc_destroy(bc_t *bc) {
  free(bc->ptr);
  bc->ptr = NULL;
  bc->size = 0;
  bc->count = 0;
  bc->cp = 0;
  bc->eoc_position = 0;
  bc->line_position = 0;
}

/*
 * Resize a bytecode segment
 */
void bc_resize(bc_t *bc, uint32_t new_size) {
  if (new_size != bc->size) {
    bc->ptr = realloc(bc->ptr, new_size);
    bc->size = new_size;
    if (new_size == 1) {
      bc->count = 0;
    }
  }
}

/*
 * add one command
 */
void bc_add1(bc_t *bc, char code) {
  if (bc->count + sizeof(byte) >= bc->size) {
    bc_resize(bc, bc->size + BC_ALLOC_INCR);
  }
  bc->ptr[bc->count] = code;
  bc->count++;
}

/*
 * add one uint32_t
 */
void bc_add_dword(bc_t *bc, uint32_t p1) {
  if (bc->count + sizeof(uint32_t) >= bc->size) {
    bc_resize(bc, bc->size + BC_ALLOC_INCR);
  }
  memcpy(bc->ptr + bc->count, &p1, sizeof(uint32_t));
  bc->count += sizeof(uint32_t);
}

/*
 * add one command and one long int (32 bits)
 */
void bc_add2l(bc_t *bc, byte code, bid_t p1) {
  if (bc->count + sizeof(bid_t) >= bc->size) {
    bc_resize(bc, bc->size + BC_ALLOC_INCR);
  }
  bc->ptr[bc->count] = code;
  bc->count++;
  memcpy(bc->ptr + bc->count, &p1, sizeof(bid_t));
  bc->count += sizeof(bid_t);
}

/*
 * add buildin function call
 */
void bc_add_fcode(bc_t *bc, bid_t idx) {
  bc_add2l(bc, kwTYPE_CALLF, idx);
}

/*
 * add buildin procedure call
 */
void bc_add_pcode(bc_t *bc, bid_t idx) {
  bc_add2l(bc, kwTYPE_CALLP, idx);
}

/*
 * add an external function-call
 */
void bc_add_extfcode(bc_t *bc, uint32_t lib, uint32_t idx) {
  bc_add_code(bc, kwTYPE_CALLEXTF);
  bc_add_dword(bc, lib);
  bc_add_dword(bc, idx);
}

/*
 * add an external procedure-call
 */
void bc_add_extpcode(bc_t *bc, uint32_t lib, uint32_t idx) {
  bc_add_code(bc, kwTYPE_CALLEXTP);
  bc_add_dword(bc, lib);
  bc_add_dword(bc, idx);
}

/*
 * add an address
 */
void bc_add_addr(bc_t *bc, bcip_t idx) {
  if (bc->count + sizeof(bcip_t) >= bc->size) {
    bc_resize(bc, bc->size + BC_ALLOC_INCR);
  }
  memcpy(bc->ptr + bc->count, &idx, sizeof(bcip_t));
  bc->count += sizeof(bcip_t);
}

/*
 * add a control code
 *
 * Control codes are followed by 2 bcip_t elements (jump on true, jump on false)
 */
void bc_add_ctrl(bc_t *bc, code_t code, bcip_t true_ip, bcip_t false_ip) {
  bc_add1(bc, code);
  bc_add_addr(bc, true_ip);
  bc_add_addr(bc, false_ip);
}

/*
 * add an integer
 */
void bc_add_cint(bc_t *bc, var_int_t v) {
  if (bc->count + sizeof(var_int_t) >= bc->size) {
    bc_resize(bc, bc->size + BC_ALLOC_INCR);
  }
  bc->ptr[bc->count] = kwTYPE_INT;
  bc->count++;
  memcpy(bc->ptr + bc->count, &v, sizeof(var_int_t));
  bc->count += sizeof(var_int_t);
}

/*
 * add an real
 */
void bc_add_creal(bc_t *bc, var_num_t v) {
  if (bc->count + sizeof(var_num_t) >= bc->size) {
    bc_resize(bc, bc->size + BC_ALLOC_INCR);
  }
  bc->ptr[bc->count] = kwTYPE_NUM;
  bc->count++;
  memcpy(bc->ptr + bc->count, &v, sizeof(var_num_t));
  bc->count += sizeof(var_num_t);
}

/*
 * add one command and one string (see: bc_store_string)
 */
void bc_add_strn(bc_t *bc, const char *str, int len) {
  bc_add_code(bc, kwTYPE_STR);
  bc_add_dword(bc, len + 1);
  if (bc->count + len >= bc->size) {
    bc_resize(bc, bc->size + BC_ALLOC_INCR + len);
  }
  memcpy(bc->ptr + bc->count, str, len);
  bc->count += len;
  bc_add1(bc, 0);
}

/*
 * adds a string.
 * returns a pointer of src to the next "element"
 */
char *bc_store_string(bc_t *bc, char *src) {
  // skip past opening quotes
  char *p = src + 1;
  char *base = src + 1;
  char escape = 0;
  cstr cs;
  cstr_init(&cs, 5);

  while (*p) {
    if ((*p == '\\' && ((*(p + 1) == '\"') || *(p + 1) == '\\'))
        || *p == V_JOIN_LINE) {
      // escaped quote " or escaped escape
      cstr_append_i(&cs, base, p - base);

      if (*p == V_JOIN_LINE) {
        // skip null newline
        comp_line++;
        if (*(p+1) == '\"') {
          base = ++p;
          continue;
        }
      }

      // include " (or \ ) in next segment
      base = ++p;
    } else if (*p == '\\' && bc_is_escape(*(p + 1), &escape)) {
      char code[] = {escape, '\0'};
      cstr_append_i(&cs, base, p - base);
      cstr_append_i(&cs, code, 1);
      // skip single escape character
      base = (++p) + 1;
    } else if (*p == '\\' && bc_is_octal(p, &escape)) {
      char code[] = {escape, '\0'};
      cstr_append_i(&cs, base, p - base);
      cstr_append_i(&cs, code, 1);
      // skip octal digits
      p += 3;
      base = p + 1;
    } else if (*p == V_QUOTE) {
      // revert hidden quote
      *p = '\"';
    } else if (*p == V_LINE) {
      // revert hidden newline
      comp_line++;
      *p = '\n';
    } else if (*p == '\"') {
      // end of string detected
      cstr_append_i(&cs, base, p - base);
      bc_add_strn(bc, cs.buf, cs.length);
      p++;
      break;
    }
    p++;
  }
  free(cs.buf);
  return p;
}

/*
 * adds an EOC mark at the current position
 */
void bc_eoc(bc_t *bc) {
  if (bc && bc->count &&
      (bc->eoc_position == 0 || bc->eoc_position != bc->count - 1)) {
    // avoid appending multiple kwTYPE_EOCs (or kwTYPE_LINE)
    bc->eoc_position = bc->count;
    bc_add1(bc, kwTYPE_EOC);
  }
}

/*
 * pops any EOC mark at the current position
 */
int bc_pop_eoc(bc_t *bc) {
  int result;
  if (bc->eoc_position > 0 && bc->eoc_position == bc->count - 1) {
    bc->eoc_position = 0;
    result = (bc->ptr[--bc->count] == kwTYPE_EOC);
  } else {
    result = 0;
  }
  return result;
}

/*
 * appends the src to dst
 */
void bc_append(bc_t *dst, bc_t *src) {
  bc_resize(dst, dst->count + src->count + 4);
  memcpy(dst->ptr + dst->count, src->ptr, src->count);
  dst->count += src->count;
}

/*
 * appends n bytes from src to dst
 */
void bc_add_n(bc_t *dst, byte *src, uint32_t n) {
  if (dst->count + n >= dst->size) {
    bc_resize(dst, dst->size + BC_ALLOC_INCR + n);
  }
  memcpy(dst->ptr + dst->count, src, n);
  dst->count += n;
}
