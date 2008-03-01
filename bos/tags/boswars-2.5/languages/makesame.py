#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  Tool to build a po from a pot with duplicated text.
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


# Usage: 
#    python makesame.py bos.pot bos-en.po

import sys

f = file(sys.argv[1])
out = file(sys.argv[2], "wt")

for line in f:
  if line.startswith("msgid "):
       out.write(line)
       out.write("msgstr %s" % (line.split(' ', 1)[1]))
  elif line.startswith("msgstr "):
       pass
  else:
       out.write(line)

