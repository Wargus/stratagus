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
/**@name trigger.c	-	The trigger handling. */
//
//	(c) Copyright 2002-2003 by Lutz Sammer and Jimmy Salmon
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
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "stratagus.h"

#include "ccl.h"
#include "unittype.h"
#include "player.h"
#include "trigger.h"
#include "campaign.h"
#include "interface.h"
#include "siodp.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

    /// Get unit-type.
extern UnitType* CclGetUnitType(SCM ptr);

#define MAX_SWITCH	256		/// Maximum number of switches

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local SCM Trigger;			/// Current trigger
global Timer GameTimer;			/// The game timer
local unsigned long WaitFrame;		/// Frame to wait for
local SCM WaitScript;			/// Script to run after wait is over
local SCM WaitTrigger;			/// Old Trigger value during wait
local unsigned char Switch[MAX_SWITCH];	/// Switches

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
global int TriggerGetPlayer(SCM player)
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
global const UnitType* TriggerGetUnitType(SCM unit)
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

local int CompareEq(int a,int b)
{
    return a==b;
}
local int CompareNEq(int a,int b)
{
    return a!=b;
}
local int CompareGrEq(int a,int b)
{
    return a>=b;
}
local int CompareGr(int a,int b)
{
    return a>b;
}
local int CompareLeEq(int a,int b)
{
    return a<=b;
}
local int CompareLe(int a,int b)
{
    return a<b;
}

typedef int (*CompareFunction)(int,int);

/**
**	Returns a function pointer to the comparison function
**
**	@param op	The operation
**
**	@return		Function pointer to the compare function
*/
local CompareFunction GetCompareFunction(const char* op)
{
    if( op[0]=='=' ) {
	if( (op[1]=='=' && op[2]=='\0') || (op[1]=='\0') ) {
	    return &CompareEq;
	}
    }
    else if( op[0]=='>' ) {
	if( op[1]=='=' && op[2]=='\0' ) {
	    return &CompareGrEq;
	}
	else if( op[1]=='\0' ) {
	    return &CompareGr;
	}
    }
    else if( op[0]=='<' ) {
	if( op[1]=='=' && op[2]=='\0' ) {
	    return &CompareLeEq;
	}
	else if( op[1]=='\0' ) {
	    return &CompareLe;
	}
    }
    else if( op[0]=='!' && op[1]=='=' && op[2]=='\0' ) {
	return &CompareNEq;
    }
    return NULL;
}

/**
**	Player has the quantity of unit-type.
*/
local SCM CclIfUnit(SCM player,SCM operation,SCM quantity,SCM unit)
{
    int plynr;
    int q;
    int pn;
    const UnitType* unittype;
    const char* op;
    CompareFunction compare;

    plynr=TriggerGetPlayer(player);
    op=get_c_string(operation);
    q=gh_scm2int(quantity);
    unittype=TriggerGetUnitType(unit);

    compare=GetCompareFunction(op);
    if( !compare ) {
	errl("Illegal comparison operation in if-unit",operation);
    }

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
		if( compare(Players[plynr].UnitTypesCount[j],q) ) {
		    return SCM_BOOL_T;
		}
	    }
	}
    } else if( unittype==ALL_UNITS ) {
	for( ; plynr<pn; ++plynr ) {
	    if( compare(Players[plynr].TotalNumUnits,q) ) {
		return SCM_BOOL_T;
	    }
	}
    } else if( unittype==ALL_FOODUNITS ) {
	for( ; plynr<pn; ++plynr ) {
	    if( compare(Players[plynr].NumFoodUnits,q) ) {
		return SCM_BOOL_T;
	    }
	}
    } else if( unittype==ALL_BUILDINGS ) {
	for( ; plynr<pn; ++plynr ) {
	    if( compare(Players[plynr].NumBuildings,q) ) {
		return SCM_BOOL_T;
	    }
	}
    } else {
	for( ; plynr<pn; ++plynr ) {
	    DebugLevel3Fn("Player%d, %d == %s\n" _C_ plynr _C_ q _C_ unittype->Ident);
	    if( compare(Players[plynr].UnitTypesCount[unittype->Type],q) ) {
		return SCM_BOOL_T;
	    }
	}
    }

    return SCM_BOOL_F;
}

/**
**	Player has the quantity of unit-type at a location.
**
**	(if-unit-at <player> <op> <quantity> <unit> <location> <location>)
*/
local SCM CclIfUnitAt(SCM list)
{
    int plynr;
    int q;
    int x1;
    int y1;
    int x2;
    int y2;
    const UnitType* unittype;
    CompareFunction compare;
    Unit* table[UnitMax];
    Unit* unit;
    int an;
    int j;
    int s;

    plynr=TriggerGetPlayer(gh_car(list));
    list=gh_cdr(list);
    compare=GetCompareFunction(get_c_string(gh_car(list)));
    if( !compare ) {
	errl("Illegal comparison operator in if-unit-at",gh_car(list));
    }
    list=gh_cdr(list);
    q=gh_scm2int(gh_car(list));
    list=gh_cdr(list);
    unittype=TriggerGetUnitType(gh_car(list));
    list=gh_cdr(list);
    x1=gh_scm2int(gh_car(gh_car(list)));
    y1=gh_scm2int(gh_car(gh_cdr(gh_car(list))));
    list=gh_cdr(list);
    x2=gh_scm2int(gh_car(gh_car(list)));
    y2=gh_scm2int(gh_car(gh_cdr(gh_car(list))));
    list=gh_cdr(list);

    //
    //	Get all unit types in location.
    //
#ifdef UNIT_ON_MAP
    // FIXME: could be done faster?
#endif
    // FIXME: I hope SelectUnits checks bounds?
    // FIXME: Yes, but caller should check.
    // NOTE: +1 right,bottom isn't inclusive :(
    an=SelectUnits(x1,y1,x2+1,y2+1,table);
    //
    //	Count the requested units
    //
    for( j=s=0; j<an; ++j ) {
	unit=table[j];
	//
	//	Check unit type
	//
	// FIXME: ALL_UNITS
	if( unittype==ANY_UNIT
		|| (unittype==ALL_FOODUNITS && !unit->Type->Building)
		|| (unittype==ALL_BUILDINGS && unit->Type->Building)
		|| (unittype==unit->Type) ) {
	    //
	    //	Check the player
	    //
	    if( plynr==-1 || plynr==unit->Player->Player ) {
		++s;
	    }
	}
    }
    if( compare(s,q) ) {
	return SCM_BOOL_T;
    }

    return SCM_BOOL_F;
}

/**
**	Player has the quantity of unit-type near to unit-type.
*/
local SCM CclIfNearUnit(SCM player,SCM operation,SCM quantity,SCM unit,
                        SCM nearunit)
{
    int plynr;
    int q;
    int n;
    int i;
    const UnitType* unittype;
    const UnitType* ut2;
    const char* op;
    Unit* table[UnitMax];
    CompareFunction compare;

    plynr=TriggerGetPlayer(player);
    op=get_c_string(operation);
    q=gh_scm2int(quantity);
    unittype=TriggerGetUnitType(unit);
    ut2=CclGetUnitType(nearunit);

    compare=GetCompareFunction(op);
    if( !compare ) {
	errl("Illegal comparison operation in if-near-unit",operation);
    }

    //
    //	Get all unit types 'near'.
    //
    n=FindUnitsByType(ut2,table);
    DebugLevel3Fn("%s: %d\n" _C_ ut2->Ident _C_ n);
    for( i=0; i<n; ++i ) {
	Unit* unit;
	Unit* around[UnitMax];
	int an;
	int j;
	int s;

	unit=table[i];

#ifdef UNIT_ON_MAP
	// FIXME: could be done faster?
#endif
	// FIXME: I hope SelectUnits checks bounds?
	// FIXME: Yes, but caller should check.
	// NOTE: +1 right,bottom isn't inclusive :(
	if( unit->Type->UnitType==UnitTypeLand ) {
	    an=SelectUnits(
		unit->X-1,unit->Y-1,
		unit->X+unit->Type->TileWidth+1,
		unit->Y+unit->Type->TileHeight+1,around);
	} else {
	    an=SelectUnits(
		unit->X-2,unit->Y-2,
		unit->X+unit->Type->TileWidth+2,
		unit->Y+unit->Type->TileHeight+2,around);
	}
	DebugLevel3Fn("Units around %d: %d\n" _C_ UnitNumber(unit) _C_ an);
	//
	//	Count the requested units
	//
	for( j=s=0; j<an; ++j ) {
	    unit=around[j];
	    //
	    //	Check unit type
	    //
	    // FIXME: ALL_UNITS
	    if( unittype==ANY_UNIT
		    || (unittype==ALL_FOODUNITS && !unit->Type->Building)
		    || (unittype==ALL_BUILDINGS && unit->Type->Building)
		    || (unittype==unit->Type) ) {
		//
		//	Check the player
		//
		if( plynr==-1 || plynr==unit->Player->Player ) {
		    ++s;
		}
	    }
	}
	// Check if we counted the unit near itself
	if( unittype==ANY_UNIT
		|| (unittype==ALL_FOODUNITS && ut2->Building)
		|| (unittype==ALL_BUILDINGS && ut2->Building) ) {
	    --s;
	}
	if( compare(s,q) ) {
	    return SCM_BOOL_T;
	}
    }

    return SCM_BOOL_F;
}

/**
**	Player has the quantity of rescued unit-type near to unit-type.
*/
local SCM CclIfRescuedNearUnit(SCM player,SCM operation,SCM quantity,SCM unit,
                               SCM nearunit)
{
    int plynr;
    int q;
    int n;
    int i;
    const UnitType* unittype;
    const UnitType* ut2;
    const char* op;
    Unit* table[UnitMax];
    CompareFunction compare;

    plynr=TriggerGetPlayer(player);
    op=get_c_string(operation);
    q=gh_scm2int(quantity);
    unittype=TriggerGetUnitType(unit);
    ut2=CclGetUnitType(nearunit);

    compare=GetCompareFunction(op);
    if( !compare ) {
	errl("Illegal comparison operation in if-rescued-near-unit",operation);
    }

    //
    //	Get all unit types 'near'.
    //
    n=FindUnitsByType(ut2,table);
    DebugLevel3Fn("%s: %d\n" _C_ ut2->Ident _C_ n);
    for( i=0; i<n; ++i ) {
	Unit* unit;
	Unit* around[UnitMax];
	int an;
	int j;
	int s;

	unit=table[i];

#ifdef UNIT_ON_MAP
	// FIXME: could be done faster?
#endif
	// FIXME: I hope SelectUnits checks bounds?
	// FIXME: Yes, but caller should check.
	// NOTE: +1 right,bottom isn't inclusive :(
	if( unit->Type->UnitType==UnitTypeLand ) {
	    an=SelectUnits(
		unit->X-1,unit->Y-1,
		unit->X+unit->Type->TileWidth+1,
		unit->Y+unit->Type->TileHeight+1,around);
	} else {
	    an=SelectUnits(
		unit->X-2,unit->Y-2,
		unit->X+unit->Type->TileWidth+2,
		unit->Y+unit->Type->TileHeight+2,around);
	}
	DebugLevel3Fn("Units around %d: %d\n" _C_ UnitNumber(unit) _C_ an);
	//
	//	Count the requested units
	//
	for( j=s=0; j<an; ++j ) {
	    unit=around[j];
	    if( unit->Rescued ) {	// only rescued units
		//
		//	Check unit type
		//
		// FIXME: ALL_UNITS
		if( unittype==ANY_UNIT
			|| (unittype==ALL_FOODUNITS && !unit->Type->Building)
			|| (unittype==ALL_BUILDINGS && unit->Type->Building)
			|| (unittype==unit->Type) ) {
		    //
		    //	Check the player
		    //
		    if( plynr==-1 || plynr==unit->Player->Player ) {
			++s;
		    }
		}
	    }
	}
	// Check if we counted the unit near itself
	if( unittype==ANY_UNIT
		|| (unittype==ALL_FOODUNITS && ut2->Building)
		|| (unittype==ALL_BUILDINGS && ut2->Building) ) {
	    --s;
	}
	if( compare(s,q) ) {
	    return SCM_BOOL_T;
	}
    }

    return SCM_BOOL_F;
}

/**
**	Player has n opponents left.
*/
local SCM CclIfOpponents(SCM player,SCM operation,SCM quantity)
{
    int plynr;
    int q;
    int pn;
    int n;
    const char* op;
    CompareFunction compare;

    plynr=TriggerGetPlayer(player);
    op=get_c_string(operation);
    q=gh_scm2int(quantity);

    compare=GetCompareFunction(op);
    if( !compare ) {
	errl("Illegal comparison operation in if-opponents",operation);
    }

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
	DebugLevel3Fn("Opponents of %d = %d\n" _C_ plynr _C_ n);
	if( compare(n,q) ) {
	    return SCM_BOOL_T;
	}
    }

    return SCM_BOOL_F;
}

/**
**	Player has the quantity of resource.
*/
local SCM CclIfResource(SCM player,SCM operation,SCM quantity,SCM resource)
{
    int plynr;
    int q;
    int pn;
    const char* res;
    const char* op;
    CompareFunction compare;
    int i;

    plynr=TriggerGetPlayer(player);
    op=get_c_string(operation);
    q=gh_scm2int(quantity);
    res=get_c_string(resource);

    compare=GetCompareFunction(op);
    if( !compare ) {
	errl("Illegal comparison operation in if-resource",operation);
    }

    if( plynr==-1 ) {
	plynr=0;
	pn=PlayerMax;
    } else {
	pn=plynr+1;
    }

    for( i=0; i<MaxCosts; ++i ) {
	if( !strcmp(res, DefaultResourceNames[i]) ) {
	    for( ; plynr<pn; ++plynr ) {
		if( compare(Players[plynr].Resources[i],q) ) {
		    return SCM_BOOL_T;
		}
	    }
	    return SCM_BOOL_F;
	}
    }
    if( !strcmp(res, "all") ) {
	int j;
	int sum;

	sum=0;
	for( ; plynr<pn; ++plynr ) {
	    for( j=1; j<MaxCosts; ++j ) {
		sum+=Players[plynr].Resources[j];
	    }
	}
	if( compare(sum,q) ) {
	    return SCM_BOOL_T;
	}
    } else if( !strcmp(res, "any") ) {
	int j;

	for( ; plynr<pn; ++plynr ) {
	    for( j=1; j<MaxCosts; ++j ) {
		if( compare(Players[plynr].Resources[j],q) ) {
		    return SCM_BOOL_T;
		}
	    }
	}
    }

    return SCM_BOOL_F;
}

/**
**	Player has quantity kills
*/
local SCM CclIfKills(SCM player,SCM operation,SCM quantity)
{
    int plynr;
    int q;
    int pn;
    int n;
    const char* op;
    CompareFunction compare;

    plynr=TriggerGetPlayer(player);
    op=get_c_string(operation);
    q=gh_scm2int(quantity);

    compare=GetCompareFunction(op);
    if( !compare ) {
	errl("Illegal comparison operation in if-kills",operation);
    }

    if( plynr==-1 ) {
	plynr=0;
	pn=PlayerMax;
    } else {
	pn=plynr+1;
    }

    for( n=0; plynr<pn; ++plynr ) {
	if( compare(Players[plynr].TotalKills,q) ) {
	    return SCM_BOOL_T;
	}
    }

    return SCM_BOOL_F;
}

/**
**	Player has a certain score
*/
local SCM CclIfScore(SCM player,SCM operation,SCM quantity)
{
    int plynr;
    int q;
    int pn;
    int n;
    const char* op;
    CompareFunction compare;

    plynr=TriggerGetPlayer(player);
    op=get_c_string(operation);
    q=gh_scm2int(quantity);

    compare=GetCompareFunction(op);
    if( !compare ) {
	errl("Illegal comparison operation in if-score",operation);
    }

    if( plynr==-1 ) {
	plynr=0;
	pn=PlayerMax;
    } else {
	pn=plynr+1;
    }

    for( n=0; plynr<pn; ++plynr ) {
	if( compare(Players[plynr].Score,q) ) {
	    return SCM_BOOL_T;
	}
    }

    return SCM_BOOL_F;
}

/**
**	Number of game cycles elapsed
*/
local SCM CclIfElapsed(SCM operation,SCM quantity)
{
    int q;
    const char* op;
    CompareFunction compare;

    op=get_c_string(operation);
    q=gh_scm2int(quantity);

    compare=GetCompareFunction(op);
    if( !compare ) {
	errl("Illegal comparison operation in if-elapsed",operation);
    }

    if( compare(GameCycle,q) ) {
	return SCM_BOOL_T;
    }

    return SCM_BOOL_F;
}

/**
**	Check the timer value
*/
local SCM CclIfTimer(SCM operation,SCM quantity)
{
    int q;
    const char* op;
    CompareFunction compare;

    if( !GameTimer.Init ) {
	return SCM_BOOL_F;
    }

    op=get_c_string(operation);
    q=gh_scm2int(quantity);

    compare=GetCompareFunction(op);
    if( !compare ) {
	errl("Illegal comparison operation in if-timer",operation);
    }

    if( compare(GameTimer.Cycles,q) ) {
	return SCM_BOOL_T;
    }

    return SCM_BOOL_F;
}

/**
**	Check the switch value
*/
local SCM CclIfSwitch(SCM number,SCM set)
{
    int i;
    unsigned char s;

    i=gh_scm2int(number);
    if( i<0 || i>=MAX_SWITCH ) {
	errl("Invalid switch number",number);
    }

    if( gh_boolean_p(set) ) {
	s=gh_scm2bool(set);
    } else {
	s=gh_scm2int(set);
	if( s ) {
	    s=1;
	}
    }

    if( Switch[i]==s ) {
	return SCM_BOOL_T;
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
**	Action set timer
*/
local SCM CclActionSetTimer(SCM cycles, SCM increasing)
{
    GameTimer.Cycles=gh_scm2int(cycles);
    GameTimer.Increasing=gh_scm2int(increasing);
    GameTimer.Init=1;
    GameTimer.LastUpdate=GameCycle;

    return SCM_UNSPECIFIED;
}

/**
**	Action start timer
*/
local SCM CclActionStartTimer(void)
{
    GameTimer.Running=1;
    GameTimer.Init=1;
    return SCM_UNSPECIFIED;
}

/**
**	Action stop timer
*/
local SCM CclActionStopTimer(void)
{
    GameTimer.Running=0;
    return SCM_UNSPECIFIED;
}

/**
**	Action wait
*/
local SCM CclActionWait(SCM ms)
{
    WaitFrame=FrameCounter+
	(FRAMES_PER_SECOND*VideoSyncSpeed/100*gh_scm2int(ms)+999)/1000;
    return SCM_UNSPECIFIED;
}

/**
**	Action stop timer
*/
local SCM CclActionSetSwitch(SCM number,SCM set)
{
    int i;
    unsigned char s;

    i=gh_scm2int(number);
    if( i<0 || i>=MAX_SWITCH ) {
	errl("Invalid switch number",number);
    }

    if( gh_boolean_p(set) ) {
	s=gh_scm2bool(set);
    } else {
	s=gh_scm2int(set);
	if( s ) {
	    s=1;
	}
    }

    Switch[i]=s;
    return set;
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
    if( gh_null_p(symbol_value(var,NIL)) ) {
	setvar(var, cons(cons(condition,action),NIL), NIL);
    } else {
	var=symbol_value(var,NIL);
	while( !gh_null_p(gh_cdr(var)) ) {
	    var=gh_cdr(var);
	}
	setcdr(var,cons(cons(condition,action),NIL));
    }

    return SCM_UNSPECIFIED;
}

/**
**	Set the current trigger number
**
**	@param number	    Trigger number
*/
local SCM CclSetTriggerNumber(SCM number)
{
    int num;
    int i;

    num=gh_scm2int(number);
    if( num==-1 ) {
	Trigger=NULL;
    } else {
	Trigger=symbol_value(gh_symbol2scm("*triggers*"),NIL);
	if( gh_null_p(Trigger) ) {
	    DebugLevel0Fn("Invalid trigger number: %d out of -1\n" _C_ num);
	} else {
	    for( i=0; i<num; ++i ) {
		if( gh_null_p(Trigger) ) {
		    DebugLevel0Fn("Invalid trigger number: %d out of %d\n" _C_
			num _C_ i-1);
		    break;
		}
		Trigger=gh_cdr(Trigger);
	    }
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Execute a trigger action
**
**	@param script	Script to execute
**
**	@return		1 if the trigger should be removed
*/
local int TriggerExecuteAction(SCM script)
{
    SCM value;

    value=NULL;

    while( !gh_null_p(script) ) {
	value=gh_eval(gh_car(script),NIL);
	script=gh_cdr(script);
	if( WaitFrame>FrameCounter ) {
	    WaitScript=script;
	    return 0;
	}
    }

    // If action returns false remove it
    if( gh_null_p(value) ) {
	return 1;
    }
    return 0;
}

/**
**	Remove a trigger
**
**	@param trig	Current trigger
*/
local void TriggerRemoveTrigger(SCM trig)
{
    if( !gh_null_p(Trigger) ) {
	setcar(trig,gh_car(Trigger));
	setcdr(trig,gh_cdr(Trigger));
    } else {
	setcar(trig,NIL);
	setcdr(trig,NIL);
    }
    Trigger=trig;
}

/**
**	Check trigger each game cycle.
*/
global void TriggersEachCycle(void)
{
    SCM pair;
    SCM trig;
    SCM value;
    SCM script;

    if( !Trigger ) {
	Trigger=symbol_value(gh_symbol2scm("*triggers*"),NIL);
    }
    trig=Trigger;

    if( WaitFrame>FrameCounter ) {
	return;
    }
    if( WaitFrame && WaitFrame<=FrameCounter ) {
	WaitFrame=0;
	if( TriggerExecuteAction(WaitScript) ) {
	    TriggerRemoveTrigger(WaitTrigger);
	}
	return;
    }

    if( GamePaused ) {
	return;
    }

    if( !gh_null_p(trig) ) {		// Next trigger
	pair=gh_car(trig);
	Trigger=gh_cdr(trig);
	WaitTrigger=trig;
	// Pair is condition action
	if( !gh_null_p(pair) ) {
	    script=gh_car(pair);
	    value=NULL;
	    while( !gh_null_p(script) ) {
		value=gh_eval(gh_car(script),NIL);
		script=gh_cdr(script);
	    }
	    // If condition is true execute action
	    if( !gh_null_p(value) ) {
		if( TriggerExecuteAction(gh_cdr(pair)) ) {
		    TriggerRemoveTrigger(trig);
		}
	    }
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
    gh_new_procedure1_0("set-trigger-number!",CclSetTriggerNumber);
    // Conditions
    gh_new_procedure4_0("if-unit",CclIfUnit);
    gh_new_procedureN("if-unit-at",CclIfUnitAt);
    gh_new_procedure5_0("if-near-unit",CclIfNearUnit);
    gh_new_procedure5_0("if-rescued-near-unit",CclIfRescuedNearUnit);
    gh_new_procedure3_0("if-opponents",CclIfOpponents);
    gh_new_procedure4_0("if-resource",CclIfResource);
    gh_new_procedure3_0("if-kills",CclIfKills);
    gh_new_procedure3_0("if-score",CclIfScore);
    gh_new_procedure2_0("if-elapsed",CclIfElapsed);
    gh_new_procedure2_0("if-timer",CclIfTimer);
    gh_new_procedure2_0("if-switch",CclIfSwitch);
    // Actions
    gh_new_procedure0_0("action-victory",CclActionVictory);
    gh_new_procedure0_0("action-defeat",CclActionDefeat);
    gh_new_procedure0_0("action-draw",CclActionDraw);
    gh_new_procedure2_0("action-set-timer",CclActionSetTimer);
    gh_new_procedure0_0("action-start-timer",CclActionStartTimer);
    gh_new_procedure0_0("action-stop-timer",CclActionStopTimer);
    gh_new_procedure1_0("action-wait",CclActionWait);
    gh_new_procedure2_0("action-set-switch",CclActionSetSwitch);

    gh_define("*triggers*",NIL);
}

/**
**	Print a trigger from a LISP object.
**	This is a modified version of lprin1g that prints
**	(lambda) instead of #<CLOSURE>
**
**	@param exp	Expression
**	@param f	File to print to
*/
local void PrintTrigger(LISP exp,FILE *f)
{
    LISP tmp;
    long n;
//    struct user_type_hooks *p;
    extern char *subr_kind_str(long);

    STACK_CHECK(&exp);
    INTERRUPT_CHECK();
    switch TYPE(exp) {
    case tc_nil:
	fprintf(f,"()");
	break;
    case tc_cons:
	fprintf(f,"(");
	PrintTrigger(car(exp),f);
	for(tmp=cdr(exp);CONSP(tmp);tmp=cdr(tmp)) {
	    fprintf(f," ");
	    PrintTrigger(car(tmp),f);
	}
	if NNULLP(tmp) {
	    fprintf(f," . ");
	    PrintTrigger(tmp,f);
	}
	fprintf(f,")");
	break;
    case tc_flonum:
	n = (long) FLONM(exp);
	if (((double) n) == FLONM(exp)) {
	    sprintf(tkbuffer,"%ld",n);
	} else {
	    sprintf(tkbuffer,"%g",FLONM(exp));
	}
	fprintf(f,tkbuffer);
	break;
    case tc_symbol:
	fprintf(f,PNAME(exp));
	break;
    case tc_subr_0:
    case tc_subr_1:
    case tc_subr_2:
    case tc_subr_2n:
    case tc_subr_3:
    case tc_subr_4:
    case tc_subr_5:
    case tc_lsubr:
    case tc_fsubr:
    case tc_msubr:
	sprintf(tkbuffer,"#<%s ",subr_kind_str(TYPE(exp)));
	fprintf(f,tkbuffer);
	fprintf(f,(*exp).storage_as.subr.name);
	fprintf(f,">");
	break;
    case tc_string:
	fprintf(f,"\"%s\"",(*exp).storage_as.string.data);
	break;
    case tc_closure:
	fprintf(f,"(lambda ");
	if CONSP((*exp).storage_as.closure.code) {
	    PrintTrigger(car((*exp).storage_as.closure.code),f);
	    fprintf(f," ");
	    PrintTrigger(cdr((*exp).storage_as.closure.code),f);
	} else
	    PrintTrigger((*exp).storage_as.closure.code,f);
	fprintf(f,")");
	break;
    default:
	break;
#if 0
	p = get_user_type_hooks(TYPE(exp));
	if (p->prin1)
	    (*p->prin1)(exp,f);
	else {
	    sprintf(tkbuffer,"#<UNKNOWN %d %p>",TYPE(exp),exp);
	    fprintf(f,tkbuffer);
	}
#endif
    }
}

/**
**	Save the trigger module.
**
**	@param file	Open file to print to
*/
global void SaveTriggers(FILE* file)
{
    SCM list;
    int i;
    int trigger;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: trigger $Id$\n\n");

    i=0;
    trigger=-1;
    list=symbol_value(gh_symbol2scm("*triggers*"),NIL);
    while( !gh_null_p(list) ) {
	if( gh_eq_p(Trigger,list) ) {
	    trigger=i;
	}
	fprintf(file,"(add-trigger '");
	PrintTrigger(gh_car(gh_car(list)),file);
	fprintf(file," '");
	PrintTrigger(gh_cdr(gh_car(list)),file);
	fprintf(file,")\n");
	list=gh_cdr(list);
	++i;
    }
    fprintf(file,"(set-trigger-number! %d)\n",trigger);

    if( GameTimer.Init ) {
	fprintf(file,"(action-set-timer %ld %d)\n",
	    GameTimer.Cycles,GameTimer.Increasing);
	if( GameTimer.Running ) {
	    fprintf(file,"(action-start-timer)\n");
	}
    }
}

/**
**	Initialize the trigger module.
*/
global void InitTriggers(void)
{
    //
    //	Setup default triggers
    //
    WaitFrame=0;

    // FIXME: choose the triggers for game type

    if( gh_null_p(symbol_value(gh_symbol2scm("*triggers*"),NIL)) ) {
	DebugLevel0Fn("Default triggers\n");
	gh_apply(symbol_value(gh_symbol2scm("single-player-triggers"),NIL),NIL);
    }

    memset(Switch,0,sizeof(Switch));
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

    memset(&GameTimer,0,sizeof(GameTimer));
}

//@}
