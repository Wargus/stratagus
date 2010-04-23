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

class Gcc(object):
  def __init__(self, cflags=[], ldflags=[], cc='g++', builddir='fbuild'):
     self.cflags = gccflags + list(cflags)
     self.ldflags = list(ldflags)
     self.cc = cc
     self.builddir = builddir
  def copy(self):
     g = Gcc(self.cflags, self.ldflags, self.cc, self.builddir)
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
  def __init__(self, cflags=[], ldflags=[], cc='cl.exe', builddir='fbuild/release'):
     self.cflags = ['/Wall', '/DUSE_WIN32'] + list(cflags)
     self.ldflags = list(ldflags)
     self.cc = 'cl.exe'
     self.linker = 'link.exe'
     self.builddir = builddir
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

def Check(b, lib=None, header='', function='', name=''):
    t = b.copy()
    name = name or lib or function or header.replace('/','_')
    testdir = b.builddir + '/conftests/'
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

def detectAlwaysDynamic(b):
    RequireLib(b, 'png', 'png.h')
    RequireLib(b, 'z', 'zlib.h')
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

def detectEmbedable(b):
    detectLua(b)
    if CheckLib(b, 'vorbis'):
       b.define('USE_VORBIS')
    if CheckLib(b, 'theora'):
       b.define('USE_THEORA')
    if CheckLib(b, 'ogg'):
       b.define('USE_OGG')

def detect(b):
    detectAlwaysDynamic(b)
    detectEmbedable(b)

def compile(b):
    objects = [b.cxx(inBuildDir(s, b.builddir), s) for s in sources]
    return objects

def link(b):
    objects = [b.oname(inBuildDir(s, b.builddir)) for s in sources]
    apptarget = b.builddir + '/' + target
    b.ld(apptarget, objects)

def make(b):
    compile(b)
    link(b)

def release(builddir='fbuild/release',**kwargs):
    b = compiler(builddir=builddir, **kwargs)
    mkdir(builddir)
    detect(b)
    b.optimize()
    make(b)

def debug(builddir='fbuild/debug',**kwargs):
    b = compiler(builddir=builddir, **kwargs)
    mkdir(builddir)
    detect(b)
    b.debug()
    b.define('DEBUG')
    make(b)

def profile(builddir='fbuild/profile',**kwargs):
    b = compiler(builddir=builddir, **kwargs)
    mkdir(builddir)
    detect(b)
    b.profile()
    make(b)

class StaticGcc(Gcc):
    def __init__(self,*args,**kwargs):
        super(StaticGcc,self).__init__(*args,**kwargs)
    def lib(self, *names):
        self.ldflags += ['-Wl,-Bstatic']
        super(StaticGcc, self).lib(*names)
        self.ldflags += ['-Wl,-Bdynamic']
    def  copy(self):
        return StaticGcc(self.cflags,self.ldflags,self.cc,self.builddir)

def static(builddir='fbuild/static', **kwargs):
    b = compiler(builddir=builddir, **kwargs)
    b.incpath('engine/apbuild')
    b.incpath('deps/incs')
    b.libpath('deps/libs')
    b.cflags += [
        '-fno-stack-protector', # disable stack protection to avoid dependency on
                                                #__stack_chk_fail@@GLIBC_2.4
        '-U_FORTIFY_SOURCE', # requires glibc 2.3.4 or higher
        '-include', 'engine/apbuild/apsymbols.h',
       ]
    b.ldflags += [
        '-Wl,--as-needed',  # must be set after all object files or the binary breaks
        '-Wl,--hash-style=both', # By default FC6 only generates a .gnu.hash section
                              # Do all main distros support .gnu.hash now ?
        '-static-libgcc',
        '-Wl,-Bstatic', '-lstdc++', '-Wl,-Bdynamic',
        '-Wl,-s'
        ]
    p = os.popen(b.cc + ' -print-file-name=libstdc++.a')
    stdcxx = p.read().strip()
    run('ln', '-sf', stdcxx)
    
    mkdir(builddir)
    detectAlwaysDynamic(b)
    b = StaticGcc(b.cflags,b.ldflags, b.cc,b.builddir)
    detectEmbedable(b)
    b.optimize()
    b.libpath('.')
    p = os.popen(b.cc + ' -print-file-name=libstdc++.a')
    make(b)

def clean():
    autoclean()

def pot():
    luas = find('.', '*.lua')
    luas.sort()
    run('xgettext','-d','bos','-k_','-o','languages/bos.pot', luas)
    s = sorted(sources)
    run('xgettext','-d','engine','-k_','-o','languages/engine.pot', s)

def all(**kwargs):
    release(**kwargs)
    debug(**kwargs)

def default(**kwargs):
    release(**kwargs)

setup(default='default')
if __name__ == '__main__':
    main()

