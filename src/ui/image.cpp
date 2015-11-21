// This file is part of SmallBASIC
//
// Image handling
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2002-2015 Chris Warren-Smith.

#include "common/sys.h"
#include "common/messages.h"
#include "common/pproc.h"
#include "common/sberr.h"
#include "common/device.h"
#include "common/fs_socket_client.h"
#include "common/var.h"
#include "common/var_map.h"
#include "lib/maapi.h"
#include "ui/system.h"

#if !defined(LODEPNG_NO_COMPILE_CPP)
  #define LODEPNG_NO_COMPILE_CPP
#endif
extern "C" {
  #include "lib/lodepng.h"
}

#define IMG_X "x"
#define IMG_Y "y"
#define IMG_OFFSET_TOP  "offsetTop"
#define IMG_OFFSET_LEFT "offsetLeft"
#define IMG_WIDTH "width"
#define IMG_HEIGHT "height"
#define IMG_ZINDEX "zIndex"
#define IMG_OPACITY "opacity"
#define IMG_ID "ID"
#define IMG_BID "BID"
#define IMG_HANDLE "HANDLE"

extern System *g_system;
unsigned nextId = 0;
strlib::List<ImageBuffer *> cache;

ImageBuffer::ImageBuffer() :
  _bid(0),
  _filename(NULL),
  _image(NULL),
  _width(0),
  _height(0) {
}

ImageBuffer::ImageBuffer(ImageBuffer &o) :
  _bid(o._bid),
  _filename(o._filename),
  _image(o._image),
  _width(o._width),
  _height(o._height) {
}

ImageBuffer::~ImageBuffer() {
  free(_filename);
  free(_image);
  _filename = NULL;
  _image = NULL;
}

ImageDisplay::ImageDisplay() : Shape(0, 0, 0, 0),
  _offsetLeft(0),
  _offsetTop(0),
  _zIndex(0),
  _opacity(0),
  _id(0),
  _bid(0),
  _buffer(NULL) {
}

ImageDisplay::ImageDisplay(ImageDisplay &o) : Shape(o._x, o._y, o._width, o._height) {
  copyImage(o);
}

void ImageDisplay::copyImage(ImageDisplay &o) {
  _x = o._x;
  _y = o._y;
  _offsetLeft = o._offsetLeft;
  _offsetTop = o._offsetTop;
  _width =o._width;
  _height = o._height;
  _zIndex = o._zIndex;
  _opacity = o._opacity;
  _id = o._id;
  _bid = o._bid;
  _buffer = o._buffer;
}

void ImageDisplay::draw(int x, int y, int w, int h, int cw) {
  MAPoint2d dstPoint;
  MARect srcRect;

  dstPoint.x = x;
  dstPoint.y = y;
  srcRect.left = MIN(_buffer->_width, _offsetLeft);
  srcRect.top = MIN(_buffer->_height, _offsetTop);
  srcRect.width = MIN(w, MIN(_buffer->_width, _width));
  srcRect.height = MIN(h, MIN(_buffer->_height, _height));

  maDrawRGB(&dstPoint, _buffer->_image, &srcRect, _opacity, _buffer->_width);
}

// share image buffer from another image variable
ImageBuffer *load_image(var_t *map) {
  ImageBuffer *result = NULL;
  int bid = map->type == V_MAP ? map_get_int(map, IMG_BID, -1) : -1;
  if (bid != -1) {
    List_each(ImageBuffer *, it, cache) {
      ImageBuffer *next = (*it);
      if (next->_bid == (unsigned)bid) {
        result = next;
        break;
      }
    }
  }
  return result;
}

ImageBuffer *load_image(dev_file_t *filep) {
  ImageBuffer *result = NULL;
  List_each(ImageBuffer *, it, cache) {
    ImageBuffer *next = (*it);
    if (next->_filename != NULL && strcmp(next->_filename, filep->name) == 0) {
      result = next;
      break;
    }
  }

  if (result == NULL) {
    unsigned w, h;
    unsigned char *image;
    unsigned error = 0;
    var_t *var_p;

    switch (filep->type) {
    case ft_http_client:
      // open "http://localhost/image1.gif" as #1
      var_p = v_new();
      http_read(filep, var_p);
      error = lodepng_decode32(&image, &w, &h, (unsigned char *)var_p->v.p.ptr,
                               var_p->v.p.size);
      v_free(var_p);
      free(var_p);
      break;
    case ft_stream:
      error = lodepng_decode32_file(&image, &w, &h, filep->name);
      break;
    default:
      error = 1;
      break;
    }
    if (!error) {
      result = new ImageBuffer();
      result->_bid = ++nextId;
      result->_width = w;
      result->_height = h;
      result->_filename = strdup(filep->name);
      result->_image = image;
      cache.add(result);
    } else {
      err_throw(ERR_IMAGE_LOAD, lodepng_error_text(error));
    }
  }
  return result;
}

ImageBuffer *load_xpm_image(char **data) {
  unsigned w, h;
  unsigned char *image;
  unsigned error = xpm_decode32(&image, &w, &h, data);
  ImageBuffer *result = NULL;
  if (!error) {
    result = new ImageBuffer();
    result->_bid = ++nextId;
    result->_width = w;
    result->_height = h;
    result->_filename = NULL;
    result->_image = image;
    cache.add(result);
  } else {
    err_throw(ERR_IMAGE_LOAD, "Invalid xpm image");
  }
  return result;
}

uint8_t *get_image_data(int x, int y, int w, int h) {
  MARect rc;
  rc.left = x;
  rc.top = y;
  rc.width = w;
  rc.height = h;
  int size = w * 4 * h * 4;
  uint8_t *result = (uint8_t *)malloc(size);
  if (result != NULL) {
    g_system->getOutput()->flushNow();
    maGetImageData(HANDLE_SCREEN, result, &rc, w);
  }
  return result;
}

void cmd_image_show(var_s *self) {
  ImageDisplay image;
  image._bid = map_get_int(self, IMG_BID, -1);

  List_each(ImageBuffer *, it, cache) {
    ImageBuffer *next = (*it);
    if (next->_bid == image._bid) {
      image._buffer = next;
      break;
    }
  }

  var_int_t x, y, z, op;
  int count = par_massget("iiii", &x, &y, &z, &op);

  if (prog_error || image._buffer == NULL || count == 1 || count > 4) {
    err_throw(ERR_PARAM);
  } else {
    // 0, 2, 3, 4 arguments accepted
    if (count >= 2) {
      image._x = x;
      image._y = y;
      map_set_int(self, IMG_X, x);
      map_set_int(self, IMG_Y, y);
    } else {
      image._x = map_get_int(self, IMG_X, -1);
      image._y = map_get_int(self, IMG_Y, -1);
    }
    if (count >= 3) {
      image._zIndex = z;
      map_set_int(self, IMG_ZINDEX, z);
    } else {
      image._zIndex = map_get_int(self, IMG_ZINDEX, -1);
    }
    if (count == 4) {
      image._opacity = op;
      map_set_int(self, IMG_OPACITY, op);
    } else {
      image._opacity = map_get_int(self, IMG_OPACITY, -1);
    }

    image._offsetLeft = map_get_int(self, IMG_OFFSET_LEFT, -1);
    image._offsetTop = map_get_int(self, IMG_OFFSET_TOP, -1);
    image._width = map_get_int(self, IMG_WIDTH, -1);
    image._height = map_get_int(self, IMG_HEIGHT, -1);
    image._id = map_get_int(self, IMG_ID, -1);
    g_system->getOutput()->addImage(image);
  }
}

void cmd_image_hide(var_s *self) {
  int id = map_get_int(self, IMG_ID, -1);
  g_system->getOutput()->removeImage(id);
}

void cmd_image_save(var_s *self) {
  var_int_t x, y, w, h;
  int count = par_massget("iiii", &x, &y, &w, &h);
  int width = g_system->getOutput()->getWidth();
  int height = g_system->getOutput()->getHeight();

  if (count == 0 && !prog_error) {
    // save entire screen
    x = 0;
    y = 0;
    w = width;
    h = height;
  } else if (count == 2) {
    // save width + height at 0, 0
    w = MIN(x, width);
    h = MIN(y, height);
    x = 0;
    y = 0;
  } else if (count == 4) {
    w = MIN(w, width);
    h = MIN(h, height);
  } else {
    err_throw(ERR_PARAM);
  }

  bool saved = false;
  int handle = map_get_int((var_p_t)self, IMG_HANDLE, -1);
  if (!prog_error && handle != -1) {
    dev_file_t *filep = dev_getfileptr(handle);
    if (filep != NULL) {
      uint8_t* image = get_image_data(x, y, w, h);
      if (image != NULL) {
        if (!lodepng_encode32_file(filep->name, image, w, h)) {
          saved = true;
        }
        free(image);
      }
    }
  }
  if (!saved) {
    err_throw(ERR_IMAGE_SAVE);
  }
}

void create_image(var_p_t var, ImageBuffer *image) {
  map_init(var);
  map_add_var(var, IMG_X, 0);
  map_add_var(var, IMG_Y, 0);
  map_add_var(var, IMG_OFFSET_TOP, 0);
  map_add_var(var, IMG_OFFSET_LEFT, 0);
  map_add_var(var, IMG_ZINDEX, 1);
  map_add_var(var, IMG_OPACITY, 0);
  map_add_var(var, IMG_ID, ++nextId);

  if (image != NULL) {
    map_add_var(var, IMG_WIDTH, image->_width);
    map_add_var(var, IMG_HEIGHT, image->_height);
    map_add_var(var, IMG_BID, image->_bid);

    var_p_t v_show = map_add_var(var, "show", 0);
    v_show->type = V_FUNC;
    v_show->v.fn.self = var;
    v_show->v.fn.cb = cmd_image_show;

    var_p_t v_hide = map_add_var(var, "hide", 0);
    v_hide->type = V_FUNC;
    v_hide->v.fn.self = var;
    v_hide->v.fn.cb = cmd_image_hide;
  } else {
    var_p_t v_save = map_add_var(var, "save", 0);
    v_save->type = V_FUNC;
    v_save->v.fn.self = var;
    v_save->v.fn.cb = cmd_image_save;
  }
}

// loads an image for the form image input type
ImageDisplay *create_display_image(var_p_t var, const char *name) {
  ImageDisplay *result = NULL;
  if (name != NULL && var != NULL) {
    dev_file_t file;
    strcpy(file.name, name);
    file.type = ft_stream;
    ImageBuffer *buffer = load_image(&file);
    if (buffer != NULL) {
      result = new ImageDisplay();
      result->_buffer = buffer;
      result->_bid = buffer->_bid;
      result->_width = buffer->_width;
      result->_height = buffer->_height;
      result->_zIndex = 0;
      result->_offsetLeft = map_get_int(var, IMG_OFFSET_LEFT, -1);
      result->_offsetTop = map_get_int(var, IMG_OFFSET_TOP, -1);
      result->_opacity = map_get_int(var, IMG_OPACITY, -1);

      if (result->_offsetLeft == -1) {
        result->_offsetLeft = 0;
      }
      if (result->_offsetTop == -1) {
        result->_offsetTop = 0;
      }
      if (result->_opacity == -1) {
        result->_opacity = 0;
      }
    }
  }
  return result;
}

void screen_dump() {
  int width = g_system->getOutput()->getWidth();
  int height = g_system->getOutput()->getHeight();
  uint8_t* image = get_image_data(0, 0, width, height);
  if (image != NULL) {
    const char *path = gsb_bas_dir;
#if defined(_ANDROID)
    path = "/sdcard/";
#endif
    for (int i = 0; i < 1000; i++) {
      char file[OS_PATHNAME_SIZE];
      if (strstr(path, "://") != NULL) {
        sprintf(file, "sbasic_dump_%d.png", i);
      } else {
        sprintf(file, "%ssbasic_dump_%d.png", path, i);
      }
      if (access(file, R_OK) != 0) {
        g_system->systemPrint("Saving screen to %s\n", file);
        lodepng_encode32_file(file, image, width, height);
        break;
      }
    }
    free(image);
  }
}

extern "C" void v_create_image(var_p_t var) {
  var_t arg;
  int handle = -1;
  ImageBuffer *image = NULL;
  dev_file_t *filep = NULL;

  byte code = code_peek();
  switch (code) {
  case kwTYPE_SEP:
    code_skipnext();
    if (code_getnext() == '#') {
      handle = par_getint();
      if (!prog_error) {
        filep = dev_getfileptr(handle);
      }
    }
    break;

  case kwTYPE_LINE:
  case kwTYPE_EOC:
    break;

  default:
    v_init(&arg);
    eval(&arg);
    if (arg.type == V_STR && !prog_error) {
      dev_file_t file;
      strcpy(file.name, arg.v.p.ptr);
      file.type = ft_stream;
      image = load_image(&file);
    } else if (arg.type == V_ARRAY && !prog_error) {
      char **data = new char*[arg.v.a.size];
      for (int i = 0; i < arg.v.a.size; i++) {
        var_p_t elem = v_elem(&arg, i);
        data[i] = elem->v.p.ptr;
      }
      image = load_xpm_image(data);
      delete [] data;
    } else {
      image = load_image(&arg);
    }
    v_free(&arg);
    break;
  };

  if (image == NULL && filep != NULL) {
    if (filep->open_flags == DEV_FILE_OUTPUT) {
      create_image(var, NULL);
      map_add_var(var, IMG_HANDLE, handle);
    } else {
      handle = -1;
      image = load_image(filep);
    }
  }

  if (image != NULL) {
    create_image(var, image);
  } else if (handle == -1) {
    err_throw(ERR_BAD_FILE_HANDLE);
  }
}

