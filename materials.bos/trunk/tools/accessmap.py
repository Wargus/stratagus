#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
#  Access map creation tool
#
#  (c) Copyright 2006-2008 by Francois Beerten.
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
--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--  (c) Copyright 2005-2008 by Francois Beerten et al.
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
     print 'Usage: accessmap.py <image> <access type:[fast|water|unpassable]> > lua file'
     sys.exit(-2)
  convertAccessMap(sys.argv[1], sys.argv[2])


