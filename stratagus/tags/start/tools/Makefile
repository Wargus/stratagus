##   ___________		     _________		      _____  __
##   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
##    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
##    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
##    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
##	  \/		    \/	   \/	     \/		   \/
##  ______________________                           ______________________
##			  T H E   W A R   B E G I N S
##	   FreeCraft - A free fantasy real time strategy game engine
##
##	Makefile	-	Makefile for tools. (Level 2)
##
##	(c) Copyright 1998,2000 by Lutz Sammer
##
##	$Id$
##

TOPDIR= ..
include $(TOPDIR)/Rules.make

EXECS=	gfx2png$(EXE) gfu2png$(EXE) img2png$(EXE) cur2png$(EXE) fnt2png$(EXE) \
	tile2png$(EXE) extract$(EXE) aledoc$(EXE) wartool$(EXE)

all::	aledoc$(EXE) wartool$(EXE)

old::	$(EXECS)

clean::
	$(RM) core gmon.out *.doc *.o

clobber:: clean
	$(RM) $(EXECS)

distclean:: clobber
	@echo

gfx2png$(EXE): gfx2png.c
	$(CC) $(CFLAGS) -DGFX -o $@ gfx2png.c $(TOOLLIBS)

gfu2png$(EXE): gfx2png.c
	$(CC) $(CFLAGS) -DGFU -o $@ gfx2png.c $(TOOLLIBS)

img2png$(EXE): gfx2png.c
	$(CC) $(CFLAGS) -DIMAGE -o $@ gfx2png.c $(TOOLLIBS)

cur2png$(EXE): gfx2png.c
	$(CC) $(CFLAGS) -DCUR -o $@ gfx2png.c $(TOOLLIBS)

fnt2png$(EXE): gfx2png.c
	$(CC) $(CFLAGS) -DFNT -o $@ gfx2png.c $(TOOLLIBS)

tile2png$(EXE): tile2png.c
	$(CC) $(CFLAGS) -DTILE -o $@ tile2png.c $(TOOLLIBS)

extract$(EXE): extract.c
	$(CC) $(CFLAGS) -o $@ extract.c $(TOOLLIBS)

aledoc$(EXE):	aledoc.c
	$(CC) $(CFLAGS) -o $@ $< $(TOOLLIBS)

wartool$(EXE): wartool.c
	$(CC) $(CFLAGS) -o $@ $< $(TOOLLIBS)

FILES= 	gfx2png.c tile2png.c extract.c \
	build.sh Makefile aledoc.c wartool.c build.bat

distlist:
	for i in $(FILES); do echo tools/$$i >>$(DISTLIST); done

ci:
	ci -l $(FILES)

lockver:
	$(LOCKVER) $(FILES)
