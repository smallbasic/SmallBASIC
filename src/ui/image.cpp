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
#include "ui/image.h"
#include "ui/system.h"
#include "ui/graphics.h"

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

extern System *g_system;
unsigned nextId = 0;
strlib::List<ImageBuffer *> cache;

void reset_image_cache() {
  cache.removeAll();
}

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

dev_file_t *eval_filep() {
  dev_file_t *result = NULL;
  code_skipnext();
  if (code_getnext() == '#') {
    int handle = par_getint();
    if (!prog_error) {
      result = dev_getfileptr(handle);
    }
  }
  return result;
}

uint8_t *get_image_data(int x, int y, int w, int h) {
  MARect rc;
  rc.left = x;
  rc.top = y;
  rc.width = w;
  rc.height = h;
  int size = w * h * 4;
  uint8_t *result = (uint8_t *)malloc(size);
  if (result != NULL) {
    g_system->getOutput()->redraw();
    maGetImageData(HANDLE_SCREEN, result, &rc, w);
  }
  return result;
}

ImageBuffer *load_image(var_int_t x) {
  var_int_t y, w, h;
  int count = par_massget("iii", &y, &w, &h);
  int width = g_system->getOutput()->getWidth();
  int height = g_system->getOutput()->getHeight();
  ImageBuffer *result = NULL;

  if (prog_error || count == 0 || count == 2) {
    err_throw(ERR_PARAM);
  } else {
    if (count == 1) {
      w = width;
      h = height;
    } else {
      w = MIN(w, width);
      h = MIN(h, height);
    }
    uint8_t* image = get_image_data(x, y, w, h);
    if (image == NULL) {
      err_throw(ERR_IMAGE_LOAD, "Failed to load screen image");
    } else {
      result = new ImageBuffer();
      result->_bid = ++nextId;
      result->_width = w;
      result->_height = h;
      result->_filename = NULL;
      result->_image = image;
      cache.add(result);
    }
  }
  return result;
}

// share image buffer from another image variable
ImageBuffer *load_image(var_t *var) {
  ImageBuffer *result = NULL;
  if (var->type == V_MAP) {
    int bid = map_get_int(var, IMG_BID, -1);
    if (bid != -1) {
      List_each(ImageBuffer *, it, cache) {
        ImageBuffer *next = (*it);
        if (next->_bid == (unsigned)bid) {
          result = next;
          break;
        }
      }
    }
  } else if (var->type == V_ARRAY && v_maxdim(var) == 2) {
    int w = ABS(v_lbound(var, 0) - v_ubound(var, 0)) + 1;
    int h = ABS(v_lbound(var, 1) - v_ubound(var, 1)) + 1;
    int size = w * h * 4;
    unsigned char *image = (unsigned char *)malloc(size);
    for (int y = 0; y < h; y++) {
      int yoffs = (4 * y * w);
      for (int x = 0; x < w; x++) {
        int pos = y * w + x;
        var_t *elem = v_elem(var, pos);
        pixel_t px = -v_getint(elem);
        uint8_t r, g, b;
        GET_RGB2(px, r, g, b);
        int offs = yoffs + (4 * x);
        image[offs + 0] = r;
        image[offs + 1] = g;
        image[offs + 2] = b;
        image[offs + 3] = 255;
      }
    }
    result = new ImageBuffer();
    result->_bid = ++nextId;
    result->_width = w;
    result->_height = h;
    result->_filename = NULL;
    result->_image = image;
    cache.add(result);
  }
  return result;
}

ImageBuffer *load_image(const unsigned char* buffer, int32_t size) {
  ImageBuffer *result = NULL;
  unsigned w, h;
  unsigned char *image;
  unsigned error = 0;

  error = lodepng_decode32(&image, &w, &h, buffer, size);
  if (!error) {
    result = new ImageBuffer();
    result->_bid = ++nextId;
    result->_width = w;
    result->_height = h;
    result->_filename = NULL;
    result->_image = image;
    cache.add(result);
  } else {
    err_throw(ERR_IMAGE_LOAD, lodepng_error_text(error));
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
                               var_p->v.p.length);
      v_free(var_p);
      v_detach(var_p);
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
  unsigned id = map_get_int(self, IMG_BID, -1);
  ImageBuffer *image = NULL;
  List_each(ImageBuffer *, it, cache) {
    ImageBuffer *next = (*it);
    if (next->_bid == id) {
      image = next;
      break;
    }
  }

  var_t *array = NULL;
  dev_file_t *filep = NULL;
  byte code = code_peek();
  switch (code) {
  case kwTYPE_SEP:
    filep = eval_filep();
    break;
  default:
    array = par_getvar_ptr();
    break;
  }

  bool saved = false;
  if (!prog_error && image != NULL) {
    int w = image->_width;
    int h = image->_height;
    if (filep != NULL && filep->open_flags == DEV_FILE_OUTPUT) {
      if (!lodepng_encode32_file(filep->name, image->_image, w, h)) {
        saved = true;
      }
    } else if (array != NULL) {
      v_tomatrix(array, h, w);
      for (int y = 0; y < h; y++) {
        int yoffs = (4 * y * w);
        for (int x = 0; x < w; x++) {
          int offs = yoffs + (4 * x);
          uint8_t r = image->_image[offs + 0];
          uint8_t g = image->_image[offs + 1];
          uint8_t b = image->_image[offs + 2];
          pixel_t px = SET_RGB(r, g, b);
          int pos = y * w + x;
          var_t *elem = v_elem(array, pos);
          v_setint(elem, -px);
        }
      }
      saved = true;
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
  map_add_var(var, IMG_WIDTH, image->_width);
  map_add_var(var, IMG_HEIGHT, image->_height);
  map_add_var(var, IMG_BID, image->_bid);
  create_func(var, "show", cmd_image_show);
  create_func(var, "hide", cmd_image_hide);
  create_func(var, "save", cmd_image_save);
}

// loads an image for the form image input type
ImageDisplay *create_display_image(var_p_t var, const char *name) {
  ImageDisplay *result = NULL;
  if (name != NULL && var != NULL) {
    dev_file_t file;
    strlcpy(file.name, name, sizeof(file.name));
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
    } else {
      err_throw(ERR_IMAGE_LOAD, name);
    }
  } else {
    err_throw(ERR_IMAGE_LOAD, "name field empty");
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
  ImageBuffer *image = NULL;
  dev_file_t *filep = NULL;

  byte code = code_peek();
  switch (code) {
  case kwTYPE_SEP:
    filep = eval_filep();
    if (filep != NULL) {
      image = load_image(filep);
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
      strlcpy(file.name, arg.v.p.ptr, sizeof(file.name));
      file.type = ft_stream;
      image = load_image(&file);
    } else if (arg.type == V_ARRAY && v_asize(&arg) > 0 && !prog_error) {
      var_p_t elem0 = v_elem(&arg, 0);
      if (elem0->type == V_STR) {
        char **data = new char*[v_asize(&arg)];
        for (unsigned i = 0; i < v_asize(&arg); i++) {
          var_p_t elem = v_elem(&arg, i);
          data[i] = elem->v.p.ptr;
        }
        image = load_xpm_image(data);
        delete [] data;
      } else if (v_maxdim(&arg) == 2) {
        // load from 2d array
        image = load_image(&arg);
      } else if (elem0->type == V_INT) {
        unsigned char *data = new unsigned char[v_asize(&arg)];
        for (unsigned i = 0; i < v_asize(&arg); i++) {
          var_p_t elem = v_elem(&arg, i);
          data[i] = (unsigned char)elem->v.i;
        }
        image = load_image(data, v_asize(&arg));
        delete [] data;
      }
    } else if (arg.type == V_INT && !prog_error) {
      image = load_image(arg.v.i);
    } else {
      image = load_image(&arg);
    }
    v_free(&arg);
    break;
  };

  if (image != NULL) {
    create_image(var, image);
  } else {
    err_throw(ERR_BAD_FILE_HANDLE);
  }
}
