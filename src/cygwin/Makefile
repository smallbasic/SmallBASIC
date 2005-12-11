# SmallBASIC for Cygwin
#
#
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

all: sbasic.exe
debug:sbasic.exe

debug: DCFLAGS = -g -O1 -Wall -D_UnixOS -D_CygWin -I..
DCFLAGS   = -O2 -Os -Wall -D_UnixOS -D_CygWin -I..
CLIBS = -lm -lgcc -lreadline -ltermcap  -s
debug: CLIBS = -lm -lgcc -lreadline -ltermcap

OBJDIR = ./objs/

SOURCE=									        \
    bc.c										\
    blib.c										\
    blib_db.c									\
    blib_func.c									\
    blib_graph.c								\
    blib_math.c 								\
    blib_sound.c								\
    blib_ui.c									\
    brun.c										\
    ceval.c										\
    console_main.c                              \
    decomp.c  								    \
    dev_null.c									\
    dev_term.c									\
    device.c									\
    eval.c										\
    extlib.c									\
    ffill.c										\
    file.c										\
    fmt.c										\
    fs_irda.c 									\
    fs_memo.c									\
    fs_pdoc.c									\
    fs_serial.c									\
    fs_socket_client.c							\
    fs_stream.c									\
    g_line.c									\
    geom.c										\
    inet.c										\
    kw.c										\
    match.c										\
    mem.c										\
    panic.c										\
    pfill.c 									\
    plot.c										\
    proc.c										\
    sberr.c 									\
    scan.c 								        \
    circle.c								    \
    str.c										\
    tasks.c										\
    units.c 									\
    unx_memmgr.c								\
    var.c										\
    vmt.c 										\

OBJS += $(addprefix ${OBJDIR}, $(addsuffix .o, $(basename $(SOURCE))))

sbasic.exe: ${OBJDIR} $(OBJS)
	gcc $(DCFLAGS) $(OBJS) -o sbasic.exe $(CLIBS)
	-cp $@ bin

bas2pdb.exe: ../bas2pdb.cc
	gcc $(DCFLAGS) -D_BAS2PDB ../bas2pdb.cc -o bas2pdb.exe -lgcc -lstdc++
	gcc $(DCFLAGS) -D_PDB2BAS ../bas2pdb.cc -o pdb2bas.exe -lgcc -lstdc++
	-cp bas2pdb.exe bin
	-cp pdb2bas.exe bin

mkpdb.exe: ../mkpdb.c
	$(DCC) $(DCFLAGS) -D_DOS ../mkpdb.c -o mkpdb.exe
	-cp mkpdb.exe bin

clean:
	-rm -f *.o *.exe

${OBJDIR}:
	mkdir -p $@

${OBJDIR}%.o: ../%.c
	gcc $(DCFLAGS) -o $@ -c $<

# DO NOT DELETE

