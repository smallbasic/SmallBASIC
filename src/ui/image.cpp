// This file is part of SmallBASIC
//
// Image handling
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2002-2019 Chris Warren-Smith.

#include "common/sberr.h"
#include "common/sys.h"
#include "common/messages.h"
#include "common/pproc.h"
#include "common/fs_socket_client.h"
#include "include/var.h"
#include "lib/maapi.h"
#include "lib/lodepng/lodepng.h"
#include "ui/image.h"
#include "ui/system.h"
#include "ui/rgb.h"
#include <cstdint>

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
strlib::List<ImageBuffer *> buffers;

extern "C" int xpm_decode32(uint8_t **image, unsigned *width, unsigned *height, const char *const *xpm);

void reset_image_cache() {
  buffers.removeAll();
}

ImageBuffer::ImageBuffer() :
  _filename(nullptr),
  _image(nullptr),
  _bid(0),
  _width(0),
  _height(0) {
}

ImageBuffer::ImageBuffer(ImageBuffer &o) = default;

ImageBuffer::~ImageBuffer() {
  free(_filename);
  free(_image);
  _filename = nullptr;
  _image = nullptr;
}

ImageDisplay::ImageDisplay() :
  Shape(0, 0, 0, 0),
  _offsetLeft(0),
  _offsetTop(0),
  _zIndex(0),
  _opacity(0),
  _id(0),
  _bid(0),
  _buffer(nullptr) {
}

ImageDisplay::ImageDisplay(ImageDisplay &o) :
  ImageDisplay() {
  copyImage(o);
}

void ImageDisplay::copyImage(ImageDisplay &o) {
  _x = o._x;
  _y = o._y;
  _offsetLeft = o._offsetLeft;
  _offsetTop = o._offsetTop;
  _width = o._width;
  _height = o._height;
  _zIndex = o._zIndex;
  _opacity = o._opacity;
  _id = o._id;
  _bid = o._bid;
  _buffer = o._buffer;
}

void ImageDisplay::draw(int x, int y, int w, int h, int cw) {
  if (_buffer != nullptr) {
    MAPoint2d dstPoint;
    MARect srcRect;

    dstPoint.x = x;
    dstPoint.y = y;
    srcRect.left = MIN(_buffer->_width, _offsetLeft);
    srcRect.top = MIN(_buffer->_height, _offsetTop);
    srcRect.width = MIN(w, MIN(_buffer->_width, _width));
    srcRect.height = MIN(h, MIN(_buffer->_height, _height));
    dev_map_point(&dstPoint.x, &dstPoint.y);

    if (unsigned(srcRect.top + srcRect.height) > _buffer->_height) {
      srcRect.height = _buffer->_height - srcRect.top;
    }
    if (unsigned(srcRect.left + srcRect.width) > _buffer->_width) {
      srcRect.width = _buffer->_width - srcRect.left;
    }

    maDrawRGB(&dstPoint, _buffer->_image, &srcRect, _opacity, _buffer->_width);
  }
}

void to_argb(unsigned char *image, unsigned w, unsigned h) {
#if defined(_SDL)
  // convert from LCT_RGBA to ARGB
  for (unsigned y = 0; y < h; y++) {
    unsigned yoffs = (y * w * 4);
    for (unsigned x = 0; x < w; x++) {
      unsigned offs = yoffs + (x * 4);
      uint8_t r = image[offs + 2];
      uint8_t b = image[offs + 0];
      image[offs + 2] = b;
      image[offs + 0] = r;
    }
  }
#endif
}

unsigned decode_png(unsigned char **image, unsigned *w, unsigned *h, const unsigned char *buffer, size_t size) {
  unsigned error = lodepng_decode32(image, w, h, buffer, size);
  if (!error) {
    to_argb(*image, *w, *h);
  }
  return error;
}

unsigned decode_png_file(unsigned char **image, unsigned *w, unsigned *h, const char *filename) {
  unsigned error = lodepng_decode32_file(image, w, h, filename);
  if (!error) {
    to_argb(*image, *w, *h);
  }
  return error;
}

unsigned encode_png_file(const char *filename, const unsigned char *image, unsigned w, unsigned h) {
  unsigned result;
#if defined(_SDL)
  unsigned size = w * h * 4;
  auto imageCopy = (uint8_t *)malloc(size);
  if (!imageCopy) {
    // lodepng memory error code
    result = 83;
  } else {
    // convert from ARGB to LCT_RGBA
    for (unsigned y = 0; y < h; y++) {
      unsigned yoffs = (y * w * 4);
      for (unsigned x = 0; x < w; x++) {
        int offs = yoffs + (x * 4);
        uint8_t a, r, g, b;
        GET_IMAGE_ARGB(image, offs, a, r, g, b);
        imageCopy[offs + 3] = a;
        imageCopy[offs + 2] = b;
        imageCopy[offs + 1] = g;
        imageCopy[offs + 0] = r;
      }
    }
    result = lodepng_encode32_file(filename, imageCopy, w, h);
    free(imageCopy);
  }
#else
  result = lodepng_encode32_file(filename, image, w, h);
#endif
  return result;
}

dev_file_t *eval_filep() {
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

uint8_t *get_image_data(int x, int y, int w, int h) {
  MARect rc;
  rc.left = x;
  rc.top = y;
  rc.width = w;
  rc.height = h;
  int size = w * h * 4;
  auto result = (uint8_t *)malloc(size);
  if (result != nullptr) {
    g_system->getOutput()->redraw();
    maGetImageData(HANDLE_SCREEN, result, &rc, w * 4);
  }
  return result;
}

ImageBuffer *get_image(unsigned bid) {
  ImageBuffer *result = nullptr;
  List_each(ImageBuffer *, it, buffers) {
    ImageBuffer *next = (*it);
    if (next->_bid == (unsigned)bid) {
      result = next;
      break;
    }
  }
  return result;
}

void scaleImage(ImageBuffer* image, var_num_t scaling) {
  if(scaling == 1.0 || scaling <= 0.0) {
    return;
  }

  uint16_t xx, yy, w, h;
  uint32_t offsetImage, offsetScaledImage;

  w = round((var_num_t)image->_width * scaling);
  h = round((var_num_t)image->_height * scaling);

  uint8_t* scaledImage = (uint8_t *)malloc(w * h * 4);
  if(!scaledImage) {
    err_throw(ERR_IMAGE_LOAD, "Failed to allocate RAM");
  }

  uint32_t* image32bit = (uint32_t*)image->_image;
  uint32_t* scaledImage32bit = (uint32_t*)scaledImage;

  for(yy = 0; yy < h; yy++) {
    for(xx = 0; xx < w; xx++) {
      offsetScaledImage = yy * w + xx;
      offsetImage = floor((var_num_t)yy / scaling) * image->_width + floor((var_num_t)xx / scaling);
      scaledImage32bit[offsetScaledImage] = image32bit[offsetImage];
    }
  }

  free(image->_image);
  image->_width = w;
  image->_height = h;
  image->_image = scaledImage;
}

//
// Copy from screen
//
ImageBuffer *load_image(var_int_t x) {
  var_int_t y, w, h;
  var_num_t scaling = 1.0;
  int count = par_massget("iiif", &y, &w, &h, &scaling);
  int width = g_system->getOutput()->getWidth();
  int height = g_system->getOutput()->getHeight();
  ImageBuffer *result = nullptr;

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
    if (image == nullptr) {
      err_throw(ERR_IMAGE_LOAD, "Failed to load screen image");
    } else {
      result = new ImageBuffer();
      result->_bid = ++nextId;
      result->_width = w;
      result->_height = h;
      result->_filename = nullptr;
      result->_image = image;
      scaleImage(result, scaling);
      buffers.add(result);
    }
  }
  return result;
}

//
// share image buffer from another image variable
//
ImageBuffer *load_image(var_t *var) {
  ImageBuffer *result = nullptr;
  var_num_t scaling = 1.0;

  if (code_peek() == kwTYPE_SEP) {
    code_skipsep();
    scaling = par_getnum();
  }

  if (var->type == V_MAP) {
    int bid = map_get_int(var, IMG_BID, -1);
    if (bid != -1) {
      if(scaling == 1.0 || scaling <= 0.0) {
        result = get_image((unsigned)bid);
      } else {
        ImageBuffer *inputImage = nullptr;
        inputImage = get_image((unsigned)bid);
        uint8_t* imageData = (uint8_t *)malloc(inputImage->_width * inputImage->_height * 4);
        if(!imageData) {
          err_throw(ERR_IMAGE_LOAD, "Failed to allocate RAM");
        }
        result = new ImageBuffer;
        result->_bid = ++nextId;
        result->_width = inputImage->_width;
        result->_height = inputImage->_height;
        result->_filename = nullptr;
        memcpy(imageData, inputImage->_image, inputImage->_width * inputImage->_height * 4);
        result->_image = imageData;
        scaleImage(result, scaling);
        buffers.add(result);
      }
    }
  } else if (var->type == V_ARRAY && v_maxdim(var) == 2) {
    int h = ABS(v_ubound(var, 0) - v_lbound(var, 0)) + 1;
    int w = ABS(v_ubound(var, 1) - v_lbound(var, 1)) + 1;
    int size = w * h * 4;
    auto imageData = (uint8_t *)malloc(size);
    if(!imageData) {
      err_throw(ERR_IMAGE_LOAD, "Failed to allocate RAM");
    }
    for (int y = 0; y < h; y++) {
      int yoffs = (y * w * 4);
      for (int x = 0; x < w; x++) {
        int pos = y * w + x;
        uint8_t a, r, g, b;
        v_get_argb(v_getint(v_elem(var, pos)), a, r, g, b);
        SET_IMAGE_ARGB(imageData, yoffs + (x * 4), a, r, g, b);
      }
    }
    result = new ImageBuffer();
    result->_bid = ++nextId;
    result->_width = w;
    result->_height = h;
    result->_filename = nullptr;
    result->_image = imageData;
    scaleImage(result, scaling);
    buffers.add(result);
  }
  return result;
}

//
// Load from file
//
ImageBuffer *load_image(const unsigned char *buffer, int32_t size) {
  ImageBuffer *result = nullptr;
  unsigned w, h;
  unsigned char *image;
  var_num_t scaling = 1.0;

  if (code_peek() == kwTYPE_SEP) {
    code_skipsep();
    scaling = par_getnum();
  }

  unsigned error = decode_png(&image, &w, &h, buffer, size);
  if (!error) {
    result = new ImageBuffer();
    result->_bid = ++nextId;
    result->_width = w;
    result->_height = h;
    result->_filename = nullptr;
    result->_image = image;
    scaleImage(result, scaling);
    buffers.add(result);
  } else {
    err_throw(ERR_IMAGE_LOAD, lodepng_error_text(error));
  }
  return result;
}

//
// Load from file
//
ImageBuffer *load_image(dev_file_t *filep) {
  ImageBuffer *result = nullptr;
  var_num_t scaling = 1.0;

  if (code_peek() == kwTYPE_SEP) {
    code_skipsep();
    scaling = par_getnum();
  }

  if(scaling == 1.0 || scaling <= 0.0) {
    List_each(ImageBuffer *, it, buffers) {
      ImageBuffer *next = (*it);
      if (next->_filename != nullptr && strcmp(next->_filename, filep->name) == 0) {
        result = next;
        break;
      }
    }
  }
  if (result == nullptr) {
    unsigned w, h;
    unsigned char *imageData;
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
        error = decode_png(&imageData, &w, &h, (unsigned char *)var_p->v.p.ptr, var_p->v.p.length);
        v_free(var_p);
        v_detach(var_p);
      }
      break;
    case ft_stream:
      error = decode_png_file(&imageData, &w, &h, filep->name);
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
      result->_image = imageData;
      scaleImage(result, scaling);
      buffers.add(result);
    }
  }
  return result;
}

//
// Create from XPM data
//
ImageBuffer *load_xpm_image(char **data) {
  unsigned w, h;
  unsigned char *image;
  unsigned error = xpm_decode32(&image, &w, &h, data);
  ImageBuffer *result = nullptr;
  var_num_t scaling = 1.0;

  if (code_peek() == kwTYPE_SEP) {
    code_skipsep();
    scaling = par_getnum();
  }

  if (!error) {
    result = new ImageBuffer();
    result->_bid = ++nextId;
    result->_width = w;
    result->_height = h;
    result->_filename = nullptr;
    result->_image = image;
    scaleImage(result, scaling);
    buffers.add(result);
  } else {
    err_throw(ERR_IMAGE_LOAD, ERR_XPM_IMAGE);
  }
  return result;
}

void get_image_display(var_s *self, ImageDisplay *image) {
  image->_bid = map_get_int(self, IMG_BID, -1);

  List_each(ImageBuffer *, it, buffers) {
    ImageBuffer *next = (*it);
    if (next->_bid == image->_bid) {
      image->_buffer = next;
      break;
    }
  }

  var_int_t x, y, z, op;
  int count = par_massget("iiii", &x, &y, &z, &op);

  if (prog_error || image->_buffer == nullptr || count == 1 || count > 4) {
    err_throw(ERR_PARAM);
  } else {
    // 0, 2, 3, 4 arguments accepted
    if (count >= 2) {
      image->_x = x;
      image->_y = y;
      map_set_int(self, IMG_X, x);
      map_set_int(self, IMG_Y, y);
    } else {
      image->_x = map_get_int(self, IMG_X, -1);
      image->_y = map_get_int(self, IMG_Y, -1);
    }
    if (count >= 3) {
      image->_zIndex = z;
      map_set_int(self, IMG_ZINDEX, z);
    } else {
      image->_zIndex = map_get_int(self, IMG_ZINDEX, -1);
    }
    if (count == 4) {
      image->_opacity = op;
      map_set_int(self, IMG_OPACITY, op);
    } else {
      image->_opacity = map_get_int(self, IMG_OPACITY, -1);
    }

    image->_offsetLeft = map_get_int(self, IMG_OFFSET_LEFT, -1);
    image->_offsetTop = map_get_int(self, IMG_OFFSET_TOP, -1);
    image->_width = map_get_int(self, IMG_WIDTH, -1);
    image->_height = map_get_int(self, IMG_HEIGHT, -1);
    image->_id = map_get_int(self, IMG_ID, -1);
  }
}

//
// png.show(x, y, zindex, opacity)
//
void cmd_image_show(var_s *self, var_s *) {
  ImageDisplay image;
  get_image_display(self, &image);
  if (!prog_error) {
    g_system->getOutput()->addImage(image);
  }
}

//
// png.draw(x, y, opacity)
//
void cmd_image_draw(var_s *self, var_s *) {
  ImageDisplay image;
  get_image_display(self, &image);
  if (!prog_error) {
    image._opacity = image._zIndex;
    g_system->getOutput()->drawImage(image);
  }
}

//
// png.hide()
//
void cmd_image_hide(var_s *self, var_s *) {
  int id = map_get_int(self, IMG_ID, -1);
  g_system->getOutput()->removeImage(id);
}

//
// Output the image to a PNG file or an array
//
// png.save("horse1.png")
// png.save(#1)
// png.save(byref var)
// file = "abc.png" : png.save(file)
//
void cmd_image_save(var_s *self, var_s *) {
  unsigned id = map_get_int(self, IMG_BID, -1);
  ImageBuffer *image = get_image(id);
  dev_file_t *file;
  var_t *var;
  var_t str;
  bool saved = false;
  if (!prog_error && image != nullptr) {
    unsigned w = image->_width;
    unsigned h = image->_height;
    switch (code_peek()) {
    case kwTYPE_SEP:
      file = eval_filep();
      if (file != nullptr && file->open_flags == DEV_FILE_OUTPUT &&
          !encode_png_file(file->name, image->_image, w, h)) {
        saved = true;
      }
      break;
    case kwTYPE_STR:
      par_getstr(&str);
      if (!prog_error &&
          !encode_png_file(str.v.p.ptr, image->_image, w, h)) {
        saved = true;
      }
      v_free(&str);
      break;
    default:
      var = par_getvar_ptr();
      if (var->type == V_STR && !prog_error &&
          !encode_png_file(var->v.p.ptr, image->_image, w, h)) {
        saved = true;
      } else if (!prog_error) {
        uint32_t offsetLeft = map_get_int(self, IMG_OFFSET_LEFT, 0);
        uint32_t offsetTop = map_get_int(self, IMG_OFFSET_TOP, 0);
        uint32_t wClip = map_get_int(self, IMG_WIDTH, w);
        uint32_t hClip = map_get_int(self, IMG_HEIGHT, h);

        if (offsetTop < h && offsetLeft < w) {
          if (offsetTop + hClip > h) {
            hClip = h - offsetTop;
          }
          if (offsetLeft + wClip > w) {
            wClip = w - offsetLeft;
          }
          v_tomatrix(var, hClip, wClip);
          //     x0   x1   x2    (w=3,h=2)
          // y0  rgba rgba rgba  ypos=0
          // y1  rgba rgba rgba  ypos=12
          //
          for (unsigned y = offsetTop; y < offsetTop + hClip; y++) {
            unsigned yoffs = (y * w * 4);
            for (unsigned x = offsetLeft; x < offsetLeft + wClip; x++) {
              uint8_t a, r, g, b;
              GET_IMAGE_ARGB(image->_image, yoffs + (x * 4), a, r, g, b);
              pixel_t px = v_get_argb_px(a, r, g, b);
              unsigned pos = (y - offsetTop ) * wClip + (x - offsetLeft);
              v_setint(v_elem(var, pos), px);
            }
          }
        } else {
          v_tomatrix(var, hClip, wClip);
        }
        saved = true;
      }
    }
  }
  if (!saved) {
    err_throw(ERR_IMAGE_SAVE);
  }
}


//
// Reduces the size of the image
// arguments: left, top, width, height
//
// png.clip(10, 10, 10, 10)
//
void cmd_image_clip(var_s *self, var_s *) {
  if (self->type == V_MAP) {
    int bid = map_get_int(self, IMG_BID, -1);
    if (bid != -1) {
      ImageBuffer *image = get_image((unsigned)bid);
      var_int_t left, top, width, heigth;
      if (image != nullptr && par_massget("iiii", &left, &top, &width, &heigth)) {
        if (left < 0) {
          left = 0;
        }
        if (top < 0) {
          top = 0;
        }
        map_set_int(self, IMG_OFFSET_LEFT, left);
        map_set_int(self, IMG_OFFSET_TOP, top);
        map_set_int(self, IMG_WIDTH, width);
        map_set_int(self, IMG_HEIGHT, heigth);
      }
    }
  }
}

void create_image(var_p_t var, ImageBuffer *image) {
  map_init(var);
  map_add_var(var, IMG_X, 0);
  map_add_var(var, IMG_Y, 0);
  map_add_var(var, IMG_OFFSET_TOP, 0);
  map_add_var(var, IMG_OFFSET_LEFT, 0);
  map_add_var(var, IMG_ZINDEX, 100);
  map_add_var(var, IMG_OPACITY, 0);
  map_add_var(var, IMG_ID, ++nextId);
  map_add_var(var, IMG_WIDTH, image->_width);
  map_add_var(var, IMG_HEIGHT, image->_height);
  map_add_var(var, IMG_BID, image->_bid);
  v_create_func(var, "draw", cmd_image_draw);
  v_create_func(var, "hide", cmd_image_hide);
  v_create_func(var, "save", cmd_image_save);
  v_create_func(var, "show", cmd_image_show);
  v_create_func(var, "clip", cmd_image_clip);
}

// loads an image for the form image input type
ImageDisplay *create_display_image(var_p_t var, const char *name) {
  ImageDisplay *result = nullptr;
  if (name != nullptr && var != nullptr) {
    dev_file_t file;
    strlcpy(file.name, name, sizeof(file.name));
    file.type = ft_stream;
    ImageBuffer *buffer = load_image(&file);
    if (buffer != nullptr) {
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
  auto image = get_image_data(0, 0, width, height);
  if (image != nullptr) {
    const char *path = gsb_bas_dir;
#if defined(_ANDROID)
    path = getenv("EXTERNAL_DIR");
#endif
    for (int i = 0; i < 1000; i++) {
      String file;
      if (strstr(path, "://") == nullptr) {
        file.append(path);
      }
      if (file.lastChar() != '/') {
        file.append("/");
      }
      file.append("sbasic_dump_");
      file.append(i);
      file.append(".png");
      if (access(file.c_str(), R_OK) != 0) {
        g_system->systemPrint("Saving screen to %s\n", file.c_str());
        unsigned error = encode_png_file(file.c_str(), image, width, height);
        if (error) {
          g_system->systemPrint("Error: %s\n", lodepng_error_text(error));
        }
        break;
      }
    }
    free(image);
  }
}

extern "C" void v_create_image(var_p_t var) {
  var_t arg;
  ImageBuffer *image = nullptr;
  dev_file_t *filep = nullptr;

  byte code = code_peek();
  switch (code) {
  case kwTYPE_SEP:
    filep = eval_filep();
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
      // Img = Image(FileName)
      dev_file_t file;
      strlcpy(file.name, arg.v.p.ptr, sizeof(file.name));
      file.type = ft_stream;
      image = load_image(&file);
    } else if (arg.type == V_ARRAY && v_asize(&arg) > 0 && !prog_error) {
      var_p_t elem0 = v_elem(&arg, 0);
      if (elem0->type == V_STR) {
        // Img = Image(PixmapData)
        char **data = new char*[v_asize(&arg)];
        for (unsigned i = 0; i < v_asize(&arg); i++) {
          var_p_t elem = v_elem(&arg, i);
          data[i] = elem->v.p.ptr;
        }
        image = load_xpm_image(data);
        delete [] data;
      } else if (v_maxdim(&arg) == 2) {
        // Img = Image(Array2D)
        image = load_image(&arg);
      } else if (elem0->type == V_INT) {
        // Create from buffer?
        auto *data = new unsigned char[v_asize(&arg)];
        for (unsigned i = 0; i < v_asize(&arg); i++) {
          var_p_t elem = v_elem(&arg, i);
          data[i] = (unsigned char)elem->v.i;
        }
        image = load_image(data, v_asize(&arg));
        delete [] data;
      }
    } else if (arg.type == V_INT && !prog_error) {
      // Copy from screen
      image = load_image(arg.v.i);
    } else {
      // Img2 = image(Img1)  ->  Img2 is pointer to existing buffer Img1
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
