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
#include <stdlib.h>

#include "freecraft.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "ccl.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

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

    old=HitPointRegeneration;
    HitPointRegeneration=gh_scm2bool(flag);

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

    old=EnableTrainingQueue;
    EnableTrainingQueue=gh_scm2bool(flag);

    return gh_bool2scm(old);
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
	} else if( gh_eq_p(value,gh_symbol2scm("player")) ) {
	    player=&Players[gh_scm2int(gh_car(list))];
	    list=gh_cdr(list);

	    DebugCheck( !type );
	    unit=MakeUnit(type,player);
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
	    DebugLevel0Fn("FIXME: orders\n");
	} else if( gh_eq_p(value,gh_symbol2scm("saved-order")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    DebugLevel0Fn("FIXME: saved order\n");
	} else if( gh_eq_p(value,gh_symbol2scm("new-order")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    DebugLevel0Fn("FIXME: new order\n");
	} else if( gh_eq_p(value,gh_symbol2scm("data-builded")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    DebugLevel0Fn("FIXME: builded\n");
	} else if( gh_eq_p(value,gh_symbol2scm("data-research")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    DebugLevel0Fn("FIXME: research\n");
	} else if( gh_eq_p(value,gh_symbol2scm("data-upgrade-to")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    DebugLevel0Fn("FIXME: upgrade-to\n");
	} else if( gh_eq_p(value,gh_symbol2scm("data-train")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    DebugLevel0Fn("FIXME: train\n");
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
    PlaceUnit(UnitSlots[gh_scm2int(unit)],gh_scm2int(x),gh_scm2int(y));
    return unit;
}


/**
**	Register CCL features for unit.
*/
global void UnitCclRegister(void)
{
    gh_new_procedure1_0("set-hitpoint-regeneration!",
	    CclSetHitPointRegeneration);
    gh_new_procedure1_0("set-training-queue!",CclSetTrainingQueue);

    gh_new_procedureN("unit",CclUnit);

    gh_new_procedure2_0("make-unit",CclMakeUnit);
    gh_new_procedure3_0("place-unit",CclPlaceUnit);
}

//@}
