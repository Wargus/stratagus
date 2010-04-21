#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  Build script for the Bos Wars engine.
#
#  (c) Copyright 2010 by Francois Beerten.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import os
import glob
import sys
from fabricate import *


target = 'boswars'
gccflags = '-Wall -fsigned-char -D_GNU_SOURCE=1 -D_REENTRANT'.split()
incpaths = 'engine/include engine/guichan/include /usr/include/SDL'.split()

def find(startdir, pattern):
    import fnmatch
    results = []
    for dirpath,dirnames,files in os.walk(startdir):
       for f in files:
          if fnmatch.fnmatch(f, pattern):
             results.append(dirpath+'/'+f)
    return results
    
sources = find('engine', '*.cpp')

class Gcc:
  def __init__(self, cflags=[], ldflags=[], cc='g++'):
     self.cflags = gccflags + list(cflags)
     self.ldflags = list(ldflags)
     self.cc = cc
  def copy(self):
     g = Gcc(self.cflags, self.ldflags, self.cc)
     return g
  def oname(self, source):
     base = os.path.splitext(source)[0]
     return '%s.o' % (base)
  def lib(self, *names):
     self.ldflags += ['-l'+ i for i in names]
  def incpath(self, *names):
     self.cflags += ['-I'+i for i in names]
  def libpath(self, *names):
     self.ldflags += ['-L'+i for i in names]
  def define(self,name, value=''):
     if value:
       name += '='+value
     self.cflags += ['-D' + name]
  def debug(self):
     self.cflags += ['-g', '-Werror']
  def optimize(self, level=2):
     self.cflags += ['-O%d'%level]
  def profile(self):
     self.cflags += ['-pg']
     self.ldflags += ['-pg']
  def cxx(self, target, source, flags=[]):
     cflags = flags + self.cflags
     target = self.oname(target)
     run(self.cc, '-c', source, '-o', self.oname(target), cflags)
     return target
  def ld(self, target, sources, flags=[]):
     ldflags = flags + self.ldflags
     objects = [self.oname(s) for s in sources]
     run(self.cc, objects, '-o', target, ldflags)
  def build(self, target, sources, flags=[]):
     flags = [] + flags + self.ldflags + self.cflags
     run(self.cc, sources, '-o', target, flags)

class Msvc:
  ''' UNTESTED !!!! '''
  def __init__(self, cflags=[], ldflags=[], cc='cl.exe'):
     self.cflags = ['/Wall', '/DUSE_WIN32'] + list(cflags)
     self.ldflags = list(ldflags)
     self.cc = 'cl.exe'
     self.linker = 'link.exe'
  def copy(self):
     g = Msvc(self.cflags, self.ldflags, self.cc)
     return g
  def normalize(self, path):
     return path.replace('/','\\')
  def oname(self, source):
     base = os.path.splitext(source)[0]
     return '%s.obj' % (base)
  def lib(self, name):
     self.ldflags += [self.normalize(name)+'.lib']
  def incpath(self, name):
     self.cflags += ['/I', self.normalize(name)]
  def libpath(self, name):
     self.ldflags += ['/LIBPATH:' + self.normalize(name)]
  def define(self,name, value=''):
     if value:
       name += '='+value
     self.cflags += ['/D'+name]
  def debug(self):
     self.ldflags += ['/Zi']
     self.ldflags += ['/DEBUG']
  def optimize(self, level=2):
     self.cflags += ['/O2']
  def profile(self):
     raise Exception('Not supported')
  def cxx(self, target, sources, flags=[]):
     cflags = []+flags + self.cflags
     target = self.normalize(self.oname(target))
     sources = [self.normalize(s) for s in sources]
     run(self.cc, '/c', s, '/OUT', target, cflags)
     return target
  def ld(self, target, sources, flags=[]):
     ldflags = flags + self.ldflags
     objects = [self.normalize(self.oname(s)) for s in sources]
     run(self.linker, objects, '/OUT:'+target+'.exe', ldflags)
  def build(self, target, sources, flags=[]):
     obj = self.normalize(self.oname(sources[0]))
     self.cxx(obj, sources, flags)
     self.ld(self.normalize(target), obj, flags)

def compiler(**kwargs):
    b = Gcc(**kwargs)
    if Check(b, name='gcc'):
        return b
    b = Msvc(**kwargs)
    if Check(b, name='msvc'):
        return b
    return None


def inBuildDir(source, builddir):
    d = builddir+'/'+os.path.basename(source)
    return d

def mkdir(dir):
    if not os.access(dir, os.F_OK):
        os.makedirs(dir)

def Check(b, lib=None, header='', function='', name='', 
       builddir='fbuild/conftests'):
    t = b.copy()
    name = name or lib or function or header.replace('/','_')
    testdir = 'fbuild/conftests/'
    testname = testdir+'test'+name
    s = testname+'.c'
    mkdir(testdir)
    f = open(s, 'wt')
    if header:
       f.write('#include "%s"\n' % header)
    if function:
       f.write('#ifdef __cplusplus\nextern "C"\n#endif\nchar %s();\n'%function)
    f.write('int main()\n{\nreturn 0;\n}\n\n')
    f.close()
    if lib:
        t.lib(lib)
    try:
       t.build(testname, [s])
    except ExecutionError, e:
       return False
    return True

def CheckLib(b, lib, header=''):
    if Check(b, lib, header):
       b.lib(lib)
       return True
    return False

def RequireLib(b, lib, header=''):
    r = CheckLib(b, lib, header)
    if not r:
       print(
         'Did not find the required %s lib or headers, exiting!' % lib)
       sys.exit(1)

def CheckLibAlternatives(b, libs, header=''):
    for i in libs:
       if CheckLib(b, i, header):
         return True

def detectLua(b):
    libs = 'lua lua5.1 lua51'.split()
    r = CheckLibAlternatives(b, libs, header='lua.h')
    if not r:
       print('Did not find the Lua library, exiting !')
       sys.exit(1)

def detectOpenGl(b):
    libs = 'GL opengl3 opengl32'.split()
    if sys.platform == 'darwin':
        b.incpath('/System/Library/Frameworks/OpenGL.framework/Libraries/')
    r = CheckLibAlternatives(b, libs)
    if not r:
       print('Did not find the OpenGL library, exiting !')
       sys.exit(1)

def detect(b):
    RequireLib(b, 'png', 'png.h')
    RequireLib(b, 'z', 'zlib.h')
    detectLua(b)
    detectOpenGl(b)
    RequireLib(b, 'SDL')
    if Check(b, function='strcasestr'):
       b.define('HAVE_STRCASESTR')
    if Check(b, function='strnlen'):
       b.define('HAVE_STRNLEN')
    if (Check(b, header='X11/Xlib.h') and 
        Check(b, header='X11/Xatom.h') and 
        Check(b, 'X11')):
       b.define('HAVE_X')
    for i in incpaths:
       b.incpath(i)
    if CheckLib(b, 'vorbis'):
       b.define('USE_VORBIS')
    if CheckLib(b, 'theora'):
       b.define('USE_THEORA')
    if CheckLib(b, 'ogg'):
       b.define('USE_OGG')

def compile(b, builddir='fbuild/release'):
    objects = [b.cxx(inBuildDir(s, builddir), s) for s in sources]
    return objects

def link(b, builddir='fbuild/release'):
    objects = [b.oname(inBuildDir(s, builddir)) for s in sources]
    apptarget = builddir + '/' + target
    b.ld(apptarget, objects)

def make(b, builddir):
    mkdir(builddir)
    compile(b, builddir)
    link(b, builddir)

def release(**kwargs):
    b = compiler(**kwargs)
    detect(b)
    b.optimize()
    make(b, 'fbuild/release')

def debug(**kwargs):
    b = compiler(**kwargs)
    detect(b)
    b.debug()
    b.define('DEBUG')
    make(b, 'fbuild/debug')

def profile(**kwargs):
    b = compiler(**kwargs)
    detect(b)
    b.profile()
    make(b, 'fbuild/profile')

def static(**kwargs):
    b = compiler(**kwargs)
    b.incpath('deps/incs')
    b.libpath('deps/libs')
    detect(b)
    staticlibs = 'lua lua5.1 lua51 vorbis theora ogg'.split()
    for i in staticlibs:
       lib = '-l'+i
       if lib in b.ldflags:
           b.ldflags.remove(lib)
    b.cflags.remove('-DUSE_THEORA')
    b.cflags.remove('-DUSE_VORBIS')
    b.cflags.remove('-DUSE_OGG')
    b.ldflags += ['-Wl,-Bstatic', '-lstdc++', '-llua',
          '-Wl,-Bdynamic']
    b.cc = 'apg++'
    b.optimize()
    b.libpath('.')
    b.ldflags += ['-static-libgcc']
    p = os.popen(b.cc + ' -print-file-name=libstdc++.a')
    stdcxx = p.read().strip()
    run('ln', '-sf', stdcxx)
    make(b, 'fbuild/static')
    run('strip', 'fbuild/static/boswars')

def clean():
    autoclean()

def pot():
    luas = find('.', '*.lua')
    luas.sort()
    run('xgettext','-d','bos','-k_','-o','languages/bos.pot',
         luas)
    s = list(sources)
    s.sort()
    run('xgettext','-d','engine','-k_','-o','languages/engine.pot',
         s)
def po():
    pass

def all(**kwargs):
    release(**kwargs)
    debug(**kwargs)

def default(**kwargs):
    release(**kwargs)

setup(default='default')
if __name__ == '__main__':
    main()

