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
/**@name ccl.h		-	The clone configuration language headerfile. */
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

#ifndef __CCL_HELPERS__
#define __CCL_HELPERS__


//@{


/*----------------------------------------------------------------------------
--	Structures
----------------------------------------------------------------------------*/

// FIXME: (mr-russ) document

typedef struct _ccl_flag_def_
{
    char*	ident;
    int		value;
} CclFlagDef;

typedef struct _ccl_field_def_
{
    char*	name;
    void 	(*convertfunc)(SCM scmfrom, void *binaryform, void *para);
    void*	offset;
    void*	para;
} CclFieldDef;

typedef struct _IOStruct_def_
{
    char*	name;
    int		size;
    int		array_size;
    CclFieldDef	defs[];
} CclStructDef;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/
extern int IOLoadingMode;
extern unsigned int IOTabLevel;
extern CLFile* IOOutFile;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Print "IOTabLevel" tabs on the ccl output
extern void IOPrintTabs();
    /// Handle saving/loading of structure
extern void IOStruct(SCM scmform, void *binaryform, void *para);
    /// Handle saving/loading a pointer to a structure.
extern void IOStructPtr(SCM scmform, void *binaryform, void *para);
    /// Handle loading a fixed size array of structure.
extern void IOStructArray( SCM from, void *binaryform, void *para);
    /// Handle saving/loading linked list.
extern void IOLinkedList(SCM scmfrom, void *binaryform, void *para);
    /// Handle saving/loading tables.
extern void IOTable(SCM scmfrom, void *binaryform, void *para);
    /// Handle the case of saving/loading pointers which are null
extern int IOHandleNullPtr(SCM scmfrom, void *binaryform);
    /// Handle saving/loading of int
extern void IOInt(SCM scmform, void *binaryform, void *para);
    /// Handle saving/loading of bool stored as int 
extern void IOBool(SCM scmfrom, void *binaryform, void *para);
    /// Handle saving/loading of bool stored as char
extern void IOCharBool(SCM scmfrom, void *binaryform, void *para);
    /// Handle saving/loading of string
extern void IOString(SCM scmfrom, void *binaryform, void *para);
    /// Handle saving/loading of SCM
extern void IOCcl(SCM scmfrom, void *binaryform, void *para);
    /// Handle saving/loading of flag stored in char
extern void IOCharFlag(SCM scmfrom, void *binaryform, void *para);
    /// Handle saving/loading a fixed length string
extern void IOStrBuffer(SCM scmfrom, void *binaryform, void *para);
    /// Handle saving/loading a dynamic array of int
extern void IOIntArrayPtr(SCM scmfrom, void *binaryform, void *para);
    /// Handle saving/loading an already allocated array of int
extern void IOIntArray(SCM scmfrom, void *binaryform, void *para);

    /// Handle saving/loading an unittype pointer (UnitType*)
extern void IOUnitTypePtr(SCM scmfrom, void *binaryform, void *para);
    /// Handle saving/loading a reference to an unit (Unit*)
extern void IOUnitPtr(SCM scmfrom, void *binaryform, void *para);
    /// Handle saving/loading a reference to an upgrade (Upgrade*)
extern void IOUpgradePtr(SCM scmfrom, void *binaryform, void *para);
    /// Handle saving/loading a reference to a player (Player*)
extern void IOPlayerPtr(SCM scmfrom, void *binaryform, void *para);


#if 0
    /// Handle saving/loading a reference to an aitype. (AiType*)
extern void IOAiTypePtr(SCM from,void * binaryform,void * para);
    /// Handle saving/loading a reference to an AiScriptAction. (AiScriptAction**)
extern void IOAiScriptActionPtr(SCM scmfrom,void * binaryform,void * para);
#endif
//@}

#endif				// !__CCL_H__
