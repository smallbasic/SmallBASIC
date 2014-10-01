// This file is part of SmallBASIC
//
// System objects
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2007-2014 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

#include "common/sys.h"
#include "common/messages.h"
#include "common/sberr.h"
#include "common/device.h"
#include "common/var.h"
#include "common/var_map.h"

#define IMG_X "x"
#define IMG_Y "y"
#define IMG_OFFSET_TOP  "offsetTop"
#define IMG_OFFSET_LEFT "offsetLeft"
#define IMG_WIDTH "width"
#define IMG_HEIGHT "height"
#define IMG_ZINDEX "zIndex"
#define IMG_OPACITY "opacity"
#define IMG_ID "id"
#define IMG_HANDLE "handle"

int next_id = 0;

var_image var_get_image(var_p_t self) {
  var_image result;
  result.x = map_get_int(self, IMG_X);
  result.y = map_get_int(self, IMG_Y);
  result.offsetTop = map_get_int(self, IMG_OFFSET_TOP);
  result.offsetLeft = map_get_int(self, IMG_OFFSET_LEFT);
  result.width = map_get_int(self, IMG_WIDTH);
  result.height = map_get_int(self, IMG_HEIGHT);
  result.zIndex = map_get_int(self, IMG_ZINDEX);
  result.opacity = map_get_int(self, IMG_OPACITY);
  result.id = map_get_int(self, IMG_ID);
  result.handle = map_get_int(self, IMG_HANDLE);
  return result;
}

void cmd_image_show(const void *arg) {
  var_image image = var_get_image((var_p_t)arg);
  dev_image_show(&image);
}

void cmd_image_hide(const void *arg) {
  int handle = map_get_int((var_p_t)arg, IMG_ID);
  dev_image_hide(handle);
}

void var_create_image(var_p_t var, int handle) {
  int loaded = dev_image_load(handle);
  if (!loaded) {
    err_throw(ERR_IMAGE_LOAD);
  } else {
    int w = dev_image_width(handle);
    int h = dev_image_height(handle);
    var->type = V_MAP;
    map_add_var(var, IMG_X, 0);
    map_add_var(var, IMG_Y, 0);
    map_add_var(var, IMG_OFFSET_TOP, 0);
    map_add_var(var, IMG_OFFSET_LEFT, 0);
    map_add_var(var, IMG_WIDTH, w);
    map_add_var(var, IMG_HEIGHT, h);
    map_add_var(var, IMG_ZINDEX, 1);
    map_add_var(var, IMG_OPACITY, 1);
    map_add_var(var, IMG_HANDLE, handle);
    map_add_var(var, IMG_ID, ++next_id);
    var_p_t v_show = map_add_var(var, "show", 0);
    v_show->type = V_FUNC;
    v_show->v.fn.self = var;
    v_show->v.fn.cb = cmd_image_show;

    var_p_t v_hide = map_add_var(var, "hide", 0);
    v_hide->type = V_FUNC;
    v_hide->v.fn.self = var;
    v_hide->v.fn.cb = cmd_image_hide;
  }
}
