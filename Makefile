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
##	(c) Copyright 1998-2000 by Lutz Sammer
##
##	$Id$
##

TOPDIR=		.

include $(TOPDIR)/Rules.make

MAKE=	make TOPDIR=`pwd`
MODULES= src tools

all:	src clone$(EXE) tools

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
	@$(MAKE) && ./clone

runp::
	@$(MAKE) && ./clone && if [ -e gmon.sum ]; then \
		gprof -s clone gmon.out gmon.sum; \
	    else mv gmon.out gmon.sum; fi

doc::
	doxygen doxygen-clone.cfg

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
clone:	src etlib/hash.o src/libclone.a 
	$(CC) -o clone src/libclone.a $(CLONELIBS) -I. $(CFLAGS)

# WIN32-TARGET
clone.exe:	src etlib/prgname.o etlib/getopt.o etlib/hash.o src/libclone.a 
	$(CC) -o clone$(EXE) main.c src/libclone.a -lSDLmain $(CLONELIBS) -I. \
	$(CFLAGS)

# -L. -lefence 
# -Lccmalloc-0.2.3/src -lccmalloc -ldl 

tools::
	@$(MAKE) -C tools all

clean::
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i clean ; done
	$(RM) core gmon.out *.doc etlib/*.o

clobber:	clean
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i clobber ; done
	$(RM) clone$(EXE) gmon.sum
	$(RM) -r srcdoc/*
	@$(MAKE) -C tools clobber

distclean:	clobber
	@echo

ci::
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i ci ; done
	ci -l Makefile Common.mk Rules.make .indent.pro \
	doxygen-clone.cfg doxygen-0.4.diff \
	$(CCLS) $(DOCS)

lockver::
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i lockver ; done
	$(LOCKVER) Makefile Common.mk Rules.make .indent.pro \
	doxygen-clone.cfg doxygen-0.4.diff \
	$(CCLS) $(DOCS)

tags::
	@$(MAKE) -C src tags

depend::
	@$(MAKE) -C src depend

##############################################################################
#	Distributions
##############################################################################

DOCS    = README doc/readme.html doc/install.html doc/clone.html doc/faq.html \
	  doc/ChangeLog.html doc/todo.html doc/clone.lsm doc/development.html \
	  doc/LICENSE doc/OPL doc/opl.html \
	  doc/ccl/unittype.html \
	  doc/graphic/*.html doc/graphic/*.png

PICS    = contrib/ale-title.png

CCLS	= data/ccl/clone.ccl data/ccl/units.ccl data/ccl/missiles.ccl \
	  data/ccl/tilesets.ccl data/ccl/sound.ccl data/ccl/freecraft.ccl \
	  data/ccl/ui.ccl data/ccl/fonts.ccl \
	  data/ccl/anim.ccl data/ccl/upgrade.ccl data/default.cm

CONTRIB	= contrib/cross.png contrib/health.png contrib/mana.png \
	  contrib/ore,stone,coal.png contrib/food.png contrib/score.png

MISC    = Makefile Common.mk Rules.make.orig doxygen-clone.cfg \
	  doxygen-0.4.diff setup \
	  .indent.pro make/common.scc make/rules.scc make/makefile.scc \
	  make/README tools/udta.c tools/ugrd.c contrib/req.cm $(CONTRIB) \
	  etlib/hash.c etlib/getopt.c etlib/prgname.c etlib/prgname.h main.c

mydate	= $(shell date +%y%m%d)
distdir	= clone-$(mydate)

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
	tar chIf $(distdir).tar.bz2 $(distdir)
	zip -q9r $(distdir).zip $(distdir)
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
	tar chIf $(distdir)-small.tar.bz2 $(distdir)
	zip -q9r $(distdir)-small.zip $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)
	du -h $(distdir)-small.tar.gz $(distdir)-small.tar.bz2 $(distdir)-small.zip

bin-dist:: all
	$(RM) $(DISTLIST)
	echo $(PICS) >>$(DISTLIST)
	echo $(CONTRIB) >>$(DISTLIST)
	echo $(CCLS) >>$(DISTLIST)
	echo $(DOCS) >>$(DISTLIST)
	echo clone$(EXE) >>$(DISTLIST)
	echo tools/wartool$(EXE) >>$(DISTLIST)
	echo tools/build.sh >>$(DISTLIST)
	rm -rf $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir) 
	chmod -R a+r $(distdir)
	strip -s -R .comment $(distdir)/clone$(EXE)
	strip -s -R .comment $(distdir)/tools/wartool$(EXE)
	tar chzf clone-$(mydate)-bin.tar.gz $(distdir)
	tar chIf clone-$(mydate)-bin.tar.bz2 $(distdir)
	$(RM) $(DISTLIST)
	$(RM) -r $(distdir)

win32-bin-dist2:: win32
	@$(RM) $(DISTLIST)
	@echo $(PICS) >>$(DISTLIST)
	@echo $(CONTRIB) >>$(DISTLIST)
	@echo $(CCLS) >>$(DISTLIST)
	@echo $(DOCS) >>$(DISTLIST)
	@echo SDL.dll >>$(DISTLIST)
	@echo clone$(EXE) >>$(DISTLIST)
	@echo tools/wartool$(EXE) >>$(DISTLIST)
	@echo tools/build.bat >>$(DISTLIST)
	@rm -rf $(distdir)
	@mkdir $(distdir)
	@chmod 777 $(distdir)
	@for i in `cat $(DISTLIST)`; do echo $$i; done | cpio -pdml --quiet $(distdir) 
	@chmod -R a+r $(distdir)
	@strip -s -R .comment $(distdir)/clone$(EXE)
	@strip -s -R .comment $(distdir)/tools/wartool$(EXE)
	@zip -q9r clone-$(mydate)-win32bin.zip $(distdir)
	@$(RM) $(DISTLIST)
	@$(RM) -r $(distdir)
	du -h clone-$(mydate)-win32bin.zip

win32-bin-dist:: win32
	@export PATH=/usr/local/cross-tools/i386-mingw32/bin:$$PATH; \
	make $(WIN32) win32-bin-dist2

difffile=	clone-`date +%y%m%d`.diff
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
	rm -rf data/*.rgb data/puds data/sound data/graphic \
	data/interface data/campaigns

release:
	make ci
	make distclean
	make depend
	make bin-dist
	make win32new
	make win32-bin-dist
	make dist
	make win32distclean

##############################################################################
#	WIN32 Crosscompiler Build
##############################################################################

WIN32=	\
    EXE='.exe' \
    VIDEO='-DUSE_WIN32 $(SDL)'	\
    VIDEOLIB='$(SDLLIB) -lwsock32 -Wl,--stack,16777216'

win32new:
	@make distclean
	export PATH=/usr/local/cross-tools/i386-mingw32/bin:$$PATH; \
	make depend

win32_2:
	make $(WIN32) all

win32:
	export PATH=/usr/local/cross-tools/i386-mingw32/bin:$$PATH; \
	make win32_2

win32distclean:
	export PATH=/usr/local/cross-tools/i386-mingw32/bin:$$PATH; \
	make $(WIN32) distclean

##############################################################################
#	INSTALL/UNINSTALL	
##############################################################################

install:	all
	install -m 755 clone $(DESTDIR)/usr/games/ale-clone

install-tools:	all
	install -m 755 tools/wartool  $(DESTDIR)/usr/lib/ale-clone/tools/
	install -m 755 tools/build.sh $(DESTDIR)/usr/lib/ale-clone/tools/

uninstall:
	@echo "under construction: make it by hand :)"
