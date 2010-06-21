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
OUTFILE ?= stratagus

-include $(TOPDIR)/$(RULESFILE)

OBJDIR ?= .

CROSSDIR ?= /usr/local/cross

INCLUDE_DIRS = src/include

MODULES = src/action src/ai src/beos src/editor src/game \
          src/guichan src/map src/network src/pathfinder \
          src/sound src/stratagus src/tolua src/ui src/unit \
          src/video src/particle

MODULES_ALL = $(MODULES)

MISC :=

HDRS :=
include $(patsubst %, %/Module.make, $(INCLUDE_DIRS))

SRC := 
include $(patsubst %, %/Module.make, $(MODULES))
OBJ := $(patsubst %.cpp, %.o, $(SRC))
OBJ := $(join $(addsuffix $(OBJDIR)/,$(dir $(OBJ))),$(notdir $(OBJ)))

ifneq ($(findstring -DUSE_WIN32, $(CPPFLAGS)),)
OBJ := $(OBJ) src/$(OBJDIR)/stratagusrc.o
endif

SRC_ALL = $(SRC)
OBJ_ALL = $(OBJ)

.SUFFIXES: .cpp .o

.PHONY:	make-objdir all-src

all:	all-src stratagus

make-objdir:
	@mkdir -p $(dir $(OBJ))

%.o: $(@D)../%.cpp $(RULESFILE)
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(subst $(OBJDIR)/../,,$<) -o $@

help:
	@-echo "make cycle			clean,depend,tags,all"
	@-echo "make install			install all files"
	@-echo "make uninstall			uninstall all files"
	@-echo "make clean			cleanup keep only executables"
	@-echo "make distclean			clean all files"
	@-echo "make doc			make source documention with doxygen"
	@-echo "make strip			strip stratagus and/or stratagus.exe"
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

doc::
	doxygen contrib/doxygen-stratagus.cfg

all-src: make-objdir $(OBJ)

stratagus: $(OBJ) 
	$(CXX) -o $(OUTFILE) $^ $(CXXFLAGS) $(LDFLAGS)

strip:
	@if [ -f $(OUTFILE) ]; then strip $(OUTFILE); fi
	@if [ -f $(OUTFILE).exe ]; then $(CROSSDIR)/i386-mingw32msvc/bin/strip $(OUTFILE).exe; fi

src/$(OBJDIR)/stratagusrc.o: src/stratagus.rc
	if [ ! -d src/$(OBJDIR) ]; then mkdir src/$(OBJDIR); fi
	cd src; $(WINDRES) -o $(OBJDIR)/stratagusrc.o stratagus.rc; cd ..

clean::
	$(RM) -r $(OBJ)
	for i in $(MODULES_ALL); do \
	$(RM) -r $$i/*.doc; done
	$(RM) core gmon.out cscope.out *.doc 
	@echo

distclean:	clean
	for i in $(MODULES_ALL); do \
	[ $(OBJDIR) = "." ] || $(RM) -r $$i/$(OBJDIR); \
	$(RM) $$i/.#* $$i/*~; done
	$(RM) $(OUTFILE) $(OUTFILE).exe gmon.sum .depend .#* *~ stderr.txt stdout.txt \
	srcdoc/* .depend Rules.make config.log config.status configure
	$(RM) Rules.make Rules.make.WIN32 aclocal.m4 config.h config.h.in stratagus-install*.exe
	$(RM) -r src/winobj/ src/guichan/sdl/winobj/ src/guichan/widgets/winobj/
	$(RM) -r autom4te.cache/
	@echo

depend:
	@echo -n >.depend
	@echo
	@for i in $(SRC) ; do \
	echo -e "\rMaking dependencies for $$i"; \
	$(CXX) -MT `dirname $$i`/$(OBJDIR)/`basename $$i | sed 's/\.cpp/\.o/g'` \
	-MM $(CPPFLAGS) $$i >>.depend; done
	@echo

ctags:
	ctags --C++-kinds=+px `find . -name '*.cpp' -o -name '*.c' -o -name '*.h'`

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

DOCS    = COPYING README doc/*.html doc/*.txt doc/scripts/*.html \
	  doc/scripts/*.py doc/graphics/*.html doc/graphics/*.gimp \
	  doc/graphics/*.png

PICS    = contrib/stratagus.ico contrib/poweredby.png

MISC    += contrib/doxygen-stratagus.cfg contrib/doxygen-header.html \
	  Rules.make.in configure.in configure config.h.in Makefile \
	  src/stratagus.rc stratagus.sln stratagus.vcproj \
	  autogen.sh SConstruct src/tolua/*.pkg \
	  contrib/intl/*.po contrib/intl/*.pot \
	  $(patsubst %, %/Module.make, $(MODULES_ALL)) \
	  $(patsubst %, %/Module.make, $(INCLUDE_DIRS))

mydate	?= $(shell date +%y%m%d)
distdir	= stratagus-$(mydate)

#why is this needed?
DISTLIST = distlist.tmp

distlist:
	@echo $(SRC_ALL) $(HDRS) > $(DISTLIST)

dist: distlist
	autoconf
	echo >>$(DISTLIST)
	echo $(PICS) >>$(DISTLIST)
	echo $(MISC) >>$(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	echo $(CONTRIB) >>$(DISTLIST)
	$(RM) -r $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for i in `cat $(DISTLIST)`; do echo $$i; done | while read j; do cp -a --parents $$j $(distdir); done
	chmod -R a+rX $(distdir)
	tar czhf $(distdir)-src.tar.gz $(distdir)
	echo "(c) 2006 The Stratagus Project" | \
	zip -zq9r $(distdir)-src.zip $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)
	ls -l $(distdir)-src.tar.gz $(distdir)-src.zip

bin-dist: all
	$(RM) $(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	echo stratagus >>$(DISTLIST)
	$(RM) -r $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for i in `cat $(DISTLIST)`; do echo $$i; done | while read j; do cp -a --parents $$j $(distdir); done
	chmod -R a+rX $(distdir)
	strip -s -R .comment $(distdir)/stratagus
	tar czhf stratagus-$(mydate)-linux.tar.gz $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)

bin-dist-gl: all
	$(RM) $(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	echo $(OUTFILE) >>$(DISTLIST)
	rm -rf $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for i in `cat $(DISTLIST)`; do echo $$i; done | while read j; do cp -a --parents $$j $(distdir); done
	chmod -R a+rX $(distdir)
	strip -s -R .comment $(distdir)/$(OUTFILE)
	tar czhf stratagus-$(mydate)-linux-gl.tar.gz $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)

deb-dist:
	debuild binary

#----------------------------------------------------------------------------

win32-bin-dist2: win32
	@$(RM) $(DISTLIST)
	@echo $(DOCS) >>$(DISTLIST)
	@echo $(OUTFILE).exe >>$(DISTLIST)
	@rm -rf $(distdir)
	@mkdir $(distdir)
	@chmod 777 $(distdir)
	@for i in `cat $(DISTLIST)`; do echo $$i; done | while read j; do cp -a --parents $$j $(distdir); done
	@chmod -R a+rX $(distdir)
	@strip -s -R .comment $(distdir)/$(OUTFILE).exe
	@echo "(c) 2003 by the Stratagus Project http://Stratagus.Org" | \
	zip -zq9r stratagus-$(mydate)-win32bin.zip $(distdir)
	@$(RM) $(DISTLIST)
	@$(RM) -r $(distdir)
	ls -l stratagus-$(mydate)-win32bin.zip

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
	$(MAKE) configuregl
	$(MAKE) depend
	$(MAKE) bin-dist-gl
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
	install -m 755 $(OUTFILE) $(PREFIX)/bin
	@echo installation of stratagus complete

uninstall:
	@echo uninstalling stratagus and stratagus tools
	rm $(PREFIX)/bin/$(OUTFILE)
	@echo uninstallation of stratagus complete

