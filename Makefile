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
##	FreeCraft is free software; you can redistribute it and/or modify
##	it under the terms of the GNU General Public License as published
##	by the Free Software Foundation; either version 2 of the License,
##	or (at your option) any later version.
##
##	FreeCraft is distributed in the hope that it will be useful,
##	but WITHOUT ANY WARRANTY; without even the implied warranty of
##	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##	GNU General Public License for more details.
##
##	$Id$
##

TOPDIR=	..
RULESFILE ?= Rules.make
include $(TOPDIR)/$(RULESFILE)

OBJS=

MODULES= clone map unit action ai ui sound video network pathfinder siod \
	 game beos missile libmodplug

all::
	@if [ ! -d ./$(OBJDIR) ] ; then mkdir ./$(OBJDIR) ; fi
	@set -e; for i in $(MODULES) ; do\
	    if [ ! -d $$i/$(OBJDIR) ] ; then mkdir $$i/$(OBJDIR) ; fi ;\
	    $(MAKE) -C $$i RULESFILE=$(RULESFILE) all ;\
	done

doc::	
	@set -e; for i in $(MODULES) include ; do $(MAKE) -C $$i RULESFILE=$(RULESFILE) doc ; done

clean::
	@set -e; for i in $(MODULES) include ; do $(MAKE) -C $$i RULESFILE=$(RULESFILE) clean ; done
	$(RM) $(OBJS) $(OBJDIR)libclone.a
	-@$(RM) $(OBJDIR)main.$(OE) $(OBJDIR)freecraftrc.$(OE)

clobber::	clean
	@set -e; for i in $(MODULES) include ; do $(MAKE) -C $$i RULESFILE=$(RULESFILE) clobber ; done
	$(RM) .depend tags

depend::
	@echo -n >.depend
	@set -e; for i in $(MODULES) ; do $(MAKE) -C $$i RULESFILE=$(RULESFILE) depend ; done

tags::
	@set -e; for i in $(MODULES) ; do cd $$i ; $(MAKE) RULESFILE=$(RULESFILE) tags ; cd .. ; done

distlist::
	echo >>$(DISTLIST)
	echo src/main.c  >>$(DISTLIST)
	echo src/freecraft.rc >>$(DISTLIST)
	echo src/Makefile >>$(DISTLIST)
	@for i in include $(MODULES) ; do $(MAKE) -C $$i RULESFILE=$(RULESFILE) distlist ; done

ci::
	@for i in include $(MODULES) ; do $(MAKE) -C $$i RULESFILE=$(RULESFILE) ci ; done
	ci -l Makefile

lockver::
	@for i in include $(MODULES) ; do $(MAKE) -C $$i RULESFILE=$(RULESFILE) lockver ; done
	$(LOCKVER) Makefile

ifeq (.depend,$(wildcard .depend))
include .depend
endif
