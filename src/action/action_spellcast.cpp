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
/**@name action_spellcast.c	-	The spell cast action. */
//
//	(c) Copyright 2000,2001 by Vladi Belperchinov-Shabanski
//
//	$Id$

/*
** This is inherited from action_attack.c, actually spell casting will
** be considered a `special' case attack action... //Vladi
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "pathfinder.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"
#include "spells.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Animate unit spell cast (it is attack really)!
**
**	@param unit	Unit, for that spell cast/attack animation is played.
*/
global void AnimateActionSpellCast(Unit * unit)
{
    int flags;

    if (unit->Type->Animations) {
	DebugCheck(!unit->Type->Animations->Attack);

	flags = UnitShowAnimation(unit, unit->Type->Animations->Attack);

	if ((flags & AnimationSound)) {	
	    PlayUnitSound(unit, VoiceAttacking);	// FIXME: spell sound?
	}

	if (flags & AnimationMissile) {	// FIXME: should cast spell ?
	    FireMissile(unit);		// we should not get here ?? 
	}
    }
}

/**
**	Handle moving to the target.
**
**	@param unit	Unit, for that the spell cast is handled.
*/
local void SpellMoveToTarget(Unit * unit)
{
    Unit *goal;
    int err;

    err = DoActionMove(unit);
    if (!unit->Reset) {
	return;
    }

    // when reached DoActionMove changes unit action
#ifdef NEW_ORDERS
    goal = unit->Orders[0].Goal;
    if (goal && MapDistanceToUnit(unit->X, unit->Y, goal)
	    <= unit->Orders[0].RangeX) {

	// there is goal and it is in range
	unit->State = 0;
	UnitHeadingFromDeltaXY(unit, goal->X - unit->X, goal->Y - unit->Y);
	unit->SubAction++;		// cast the spell
	return;
    } else if (!goal && MapDistance(unit->X, unit->Y, unit->Orders[0].X,
	unit->Orders[0].Y) <= unit->Orders[0].RangeX) {
	// there is no goal and target spot is in range
	unit->State = 0;
	UnitHeadingFromDeltaXY(unit,
		unit->Orders[0].X
			+((SpellType*)unit->Orders[0].Arg1)->Range-unit->X,
		unit->Orders[0].Y
			+((SpellType*)unit->Orders[0].Arg1)->Range-unit->Y);
	unit->SubAction++;		// cast the spell
	return;
    } else if (err) {
	DebugCheck( unit->Orders[0].Action!=UnitActionStill );
#else
    unit->Command.Action = UnitActionSpellCast;

    goal = unit->Command.Data.Move.Goal;
    if (goal && MapDistanceToUnit(unit->X, unit->Y, goal)
	    <= unit->Command.Data.Move.Range) {

	// there is goal and it is in range
	unit->State = 0;
	UnitHeadingFromDeltaXY(unit, goal->X - unit->X, goal->Y - unit->Y);
	unit->SubAction++;		// cast the spell
	return;
    } else if (!goal
	       && MapDistance(unit->X, unit->Y, unit->Command.Data.Move.DX,
			      unit->Command.Data.Move.DY)
	       <= unit->Command.Data.Move.Range) {
	// there is no goal and target spot is in range
	unit->State = 0;
	UnitHeadingFromDeltaXY(unit, unit->Command.Data.Move.DX - unit->X,
			       unit->Command.Data.Move.DY - unit->Y);
	unit->SubAction++;		// cast the spell
	return;
    } else if (err) {
#endif
	// goal/spot out of range -- move to target
	// FIXME: Should handle new return codes (err) here (for Fabrice)
	unit->State = 0;
	unit->SubAction = 0;
    }
    DebugCheck(unit->Type->Vanishes || unit->Destroyed);
}

/**
**	Unit casts a spell!
**
**	@param unit	Unit, for that the spell cast is handled.
*/
global void HandleActionSpellCast(Unit * unit)
{
    int repeat;
    const SpellType *spell;

    repeat = 0;			// repeat spell on next pass? (defaults to `no')

    DebugLevel3Fn("SpellCast %Zd\n", UnitNumber(unit));

    switch (unit->SubAction) {

    case 0:				// first entry.
#ifdef NEW_ORDERS
	NewResetPath(unit);
#endif
	unit->SubAction=1;
	// FALL THROUGH
    case 1:				// Move to the target.
	SpellMoveToTarget(unit);
	break;

    case 2:				// Cast spell on the target.
#ifdef NEW_ORDERS
	spell = unit->Orders[0].Arg1;
#else
	spell = unit->Command.Data.Move.Spell;
#endif
	// FIXME: Use the general unified message system.
	if (unit->Mana < spell->ManaCost) {
	    if (unit->Player == ThisPlayer) {
		SetMessage("%s: not enough mana for spell: %s",
			   unit->Type->Name, spell->Name);
	    }
	    repeat = 0;
	} else {
	    UnitShowAnimation(unit, unit->Type->Animations->Attack);
	    if (!unit->Reset) {
		return;
	    }
	    // FIXME: what todo, if unit/goal is removed?
#ifdef NEW_ORDERS
	    if (unit->Orders[0].Goal && unit->Orders[0].Goal->Destroyed) {
		repeat = 0;
	    } else {
		repeat = SpellCast(unit->Orders[0].Arg1, unit,
		   unit->Orders[0].Goal,unit->Orders[0].X,unit->Orders[0].Y);
	    }
	}
	if (!repeat) {
	    unit->Orders[0].Action = UnitActionStill;
	    unit->SubAction = 0;
	    unit->Wait = 1;
	    if (unit->Orders[0].Goal) {
		RefsDebugCheck(!unit->Orders[0].Goal->Refs);
		unit->Orders[0].Goal->Refs--;
		RefsDebugCheck(!unit->Orders[0].Goal->Refs);
		unit->Orders[0].Goal=NoUnitP;
	    }
	}
#else
	    if (unit->Command.Data.Move.Goal
		    && unit->Command.Data.Move.Goal->Destroyed) {
		repeat = 0;
	    } else {
		repeat = SpellCast(spell, unit,
		   unit->Command.Data.Move.Goal,
		   unit->Command.Data.Move.DX,unit->Command.Data.Move.DY);
	    }
	}
	if (!repeat) {
	    unit->Command.Action = UnitActionStill;
	    unit->SubAction = 0;
	    unit->Wait = 1;
	    if (unit->Command.Data.Move.Goal) {
		RefsDebugCheck(!unit->Command.Data.Move.Goal->Refs);
		unit->Command.Data.Move.Goal->Refs--;
		RefsDebugCheck(!unit->Command.Data.Move.Goal->Refs);
	    }
	}
#endif
	break;

    default:
	unit->SubAction = 0;		// Move to the target
	break;
    }
}

//@}
