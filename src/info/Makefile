all: cinfo ninf

cinfo: info.cc info.h cinfo.cc
	gcc -D_UnixOS -g info.cc cinfo.cc -o cinfo -lncurses

ninf: info.cc info.h ninf.cc
	gcc -D_UnixOS -g info.cc ninf.cc -o ninf -lncurses

clean:
	-rm -f cinfo ninf
