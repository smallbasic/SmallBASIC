UCC=gcc
CFLAGS   = -g -O2 -D_UnixOS -D_SBIDE -I/usr/local/include -I../editor -I..
ULIBS    = -lm -ldl -lpthread -lncurses -lsbfbdrv -lsbasic 

sources=../editor/ted.cpp ted_dt.cc sbide.cc

all: sbide

sbide: $(sources) 
	$(UCC) $(CFLAGS) $(sources) -o sbide $(ULIBS)
	cp sbide bin

clean:
	-rm -f sbide


