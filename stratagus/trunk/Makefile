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
##	(c) Copyright 1998-2001 by Lutz Sammer
##
##	$Id$
##

TOPDIR=		.

include $(TOPDIR)/Rules.make

MAKE=	make TOPDIR=`pwd`
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
	@-echo "make tags			create ctags"
	@-echo "make depend			create dependencies"
	@-echo "make dist			create distribution"
	@-echo "make small-dist			create small distribution"
	@-echo "make buildit			create data files from original data"
	@-echo "make buildclean			cleanup build data files"
	@-echo "make win32new			start new win32"
	@-echo "make win32			build the win32 version"
	@-echo "make win32distclean		clean all files of win32"
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
	@$(MAKE) -C src doc
	@if [ ! -d srcdoc ]; then mkdir srcdoc; fi
	@$(DOCPP) -v -H -A -a -b -c -j -d srcdoc `find . -name "*.doc" -print`

src::
	@$(MAKE) -C src all

etlib/hash.o: etlib/hash.c
etlib/getopt.o: etlib/getopt.c
etlib/prgname.o: etlib/prgname.c

# UNIX-TARGET
freecraft:	src etlib/hash.o src/libclone.a
	$(CC) -o freecraft src/libclone.a $(CLONELIBS) -I. $(CFLAGS)

# WIN32-TARGET
freecraft.exe:	src etlib/prgname.o etlib/getopt.o etlib/hash.o \
		src/freecraftrc.o src/libclone.a
	$(CC) -o freecraft$(EXE) src/main.c src/libclone.a src/freecraftrc.o \
	-lSDLmain $(CLONELIBS) -I. $(CFLAGS)

src/freecraftrc.o: src/freecraft.rc
	windres --include-dir contrib -osrc/freecraftrc.o src/freecraft.rc

# -L. -lefence
# -Lccmalloc-0.2.3/src -lccmalloc -ldl

tools::
	@$(MAKE) -C tools all

clean::
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i clean ; done
	$(RM) core gmon.out *.doc etlib/*.o .#*

clobber:	clean
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i clobber ; done
	$(RM) freecraft$(EXE) gmon.sum
	$(RM) -r srcdoc/*
	@$(MAKE) -C tools clobber

distclean:	clobber
	@echo

ci::
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i ci ; done
	ci -l Makefile Common.mk Rules.make .indent.pro \
	contrib/doxygen-freecraft.cfg \
	$(CCLS) $(DOCS)

lockver::
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i lockver ; done
	$(LOCKVER) Makefile Common.mk Rules.make .indent.pro \
	contrib/doxygen-freecraft.cfg \
	$(CCLS) $(DOCS)

tags::
	@$(MAKE) -C src tags

depend::
	@$(MAKE) -C src depend

##############################################################################
#	Distributions
##############################################################################

DOCS    = README README.BeOS doc/readme.html doc/install.html \
	  doc/freecraft.html doc/datadir.html doc/unit.html \
	  doc/faq.html doc/ChangeLog.html doc/todo.html doc/freecraft.lsm \
	  doc/development.html doc/LICENSE doc/OPL doc/opl.html \
	  doc/artistic-license.html doc/ccl/ccl.html doc/ccl/tileset.html \
	  doc/ccl/unittype.html doc/graphic/*.html doc/graphic/*.png

PICS    = contrib/freecraft.png contrib/freecraft.ico

CCLS	= data/ccl/units.ccl data/ccl/missiles.ccl \
	  data/ccl/tilesets.ccl data/ccl/sound.ccl data/ccl/freecraft.ccl \
	  data/ccl/ui.ccl data/ccl/fonts.ccl data/ccl/ai.ccl \
	  data/ccl/summer.ccl data/ccl/winter.ccl data/ccl/wasteland.ccl \
	  data/ccl/swamp.ccl data/ccl/anim.ccl data/ccl/upgrade.ccl \
	  data/ccl/wc2.ccl data/default.cm

CONTRIB	= contrib/cross.png contrib/health.png contrib/mana.png \
	  contrib/ore,stone,coal.png contrib/food.png contrib/score.png

MISC    = Makefile Common.mk Rules.make.orig FreeCraft-beos.proj setup \
	  contrib/doxygen-freecraft.cfg contrib/doxygen-header.html \
	  .indent.pro make/common.scc make/rules.scc make/makefile.scc \
	  make/README tools/udta.c tools/ugrd.c contrib/req.cm $(CONTRIB) \
	  etlib/hash.c etlib/getopt.c etlib/prgname.c etlib/prgname.h

mydate	= $(shell date +%y%m%d)
distdir	= freecraft-$(mydate)

dist::
	$(RM) $(DISTLIST)
	@set -e; for i in $(MODULES); do $(MAKE) -C $$i distlist ; done
	echo >>$(DISTLIST)
	echo $(PICS) >>$(DISTLIST)
	echo $(MISC) >>$(DISTLIST)
	echo $(CCLS) >>$(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	rm -rf $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir)
	chmod -R a+r $(distdir)
	tar chzf $(distdir).tar.gz $(distdir)
	tar cjhf $(distdir).tar.bz2 $(distdir)
	echo "(c) 2001 by the FreeCraft Project http://FreeCraft.Org" | \
	zip -zq9r $(distdir).zip $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)
	du -h $(distdir).tar.gz $(distdir).tar.bz2 $(distdir).zip

small-dist::
	@$(RM) $(DISTLIST)
	$(MAKE) -C src distlist
	$(MAKE) -C tools distlist
	echo $(MISC) >>$(DISTLIST)
	echo $(CCLS) >>$(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	rm -rf $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir)
	chmod -R a+r $(distdir)
	tar chzf $(distdir)-small.tar.gz $(distdir)
	tar cjhf $(distdir)-small.tar.bz2 $(distdir)
	echo "(c) 2001 by the FreeCraft Project http://FreeCraft.Org" | \
	zip -zq9r $(distdir)-small.zip $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)
	du -h $(distdir)-small.tar.gz $(distdir)-small.tar.bz2 $(distdir)-small.zip

bin-dist:: all
	$(RM) $(DISTLIST)
	echo $(PICS) >>$(DISTLIST)
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
	chmod -R a+r $(distdir)
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
	@echo $(CONTRIB) >>$(DISTLIST)
	@echo $(CCLS) >>$(DISTLIST)
	@echo $(DOCS) >>$(DISTLIST)
	@echo SDL.dll README-SDL.txt doc/ZIP-LICENSE >>$(DISTLIST)
	@echo freecraft$(EXE) >>$(DISTLIST)
	@echo tools/wartool$(EXE) >>$(DISTLIST)
	@echo tools/build.bat >>$(DISTLIST)
	@rm -rf $(distdir)
	@mkdir $(distdir)
	@chmod 777 $(distdir)
	@for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir)
	@cp tools/build.bat $(distdir)
	@chmod -R a+r $(distdir)
	@strip -s -R .comment $(distdir)/freecraft$(EXE)
	@strip -s -R .comment $(distdir)/tools/wartool$(EXE)
	@echo "(c) 2001 by the FreeCraft Project http://FreeCraft.Org" | \
	zip -zq9r freecraft-$(mydate)-win32bin.zip $(distdir)
	@$(RM) $(DISTLIST)
	@$(RM) -r $(distdir)
	du -h freecraft-$(mydate)-win32bin.zip

win32-bin-dist: win32
	@export PATH=/usr/local/cross-tools/i386-mingw32msvc/bin:$$PATH; \
	$(MAKE) $(WIN32) win32-bin-dist2

win32-exe-dist:	win32-bin-dist
	cat tools/SFXWiz32-gcc.exe freecraft-$(mydate)-win32bin.zip \
		> freecraft-$(mydate)-win32bin.exe

#----------------------------------------------------------------------------

MYDATE	= $(shell date +%y%m%d)
PCRAFT= freecraft-$(MYDATE).tar.bz2
LCRAFT= freecraft-$(MYDATE)-bin.tar.bz2
WCRAFT= freecraft-$(MYDATE)-win32bin.zip
FCRAFT=	../fcraft-0.14pre2.tar.gz
SCRAFT= ../scraft-0.02.tar.bz2

linux-complete:
	tar xzf $(FCRAFT)
	rm -rf fcraft/freecraft fcraft/data/cvs_ccl
	#cp data/graphic/title.png "data/graphic/interface/Menu background without title.png"
	tar xjf $(SCRAFT)
	tar xjf $(PCRAFT)
	tar xjf $(LCRAFT)
	mkdir freecraft-complete
	cp -a freecraft-$(MYDATE)/* freecraft-complete
	cp -a fcraft/* freecraft-complete
	cp -a fclone/* freecraft-complete
	rm -rf freecraft-$(MYDATE)
	rm -rf fcraft
	rm -rf fclone
	-tar czhf freecraft-$(MYDATE)-complete-linux.tar.gz freecraft-complete
	-tar cjhf freecraft-$(MYDATE)-complete-linux.tar.bz2 freecraft-complete
	rm -rf freecraft-complete

win32-complete:
	tar xzf $(FCRAFT)
	rm -rf fcraft/freecraft fcraft/data/cvs_ccl
	#cp data/graphic/title.png "data/graphic/interface/Menu background without title.png"
	tar xjf $(SCRAFT)
	tar xjf $(PCRAFT)
	unzip -oq $(WCRAFT)
	mkdir freecraft-complete
	cp -a freecraft-$(MYDATE)/* freecraft-complete
	cp -a fcraft/* freecraft-complete
	cp -a fclone/* freecraft-complete
	rm -rf freecraft-$(MYDATE)
	rm -rf fcraft
	rm -rf fclone
	mv freecraft-complete/CONTRIB freecraft-complete/CONTRIB-fgp.txt
	echo "(c) 2001 by the FreeCraft Project http://FreeCraft.Org" | \
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
	$(MAKE) -C src distlist
	$(MAKE) -C tools distlist
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
	rm -rf data/graphics data/sounds data/texts

release:
	$(MAKE) distclean
	$(MAKE) depend
	$(MAKE) bin-dist "ZDEFS=-DUSE_ZLIB -DUSE_BZ2LIB" "ZLIBS=-lz -lbz2"
	$(MAKE) win32new
	$(MAKE) win32-bin-dist
	$(MAKE) win32-exe-dist
	$(MAKE) win32distclean
	$(MAKE) dist

##############################################################################
#	WIN32 Crosscompiler Build
##############################################################################

WIN32=	\
    EXE='.exe' \
    VIDEO='-DUSE_WIN32 $(SDL)'	\
    VIDEOLIB='-L/usr/local/cross-tools/i386-mingw32msvc/lib $(SDLLIB) -lwsock32 -Wl,--stack,16777216'

win32new:
	@$(MAKE) distclean
	export PATH=/usr/local/cross-tools/i386-mingw32msvc/bin:$$PATH; \
	$(MAKE) $(WIN32) depend

win32_2:
	$(MAKE) $(WIN32) all

win32:
	export PATH=/usr/local/cross-tools/i386-mingw32msvc/bin:$$PATH; \
	$(MAKE) win32_2

win32distclean:
	export PATH=/usr/local/cross-tools/i386-mingw32msvc/bin:$$PATH; \
	$(MAKE) $(WIN32) distclean

##############################################################################
#	INSTALL/UNINSTALL
##############################################################################

install:	all
	install -m 755 freecraft $(DESTDIR)/usr/games/freecraft

install-tools:	all
	install -m 755 tools/wartool  $(DESTDIR)/usr/lib/freecraft/tools/
	install -m 755 tools/build.sh $(DESTDIR)/usr/lib/freecraft/tools/

uninstall:
	@echo "under construction: make it by hand :)"
