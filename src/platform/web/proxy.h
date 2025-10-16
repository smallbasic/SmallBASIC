// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

#include <microhttpd.h>
#include <string>

//
// called during startup
//
void proxy_init(const char *path, const char *host);

//
// called during shutdown
//
void proxy_cleanup();

//
// whether to proxy this request
//
bool proxy_accept(MHD_Connection *connection, const char *path);

//
// Proxy the request to another backend service
//
MHD_Response *proxy_request(MHD_Connection *connection, const char *path, const char *method, std::string &body);

