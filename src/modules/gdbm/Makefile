# Linux settings

MODNAME=gdbm
MODLIBS=-lgdbm
MODIDIR=/usr/lib/sbasic/modules

CINC=-I../..
CFLAGS=-Wall -fPIC $(CINC) -D_UnixOS -DLNX_EXTLIB

all: $(MODIDIR)/$(MODNAME).so

$(MODIDIR)/$(MODNAME).so: $(MODNAME).c
	-mkdir -p $(MODIDIR)
	gcc $(CFLAGS) -c $(MODNAME).c -o $(MODNAME).o
	gcc -shared -Wl,-soname,$(MODNAME).so -o $(MODNAME).so $(MODNAME).o $(MODLIBS)
	mv $(MODNAME).so $(MODIDIR)
	ldconfig -n $(MODIDIR)

clean:
	-rm -f *.so *.o $(MODIDIR)/$(MODNAME).so


