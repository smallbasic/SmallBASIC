cc=g++
cflags=-D_UnixOS $(shell rhtv-config --include) $(shell rhtv-config --cppflags)
csb=-lsbasic -lsbfbdrv 
clibs=$(csb) $(shell rhtv-config --dlibs)
exe=tsbide

all: $(exe)

srcs = tvedit.cc helpbase.cc help.cc tprop.cc

objs = $(addsuffix .o, $(basename $(srcs)))

%.o: %.cc
	$(cc) $(cflags) -c $< 

parts/tvhc: parts/tvhc.cc
	make -C parts

doc/sbasic.txt:
	make -C doc

$(exe): parts/tvhc doc/sbasic.txt $(objs)
	$(cc) $(cflags) $(objs) -D_GUI ../bas2pdb.cc -o $(exe) $(clibs)
	strip $(exe)
	cp $(exe) bin

clean:
	make -C parts clean
	make -C doc clean
	-rm -f $(exe) *.o

