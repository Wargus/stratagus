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
/**@name ccl_player.c	-	The player ccl functions. */
//
//	(c) Copyright 2001 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"

#if defined(USE_CCL) // {

#include <stdlib.h>

#include "player.h"
#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Set player unit limit.
**
**	@param limit	Unit limit.
*/
local SCM CclSetAllPlayersFoodUnitLimit(SCM limit)
{
    int i;

    for( i=0; i<PlayerMax; ++i ) {
	Players[i].FoodUnitLimit=gh_scm2int(limit);
    }

    return limit;
}

/**
**	Set player unit limit.
**
**	@param limit	Unit limit.
*/
local SCM CclSetAllPlayersBuildingLimit(SCM limit)
{
    int i;

    for( i=0; i<PlayerMax; ++i ) {
	Players[i].BuildingLimit=gh_scm2int(limit);
    }

    return limit;
}

/**
**	Set player unit limit.
**
**	@param limit	Unit limit.
*/
local SCM CclSetAllPlayersTotalUnitLimit(SCM limit)
{
    int i;

    for( i=0; i<PlayerMax; ++i ) {
	Players[i].TotalUnitLimit=gh_scm2int(limit);
    }

    return limit;
}

// ----------------------------------------------------------------------------

/**
**	Register CCL features for players.
*/
global void PlayerCclRegister(void)
{
    gh_new_procedure1_0("set-all-players-food-unit-limit",
		CclSetAllPlayersFoodUnitLimit);
    gh_new_procedure1_0("set-all-players-building-limit",
		CclSetAllPlayersBuildingLimit);
    gh_new_procedure1_0("set-all-players-total-unit-limit",
		CclSetAllPlayersTotalUnitLimit);
}

#endif	// } USE_CCL

//@}
