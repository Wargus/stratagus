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
##	Makefile	-	The make file.
##
##	(c) Copyright 1998-2002 by Lutz Sammer
##
##	FreeCraft is free software; you can redistribute it and/or modify
##	it under the terms of the GNU General Public License as published
##	by the Free Software Foundation; only version 2 of the License.
##
##	FreeCraft is distributed in the hope that it will be useful,
##	but WITHOUT ANY WARRANTY; without even the implied warranty of
##	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##	GNU General Public License for more details.
##
##	$Id$
##

TOPDIR=		.

RULESFILE ?= Rules.make
WINRULESFILE = Rules.make.WIN32

include $(TOPDIR)/$(RULESFILE)
OBJDIR ?= .

CROSSDIR=/usr/local/cross-tools

MAKEFLAGS= TOPDIR=$(shell pwd)
MODULES= src tools

all:	src freecraft$(EXE) tools

help:
	@-echo "make cycle			clean,depend,tags,all"
	@-echo "make install			install all files"
	@-echo "make uninstall			uninstall all files"
	@-echo "make run			create and run"
	@-echo "make runp			create and run with profiler"
	@-echo "make clean			cleanup keep only executables"
	@-echo "make clobber			clean all files"
	@-echo "make distclean			clean all files"
	@-echo "make ci				check in RS"
	@-echo "make doc			make source documention with doxygen"
	@-echo "make doc++			make source documention with doc++"
	@-echo "make lockver NAME="version"	label current version with symbolic name"
	@-echo "make strip			strip freecraft and/or freecraft.exe"
	@-echo "make tags			create ctags"
	@-echo "make depend			create dependencies"
	@-echo "make dist			create distribution"
	@-echo "make small-dist			create small distribution"
	@-echo "make buildit			create data files from original data"
	@-echo "make buildclean			cleanup build data files"
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
	@$(MAKE) && ./freecraft

runp::
	@$(MAKE) && ./freecraft && if [ -e gmon.sum ]; then \
		gprof -s freecraft gmon.out gmon.sum; \
	    else mv gmon.out gmon.sum; fi

doc::
	doxygen contrib/doxygen-freecraft.cfg

doc++::
	@$(MAKE) -C src RULESFILE=$(RULESFILE) doc
	@if [ ! -d srcdoc ]; then mkdir srcdoc; fi
	@$(DOCPP) -v -H -A -a -b -c -j -d srcdoc `find . -name "*.doc" -print`

src::
	@$(MAKE) -C src RULESFILE=$(RULESFILE) OBJDIR=$(OBJDIR) all

etlib/$(OBJDIR)/hash.$(OE): etlib/hash.c
	@if [ ! -d etlib/$(OBJDIR) ]; then mkdir etlib/$(OBJDIR); fi
	$(CC) -c $(CFLAGS) $< -o $@

etlib/$(OBJDIR)/getopt.$(OE): etlib/getopt.c
	@if [ ! -d etlib/$(OBJDIR) ]; then mkdir etlib/$(OBJDIR); fi
	$(CC) -c $(CFLAGS) $< -o $@

etlib/$(OBJDIR)/prgname.$(OE): etlib/prgname.c
	@if [ ! -d etlib/$(OBJDIR) ]; then mkdir etlib/$(OBJDIR); fi
	$(CC) -c $(CFLAGS) $< -o $@

src/$(OBJDIR)/main.$(OE): src/main.c
	@if [ ! -d src/$(OBJDIR) ]; then mkdir src/$(OBJDIR); fi
	$(CC) -c $(CFLAGS) $< -o $@

src/$(OBJDIR)/libclone.a: ;

# UNIX-TARGET
freecraft:	src etlib/$(OBJDIR)/hash.$(OE) src/$(OBJDIR)/libclone.a
	$(CCLD) -o freecraft src/$(OBJDIR)/libclone.a etlib/$(OBJDIR)/hash.$(OE) $(CLONELIBS) -I. $(CFLAGS)

# WIN32-TARGET
freecraft.exe:	src etlib/$(OBJDIR)/prgname.$(OE) etlib/$(OBJDIR)/getopt.$(OE) \
		etlib/$(OBJDIR)/hash.$(OE) src/$(OBJDIR)/freecraftrc.$(OE) \
		src/$(OBJDIR)/libclone.a src/$(OBJDIR)/main.$(OE)
	$(CCLD) -o freecraft$(EXE) src/$(OBJDIR)/main.$(OE) \
		src/$(OBJDIR)/libclone.a src/$(OBJDIR)/freecraftrc.$(OE) \
		etlib/$(OBJDIR)/prgname.$(OE) etlib/$(OBJDIR)/getopt.$(OE) \
		etlib/$(OBJDIR)/hash.$(OE) \
		-lSDLmain $(CLONELIBS) -I. $(CFLAGS)

strip:
	@if [ -f freecraft ]; then strip freecraft; fi
	@if [ -f freecraft.exe ]; then $(CROSSDIR)/i386-mingw32msvc/bin/strip freecraft.exe; fi

src/$(OBJDIR)/freecraftrc.$(OE): src/freecraft.rc
	windres --include-dir contrib -osrc/$(OBJDIR)/freecraftrc.$(OE) src/freecraft.rc

# -L. -lefence
# -Lccmalloc-0.2.3/src -lccmalloc -ldl

echo::
	@-echo CFLAGS: $(CFLAGS)
	@-echo LIBS: $(CLONELIBS)

tools::
	@$(MAKE) -C tools RULESFILE=$(RULESFILE) all

clean::
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i RULESFILE=$(RULESFILE) clean ; done
	$(RM) core gmon.out cscope.out *.doc etlib/$(OBJDIR)/*.$(OE) .#*

clobber:	clean
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i RULESFILE=$(RULESFILE) clobber ; done
	$(RM) freecraft$(EXE) gmon.sum *~ stderr.txt stdout.txt
	$(RM) -r srcdoc/*
	@$(MAKE) -C tools RULESFILE=$(RULESFILE) clobber

distclean:	clobber
	@echo

ci::
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i RULESFILE=$(RULESFILE) ci ; done
	ci -l Makefile Common.mk $(RULESFILE) .indent.pro \
	contrib/doxygen-freecraft.cfg \
	$(CCLS) $(DOCS)

lockver::
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i RULESFILE=$(RULESFILE) lockver ; done
	$(LOCKVER) Makefile Common.mk $(RULESFILE) .indent.pro \
	contrib/doxygen-freecraft.cfg \
	$(CCLS) $(DOCS)

tags::
	@$(MAKE) -C src RULESFILE=$(RULESFILE) tags

depend::
	@$(MAKE) -C src RULESFILE=$(RULESFILE) depend

##############################################################################
#	Distributions
##############################################################################

DOCS    = README README.BeOS doc/readme.html doc/install.html \
	  doc/freecraft.html doc/datadir.html \
	  doc/faq.html doc/ChangeLog.html doc/todo.html doc/freecraft.lsm \
	  doc/development.html doc/gpl.txt doc/gpl.html doc/SIOD.txt \
	  doc/ccl/ai.html doc/ccl/ccl.html doc/ccl/config.html \
	  doc/ccl/icon.html doc/ccl/tileset.html doc/ccl/unittype.html \
	  doc/ccl/research.html doc/graphic/*.html doc/graphic/*.png \
	  doc/trigger.txt debian/freecraft.6

PICS    = contrib/freecraft.png contrib/freecraft.ico

PUDS	= contrib/puds/single/*.pud.gz contrib/puds/multi/*.pud.gz \
	  contrib/puds/single/*.txt contrib/puds/multi/*.txt

CCLS	= data/ccl/units.ccl data/ccl/human/units.ccl data/ccl/orc/units.ccl \
	  data/ccl/constructions.ccl data/ccl/human/constructions.ccl \
	  data/ccl/orc/constructions.ccl \
	  data/ccl/missiles.ccl data/ccl/icons.ccl \
	  data/ccl/sound.ccl data/ccl/freecraft.ccl \
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
	  data/ccl/anim.ccl data/ccl/wc2.ccl data/default.cm \
	  data/ccl/tips.ccl data/ccl/menus.ccl data/ccl/keystrokes.ccl \
	  data/ccl/editor.ccl data/campaigns/*/*.cm

CONTRIB	= contrib/cross.png contrib/red_cross.png \
	  contrib/health.png contrib/mana.png \
	  contrib/health2.png contrib/mana2.png \
	  contrib/ore,stone,coal.png contrib/food.png contrib/score.png \
	  contrib/music/toccata.mod.gz \
	  contrib/msvc.zip contrib/macosx.tgz contrib/stdint.h

MISC    = Makefile Common.mk Rules.make.orig FreeCraft-beos.proj setup \
	  contrib/doxygen-freecraft.cfg contrib/doxygen-header.html \
	  .indent.pro make/common.scc make/rules.scc make/makefile.scc \
	  make/README tools/udta.c tools/ugrd.c $(CONTRIB) \
	  etlib/hash.c etlib/getopt.c etlib/prgname.c etlib/prgname.h

mydate	= $(shell date +%y%m%d)
distdir	= freecraft-$(mydate)

dist::
	$(RM) $(DISTLIST)
	@set -e; for i in $(MODULES); do $(MAKE) -C $$i RULESFILE=$(RULESFILE) distlist ; done
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
	chown -R johns:freecraft $(distdir)
	chmod -R a+rX $(distdir)
	tar chzf $(distdir).tar.gz $(distdir)
	tar cjhf $(distdir).tar.bz2 $(distdir)
	echo "(c) 2002 by the FreeCraft Project http://FreeCraft.Org" | \
	zip -zq9r $(distdir).zip $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)
	du -h $(distdir).tar.gz $(distdir).tar.bz2 $(distdir).zip

small-dist::
	@$(RM) $(DISTLIST)
	$(MAKE) -C src RULESFILE=$(RULESFILE) distlist
	$(MAKE) -C tools RULESFILE=$(RULESFILE) distlist
	echo $(MISC) >>$(DISTLIST)
	echo $(CCLS) >>$(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	rm -rf $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir)
	chown -R johns:freecraft $(distdir)
	chmod -R a+rX $(distdir)
	tar chzf $(distdir)-small.tar.gz $(distdir)
	tar cjhf $(distdir)-small.tar.bz2 $(distdir)
	echo "(c) 2002 by the FreeCraft Project http://FreeCraft.Org" | \
	zip -zq9r $(distdir)-small.zip $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)
	du -h $(distdir)-small.tar.gz $(distdir)-small.tar.bz2 $(distdir)-small.zip

bin-dist:: all
	$(RM) $(DISTLIST)
	echo $(PICS) >>$(DISTLIST)
	echo $(PUDS) >>$(DISTLIST)
	echo $(CONTRIB) >>$(DISTLIST)
	echo $(CCLS) >>$(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	echo freecraft$(EXE) >>$(DISTLIST)
	echo tools/wartool$(EXE) >>$(DISTLIST)
	echo tools/build.sh >>$(DISTLIST)
	rm -rf $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir)
	chown -R johns:freecraft $(distdir)
	chmod -R a+rX $(distdir)
	strip -s -R .comment $(distdir)/freecraft$(EXE)
	strip -s -R .comment $(distdir)/tools/wartool$(EXE)
	tar chzf freecraft-$(mydate)-bin.tar.gz $(distdir)
	tar cjhf freecraft-$(mydate)-bin.tar.bz2 $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)

#----------------------------------------------------------------------------

win32-bin-dist2:: win32
	@$(RM) $(DISTLIST)
	@echo $(PICS) >>$(DISTLIST)
	@echo $(PUDS) >>$(DISTLIST)
	@echo $(CONTRIB) >>$(DISTLIST)
	@echo $(CCLS) >>$(DISTLIST)
	@echo $(DOCS) >>$(DISTLIST)
	@echo SDL.dll doc/README-SDL.txt doc/ZIP-LICENSE >>$(DISTLIST)
	@echo freecraft$(EXE) >>$(DISTLIST)
	@echo tools/wartool$(EXE) >>$(DISTLIST)
	@echo tools/build.bat >>$(DISTLIST)
	@rm -rf $(distdir)
	@mkdir $(distdir)
	@chmod 777 $(distdir)
	@for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir)
	@cp tools/build.bat $(distdir)
	chown -R johns:freecraft $(distdir)
	@chmod -R a+rX $(distdir)
	@strip -s -R .comment $(distdir)/freecraft$(EXE)
	@strip -s -R .comment $(distdir)/tools/wartool$(EXE)
	@echo "(c) 2002 by the FreeCraft Project http://FreeCraft.Org" | \
	zip -zq9r freecraft-$(mydate)-win32bin.zip $(distdir)
	@$(RM) $(DISTLIST)
	@$(RM) -r $(distdir)
	du -h freecraft-$(mydate)-win32bin.zip

win32-bin-dist: win32
	@export PATH=$(CROSSDIR)/i386-mingw32msvc/bin:$(CROSSDIR)/bin:$$PATH; \
	$(MAKE) RULESFILE=$(WINRULESFILE) $(WIN32) win32-bin-dist2

win32-exe-dist:	win32-bin-dist
	cat tools/SFXWiz32-gcc.exe freecraft-$(mydate)-win32bin.zip \
		> freecraft-$(mydate)-win32bin.exe

#----------------------------------------------------------------------------

MYDATE	= $(shell date +%y%m%d)
PCRAFT= freecraft-$(MYDATE).tar.bz2
LCRAFT= freecraft-$(MYDATE)-bin.tar.bz2
WCRAFT= freecraft-$(MYDATE)-win32bin.zip
FCMP=	../fcmp-*.tar.gz

linux-complete:
	mkdir freecraft-complete
	tar xjf $(PCRAFT)
	tar xjf $(LCRAFT)
	cp -a freecraft-$(MYDATE)/* freecraft-complete
	mv freecraft-complete/data freecraft-complete/data.wc2
	tar xzCf freecraft-complete $(FCMP)
	rm -rf freecraft-$(MYDATE)
	chmod 777 freecraft-complete
	chown -R johns:freecraft freecraft-complete
	chmod -R a+rX freecraft-complete
	-tar czhf freecraft-$(MYDATE)-complete-linux.tar.gz freecraft-complete
	-tar cjhf freecraft-$(MYDATE)-complete-linux.tar.bz2 freecraft-complete
	rm -rf freecraft-complete

win32-complete:
	mkdir freecraft-complete
	tar xjf $(PCRAFT)
	unzip -oq $(WCRAFT)
	cp -a freecraft-$(MYDATE)/* freecraft-complete
	mv freecraft-complete/data freecraft-complete/data.wc2
	tar xzCf freecraft-complete $(FCMP)
	rm -rf freecraft-$(MYDATE)
	chmod 777 freecraft-complete
	chown -R johns:freecraft freecraft-complete
	chmod -R a+rX freecraft-complete
	echo "(c) 2002 by the FreeCraft Project http://FreeCraft.Org" | \
	zip -zq9r freecraft-$(MYDATE)-complete-win32.zip freecraft-complete
	cat tools/SFXWiz32-gcc.exe freecraft-$(MYDATE)-complete-win32.zip \
		> freecraft-$(MYDATE)-complete-win32.exe
	rm -rf freecraft-complete

complete:	linux-complete win32-complete

#----------------------------------------------------------------------------
difffile=	freecraft-`date +%y%m%d`.diff
diff:
	@$(RM) $(difffile)
	@$(RM) $(DISTLIST)
	$(MAKE) -C src RULESFILE=$(RULESFILE) distlist
	$(MAKE) -C tools RULESFILE=$(RULESFILE) distlist
	echo $(MISC) >>$(DISTLIST)
	echo $(CCLS) >>$(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	rcsdiff -u `cat $(DISTLIST)` > $(difffile)

buildit:	tools
	. tools/build.sh

buildclean:
	rm -rf data/*.rgb data/*.gimp data/puds data/sound data/graphic \
	data/interface data/campaigns data/text data/health.png data/mana.png \
	data/default.pud.gz data/freecraft.png
	rm -rf data/graphics data/sounds data/texts data/music data/videos

release:
	$(MAKE) distclean
	$(MAKE) depend
	$(MAKE) bin-dist
	$(MAKE) win32new
	$(MAKE) win32-bin-dist
	$(MAKE) win32-exe-dist
	$(MAKE) win32distclean
	$(MAKE) dist

##############################################################################
#	WIN32 Crosscompiler Build
##############################################################################

#-lws2_32 -Wl,--stack,63550000  -Wl,--stack,16777216
WIN32=	\
    EXE='.exe' \
    XLDFLAGS='' \
    XIFLAGS='' \
    VIDEO='-DUSE_WIN32 $(SDL)'  \
    VIDEOLIB='-L$(CROSSDIR)/i386-mingw32msvc/lib $(SDLLIB) -lwsock32 -lws2_32' \
    RULESFILE=$(WINRULESFILE)

win32new:
	@$(MAKE) RULESFILE=$(WINRULESFILE) distclean
	export PATH=$(CROSSDIR)/i386-mingw32msvc/bin:$(CROSSDIR)/bin:$$PATH; \
	$(MAKE) $(WIN32) depend

win32_2:
	$(MAKE) $(WIN32) all

win32:
	export PATH=$(CROSSDIR)/i386-mingw32msvc/bin:$(CROSSDIR)/bin:$$PATH; \
	$(MAKE) $(WIN32) win32_2

win32distclean:
	export PATH=$(CROSSDIR)/i386-mingw32msvc/bin:$(CROSSDIR)/bin:$$PATH; \
	$(MAKE) $(WIN32) distclean

##############################################################################
#	INSTALL/UNINSTALL
##############################################################################

install:	all install-freecraft install-tools

install-freecraft:
	@echo installing freecraft
	mkdir -p $(PREFIX)/lib/games/freecraft
	mkdir -p /var/lib/games
	install -m 755 freecraft $(PREFIX)/lib/games/freecraft
	cp -R data $(PREFIX)/lib/games/freecraft
	echo "$(PREFIX)/lib/games/freecraft/freecraft \
	-d $(PREFIX)/lib/games/freecraft/data  "\$$\@" | tee /var/lib/games/freecraft.log" \
	>$(PREFIX)/bin/freecraft
	chmod +x $(PREFIX)/bin/freecraft
	@echo installation of freecraft complete

install-tools:	all
	@echo installing freecraft tools
	mkdir -p $(PREFIX)/lib/games/freecraft/tools
	install -m 755 tools/wartool  $(PREFIX)/lib/games/freecraft/tools
	install -m 755 tools/build.sh $(PREFIX)/lib/games/freecraft/tools
	@echo installation of freecraft tools complete

uninstall:
	@echo uninstalling freecraft and freecraft tools
	rm -rf $(PREFIX)/lib/games/freecraft
	rm $(PREFIX)/bin/freecraft
	@echo uninstallation complete

