# SmallBASIC for eBookMan
# Copyright(C) 2001-2002 Chris Warren-Smith. Gawler, South Australia
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

APPNAME = SmallBASIC
SRCS_CPP = ebm_fs.cpp ebm_osd.cpp ebm_main.cpp AnsiWindow.cpp  \
           ebjlib.cpp resource.cpp HTMLWindow.cpp input.cpp ebm_mem.cpp \
           Dialog.cpp FileDlg.cpp

SRCS_C = brun.c ceval.c eval.c str.c match.c \
   panic.c blib.c bc.c blib_db.c blib_math.c \
   var.c proc.c blib_func.c blib_sound.c  \
   blib_graph.c circle.c ffill.c pfill.c \
   device.c scan.c fmt.c kw.c sberr.c \
   extlib.c g_line.c mem.c blib_ui.c 

HEAPSPACE       = 1024000
MAINSTACKSPACE  = 16384
TIMERSTACKSPACE = 4096
GUI_LOCAL_COPY  = no
OS_LOCAL_COPY   = no
LANG_LOCAL_COPY = no
GUI_LANGUAGE    = GUI_Amr.pkg
OS_BASEDIR      = ../../../os
GUI_BASEDIR     = ../source
ERROR           = -Werror
include ${EBOOKMAN_SDK}/ebsdk.uses
include ${ESDK_TARGET_INC}/Makefile.GUI
INCFLAGS       += -I../ -I.
ZIP             = ${APPNAME}.tgz
OBJS            = ${SRCS_CPP:%.cpp=${OBJDIR}/%.o} ${SRCS_C:%.c=${OBJDIR}/%.o} 

${OBJDIR}/%.o :: %.cpp
	${ESDK_GXX32_EXE} -b sneak32 -g -D_FRANKLIN_EBM $(CFLAGS) -o $@ -c $<

${OBJDIR}/%.o :: ../%.c
	${ESDK_GCC32_EXE} -b sneak32 -g -D_FRANKLIN_EBM -O2 -fno-defer-pop\
 -Wmissing-declarations ${INCFLAGS} -o $@ -c $<

sGDB: Makefile
	@echo "#! /bin/sh" > $@.tmp
	@echo "${ESDK_INSIGHT_EXE} ${APPNAME}_sa.coff ${GDBDIRS} -dir ../" >> $@.tmp
	@chmod a+x $@.tmp
	@mv $@.tmp $@

lib.snk32/ebm_main.o: ebjlib.h ebm_main.h AnsiWindow.h HTMLWindow.h

all: os

# To run this you need to first edit lib.snk32\objects.link
# and add the line:
# OBJECT  ./lib.snk32/piezo
debug: ./lib.snk32/piezo.o sa
	./sGDB

objs:${OBJS}

sync: ${APPNAME}.seb
	cp -p ${APPNAME}.seb /sync

res:
	rcp2ebm.sh resource.rcp -header > resource.h
	rcp2ebm.sh resource.rcp > resource.cpp

resource.cpp:resource.rcp
	rcp2ebm.sh resource.rcp > resource.cpp

sb:
	mkdir lib.snk32
	make -f Makefile.sb

help:
	${EBOOKMAN_SDK}/${ARCH}/bin/binpkg.exe doc/files.pkg \
        doc/strings.txt ${APPNAME}.help

backup:
	make clean
	touch $(ZIP)
	rm $(ZIP)
	tar -zcvf $(ZIP) *
	cp $(ZIP) /mnt/floppy

