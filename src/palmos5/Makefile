# PalmOS/prc-tools (cross-compile from Unix)

all: sbpad.prc

PCC = m68k-palmos-gcc
AS  = m68k-palmos-as

IMGS=images/sbpad-small.bmp \
	images/sbpad1.bmp images/sbpad2.bmp images/sbpad4.bmp images/sbpad8.bmp \
	images/sbgo-small.bmp images/sbgo.bmp

# remark the following line if you have not installed the Sony's SDK
#SONYSDK=-DSONY_CLIE
#WARNS=-w -Wimplicit-function-declaration -Wtraditional -Wbad-function-cast -Wpointer-arith -Wconversion -Wstrict-prototypes -Wredundant-decls -Wmissing-declarations -Wmissing-prototypes
WARNS=-Wall
#SDK=-palmos3.5
PIMGS=images/sbpad1.bmp images/sbpad2.bmp images/sbpad4.bmp images/sbpad8.bmp

PCFLAGS =-I.. -I. -Os -D_PalmOS $(SDK) $(SONYSDK) $(WARNS)
PLIBS=-lgcc

PILRC = pilrc -q
POBJRES = m68k-palmos-obj-res
POBJDMP = m68k-palmos-objdump
PNM = m68k-palmos-nm
PAS = m68k-palmos-as
BUILDPRC = build-prc
PILOTXFER = pilot-xfer
MGN=m68k-palmos-multigen 

FONTS   = data/hkfont.def
FONTRES = data/pFNT0708.bin

# Common sources
sources=\
	brun.c ceval.c eval.c str.c match.c fmt.c kw.c sberr.c \
	mem.c panic.c blib.c bc.c file.c blib_db.c blib_math.c \
	var.c proc.c blib_func.c blib_sound.c inet.c vmt.c \
	blib_graph.c scan.c circle.c ffill.c pfill.c g_bmp.c \
	extlib.c blib_ui.c plot.c device.c tasks.c units.c geom.c \
    fs_stream.c fs_serial.c fs_memo.c fs_pdoc.c fs_socket_client.c fs_irda.c

com_sources = $(addprefix ../, $(sources))
com_objects = $(addsuffix .o,  $(basename $(sources)))

# Platform specific sources
palm_src  = ui.c sbpad.c mathlib.c dev_palm.c palmfs.c lopen_to_palmos_bridge.c syslib.c
palm_obj  = $(addsuffix .o, $(basename $(palm_src)))
palm_obj += $(com_objects)

%.o: ../%.c
	$(PCC) $(PCFLAGS) -c $<

%.o: %.c
	$(PCC) $(PCFLAGS) -c $<

$(FONTRES): data/hkfont.def
	cp data/hkfont.def $(FONTRES)

sbpad: $(palm_obj)
	$(MGN) sbpad.def
	$(PAS) -o sbpad-sections.o sbpad-sections.s
	$(PCC) -Wl,-Map,sbpad.map $(PCFLAGS) $(palm_obj) sbpad-sections.o sbpad-sections.ld -o sbpad $(PLIBS)
	$(PNM) -u sbpad

sbpad.prc: sbpad sbpad.def sbpad.rcp $(IMGS) $(FONTRES) data/sm9.pfn data/DOS5x9.pfn data/DOS4x9.pfn $(PIMGS)
	$(PILRC) sbpad.rcp
	$(BUILDPRC) sbpad.def sbpad *.bin $(FONTRES) -o sbpad.prc
	$(POBJDMP) sbpad -h
	ls sbpad.prc -la
	-cp sbpad.prc bin

palm: sbpad.prc

picons: sbpad.prc build_sbgo_icons.c images/sbgo.bmp images/sbgo-small.bmp
	gcc build_sbgo_icons.c -o build_sbgo_icons
	build_sbgo_icons
	touch sbpad.c
	-@printf "\n\033[7m * %s * \033[0m\n\n" "run 'make palm', again"

##################
#	Utilities

bas2pdb: ../bas2pdb.cc
	$(UCC) $(CFLAGS) -D_BAS2PDB ../bas2pdb.cc -o bas2pdb
	$(UCC) $(CFLAGS) -D_PDB2BAS ../bas2pdb.cc -o pdb2bas

mkpdb: mkpdb.c
	$(UCC) $(CFLAGS) ../mkpdb.c -o mkpdb

dumpvmt: dumpvmt.c vmt.c mem.c unx_memmgr.c panic.c
	$(UCC) $(CFLAGS) -D_UnixOS dumpvmt.c vmt.c mem.c unx_memmgr.c panic.c -o dumpvmt 

send: sbpad.prc
	$(PILOTXFER) -i sbpad.prc

clean:
	-rm -f sbpad.prc sbpad *.o sbpad.map ../samples/*.pdb
	-rm -f diff.lst *~ *.bak *.bin
	-rm -f build_sbgo_icons

#	Install
install: all
	cp bas2pdb $(INSTALLBIN)
	cp pdb2bas $(INSTALLBIN)
	cp mkpdb $(INSTALLBIN)

# DO NOT DELETE
