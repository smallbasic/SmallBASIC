#

all: sbfltk

psrc = sbfltk.cxx sbfui.cxx ../bas2pdb.cc 
csrc = dev_fltk.c ../device.c ../dev_genfb.c 

sbfui.cxx: sbfui.fl
	fluid -c sbfui.fl

build_kwpc: build_kwpc.c
	g++ build_kwpc.c -o build_kwpc
	-rm -f kwp

kwp.cxx: build_kwpc
	./build_kwpc

sbfltk: $(csrc) $(psrc) kwp.cxx
	gcc -c -D_UnixOS -D_GUI -I.. dev_fltk.c -o dev_fltk.o
	gcc -c -D_UnixOS -D_GUI -I.. ../device.c -o device.o
	gcc -c -D_UnixOS -D_GUI -I.. ../dev_genfb.c -o dev_genfb.o
	g++ -D_UnixOS -D_GUI -I/usr/include -I/usr/X11/include -I.. $(psrc) dev_fltk.o device.o dev_genfb.o -o sbfltk -L/usr/X11/lib \
		-lsbasic -lm -ldl -lncurses -lX11 -lXt -lXpm -lXext -lfltk -lpcre
	cp sbfltk bin

clean:
	-rm -f sbfltk sbfui.cxx sbfui.h build_kwpc kwp.cxx kwp

