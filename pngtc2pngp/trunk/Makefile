CFLAGS=-O2 -std=c99 -Wall
LDFLAGS=-lpng -a

pngtc2pngp:pngtc2pngp.o cmdline.o

pngtc2pngp.o:pngtc2pngp.c pngtc2pngp.h
cmdline.o   :cmdline.c

.PHONY : clean
clean:
	rm -f pngtc2pngp pngtc2pngp.o cmdline.o
