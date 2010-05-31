#  Pretty printing of C++ STL objects in GDB.
#
#  (c) Copyright 2010 by Kalle Olavi Niemitalo.
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
Pretty printing of C++ STL objects in the GNU Debugger.
This requires GDB 7.0 or later, configured --with-python.
The code was written for Python 2.5.5 but may work with earlier versions too.
The C++ STL pretty printers support libstdc++ 6.0.13 of GCC 4.4.4.

These pretty printers should not be used with GCC 4.5 or later,
because those versions of libstdc++ already include pretty printers
maintained as part of the library and therefore better up to date.
According to the documentation, GDB looks for pretty printers first
in the objfiles, then in the progspace, and last in the gdb module;
thus, if you register these pretty printers in the gdb module and
let GDB register the pretty printers of libstdc++ in the objfile of
libstdc++, then they should automatically take precedence over these.
"""

import gdb
import re

class StdStringPrinter:
    "Pretty-print an std::string."

    __typeRegex = re.compile("^std::basic_string<.*>$|^std::string")

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def display_hint(self):
        "Called by GDB to find how to display the value and children."
        return "string"

    def to_string(self):
        "Called by GDB to find what to display as the value."
        data = self.val["_M_dataplus"]["_M_p"]
        if data == 0:
            return data
        repType = gdb.lookup_type(str(self.val.type.strip_typedefs().unqualified()) + "::_Rep")
        rep = data.cast(repType.pointer()) - 1
        return data.string(length = rep["_M_length"])

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        tag = val.type.tag
        if tag and StdStringPrinter.__typeRegex.match(tag):
            return StdStringPrinter(val)

class StdVectorPrinter:
    "Pretty-print an std::vector."

    __typeRegex = re.compile("^std::vector<.*>$")

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def children(self):
        "Called by GDB to enumerate the children of the value."
        start = self.val["_M_impl"]["_M_start"]
        finish = self.val["_M_impl"]["_M_finish"]
        for index in range(finish - start):
            element = (start + index).dereference()
            yield "[%d]" % index, element

    def display_hint(self):
        "Called by GDB to find how to display the value and children."
        return "array"

    def to_string(self):
        "Called by GDB to find what to display as the value."
        return "std::vector"

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        tag = val.type.tag
        if tag and StdVectorPrinter.__typeRegex.match(tag):
            return StdVectorPrinter(val)

class StdListPrinter:
    "Pretty-print an std::list."

    __typeRegex = re.compile("^std::list<.*>$")

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def children(self):
        "Called by GDB to enumerate the children of the value."
        elementType = self.val.type.template_argument(0)
        nodeTypeName = "std::_List_node<%s >" % elementType
        nodeType = gdb.lookup_type(nodeTypeName)
        nodePointerType = nodeType.pointer()
        sentinelPointer = self.val["_M_impl"]["_M_node"].address
        nodeBasePointer = sentinelPointer
        while nodeBasePointer.dereference()["_M_next"] != sentinelPointer:
            nodeBasePointer = nodeBasePointer.dereference()["_M_next"]
            nodePointer = nodeBasePointer.cast(nodePointerType)
            yield "element", nodePointer.dereference()["_M_data"]

    def display_hint(self):
        "Called by GDB to find how to display the value and children."
        return "array"

    def to_string(self):
        "Called by GDB to find what to display as the value."
        return "std::list"

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        tag = val.type.tag
        if tag and StdListPrinter.__typeRegex.match(tag):
            return StdListPrinter(val)

class StdMapPrinter:
    "Pretty-print an std::map."

    __typeRegex = re.compile("^std::map<.*>$")

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def children(self):
        "Called by GDB to enumerate the children of the value."
        keyType = self.val.type.template_argument(0)
        valueType = self.val.type.template_argument(1)
        nodeTypeName = "std::_Rb_tree_node<std::pair<%s const, %s > >" \
            % (keyType, valueType)
        nodeType = gdb.lookup_type(nodeTypeName)
        nodePointerType = nodeType.pointer()
        def recurse(nodeBasePointer):
            if nodeBasePointer != 0:
                nodeBase = nodeBasePointer.dereference()
                node = nodeBasePointer.cast(nodePointerType).dereference()
                for tag, data in recurse(nodeBase["_M_left"]):
                    yield tag, data
                yield "key", node["_M_value_field"]["first"]
                yield "value", node["_M_value_field"]["second"]
                for tag, data in recurse(nodeBase["_M_right"]):
                    yield tag, data
        return recurse(self.val["_M_t"]["_M_impl"]["_M_header"]["_M_parent"])

    def display_hint(self):
        "Called by GDB to find how to display the value and children."
        return "map"

    def to_string(self):
        "Called by GDB to find what to display as the value."
        return "std::map"

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        tag = val.type.tag
        if tag and StdMapPrinter.__typeRegex.match(tag):
            return StdMapPrinter(val)

def register_printers(objfile):
    """Register all the pretty printers defined by this module.
OBJFILE may be an objfile, a progspace, or the entire gdb module."""
    objfile.pretty_printers.append(StdStringPrinter.lookup_printer)
    objfile.pretty_printers.append(StdVectorPrinter.lookup_printer)
    objfile.pretty_printers.append(StdListPrinter.lookup_printer)
    objfile.pretty_printers.append(StdMapPrinter.lookup_printer)
