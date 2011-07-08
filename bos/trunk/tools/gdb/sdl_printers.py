#  Pretty printing of SDL objects in GDB.
#
#  (c) Copyright 2011 by Kalle Olavi Niemitalo.
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

"""
Pretty printing of SDL objects in the GNU Debugger.
This requires GDB 7.0 or later, configured --with-python.
The code was written for Python 2.6.6 but may work with earlier versions too.
"""

import gdb

class Uint8Printer:
    "Pretty-print a Uint8."

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def to_string(self):
        "Called by GDB to find what to display as the value."
        # Stop GDB from displaying it as a character literal.
        return self.val.cast(gdb.lookup_type("unsigned int"))

def lookup_printer(val):
    "Return a printer for the value, if supported; else None."
    type = val.type
    while type.code == gdb.TYPE_CODE_TYPEDEF:
        tag = str(type)
        if tag == "Uint8":
            return Uint8Printer(val)
        type = type.target()

def register_printers(objfile):
    """Register all the pretty printers defined by this module.
OBJFILE may be an objfile, a progspace, or the entire gdb module."""
    objfile.pretty_printers.append(lookup_printer)
