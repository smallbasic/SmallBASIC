# Linux settings

CINC=-I..
CFLAGS=-Wall -fPIC $(CINC) -D_UnixOS

module_dir=/usr/lib/sbasic/modules

all: $(module_dir)/example1.so

$(module_dir)/example1.so: example1.c
	-mkdir -p $(module_dir)
	gcc $(CFLAGS) -c example1.c -o example1.o
	gcc -shared -Wl,-soname,example1.so -o example1.so example1.o 
	mv example1.so $(module_dir)
	ldconfig -n $(module_dir)

clean:
	-rm -f *.so *.o $(module_dir)/*.so


