# SmallBASIC for eBookMan - Flite module
# Copyright(C) 2004 Chris Warren-Smith. Gawler, South Australia
# cwarrens@twpo.com.au
#
#                  _.-_:\
#                 /      \
#                 \_.--*_/
#                       v
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

TOP=/c/src_3/flite/flite-1.2-release

flite_LIBS = flite_$(FL_VOX) flite_$(FL_LANG) flite_$(FL_LEX) 
flite_LIBS_flags = -L$(LIBDIR) $(flite_LIBS:%=-l%)
flite_LIBS_deps = $(flite_LIBS:%=$(LIBDIR)/lib%.a)

include $(TOP)/config/common_make_rules

CC = gcc
CINC=-I$(TOP)/include -I$(TOP)/lang/$(FL_VOX) -I../.. -I.
CFLAGS=-Wall -fPIC -D_UnixOS $(CINC)
MODULE_DIR=/usr/local/lib/sbasic/modules
CLIBS = -lm -lgcc -s -shared
TARGET=flite12.dll

cygwin: CFLAGS = -O2 -Os -Wall -D_UnixOS -D_CygWin $(CINC)

all: $(MODULE_DIR)/${TARGET}
cygwin: $(MODULE_DIR)/${TARGET}

SRC= flite.c
OBJS = $(addsuffix .o, $(basename $(SRC)))

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(MODULE_DIR)/${TARGET}: $(OBJS) $(FLITELIBS) $(flite_LIBS_deps)
	$(CC) ${CLIBS} -Wl,-soname,${TARGET} -o ${TARGET} $(OBJS) $(flite_LIBS_flags) $(LDFLAGS)

clean:
	-rm -f *.so *.o $(MODULE_DIR)/*.so $(MODULE_DIR)/*.dll

install: ${TARGET}
	-mkdir -p $(MODULE_DIR)
	cp ${<} $(MODULE_DIR)

###########################################################################
##                                                                       ##
##                  Language Technologies Institute                      ##
##                     Carnegie Mellon University                        ##
##                        Copyright (c) 2001                             ##
##                        All Rights Reserved.                           ##
##                                                                       ##
##  Permission is hereby granted, free of charge, to use and distribute  ##
##  this software and its documentation without restriction, including   ##
##  without limitation the rights to use, copy, modify, merge, publish,  ##
##  distribute, sublicense, and/or sell copies of this work, and to      ##
##  permit persons to whom this work is furnished to do so, subject to   ##
##  the following conditions:                                            ##
##   1. The code must retain the above copyright notice, this list of    ##
##      conditions and the following disclaimer.                         ##
##   2. Any modifications must be clearly marked as such.                ##
##   3. Original authors' names are not deleted.                         ##
##   4. The authors' names are not used to endorse or promote products   ##
##      derived from this software without specific prior written        ##
##      permission.                                                      ##
##                                                                       ##
##  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         ##
##  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      ##
##  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   ##
##  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      ##
##  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    ##
##  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   ##
##  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          ##
##  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       ##
##  THIS SOFTWARE.                                                       ##
##                                                                       ##
###########################################################################
