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
//	(c) Copyright 2000,2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
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
	exit(-1);
    }
    while( i-- ) {
	*cp++=gh_scm2newstr(gh_car(list),NULL);
	list=gh_cdr(list);
    }
    *cp=NULL;

    return SCM_UNSPECIFIED;
}

/**
**	Parse the special slot part of a tileset definition
**
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
**	@param list	Tagged list defining a solid slot.
*/
local int DefineTilesetParseSolid(Tileset* tileset,int index,SCM list)
{
    SCM value;
    SCM data;
    int i;
    int l;

    tileset->Table=realloc(tileset->Table,(index+16)*sizeof(*tileset->Table));
    if( !tileset->Table ) {
	fprintf(stderr,"out of memory.\n");
	exit(-1);
    }

    value=gh_car(list);			// name
    list=gh_cdr(list);

    //
    //	Parse the list:	flags of the slot
    //
    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);

	if( !gh_symbol_p(value) ) {
	    break;
	}

	//
	//	Flags are only needed for the editor
	//
	if( gh_eq_p(value,gh_symbol2scm("water")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("no-building")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("forest")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("rock")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("wall")) ) {
	} else {
	    errl("solid: unsupported tag",value);
	}
    }

    //
    //	Vector: the tiles.
    //
    l=gh_vector_length(value);
    for( i=0; i<l; ++i ) {
	data=gh_vector_ref(value,gh_int2scm(i));
	tileset->Table[index+i]=gh_scm2int(data);
    }
    while( i<16 ) {
	tileset->Table[index+i++]=0;
    }

    return index+16;
}

/**
**	Parse the mixed slot part of a tileset definition
**
**	@param list	Tagged list defining a mixed slot.
*/
local int DefineTilesetParseMixed(Tileset* tileset,int index,SCM list)
{
    SCM value;
    SCM data;
    int i;
    int l;

    tileset->Table=realloc(tileset->Table,(index+256)*sizeof(*tileset->Table));
    if( !tileset->Table ) {
	fprintf(stderr,"out of memory.\n");
	exit(-1);
    }

    value=gh_car(list);			// base name
    list=gh_cdr(list);
    value=gh_car(list);			// mixed name
    list=gh_cdr(list);

    //
    //	Parse the list:	flags of the slot
    //
    while( !gh_null_p(list) ) {
	value=gh_car(list);

	if( !gh_symbol_p(value) ) {
	    break;
	}
	list=gh_cdr(list);

	//
	//	Flags are only needed for the editor
	//
	if( gh_eq_p(value,gh_symbol2scm("water")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("no-building")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("forest")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("rock")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("wall")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("coast")) ) {
	} else {
	    errl("solid: unsupported tag",value);
	}
    }

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
	}
	while( i<16 ) {
	    tileset->Table[index+i++]=0;
	}
	index+=16;
    }

    return index;
}

/**
**	Parse the slot part of a tileset definition
**
**	@param list	Tagged list defining a slot.
*/
local void DefineTilesetParseSlot(Tileset* tileset,SCM list)
{
    SCM value;
    SCM data;
    int index;

    index=0;
    tileset->Table=malloc(16*sizeof(*tileset->Table));
    if( !tileset->Table ) {
	fprintf(stderr,"out of memory.\n");
	exit(-1);
    }

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
		free(Tilesets[type]->Class);
		free(Tilesets[type]->Name);
		free(Tilesets[type]->File);
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
	exit(-1);
    }
    Tilesets[type]=tileset=malloc(sizeof(Tileset));
    if( !tileset ) {
	fprintf(stderr,"out of memory.\n");
	exit(-1);
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

	if( gh_eq_p(value,gh_symbol2scm("class")) ) {
	    tileset->Class=gh_scm2newstr(data,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("name")) ) {
	    tileset->Name=gh_scm2newstr(data,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("image")) ) {
	    tileset->File=gh_scm2newstr(data,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("palette")) ) {
	    tileset->PaletteFile=gh_scm2newstr(data,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("slots")) ) {
	    DefineTilesetParseSlot(tileset,data);
	} else if( gh_eq_p(value,gh_symbol2scm("animations")) ) {
	    DebugLevel0Fn("Animations not supported.\n");
	} else if( gh_eq_p(value,gh_symbol2scm("objects")) ) {
	    DebugLevel0Fn("objects not supported.\n");
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
