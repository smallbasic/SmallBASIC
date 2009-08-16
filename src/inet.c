// $Id$
// This file is part of SmallBASIC
//
// Network library (byte-stream sockets)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "sys.h"
#include "inet.h"
#include "device.h"

#if defined(_UnixOS)
#include "unix/inet.c"
#elif defined(_PalmOS)
#include "palmos/inet.c"
#endif

/* End of "$Id$". */
