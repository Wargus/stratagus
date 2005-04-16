##       _________ __                 __
##      /   _____//  |_____________ _/  |______     ____  __ __  ______
##      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
##      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \
##     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
##             \/                  \/          \//_____/            \/
##  ______________________                           ______________________
##                        T H E   W A R   B E G I N S
##         Stratagus - A free fantasy real time strategy game engine
##
##      (c) Copyright 2005 by Francois Beerten
##
##      Stratagus is free software; you can redistribute it and/or modify
##      it under the terms of the GNU General Public License as published
##      by the Free Software Foundation; only version 2 of the License.
##
##      Stratagus is distributed in the hope that it will be useful,
##      but WITHOUT ANY WARRANTY; without even the implied warranty of
##      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##      GNU General Public License for more details.
##
##      $Id$
##

ccflags = "-O2 -pipe -fsigned-char -fomit-frame-pointer -fexpensive-optimizations -ffast-math "

env = Environment()
env.Append(CCFLAGS = ccflags)
env.Append(CPPDEFINES = Split("USE_HP_FOR_XP MAP_REGIONS"))

sources = Split("""
src/ai/ai.c
src/ai/ai_magic.c
src/ai/ai_plan.c
src/ai/ai_force.c
src/ai/ai_resource.c
src/ai/script_ai.c
src/ai/ai_building.c
src/ui/ui.c
src/ui/interface.c
src/ui/botpanel.c
src/ui/menu_proc.c
src/ui/menus.c
src/ui/mouse.c
src/ui/button_checks.c
src/ui/script_ui.c
src/ui/icons.c
src/ui/mainscr.c
src/map/tileset.c
src/map/map.c
src/map/map_draw.c
src/map/map_save.c
src/map/minimap.c
src/map/script_map.c
src/map/map_wall.c
src/map/map_radar.c
src/map/map_fog.c
src/map/script_tileset.c
src/beos/beos.c
src/game/trigger.c
src/game/savegame.c
src/game/campaign.c
src/game/game.c
src/game/loadgame.c
src/game/intro.c
src/unit/script_unit.c
src/unit/upgrade.c
src/unit/unittype.c
src/unit/depend.c
src/unit/unit_draw.c
src/unit/unit_find.c
src/unit/unit.c
src/unit/script_unittype.c
src/unit/unit_cache.c
src/stratagus/script_missile.c
src/stratagus/pud.c
src/stratagus/script_spell.c
src/stratagus/spells.c
src/stratagus/mainloop.c
src/stratagus/missile.c
src/stratagus/construct.c
src/stratagus/groups.c
src/stratagus/script_player.c
src/stratagus/selection.c
src/stratagus/script.c
src/stratagus/stratagus.c
src/stratagus/util.c
src/stratagus/player.c
src/stratagus/iolib.c
src/sound/sound_server.c
src/sound/mad.c
src/sound/ogg.c
src/sound/wav.c
src/sound/script_sound.c
src/sound/sdl_audio.c
src/sound/sound.c
src/sound/cdda.c
src/sound/flac.c
src/sound/mikmod.c
src/sound/music.c
src/sound/unitsound.c
src/sound/libcda.c
src/sound/sound_id.c
src/sound/cdaudio.c
src/video/linedraw.c
src/video/mng.c
src/video/png.c
src/video/sdl.c
src/video/sprite.c
src/video/cursor.c
src/video/font.c
src/video/movie.c
src/video/graphic.c
src/video/video.c
src/action/action_build.c
src/action/action_research.c
src/action/command.c
src/action/action_attack.c
src/action/action_repair.c
src/action/action_returngoods.c
src/action/action_stand.c
src/action/action_still.c
src/action/action_unload.c
src/action/action_upgradeto.c
src/action/action_follow.c
src/action/action_train.c
src/action/action_patrol.c
src/action/action_spellcast.c
src/action/action_die.c
src/action/actions.c
src/action/action_resource.c
src/action/action_board.c
src/action/action_move.c
src/editor/editor.c
src/editor/edmap.c
src/editor/script_editor.c
src/editor/editloop.c
src/pathfinder/pathfinder.c
src/pathfinder/splitter_debug.c
src/pathfinder/splitter_zoneset.c
src/pathfinder/splitter_lowlevel.c
src/pathfinder/splitter.c
src/pathfinder/astar.c
src/pathfinder/script_pathfinder.c
src/network/commands.c
src/network/master.c
src/network/network.c
src/network/lowlevel.c
src/network/netconnect.c
""")

sourcesMetaserver = Split("""
 src/metaserver/cmd.c   
 src/metaserver/netdriver.c
 src/metaserver/main.c  
 src/metaserver/query.c
 src/network/lowlevel.c
""")


# determine compiler and linker flags for SDL
env.ParseConfig('sdl-config --cflags')
env.ParseConfig('sdl-config --libs')


conf = Configure(env)

## check for required libs ##
if not conf.CheckLibWithHeader('SDL', 'SDL.h', 'c'):
    print 'Did not find SDL libraryor headers, exiting!'
    Exit(1)
if not conf.CheckLibWithHeader('png', 'png.h', 'c'):
    print 'Did not find png library or headers, exiting!'
    Exit(1)
if not conf.CheckLibWithHeader('z', 'zlib.h', 'c'):
    print 'Did not find the zlib library or headers, exiting!'
    Exit(1)
if not conf.CheckLib('dl'):
    print 'Did not find dl library which is needed on some systems for lua. Exiting!'
    Exit(1)
if not conf.CheckLibWithHeader('lua', 'lua.h', 'c'):
    print 'Did not find lua library or headers, exiting!'
    Exit(1)
if not conf.CheckLibWithHeader('lualib', 'lualib.h', 'c'):
    print 'Did not find lualib library of headers, exiting!'
    Exit(1)
# stratagus defines for required libraries
env.Append(CPPDEFINES = Split("USE_ZLIB USE_SDL"))

# Check for optional libraries #
if conf.CheckLib('bz2'):
    env.Append(CPPDEFINES = 'USE_BZLIB2')
if conf.CheckLib('ogg'):
    env.Append(CPPDEFINES = 'USE_OGG')
if conf.CheckLib('vorbis'):
    env.Append(CPPDEFINES = 'USE_VORBIS')
if conf.CheckLib('theora'):
    env.Append(CPPDEFINES = 'USE_THEORA')
if conf.CheckLib('mikmod'):
    env.Append(CPPDEFINES = 'USE_MIKMOD')
if conf.CheckLib('mad'):
    env.Append(CPPDEFINES = 'USE_MAD')
if conf.CheckLib('flac'):
    env.Append(CPPDEFINES = 'USE_FLAC')
env = conf.Finish()


# Stratagus build 
env.Append(CPPPATH='src/include')

# Targets
env.Program('stratagus', sources)
env.Default('stratagus')

env.Program('metaserver', sourcesMetaserver)


