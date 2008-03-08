##     ____                _       __               
##    / __ )____  _____   | |     / /___ ___________
##   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
##  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
## /_____/\____/____/     |__/|__/\__,_/_/  /____/  
##                                              
##       A futuristic real-time strategy game.
##          This file is part of Bos Wars.
##
##      SConstruct build file. See http://www.scons.org for info about scons.
##      (c) Copyright 2005-2008 by Francois Beerten
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
##

import os
import sys
import glob
from stat import *
import filecmp

ccflags = "-fsigned-char"
SConsignFile()

def DefineOptions(filename, args):
   opts = Options(filename, args)
   opts.Add('CPPPATH', 'Additional preprocessor paths', ['/usr/local/include'])
   opts.Add('CPPFLAGS', 'Additional preprocessor flags')
   opts.Add('CPPDEFINES', 'defined constants', Split(''))
   opts.Add('LIBPATH', 'Additional library paths', ['/usr/local/lib'])
   opts.Add('LIBS', 'Additional libraries')
   opts.Add('CCFLAGS', 'C Compiler flags', Split(ccflags))
   opts.Add('LINKFLAGS', 'Linker Compiler flags')
   opts.Add('CC', 'C Compiler')
   opts.Add('CXX', 'C++ Compiler')
   opts.Add('LINK', 'Linker')
   opts.Add('extrapath', 'Path to extra root directory for includes and libs', '')
   opts.Add('MINGWCPPPATH', 'Additional include path for crosscompilation', [])
   opts.Add('MINGWLIBPATH', 'Additional lib path for crosscompilation', [])
   return opts


opts = DefineOptions("build_options.py", ARGUMENTS)
env = Environment(ENV = {'PATH':os.environ['PATH']}) # for an unknown reason Environment(options=opts) doesnt work well
opts.Update(env) # Needed as Environment(options=opts) doesnt seem to work
Help(opts.GenerateHelpText(env))
mingw = env.Copy()
optionsChanged = True
if os.path.exists('build_options.py'):
   os.rename('build_options.py', 'build_options_OLD_FOR_CHECK.py')
   optionsChanged = False
opts.Save("build_options.py", env)
if not optionsChanged:
   optionsChanged  = not filecmp.cmp('build_options.py', 'build_options_OLD_FOR_CHECK.py', False)
   os.remove('build_options_OLD_FOR_CHECK.py')

engineSourceDir = 'engine'

def globSources(sourceDirs, builddir):
  sources = []
  sourceDirs = Split(sourceDirs)
  for d in sourceDirs:
    sources.append(glob.glob(engineSourceDir + '/' + d + '/*.cpp'))
  sources = Flatten(sources)
  targetsources = []
  for s in sources:
    targetsources.append(builddir + s[len(engineSourceDir):])
  return targetsources

def buildSourcesList(builddir):
   sources = globSources("action ai editor game map network particle pathfinder sound stratagus ui unit video tolua", builddir)
   sources.append(globSources("guichan guichan/sdl guichan/widgets", builddir))
   return sources
sourcesEngine = buildSourcesList('build')

def ParseConfig(env, command, function=None):
    import string

    """
    Copied from the scons, copyright 2001-2004 The SCons Foundation.
    Adapted by Francois Beerten to use the exit value of pkg-config.
    """
    # the default parse function
    def parse_conf(env, output):
        dict = {
           'ASFLAGS'       : [],
           'CCFLAGS'       : [],
           'CPPFLAGS'      : [],
           'CPPPATH'       : [],
           'LIBPATH'       : [],
           'LIBS'          : [],
           'LINKFLAGS'     : [],
        }
        static_libs = []

        params = string.split(output)
        for arg in params:
            if arg[0] != '-':
                static_libs.append(arg)
            elif arg[:2] == '-L':
                dict['LIBPATH'].append(arg[2:])
            elif arg[:2] == '-l':
                dict['LIBS'].append(arg[2:])
            elif arg[:2] == '-I':
                dict['CPPPATH'].append(arg[2:])
            elif arg[:4] == '-Wa,':
                dict['ASFLAGS'].append(arg)
            elif arg[:4] == '-Wl,':
                dict['LINKFLAGS'].append(arg)
            elif arg[:4] == '-Wp,':
                dict['CPPFLAGS'].append(arg)
            elif arg == '-pthread':
                dict['CCFLAGS'].append(arg)
                dict['LINKFLAGS'].append(arg)
            else:
                dict['CCFLAGS'].append(arg)
        apply(env.Append, (), dict)
        return static_libs

    if function is None:
        function = parse_conf
    if type(command) is type([]):
        command = string.join(command)
    command = env.subst(command)
    _, f, _ = os.popen3(command)
    read = f.read()
    exitcode = f.close()
    if exitcode == None:
        return (0, function(env, read))
    else:
        return (exitcode, [])

def CheckOpenGL(env, conf):
  opengl = {}
  opengl['linux'] = { 
      'LIBS': ['GL'], 
      'LIBPATH': ['/usr/lib', '/usr/X11R6/lib'],
      'CPPPATH': ['/usr/include']}
  opengl['cygwin'] = {
      'LIBS': ['opengl3']}
  opengl['darwin'] = {
      'LIBS': ['GL'],
      'LIBPATH': ['/System/Library/Frameworks/OpenGL.framework/Libraries/']}
  platform = sys.platform
  if 'USE_WIN32' in env['CPPDEFINES']:
     glconfig = {'LIBS': ['opengl32']}
  else:
     if sys.platform[:5] == 'linux':
        platform = 'linux'
     glconfig = opengl.get(platform, {})
  for key in glconfig:
      if key != 'LIBS':
         for i in opengl[platform][key]:
            env[key].append(i)
  if 'LIBS' in glconfig:
     for lib in glconfig['LIBS']:
         if not conf.CheckLib(lib):
             print("Can't find OpenGL libs. Exiting")
             sys.exit(1)
  return True

def CheckLuaLib(env, conf):
  if not 'USE_WIN32' in env['CPPDEFINES']:
     if env.WhereIs('pkg-config'):
        for packagename in ['lua5.1', 'lua51', 'lua']:
           exitcode, _ = ParseConfig(env, 'pkg-config --cflags --libs ' + packagename)
           if exitcode == 0:
              break
  if conf.CheckLibWithHeader('lua51', 'lua.h', 'c'):
    return 1
  if conf.CheckLibWithHeader('lua5.1', 'lua.h', 'c'):
    return 1
  if not conf.CheckLibWithHeader('lua', 'lua.h', 'c'):
    return 0
  # make sure we have lualib which is included in lua 5.1
  if conf.CheckFunc('luaopen_base'):
     return 1
  return 0

def AutoConfigure(env):
  conf = Configure(env)

  ## check for required libs ##
  if not conf.CheckLibWithHeader('png', 'png.h', 'c'):
     print 'Did not find png library or headers, exiting!'
     Exit(1)
  if not conf.CheckLibWithHeader('z', 'zlib.h', 'c'):
     print 'Did not find the zlib library or headers, exiting!'
     Exit(1)
  if not 'USE_WIN32' in env['CPPDEFINES'] and not sys.platform.startswith('freebsd'):
     if not conf.CheckLib('dl'):
        print 'Did not find dl library or header which is needed on some systems for lua. Exiting!'
        Exit(1)
  if not CheckLuaLib(env, conf):
     print 'Did not find required lua library. Exiting!'
     Exit(1)
  if not CheckOpenGL(env, conf):
     print 'Did not find required OpenGL library. Exiting!'
     Exit(1)

  # Check for optional libraries #
  if conf.CheckLib('vorbis'):
     env.Append(CPPDEFINES = 'USE_VORBIS')
  if conf.CheckLib('theora'):
     env.Append(CPPDEFINES = 'USE_THEORA')
  if conf.CheckLib('ogg'):
     env.Append(CPPDEFINES = 'USE_OGG')

  # check for optional functions
  if conf.CheckFunc('strcasestr'):
     env.Append(CPPDEFINES = 'HAVE_STRCASESTR')
  if conf.CheckFunc('strnlen'):
     env.Append(CPPDEFINES = 'HAVE_STRNLEN')

  # check for optional headers
  if (conf.CheckHeader('X11/Xlib.h') and conf.CheckHeader('X11/Xatom.h') and
     conf.CheckLib('X11')):
     env.Append(CPPDEFINES = 'HAVE_X')

  # Determine compiler and linker flags for SDL. Must be done at the end 
  # as on some platforms, SDL redefines main which conflicts with the checks.
  if not 'USE_WIN32' in env['CPPDEFINES']:
    env.ParseConfig('sdl-config --cflags')
    env.ParseConfig('sdl-config --libs')
    if sys.platform != "darwin" and not '-Dmain=SDL_main' in env['CCFLAGS']:
       if not conf.CheckLibWithHeader('SDL', 'SDL.h', 'c'):
          print 'Did not find SDL library or headers, exiting!'
          Exit(1)

  env = conf.Finish()

def AutoConfigureIfNeeded(env, name):
   cachename = "build_conf_%scache.py" % name
   if os.path.exists(cachename):  
      if optionsChanged or \
         os.stat(cachename)[ST_MTIME] < os.stat("SConstruct")[ST_MTIME]:
            # Remove outdated cache file
            os.remove(cachename)
   if optionsChanged or not os.path.exists(cachename):
      print cachename + " doesn't exist or out of date."
      print "Generating new build config cache ..."
      cache = DefineOptions(cachename, {})
      AutoConfigure(env)
      cache.Save(cachename, env)
   else:
      cache = DefineOptions(cachename, {})
      print "Using " + cachename
      cache.Update(env)

AutoConfigureIfNeeded(env, '')

def addBosWarsPaths(env):
   # Stratagus build specifics
   env.Append(CPPPATH=engineSourceDir+'/include')
   env.Append(CPPPATH=engineSourceDir+'/guichan/include')
addBosWarsPaths(env)

# define the different build environments (variants)
release = env.Copy()
release.Append(CCFLAGS = Split('-O2 -pipe -fomit-frame-pointer -fexpensive-optimizations -ffast-math'))

if mingw['extrapath']:
  mingw.Tool('crossmingw', toolpath = ['tools/scons/'])
  mingw['CPPDEFINES'] = ['USE_WIN32']
  if mingw['extrapath'] != '':
     x = mingw['extrapath']
     mingw['CPPPATH'] = [x + '/include']
     mingw['LIBPATH'] = [x + '/lib']
  mingw.Append(CPPPATH = mingw['MINGWCPPPATH'])
  mingw.Append(LIBPATH = mingw['MINGWLIBPATH'])
  AutoConfigureIfNeeded(mingw, 'mingw')
  addBosWarsPaths(mingw)
  mingw.Append(LIBS = ['mingw32', 'SDLmain', 'SDL', 'wsock32', 'ws2_32'])
  mingw.Append(LINKFLAGS = ['-mwindows'])
else:
  mingw = None

debug = env.Copy()
debug.Append(CPPDEFINES = 'DEBUG')
debug.Append(CCFLAGS = Split('-g -Wsign-compare -Wall -Werror'))

profile = debug.Copy()
profile.Append(CCFLAGS = Split('-pg'))
profile.Append(LINKFLAGS = Split('-pg'))

staticenv = None
if sys.platform.startswith('linux'):
   staticenv = release.Copy()
   staticlibs = 'lua lua50 lua5.0 lua5.1 lua51 lualib lualib50 lualib5.0 vorbis theora ogg'
   staticlibs = staticlibs.split(' ')
   linkflags = '-L. -static-libgcc -Wl,-Bstatic -lstdc++ '
   for i in staticlibs:
       if i in staticenv['LIBS']:
           linkflags += '-l%s ' % i
           staticenv['LIBS'].remove(i)
   linkflags += '-Wl,-Bdynamic'
   # To successfully link with static libraries with GCC 4.1.2,
   # the static libs must be at the end. This trick enforces it.
   staticenv['STATICLINKFLAGS'] = linkflags.split()
   staticenv['LINKCOM'] += ' $STATICLINKFLAGS'

# Targets
def DefineVariant(venv, v, vv = None):
   if vv == None:
      vv = '-' + v
   BuildDir('build/' + v, engineSourceDir, duplicate = 0)
   r = venv.Program('boswars' + vv, buildSourcesList('build/' + v))
   Alias(v, 'boswars' + vv)
   return r 

r = DefineVariant(release, 'release', '')
Default(r)
DefineVariant(debug, 'debug')
DefineVariant(profile, 'profile')
if staticenv:
   stdlibcxx = staticenv.Command('libstdc++.a', None, 
          Action('ln -sf `%s -print-file-name=libstdc++.a`' % staticenv['CXX']))
   # staticenv.Append(LIBS = [stdlibcxx])   <= does not work with scons 0.96.1
   prog = DefineVariant(staticenv, 'static')
   staticenv.Depends(prog, stdlibcxx)
if mingw:
   DefineVariant(mingw, 'mingw', '.exe')

