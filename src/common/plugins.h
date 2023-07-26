// This file is part of SmallBASIC
//
// SmallBASIC - External library support (plugins)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2001 Nicholas Christopoulos

#if !defined(SB_PLUGINS)
#define SB_PLUGINS

#include "common/sys.h"
#include "common/var.h"
#include "common/device.h"
#include "include/module.h"

#if defined(__cplusplus)
extern "C" {
#endif

//
// initialise the plugin system
//
void plugin_init();

//
// locates the plugin, imports the keywords and returns the associated ID
//
int plugin_import(const char *name, const char *alias);

//
// opens the plugin ready for execution. reuses any existing compiler data
// when not invoked from an sbx. otherwise creates the name:id association.
//
void plugin_open(const char *name, int lib_id);

//
// returns the keyword ID
//
int plugin_get_kid(int lib_id, const char *keyword);

//
// returns the function pointer for the given function name
//
void *plugin_get_func(const char *name);
  
//
// executes the plugin procedure at the given index
//
int plugin_procexec(int lib_id, int index);

//
// executes the plugin function at the given index
//
int plugin_funcexec(int lib_id, int index, var_t *ret);

//
// cleanup any resources held against the map data
//
void plugin_free(int lib_id, int cls_id, int id);

//
// closes the plugin system
//
void plugin_close();

#if defined(__cplusplus)
}
#endif
#endif
