//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name ccl_map.c	-	The map ccl functions. */
//
//	(c) Copyright 1999-2002 by Lutz Sammer
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
#include "map.h"
#include "minimap.h"
#include "actions.h"
#include "campaign.h"

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

    if( !TheMap.Info ) {
	TheMap.Info=calloc(1, sizeof(MapInfo));
    }

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
	} else if( gh_eq_p(value,gh_symbol2scm("uid")) ) {
	    TheMap.Info->MapUID = gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("description")) ) {
	    data=gh_car(list);
	    list=gh_cdr(list);

	    str=gh_scm2newstr(data,NULL);
	    strncpy(TheMap.Description,str,sizeof(TheMap.Description));
	    TheMap.Info->Description=strdup(str);
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
		    LoadTileset();
		   
		} else if( gh_eq_p(value,gh_symbol2scm("size")) ) {
		    value=gh_car(data);
		    data=gh_cdr(data);

		    TheMap.Width=gh_scm2int(gh_car(value));
		    TheMap.Height=gh_scm2int(gh_cadr(value));

		    free(TheMap.Fields);
		    TheMap.Fields=calloc(TheMap.Width*TheMap.Height,
			    sizeof(*TheMap.Fields));
		    TheMap.Visible[0]=calloc(TheMap.Width*TheMap.Height/8,1);
		    InitUnitCache();
		    // FIXME: this should be CreateMap or InitMap?

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
#ifdef UNITS_ON_MAP
			TheMap.Fields[i].Building = 0xffff;
			TheMap.Fields[i].AirUnit = 0xffff;
			TheMap.Fields[i].LandUnit = 0xffff;
			TheMap.Fields[i].SeaUnit = 0xffff;
#endif /* UNITS_ON_MAP */
			while( !gh_null_p(field) ) {
			    if( gh_exact_p(gh_car(field)) ) {
				TheMap.Fields[i].Value=
					gh_scm2int(gh_car(field));
			    } else if( gh_eq_p(gh_car(field),
					gh_symbol2scm("explored")) ) {
				field=gh_cdr(field);
				TheMap.Fields[i].Visible[gh_scm2int(gh_car(field))] = 1;
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
	PrintFunction();
	fprintf(stdout,"Only supported within config file\n");
    }
    FlagRevealMap=1;

    return SCM_UNSPECIFIED;
}

/**
**	Set the default map.
**
**	@param map	Path to the default map.
**
**	@return		The old default map.
*/
local SCM CclSetDefaultMap(SCM map)
{
    SCM old;
    char* str;

    old=NIL;
    if( !gh_null_p(map) ) {
	old=gh_str02scm(DefaultMap);
	str=gh_scm2newstr(map,NULL);
	strcpy(DefaultMap,str);
	free(str);
    }
    return old;
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

/**
**	Enable display of terrain in minimap.
**
**	@param flag	#t = show minimap with terrain, #f = show no terrain.
**
**	@return		The old state of the minimap with terrain.
*/
local SCM CclSetMinimapTerrain(SCM flag)
{
    int old;

    old=MinimapWithTerrain;
    MinimapWithTerrain=gh_scm2bool(flag);

    return gh_bool2scm(old);
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
**	Alpha style fog of war.
*/
local SCM CclAlphaFogOfWar(void)
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
local SCM CclSetFogOfWarContrast(SCM contrast)
{
    int i;
    int o;

    i=gh_scm2int(contrast);
    if( i<0 || i>400 ) {
	PrintFunction();
	fprintf(stdout,"Contrast should be 0-400\n");
	i=100;
    }
    o=FogOfWarContrast;
    FogOfWarContrast=i;

    if( !CclInConfigFile ) {
	InitMapFogOfWar();
    }

    return gh_int2scm(o);
}

/**
**	Gray style fog of war brightness.
*/
local SCM CclSetFogOfWarBrightness(SCM brightness)
{
    int i;
    int o;

    i=gh_scm2int(brightness);
    if( i<-100 || i>100 ) {
	PrintFunction();
	fprintf(stdout,"Brightness should be -100-100\n");
	i=0;
    }
    o=FogOfWarBrightness;
    FogOfWarBrightness=i;

    if( !CclInConfigFile ) {
	InitMapFogOfWar();
    }

    return gh_int2scm(o);
}

/**
**	Gray style fog of war saturation.
*/
local SCM CclSetFogOfWarSaturation(SCM saturation)
{
    int i;
    int o;

    i=gh_scm2int(saturation);
    if( i<-100 || i>200 ) {
	PrintFunction();
	fprintf(stdout,"Saturation should be -100-200\n");
	i=0;
    }
    o=FogOfWarSaturation;
    FogOfWarSaturation=i;

    if( !CclInConfigFile ) {
	InitMapFogOfWar();
    }

    return gh_int2scm(o);
}

/**
**	Set forest regeneration speed.
**
**	@param speed	New regeneration speed (0 disabled)
**
**	@return		Old speed
*/
local SCM CclSetForestRegeneration(SCM speed)
{
    int i;
    int o;

    i=gh_scm2int(speed);
    if( i<0 || i>255 ) {
	PrintFunction();
	fprintf(stdout,"Regneration speed should be 0-255\n");
	i=0;
    }
    o=ForestRegeneration;
    ForestRegeneration=i;

    return gh_int2scm(o);
}

/**
**	Set gold-mine depleted rate.
**
**	@param rate	New depleted rate (0 disabled)
**
**	@return		Old rate
*/
local SCM CclSetGoldmineDepleted(SCM rate)
{
    int i;
    int o;

    i=gh_scm2int(rate);
    if( i<0 || i>100 ) {
	PrintFunction();
	fprintf(stdout,"Deplated rate should be 0-100\n");
	i=0;
    }
    o=OptionUseDepletedMines;
    OptionUseDepletedMines=i;

    return gh_int2scm(o);
}

/**
**	Set burning buildings percent and rate.
**
**	@param percent	    Max percent needed to burn buildings
**	@param rate	    HP per second to damage buildings
*/
local SCM CclSetBurnBuildings(SCM percent,SCM rate)
{
    int p;
    int r;

    p=gh_scm2int(percent);
    r=gh_scm2int(rate);
    if( p<0 || p>100 ) {
	PrintFunction();
	fprintf(stdout,"Burn percent should be 0-100\n");
	p=0;
    }
    if( r<=0 ) {
	PrintFunction();
	fprintf(stderr,"Burn rate should be greater than 0\n");
	p=0;
    }
    BurnBuildingPercent=p;
    BurnBuildingDamageRate=r;

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for map.
*/
global void MapCclRegister(void)
{
    gh_new_procedureN("freecraft-map",CclFreeCraftMap);
    gh_new_procedure0_0("reveal-map",CclRevealMap);

    gh_new_procedure1_0("set-default-map!",CclSetDefaultMap);
    gh_new_procedure1_0("set-fog-of-war!",CclSetFogOfWar);
    gh_new_procedure1_0("set-minimap-terrain!",CclSetMinimapTerrain);

    gh_new_procedure0_0("original-fog-of-war",CclOriginalFogOfWar);
    gh_new_procedure0_0("alpha-fog-of-war",CclAlphaFogOfWar);

    gh_new_procedure1_0("set-fog-of-war-contrast!",CclSetFogOfWarContrast);
    gh_new_procedure1_0("set-fog-of-war-brightness!",CclSetFogOfWarBrightness);
    gh_new_procedure1_0("set-fog-of-war-saturation!",CclSetFogOfWarSaturation);

    gh_new_procedure1_0("set-forest-regeneration!",CclSetForestRegeneration);
    gh_new_procedure1_0("set-goldmine-depleted!",CclSetGoldmineDepleted);

    gh_new_procedure2_0("set-burn-buildings!",CclSetBurnBuildings);
}

//@}
