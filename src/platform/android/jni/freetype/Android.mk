# SmallBASIC
# Copyright(C) 2001-2014 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(FREETYPE_HOME)/include \
                    $(FREETYPE_HOME)/builds \
                    $(FREETYPE_HOME)/include/freetype/config \
                    $(FREETYPE_HOME)/include/freetype
LOCAL_MODULE     := freetype
LOCAL_CFLAGS     := -Wall -std=gnu99 \
                    "-DFT_CONFIG_CONFIG_H=<ftconfig.h>" \
                    "-DFT2_BUILD_LIBRARY" \
                    "-DFT_CONFIG_MODULES_H=<ftmodule.h>"

LOCAL_SRC_FILES:=                               \
  $(FREETYPE_HOME)/src/type1/type1.c            \
  $(FREETYPE_HOME)/src/cid/type1cid.c           \
  $(FREETYPE_HOME)/src/pfr/pfr.c                \
  $(FREETYPE_HOME)/src/type42/type42.c          \
  $(FREETYPE_HOME)/src/winfonts/winfnt.c        \
  $(FREETYPE_HOME)/src/pcf/pcf.c                \
  $(FREETYPE_HOME)/src/psaux/psaux.c            \
  $(FREETYPE_HOME)/src/bdf/bdf.c                \
  $(FREETYPE_HOME)/src/base/ftbbox.c            \
  $(FREETYPE_HOME)/src/base/ftbitmap.c          \
  $(FREETYPE_HOME)/src/base/ftglyph.c           \
  $(FREETYPE_HOME)/src/base/ftstroke.c          \
  $(FREETYPE_HOME)/src/base/ftbase.c            \
  $(FREETYPE_HOME)/src/base/ftsystem.c          \
  $(FREETYPE_HOME)/src/base/ftinit.c            \
  $(FREETYPE_HOME)/src/base/ftgasp.c            \
  $(FREETYPE_HOME)/src/raster/raster.c          \
  $(FREETYPE_HOME)/src/sfnt/sfnt.c              \
  $(FREETYPE_HOME)/src/smooth/smooth.c          \
  $(FREETYPE_HOME)/src/autofit/autofit.c        \
  $(FREETYPE_HOME)/src/truetype/truetype.c      \
  $(FREETYPE_HOME)/src/cff/cff.c                \
  $(FREETYPE_HOME)/src/psnames/psnames.c        \
  $(FREETYPE_HOME)/src/pshinter/pshinter.c      \
  $(FREETYPE_HOME)/src/gzip/ftgzip.c            \
  $(FREETYPE_HOME)/src/lzw/ftlzw.c              \
  $(FREETYPE_HOME)/src/lzw/ftzopen.c

include $(BUILD_STATIC_LIBRARY)
