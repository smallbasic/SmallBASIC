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
#define IMG_ID "ID"
#define IMG_BID "BID"

#include "config.h"
#include "common/sys.h"
#include "common/messages.h"
#include "common/pproc.h"
#include "common/fs_socket_client.h"
#include "ui/strlib.h"
#include "ui/utils.h"
#include "ui/rgb.h"

extern "C" {
#include "lib/lodepng.h"
  int xpm_decode32(uint8_t **image, unsigned *width, unsigned *height, const char *const *xpm);
}

struct ImageBuffer {
  ImageBuffer();
  ImageBuffer(ImageBuffer &imageBuffer);
  virtual ~ImageBuffer();

  unsigned _bid;
  char *_filename;
  unsigned char *_image;
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

void create_func(var_p_t map, const char *name, method cb) {
  var_p_t v_func = map_add_var(map, name, 0);
  v_func->type = V_FUNC;
  v_func->v.fn.cb = cb;
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

ImageBuffer *load_image(int w) {
  ImageBuffer *result = NULL;
  if (!par_getsep()) {
    err_throw(ERR_PARAM);
  } else {
    int h = par_getint();
    if (prog_error) {
      err_throw(ERR_PARAM);
    } else {
      int size = w * h * 4;
      unsigned char *image = (unsigned char *)calloc(size, 1);
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

// share image buffer from another image variable or array
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
        pixel_t px = v_getint(elem);
        uint8_t r, g, b, a;
        GET_ARGB(px, a, r, g, b);
        int offs = yoffs + (4 * x);
        image[offs + 0] = r;
        image[offs + 1] = g;
        image[offs + 2] = b;
        image[offs + 3] = a;
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

//
// png = image(#1)
//
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
        error = lodepng_decode32(&image, &w, &h, (unsigned char *)var_p->v.p.ptr,
                                 var_p->v.p.length);
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
    err_throw(ERR_IMAGE_LOAD, ERR_XPM_IMAGE);
  }
  return result;
}

//
// png.save("horse1.png")
// png.save(#1)
//
void cmd_image_save(var_s *self) {
  ImageBuffer *image = load_image(self);
  dev_file_t *filep = NULL;
  byte code = code_peek();
  int error = -1;
  int w = image->_width;
  int h = image->_height;
  var_t var;

  if (!prog_error && image != NULL) {
    switch (code) {
    case kwTYPE_SEP:
      filep = eval_filep();
      if (filep != NULL && filep->open_flags == DEV_FILE_OUTPUT) {
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

//
// calls the supplied callback function on each pixel
//
// func colorToAlpha(x)
//   return x
// end
// png.filter(use colorToAlpha(x))
//
void cmd_image_filter(var_s *self) {
  ImageBuffer *image_buffer = load_image(self);
  if (code_peek() == kwUSE && image_buffer != NULL) {
    code_skipnext();
    bcip_t use_ip = code_getaddr();
    bcip_t exit_ip = code_getaddr();
    int w = image_buffer->_width;
    int h = image_buffer->_height;
    unsigned char *image = image_buffer->_image;
    var_t var;
    v_init(&var);
    for (int y = 0; y < h; y++) {
      int yoffs = (4 * y * w);
      for (int x = 0; x < w; x++) {
        int offs = yoffs + (4 * x);
        uint8_t r = image[offs + 0];
        uint8_t g = image[offs + 1];
        uint8_t b = image[offs + 2];
        uint8_t a = image[offs + 3];
        pixel_t px = SET_ARGB(a, r, g, b);
        v_setint(&var, px);
        exec_usefunc(&var, use_ip);
        px = v_getint(&var);
        GET_ARGB(px, a, r, g, b);
        image[offs + 0] = r;
        image[offs + 1] = g;
        image[offs + 2] = b;
        image[offs + 3] = a;
      }
    }
    code_jump(exit_ip);
  } else {
    err_throw(ERR_PARAM);
  }
}

//
// png2 = image(w, h)
// png2.paste(png1, 0, 0, w, h)
//
void cmd_image_paste(var_s *self) {
  var_int_t x, y, w, h;
  var_t *var;
  ImageBuffer *image = load_image(self);
  int count = par_massget("Piiii", &var, &x, &y, &w, &h);
  if (image != NULL && (count == 1 || count == 3 || count == 5)) {
    ImageBuffer *srcImage = load_image(var);
    if (srcImage == NULL) {
      err_throw(ERR_PARAM);
    } else {
      if (count == 1) {
        x = 0;
        y = 0;
      }
      if (count < 5) {
        w = image->_width;
        h = image->_height;
      }
      int dw = image->_width;
      int dh = image->_height;
      int sw = MIN(w, srcImage->_width);
      int sh = MIN(h, srcImage->_height);
      unsigned char *src = srcImage->_image;
      unsigned char *dst = image->_image;
      for (int sy = 0, dy = y; sy < sh && dy < dh; sy++, dy++) {
        int syoffs = (4 * sy * srcImage->_width);
        int dyoffs = (4 * dy * dw);
        for (int sx = 0, dx = x; sx < sw && dx < dw; sx++, dx++) {
          int soffs = syoffs + (4 * sx);
          int doffs = dyoffs + (4 * dx);
          dst[doffs + 0] = src[soffs + 0];
          dst[doffs + 1] = src[soffs + 1];
          dst[doffs + 2] = src[soffs + 2];
          dst[doffs + 3] = src[soffs + 3];
        }
      }
    }
  } else {
    err_throw(ERR_PARAM);
  }
}

void create_image(var_p_t var, ImageBuffer *image) {
  map_init(var);
  map_add_var(var, IMG_ID, ++nextId);
  map_add_var(var, IMG_WIDTH, image->_width);
  map_add_var(var, IMG_HEIGHT, image->_height);
  map_add_var(var, IMG_BID, image->_bid);
  create_func(var, "filter", cmd_image_filter);
  create_func(var, "paste", cmd_image_paste);
  create_func(var, "save", cmd_image_save);
}

//
// png = image(#1)
// png = image("file.png")
// png = image(array)
// png = image(png)
// png = image(10, 10)
//
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
