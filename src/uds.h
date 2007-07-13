// -*- c-file-style: "java" -*-
// $Id: uds.h,v 1.1 2007-07-13 23:06:43 zeeb90au Exp $
// This file is part of SmallBASIC
//
// user-defined structures
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2007 Chris Warren-Smith. [http://tinyurl.com/ja2ss]


#include "var.h"

#ifndef UDS_H
#define UDS_H

int uds_is_empty(var_p_t var_p);
int uds_to_int(var_p_t var_p);
void uds_clear(const var_p_t var);
void uds_free(var_p_t var_p);
var_p_t uds_resolve_fields(var_p_t var_p);
void uds_set(var_p_t dest, const var_p_t src);
void uds_to_str(var_p_t var_p, const char* out, int max_len);
void uds_write(var_p_t var_p, int method, int handle);

#endif

// End of $Id: uds.h,v 1.1 2007-07-13 23:06:43 zeeb90au Exp $

