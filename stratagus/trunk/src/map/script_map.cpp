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
/**@name ccl_map.c	-	The map ccl functions. */
//
//	(c) Copyright 1999-2001 by Lutz Sammer
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

#if defined(USE_CCL)	// {

#include "ccl.h"
#include "map.h"
#include "minimap.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Parse a freecraft map.
**
**	@param list	list of tuples keyword data
*/
local SCM CclFreeCraftMap(SCM list)
{
    SCM value;
    SCM data;
    char* str;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("version")) ) {
	    char buf[32];

	    data=gh_car(list);
	    list=gh_cdr(list);
	    str=gh_scm2newstr(data,NULL);
	    sprintf(buf,FreeCraftFormatString,
		    FreeCraftFormatArgs(FreeCraftVersion));
	    if( strcmp(buf,str) ) {
		fprintf(stderr,"Warning not saved with this version.\n");
	    }
	    free(str);
	} else if( gh_eq_p(value,gh_symbol2scm("description")) ) {
	    data=gh_car(list);
	    list=gh_cdr(list);

	    str=gh_scm2newstr(data,NULL);
	    strncpy(TheMap.Description,str,sizeof(TheMap.Description));
	    free(str);
	} else if( gh_eq_p(value,gh_symbol2scm("the-map")) ) {
	    data=gh_car(list);
	    list=gh_cdr(list);

	    while( !gh_null_p(data) ) {
		value=gh_car(data);
		data=gh_cdr(data);

		if( gh_eq_p(value,gh_symbol2scm("terrain")) ) {
		    int i;

		    value=gh_car(data);
		    data=gh_cdr(data);

		    free(TheMap.TerrainName);
		    TheMap.TerrainName=str=gh_scm2newstr(gh_car(value),NULL);
		    //
		    //	Lookup the index of this tileset.
		    //
		    for( i=0; TilesetWcNames[i]
			    && strcmp(str,TilesetWcNames[i]); ++i ) {
		    }
		    TheMap.Terrain=i;
		    // Ignore: str=gh_scm2newstr(gh_cadr(value),NULL);
		   
		} else if( gh_eq_p(value,gh_symbol2scm("size")) ) {
		    value=gh_car(data);
		    data=gh_cdr(data);

		    TheMap.Width=gh_scm2int(gh_car(value));
		    TheMap.Height=gh_scm2int(gh_cadr(value));

		    free(TheMap.Fields);
		    TheMap.Fields=calloc(TheMap.Width*TheMap.Height,
			    sizeof(*TheMap.Fields));
		    InitUnitCache();

		} else if( gh_eq_p(value,gh_symbol2scm("fog-of-war")) ) {

		    TheMap.NoFogOfWar=0;

		} else if( gh_eq_p(value,gh_symbol2scm("no-fog-of-war")) ) {

		    TheMap.NoFogOfWar=1;

		} else if( gh_eq_p(value,gh_symbol2scm("map-fields")) ) {
		    int i;

		    value=gh_car(data);
		    data=gh_cdr(data);

		    i=gh_length(value);
		    if( i!=TheMap.Width*TheMap.Height ) {
			fprintf(stderr,"Wrong tile table length %d\n",i);
		    }
		    i=0;
		    while( !gh_null_p(value) ) {
			SCM field;

			field=gh_car(value);
			value=gh_cdr(value);

			TheMap.Fields[i].Tile=gh_scm2int(gh_car(field));
			field=gh_cdr(field);
			TheMap.Fields[i].SeenTile=gh_scm2int(gh_car(field));
			field=gh_cdr(field);
			while( !gh_null_p(field) ) {
			    if( gh_exact_p(gh_car(field)) ) {
				TheMap.Fields[i].Value=
					gh_scm2int(gh_car(field));

			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("visible")) ) {
				TheMap.Fields[i].Flags|=MapFieldVisible;
			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("explored")) ) {
				TheMap.Fields[i].Flags|=MapFieldExplored;

			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("human")) ) {
				TheMap.Fields[i].Flags|=MapFieldHuman;

			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("land")) ) {
				TheMap.Fields[i].Flags|=MapFieldLandAllowed;
			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("coast")) ) {
				TheMap.Fields[i].Flags|=MapFieldCoastAllowed;
			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("water")) ) {
				TheMap.Fields[i].Flags|=MapFieldWaterAllowed;

			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("mud")) ) {
				TheMap.Fields[i].Flags|=MapFieldNoBuilding;
			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("block")) ) {
				TheMap.Fields[i].Flags|=MapFieldUnpassable;

			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("wall")) ) {
				TheMap.Fields[i].Flags|=MapFieldWall;
			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("rock")) ) {
				TheMap.Fields[i].Flags|=MapFieldRocks;
			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("wood")) ) {
				TheMap.Fields[i].Flags|=MapFieldForest;

			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("ground")) ) {
				TheMap.Fields[i].Flags|=MapFieldLandUnit;
			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("air")) ) {
				TheMap.Fields[i].Flags|=MapFieldAirUnit;
			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("sea")) ) {
				TheMap.Fields[i].Flags|=MapFieldSeaUnit;
			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("building")) ) {
				TheMap.Fields[i].Flags|=MapFieldBuilding;

			    } else {
			       // FIXME: this leaves a half initialized map
			       errl("Unsupported tag",value);
			    }
			    field=gh_cdr(field);
			}
			++i;
		    }

		} else {
		   // FIXME: this leaves a half initialized map
		   errl("Unsupported tag",value);
		}
	    }

	} else {
	   // FIXME: this leaves a half initialized map
	   errl("Unsupported tag",value);
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Reveal the complete map.
**	FIXME: only functional in init file!
*/
local SCM CclRevealMap(void)
{
    if( !CclInConfigFile ) {
	fprintf(stderr,__FUNCTION__": only in config file supported\n");
    }
    FlagRevealMap=1;

    return SCM_UNSPECIFIED;
}

/**
**	Set fog of war on/off.
**
**	@param flag	True = turning fog of war on, false = off.
**
**	@return		The old state of fog of war.
*/
local SCM CclSetFogOfWar(SCM flag)
{
    int old;

    old=!TheMap.NoFogOfWar;
    TheMap.NoFogOfWar=!gh_scm2bool(flag);

    return gh_bool2scm(old);
}

#if 1

// FIXME: Should be removed

/**
**	Enable fog of war.
*/
local SCM CclFogOfWar(void)
{
    TheMap.NoFogOfWar=0;

    return SCM_UNSPECIFIED;
}

/**
**	Disable fog of war.
*/
local SCM CclNoFogOfWar(void)
{
    TheMap.NoFogOfWar=1;

    return SCM_UNSPECIFIED;
}

#endif

/**
**	Enable display of terrain in minimap.
*/
local SCM CclMinimapTerrain(void)
{
    MinimapWithTerrain=1;

    return SCM_UNSPECIFIED;
}

/**
**	Disable display of terrain in minimap.
*/
local SCM CclNoMinimapTerrain(void)
{
    MinimapWithTerrain=0;

    return SCM_UNSPECIFIED;
}

/**
**	Original fog of war.
*/
local SCM CclOriginalFogOfWar(void)
{
    OriginalFogOfWar=1;

    if( !CclInConfigFile ) {
	InitMapFogOfWar();
    }

    return SCM_UNSPECIFIED;
}

/**
**	Gray style fog of war.
*/
local SCM CclGrayFogOfWar(void)
{
    OriginalFogOfWar=0;

    if( !CclInConfigFile ) {
	InitMapFogOfWar();
    }

    return SCM_UNSPECIFIED;
}

/**
**	Gray style fog of war contrast.
*/
local SCM CclFogOfWarContrast(SCM contrast)
{
    int i;

    i=gh_scm2int(contrast);
    if( i<0 || i>400 ) {
	fprintf(stderr,__FUNCTION__": contrast should be 0-400\n");
	i=100;
    }
    FogOfWarContrast=i;

    if( !CclInConfigFile ) {
	InitMapFogOfWar();
    }

    return SCM_UNSPECIFIED;
}

/**
**	Gray style fog of war brightness.
*/
local SCM CclFogOfWarBrightness(SCM brightness)
{
    int i;

    i=gh_scm2int(brightness);
    if( i<-100 || i>100 ) {
	fprintf(stderr,__FUNCTION__": brightness should be -100-100\n");
	i=0;
    }
    FogOfWarBrightness=i;

    if( !CclInConfigFile ) {
	InitMapFogOfWar();
    }

    return SCM_UNSPECIFIED;
}

/**
**	Gray style fog of war saturation.
*/
local SCM CclFogOfWarSaturation(SCM saturation)
{
    int i;

    i=gh_scm2int(saturation);
    if( i<-100 || i>200 ) {
	fprintf(stderr,__FUNCTION__": saturation should be -100-200\n");
	i=0;
    }
    FogOfWarSaturation=i;

    if( !CclInConfigFile ) {
	InitMapFogOfWar();
    }

    return SCM_UNSPECIFIED;
}

/**
**	Forest regeneration speed.
*/
local SCM CclForestRegeneration(SCM speed)
{
    int i;

    i=gh_scm2int(speed);
    if( i<0 || i>255 ) {
	fprintf(stderr,__FUNCTION__": regneration speed should be 0-255\n");
	i=0;
    }
    ForestRegeneration=i;

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for map.
*/
global void MapCclRegister(void)
{
    gh_new_procedureN("freecraft-map",CclFreeCraftMap);
    gh_new_procedure0_0("reveal-map",CclRevealMap);

    gh_new_procedure1_0("set-fog-of-war!",CclSetFogOfWar);

#if 1
    // FIXME: should be removed

    gh_new_procedure0_0("fog-of-war",CclFogOfWar);
    gh_new_procedure0_0("no-fog-of-war",CclNoFogOfWar);
#endif

    gh_new_procedure0_0("minimap-terrain",CclMinimapTerrain);
    gh_new_procedure0_0("no-minimap-terrain",CclNoMinimapTerrain);

    gh_new_procedure0_0("original-fog-of-war",CclOriginalFogOfWar);
    gh_new_procedure0_0("gray-fog-of-war",CclGrayFogOfWar);

    gh_new_procedure1_0("fog-of-war-contrast",CclFogOfWarContrast);
    gh_new_procedure1_0("fog-of-war-brightness",CclFogOfWarBrightness);
    gh_new_procedure1_0("fog-of-war-saturation",CclFogOfWarSaturation);

    gh_new_procedure1_0("forest-regeneration",CclForestRegeneration);
}

#endif	// } defined(USE_CCL)

//@}
