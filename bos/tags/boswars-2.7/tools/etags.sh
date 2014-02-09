#! /bin/sh -e
#
#  Script that generates a TAGS file for GNU Emacs.
#
#  (c) Copyright 2010 by Kalle Olavi Niemitalo.
#  (unless this is too simple to be copyrightable)
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
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

# The etags program does not recognize "class GCN_CORE_DECLSPEC DropDown"
# and "class GCN_EXTENSION_DECLSPEC SDLGraphics" unless we help it
# with a regex.

find . -name mingwdeps -prune -o '(' -name "*.h" -o -name "*.cpp" -o -name "*.lua" ')' -print \
    | etags --regex='/[ \t]*class[ \t]+GCN_\w+_DECLSPEC[ \t]+\(\w+\)/\1/' -
