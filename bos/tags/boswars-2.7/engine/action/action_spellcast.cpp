//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_spellcast.cpp - The spell cast action. */
//
//      (c) Copyright 2000-2008 by Vladi Belperchinov-Shabanski and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

/*
** This is inherited from action_attack.c, actually spell casting will
** be considered a `special' case attack action... //Vladi
*/

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "video.h"
#include "unittype.h"
#include "animation.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "pathfinder.h"
#include "sound.h"
#include "map.h"
#include "spells.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

#if 0

/**
**  Animate unit spell cast (it is attack really)!
**
**  @param unit  Unit, for that spell cast/attack animation is played.
*/
void AnimateActionSpellCast(CUnit *unit)
{
	int flags;

	if (unit->Type->Animations) {
		Assert(unit->Type->Animations->Attack);

		flags = UnitShowAnimation(unit, unit->Type->Animations->Attack);

		if (flags & AnimationMissile) { // FIXME: should cast spell ?
			FireMissile(unit);          // we should not get here ??
		}
	}
}

#endif

/**
**  Handle moving to the target.
**
**  @param unit  Unit, for that the spell cast is handled.
*/
static void SpellMoveToTarget(CUnit *unit)
{
	CUnit *goal;
	int err;

	// Unit can't move
	err = 1;
	if (CanMove(unit)) {
		err = DoActionMove(unit);
		if (unit->Anim.Unbreakable) {
			return;
		}
	}

	// when reached DoActionMove changes unit action
	// FIXME: use return codes from pathfinder
	goal = unit->Orders[0]->Goal;

	if (goal && MapDistanceBetweenUnits(unit, goal) <=
			unit->Orders[0]->Range) {

		// there is goal and it is in range
		unit->State = 0;
		UnitHeadingFromDeltaXY(unit,
			goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
			goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);
		unit->SubAction++; // cast the spell
		return;
	} else if (!goal && MapDistanceToUnit(unit->Orders[0]->X,
			unit->Orders[0]->Y, unit) <= unit->Orders[0]->Range) {
		// there is no goal and target spot is in range
		UnitHeadingFromDeltaXY(unit,
			unit->Orders[0]->X + unit->Orders[0]->Arg1.Spell->Range - unit->X,
			unit->Orders[0]->Y + unit->Orders[0]->Arg1.Spell->Range - unit->Y);
		unit->SubAction++; // cast the spell
		return;
	} else if (err == PF_UNREACHABLE) {
		//
		// goal/spot unreachable and out of range -- give up
		//
		unit->ClearAction();
		unit->State = 0;

		if (unit->Orders[0]->Goal) { // Release references
			unit->Orders[0]->Goal->RefsDecrease();
			unit->Orders[0]->Goal = NoUnitP;
		}
	}
	Assert(!unit->Type->Vanishes && !unit->Destroyed);
}

/**
**  Unit casts a spell!
**
**  @param unit  Unit, for that the spell cast is handled.
*/
void HandleActionSpellCast(CUnit *unit)
{
	const SpellType *spell;

	if (unit->Wait) {
		unit->Wait--;
		return;
	}

	switch (unit->SubAction) {
		case 0:
			//
			// Check if we can cast the spell.
			//
			spell = unit->Orders[0]->Arg1.Spell;
			if (!CanCastSpell(unit, spell, unit->Orders[0]->Goal,
					unit->Orders[0]->X, unit->Orders[0]->Y)) {

				//
				// Notify player about this problem
				//
				if (unit->Variable[MANA_INDEX].Value < spell->ManaCost) {
					unit->Player->Notify(NotifyYellow, unit->X, unit->Y,
						_("%s: not enough mana for spell: %s"),
						unit->Type->Name.c_str(), spell->Name.c_str());
				} else {
					unit->Player->Notify(NotifyYellow, unit->X, unit->Y,
						_("%s: can't cast spell: %s"),
						unit->Type->Name.c_str(), spell->Name.c_str());
				}

				if (unit->Player->AiEnabled) {
					DebugPrint("FIXME: do we need an AI callback?\n");
				}
				unit->ClearAction();
				if (unit->Orders[0]->Goal) {
					unit->Orders[0]->Goal->RefsDecrease();
					unit->Orders[0]->Goal = NoUnitP;
				}
				return;
			}
			// FIXME FIXME FIXME: Check if already in range and skip straight to 2(casting)
			NewResetPath(unit);
			unit->ReCast = 0; // repeat spell on next pass? (defaults to `no')
			unit->SubAction = 1;
			// FALL THROUGH
		case 1:                         // Move to the target.
			if ((spell = unit->Orders[0]->Arg1.Spell)->Range != INFINITE_RANGE) {
				SpellMoveToTarget(unit);
				break;
			} else {
				unit->SubAction = 2;
			}
			// FALL THROUGH
		case 2:                         // Cast spell on the target.
			// FIXME: should use AnimateActionSpellCast here
			if (unit->Type->Animations->Attack) {
				UnitShowAnimation(unit, unit->Type->Animations->Attack);
				if (unit->Anim.Unbreakable) { // end of animation
					return;
				}
			} else {
				// FIXME: what todo, if unit/goal is removed?
				if (unit->Orders[0]->Goal && !unit->Orders[0]->Goal->IsVisibleAsGoal(unit->Player)) {
					unit->ReCast = 0;
				} else {
					spell = unit->Orders[0]->Arg1.Spell;
					unit->ReCast = SpellCast(unit, spell, unit->Orders[0]->Goal,
						unit->Orders[0]->X, unit->Orders[0]->Y);
				}
			}
			if (!unit->ReCast && unit->Orders[0]->Action != UnitActionDie) {
				unit->ClearAction();
				if (unit->Orders[0]->Goal) {
					unit->Orders[0]->Goal->RefsDecrease();
					unit->Orders[0]->Goal = NoUnitP;
				}
			}
			break;

		default:
			unit->SubAction = 0; // Reset path, than move to target
			break;
	}
}

//@}
