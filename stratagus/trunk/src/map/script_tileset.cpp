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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"

#if defined(USE_CCL)	// {

#include "ccl.h"
#include "tileset.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Parse tileset definition.
**
**	@param slot	Slot name
**	@param name	Reference name
**	@param file	Graphic file
**	@param palette	Palette file
**	@param table	Conversion table
*/
local SCM CclTileset(SCM slot,SCM name,SCM file,SCM palette,SCM table)
{
    int type;
    int i;
    unsigned short* wp;

    if( !gh_symbol_p(slot) ) {
	fprintf(stderr,"Illegal tileset slot name\n");
	return SCM_UNSPECIFIED;
    }
    if( slot==gh_symbol2scm("tileset-summer") ) {
	type=TilesetSummer;
    } else if( slot==gh_symbol2scm("tileset-winter") ) {
	type=TilesetWinter;
    } else if( slot==gh_symbol2scm("tileset-wasteland") ) {
	type=TilesetWasteland;
    } else if( slot==gh_symbol2scm("tileset-swamp") ) {
	type=TilesetSwamp;
    } else {
	fprintf(stderr,"Wrong tileset slot name\n");
	return SCM_UNSPECIFIED;
    }
    Tilesets[type].Name=gh_scm2newstr(name,NULL);
    Tilesets[type].File=gh_scm2newstr(file,NULL);
    Tilesets[type].PaletteFile=gh_scm2newstr(palette,NULL);

    // CONVERT TABLE!!
    if( gh_vector_length(table)!=2528 ) {	// 0x9E0
	fprintf(stderr,"Wrong conversion table length\n");
	return SCM_UNSPECIFIED;
    }

    Tilesets[type].Table=wp=malloc(sizeof(*Tilesets[type].Table)*2528);
    for( i=0; i<2528; ++i ) {
	wp[i]=gh_scm2int(gh_vector_ref(table,gh_int2scm(i)));
    }

    return SCM_UNSPECIFIED;
}

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
    while( i-- ) {
	*cp++=gh_scm2newstr(gh_car(list),NULL);
	list=gh_cdr(list);
    }
    *cp=NULL;

    return SCM_UNSPECIFIED;
}

/**
**	Define tileset
**
*/
local SCM CclDefineTileset(SCM list)
{
    // FIXME: write this
    return list;
}

/**
**	Register CCL features for tileset.
*/
global void TilesetCclRegister(void)
{
    // FIXME: will be removed
    gh_new_procedure5_0("tileset",CclTileset);

    gh_new_procedureN("define-tileset-wc-names",CclDefineTilesetWcNames);
    gh_new_procedureN("define-tileset",CclDefineTileset);
}

#endif	// } defined(USE_CCL)

//@}
