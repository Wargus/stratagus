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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"

#if defined(USE_CCL) || defined(USE_CCL2)	// {

#include "ccl.h"
#include "map.h"
#include "minimap.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

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

    old=TheMap.NoFogOfWar;
    TheMap.NoFogOfWar=gh_scm2bool(flag);

    return gh_int2scm(old);
}

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
    gh_new_procedure0_0("reveal-map",CclRevealMap);

    gh_new_procedure1_0("set-fog-of-war!",CclSetFogOfWar);

    gh_new_procedure0_0("fog-of-war",CclFogOfWar);
    gh_new_procedure0_0("no-fog-of-war",CclNoFogOfWar);

    gh_new_procedure0_0("minimap-terrain",CclMinimapTerrain);
    gh_new_procedure0_0("no-minimap-terrain",CclNoMinimapTerrain);

    gh_new_procedure0_0("original-fog-of-war",CclOriginalFogOfWar);
    gh_new_procedure0_0("gray-fog-of-war",CclGrayFogOfWar);

    gh_new_procedure1_0("fog-of-war-contrast",CclFogOfWarContrast);
    gh_new_procedure1_0("fog-of-war-brightness",CclFogOfWarBrightness);
    gh_new_procedure1_0("fog-of-war-saturation",CclFogOfWarSaturation);

    gh_new_procedure1_0("forest-regeneration",CclForestRegeneration);
}

#endif	// } defined(USE_CCL) || defined(USE_CCL2)

//@}
