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
##	Makefile	-	The make file (3 level).
##
##	(c) Copyright 1998,2000,2001 by Lutz Sammer
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

TOPDIR	= ../..
RULESFILE ?= Rules.make
include $(TOPDIR)/$(RULESFILE)

##
## USER INTERFACE
##
## TODO: move here all user-interface-related files
##

MODULE	= ui

SRCS	= mouse.c button_checks.c botpanel.c mainscr.c \
	  icons.c ui.c ccl_ui.c menus.c console.c \
	  interface.c

OBJS	= $(addprefix $(OBJDIR), $(SRCS:.c=.$(OE)))

include $(TOPDIR)/Common.mk
