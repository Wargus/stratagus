//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name ccl_helpers.c	-	Some functions for loading/saving ccl. */
//
//      (c) Copyright 2003 by Ludovic Pollet
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//      $Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unittype.h"
#include "ccl.h"
#include "ai.h"
#include "ccl_helpers.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/


global int IOLoadingMode;
global unsigned int IOTabLevel;
global CLFile *IOOutFile;

/**
**	Output tabs when saving a ccl value
*/
global void IOPrintTabs(void)
{
    unsigned int tableft;
    unsigned int cur;
    char buffer[256];
    tableft = IOTabLevel;
    while (tableft) {
	cur = (tableft >= sizeof (buffer) ? sizeof (buffer) - 1 : tableft);

	memset(buffer, 9, cur);
	buffer[cur] = 0;

	CLprintf(IOOutFile, buffer);

	tableft -= cur;
    }
}

/**
**	Save a structure.
**
**	@param defs	the structure definition
**	@param data	pointer to the structure
*/
local void saveData(IOFieldDef * defs, void *data)
{
    unsigned int i;
    while (defs->name) {
	if (defs->convertfunc) {
	    IOPrintTabs();
	    for (i = 0; i < IOTabLevel; i++) {
		DebugLevel3Fn("\t");
	    }
	    DebugLevel3Fn("saving %s\n" _C_ defs->name);
	    // name
	    CLprintf(IOOutFile, "%s", defs->name);
	    IOTabLevel++;
	    // real output
	    (*defs->convertfunc) (SCM_UNSPECIFIED, ((char *) data) + (int) defs->offset, defs->para);
	    IOTabLevel--;
	    CLprintf(IOOutFile, "\n");
	}
	defs++;
    }
}

/**
**	Load a structure.
**
**	@param defs	the structure definition
**	@param data	pointer to the structure
**	@param desc	SCM to parse
*/
local void restoreData(IOFieldDef * defs, void *data, SCM desc)
{
    SCM ident;
    SCM value;
    IOFieldDef *curdef;
    while (!gh_null_p(desc)) {
	ident = gh_car(desc);
	desc = gh_cdr(desc);
	value = gh_car(desc);
	desc = gh_cdr(desc);
	for (curdef = defs; curdef->name; curdef++) {
	    if (curdef->convertfunc && gh_eq_p(ident, gh_symbol2scm(curdef->name))) {
		(*curdef->convertfunc) (value, ((char *) data) + (int) curdef->offset,
		    curdef->para);
		break;
	    }
	}
	if (!curdef->name) {
	    gh_display(ident);
	    errl("invalid identifier in this context", ident);
	}
    }
}

/**
**	Handle saving/loading of structure.
**	binaryform points to the structure to load/save.
**	para is used as a (IOStructDef*)
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the structure to load/save
**	@param	para		Pointer to a IOStructDef structure.
*/
global void IOStruct(SCM scmform, void *binaryform, void *para)
{
    IOStructDef *structDef;

    structDef = (IOStructDef *) para;

    if (IOLoadingMode) {
	restoreData(structDef->defs, binaryform, scmform);
    } else {
	CLprintf(IOOutFile, " (\n");
	++IOTabLevel;
	saveData(structDef->defs, binaryform);
	--IOTabLevel;
	IOPrintTabs();
	CLprintf(IOOutFile, ")");
    }
}

/**
**	Handle saving/loading a reference to a structure.
**	The structure content is also saved/loaded. 
**	On load, the structure is malloc'ed. 
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the structure'reference to load/save ( <structure-type> ** )
**	@param	para		Pointer to a IOStructDef structure.
*/
global void IOStructPtr(SCM scmform, void *binaryform, void *para)
{
    void **structptr;
    IOStructDef *def;

    if (IOHandleNullPtr(scmform, binaryform)) {
	return;
    }

    def = (IOStructDef *) para;

    structptr = (void **) binaryform;

    if (IOLoadingMode) {
	(*structptr) = (void *) malloc(def->size);
	memset((*structptr), 0, def->size);
	IOStruct(scmform, (*structptr), para);
    } else {
	IOStruct(scmform, (*structptr), para);
    }
}

/**
**	Handle saving/loading an array of structures.
**	The array size is found in the array_size field of the IOStructDef structure. 
**	The array is NOT malloc'ed. 
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the structure'reference to load/save ( <structure-type> * )
**	@param	para		Pointer to a IOStructDef structure, describing format of the structure.
*/
global void IOStructArray(SCM from, void *binaryform, void *para)
{
    IOStructDef *def;
    int i;

    def = (IOStructDef *) para;

    if (IOLoadingMode) {
	for (i = 0; i < def->array_size; ++i) {
	    IOStruct(gh_car(from), binaryform, para);

	    from = gh_cdr(from);
	    (char *) binaryform += def->size;
	}
    } else {
	CLprintf(IOOutFile, " (\n");
	++IOTabLevel;
	for (i = 0; i < def->array_size; ++i) {
	    IOPrintTabs();
	    IOStruct(from, binaryform, para);
	    CLprintf(IOOutFile, "\n");
	    (char *) binaryform += def->size;
	}
	--IOTabLevel;
	IOPrintTabs();
	CLprintf(IOOutFile, ")");
    }
}

/**
**	Handle saving/loading a linked list of structure.
**	The binaryform is a pointer to the "first" field.
**	The third parameter is a pointer to a IOStructDef, describing list elements.
**
**	defs[0] must contain the reference to the next field on the loaded structure.  
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the "first"'ref ( <any-structure>** )
**	@param	para		Pointer to the IOStructDef
*/
global void IOLinkedList(SCM scmfrom, void *binaryform, void *para)
{
    SCM item;
    IOStructDef *itemDef;
    void **current;
    itemDef = (IOStructDef *) para;

    if (IOLoadingMode) {
	current = (void **) binaryform;
	while (!gh_null_p(scmfrom)) {
	    item = gh_car(scmfrom);
	    scmfrom = gh_cdr(scmfrom);

	    // Just to be safe... 
	    if (!gh_null_p(item)) {
		IOStructPtr(item, current, itemDef);
		current =
		    (void **) (((char *) (*current)) + (int) itemDef->defs->offset);
	    }
	}
    } else {
	current = ((void **) binaryform);

	CLprintf(IOOutFile, " (\n");
	++IOTabLevel;
	while (*((void **) current)) {
	    IOPrintTabs();
	    IOStructPtr(gh_car(scmfrom), current, itemDef);
	    CLprintf(IOOutFile, "\n");

	    // Get the next...
	    current = (void **) (((char *) (*current)) + (int) itemDef->defs->offset);
	}
	--IOTabLevel;
	IOPrintTabs();
	CLprintf(IOOutFile, ")\n");
	IOPrintTabs();
    }
}

/**
**	Handle saving/loading a table of structure.
**	The table is composed of two thing : a pointer to data and a counter. 
**	The third parameter is a pointer to a IOStructDef, describing the table
**	fields of the IOStructDef are :
**		size		indicate the size of one element of the table.
**		defs[0]:	describe the data field ( should be <any-structure>** ) 
**		defs[1]:	describe the counter field ( should be int )
**		defs[2]:	describe the type of each structure to load. ( func & para )
**	
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the "first"'ref ( <any-structure>** )
**	@param	para		Pointer to the IOStructDef
*/
global void IOTable(SCM scmfrom, void *binaryform, void *para)
{
    IOStructDef *def;
    int count;
    void *org;

    def = (IOStructDef *) para;
    if (IOLoadingMode) {
	count = 0;
	org = 0;
	while (!gh_null_p(scmfrom)) {
	    ++count;
	    org = realloc(org, def->size * count);
	    (*def->defs[2].convertfunc) (gh_car(scmfrom),
		(void *) ((char *) org + (def->size * (count - 1))), def->defs[2].para);
	    scmfrom = gh_cdr(scmfrom);
	}
	*((void **) ((char *) binaryform + (int) def->defs[0].offset)) = org;
	*((int *) ((char *) binaryform + (int) def->defs[1].offset)) = count;
    } else {
	CLprintf(IOOutFile, " (\n");
	++IOTabLevel;
	count = *((int *) ((char *) binaryform + (int) def->defs[1].offset));
	org = *((void **) ((char *) binaryform + (int) def->defs[0].offset));

	while (count) {
	    IOPrintTabs();
	    (*def->defs[2].convertfunc) (scmfrom, org, def->defs[2].para);
	    CLprintf(IOOutFile, "\n");
	    org = (void *) ((char *) org + def->size);
	    --count;
	}
	--IOTabLevel;
	IOPrintTabs();
	CLprintf(IOOutFile, " )");
    }
}

/**
**	Handle saving/loading of ints.
**	binaryform points to the int to load/save.
**	para is unused
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the int value to load/save
**	@param	para		unused
*/
global void IOInt(SCM scmfrom, void *binaryform, void *para)
{
    if (IOLoadingMode) {
	(*((int *) binaryform)) = gh_scm2long(scmfrom);
    } else {
	CLprintf(IOOutFile, " %d", (*((int *) binaryform)));
    }
}

/**
**	Handle saving/loading of ints.
**	binaryform points to the int to load/save.
**	para is unused
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the int value to load/save
**	@param	para		unused
*/
global void IOString(SCM scmfrom, void *binaryform, void *para)
{
    if (IOHandleNullPtr(scmfrom, binaryform)) {
	return;
    }
    if (IOLoadingMode) {
	(*((char **) binaryform)) = gh_scm2newstr(scmfrom, 0);
    } else {				// FIXME : (pludov) better string support
	CLprintf(IOOutFile, " \"%s\"", (*((char **) binaryform)));
    }
}

/**
**	Handle saving/loading of bools
**	binaryform points to the bool (really int) to load/save.
**	para is unused
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the bool value to load/save ( int* )
**	@param	para		unused
*/
global void IOBool(SCM scmfrom, void *binaryform, void *para)
{
    if (IOLoadingMode) {
	(*((int *) binaryform)) = gh_null_p(scmfrom) ? 0 : 1;
    } else {
	CLprintf(IOOutFile, " %s", (*((int *) binaryform) ? "#t" : "#f"));
    }
}

/**
**	Handle saving/loading of bools stored in char
**	binaryform points to the bool (really char) to load/save.
**	para is unused
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the bool value to load/save ( char* )
**	@param	para		unused
*/
global void IOCharBool(SCM scmfrom, void *binaryform, void *para)
{
    if (IOLoadingMode) {
	(*((char *) binaryform)) = (gh_null_p(scmfrom) ? 0 : 1);
    } else {
	CLprintf(IOOutFile, " %s", (*((char *) binaryform) ? "#t" : "#f"));
    }
}

/**
**	Handle saving/loading a scm value
**	When loading, the value is gc protected.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the scm value to load/save ( SCM* )
**	@param	para		unused
*/
global void IOCcl(SCM scmfrom, void *binaryform, void *para)
{
    SCM *ptr;
    ptr = (SCM *) binaryform;
    if (IOLoadingMode) {
	*ptr = scmfrom;
	CclGcProtect(*ptr);
    } else {
	CLprintf(IOOutFile, " ");
	lprin1CL(*ptr, IOOutFile);
    }
}

/**
**	Handle saving/loading a flag stored on a char
**	Flag are defined as an array of IOFlagDef, terminated by {0,0}
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the scm value to load/save ( char* )
**	@param	para		Array of IOFlagDef, describing possible values ( IOFlagDef * ) 
*/
global void IOCharFlag(SCM scmfrom, void *binaryform, void *para)
{
    IOFlagDef *flags;

    flags = para;

    if (IOLoadingMode) {
	if (gh_exact_p(scmfrom)) {
	    (*((char *) binaryform)) = gh_scm2int(scmfrom);
	}
	while (flags->ident) {
	    if (gh_eq_p(scmfrom, gh_symbol2scm(flags->ident))) {
		(*((char *) binaryform)) = flags->value;
		return;
	    }
	    ++flags;
	}
	errl("invalid flag", scmfrom);
    } else {
	while (flags->ident) {
	    if (flags->value == (*((char *) binaryform))) {
		CLprintf(IOOutFile, " %s", flags->ident);
		return;
	    }
	    ++flags;
	}
	CLprintf(IOOutFile, "\n;; WARNING : no flag defined for value %d\n",
	    (*((char *) binaryform)));
	CLprintf(IOOutFile, ";; defined flags are : ");
	flags = para;
	while (flags->ident) {
	    CLprintf(IOOutFile, " %s(%d) ", flags->ident, flags->value);
	    ++flags;
	}
	IOPrintTabs();
	CLprintf(IOOutFile, "%d", (*((char *) binaryform)));
    }
}

/**
**	Handle saving/loading a string stored in a fixed-size array of char
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the string buffer
**	@param	para		Max size of string ( int ) 
*/
global void IOStrBuffer(SCM scmfrom, void *binaryform, void *para)
{
    int size;
    char *buffer;
    char *str;

    size = (int) para;
    buffer = (char *) binaryform;
    if (IOLoadingMode) {
	str = gh_scm2newstr(scmfrom, NULL);
	strncpy(buffer, str, size);
	free(str);
    } else {
	// TODO better string support !
	CLprintf(IOOutFile, " \"%s\"", buffer);
    }
}

/**
**	Handle saving/loading a null pointer, so that function saving pointer
**	don't have to handle this case.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the pointer to load ( void ** )
**	@param	para		Unused
**	@return 1 if a null value was saved/loaded, meaning that no more processing is required.
*/
global int IOHandleNullPtr(SCM scmfrom, void *binaryform)
{
    if (IOLoadingMode) {
	if (gh_null_p(scmfrom)) {
	    *((void **) binaryform) = 0;
	    return 1;
	}
    } else {
	if (!*((void **) binaryform)) {
	    CLprintf(IOOutFile, " ()");
	    return 1;
	}
    }
    return 0;
}

/**
**	Handle saving/loading a dynamic array of int.
**	The size of the array is fixed. The array is malloc'ed.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the array'ref ( int ** )
**	@param	para		Size of array to allocate/save 
*/
global void IOIntArrayPtr(SCM scmfrom, void *binaryform, void *para)
{
    int size;
    int i;
    int **array;

    if (IOHandleNullPtr(scmfrom, binaryform)) {
	return;
    }

    size = (int) para;
    array = (int **) binaryform;

    if (IOLoadingMode) {
	(*array) = (int *) malloc(sizeof (int) * size);
	for (i = 0; i < size; ++i) {
	    (*array)[i] = gh_scm2int(gh_car(scmfrom));
	    scmfrom = gh_cdr(scmfrom);
	}
    } else {
	i = 0;
	CLprintf(IOOutFile, " (");
	while (i < size) {
	    CLprintf(IOOutFile, " %d", (*array)[i]);
	    ++i;
	}
	CLprintf(IOOutFile, " )");
    }
}

/**
**	Handle saving/loading an array of int.
**	The size of the array is fixed. The array must already have been malloc'ed.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the array ( int * )
**	@param	para		Size of array to allocate/save 
*/
global void IOIntArray(SCM scmfrom, void *binaryform, void *para)
{
    int i;
    int size;

    size = (int) para;

    if (IOLoadingMode) {
	for (i = 0; i < size; ++i) {
	    ((int *) binaryform)[i] = gh_scm2int(gh_car(scmfrom));
	    scmfrom = gh_cdr(scmfrom);
	}
    } else {
	CLprintf(IOOutFile, " (");
	++IOTabLevel;
	if (size > 16) {
	    CLprintf(IOOutFile, "\n");
	    IOPrintTabs();
	}

	for (i = 0; i < size; ++i) {
	    CLprintf(IOOutFile, " %d", ((int *) binaryform)[i]);
	    if (i > 0 && !(i & 15)) {
		CLprintf(IOOutFile, "\n");
		IOPrintTabs();
	    }
	}

	CLprintf(IOOutFile, ")");
	--IOTabLevel;
	if (size > 16) {
	    CLprintf(IOOutFile, "\n");
	    IOPrintTabs();
	}
    }
}

/**
**	Handle saving/loading a reference to an unittype.
**	The value can be null.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the unittype'ref ( UnitType ** )
**	@param	para		unused
*/
global void IOUnitTypePtr(SCM scmfrom, void *binaryform, void *para)
{
    char *str;
    UnitType **unittype;

    if (IOHandleNullPtr(scmfrom, binaryform)) {
	return;
    }

    unittype = (UnitType **) binaryform;
    if (IOLoadingMode) {
	str = gh_scm2newstr(scmfrom, NULL);
	(*unittype) = UnitTypeByIdent(str);
	free(str);
    } else {
	CLprintf(IOOutFile, " %s", (*unittype)->Ident);
    }
}

/**
**	Handle saving/loading a reference to an upgrade.
**	The value can be null.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the unittype'ref ( Upgrade ** )
**	@param	para		unused
*/
global void IOUpgradePtr(SCM scmfrom, void *binaryform, void *para)
{
    char *str;
    Upgrade **upgrade;

    if (IOHandleNullPtr(scmfrom, binaryform)) {
	return;
    }

    upgrade = (Upgrade **) binaryform;
    if (IOLoadingMode) {
	str = gh_scm2newstr(scmfrom, NULL);
	(*upgrade) = UpgradeByIdent(str);
	free(str);
    } else {
	CLprintf(IOOutFile, " %s", (*upgrade)->Ident);
    }
}

/**
**	Handle saving/loading a reference to an unit.
**	The null case is handled.
**	On loading, the unit's usage count is incremented
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the unit'ref ( Unit ** )
**	@param	para		unused
*/
global void IOUnitPtr(SCM scmfrom, void *binaryform, void *para)
{
    int slot;

    if (IOHandleNullPtr(scmfrom, binaryform)) {
	return;
    }
    if (IOLoadingMode) {
	slot = gh_scm2int(scmfrom);
	*((Unit **) binaryform) = UnitSlots[slot];
	// FIXME: (pludov) increment unit usage count!
    } else {
	CLprintf(IOOutFile, " %d", (*((Unit **) binaryform))->Slot);
    }
}

/**
**	Handle saving/loading a reference to a Player ( Player * ).
**	The null case is handled.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the unit'ref ( Player ** )
**	@param	para		unused
*/
global void IOPlayerPtr(SCM scmfrom, void *binaryform, void *para)
{
    int playerid;

    if (IOHandleNullPtr(scmfrom, binaryform)) {
	return;
    }
    if (IOLoadingMode) {
	// Load a player from scm
	playerid = gh_scm2int(scmfrom);
	*((Player **) binaryform) = Players + playerid;
    } else {
	// Save a player to scm
	playerid = (*((Player **) binaryform))->Player;
	CLprintf(IOOutFile, " %d", playerid);
    }
}
