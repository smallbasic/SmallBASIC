
# Makefile

CFLAGS  = -O3 -m486 -funsigned-char
LDFLAGS =
DEBUG   = -Wall -W -Werror -g
INC     = -I/usr/include/rhtvision
LIBS    = -lncurses -lgpm
TVLIB   = -lrhtv
BINDIR  = /usr/local/bin

all: tvhc

tvhc: tvhc.cc tvhc.h helpbase.cc
	g++ $(CFLAGS) $(LDFLAGS) $(INC) $(DEBUG) tvhc.cc helpbase.cc $(LIBS) $(TVLIB) -o tvhc

clean:
	rm -f core *.o tvhc

install:
	install --strip --group=bin --owner=root --mode=0755 tvhc $(BINDIR)

# eof

