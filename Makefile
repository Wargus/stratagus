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

INCLUDE_DIRS = src/include src/movie/vp31/include etlib

MODULES = src/action src/ai src/beos src/stratagus src/editor src/stratagus src/game src/libmodplug src/map \
          src/missile src/movie src/movie/vp31 src/network src/pathfinder src/sound src/ui src/unit \
          src/video etlib

MODULES_TOOLS = tools

MODULES_ALL = $(MODULES) $(MODULES_TOOLS)

MISC :=

HDRS :=
include $(patsubst %, %/Module.make, $(INCLUDE_DIRS))

SRC := 
include $(patsubst %, %/Module.make, $(MODULES))
OBJ := $(patsubst %.c, %.o, $(SRC))
OBJ := $(join $(addsuffix $(OBJDIR)/,$(dir $(OBJ))),$(notdir $(OBJ)))

SRC_TOOLS := 
include $(patsubst %, %/Module.make, $(MODULES_TOOLS))
OBJ_TOOLS := $(patsubst %.c, %.o, $(SRC_TOOLS))
OBJ_TOOLS := $(join $(addsuffix $(OBJDIR)/,$(dir $(OBJ_TOOLS))),$(notdir $(OBJ_TOOLS)))

SRC_ALL = $(SRC) $(SRC_TOOLS)
OBJ_ALL = $(OBJ) $(OBJ_TOOLS)

.SUFFIXES: .c .o

.PHONY:	make-objdir all-src

all:	all-src stratagus$(EXE) tools

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
	touch Rules.make
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
	for i in $(SRC) $(SRC_TOOLS); do \
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

$(OBJ):		$(RULESFILE)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

##############################################################################
#	TOOLS
##############################################################################

tools: tools/aledoc$(EXE)

tools/aledoc$(EXE): tools/aledoc.c
	$(CC) $(CFLAGS) -o $@ $< $(TOOLLIBS)

##############################################################################
#	Distributions
##############################################################################

DOCS    = README doc/index.html doc/install.html \
	  doc/media.html doc/datadir.html doc/README-SDL.txt\
	  doc/faq.html doc/ChangeLog.html doc/todo.html \
	  doc/development.html doc/gpl.html \
	  doc/ccl/ai.html doc/ccl/ccl.html doc/ccl/config.html \
	  doc/ccl/icon.html doc/ccl/tileset.html doc/ccl/unittype.html \
	  doc/ccl/research.html doc/graphic/* \
	  doc/trigger.txt doc/vp32_opensource_license_9-6-01.txt \
	  debian/stratagus.6 doc/ccl/ccl-index.html doc/ccl/game.html \
	  doc/ccl/icon.html doc/ccl/sound.html doc/ccl/triggers.html \
	  doc/ccl/ui.html

PICS    = contrib/stratagus.png contrib/stratagus.ico

PUDS	= contrib/puds/single/*.txt contrib/puds/single/*.pud.gz \
	  contrib/puds/multi/*.txt contrib/puds/multi/*.pud.gz

CCLS	= data/ccl/units.ccl data/ccl/human/units.ccl data/ccl/orc/units.ccl \
	  data/ccl/constructions.ccl data/ccl/human/constructions.ccl \
	  data/ccl/orc/constructions.ccl \
	  data/ccl/missiles.ccl data/ccl/icons.ccl \
	  data/ccl/sound.ccl data/ccl/stratagus.ccl \
	  data/ccl/ui.ccl data/ccl/human/ui.ccl data/ccl/orc/ui.ccl \
	  data/ccl/upgrade.ccl data/ccl/human/upgrade.ccl \
	  data/ccl/orc/upgrade.ccl \
	  data/ccl/buttons.ccl data/ccl/human/buttons.ccl \
	  data/ccl/orc/buttons.ccl \
	  data/ccl/fonts.ccl data/ccl/ai.ccl \
	  data/ccl/tilesets.ccl data/ccl/tilesets/summer.ccl \
	  data/ccl/tilesets/winter.ccl \
	  data/ccl/tilesets/wasteland.ccl data/ccl/tilesets/swamp.ccl \
	  data/ccl/campaigns.ccl data/ccl/credits.ccl \
	  data/ccl/human/campaign1.ccl data/ccl/human/campaign2.ccl \
	  data/ccl/orc/campaign1.ccl data/ccl/orc/campaign2.ccl \
	  data/ccl/anim.ccl data/ccl/wc2.ccl data/ccl/ranks.ccl \
	  data/ccl/tips.ccl data/ccl/menus.ccl data/ccl/keystrokes.ccl \
	  data/ccl/spells.ccl \
	  data/ccl/editor.ccl # data/campaigns/*/*.cm

CONTRIB	= contrib/cross.png contrib/red_cross.png \
	  contrib/health.png contrib/mana.png \
	  contrib/health2.png contrib/mana2.png \
	  contrib/ore,stone,coal.png contrib/food.png contrib/score.png \
	  contrib/music/toccata.mod.gz \
	  contrib/Stratagus-beos.proj.gz \
	  contrib/msvc.zip contrib/macosx.tgz contrib/stdint.h \
	  contrib/campaigns/*/*.cm

MISC    += Makefile Rules.make.orig setup \
	  contrib/doxygen-stratagus.cfg contrib/doxygen-header.html \
	  .indent.pro Rules.make.in configure.in configure \
	  $(CONTRIB) \
	  \
	  src/stratagus.rc data/default.cm

mydate	= $(shell date +%y%m%d)
distdir	= stratagus-$(mydate)

distlist:
	@echo $(SRC_ALL) $(HDRS) src/beos/beos.cpp > $(DISTLIST)
	for i in $(MODULES_ALL); do echo $$i/Module.make >> $(DISTLIST); done
	for i in $(INCLUDE_DIRS); do echo $$i/Module.make >> $(DISTLIST); done
#	@echo src/include >> $(DISTLIST)

dist: distlist
	echo >>$(DISTLIST)
	echo $(PICS) >>$(DISTLIST)
	echo $(PUDS) >>$(DISTLIST)
	echo $(MISC) >>$(DISTLIST)
	echo $(CCLS) >>$(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	rm -rf $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir)
	chmod -R a+rX $(distdir)
	tar czhf $(distdir)-src.tar.gz $(distdir)
	echo "(c) 2003 by the Stratagus Project http://Stratagus.Org" | \
	zip -zq9r $(distdir)-src.zip $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)
	du -h $(distdir)-src.tar.gz $(distdir)-src.zip

bin-dist: all
	$(RM) $(DISTLIST)
	echo $(PICS) >>$(DISTLIST)
	echo $(PUDS) >>$(DISTLIST)
	echo $(CONTRIB) >>$(DISTLIST)
	echo $(CCLS) >>$(DISTLIST)
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
	@echo $(PICS) >>$(DISTLIST)
	@echo $(PUDS) >>$(DISTLIST)
	@echo $(CONTRIB) >>$(DISTLIST)
	@echo $(CCLS) >>$(DISTLIST)
	@echo $(DOCS) >>$(DISTLIST)
	@echo doc/README-SDL.txt >>$(DISTLIST)
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
	$(MAKE) -C tools RULESFILE=$(RULESFILE) distlist
	echo $(MISC) >>$(DISTLIST)
	echo $(CCLS) >>$(DISTLIST)
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
	touch Rules.make.WIN32

win32configure:
	autoconf
	export PATH=$(CROSSDIR)/i386-mingw32msvc/bin:$(CROSSDIR)/bin:$$PATH; \
	./configure --enable-win32 --host=i386-mingw32msvc --build=i386-linux \
	--with-lua=/usr/local/cross

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

install:	all install-stratagus install-tools

install-stratagus:
	@echo installing stratagus
	mkdir -p $(PREFIX)/lib/games/stratagus
	mkdir -p /var/lib/games
	install -m 755 stratagus $(PREFIX)/lib/games/stratagus
	cp -R data $(PREFIX)/lib/games/stratagus
	echo "$(PREFIX)/lib/games/stratagus/stratagus \
	-d $(PREFIX)/lib/games/stratagus/data  "\$$\@" | tee /var/lib/games/stratagus.log" \
	>$(PREFIX)/bin/stratagus
	chmod +x $(PREFIX)/bin/stratagus
	@echo installation of stratagus complete

install-tools:	all
	@echo installing stratagus tools
	mkdir -p $(PREFIX)/lib/games/stratagus/tools
	@echo installation of stratagus tools complete

uninstall:
	@echo uninstalling stratagus and stratagus tools
	rm -rf $(PREFIX)/lib/games/stratagus
	rm $(PREFIX)/bin/stratagus
	@echo uninstallation complete

