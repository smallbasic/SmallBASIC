# DOS djgpp - unix cross-compile

all: sbrgrx.exe bas2pdb.exe mkpdb.exe

DCC = gcc-dos
DCPPFLAGS = -O2 -Wall -D_DOS -I.. -I.
DCFLAGS   = $(DCPPFLAGS)
CLIBS = -lm -lgcc -lreadline -lhistory
sources=\
	brun.c ceval.c eval.c str.c match.c fmt.c kw.c sberr.c \
	mem.c panic.c blib.c bc.c file.c blib_db.c blib_math.c \
	var.c proc.c blib_func.c blib_sound.c inet.c vmt.c \
	blib_graph.c scan.c circle.c ffill.c pfill.c g_bmp.c \
	extlib.c blib_ui.c plot.c tasks.c units.c \
    fs_stream.c fs_serial.c fs_memo.c fs_pdoc.c fs_socket_client.c fs_irda.c \
	unx_memmgr.c dev_term.c g_line.c decomp.c console_main.c \
	interactive_mode.c help_subsys.c geom.c

com_sources = $(addprefix ../, $(sources))
com_objects = $(addsuffix .o,  $(basename $(sources)))

# Platform specific sources
#dos_src  =
#dos_obj  = $(addsuffix .o, $(basename $(dos_src)))
dos_obj += $(com_objects)

%.o: ../%.c
	$(DCC) $(DCFLAGS) -c $<

%.o: %.c
	$(DCC) $(DCFLAGS) -c $<

#   Add-on sound drivers:
#       drv_adlib.c: ADLIB or compatible FM chip
#       drv_djspk.c: PC speaker (djgpp only)
#
#	other:
#		dos/svasync: serial I/O lib

# unstable
sbrdos.exe: $(dos_obj) dev_dos.c dev_genfb.c $(DOSDRVS) data/rom16.c  $(help_files)
	$(DCC) $(DCFLAGS)-DDRVSOUND dev_dos.c dev_genfb.c $(DOSDRVS) $(dos_obj) -o sbrdos.exe $(CLIBS)
	-mv sbrdos.exe sbasic.exe
	-cp sbasic.exe bin

# stable
sbrgrx.exe: $(dos_obj) dev_grx.c $(DOSDRVS) data/rom16.c $(help_files)
	$(DCC) $(DCFLAGS) -DDRVSOUND dev_grx.c $(dos_obj) $(DOSDRVS) -o sbrgrx.exe $(CLIBS) -lgrx20
	-mv sbrgrx.exe sbasic.exe
	-cp sbasic.exe bin

# DOS
DOSDRVS=../device.c dev_adlib.c svasync/isr.s svasync/svasync.c

bas2pdb.exe: ../bas2pdb.cc
	$(DCC) $(DCFLAGS) -D_BAS2PDB ../bas2pdb.cc -o bas2pdb.exe
	$(DCC) $(DCFLAGS) -D_PDB2BAS ../bas2pdb.cc -o pdb2bas.exe
	-cp bas2pdb.exe bin
	-cp pdb2bas.exe bin

mkpdb.exe: ../mkpdb.c
	$(DCC) $(DCFLAGS) -D_DOS ../mkpdb.c -o mkpdb.exe
	-cp mkpdb.exe bin

clean:
	-rm -f *.o *.exe

# DO NOT DELETE

