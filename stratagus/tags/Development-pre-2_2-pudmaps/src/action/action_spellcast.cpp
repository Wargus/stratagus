//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name action_spellcast.c - The spell cast action. */
//
//      (c) Copyright 2000-2004 by Vladi Belperchinov-Shabanski
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
//      $Id$

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
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "pathfinder.h"
#include "sound.h"
#include "tileset.h"
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
void AnimateActionSpellCast(Unit* unit)
{
	int flags;

	if (unit->Type->Animations) {
		Assert(unit->Type->Animations->Attack);

		flags = UnitShowAnimation(unit, unit->Type->Animations->Attack);

		if ((flags & AnimationSound)) {
			PlayUnitSound(unit, VoiceAttacking); // FIXME: spell sound?
		}

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
static void SpellMoveToTarget(Unit* unit)
{
	SpellType* spell;
	Unit* goal;
	int err;

	// Unit can't move
	err = 1;
	if (CanMove(unit)) {
		err = DoActionMove(unit);
		if (!unit->Reset) {
			return;
		}
	}

	// when reached DoActionMove changes unit action
	// FIXME: use return codes from pathfinder
	goal = unit->Orders[0].Goal;

	if (goal && MapDistanceBetweenUnits(unit, goal) <=
			unit->Orders[0].Range) {

		// there is goal and it is in range
		unit->State = 0;
		UnitHeadingFromDeltaXY(unit,
			goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
			goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);
		unit->SubAction++; // cast the spell
		return;
	} else if (!goal && MapDistanceToUnit(unit->Orders[0].X,
			unit->Orders[0].Y, unit) <= unit->Orders[0].Range) {
		// there is no goal and target spot is in range
		UnitHeadingFromDeltaXY(unit,
			unit->Orders[0].X +
				unit->Orders[0].Arg1.Spell->Range - unit->X,
			unit->Orders[0].Y +
				unit->Orders[0].Arg1.Spell->Range - unit->Y);
		unit->SubAction++; // cast the spell
		return;
	} else if (err) {
		//
		// Target self-> we don't really have to get in range,
		// just as close as possible, since the spell is centered
		// on the caster anyway.
		//
		if ((spell = unit->Orders[0].Arg1.Spell)->Target == TargetSelf) {
			DebugPrint("Increase range for spellcast.");
			unit->Orders->Range++;
		} else {
			//
			// goal/spot out of range -- give up
			//
			unit->Orders[0].Action = UnitActionStill;
			unit->State = unit->SubAction = 0;

			if (unit->Orders[0].Goal) { // Release references
				RefsDecrease(unit->Orders->Goal);
				unit->Orders[0].Goal = NoUnitP;
			}
		}
	}
	Assert(!unit->Type->Vanishes && !unit->Destroyed);
}

/**
**  Unit casts a spell!
**
**  @param unit  Unit, for that the spell cast is handled.
*/
void HandleActionSpellCast(Unit* unit)
{
	int flags;
	const SpellType* spell;

	switch (unit->SubAction) {
		case 0:
			//
			// Check if we can cast the spell.
			//
			spell = unit->Orders[0].Arg1.Spell;
			if (!CanCastSpell(unit, spell, unit->Orders[0].Goal,
					unit->Orders[0].X, unit->Orders[0].Y)) {

				//
				// Notify player about this problem
				//
				if (unit->Mana < spell->ManaCost) {
					NotifyPlayer(unit->Player,NotifyYellow,unit->X,unit->Y,
						"%s: not enough mana for spell: %s",
						unit->Type->Name, spell->Name);
				} else {
					NotifyPlayer(unit->Player,NotifyYellow,unit->X,unit->Y,
						"%s: can't cast spell: %s",
						unit->Type->Name, spell->Name);
				}

				if (unit->Player->AiEnabled) {
					DebugPrint("FIXME: do we need an AI callback?\n");
				}
				unit->Orders[0].Action = UnitActionStill;
				unit->SubAction = 0;
				unit->Wait = 1;
				if (unit->Orders[0].Goal) {
					RefsDecrease(unit->Orders->Goal);
					unit->Orders[0].Goal = NoUnitP;
				}
				return;
			}
			// FIXME FIXME FIXME: Check if already in range and skip straight to 2(casting)
			NewResetPath(unit);
			unit->ReCast = 0; // repeat spell on next pass? (defaults to `no')
			unit->SubAction = 1;
			// FALL THROUGH
		case 1:                         // Move to the target.
			if ((spell = unit->Orders[0].Arg1.Spell)->Range != INFINITE_RANGE) {
				SpellMoveToTarget(unit);
				break;
			} else {
				unit->SubAction = 2;
			}
			// FALL THROUGH
		case 2:                         // Cast spell on the target.
			// FIXME: should use AnimateActionSpellCast here
			if (unit->Type->Animations && unit->Type->Animations->Attack) {
				flags = UnitShowAnimation(unit, unit->Type->Animations->Attack);
				if (flags & AnimationMissile) {
					// FIXME: what todo, if unit/goal is removed?
					if (unit->Orders[0].Goal && !UnitVisibleAsGoal(unit->Orders->Goal, unit->Player)) {
						unit->ReCast = 0;
					} else {
						spell = unit->Orders[0].Arg1.Spell;
						unit->ReCast = SpellCast(unit, spell, unit->Orders[0].Goal,
							unit->Orders[0].X, unit->Orders[0].Y);
					}
				}
				if (!(flags & AnimationReset)) { // end of animation
					return;
				}
			} else {
				// FIXME: what todo, if unit/goal is removed?
				if (unit->Orders[0].Goal && !UnitVisibleAsGoal(unit->Orders->Goal, unit->Player)) {
					unit->ReCast = 0;
				} else {
					spell = unit->Orders[0].Arg1.Spell;
					unit->ReCast = SpellCast(unit, spell, unit->Orders[0].Goal,
						unit->Orders[0].X, unit->Orders[0].Y);
				}
			}
			if (!unit->ReCast) {
				unit->Orders[0].Action = UnitActionStill;
				unit->SubAction = 0;
				unit->Wait = 1;
				if (unit->Orders[0].Goal) {
					RefsDecrease(unit->Orders->Goal);
					unit->Orders[0].Goal = NoUnitP;
				}
			}
			break;

		default:
			unit->SubAction = 0; // Reset path, than move to target
			break;
	}
}

//@}
