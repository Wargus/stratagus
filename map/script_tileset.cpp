//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name ccl_tileset.c	-	The tileset ccl functions. */
//
//	(c) Copyright 2000-2002 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "ccl.h"
#include "tileset.h"
#include "map.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Define tileset mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
local SCM CclDefineTilesetWcNames(SCM list)
{
    int i;
    char** cp;

    if( (cp=TilesetWcNames) ) {		// Free all old names
	while( *cp ) {
	    free(*cp++);
	}
	free(TilesetWcNames);
    }

    //
    //	Get new table.
    //
    i=gh_length(list);
    TilesetWcNames=cp=malloc((i+1)*sizeof(char*));
    if( !cp ) {
	fprintf(stderr,"out of memory.\n");
	ExitFatal(-1);
    }
    while( i-- ) {
	*cp++=gh_scm2newstr(gh_car(list),NULL);
	list=gh_cdr(list);
    }
    *cp=NULL;

    return SCM_UNSPECIFIED;
}

/**
**	Extend tables of the tileset.
**
**	@param tileset	Tileset to be extended.
**	@param tiles	Number of tiles.
*/
local void ExtendTilesetTables(Tileset * tileset, int tiles)
{
    tileset->Table = realloc(tileset->Table, tiles * sizeof(*tileset->Table));
    if (!tileset->Table) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }
    tileset->FlagsTable =
	realloc(tileset->FlagsTable, tiles * sizeof(*tileset->FlagsTable));
    if (!tileset->FlagsTable) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }
    tileset->BasicNameTable = realloc(tileset->BasicNameTable,
	tiles * sizeof(*tileset->BasicNameTable));
    if (!tileset->BasicNameTable) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }
    tileset->MixedNameTable = realloc(tileset->MixedNameTable,
	tiles * sizeof(*tileset->MixedNameTable));
    if (!tileset->MixedNameTable) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }
}

/**
**	Parse the name field in tileset definition.
**
**	@param tileset	Tileset currently parsed.
**	@param list	List with name.
*/
local int TilesetParseName(Tileset* tileset, SCM list)
{
    char *ident;
    int i;

    ident = gh_scm2newstr(gh_car(list), NULL);
    for (i = 0; i < tileset->NumNames; ++i) {
	if (!strcmp(ident, tileset->TileNames[i])) {
	    free(ident);
	    return i;
	}
    }
    tileset->TileNames = realloc(tileset->TileNames,
	++tileset->NumNames * sizeof(*tileset->TileNames));
    tileset->TileNames[i] = ident;

    return i;
}

/**
**	Parse the flag section of a tile definition.
**
**	@param list	list of flags.
**	@param back	pointer for the flags (return).
**
**	@return		remaining list
*/
local SCM ParseTilesetTileFlags(SCM list, int* back)
{
    int flags;

    //
    //  Parse the list: flags of the slot
    //
    flags = 0;
    while (!gh_null_p(list)) {
	SCM value;

	value = gh_car(list);

	if (!gh_symbol_p(value)) {
	    break;
	}
	list = gh_cdr(list);

	//
	//      Flags are only needed for the editor
	//
	if (gh_eq_p(value, gh_symbol2scm("water"))) {
	    flags |= MapFieldWaterAllowed;
	} else if (gh_eq_p(value, gh_symbol2scm("land"))) {
	    flags |= MapFieldLandAllowed;
	} else if (gh_eq_p(value, gh_symbol2scm("coast"))) {
	    flags |= MapFieldCoastAllowed;
	} else if (gh_eq_p(value, gh_symbol2scm("no-building"))) {
	    flags |= MapFieldNoBuilding;
	} else if (gh_eq_p(value, gh_symbol2scm("unpassable"))) {
	    flags |= MapFieldUnpassable;
	} else if (gh_eq_p(value, gh_symbol2scm("wall"))) {
	    flags |= MapFieldWall;
	} else if (gh_eq_p(value, gh_symbol2scm("rock"))) {
	    flags |= MapFieldRocks;
	} else if (gh_eq_p(value, gh_symbol2scm("forest"))) {
	    flags |= MapFieldForest;
	} else if (gh_eq_p(value, gh_symbol2scm("land-unit"))) {
	    flags |= MapFieldLandUnit;
	} else if (gh_eq_p(value, gh_symbol2scm("air-unit"))) {
	    flags |= MapFieldAirUnit;
	} else if (gh_eq_p(value, gh_symbol2scm("sea-unit"))) {
	    flags |= MapFieldSeaUnit;
	} else if (gh_eq_p(value, gh_symbol2scm("building"))) {
	    flags |= MapFieldBuilding;
	} else if (gh_eq_p(value, gh_symbol2scm("human"))) {
	    flags |= MapFieldHuman;
	} else {
	    errl("solid: unsupported tag", value);
	}
    }
    *back = flags;
    return list;
}

/**
**	Parse the special slot part of a tileset definition
**
**	@param tileset	Tileset to be filled.
**	@param list	Tagged list defining a special slot.
*/
local void DefineTilesetParseSpecial(Tileset* tileset,SCM list)
{
    SCM value;
    SCM data;
    int i;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	data=gh_car(list);
	list=gh_cdr(list);

	//
	//	Extra-trees
	//
	if( gh_eq_p(value,gh_symbol2scm("extra-trees")) ) {
	    if( gh_vector_length(data)!=6 ) {
		errl("extra-trees: Wrong vector length",data);
	    }
	    for( i=0; i<6; ++i ) {
		value=gh_vector_ref(data,gh_int2scm(i));
		tileset->ExtraTrees[i]=gh_scm2int(value);
	    }
	//
	//	top-one-tree, mid-one-tree, bot-one-tree
	//
	} else if( gh_eq_p(value,gh_symbol2scm("top-one-tree")) ) {
	    tileset->TopOneTree=gh_scm2int(data);
	} else if( gh_eq_p(value,gh_symbol2scm("mid-one-tree")) ) {
	    tileset->MidOneTree=gh_scm2int(data);
	} else if( gh_eq_p(value,gh_symbol2scm("bot-one-tree")) ) {
	    tileset->BotOneTree=gh_scm2int(data);
	//
	//	removed-tree
	//
	} else if( gh_eq_p(value,gh_symbol2scm("removed-tree")) ) {
	    tileset->RemovedTree=gh_scm2int(data);
	//
	//	growing-tree
	//
	} else if( gh_eq_p(value,gh_symbol2scm("growing-tree")) ) {
	    if( gh_vector_length(data)!=2 ) {
		errl("growing-tree: Wrong vector length",data);
	    }
	    for( i=0; i<2; ++i ) {
		value=gh_vector_ref(data,gh_int2scm(i));
		tileset->GrowingTree[i]=gh_scm2int(value);
	    }

	//
	//	extra-rocks
	//
	} else if( gh_eq_p(value,gh_symbol2scm("extra-rocks")) ) {
	    if( gh_vector_length(data)!=6 ) {
		errl("extra-rocks: Wrong vector length",data);
	    }
	    for( i=0; i<6; ++i ) {
		value=gh_vector_ref(data,gh_int2scm(i));
		tileset->ExtraRocks[i]=gh_scm2int(value);
	    }
	//
	//	top-one-rock, mid-one-rock, bot-one-rock
	//
	} else if( gh_eq_p(value,gh_symbol2scm("top-one-rock")) ) {
	    tileset->TopOneRock=gh_scm2int(data);
	} else if( gh_eq_p(value,gh_symbol2scm("mid-one-rock")) ) {
	    tileset->MidOneRock=gh_scm2int(data);
	} else if( gh_eq_p(value,gh_symbol2scm("bot-one-rock")) ) {
	    tileset->BotOneRock=gh_scm2int(data);
	//
	//	removed-rock
	//
	} else if( gh_eq_p(value,gh_symbol2scm("removed-rock")) ) {
	    tileset->RemovedRock=gh_scm2int(data);
	} else {
	    errl("special: unsupported tag",value);
	}
    }
}

/**
**	Parse the solid slot part of a tileset definition
**
**	@param tileset	Tileset to be filled.
**	@param index	Current table index.
**	@param list	Tagged list defining a solid slot.
*/
local int DefineTilesetParseSolid(Tileset* tileset,int index,SCM list)
{
    SCM value;
    SCM data;
    int i;
    int f;
    int l;
    int basic_name;

    ExtendTilesetTables(tileset,index+16);

    basic_name=TilesetParseName(tileset,list);	// base name
    list=gh_cdr(list);

    list=ParseTilesetTileFlags(list,&f);

    //
    //	Vector: the tiles.
    //
    value = gh_car(list);
    l=gh_vector_length(value);

    // hack for sc tilesets, remove when fixed
    if( l>16 ) {
	ExtendTilesetTables(tileset,index+l);
    }

    for( i=0; i<l; ++i ) {
	data=gh_vector_ref(value,gh_int2scm(i));
	tileset->Table[index+i]=gh_scm2int(data);
	tileset->FlagsTable[index+i]=f;
	tileset->BasicNameTable[index+i]=basic_name;
	tileset->MixedNameTable[index+i]=0;
    }
    while( i<16 ) {
	tileset->Table[index+i]=0;
	tileset->FlagsTable[index+i]=0;
	tileset->BasicNameTable[index+i]=0;
	tileset->MixedNameTable[index+i]=0;
	++i;
    }

    // hack for sc tilesets, remove when fixed
    if( l<16 ) {
	return index+16;
    }
    return index+l;
//    return index+16;
}

/**
**	Parse the mixed slot part of a tileset definition
**
**	@param tileset	Tileset to be filled.
**	@param index	Current table index.
**	@param list	Tagged list defining a mixed slot.
*/
local int DefineTilesetParseMixed(Tileset* tileset,int index,SCM list)
{
    SCM value;
    SCM data;
    int i;
    int l;
    int f;
    int basic_name;
    int mixed_name;

    ExtendTilesetTables(tileset,index+256);

    basic_name=TilesetParseName(tileset,list);	// base name
    list=gh_cdr(list);
    mixed_name=TilesetParseName(tileset,list);	// mixed name
    list=gh_cdr(list);

    list=ParseTilesetTileFlags(list,&f);

    //
    //	Parse the list:	slots FIXME: no error checking number of slots
    //
    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);


	//
	//	Vector: the tiles.
	//
	l=gh_vector_length(value);
	for( i=0; i<l; ++i ) {
	    data=gh_vector_ref(value,gh_int2scm(i));
	    tileset->Table[index+i]=gh_scm2int(data);
	    tileset->FlagsTable[index+i]=f;
	    tileset->BasicNameTable[index+i]=basic_name;
	    tileset->MixedNameTable[index+i]=mixed_name;
	}
	while( i<16 ) {			// Fill missing slots
	    tileset->Table[index+i]=0;
	    tileset->FlagsTable[index+i]=0;
	    tileset->BasicNameTable[index+i]=0;
	    tileset->MixedNameTable[index+i]=0;
	    ++i;
	}
	index+=16;
    }

    return index;
}

/**
**	Parse the slot part of a tileset definition
**
**	@param tileset	Tileset to be filled.
**	@param list	Tagged list defining a slot.
*/
local void DefineTilesetParseSlot(Tileset* tileset,SCM list)
{
    SCM value;
    SCM data;
    int index;

    index=0;
    tileset->Table = malloc(16 * sizeof(*tileset->Table));
    if (!tileset->Table) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }
    tileset->FlagsTable =
	malloc(16 * sizeof(*tileset->FlagsTable));
    if (!tileset->FlagsTable) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }
    tileset->BasicNameTable = malloc(16 * sizeof(*tileset->BasicNameTable));
    if (!tileset->BasicNameTable) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }
    tileset->MixedNameTable = malloc(16 * sizeof(*tileset->MixedNameTable));
    if (!tileset->MixedNameTable) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }
    tileset->TileNames = malloc(sizeof(char*));
    if (!tileset->TileNames) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }
    tileset->TileNames[0] = strdup("unused");
    tileset->NumNames = 1;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	data=gh_car(list);
	list=gh_cdr(list);

	//
	//	special part
	//
	if( gh_eq_p(value,gh_symbol2scm("special")) ) {
	    DefineTilesetParseSpecial(tileset,data);
	//
	//	solid part
	//
	} else if( gh_eq_p(value,gh_symbol2scm("solid")) ) {
	    index=DefineTilesetParseSolid(tileset,index,data);
	//
	//	mixed part
	//
	} else if( gh_eq_p(value,gh_symbol2scm("mixed")) ) {
	    index=DefineTilesetParseMixed(tileset,index,data);
	} else {
	    errl("slots: unsupported tag",value);
	}
    }
    tileset->NumTiles=index;
}

/**
**	Define tileset
**
**	@param list	Tagged list defining a tileset.
*/
local SCM CclDefineTileset(SCM list)
{
    SCM value;
    SCM data;
    int type;
    Tileset* tileset;
    char* ident;

    value=gh_car(list);
    list=gh_cdr(list);

    if( !gh_symbol_p(value) ) {
	errl("illegal tileset slot name",value);
    }
    ident=gh_scm2newstr(value,NULL);

    //
    //	Find the tile set.
    //
    if( Tilesets ) {
	for( type=0; type<NumTilesets; ++type ) {
	    if( !strcmp(Tilesets[type]->Ident,ident) ) {
		free(Tilesets[type]->Ident);
		free(Tilesets[type]->File);
		free(Tilesets[type]->Class);
		free(Tilesets[type]->Name);
		free(Tilesets[type]->ImageFile);
		free(Tilesets[type]->PaletteFile);
		free(Tilesets[type]->Table);
		free(Tilesets[type]->TileTypeTable);
		free(Tilesets[type]->AnimationTable);
		free(Tilesets[type]);
		break;
	    }
	}
	if( type==NumTilesets ) {
	    Tilesets=realloc(Tilesets,++NumTilesets*sizeof(*Tilesets));
	}
    } else {
	Tilesets=malloc(sizeof(*Tilesets));
	type=0;
	++NumTilesets;
    }
    if( !Tilesets ) {
	fprintf(stderr,"out of memory.\n");
	ExitFatal(-1);
    }
    Tilesets[type]=tileset=calloc(sizeof(Tileset),1);
    if( !tileset ) {
	fprintf(stderr,"out of memory.\n");
	ExitFatal(-1);
    }
    Tilesets[type]->Ident=ident;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	list=gh_cdr(list);
	data=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("file")) ) {
	    tileset->File=gh_scm2newstr(data,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("class")) ) {
	    tileset->Class=gh_scm2newstr(data,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("name")) ) {
	    tileset->Name=gh_scm2newstr(data,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("image")) ) {
	    tileset->ImageFile=gh_scm2newstr(data,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("palette")) ) {
	    tileset->PaletteFile=gh_scm2newstr(data,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("slots")) ) {
	    DefineTilesetParseSlot(tileset,data);
	} else if( gh_eq_p(value,gh_symbol2scm("animations")) ) {
	    DebugLevel0Fn("Animations not supported.\n");
	} else if( gh_eq_p(value,gh_symbol2scm("objects")) ) {
	    DebugLevel0Fn("Objects not supported.\n");
	} else {
	    errl("Unsupported tag",value);
	}
    }
    return list;
}

/**
**	Register CCL features for tileset.
*/
global void TilesetCclRegister(void)
{
    gh_new_procedureN("define-tileset-wc-names",CclDefineTilesetWcNames);
    gh_new_procedureN("define-tileset",CclDefineTileset);
}

//@}
