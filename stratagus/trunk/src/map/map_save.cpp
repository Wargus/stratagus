//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name map_save.c	-	Saving the map. */
//
//	(c) Copyright 2001-2003 by Lutz Sammer
//
//	Stratagus is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	Stratagus is distributed in the hope that it will be useful,
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
#include <string.h>

#include "stratagus.h"
#include "map.h"
#include "minimap.h"
#include "player.h"
#include "unit.h"
#include "pathfinder.h"
#include "pud.h"
#include "ui.h"

#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Save the complete map.
**
**	@param file	Output file.
*/
global void SaveMap(FILE* file)
{
    int w;
    int h;
    int i;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: map $Id$\n");

    fprintf(file,"(stratagus-map\n");

    fprintf(file,"  'version \"" StratagusFormatString "\"\n",
	    StratagusFormatArgs(StratagusVersion));
    fprintf(file,"  'description \"%s\"\n",TheMap.Description);

    fprintf(file,"  'the-map '(\n");

    // FIXME: Why terrain? TheMap->Tileset->Class should be correct
    fprintf(file,"  terrain (%s \"%s\")\n"
	    ,TheMap.TerrainName,Tilesets[TheMap.Terrain]->Class);

    fprintf(file,"  size (%d %d)\n",TheMap.Width,TheMap.Height);
    fprintf(file,"  %s\n",TheMap.NoFogOfWar ? "no-fog-of-war" : "fog-of-war");

    fprintf(file,"  map-fields (\n");
    for( h=0; h<TheMap.Height; ++h ) {
	fprintf(file,"  ; %d\n",h);
	for( w=0; w<TheMap.Width; ++w ) {
	    MapField* mf;

	    mf=&TheMap.Fields[h*TheMap.Width+w];
	    fprintf(file,"  (%3d %3d",mf->Tile,mf->SeenTile);
	    if( mf->Value ) {
		fprintf(file," %d",mf->Value);
	    }
	    for( i=0; i < PlayerMax; ++i ) {
		if( mf->Visible[i] == 1) {
		    fprintf(file," explored %d",i);
		}
	    }
	    if( mf->Flags&MapFieldHuman ) {
		fprintf(file," human");
	    }
	    if( mf->Flags&MapFieldLandAllowed ) {
		fprintf(file," land");
	    }
	    if( mf->Flags&MapFieldCoastAllowed ) {
		fprintf(file," coast");
	    }
	    if( mf->Flags&MapFieldWaterAllowed ) {
		fprintf(file," water");
	    }
	    if( mf->Flags&MapFieldNoBuilding ) {
		fprintf(file," mud");
	    }
	    if( mf->Flags&MapFieldUnpassable ) {
		fprintf(file," block");
	    }
	    if( mf->Flags&MapFieldWall ) {
		fprintf(file," wall");
	    }
	    if( mf->Flags&MapFieldRocks ) {
		fprintf(file," rock");
	    }
	    if( mf->Flags&MapFieldForest ) {
		fprintf(file," wood");
	    }
#if 1
	    // Not Required for save
	    // These are required for now, UnitType::FieldFlags is 0 until
	    // UpdateStats is called which is after the game is loaded
	    if( mf->Flags&MapFieldLandUnit ) {
		fprintf(file," ground");
	    }
	    if( mf->Flags&MapFieldAirUnit ) {
		fprintf(file," air");
	    }
	    if( mf->Flags&MapFieldSeaUnit ) {
		fprintf(file," sea");
	    }
	    if( mf->Flags&MapFieldBuilding ) {
		fprintf(file," building");
	    }
#endif
	    if( w&1 ) {
		fprintf(file,")\n");
	    } else {
		fprintf(file,")\t");
	    }
	}
    }
    fprintf(file,")))\n");
}

//@}
