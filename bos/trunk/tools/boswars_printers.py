#  Pretty printing of Bos Wars objects in GDB.
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
Pretty printing of Bos Wars objects in the GNU Debugger.
This requires GDB 7.0 or later, configured --with-python.
The code was written for Python 2.5.5 but may work with earlier versions too.
"""

import gdb

class COrderPrinter:
    "Pretty-print a COrder."

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def children(self):
        "Called by GDB to enumerate the children of the value."
        for field in self.val.type.fields():
            if field.name == "Action":
                # COrder::Action is defined as unsigned char, but its
                # possible values are listed in enum _unit_action_.
                yield field.name, self.val[field.name].cast(gdb.lookup_type("enum _unit_action_"))
            elif field.name == "Width" or field.name == "Height":
                # These are unsigned char but cast them to int so
                # gdb won't try to display them as e.g. '\0'.
                yield field.name, self.val[field.name].cast(gdb.lookup_type("int"))
            elif field.name == "Arg1":
                # COrder::Action shows which member of the union is used.
                for enum in gdb.lookup_type("enum _unit_action_").fields():
                    if enum.bitpos == self.val["Action"]:
                        if enum.name == "UnitActionPatrol":
                            yield "Arg1.Patrol", self.val["Arg1"]["Patrol"];
                            usedArg1 = true
                        elif enum.name == "UnitActionSpellCast":
                            yield "Arg1.Spell", self.val["Arg1"]["Spell"];
                            usedArg1 = true
            else:
                yield field.name, self.val[field.name]

    def to_string(self):
        "Called by GDB to find what to display as the value."
        return "COrder"

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        if val.type.tag == "COrder":
            return COrderPrinter(val)

class COrderPointerPrinter:
    "Pretty-print a pointer to COrder."

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def to_string(self):
        "Called by GDB to find what to display as the value."
        voidPointerType = gdb.lookup_type("void").pointer()
        # str(self.val) would cause a recursive call; the cast avoids that.
        result = str(self.val.cast(voidPointerType))
        if self.val != 0:
            order = self.val.dereference()
            action = order["Action"].cast(gdb.lookup_type("enum _unit_action_"))
            result = result + (" {%s}" % action)
        return result

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        if val.type.code == gdb.TYPE_CODE_PTR:
            if val.type.target().tag == "COrder":
                return COrderPointerPrinter(val)

class CUnitTypePointerPrinter:
    "Pretty-print a pointer to CUnitType."

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def to_string(self):
        "Called by GDB to find what to display as the value."
        voidPointerType = gdb.lookup_type("void").pointer()
        # str(self.val) would cause a recursive call; the cast avoids that.
        result = str(self.val.cast(voidPointerType))
        if self.val != 0:
            unitTypeIdent = self.val.dereference()["Ident"]
            result = result + (" {%s}" % unitTypeIdent)
        return result

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        if val.type.code == gdb.TYPE_CODE_PTR:
            if val.type.target().tag == "CUnitType":
                return CUnitTypePointerPrinter(val)

class CUnitPointerPrinter:
    "Pretty-print a pointer to CUnit."

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def to_string(self):
        "Called by GDB to find what to display as the value."
        voidPointerType = gdb.lookup_type("void").pointer()
        # str(self.val) would cause a recursive call; the cast avoids that.
        result = str(self.val.cast(voidPointerType))
        if self.val != 0:
            unit = self.val.dereference()
            unitTypePointer = unit["Type"]
            if unitTypePointer != 0:
                unitTypeIdent = unitTypePointer.dereference()["Ident"]
                result = result + (" {%s}" % unitTypeIdent)
        return result

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        if val.type.code == gdb.TYPE_CODE_PTR:
            if val.type.target().tag == "CUnit":
                return CUnitPointerPrinter(val)

class CGraphicPointerPrinter:
    "Pretty-print a pointer to CGraphic."

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def to_string(self):
        "Called by GDB to find what to display as the value."
        voidPointerType = gdb.lookup_type("void").pointer()
        # str(self.val) would cause a recursive call; the cast avoids that.
        result = str(self.val.cast(voidPointerType))
        if self.val != 0:
            graphic = self.val.dereference()
            result = result + (" {%s}" % graphic["File"])
        return result

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        if val.type.code == gdb.TYPE_CODE_PTR:
            if val.type.target().tag == "CGraphic":
                return CGraphicPointerPrinter(val)

class CPatchTypePointerPrinter:
    "Pretty-print a pointer to CPatchType."

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def to_string(self):
        "Called by GDB to find what to display as the value."
        voidPointerType = gdb.lookup_type("void").pointer()
        # str(self.val) would cause a recursive call; the cast avoids that.
        result = str(self.val.cast(voidPointerType))
        if self.val != 0:
            patchType = self.val.dereference()
            result = result + (" {%s}" % patchType["name"])
        return result

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        if val.type.code == gdb.TYPE_CODE_PTR:
            if val.type.target().tag == "CPatchType":
                return CPatchTypePointerPrinter(val)

class CPatchPointerPrinter:
    "Pretty-print a pointer to CPatch."

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def to_string(self):
        "Called by GDB to find what to display as the value."
        voidPointerType = gdb.lookup_type("void").pointer()
        # str(self.val) would cause a recursive call; the cast avoids that.
        result = str(self.val.cast(voidPointerType))
        if self.val != 0:
            patch = self.val.dereference()
            result = result + (" {(%d,%d)" % (patch["x"], patch["y"]))
            patchTypePointer = patch["type"]
            if patchTypePointer != 0:
                patchType = patchTypePointer.dereference()
                result = result + (" %s" % patchType["name"])
            result = result + "}"
        return result

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        if val.type.code == gdb.TYPE_CODE_PTR:
            if val.type.target().tag == "CPatch":
                return CPatchPointerPrinter(val)

class StructPrinter:
    """Pretty-print an arbitrary named struct.
This just displays the data members, without any special interpretation.
However, without this pretty printer, GDB 7.0.1 would not apply
the std::string pretty printer to such data members."""

    def __init__(self, val):
        object.__init__(self)
        self.val = val

    def children(self):
        "Called by GDB to enumerate the children of the value."
        for field in self.val.type.fields():
            fieldValue = self.val[field.name]
            yield field.name, self.val[field.name]

    def to_string(self):
        "Called by GDB to find what to display as the value."
        return self.val.type.tag

    @staticmethod
    def lookup_printer(val):
        "Return a printer for the value, if supported; else None."
        if val.type.tag == "CUnitType" \
                or val.type.tag == "CPlayer" \
                or val.type.tag == "CPatchType" \
                or val.type.tag == "CEditor" \
                or val.type.tag == "CMapInfo" \
                or val.type.tag == "IconConfig":
            return StructPrinter(val)

def register_printers(objfile):
    """Register all the pretty printers defined by this module.
OBJFILE may be an objfile, a progspace, or the entire gdb module."""
    objfile.pretty_printers.append(COrderPrinter.lookup_printer)
    objfile.pretty_printers.append(COrderPointerPrinter.lookup_printer)
    objfile.pretty_printers.append(CUnitTypePointerPrinter.lookup_printer)
    objfile.pretty_printers.append(CUnitPointerPrinter.lookup_printer)
    objfile.pretty_printers.append(CGraphicPointerPrinter.lookup_printer)
    objfile.pretty_printers.append(CPatchTypePointerPrinter.lookup_printer)
    objfile.pretty_printers.append(CPatchPointerPrinter.lookup_printer)
    objfile.pretty_printers.append(StructPrinter.lookup_printer)
