# SmallBASIC for FLTK
# Copyright(C) 2001-2002 Chris Warren-Smith. Gawler, South Australia
# cwarrens@twpo.com.au
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

all:    sbfltk
cygwin: sbfltk

SOURCE =                                        \
	Fl_Ansi_Window.cpp                          \
	MainWindow.cpp                              \
	EditorWindow.cpp                            \
    dev_fltk.cpp                                \
    blib_fltk_ui.c                              \

COMMON=                                         \
    bc.c                                        \
    blib.c                                      \
    blib_db.c                                   \
    blib_func.c                                 \
    blib_graph.c                                \
    blib_math.c                                 \
    blib_sound.c                                \
    brun.c                                      \
    ceval.c                                     \
    circle.c                                    \
    decomp.c                                    \
    device.c                                    \
    eval.c                                      \
    extlib.c                                    \
    file.c                                      \
    ffill.c                                     \
    fmt.c                                       \
    fs_irda.c                                   \
    fs_memo.c                                   \
    fs_pdoc.c                                   \
    fs_serial.c                                 \
    fs_socket_client.c                          \
    fs_stream.c                                 \
    g_bmp.c                                     \
    g_line.c                                    \
    geom.c                                      \
    inet.c                                      \
    kw.c                                        \
    match.c                                     \
    mem.c                                       \
    panic.c                                     \
    pfill.c                                     \
    plot.c                                      \
    proc.c                                      \
    sberr.c                                     \
    scan.c                                      \
    str.c                                       \
    tasks.c                                     \
    units.c                                     \
    unx_memmgr.c                                \
    var.c                                       \
    vmt.c                                       \

MAKEDEPEND	= /usr/X11R6/bin/makedepend -Y
##OPTIM =  -O2 -Wall -Wunused -fno-exceptions
OPTIM =  -g -O0 -Wall -Wunused -fno-exceptions
PLATFORM=-D_UnixOS -D_CygWin -D_FLTK

## patched fltk-config to remove -mwindows
##LDLIBS = -lm -lgcc -lreadline -ltermcap -s `fltk-config --ldflags`
LDLIBS = -lm -lgcc -lreadline -ltermcap `fltk-config --ldflags`
RESOURCES = sbfltk.res

CXXFLAGS = ${OPTIM} -I../ -I. `fltk-config --cxxflags`

linux: PLATFORM=-D_UnixOS RESOURCES=
debug: OPTIM = -g -O0 -Wall -Wunused -fno-exceptions

OBJDIR = ./objs/

OBJS += $(addprefix ${OBJDIR}, $(addsuffix .o, $(basename $(SOURCE)))) \
        $(addprefix ${OBJDIR}, $(addsuffix .o, $(basename $(COMMON))))

# Make dependencies, excluding standard include directories
depend:
	$(MAKEDEPEND) -fmakedepend -I.. -I. -I/usr/include `fltk-config --cxxflags` $(SOURCE)

# Include automatically generated dependencies
include makedepend

##.SILENT:

sbfui.cxx: sbfui.fl
	fluid -c sbfui.fl

build_kwpc: build_kwpc.c
	g++ build_kwpc.c -o build_kwpc
	-rm -f kwp

EditorWindow.cpp:kwp.cxx

kwp.cxx: build_kwpc
	./build_kwpc

sbfltk: $(OBJS) ${RESOURCES}
	gcc $(OBJS) ${LDLIBS} ${RESOURCES} -o $@

sbfltk.res : sbfltk.rc
	windres $< -O coff -o $@

#unit test
Fl_Ansi_Window.exe: Fl_Ansi_Window.o 
	gcc ${OPTIM} $< ${LDLIBS} -o $@

clean:
	-rm -f sbfltk sbfui.cxx sbfui.h build_kwpc kwp.cxx kwp objs/*.o

${OBJDIR}:
	mkdir -p $@

${OBJDIR}%.o: ../%.c
	gcc ${PLATFORM} ${OPTIM} ${CXXFLAGS} -o $@ -c $<

${OBJDIR}%.o: %.cpp
	gcc ${PLATFORM} ${OPTIM} ${CXXFLAGS} -o $@ -c $<

${OBJDIR}%.o: %.cxx
	gcc ${PLATFORM} ${OPTIM} ${CXXFLAGS} -o $@ -c $<

${OBJDIR}%.o: %.c
	gcc ${PLATFORM} ${OPTIM} ${CXXFLAGS} -o $@ -c $<

