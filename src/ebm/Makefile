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

SRCS_CPP = \
resource.cpp                                    \
Browser.cpp                                     \
ebm_fs.cpp                                      \
ebm_main.cpp                                    \
ebm_mem.cpp                                     \
ebm_osd.cpp                                     \
input.cpp

SRCS_C = \
bc.c                                            \
blib.c                                          \
blib_db.c                                       \
blib_func.c                                     \
blib_graph.c                                    \
blib_math.c                                     \
blib_sound.c                                    \
blib_ui.c                                       \
brun.c                                          \
ceval.c                                         \
circle.c                                        \
decomp.c                                        \
device.c                                        \
eval.c                                          \
extlib.c                                        \
ffill.c                                         \
fmt.c                                           \
g_bmp.c                                         \
g_line.c                                        \
geom.c                                          \
kw.c                                            \
match.c                                         \
mem.c                                           \
panic.c                                         \
pfill.c                                         \
plot.c                                          \
proc.c                                          \
sberr.c                                         \
scan.c                                          \
str.c                                           \
tasks.c                                         \
units.c                                         \
var.c

EBJLIB_HOME     = /home/src/ebjlib
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
DEBUG_DIRS      = ${GDBDIRS} -dir ../ -dir ${EBJLIB_HOME}
CLEANLIST      += resource.cpp app_resource.h

include ${EBOOKMAN_SDK}/ebsdk.uses
include ${ESDK_TARGET_INC}/Makefile.GUI
INCFLAGS       += -I../ -I. -I${EBJLIB_HOME}
OBJS            = ${SRCS_CPP:%.cpp=${OBJDIR}/%.o} ${SRCS_C:%.c=${OBJDIR}/%.o} 

${OBJDIR}/%.o :: %.cpp
	${ESDK_GXX32_EXE} -b sneak32 -g -D_FRANKLIN_EBM $(CFLAGS) -o $@ -c $<

${OBJDIR}/%.o :: ../%.c
	${ESDK_GCC32_EXE} -b sneak32 -g -D_FRANKLIN_EBM -O2 -fno-defer-pop\
 -Wmissing-declarations ${INCFLAGS} -o $@ -c $<

resource.cpp:resource.rcp
	rcp2ebm.sh resource.rcp -header > app_resource.h
	rcp2ebm.sh resource.rcp -include resource.h > resource.cpp

all: ${EBJLIB_HOME}/ebjlib.lib resource.cpp os

${APPNAME}_sa.coff: ${EBJLIB_HOME}/ebjlib.lib
${APPNAME}.seb: ${EBJLIB_HOME}/ebjlib.lib

debug_sa: ./lib.snk32/piezo.o
	ar rs debug_lib.lib $<
	rm SmallBASIC_sa.coff

# To run this you need to first add the following line to libraries.link
# LIBRARY debug_lib.lib
# and then make clean; make debug
debug: sa
	@echo "#! /bin/sh" > $@.tmp
	@echo "${ESDK_INSIGHT_EXE} ${APPNAME}_sa.coff ${DEBUG_DIRS}" >> $@.tmp
	@chmod a+x $@.tmp
	./$@.tmp

install: ${APPNAME}.seb
	/ebm/ebmsync/ebmsync -M 1 -p ${APPNAME}.seb

help:
	${EBOOKMAN_SDK}/${ARCH}/bin/binpkg.exe doc/files.pkg \
        doc/strings.txt ${APPNAME}.help

${OBJDIR}/objects.link: libraries.link
	@echo "Processing link files..."
	@echo "marker ram 2 _HEAP_START ${HEAPSPACE}" > $@.tmp
	@echo "marker ram 2 _HEAP_END" >> $@.tmp
	@for tt in $(OBJS:%.o=%) ; \
	do \
		echo "OBJECT " $$tt >> $@.tmp ; \
	done
	cat libraries.link >> $@.tmp
	@mv $@.tmp $@


