#! /usr/bin/env python

##     ____                _       __               
##    / __ )____  _____   | |     / /___ ___________
##   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
##  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
## /_____/\____/____/     |__/|__/\__,_/_/  /____/  
##                                              
##       A futuristic real-time strategy game.
##          This file is part of Bos Wars.
##
##      Script that generates all the distribution packages.
##      (c) Copyright 2007 by Francois Beerten
##
##      Stratagus is free software; you can redistribute it and/or modify
##      it under the terms of the GNU General Public License as published
##      by the Free Software Foundation; only version 2 of the License.
##
##      Stratagus is distributed in the hope that it will be useful,
##      but WITHOUT ANY WARRANTY; without even the implied warranty of
##      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##      GNU General Public License for more details.

import sys
import os
import tarfile
import zipfile
import time

def printDot():
  sys.stdout.write('.')
  sys.stdout.flush()

def createTarArchive(archivename, namelist):
  print "\nCreating %s.tar.gz" % archivename
  tar = tarfile.open(archivename + ".tar.gz", "w:gz")
  for name in namelist:
      tarinfo = tar.gettarinfo(name, archivename + "/" + name)
      tarinfo.uid = 500
      tarinfo.gid = 500
      tarinfo.uname = "user"
      tarinfo.gname = "user"
      printDot()
      tar.addfile(tarinfo, file(name))
  tar.close()
  print

def createZipArchive(archivename, namelist):
  print "\nCreating %s.zip" % archivename
  z = zipfile.ZipFile(archivename + ".zip", "w", zipfile.ZIP_DEFLATED)
  for name in namelist:
      z.write(name, archivename + "/" + name)
      printDot()
  z.close()
  print


def isSvnWorkSpace():
   return os.access('.svn', os.F_OK)

def listSvnFiles():
   entries = os.popen('svn ls -R').readlines()
   files = []
   for e in entries:
     e = e.strip()
     if not e.endswith('/'):
        files.append(e)
   return files

def listFilesInDirectories():
   raise "Not implemented"

def listFiles():
   if isSvnWorkSpace():
      return listSvnFiles()
   else:
      return listFilesInDirectories()

def buildDefaultVersionName():
   t = time.time()
   v = time.strftime('%Y%m%d', time.gmtime(t))
   return v

def buildVersionName():
   if len(sys.argv) > 1:
      return sys.argv[1]
   else:
      return buildDefaultVersionName()

def filterDirectories(files):
   return [x for x in files if not x.endswith('/')]
def filterSources(files):
   buildfiles = ['SConstruct', 'bos.sln']
   def isGoodFile(f):
     return (not f.startswith('engine/') and not f.startswith('tools/') and
             f not in buildfiles)
   return [x for x in files if isGoodFile(x)]
   


def buildPackages():
  name = 'boswars-' + buildVersionName() + '-%s'

  os.chdir('../..')
  files = listFiles()
  files = filterDirectories(files)

  # create source packages
  createTarArchive(name % 'src', files)
  createZipArchive(name % 'src', files)

  # create binary packages
  #os.popen('scons static=1 debug=0 CXX=apg++')
  binarydistfiles = filterSources(files)
  linuxfiles = binarydistfiles[:]
  linuxfiles.append('boswars')
  createTarArchive(name % 'linux', linuxfiles)
  if os.access('boswars.exe', os.F_OK):
     winfiles = binarydistfiles[:]
     winfiles.append('boswars.exe')
     createZipArchive(name % 'win32', winfiles)
  

if __name__ == '__main__':
  buildPackages()





