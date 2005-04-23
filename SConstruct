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
##      SConstruct build file. See http://www.scons.org for info about scons.
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

import os
import sys
from stat import *

ccflags = "-fsigned-char"
customDefines = "USE_HP_FOR_XP MAP_REGIONS"

if os.path.exists("build_config.py")  \
     and os.stat("build_config.py")[ST_MTIME] < os.stat("SConstruct")[ST_MTIME]:
   # Remove outdated build_config.py
   os.remove("build_config.py")
opts = Options("build_config.py", ARGUMENTS)
opts.Add('CPPPATH', 'Additional preprocessor paths')
opts.Add('CPPFLAGS', 'Additional preprocessor flags')
opts.Add('CPPDEFINES', 'defined constants', Split(customDefines))
opts.Add('LIBPATH', 'Additional library paths')
opts.Add('LIBS', 'Additional libraries')
opts.Add('CCFLAGS', 'C Compiler flags', Split(ccflags))
opts.Add('CC', 'C Compiler')
opts.Add('debug', 'Build with debugging options', 0)
opts.Add('opengl', 'Build with opengl support', 0)
env = Environment() # for an unknown reason Environment(options=opts) doesnt work well
opts.Update(env) # Needed as Environment(options=opts) doesnt seem to work
Help(opts.GenerateHelpText(env))

sources = Split("""
build/ai/ai.c
build/ai/ai_magic.c
build/ai/ai_plan.c
build/ai/ai_force.c
build/ai/ai_resource.c
build/ai/script_ai.c
build/ai/ai_building.c
build/ui/ui.c
build/ui/interface.c
build/ui/botpanel.c
build/ui/menu_proc.c
build/ui/menus.c
build/ui/mouse.c
build/ui/button_checks.c
build/ui/script_ui.c
build/ui/icons.c
build/ui/mainscr.c
build/map/tileset.c
build/map/map.c
build/map/map_draw.c
build/map/map_save.c
build/map/minimap.c
build/map/script_map.c
build/map/map_wall.c
build/map/map_radar.c
build/map/map_fog.c
build/map/script_tileset.c
build/beos/beos.c
build/game/trigger.c
build/game/savegame.c
build/game/campaign.c
build/game/game.c
build/game/loadgame.c
build/game/intro.c
build/unit/script_unit.c
build/unit/upgrade.c
build/unit/unittype.c
build/unit/depend.c
build/unit/unit_draw.c
build/unit/unit_find.c
build/unit/unit.c
build/unit/script_unittype.c
build/unit/unit_cache.c
build/stratagus/script_missile.c
build/stratagus/pud.c
build/stratagus/script_spell.c
build/stratagus/spells.c
build/stratagus/mainloop.c
build/stratagus/missile.c
build/stratagus/construct.c
build/stratagus/groups.c
build/stratagus/script_player.c
build/stratagus/selection.c
build/stratagus/script.c
build/stratagus/stratagus.c
build/stratagus/util.c
build/stratagus/player.c
build/stratagus/iolib.c
build/sound/sound_server.c
build/sound/mad.c
build/sound/ogg.c
build/sound/wav.c
build/sound/script_sound.c
build/sound/sdl_audio.c
build/sound/sound.c
build/sound/cdda.c
build/sound/flac.c
build/sound/mikmod.c
build/sound/music.c
build/sound/unitsound.c
build/sound/libcda.c
build/sound/sound_id.c
build/sound/cdaudio.c
build/video/linedraw.c
build/video/mng.c
build/video/png.c
build/video/sdl.c
build/video/sprite.c
build/video/cursor.c
build/video/font.c
build/video/movie.c
build/video/graphic.c
build/video/video.c
build/action/action_build.c
build/action/action_research.c
build/action/command.c
build/action/action_attack.c
build/action/action_repair.c
build/action/action_returngoods.c
build/action/action_stand.c
build/action/action_still.c
build/action/action_unload.c
build/action/action_upgradeto.c
build/action/action_follow.c
build/action/action_train.c
build/action/action_patrol.c
build/action/action_spellcast.c
build/action/action_die.c
build/action/actions.c
build/action/action_resource.c
build/action/action_board.c
build/action/action_move.c
build/editor/editor.c
build/editor/edmap.c
build/editor/script_editor.c
build/editor/editloop.c
build/pathfinder/pathfinder.c
build/pathfinder/splitter_debug.c
build/pathfinder/splitter_zoneset.c
build/pathfinder/splitter_lowlevel.c
build/pathfinder/splitter.c
build/pathfinder/astar.c
build/pathfinder/script_pathfinder.c
build/network/commands.c
build/network/master.c
build/network/network.c
build/network/lowlevel.c
build/network/netconnect.c
""")

sourcesMetaserver = Split("""
 build/metaserver/cmd.c   
 build/metaserver/netdriver.c
 build/metaserver/main.c  
 build/metaserver/query.c
 build/network/lowlevel.c
""")

def CheckOpenGL(env, conf):
  opengl = {}
  opengl['linux'] = { 
      'LIBS': ['GL'], 
      'LIBPATH': ['/usr/lib', '/usr/X11R6/lib'],
      'CPPPATH': ['/usr/include']}
  opengl['cygwin'] = {
      'LIBS': ['opengl3']}
  platform = sys.platform
  if sys.platform[:5] == 'linux':
     platform = 'linux'
  for key in opengl[platform].keys():
      if key != 'LIBS':
         for i in opengl[platform][key]:
            env[key].append(i)
  for lib in opengl[platform]['LIBS']:
     if not conf.CheckLib('GL'):
         print("Can't find OpenGL libs. Exiting")
         sys.exit(1)
  env.Append(CPPDEFINES = 'USE_OPENGL')

def CheckLuaLib(env, conf):
  if env.WhereIs('lua-config'):
    env.ParseConfig('lua-config --include --libs')
  found = 0
  if conf.CheckLibWithHeader('lua', 'lua.h', 'c'):
    found = 1
  if not found and conf.CheckLibWithHeader('lua50', 'lua.h', 'c'):
    found =1
  if not found and conf.CheckLibWithHeader('lua5.0', 'lua.h', 'c'):
    found =1
  if not found:
    return 0

  if conf.CheckLibWithHeader('lualib', 'lualib.h', 'c'):
     return 1
  if conf.CheckLibWithHeader('lualib50', 'lualib.h', 'c'):
     return 1
  if conf.CheckLibWithHeader('lualib5.0', 'lualib.h', 'c'):
     return 1
  return 0

def AutoConfigure(env):
  # determine compiler and linker flags for SDL
  env.ParseConfig('sdl-config --cflags')
  env.ParseConfig('sdl-config --libs')
  conf = Configure(env)

  ## check for required libs ##
  if not conf.CheckLibWithHeader('SDL', 'SDL.h', 'c'):
     print 'Did not find SDL library or headers, exiting!'
     Exit(1)
  if not conf.CheckLibWithHeader('png', 'png.h', 'c'):
     print 'Did not find png library or headers, exiting!'
     Exit(1)
  if not conf.CheckLibWithHeader('z', 'zlib.h', 'c'):
     print 'Did not find the zlib library or headers, exiting!'
     Exit(1)
  if not conf.CheckLib('dl'):
     print 'Did not find dl library or header which is needed on some systems for lua. Exiting!'
     Exit(1)
  if not CheckLuaLib(env, conf):
     print 'Did not find required lua library. Exiting!'
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
  if env['opengl']:
     CheckOpenGL(env, conf)
  
  env = conf.Finish()

if not os.path.exists("build_config.py")  \
     or os.stat("build_config.py")[ST_MTIME] < os.stat("SConstruct")[ST_MTIME]:
    print "build_config.py doesn't exist or out of date."
    print "Generating new build config..."
    AutoConfigure(env)
    opts.Save("build_config.py", env)
else:
    print "Using build_config.py"

# Stratagus build specifics
env.Append(CPPPATH='src/include')
BuildDir('build', 'src', duplicate = 0)
if env['debug']:
    env.Append(CPPDEFINES = 'DEBUG')
    env.Append(CCFLAGS = Split('-g -Wsign-compare -Werror -Wall'))
else:
    env.Append(CCFLAGS = Split('-O2 -pipe -fomit-frame-pointer -fexpensive-optimizations -ffast-math'))
if not os.path.exists('config.h'):
    # create a dummy config.h needed by stratagus
    open('config.h', 'wt').close()

# Targets
Default(env.Program('stratagus', sources))
env.Program('metaserver', sourcesMetaserver)


