all: libsv.a term.exe

GCC=gcc-dos -O2 -Wall -g
AR=ar-dos
AS=as-dos

libsv.a : svasync.o isr.o
	$(AR) -rs libsv.a svasync.o isr.o

svasync.o : svasync.c
	$(GCC) -c svasync.c

isr.o : isr.s
	$(GCC) -c isr.s

term.exe : term.o libsv.a
	$(GCC) term.o libsv.a -o term.exe -lpc

term.o : term.c
	$(GCC) -c term.c

clean :
	-rm *.o libsv.a term.exe
