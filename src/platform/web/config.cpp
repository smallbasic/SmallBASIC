// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <config.h>

#if defined(USE_LIB_CURL)
#include "proxy.cpp"
#else
#include <microhttpd.h>
void proxy_init(const char *path, const char *host) {}
void proxy_cleanup() {}
bool proxy_accept(MHD_Connection *connection, const char *path) { return false; }
MHD_Response *proxy_request(MHD_Connection *connection, const char *path, const char *method, const char *body) { return nullptr; }
#endif
