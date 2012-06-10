#! /usr/bin/env python
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

# This script is compatible with both Python 2 and Python 3.

import os
import glob
import sys
try:
   import Queue as queue
except ImportError:
   import queue
import threading
from warnings import warn
from fabricate import *


target = 'boswars'
gccflags = '-Wall -fsigned-char -D_GNU_SOURCE=1 -D_REENTRANT'.split()
incpaths = 'engine/include engine/guichan/include'.split()

def find(startdir, pattern):
    import fnmatch
    results = []
    for dirpath,dirnames,files in os.walk(startdir):
       for f in files:
          # Ignore dot files.  GNU Emacs especially creates lock files
          # with names like ".#script.cpp".  Don't try to compile those.
          if fnmatch.fnmatch(f, pattern) and not f.startswith("."):
             results.append(dirpath+'/'+f)
    return results
    
sources = find('engine', '*.cpp')

class Gcc(object):
  def __init__(self, cflags=[], ldflags=[], cc='g++', builddir='fbuild', 
                      usepkgconfig=True):
     self.cflags = gccflags + list(cflags)
     self.ldflags = list(ldflags)
     self.cc = cc
     self.builddir = builddir
     self.usepkgconfig = usepkgconfig
  def copy(self):
     g = Gcc(self.cflags, self.ldflags, self.cc, self.builddir, self.usepkgconfig)
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
     return (self.cc, '-c', source, '-o', self.oname(target), cflags)
  def ld(self, target, sources, flags=[]):
     ldflags = flags + self.ldflags
     objects = [self.oname(s) for s in sources]
     return (self.cc, objects, '-o', target, ldflags)
  def build(self, target, sources, flags=[]):
     flags = [] + flags + self.ldflags + self.cflags
     return (self.cc, sources, '-o', target, flags)

def compiler(**kwargs):
    return Gcc(**kwargs)


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
       run(*t.build(testname, [s]))
    except ExecutionError as e:
       return False
    return True

def cmdline2list(s):
    r"""Split a string into words and remove quotation marks and backslashes,
as a Bourne shell would.

If e.g. pkg-config --cflags lua5.1 outputs:
-DDEB_HOST_MULTIARCH=\"x86_64-linux-gnu\" -I/usr/include/lua5.1  
this function can convert it to a list of words:
('-DDEB_HOST_MULTIARCH="x86_64-linux-gnu"', '-I/usr/include/lua5.1')
that you can then pass to subprocess.list2cmdline and get the
backslashes back.

This function neither parses nor executes any shell expansions.  For
example, it does not support the ${variable} or `command` syntaxes.
If it sees any of those, it warns and passes the metacharacters
through unchanged."""
    words = ()
    pos = 0
    # Read words from the string until we get to the end.
    while pos < len(s):
        if s[pos].isspace():
            # Spaces delimit words.  Discard them.
            pos += 1
        else:
            # Any non-space character begins a word.
            word = ""
            if pos < len(s) and s[pos] == '~':
                # An unquoted tilde at the beginning of a word should
                # refer to a home directory or to a directory stack.
                # We support neither.
                warn("tilde expansion not supported")
            while pos < len(s) and not s[pos].isspace():
                if s[pos] == '\\':
                    # An unquoted backslash escapes the following character.
                    pos += 1
                    if pos >= len(s):
                        warn("trailing backslash")
                    else:
                        word += s[pos]
                        pos += 1
                    continue
                if s[pos] == '"':
                    # A double-quoted string.  Read characters until we get
                    # to the closing double-quotation mark.
                    pos += 1
                    while True:
                        if pos >= len(s):
                            warn("unterminated double-quoted string")
                            break
                        elif s[pos] == '"':
                            # This closes the string.
                            pos += 1
                            break
                        elif s[pos] == '\\':
                            # Within a double-quoted string, a
                            # backslash followed by another backslash,
                            # double-quotation mark, dollar sign, or
                            # backtick escapes that character.
                            # Otherwise, it's just part of the string.
                            if pos + 1 >= len(s):
                                warn("trailing backslash in double-quoted string")
                            elif s[pos + 1] in '\\"$`':
                                word += s[pos + 1]
                                pos += 2
                                continue
                        elif s[pos] in "$`":
                            # Within a double-quoted string, a dollar
                            # sign or a backtick would begin some
                            # shell-expansion syntax that this
                            # function does not support.
                            warn("unsupported character %s in double-quoted string" % s[pos])
                        # Otherwise, the character is part of the string.
                        word += s[pos]
                        pos += 1
                    continue
                elif s[pos] == "'":
                    # A single-quoted string.  Backslashes have
                    # no special meaning here.  Just search for
                    # the closing single-quotation mark.
                    pos += 1
                    while True:
                        if pos >= len(s):
                            warn("unterminated single-quoted string")
                            break
                        elif s[pos] == "'":
                            # This closes the string.
                            pos += 1
                            break
                        # Otherwise, the character is part of the string.
                        word += s[pos]
                        pos += 1
                    continue
                elif s[pos] in "#$&()*;<>?[]`|":
                    # When not part of a {double,single}-quoted
                    # string, these characters invoke various shell
                    # features that this function does not support.
                    warn("unsupported character %s" % s[pos])
                word += s[pos]
                pos += 1
            # Finished reading the word; reached a space or the end of
            # the input string.
            words += (word,)
    return words

def pkgconfig(b, package):
    try:
        b.cflags += cmdline2list(shell('pkg-config', '--cflags', package).decode())
        b.ldflags += cmdline2list(shell('pkg-config', '--libs', package).decode())
    except ExecutionError as e:
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
    if CheckLibAlternatives(b, libs, header='lua.h'):
        return
    if b.usepkgconfig:
        for i in libs:
            if pkgconfig(b, i):
                return
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

def detectSdl(b):
    b.incpath('/usr/include/SDL')
    header = 'SDL.h'
    if '-DUSE_WIN32' in b.cflags:
        header = ''
    if CheckLib(b, 'SDL', header=header):
        return
    if not b.usepkgconfig or not pkgconfig(b, 'sdl'):
       print('Did not find the SDL library, exiting !')
       sys.exit(1)

def detectAlwaysDynamic(b):
    RequireLib(b, 'png', 'png.h')
    RequireLib(b, 'z', 'zlib.h')
    detectOpenGl(b)
    detectSdl(b)
    if Check(b, function='strcasestr'):
       b.define('HAVE_STRCASESTR')
    if Check(b, function='strnlen'):
       b.define('HAVE_STRNLEN')
    if (Check(b, header='X11/Xlib.h') and 
        Check(b, header='X11/Xatom.h') and 
        CheckLib(b, 'X11')):
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

def parallel_run(commands, jobs=2, builder=default_builder):
        """ The different commands must be independent. """
        requests, results = queue.Queue(), queue.Queue()
        for i in commands:
            c = builder.prepare(i)
            if builder.should_run(*c):
                requests.put(c)
        totalrequests = requests.qsize()
        requests.active = True

        def runQueuedCommands(requests, results):
            while requests.active:
                c = requests.get()
                if c is None:
                    return
                arglist, command = c
                builder.echo_command(command)
                try:
                     deps, outputs = builder.runner(*arglist)
                     results.put((command, deps, outputs))
                except ExecutionError as e:
                     results.put(e)
                     requests.active = True  # abort future requests

        for i in range(jobs):
            t = threading.Thread(target=runQueuedCommands, args=(requests, results))
            t.daemon = True
            requests.put(None)
            t.start()
        for _ in range(totalrequests):
           r = results.get()
           if isinstance(r, ExecutionError): raise r
           command, deps, outputs = r
           builder.store_deps(command, deps, outputs)

def runall(commands, jobs=None):
    jobs= jobs or int(main.options.jobs)
    if jobs > 1:
        parallel_run(commands)
    else:
        for i in commands:
           run(*i)

def compile(b):
    commands = [b.cxx(inBuildDir(s, b.builddir), s) for s in sources]
    runall(commands)

def link(b):
    objects = [b.oname(inBuildDir(s, b.builddir)) for s in sources]
    apptarget = b.builddir + '/' + target
    run(*b.ld(apptarget, objects))

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
        return StaticGcc(self.cflags, self.ldflags, self.cc, self.builddir, self.usepkgconfig)

def static(builddir='fbuild/static', **kwargs):
    b = compiler(builddir=builddir, usepkgconfig=False, **kwargs)
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
    make(b)

def mingw(builddir='fbuild/mingw', cc='i486-mingw32-g++', **kwargs):
    b = compiler(builddir=builddir, cc=cc, usepkgconfig=False, **kwargs)
    b.define('USE_WIN32')
    b.incpath('mingwdeps/include')
    b.libpath('mingwdeps/lib')
    b.lib('mingw32', 'SDLmain', 'wsock32', 'ws2_32')
    b.ldflags += ['-mwindows']
    mkdir(builddir)
    detect(b)
    b.cflags += ['-UHAVE_STRCASESTR','-UHAVE_STRNLEN']
    make(b)

def clean():
    autoclean()

# Keep this equivalent to languages/genpot.sh.
def pot():
    luas = find('.', '*.lua')
    luas.sort()
    run('xgettext','-d','bos','-k_','-o','languages/bos.pot', luas)
    s = sorted(sources)
    run('xgettext','-d','engine','-C','-k_','--add-comments=TRANSLATORS','-o','languages/engine.pot', s)

def all(**kwargs):
    release(**kwargs)
    debug(**kwargs)

def default(**kwargs):
    release(**kwargs)

setup(default='default')
if __name__ == '__main__':
    import optparse
    j = optparse.Option('-j', '--jobs', action='store',
                      help='the number of jobs to run simultaneously',
                      default=1)
    main(extra_options=[j])

