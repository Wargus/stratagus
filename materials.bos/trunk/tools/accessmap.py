#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
#  Access map creation tool
#
#  (c) Copyright 2006 by Francois Beerten.
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


import Image
import sys

header = """
--            ____
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  )
--        /_____/\____/____/
--
--  Invasion - Battle of Survival
--   A GPL'd futuristic RTS game
--
--  (c) Copyright 2005-2006 by Francois Beerten et al.
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program; if not, write to the Free Software
--  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

"""

def convertAccessMap(imagename, accessname):
  im = Image.open(imagename)
  width = im.size[0]
  height = im.size[1]
  
  print header
  print '-- access file from %s' % imagename
  print '-- width:%d height:%d\n' % (width, height)
  
  for y in xrange(0, height):
    for x in xrange(0, width):
       if im.getpixel((x,y)) == 0:
           print 'SetTileFlags(%d, {"%s"})' % ( x + y * width, accessname)

if __name__ == '__main__':
  if len(sys.argv) != 3:
     print 'Convert a png image into access instructions for a terrain'
     print 'Usage: accessmap.py <image> <access type> > lua file'
     sys.exit(-2)
  convertAccessMap(sys.argv[1], sys.argv[2])


