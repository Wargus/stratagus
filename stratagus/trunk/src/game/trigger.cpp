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
/**@name trigger.c	-	The trigger handling. */
//
//	(c) Copyright 2002 by Lutz Sammer
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
#include <string.h>
#include "freecraft.h"

#include "ccl.h"
#include "unittype.h"
#include "player.h"
#include "trigger.h"
#include "campaign.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

    /// Get unit-type.
extern UnitType* CclGetUnitType(SCM ptr);


#define ANY_UNIT	((const UnitType*)0)
#define ALL_UNITS	((const UnitType*)-1)
#define ALL_FOODUNITS	((const UnitType*)-2)
#define ALL_BUILDINGS	((const UnitType*)-3)

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local SCM Trigger;			/// Current trigger

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Get player number.
**
**	@param player	The player
**
**	@return		The player number, -1 matches any.
*/
local int TriggerGetPlayer(SCM player)
{
    int ret;

    if (gh_exact_p(player)) {
	ret = gh_scm2int(player);
	if (ret < 0 || ret > PlayerMax) {
	    errl("bad player", player);
	}
	return ret;
    }
    if (gh_eq_p(player, gh_symbol2scm("any"))) {
	return -1;
    } else if (gh_eq_p(player, gh_symbol2scm("this"))) {
	return ThisPlayer->Player;
    }
    errl("bad player", player);

    return 0;
}

/**
**	Get the unit-type.
**
**	@param unit	The unit type.
**
**	@return		The unit-type pointer.
*/
local const UnitType* TriggerGetUnitType(SCM unit)
{
    if (gh_eq_p(unit, gh_symbol2scm("any"))) {
	return ANY_UNIT;
    } else if (gh_eq_p(unit, gh_symbol2scm("all"))) {
	return ALL_UNITS;
    } else if (gh_eq_p(unit, gh_symbol2scm("units"))) {
	return ALL_FOODUNITS;
    } else if (gh_eq_p(unit, gh_symbol2scm("buildings"))) {
	return ALL_BUILDINGS;
    }

    return CclGetUnitType(unit);
}

// --------------------------------------------------------------------------
//	Conditions

/**
**	Player has the quantity of unit-type.
*/
local SCM CclIfUnit(SCM player,SCM quantity,SCM unit)
{
    int plynr;
    int q;
    int pn;
    const UnitType* unittype;

    plynr=TriggerGetPlayer(player);
    q=gh_scm2int(quantity);
    unittype=TriggerGetUnitType(unit);

    if( plynr==-1 ) {
	plynr=0;
	pn=PlayerMax;
    } else {
	pn=plynr+1;
    }

    if( unittype==ANY_UNIT ) {
	for( ; plynr<pn; ++plynr ) {
	    int j;

	    for( j=0; j<NumUnitTypes; ++j ) {
		if( Players[plynr].UnitTypesCount[j]==q ) {
		    return SCM_BOOL_T;
		}
	    }
	}
    } else if( unittype==ALL_UNITS ) {
	for( ; plynr<pn; ++plynr ) {
	    if( Players[plynr].TotalNumUnits==q ) {
		return SCM_BOOL_T;
	    }
	}
    } else if( unittype==ALL_FOODUNITS ) {
	for( ; plynr<pn; ++plynr ) {
	    if( Players[plynr].NumFoodUnits==q ) {
		return SCM_BOOL_T;
	    }
	}
    } else if( unittype==ALL_BUILDINGS ) {
	for( ; plynr<pn; ++plynr ) {
	    if( Players[plynr].NumBuildings==q ) {
		return SCM_BOOL_T;
	    }
	}
    } else {
	for( ; plynr<pn; ++plynr ) {
	    DebugLevel3Fn("Player%d, %d == %s\n",plynr,q,unittype->Ident);
	    if( Players[plynr].UnitTypesCount[unittype->Type]==q ) {
		return SCM_BOOL_T;
	    }
	}
    }

    return SCM_BOOL_F;
}

/**
**	Player has the quantity of unit-type near to unit-type.
*/
local SCM CclIfNearUnit(SCM player,SCM quantity,SCM unit,SCM near)
{
    int plynr;
    int q;
    const UnitType* unittype;
    const UnitType* ut2;

    plynr=TriggerGetPlayer(player);
    q=gh_scm2int(quantity);
    unittype=TriggerGetUnitType(unit);
    ut2=CclGetUnitType(unit);

    // ANY, ALL, BUILDINGS, FOODUNITS.

    // Player type

    DebugLevel0Fn("FIXME: not written\n");

    return SCM_BOOL_F;
}

/**
**	Player has n opponents left.
*/
local SCM CclIfOpponents(SCM player,SCM quantity)
{
    int plynr;
    int q;
    int pn;
    int n;

    plynr=TriggerGetPlayer(player);
    q=gh_scm2int(quantity);

    if( plynr==-1 ) {
	plynr=0;
	pn=PlayerMax;
    } else {
	pn=plynr+1;
    }

    //
    //	Check the player opponents
    //
    for( n=0; plynr<pn; ++plynr ) {
	int i;

	for( i=0; i<PlayerMax; ++i ) {
	    //
	    //	This player is our enemy and has units left.
	    //
	    if( (Players[i].Enemy&(1<<plynr)) && Players[i].TotalNumUnits ) {
		++n;
	    }
	}
	DebugLevel3Fn("Opponents of %d = %d\n",plynr,n);
	if( n==q ) {
	    return SCM_BOOL_T;
	}
    }

    return SCM_BOOL_F;
}

// --------------------------------------------------------------------------
//	Actions

/**
**	Action condition player wins.
*/
local SCM CclActionVictory(void)
{
    GameResult=GameVictory;
    GamePaused=1;
    GameRunning=0;
    return SCM_UNSPECIFIED;
}

/**
**	Action condition player lose.
*/
local SCM CclActionDefeat(void)
{
    GameResult=GameDefeat;
    GamePaused=1;
    GameRunning=0;
    return SCM_UNSPECIFIED;
}

/**
**	Action condition player draw.
*/
local SCM CclActionDraw(void)
{
    GameResult=GameDraw;
    GamePaused=1;
    GameRunning=0;
    return SCM_UNSPECIFIED;
}

/**
**	Add a trigger.
*/
local SCM CclAddTrigger(SCM condition,SCM action)
{
    SCM var;

    //
    //	Make a list of all triggers.
    //		A trigger is a pair of condition and action
    //
    var=gh_symbol2scm("*triggers*");
    setvar(var,cons(cons(condition,action),symbol_value(var,NIL)),NIL);

    return SCM_UNSPECIFIED;
}

/**
**	Check trigger each frame.
*/
global void TriggersEachFrame(void)
{
    SCM pair;
    SCM val;

    if( !Trigger ) {
	Trigger=symbol_value(gh_symbol2scm("*triggers*"),NIL);
    }

    if( !gh_null_p(Trigger) ) {		// Next trigger
	pair=gh_car(Trigger);
	Trigger=gh_cdr(Trigger);
	if( !gh_null_p(val=gh_apply(car(pair),NIL)) ) {
	    gh_apply(cdr(pair),NIL);
	}
    } else {
	Trigger=NULL;
    }
}

/**
**	Register CCL features for triggers.
*/
global void TriggerCclRegister(void)
{
    gh_new_procedure2_0("add-trigger",CclAddTrigger);
    // Conditions
    gh_new_procedure3_0("if-unit",CclIfUnit);
    gh_new_procedure4_0("if-near-unit",CclIfNearUnit);
    gh_new_procedure2_0("if-opponents",CclIfOpponents);
    // Actions
    gh_new_procedure0_0("action-victory",CclActionVictory);
    gh_new_procedure0_0("action-defeat",CclActionDefeat);
    gh_new_procedure0_0("action-draw",CclActionDraw);

    gh_define("*triggers*",NIL);
}

/**
**	Save the trigger module.
*/
global void SaveTriggers(FILE* file)
{
    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: trigger $Id$\n\n");
    fprintf(file,";;; FIXME: Save not written\n\n");
}

/**
**	Initialize the trigger module.
*/
global void InitTriggers(void)
{
    //
    //	Setup default triggers
    //
    if( gh_null_p(symbol_value(gh_symbol2scm("*triggers*"),NIL)) ) {
	DebugLevel0Fn("Default triggers\n");
	gh_apply(symbol_value(gh_symbol2scm("single-player-triggers"),NIL),NIL);
    }
}

/**
**	Clean up the trigger module.
*/
global void CleanTriggers(void)
{
    SCM var;

    DebugLevel0Fn("FIXME: Cleaning trigger not written\n");

    var=gh_symbol2scm("*triggers*");
    setvar(var,NIL,NIL);

    Trigger=NULL;
}

//@}
