# PalmOS/prc-tools (cross-compile from Unix)

all: sbpad-arm.prc

PCC = arm-palmos-gcc
AS  = arm-palmos-as

IMGS=../palmos/images/sbpad-small.bmp \
	../palmos/images/sbpad1.bmp ../palmos/images/sbpad2.bmp ../palmos/images/sbpad4.bmp ../palmos/images/sbpad8.bmp \
	../palmos/images/sbgo-small.bmp ../palmos/images/sbgo.bmp

# remark the following line if you have not installed the Sony's SDK
#SONYSDK=-DSONY_CLIE
#WARNS=-w -Wimplicit-function-declaration -Wtraditional -Wbad-function-cast -Wpointer-arith -Wconversion -Wstrict-prototypes -Wredundant-decls -Wmissing-declarations -Wmissing-prototypes
#WARNS=-Wall
WARNS=-w
SDK=-palmos5
PIMGS=../palmos/images/sbpad1.bmp ../palmos/images/sbpad2.bmp ../palmos/images/sbpad4.bmp ../palmos/images/sbpad8.bmp

PCFLAGS =-I.. -I. -I../palmos/ -Os -D_ArmPalmOS -D_PalmOS $(SDK) $(SONYSDK) $(WARNS)
PLIBS=-lgcc

PILRC = pilrc -q
POBJRES = arm-palmos-obj-res
POBJDMP = arm-palmos-objdump
PNM = arm-palmos-nm
PAS = arm-palmos-as
BUILDPRC = build-prc
PILOTXFER = pilot-xfer

FONTS   = ../palmos/data/hkfont.def
FONTRES = ../palmos/data/pFNT0708.bin

# Common sources
sources=\
	brun.c ceval.c eval.c str.c match.c fmt.c kw.c sberr.c \
	mem.c panic.c blib.c bc.c file.c blib_db.c blib_math.c \
	var.c proc.c blib_func.c blib_sound.c inet.c vmt.c \
	blib_graph.c scan.c circle.c ffill.c pfill.c g_bmp.c \
	extlib.c blib_ui.c plot.c device.c \
    fs_stream.c fs_serial.c fs_memo.c fs_pdoc.c fs_socket_client.c fs_irda.c

com_sources = $(addprefix ../, $(sources))
com_objects = $(addsuffix .o,  $(basename $(sources)))

# Platform specific sources
#palm_src  = $(addprefix ../palmos/, ui.c sbpad.c mathlib.c dev_palm.c palmfs.c lopen_to_palmos_bridge.c)
palm_src  = ui.c sbpad.c mathlib.c dev_palm.c palmfs.c lopen_to_palmos_bridge.c
palm_obj  = $(addsuffix .o, $(basename $(palm_src)))
palm_obj += $(com_objects)

%.o: ../palmos/%.c
	$(PCC) $(PCFLAGS) -c $<

%.o: ../%.c
	$(PCC) $(PCFLAGS) -c $<

%.o: %.c
	$(PCC) $(PCFLAGS) -c $<

$(FONTRES): ../palmos/data/hkfont.def
	cp ../palmos/data/hkfont.def $(FONTRES)

sbpad: $(palm_obj)
	$(PCC) -Wl,-Map,sbpad.map $(PCFLAGS) $(palm_obj) -o sbpad $(PLIBS)
	$(PNM) -u sbpad

#multigen sbpad.def
#$(PAS) -o sbpad-sections.o sbpad-sections.s
#$(PCC) -Wl,-Map,sbpad.map $(PCFLAGS) $(palm_obj) sbpad-sections.o sbpad-sections.ld -o sbpad $(PLIBS)
#$(PNM) -u sbpad


sbpad-arm.prc: sbpad sbpad.def ../palmos/sbpad.rcp $(IMGS) $(FONTRES) ../palmos/data/sm9.pfn ../palmos/data/DOS5x9.pfn ../palmos/data/DOS4x9.pfn $(PIMGS)
	$(PILRC) sbpad.rcp 
	$(BUILDPRC) sbpad.def sbpad *.bin $(FONTRES) -o sbpad-arm.prc
	$(POBJDMP) sbpad -h
	ls sbpad-arm.prc -la
	-cp sbpad-arm.prc bin

palm: sbpad-arm.prc

picons: sbpad-arm.prc build_sbgo_icons.c ../palmos/images/sbgo.bmp ../palmos/images/sbgo-small.bmp
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

send: sbpad-arm.prc
	$(PILOTXFER) -i sbpad-arm.prc

clean:
	-rm -f sbpad-arm.prc sbpad *.o sbpad.map ../samples/*.pdb
	-rm -f diff.lst *~ *.bak *.bin
	-rm -f build_sbgo_icons

#	Install
install: all
	cp bas2pdb $(INSTALLBIN)
	cp pdb2bas $(INSTALLBIN)
	cp mkpdb $(INSTALLBIN)

# DO NOT DELETE
