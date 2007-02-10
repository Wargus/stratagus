##     ____                _       __               
##    / __ )____  _____   | |     / /___ ___________
##   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
##  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
## /_____/\____/____/     |__/|__/\__,_/_/  /____/  
##                                              
##       A futuristic real-time strategy game.
##          This file is part of Bos Wars.
##
##      Module.make - Module Makefile (included from Makefile).
##
##      (c) Copyright 2004-2005 by The Stratagus Team
##
##      This program is free software; you can redistribute it and/or modify
##      it under the terms of the GNU General Public License as published by
##      the Free Software Foundation; only version 2 of the License.
##
##      This program is distributed in the hope that it will be useful,
##      but WITHOUT ANY WARRANTY; without even the implied warranty of
##      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##      GNU General Public License for more details.
##
##      You should have received a copy of the GNU General Public License
##      along with this program; if not, write to the Free Software
##      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

MODULE = src/include
MHDRS =  actions.h ai.h animation.h commands.h construct.h cursor.h depend.h \
         editor.h font.h stratagus.h icons.h interface.h iocompat.h iolib.h \
         map.h menus.h minimap.h missile.h movie.h myendian.h \
         net_lowlevel.h netconnect.h network.h pathfinder.h player.h \
         results.h script.h script_sound.h settings.h sound.h sound_server.h \
         spells.h tileset.h translate.h trigger.h ui.h unit.h unitsound.h \
         unittype.h upgrade.h upgrade_structs.h util.h video.h wav.h widgets.h

HDRS +=  $(addprefix $(MODULE)/,$(MHDRS))
