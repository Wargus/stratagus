##   ___________		     _________		      _____  __
##   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
##    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
##    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
##    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
##	  \/		    \/	   \/	     \/		   \/
##  ______________________                           ______________________
##			  T H E   W A R   B E G I N S
##	   FreeCraft - A free fantasy real time strategy game engine
##
##	Module.make	-	Module Makefile (included from Makefile).
##
##	(c) Copyright 2002 by Nehal Mistry
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

MODULE = src/action
MSRC =	 action_attack.c action_board.c action_build.c action_demolish.c \
	 action_die.c action_follow.c action_harvest.c action_minegold.c \
	 action_move.c action_patrol.c action_repair.c action_research.c \
	 action_resource.c action_returngoods.c action_spellcast.c \
	 action_stand.c action_still.c action_train.c action_unload.c \
	 action_upgradeto.c actions.c command.c 

SRC+=	$(addprefix $(MODULE)/,$(MSRC))
HDRS+=

