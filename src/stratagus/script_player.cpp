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
**	Parse the player configuration.
**
**	@param list	Tagged list of all informations.
*/
local SCM CclPlayer(SCM list)
{
    SCM value;
    SCM data;
    Player* player;
    //char* str;

    player=&Players[gh_scm2int(gh_car(list))];
    list=gh_cdr(list);

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("name")) ) {
	    player->Name=gh_scm2newstr(data=gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("type")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("race")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("ai")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("team")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("enemy")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("allied")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("start")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("resources")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("incomes")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("ai-enabled")) ) {
	    DebugLevel0Fn("FIXME:\n");
	} else if( gh_eq_p(value,gh_symbol2scm("ai-disabled")) ) {
	    DebugLevel0Fn("FIXME:\n");
	} else if( gh_eq_p(value,gh_symbol2scm("food-unit-limit")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("building-limit")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("total-unit-limit")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("score")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("timers")) ) {
	    DebugLevel0Fn("FIXME:\n");
	    list=gh_cdr(list);
	} else {
	   // FIXME: this leaves a half initialized player
	   errl("Unsupported tag",value);
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Set ThisPlayer.
**
**	@param plynr	This player number.
*/
local SCM CclThisPlayer(SCM plynr)
{
    ThisPlayer=&Players[gh_scm2int(plynr)];

    return plynr;
}

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
    gh_new_procedureN("player",CclPlayer);
    gh_new_procedure1_0("this-player",CclThisPlayer);

    gh_new_procedure1_0("set-all-players-food-unit-limit!",
		CclSetAllPlayersFoodUnitLimit);
    gh_new_procedure1_0("set-all-players-building-limit!",
		CclSetAllPlayersBuildingLimit);
    gh_new_procedure1_0("set-all-players-total-unit-limit!",
		CclSetAllPlayersTotalUnitLimit);
}

#endif	// } USE_CCL

//@}
