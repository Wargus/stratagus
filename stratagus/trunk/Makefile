##       _________ __                 __                               
##      /   _____//  |_____________ _/  |______     ____  __ __  ______
##      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
##      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \
##     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
##             \/                  \/          \//_____/            \/ 
##  ______________________                           ______________________
##			  T H E   W A R   B E G I N S
##	   Stratagus - A free fantasy real time strategy game engine
##
##	Makefile	-	The make file.
##
##	(c) Copyright 1998-2003 by Lutz Sammer and Nehal Mistry
##
##	Stratagus is free software; you can redistribute it and/or modify
##	it under the terms of the GNU General Public License as published
##	by the Free Software Foundation; only version 2 of the License.
##
##	Stratagus is distributed in the hope that it will be useful,
##	but WITHOUT ANY WARRANTY; without even the implied warranty of
##	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##	GNU General Public License for more details.
##
##	$Id$
##

TOPDIR=	.

RULESFILE ?= Rules.make
WINRULESFILE = Rules.make.WIN32

-include $(TOPDIR)/$(RULESFILE)

OBJDIR ?= .

CROSSDIR = /usr/local/cross

INCLUDE_DIRS = src/include src/movie/vp31/include

MODULES = src/action src/ai src/beos src/stratagus src/editor src/game src/map \
          src/missile src/movie src/movie/vp31 src/network src/pathfinder src/sound src/ui src/unit \
          src/video etlib

MODULES_ALL = $(MODULES)

MISC :=

HDRS :=
include $(patsubst %, %/Module.make, $(INCLUDE_DIRS))

SRC := 
include $(patsubst %, %/Module.make, $(MODULES))
OBJ := $(patsubst %.c, %.o, $(SRC))
OBJ := $(join $(addsuffix $(OBJDIR)/,$(dir $(OBJ))),$(notdir $(OBJ)))

SRC_ALL = $(SRC)
OBJ_ALL = $(OBJ)

.SUFFIXES: .c .o

.PHONY:	make-objdir all-src

all:	all-src stratagus$(EXE)

make-objdir:
	@for i in $(MODULES); do \
	if [ ! -d $$i/$(OBJDIR) ]; then mkdir $$i/$(OBJDIR); fi; done

%.o: $(@D)../%.c
	$(CC) -c $(CFLAGS) $< -o $@

help:
	@-echo "make cycle			clean,depend,tags,all"
	@-echo "make install			install all files"
	@-echo "make uninstall			uninstall all files"
	@-echo "make run			create and run"
	@-echo "make runp			create and run with profiler"
	@-echo "make clean			cleanup keep only executables"
	@-echo "make distclean			clean all files"
	@-echo "make doc			make source documention with doxygen"
	@-echo "make doc++			make source documention with doc++"
	@-echo "make lockver NAME="version"	label current version with symbolic name"
	@-echo "make strip			strip stratagus and/or stratagus.exe"
	@-echo "make tags			create ctags"
	@-echo "make depend			create dependencies"
	@-echo "make dist			create distribution"
	@-echo "make win32new			(CROSS-COMPILER ONLY) start new win32"
	@-echo "make win32			(CROSS-COMPILER ONLY) build the win32 version"
	@-echo "make win32distclean		(CROSS-COMPILER ONLY) clean all files of win32"
	@-echo "make release			release it"

cycle::
	@$(MAKE) clean
	@$(MAKE) depend
	@$(MAKE) tags
	@$(MAKE) all

run::
	@$(MAKE) && ./stratagus

runp::
	@$(MAKE) && ./stratagus && if [ -e gmon.sum ]; then \
		gprof -s stratagus gmon.out gmon.sum; \
	    else mv gmon.out gmon.sum; fi

doc::
	doxygen contrib/doxygen-stratagus.cfg

doc++::
	@$(MAKE) -C src RULESFILE=$(RULESFILE) doc
	@if [ ! -d srcdoc ]; then mkdir srcdoc; fi
	@$(DOCPP) -v -H -A -a -b -c -j -d srcdoc `find . -name "*.doc" -print`

all-src: make-objdir $(OBJ)

# UNIX-TARGET
stratagus: $(OBJ) 
	$(CCLD) -o stratagus $^ $(STRATAGUS_LIBS) -I. $(CFLAGS)

# WIN32-TARGET
stratagus.exe:	$(OBJ) etlib/$(OBJDIR)/getopt.$(OE) \
	    src/$(OBJDIR)/stratagusrc.$(OE)
	$(CCLD) -o stratagus$(EXE) $^ -lSDLmain $(STRATAGUS_LIBS) -I. $(CFLAGS)

strip:
	@if [ -f stratagus ]; then strip stratagus; fi
	@if [ -f stratagus.exe ]; then $(CROSSDIR)/i386-mingw32msvc/bin/strip stratagus.exe; fi

src/$(OBJDIR)/stratagusrc.$(OE): src/stratagus.rc
	if [ ! -d src/$(OBJDIR) ]; then mkdir src/$(OBJDIR); fi
	windres --include-dir contrib -o src/$(OBJDIR)/stratagusrc.$(OE) src/stratagus.rc

echo::
	@-echo CFLAGS: $(CFLAGS)
	@-echo LIBS: $(STRATAGUS_LIBS)

clean::
	for i in $(MODULES_ALL); do \
	$(RM) -rf $$i/$(OBJDIR)/*.o $$i/*.doc; done
	$(RM) core gmon.out cscope.out *.doc etlib/$(OBJDIR)/*.$(OE)
	@echo

distclean:	clean
	for i in $(MODULES_ALL); do \
	[ $(OBJDIR) == "." ] || $(RM) -rf $$i/$(OBJDIR); \
	$(RM) $$i/.#* $$i/*~; done
	$(RM) stratagus$(EXE) gmon.sum .depend .#* *~ stderr.txt stdout.txt \
	srcdoc/* .depend Rules.make config.log config.status configure
	$(RM) -rf autom4te.cache/
	@echo

configure:
	autoconf
	./configure

lockver:
	$(LOCKVER) Makefile $(RULESFILE) .indent.pro \
	contrib/doxygen-stratagus.cfg \
	$(CCLS) $(DOCS) $(SRC_ALL) src/beos/beos.cpp $(HDRS) Makefile
	for i in $(MODULES_ALL); do $(LOCKVER) Module.make; done

tags:
	for i in $(SRC); do \
	ctags --c-types=defmpstuvx -a -f tags `pwd`/$$i ; done

depend:
	@echo -n >.depend
	@echo
	@for i in $(SRC) ; do\
	echo -e "\rMaking dependencies for $$i";\
	echo -n `dirname $$i`/$(OBJDIR)/ >> .depend;\
	$(CC) -MM $(IFLAGS) $(DFLAGS) $(CFLAGS) $$i >>.depend ; done
	@echo

##############################################################################
#
#	include dependency files, if they exist
#

$(OBJ):                $(RULESFILE)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

##############################################################################
#	Distributions
##############################################################################

DOCS    = README doc/*.html doc/*.txt doc/ccl/*.html doc/ccl/*.py \
	  doc/graphics/*.html doc/graphics/*.gimp doc/graphics/*.png

PICS    = contrib/stratagus.ico

CONTRIB	= contrib/macosx.tgz

MISC    += Makefile Rules.make.orig \
	  contrib/doxygen-stratagus.cfg contrib/doxygen-header.html \
	  Rules.make.in configure.in configure \
	  src/stratagus.rc stratagus.dsw stratagus.dsp \
	  $(patsubst %, %/Module.make, $(MODULES)) \
	  $(patsubst %, %/Module.make, $(INCLUDE_DIRS))

mydate	= $(shell date +%y%m%d)
distdir	= stratagus-$(mydate)

#why is this needed?
DISTLIST = distlist.tmp

distlist:
	@echo $(SRC_ALL) $(HDRS) src/beos/beos.cpp > $(DISTLIST)

dist: distlist
	autoconf
	echo >>$(DISTLIST)
	echo $(PICS) >>$(DISTLIST)
	echo $(MISC) >>$(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	echo $(CONTRIB) >>$(DISTLIST)
	rm -rf $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir)
	chmod -R a+rX $(distdir)
	tar czhf $(distdir)-src.tar.gz $(distdir)
	echo "(c) 2004 The Stratagus Project" | \
	zip -zq9r $(distdir)-src.zip $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)
	du -h $(distdir)-src.tar.gz $(distdir)-src.zip

bin-dist: all
	$(RM) $(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	echo stratagus$(EXE) >>$(DISTLIST)
	rm -rf $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir)
	chmod -R a+rX $(distdir)
	strip -s -R .comment $(distdir)/stratagus$(EXE)
	tar czhf stratagus-$(mydate)-linux.tar.gz $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)

#----------------------------------------------------------------------------

win32-bin-dist2: win32
	@$(RM) $(DISTLIST)
	@echo $(DOCS) >>$(DISTLIST)
	@echo stratagus$(EXE) >>$(DISTLIST)
	@rm -rf $(distdir)
	@mkdir $(distdir)
	@chmod 777 $(distdir)
	@for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir)
	@chmod -R a+rX $(distdir)
	@strip -s -R .comment $(distdir)/stratagus$(EXE)
	@echo "(c) 2003 by the Stratagus Project http://Stratagus.Org" | \
	zip -zq9r stratagus-$(mydate)-win32bin.zip $(distdir)
	@$(RM) $(DISTLIST)
	@$(RM) -r $(distdir)
	du -h stratagus-$(mydate)-win32bin.zip

win32-bin-dist: win32
	@export PATH=$(CROSSDIR)/i386-mingw32msvc/bin:$(CROSSDIR)/bin:$$PATH; \
	$(MAKE) RULESFILE=$(WINRULESFILE) $(WIN32) win32-bin-dist2

#----------------------------------------------------------------------------

difffile=	stratagus-`date +%y%m%d`.diff
diff:
	@$(RM) $(difffile)
	@$(RM) $(DISTLIST)
	$(MAKE) -C src RULESFILE=$(RULESFILE) distlist
	echo $(MISC) >>$(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	rcsdiff -u `cat $(DISTLIST)` > $(difffile)

release:
	$(MAKE) distclean
	$(MAKE) configure
	$(MAKE) depend
	$(MAKE) bin-dist
	$(MAKE) win32new
	$(MAKE) win32configure
	$(MAKE) win32-bin-dist
	$(MAKE) win32distclean
	$(MAKE) dist

##############################################################################
#	WIN32 Crosscompiler Build
##############################################################################

WIN32=	\
    VIDEOLIB='-L$(CROSSDIR)/i386-mingw32msvc/lib $(SDLLIB) -lwsock32 -lws2_32' \
    RULESFILE=$(WINRULESFILE)

win32new:
	@$(MAKE) RULESFILE=$(WINRULESFILE) distclean
	export PATH=$(CROSSDIR)/i386-mingw32msvc/bin:$(CROSSDIR)/bin:$$PATH; \

win32configure:
	autoconf
	export PATH=$(CROSSDIR)/bin:$(CROSSDIR)/i386-mingw32msvc/bin:$$PATH; \
	./configure --enable-win32 --host=i386-mingw32msvc --build=i386-linux \
	--with-lua=/usr/local/cross --enable-static

win32_2:
	$(MAKE) $(WIN32) all

win32:
	export PATH=$(CROSSDIR)/i386-mingw32msvc/bin:$(CROSSDIR)/bin:$$PATH; \
	$(MAKE) $(WIN32) win32_2

win32depend:
	export PATH=$(CROSSDIR)/i386-mingw32msvc/bin:$(CROSSDIR)/bin:$$PATH; \
	$(MAKE) $(WIN32) depend

win32distclean:
	export PATH=$(CROSSDIR)/i386-mingw32msvc/bin:$(CROSSDIR)/bin:$$PATH; \
	$(MAKE) $(WIN32) distclean

##############################################################################
#	INSTALL/UNINSTALL
##############################################################################

install:	all install-stratagus

install-stratagus:
	install -m 755 stratagus $(PREFIX)/bin
	@echo installation of stratagus complete

uninstall:
	@echo uninstalling stratagus and stratagus tools
	rm $(PREFIX)/bin/stratagus
	@echo uninstallation of stratagus complete

