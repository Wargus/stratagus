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
/**@name ccl_helpers.h		-	The clone configuration language headerfile. */
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
--		Documentation
----------------------------------------------------------------------------*/

/**
**
**		This Module is a translator to and from scheme.
**
**		It builds a nice looking scheme list from a structure and it's description
**		It can also load a structure from this list and the description
**
**		The IOLoadingMode variable control wether IO functions will load or save datas.
**		Theses IO functions all take the same parameters :
**				A list : The ccl list to translate into data (unused when saving)
**				A pointer : points to the structure to load,
**							or to a (structure*), when loading a ptr
**				A parameter which is used differently by the functions
**
**		Example:
**		1 To load or save a unit ptr
**
**		void load_dummy_struct(void)
**		{
**				Unit* u;
**				IOLoadingMode=1;
**				IOUnitPtr(list, (void*)&u, 0);
**		}
**
**
**		2 To load/save a full struct
**
**		If the struct is :
**
**		typedef struct _my_struct_{
**				UnitType*		unit;
**				int				count;
**		} MyStruct;
**
**		These describe the structure :
**
**		IOStructDef MyStructDef = {
**				"MyStruct",				// Name
**				sizeof(MyStruct),		// Size
**				-1,						// Array size
**				{
**					{ "unit", IOUnitPtr, &((MyStruct *) 0)->Unit,		0 },
**					{ "count",IOInt,	 &((MyStruct *) 0)->Count,		0 },
**					{ 0,0,0,0 }
**			  }
**
**		Then :
**		IOStructPtr(list,(void*)&myglobalstructptr,(void*)&MyStructDef);
**		This code will load or save the myglobalstructptr, depending on IOLoadingMode value
**
**		There are more facility available ( for handling arrays,linked list,... ).
**		Have a look at the differents IOxxx functions for details
*/

/*----------------------------------------------------------------------------
--		Structures
----------------------------------------------------------------------------*/

/**
**		Definition of flags.
**		Each flag map an int value to a SCM symbol
*/
typedef struct _io_flag_def_ {
	char*				ident;				/// Flag name
	int						value;				/// Flag value
} IOFlagDef;

/**
**		The IOFieldDef structure define each field in a structure
**		The name of the field is used to identify the field in Scheme LIST.
**
**		The fonction receive as parameter (binaryform), a void pointer to the field
*/
typedef struct _io_field_def_ {
	char*				name;				/// Name of the field ( used as ccl ident )
#if defined(USE_GUILE) || defined(USE_SIOD)
	void				(*convertfunc) (SCM scmfrom, void* binaryform, void* para);
											/// Function to load/save the field
#elif defined(USE_LUA)
#endif
	void*				offset;				/// Offset of the field in the structure
	void*				para;				/// Parameter passed to the field
} IOFieldDef;

/**
**		The IOStructDef define a full structure, for loading & saving.
**		It is meant to be used with the IOStruct or IOStruct parameter,
**		to load or save a structure (depending on IOLoadingMode value)
**
*/
typedef struct _io_struct_def_ {
	char*				name;				/// Name of the structure (debugging only)
	int				 size;				/// Size of the structure (for malloc)
	int				 array_size;		/// Number of element when in an array
	IOFieldDef				defs[];				/// Definition of fields, terminated by a null field
} IOStructDef;

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/
extern int		  IOLoadingMode;		/// IOxxx functions do load (1) or save (0) struct
extern unsigned int IOTabLevel;				/// When saving to ccl, current indentation level
extern CLFile	  *IOOutFile;				/// When saving to ccl, output file

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

	/// Print "IOTabLevel" tabs on the ccl output
extern void IOPrintTabs();
#if defined(USE_GUILE) || defined(USE_SIOD)
	/// Handle saving/loading of structure
extern void IOStruct(SCM scmform, void* binaryform, void* para);
	/// Handle saving/loading a pointer to a structure.
extern void IOStructPtr(SCM scmform, void* binaryform, void* para);
	/// Handle loading a fixed size array of structure.
extern void IOStructArray(SCM from, void* binaryform, void* para);
	/// Handle saving/loading linked list.
extern void IOLinkedList(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading tables.
extern void IOTable(SCM scmfrom, void* binaryform, void* para);
	/// Handle the case of saving/loading pointers which are null
extern int IOHandleNullPtr(SCM scmfrom, void* binaryform);
	/// Handle saving/loading of int
extern void IOInt(SCM scmform, void* binaryform, void* para);
	/// Handle saving/loading of bool stored as int
extern void IOBool(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading of bool stored as char
extern void IOCharBool(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading of string
extern void IOString(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading of SCM
extern void IOCcl(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading of flag stored in char
extern void IOCharFlag(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading a fixed length string
extern void IOStrBuffer(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading a dynamic array of int
extern void IOIntArrayPtr(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading an already allocated array of int
extern void IOIntArray(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading an unittype pointer (UnitType*)
extern void IOUnitTypePtr(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading a reference to an unit (Unit*)
extern void IOUnitPtr(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading a reference to an upgrade (Upgrade*)
extern void IOUpgradePtr(SCM scmfrom, void* binaryform, void* para);
	/// Handle saving/loading a reference to a player (Player*)
extern void IOPlayerPtr(SCM scmfrom, void* binaryform, void* para);
#elif defined(USE_LUA)
#endif

//@}

#endif				// !__CCL_H__
