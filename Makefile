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

TOPDIR=	..
include $(TOPDIR)/Rules.make

OBJS=

MODULES= clone map unit action ai ui sound video network pathfinder siod \
	 game beos

all::
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i all ; done

doc::	
	@set -e; for i in $(MODULES) include ; do $(MAKE) -C $$i doc ; done

clean::
	@set -e; for i in $(MODULES) include ; do $(MAKE) -C $$i clean ; done
	$(RM) $(OBJS) libclone.a

clobber::	clean
	@set -e; for i in $(MODULES) include ; do $(MAKE) -C $$i clobber ; done
	$(RM) .depend tags

depend::
	@echo -n >.depend
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i depend ; done

tags::
	@set -e; for i in $(MODULES) ; do cd $$i ; $(MAKE) tags ; cd .. ; done

distlist::
	echo >>$(DISTLIST)
	echo src/main.c  >>$(DISTLIST)
	echo src/freecraft.rc >>$(DISTLIST)
	echo src/Makefile >>$(DISTLIST)
	@for i in include $(MODULES) ; do $(MAKE) -C $$i distlist ; done

ci::
	@for i in include $(MODULES) ; do $(MAKE) -C $$i ci ; done
	ci -l Makefile

lockver::
	@for i in include $(MODULES) ; do $(MAKE) -C $$i lockver ; done
	$(LOCKVER) Makefile

ifeq (.depend,$(wildcard .depend))
include .depend
endif
