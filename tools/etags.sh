#! /bin/sh -e
#     ____                _       __
#    / __ )____  _____   | |     / /___ ___________
#   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
#  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  )
# /_____/\____/____/     |__/|__/\__,_/_/  /____/
#
#       A futuristic real-time strategy game.
#          This file is part of Bos Wars.
#
#      Script that generates a TAGS file for GNU Emacs.
#      (c) Copyright 2010 by Kalle Olavi Niemitalo
#      (unless this is too simple to be copyrightable)
#
#      Bos Wars is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published
#      by the Free Software Foundation; only version 2 of the License.
#
#      Bos Wars is distributed in the hope that it will be useful,
#      but WITHOUT ANY WARRANTY; without even the implied warranty of
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#      GNU General Public License for more details.

# The etags program does not recognize "class GCN_CORE_DECLSPEC DropDown"
# and "class GCN_EXTENSION_DECLSPEC SDLGraphics" unless we help it
# with a regex.

find . '(' -name "*.h" -o -name "*.cpp" -o -name "*.lua" ')' -print \
    | etags --regex='/[ \t]*class[ \t]+GCN_\w+_DECLSPEC[ \t]+\(\w+\)/\1/' -
