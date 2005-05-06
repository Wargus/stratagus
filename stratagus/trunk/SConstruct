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
import glob
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

sources = []
sourceDirs = Split("action ai editor game map network pathfinder sound stratagus ui unit video")
for d in sourceDirs:
  sources.append(glob.glob('src/' + d + '/*.c'))
sources = Flatten(sources)
targetsources = []
for s in sources:
  targetsources.append('build' + s[3:])

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
  env.Append(CPPDEFINES = Split("USE_ZLIB"))

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
Default(env.Program('stratagus', targetsources))
env.Program('metaserver', sourcesMetaserver)


