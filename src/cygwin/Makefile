# CYGWIN

all: sbnull.exe
debug:sbnull.exe

DCC = gcc
DCPPFLAGS = -O2 -Os -Wall -D_UnixOS -D_CygWin -I.. -I.
debug: DCPPFLAGS = -g -O1 -Wall -D_UnixOS -D_CygWin -I.. -I.
DCFLAGS   = $(DCPPFLAGS)
CLIBS = -lm -lgcc -lreadline -ltermcap  -s

# Common sources
sources=\
	brun.c ceval.c eval.c str.c match.c fmt.c kw.c sberr.c \
	mem.c panic.c blib.c bc.c file.c blib_db.c blib_math.c \
	var.c proc.c blib_func.c blib_sound.c inet.c vmt.c \
	blib_graph.c scan.c circle.c ffill.c pfill.c g_bmp.c \
	extlib.c blib_ui.c plot.c tasks.c units.c \
    fs_stream.c fs_serial.c fs_memo.c fs_pdoc.c fs_socket_client.c fs_irda.c \
	unx_memmgr.c dev_term.c g_line.c decomp.c  \
	geom.c

#
com_sources = $(addprefix ../, $(sources))
com_objects = $(addsuffix .o,  $(basename $(sources)))


# Platform specific sources
dos_obj += $(com_objects)

%.o: ../%.c
	$(DCC) $(DCFLAGS) -c $<

%.o: %.c
	$(DCC) $(DCFLAGS) -c $<

# win32/cygwin without graphics & sound
sbnull.exe: $(dos_obj) ../dev_null.c ../device.c ../console_main.c $(help_files)
	$(DCC) $(DCFLAGS) $(dos_obj) ../dev_null.c ../device.c ../console_main.c -o sbnull.exe $(CLIBS)
	-mv sbnull.exe sbasic.exe
	-cp sbasic.exe bin

bas2pdb.exe: ../bas2pdb.cc
	$(DCC) $(DCFLAGS) -D_BAS2PDB ../bas2pdb.cc -o bas2pdb.exe -lgcc -lstdc++
	$(DCC) $(DCFLAGS) -D_PDB2BAS ../bas2pdb.cc -o pdb2bas.exe -lgcc -lstdc++
	-cp bas2pdb.exe bin
	-cp pdb2bas.exe bin

mkpdb.exe: ../mkpdb.c
	$(DCC) $(DCFLAGS) -D_DOS ../mkpdb.c -o mkpdb.exe
	-cp mkpdb.exe bin

clean:
	-rm -f *.o *.exe

# DO NOT DELETE

