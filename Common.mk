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
##	Common.mk        -       Common make (GNU Make)
##
##	(c) Copyright 1998,2000,2002 by Lutz Sammer
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

all:	$(OBJS)

doc:	$(SRCS:.c=.doc) $(CPPSRCS.cpp=.doc) $(HDRS:.h=.doc)

clean::
	$(RM) $(OBJS) core *.doc

clobber: clean
	$(RM) -rf $(OBJDIR)
	$(RM) .depend .#* *~ *.$(OE)

depend::
	@echo -n >.depend
	@for i in $(SRCS) $(CPPSRCS) ; do\
	echo -n $(OBJDIR)/ >> .depend;\
	$(CC) -MM $(IFLAGS) $(DFLAGS) $(CFLAGS) $$i >>.depend ; done

tags::
	for i in $(SRCS) $(CPPSRCS) ; do\
	ctags $(CTAGSFLAGS) $(TAGS) `pwd`/$$i ;\
	done

ci::
	ci -l $(SRCS) $(CPPSRCS) $(HDRS) Makefile

lockver::
	$(LOCKVER) $(SRCS) $(CPPSRCS) $(HDRS) Makefile

distlist::
	@echo >>$(DISTLIST)
	@for i in $(SRCS) $(CPPSRCS) $(HDRS) Makefile $(EXTRA) ; do \
	echo src/$(MODULE)/$$i >>$(DISTLIST) ; done

$(OBJS): $(TOPDIR)/Rules.make

#
#	include dependency files, if they exist
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
