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
/**@name ccl_unit.c	-	The unit ccl functions. */
//
//	(c) Copyright 2001-2002 by Lutz Sammer
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
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "ccl.h"
#include "spells.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local int NumOfUnitsToLoad;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Get unit-type.
extern UnitType* CclGetUnitType(SCM ptr);

/**
**	Set hit-point regeneration
**
**	@param flag	Flag enabling or disabling it.
**
**	@return		The old state of the hit-point regeneration.
*/
local SCM CclSetHitPointRegeneration(SCM flag)
{
    int old;

    old = HitPointRegeneration;
    HitPointRegeneration = gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Set training queue
**
**	@param flag	Flag enabling or disabling it.
**
**	@return		The old state of the training queue
*/
local SCM CclSetTrainingQueue(SCM flag)
{
    int old;

    old = EnableTrainingQueue;
    EnableTrainingQueue = gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Set capture buildings
**
**	@param flag	Flag enabling or disabling it.
**
**	@return		The old state of the flag
*/
local SCM CclSetBuildingCapture(SCM flag)
{
    int old;

    old = EnableBuildingCapture;
    EnableBuildingCapture = gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Set reveal attacker
**
**	@param flag	Flag enabling or disabling it.
**
**	@return		The old state of the flag
*/
local SCM CclSetRevealAttacker(SCM flag)
{
    int old;

    old = RevealAttacker;
    RevealAttacker = gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Get an unit pointer
**
**	@param value	Unit slot number.
**
**	@return		The unit pointer
*/
local Unit* CclGetUnit(SCM value)
{
    return UnitSlots[gh_scm2int(value)];
}

/**
**	Parse order
**
**	@param list	All options of the order.
**	@param order	OUT: resulting order.
*/
local void CclParseOrder(SCM list,Order* order)
{
    SCM value;
    SCM sublist;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	if( gh_eq_p(value,gh_symbol2scm("action-none")) ) {
	    order->Action=UnitActionNone;
	} else if( gh_eq_p(value,gh_symbol2scm("action-still")) ) {
	    order->Action=UnitActionStill;
	} else if( gh_eq_p(value,gh_symbol2scm("action-stand-ground")) ) {
	    order->Action=UnitActionStandGround;
	} else if( gh_eq_p(value,gh_symbol2scm("action-follow")) ) {
	    order->Action=UnitActionFollow;
	} else if( gh_eq_p(value,gh_symbol2scm("action-move")) ) {
	    order->Action=UnitActionMove;
	} else if( gh_eq_p(value,gh_symbol2scm("action-attack")) ) {
	    order->Action=UnitActionAttack;
	} else if( gh_eq_p(value,gh_symbol2scm("action-attack-ground")) ) {
	    order->Action=UnitActionAttackGround;
	} else if( gh_eq_p(value,gh_symbol2scm("action-die")) ) {
	    order->Action=UnitActionDie;
	} else if( gh_eq_p(value,gh_symbol2scm("action-spell-cast")) ) {
	    order->Action=UnitActionSpellCast;
	} else if( gh_eq_p(value,gh_symbol2scm("action-train")) ) {
	    order->Action=UnitActionTrain;
	} else if( gh_eq_p(value,gh_symbol2scm("action-upgrade-to")) ) {
	    order->Action=UnitActionUpgradeTo;
	} else if( gh_eq_p(value,gh_symbol2scm("action-research")) ) {
	    order->Action=UnitActionResearch;
	} else if( gh_eq_p(value,gh_symbol2scm("action-builded")) ) {
	    order->Action=UnitActionBuilded;
	} else if( gh_eq_p(value,gh_symbol2scm("action-board")) ) {
	    order->Action=UnitActionBoard;
	} else if( gh_eq_p(value,gh_symbol2scm("action-unload")) ) {
	    order->Action=UnitActionUnload;
	} else if( gh_eq_p(value,gh_symbol2scm("action-patrol")) ) {
	    order->Action=UnitActionPatrol;
	} else if( gh_eq_p(value,gh_symbol2scm("action-build")) ) {
	    order->Action=UnitActionBuild;
	} else if( gh_eq_p(value,gh_symbol2scm("action-repair")) ) {
	    order->Action=UnitActionRepair;
	} else if( gh_eq_p(value,gh_symbol2scm("action-harvest")) ) {
	    order->Action=UnitActionHarvest;
	} else if( gh_eq_p(value,gh_symbol2scm("action-mine-gold")) ) {
	    order->Action=UnitActionMineGold;
	} else if( gh_eq_p(value,gh_symbol2scm("action-mine-ore")) ) {
	    order->Action=UnitActionMineOre;
	} else if( gh_eq_p(value,gh_symbol2scm("action-mine-coal")) ) {
	    order->Action=UnitActionMineCoal;
	} else if( gh_eq_p(value,gh_symbol2scm("action-quarry-stone")) ) {
	    order->Action=UnitActionQuarryStone;
	} else if( gh_eq_p(value,gh_symbol2scm("action-return-goods")) ) {
	    order->Action=UnitActionReturnGoods;
	} else if( gh_eq_p(value,gh_symbol2scm("action-demolish")) ) {
	    order->Action=UnitActionDemolish;

	} else if( gh_eq_p(value,gh_symbol2scm("flags")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    order->Flags=gh_scm2int(value);

	} else if( gh_eq_p(value,gh_symbol2scm("range")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    order->RangeX=gh_scm2int(gh_car(sublist));
	    order->RangeY=gh_scm2int(gh_cadr(sublist));

	} else if( gh_eq_p(value,gh_symbol2scm("goal")) ) {
	    char* str;
	    int slot;

	    value=gh_car(list);
	    list=gh_cdr(list);
	    str=gh_scm2newstr(value,NULL);

	    slot=strtol(str+1,NULL,16);
	    order->Goal=UnitSlots[slot];
	    if( !UnitSlots[slot] ) {
		DebugLevel0Fn("FIXME: Forward reference not supported\n");
	    }
	    ++UnitSlots[slot]->Refs;
	    free(str);

	} else if( gh_eq_p(value,gh_symbol2scm("tile")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    order->X=gh_scm2int(gh_car(sublist));
	    order->Y=gh_scm2int(gh_cadr(sublist));

	} else if( gh_eq_p(value,gh_symbol2scm("type")) ) {
	    char* str;

	    value=gh_car(list);
	    list=gh_cdr(list);
	    str=gh_scm2newstr(value,NULL);
	    order->Type=UnitTypeByIdent(str);
	    free(str);

	} else if( gh_eq_p(value,gh_symbol2scm("patrol")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    order->Arg1=(void*)((gh_scm2int(gh_car(sublist))<<16)|
		    gh_scm2int(gh_cadr(sublist)));

	} else if( gh_eq_p(value,gh_symbol2scm("spell")) ) {
	    char* str;

	    value=gh_car(list);
	    list=gh_cdr(list);
	    str=gh_scm2newstr(value,NULL);
	    order->Arg1=SpellTypeByIdent(str);
	    free(str);

	} else if( gh_eq_p(value,gh_symbol2scm("upgrade")) ) {
	    char* str;

	    value=gh_car(list);
	    list=gh_cdr(list);
	    str=gh_scm2newstr(value,NULL);
	    order->Arg1=UpgradeByIdent(str);
	    free(str);

	} else if( gh_eq_p(value,gh_symbol2scm("mine")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    order->Arg1=(void*)((gh_scm2int(gh_car(sublist))<<16)|
		    gh_scm2int(gh_cadr(sublist)));

	} else if( gh_eq_p(value,gh_symbol2scm("arg1")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    order->Arg1 = (void * )gh_scm2int (value);

	} else {
	   // FIXME: this leaves a half initialized unit
	   errl("Unsupported tag",value);
	}
    }
}

/**
**	Parse orders.
**
**	@param unit	Unit pointer which should get the orders.
**	@param vector	All options of the order.
*/
local void CclParseOrders(Unit* unit,SCM vector)
{
    int i;
    int n;

    n=gh_vector_length(vector);
    DebugCheck( n!=MAX_ORDERS );
    for( i=0; i<n; ++i ) {
	CclParseOrder(gh_vector_ref(vector,gh_int2scm(i)),&unit->Orders[i]);
    }
}

/**
**	Parse builded
**
**	@param unit	Unit pointer which should be filled with the data.
**	@param list	All options of the builded data.
*/
local void CclParseBuilded(Unit* unit __attribute__((unused)),
	SCM list __attribute__((unused)))
{
    DebugLevel0Fn("FIXME: builded\n");
}

/**
**	Parse stored data for train order
**
**	@param unit	Unit pointer which should be filled with the data.
**	@param list	All options of the trained order
*/
local void CclParseTrain (Unit *unit, SCM list)
{
    SCM value, sublist;
    int i;

    while ( !gh_null_p (list) ) {
	value = gh_car (list);
	list = gh_cdr (list);
	if (gh_eq_p (value, gh_symbol2scm ("ticks")) ) {
	    value = gh_car (list);
	    list = gh_cdr (list);
	    unit->Data.Train.Ticks = gh_scm2int (value);
	} else if (gh_eq_p (value, gh_symbol2scm ("count")) ) {
	    value = gh_car (list);
	    list = gh_cdr (list);
	    unit->Data.Train.Count = gh_scm2int (value);
	} else if (gh_eq_p (value, gh_symbol2scm ("queue")) ) {
	    sublist=gh_car (list);
	    list=gh_cdr (list);
	    for (i=0; i<MAX_UNIT_TRAIN; ++i) {
		value = gh_vector_ref (sublist, gh_int2scm(i));
		if ( gh_eq_p (value, gh_symbol2scm ("unit-none")) ) {
		    unit->Data.Train.What[i] = NULL;
		} else {
		    char *ident = gh_scm2newstr (value, NULL);
		    unit->Data.Train.What[i] = UnitTypeByIdent (ident);
		    free (ident);
		}
	    }
	}
    }
}

/**
**	Parse unit
**
**	@param list	List describing unit
*/
local SCM CclUnit(SCM list)
{
    SCM value;
    SCM sublist;
    Unit* unit;
    UnitType* type;
    Player* player;
    int slot;
    int i;
    char* str;
    char* s;

    str=gh_scm2newstr(gh_car(list),NULL);
    list=gh_cdr(list);

    slot=strtol(str+1,NULL,16);
    free(str);
    unit=NULL;
    type=NULL;
    player=NULL;
    DebugLevel0Fn ("parsing unit #%d\n", slot);

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("type")) ) {
	    type=UnitTypeByIdent(str=gh_scm2newstr(gh_car(list),NULL));
	    free(str);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("name")) ) {
	    unit->Name=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("player")) ) {
	    player=&Players[gh_scm2int(gh_car(list))];
	    list=gh_cdr(list);

	    DebugCheck( !type );
	    //unit=MakeUnit(type,player);
	    unit = UnitSlots[slot];
	    InitUnit (unit, type, player);
	    unit->Active=0;
	    unit->Removed=0;
	    unit->Reset=0;
	    DebugCheck( unit->Slot!=slot );
	} else if( gh_eq_p(value,gh_symbol2scm("next")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
#if 0
	    // This is currently not used.
	    if( !gh_null_p(value) ) {
		str=gh_scm2newstr(value,NULL);

		slot=strtol(str+1,NULL,16);
		unit->Next=UnitSlots[slot];
		if( !UnitSlots[slot] ) {
		    DebugLevel0Fn("FIXME: Forward reference not supported\n");
		}
		free(str);
	    }
#endif
	} else if( gh_eq_p(value,gh_symbol2scm("tile")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    unit->X=gh_scm2int(gh_car(value));
	    unit->Y=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(value,gh_symbol2scm("stats")) ) {
	    unit->Stats=&type->Stats[gh_scm2int(gh_car(list))];
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("pixel")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    unit->IX=gh_scm2int(gh_car(value));
	    unit->IY=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(value,gh_symbol2scm("frame")) ) {
	    unit->Frame=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("flipped-frame")) ) {
	    unit->Frame=128|gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("seen")) ) {
	    unit->SeenFrame=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("flipped-seen")) ) {
	    unit->SeenFrame=128|gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("not-seen")) ) {
	    unit->SeenFrame=-1;
	} else if( gh_eq_p(value,gh_symbol2scm("direction")) ) {
	    unit->Direction=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("attacked")) ) {
	    unit->Attacked=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("burning")) ) {
	    unit->Burning=1;
	} else if( gh_eq_p(value,gh_symbol2scm("destroyed")) ) {
	    unit->Destroyed=1;
	} else if( gh_eq_p(value,gh_symbol2scm("removed")) ) {
	    unit->Removed=1;
	} else if( gh_eq_p(value,gh_symbol2scm("selected")) ) {
	    unit->Selected=1;
	} else if( gh_eq_p(value,gh_symbol2scm("visible")) ) {
	    str=s=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	    for( i=0; i<PlayerMax && *s; ++i,++s ) {
		if( *s=='-' || *s=='_' || *s==' ' ) {
		    unit->Visible&=~(1<<i);
		} else {
		    unit->Visible|=(1<<i);
		}
	    }
	    free(str);
	} else if( gh_eq_p(value,gh_symbol2scm("constructed")) ) {
	    unit->Constructed=1;
	} else if( gh_eq_p(value,gh_symbol2scm("active")) ) {
	    unit->Active=1;
	} else if( gh_eq_p(value,gh_symbol2scm("mana")) ) {
	    unit->Mana=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("hp")) ) {
	    unit->HP=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("xp")) ) {
	    unit->XP=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("kills")) ) {
	    unit->Kills=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("ttl")) ) {
	    unit->TTL=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("bloodlust")) ) {
	    unit->Bloodlust=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("haste")) ) {
	    unit->Haste=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("slow")) ) {
	    unit->Slow=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("invisible")) ) {
	    unit->Invisible=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("flame-shield")) ) {
	    unit->FlameShield=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("unholy-armor")) ) {
	    unit->UnholyArmor=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("group-id")) ) {
	    unit->GroupId=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("last-group")) ) {
	    unit->LastGroup=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("value")) ) {
	    unit->Value=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("sub-action")) ) {
	    unit->SubAction=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("wait")) ) {
	    unit->Wait=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("state")) ) {
	    unit->State=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("reset")) ) {
	    unit->Reset=1;
	} else if( gh_eq_p(value,gh_symbol2scm("blink")) ) {
	    unit->Blink=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("moving")) ) {
	    unit->Moving=1;
	} else if( gh_eq_p(value,gh_symbol2scm("rs")) ) {
	    unit->Rs=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("revealer")) ) {
	    unit->Revealer=1;
	} else if( gh_eq_p(value,gh_symbol2scm("on-board")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    for( i=0; i<MAX_UNITS_ONBOARD; ++i ) {
		value=gh_vector_ref(sublist,gh_int2scm(i));
		if( !gh_null_p(value) ) {
		    str=gh_scm2newstr(value,NULL);

		    slot=strtol(str+1,NULL,16);
		    unit->OnBoard[i]=UnitSlots[slot];
		    if( !UnitSlots[slot] ) {
			DebugLevel0Fn("FIXME: Forward reference not supported\n");
		    }
		    ++UnitSlots[slot]->Refs;
		    free(str);
		}
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("order-count")) ) {
	    unit->OrderCount=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("order-flush")) ) {
	    unit->OrderFlush=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("orders")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    CclParseOrders(unit,sublist);
	} else if( gh_eq_p(value,gh_symbol2scm("saved-order")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    CclParseOrder(value,&unit->SavedOrder);
	} else if( gh_eq_p(value,gh_symbol2scm("new-order")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    CclParseOrder(value,&unit->NewOrder);

	} else if( gh_eq_p(value,gh_symbol2scm("data-builded")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    CclParseBuilded(unit,value);
	} else if( gh_eq_p(value,gh_symbol2scm("data-research")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    DebugLevel0Fn("FIXME: research\n");
	} else if( gh_eq_p(value,gh_symbol2scm("data-upgrade-to")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    DebugLevel0Fn("FIXME: upgrade-to\n");
	} else if( gh_eq_p(value,gh_symbol2scm("data-train")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    CclParseTrain (unit, sublist);
	} else if( gh_eq_p(value,gh_symbol2scm("data-move")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    DebugLevel0Fn("FIXME: move\n");

	} else {
	   // FIXME: this leaves a half initialized unit
	   errl("Unsupported tag",value);
	}
    }

    //
    //	Place on map
    //
    if( !unit->Removed && !unit->Destroyed ) {
	unit->Removed=1;
	PlaceUnit(unit,unit->X,unit->Y);

    //
    //	Connect unit to position (on-board,building,in store,in deposite)
    //
    } else {
    }

    // FIXME: johns: works only for debug code.
    NewResetPath(unit);
    DebugLevel0Fn("FIXME: not written\n");

    return SCM_UNSPECIFIED;
}

/**
**	Make an unit.
**
**	@param type	Unit-type of the unit,
**	@param player	Owning player number of the unit.
**
**	@return		Returns the slot number of the made unit.
*/
local SCM CclMakeUnit(SCM type,SCM player)
{
    UnitType* unittype;
    Unit* unit;

    unittype=CclGetUnitType(type);
    unit=MakeUnit(unittype,&Players[gh_scm2int(player)]);

    return gh_int2scm(unit->Slot);
}

/**
**	Place an unit on map.
**
**	@param unit	Unit (slot number) to be placed.
**	@param x	X map tile position.
**	@param y	Y map tile position.
**
**	@return		Returns the slot number of the made placed.
*/
local SCM CclPlaceUnit(SCM unit,SCM x,SCM y)
{
    PlaceUnit(CclGetUnit(unit),gh_scm2int(x),gh_scm2int(y));
    return unit;
}

/**
**	Get the unholy-armor of the unit structure.
**
**	@param ptr	Unit object.
**
**	@return		The unholy-armor of the unit.
*/
local SCM CclGetUnitUnholyArmor(SCM ptr)
{
    const Unit* unit;
    SCM value;

    unit=CclGetUnit(ptr);
    value=gh_int2scm(unit->UnholyArmor);
    return value;
}

/**
**	Set the unholy-armor of the unit structure.
**
**	@param ptr	Unit object.
**	@param value	The value to set.
**
**	@return		The value of the unit.
*/
local SCM CclSetUnitUnholyArmor(SCM ptr,SCM value)
{
    Unit* unit;

    unit=CclGetUnit(ptr);
    unit->UnholyArmor=gh_scm2int(value);

    return value;
}

local SCM CclSetNumUnitsToLoad (SCM num)
{
    NumOfUnitsToLoad = gh_scm2int(num);
    DebugLevel0Fn ("Will load %d units.\n", NumOfUnitsToLoad);
    return SCM_UNSPECIFIED;
}

local SCM CclSlotUsage (SCM list)
{
#if 0
    /* the old way */
    int len = gh_vector_length (vector);
    unsigned char SlotUsage[len];
    int i;
    Unit *UnitMemory;

    for (i=0; i<len; i++) {
	SlotUsage[i] = (unsigned char )
		gh_scm2int (gh_vector_ref (vector, gh_int2scm (i)));
    }
#else
    int len = MAX_UNIT_SLOTS/8 + 1;
    unsigned char SlotUsage[len];
    int i, prev;
    Unit *UnitMemory;
    SCM value;

    DebugLevel0Fn ("entered\n");
    memset (SlotUsage, 0, len);
    prev = -1;
    while ( !gh_null_p (list) ) {
	value = gh_car (list);
	list = gh_cdr (list);
	if (gh_eq_p (value, gh_symbol2scm ("-"))) {
	    int range_end;
	    value = gh_car (list);
	    list = gh_cdr (list);
	    range_end = gh_scm2int (value);
	    for (i=prev; i<=range_end; i++)
		SlotUsage[i/8] |= 1 << (i%8);
	    prev = -1;
	} else {
	    if (prev >= 0)
		SlotUsage[prev/8] |= 1 << (prev%8);
	    prev = gh_scm2int (value);
	}
    }
#endif

    UnitMemory = (Unit * )calloc (NumOfUnitsToLoad, sizeof (Unit));
    /* now walk through the bitfield and create the needed unit slots */
    for (i=0; i<len*8; i++) {
	if ( SlotUsage[i/8] & (1 << i%8) ) {
	    Unit *new_unit = UnitMemory++;
	    UnitSlotFree = (void *)UnitSlots[i];
	    UnitSlots[i] = new_unit;
	    new_unit->Slot = i;
	}
    }
    DebugLevel0Fn ("leaved\n");
    return SCM_UNSPECIFIED;
}

// FIXME: write the missing access functions

/**
**	Register CCL features for unit.
*/
global void UnitCclRegister(void)
{
    gh_new_procedure1_0("set-hitpoint-regeneration!",
	    CclSetHitPointRegeneration);
    gh_new_procedure1_0("set-training-queue!",CclSetTrainingQueue);
    gh_new_procedure1_0("set-building-capture!",CclSetBuildingCapture);
    gh_new_procedure1_0("set-reveal-attacker!",CclSetRevealAttacker);

    gh_new_procedureN("unit",CclUnit);

    gh_new_procedure2_0("make-unit",CclMakeUnit);
    gh_new_procedure3_0("place-unit",CclPlaceUnit);

    // unit member access functions
    gh_new_procedure1_0("get-unit-unholy-armor",CclGetUnitUnholyArmor);
    gh_new_procedure2_0("set-unit-unholy-armor!",CclSetUnitUnholyArmor);

    gh_new_procedure1_0 ("num-units!", CclSetNumUnitsToLoad);
    gh_new_procedure1_0 ("slot-usage", CclSlotUsage);
}

//@}
