// This file is part of SmallBASIC
//
// Image handling
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2002-2018 Chris Warren-Smith.

#define LODEPNG_NO_COMPILE_CPP
#define PIXELFORMAT_RGBA8888
#define IMG_WIDTH "width"
#define IMG_HEIGHT "height"
#define IMG_NAME "name"
#define IMG_ID "ID"
#define IMG_BID "BID"

#include "config.h"
#include "common/sys.h"
#include "common/messages.h"
#include "common/pproc.h"
#include "common/fs_socket_client.h"
#include "ui/strlib.h"
#include "ui/rgb.h"
#include "lib/lodepng/lodepng.h"

extern "C" int xpm_decode32(uint8_t **image, unsigned *width, unsigned *height, const char *const *xpm);

struct ImageBuffer {
  ImageBuffer();
  ImageBuffer(ImageBuffer &imageBuffer);
  virtual ~ImageBuffer();

  unsigned _bid;
  char *_filename;
  uint8_t *_image;
  int _width;
  int _height;
};

unsigned nextId = 0;
strlib::List<ImageBuffer *> cache;

void reset_image_cache() {
  cache.removeAll();
}

ImageBuffer::ImageBuffer() :
  _bid(0),
  _filename(nullptr),
  _image(nullptr),
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
  _filename = nullptr;
  _image = nullptr;
}

dev_file_t *get_file() {
  dev_file_t *result = nullptr;
  code_skipnext();
  if (code_getnext() == '#') {
    int handle = par_getint();
    if (!prog_error) {
      result = dev_getfileptr(handle);
    }
  }
  return result;
}

ImageBuffer *load_image(int w) {
  ImageBuffer *result = nullptr;
  if (!par_getsep()) {
    err_throw(ERR_PARAM);
  } else {
    int h = par_getint();
    if (prog_error) {
      err_throw(ERR_PARAM);
    } else {
      int size = w * h * 4;
      uint8_t *image = (uint8_t *)calloc(size, 1);
      result = new ImageBuffer();
      result->_bid = ++nextId;
      result->_width = w;
      result->_height = h;
      result->_filename = nullptr;
      result->_image = image;
      cache.add(result);
    }
  }
  return result;
}

// share image buffer from another image variable or array
ImageBuffer *load_image(var_t *var) {
  ImageBuffer *result = nullptr;
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
    int h = ABS(v_ubound(var, 0) - v_lbound(var, 0)) + 1;
    int w = ABS(v_ubound(var, 1) - v_lbound(var, 1)) + 1;
    int size = w * h * 4;
    auto image = (uint8_t *)malloc(size);
    for (int y = 0; y < h; y++) {
      int yoffs = (4 * y * w);
      for (int x = 0; x < w; x++) {
        int pos = y * w + x;
        uint8_t a, r, g, b;
        v_get_argb(-v_getint(v_elem(var, pos)), a, r, g, b);
        SET_IMAGE_ARGB(image, yoffs + (x * 4), a, r, g, b);
      }
    }
    result = new ImageBuffer();
    result->_bid = ++nextId;
    result->_width = w;
    result->_height = h;
    result->_filename = nullptr;
    result->_image = image;
    cache.add(result);
  }
  return result;
}

ImageBuffer *load_image(const uint8_t* buffer, int32_t size) {
  ImageBuffer *result = nullptr;
  unsigned w, h;
  uint8_t *image;
  unsigned error = 0;

  error = lodepng_decode32(&image, &w, &h, buffer, size);
  if (!error) {
    result = new ImageBuffer();
    result->_bid = ++nextId;
    result->_width = w;
    result->_height = h;
    result->_filename = nullptr;
    result->_image = image;
    cache.add(result);
  } else {
    err_throw(ERR_IMAGE_LOAD, lodepng_error_text(error));
  }
  return result;
}

//
// png = image(#1)
//
ImageBuffer *load_image(dev_file_t *filep) {
  ImageBuffer *result = nullptr;
  List_each(ImageBuffer *, it, cache) {
    ImageBuffer *next = (*it);
    if (next->_filename != nullptr && strcmp(next->_filename, filep->name) == 0) {
      result = next;
      break;
    }
  }

  if (result == nullptr) {
    unsigned w, h;
    uint8_t *image;
    unsigned error = 0;
    unsigned network_error = 0;
    var_t *var_p;

    switch (filep->type) {
    case ft_http_client:
      // open "http://localhost/image1.gif" as #1
      if (filep->handle == -1) {
        network_error = 1;
      } else {
        var_p = v_new();
        http_read(filep, var_p);
        error = lodepng_decode32(&image, &w, &h, (uint8_t *)var_p->v.p.ptr, var_p->v.p.length);
        v_free(var_p);
        v_detach(var_p);
      }
      break;
    case ft_stream:
      error = lodepng_decode32_file(&image, &w, &h, filep->name);
      break;
    default:
      error = 1;
      break;
    }
    if (network_error) {
      err_throw(ERR_IMAGE_LOAD, ERR_NETWORK);
    } else if (error) {
      err_throw(ERR_IMAGE_LOAD, lodepng_error_text(error));
    } else {
      result = new ImageBuffer();
      result->_bid = ++nextId;
      result->_width = w;
      result->_height = h;
      result->_filename = strdup(filep->name);
      result->_image = image;
      cache.add(result);
    }
  }
  return result;
}

ImageBuffer *load_xpm_image(char **data) {
  unsigned w, h;
  uint8_t *image;
  unsigned error = xpm_decode32(&image, &w, &h, data);
  ImageBuffer *result = nullptr;
  if (!error) {
    result = new ImageBuffer();
    result->_bid = ++nextId;
    result->_width = w;
    result->_height = h;
    result->_filename = nullptr;
    result->_image = image;
    cache.add(result);
  } else {
    err_throw(ERR_IMAGE_LOAD, ERR_XPM_IMAGE);
  }
  return result;
}

//
// Reduces the size of the image
// arguments: left, top, right, bottom
//
// png.clip(10, 10, 10, 10)
//
void cmd_image_clip(var_s *self, var_s *) {
  var_int_t left, top, right, bottom;
  ImageBuffer *image = load_image(self);
  if (par_massget("iiii", &left, &top, &right, &bottom) == 4) {
    int w = image->_width - (right + left);
    int h = image->_height - (bottom + top);
    int size = w * h * 4;
    int oldSize = image->_width * image->_height * 4;
    if (size > oldSize) {
      err_throw(ERR_PARAM);
    } else if (size != oldSize) {
      uint8_t *dst = (uint8_t *)calloc(size, 1);
      uint8_t *src = image->_image;
      for (int y = 0; y < h; y++) {
        int syoffs = (4 * (y + top) * image->_width);
        int dyoffs = (4 * y * w);
        for (int x = 0; x < w; x++) {
          int spos = syoffs + (4 * (x + left));
          int dpos = dyoffs + (4 * x);
          dst[dpos + 0] = src[spos + 0];
          dst[dpos + 1] = src[spos + 1];
          dst[dpos + 2] = src[spos + 2];
          dst[dpos + 3] = src[spos + 3];
        }
      }
      free(image->_image);
      image->_image = dst;
      image->_width = w;
      image->_height = h;
      map_set_int(self, IMG_WIDTH, w);
      map_set_int(self, IMG_HEIGHT, h);
    }
  } else {
    err_throw(ERR_PARAM);
  }
}

//
// Calls the supplied callback function on each pixel
//
// func colorToAlpha(x)
//   return x
// end
// png.filter(use colorToAlpha(x))
//
void cmd_image_filter(var_s *self, var_s *) {
  ImageBuffer *image_buffer = load_image(self);
  if (code_peek() == kwUSE && image_buffer != nullptr) {
    code_skipnext();
    bcip_t use_ip = code_getaddr();
    bcip_t exit_ip = code_getaddr();
    int w = image_buffer->_width;
    int h = image_buffer->_height;
    auto image = image_buffer->_image;
    var_t var;
    v_init(&var);
    for (int y = 0; y < h; y++) {
      int yoffs = (4 * y * w);
      for (int x = 0; x < w; x++) {
        int offs = yoffs + (4 * x);
        uint8_t a, r, g, b;
        GET_IMAGE_ARGB(image, offs, a, r, g, b);
        pixel_t px = v_get_argb_px(a, r, g, b);
        v_setint(&var, px);
        exec_usefunc(&var, use_ip);
        v_get_argb(v_getint(&var), a, r, g, b);
        SET_IMAGE_ARGB(image, offs, a, r, g, b);
      }
    }
    code_jump(exit_ip);
  } else {
    err_throw(ERR_PARAM);
  }
}

//
// Paste the given image into this image at the given x, y location
//
// png2 = image(w, h)
// png2.paste(png1, 0, 0)
//
void cmd_image_paste(var_s *self, var_s *) {
  var_int_t x, y;
  var_t *var;
  ImageBuffer *image = load_image(self);
  int count = par_massget("Piiii", &var, &x, &y);
  if (image != nullptr && (count == 1 || count == 3)) {
    ImageBuffer *srcImage = load_image(var);
    if (srcImage == nullptr) {
      err_throw(ERR_PARAM);
    } else {
      if (count == 1) {
        x = 0;
        y = 0;
      }
      int dw = image->_width;
      int dh = image->_height;
      int sw = srcImage->_width;
      int sh = srcImage->_height;
      uint8_t *src = srcImage->_image;
      uint8_t *dst = image->_image;
      for (int sy = 0, dy = y; sy < sh && dy < dh; sy++, dy++) {
        int syoffs = (4 * sy * srcImage->_width);
        int dyoffs = (4 * dy * dw);
        for (int sx = 0, dx = x; sx < sw && dx < dw; sx++, dx++) {
          int spos = syoffs + (4 * sx);
          int dpos = dyoffs + (4 * dx);
          dst[dpos + 0] = src[spos + 0];
          dst[dpos + 1] = src[spos + 1];
          dst[dpos + 2] = src[spos + 2];
          dst[dpos + 3] = src[spos + 3];
        }
      }
    }
  } else {
    err_throw(ERR_PARAM);
  }
}

//
// Output the image to a PNG file
//
// png.save("horse1.png")
// png.save(#1)
//
void cmd_image_save(var_s *self, var_s *) {
  ImageBuffer *image = load_image(self);
  dev_file_t *filep = nullptr;
  byte code = code_peek();
  int error = -1;
  int w = image->_width;
  int h = image->_height;
  var_t var;

  if (!prog_error && image != nullptr) {
    switch (code) {
    case kwTYPE_SEP:
      filep = get_file();
      if (filep != nullptr && filep->open_flags == DEV_FILE_OUTPUT) {
        error = lodepng_encode32_file(filep->name, image->_image, w, h);
      }
      break;
    default:
      v_init(&var);
      eval(&var);
      if (var.type == V_STR && !prog_error) {
        error = lodepng_encode32_file(var.v.p.ptr, image->_image, w, h);
      }
      v_free(&var);
      break;
    }
  }
  if (error == -1) {
    err_throw(ERR_PARAM);
  } else if (error != 0) {
    err_throw(ERR_IMAGE_SAVE_ERR, lodepng_error_text(error));
  }
}

void create_image(var_p_t var, ImageBuffer *image) {
  map_init(var);
  map_add_var(var, IMG_ID, ++nextId);
  map_add_var(var, IMG_WIDTH, image->_width);
  map_add_var(var, IMG_HEIGHT, image->_height);
  map_add_var(var, IMG_BID, image->_bid);
  if (image->_filename != nullptr) {
    var_p_t value = map_add_var(var, IMG_NAME, 0);
    v_setstr(value, image->_filename);
  }
  v_create_func(var, "clip", cmd_image_clip);
  v_create_func(var, "filter", cmd_image_filter);
  v_create_func(var, "paste", cmd_image_paste);
  v_create_func(var, "save", cmd_image_save);
}

//
// Creates an image object
//
// png = image(#1)
// png = image("file.png")
// png = image(array)
// png = image(png)
// png = image(10, 10)
//
extern "C" void v_create_image(var_p_t var) {
  var_t arg;
  ImageBuffer *image = nullptr;
  dev_file_t *filep = nullptr;

  byte code = code_peek();
  switch (code) {
  case kwTYPE_SEP:
    filep = get_file();
    if (filep != nullptr) {
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
        uint8_t *data = new uint8_t[v_asize(&arg)];
        for (unsigned i = 0; i < v_asize(&arg); i++) {
          var_p_t elem = v_elem(&arg, i);
          data[i] = (uint8_t)elem->v.i;
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

  if (image != nullptr) {
    create_image(var, image);
  } else {
    err_throw(ERR_BAD_FILE_HANDLE);
  }
}
