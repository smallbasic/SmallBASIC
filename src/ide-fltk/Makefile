#

all: sbfltk

src = sbfltk.cxx sbfui.cxx dev_fltk.c ../device.c ../dev_genfb.c ../bas2pdb.cc

sbfui.cxx: sbfui.fl
	fluid -c sbfui.fl

build_kwpc: build_kwpc.c
	gcc build_kwpc.c -o build_kwpc
	-rm -f kwp

kwp.cxx: build_kwpc
	./build_kwpc

sbfltk: $(src) kwp.cxx
#	gcc -D_UnixOS -D_GUI -I/usr/include -I/usr/X11/include -I.. $(src) -o sbfltk -L/usr/X11/lib -lfltk -lsbasic
	gcc -D_UnixOS -D_GUI -I/usr/include -I/usr/X11/include -I.. $(src) -o sbfltk -L/usr/X11/lib \
		-lsbasic -lm -ldl -lncurses -lX11 -lXt -lXpm -lXext -lfltk 
	cp sbfltk bin

clean:
	-rm -f sbfltk sbfui.cxx sbfui.h build_kwpc kwp.cxx kwp

