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
/**@name ccl_pathfinder.c	-	pathfinder ccl functions. */
/*
**	(c) Copyright 2000 by Lutz Sammer and Fabrice Rossi
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clone.h"

#if defined(USE_CCL) || defined(USE_CCL2)	// {

#include "video.h"
#include "tileset.h"
#include "map.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "ccl.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
 **	Enable a*.
*/
local SCM CclAStar(void)
{
    AStarOn=1;

    return SCM_UNSPECIFIED;
}

/**
**	Disable a*.
*/
local SCM CclNoAStar(void)
{
    AStarOn=0;

    return SCM_UNSPECIFIED;
}

/**
**	Set a* parameter (cost of FIXED unit tile crossing).
*/
local SCM CclAStarSetFixedUCC(SCM cost)
{
    int i;

    i=gh_scm2int(cost);
    if( i<=0) {
	fprintf(stderr,__FUNCTION__": Fixed unit crossing cost must be strictly positive\n");
	i=MaxMapWidth*MaxMapHeight;
    }
    AStarFixedUnitCrossingCost=i;

    return SCM_UNSPECIFIED;
}

/**
**	Set a* parameter (cost of MOVING unit tile crossing).
*/
local SCM CclAStarSetMovingUCC(SCM cost)
{
    int i;

    i=gh_scm2int(cost);
    if( i<=0) {
	fprintf(stderr,__FUNCTION__": Moving unit crossing cost must be strictly positive\n");
	i=1;
    }
    AStarMovingUnitCrossingCost=i;

    return SCM_UNSPECIFIED;
}


/**
**	Register CCL features for pathfinder.
*/
global void PathfinderCclRegister(void)
{
    gh_new_procedure0_0("a-star",CclAStar);
    gh_new_procedure0_0("no-a-star",CclNoAStar);
    gh_new_procedure1_0("a-star-fixed-unit-cost",CclAStarSetFixedUCC);
    gh_new_procedure1_0("a-star-moving-unit-cost",CclAStarSetMovingUCC);
}



#endif	// } defined(USE_CCL) || defined(USE_CCL2)

//@}
