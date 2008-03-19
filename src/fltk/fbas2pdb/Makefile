all: fbas2pdb

INC=-I/usr/X11/include -L/usr/X11/lib
LIB=-lfltk
DFN=-D_UnixOS -D_GUI

src = fb2p.cc fmain.cc bas2pdb.cc

fbas2pdb: $(src)
	gcc $(DFN) $(src) -o fb2p $(INC) $(LIB)
