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
/**@name action_spellcast.cpp - The spell cast action. */
//
//      (c) Copyright 2000-2005 by Vladi Belperchinov-Shabanski and Jimmy Salmon
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
#include "tileset.h"
#include "map.h"
#include "spells.h"
#include "interface.h"
#include "iolib.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/* virtual */ COrder_SpellCast *COrder_SpellCast::Clone() const
{
	return new COrder_SpellCast(*this);
}

/* virtual */ void COrder_SpellCast::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-spell-cast\",");

	file.printf(" \"range\", %d,", this->Range);
	file.printf(" \"width\", %d,", this->Width);
	file.printf(" \"height\", %d,", this->Height);
	file.printf(" \"min-range\", %d,", this->MinRange);
	if (this->HasGoal()) {
		CUnit &goal = *this->GetGoal();
		if (goal.Destroyed) {
			/* this unit is destroyed so it's not in the global unit
			 * array - this means it won't be saved!!! */
			printf ("FIXME: storing destroyed Goal - loading will fail.\n");
		}
		file.printf(" \"goal\", \"%s\",", UnitReference(goal).c_str());
	}
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);

	file.printf(" \"spell\", \"%s\",\n  ", this->Spell->Ident.c_str());

	SaveDataMove(file);
	file.printf("}");
}

/* virtual */ bool COrder_SpellCast::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "spell")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->Spell = SpellTypeByIdent(LuaToString(l, -1));
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}


/**
**  Call when animation step is "attack"
*/
/* virtual */ void COrder_SpellCast::OnAnimationAttack(CUnit &unit)
{
	CUnit *goal = GetGoal();
	if (goal && !goal->IsVisibleAsGoal(*unit.Player)) {
		unit.ReCast = 0;
	} else {
		unit.ReCast = SpellCast(unit, this->Spell, goal, goalPos.x, goalPos.y);
	}
	UnHideUnit(unit); // unit is invisible until attacks
}



/**
**  Animate unit spell cast 
**
**  @param unit  Unit, for that spell cast/attack animation is played.
*/
static void AnimateActionSpellCast(CUnit &unit, COrder_SpellCast &order)
{
	const CAnimations *animations = unit.Type->Animations;

	if (!animations || (!animations->Attack && !animations->SpellCast)) {
		// if don't have animations just cast spell
		order.OnAnimationAttack(unit);
		return;
	}
	if (animations->SpellCast) {
		UnitShowAnimation(unit, animations->SpellCast);
	} else {
		UnitShowAnimation(unit, animations->Attack);
	}
}

/**
**  Handle moving to the target.
**
**  @param unit  Unit, for that the spell cast is handled.
*/
static void SpellMoveToTarget(CUnit &unit)
{
	CUnit *goal;
	int err;

	// Unit can't move
	err = 1;
	if (unit.CanMove()) {
		err = DoActionMove(unit);
		if (unit.Anim.Unbreakable) {
			return;
		}
	}

	// when reached DoActionMove changes unit action
	// FIXME: use return codes from pathfinder
	COrderPtr order = unit.CurrentOrder();
	goal = order->GetGoal();

	if (goal && unit.MapDistanceTo(*goal) <= order->Range) {
		// there is goal and it is in range
		unit.State = 0;
		UnitHeadingFromDeltaXY(unit, goal->tilePos + goal->Type->GetHalfTileSize() - unit.tilePos);
		unit.SubAction++; // cast the spell
		return;
	} else if (!goal && unit.MapDistanceTo(order->goalPos.x, order->goalPos.y) <= order->Range) {
		// there is no goal and target spot is in range
		UnitHeadingFromDeltaXY(unit, order->goalPos - unit.tilePos);
		unit.SubAction++; // cast the spell
		return;
	} else if (err == PF_UNREACHABLE) {
		//
		// goal/spot unreachable and out of range -- give up
		//
		unit.ClearAction();
		unit.State = 0;
		order->ClearGoal(); // Release references
	}
	Assert(!unit.Type->Vanishes && !unit.Destroyed);
}


/* virtual */ bool COrder_SpellCast::Execute(CUnit &unit)
{
	COrder_SpellCast &order = *this;

	if (unit.Wait) {
		unit.Wait--;
		return false;
	}
	const SpellType &spell = order.GetSpell();
	switch (unit.SubAction) {
		case 0:
			// Check if we can cast the spell.
			if (!CanCastSpell(unit, &spell, order.GetGoal(), order.goalPos.x, order.goalPos.y)) {
				// Notify player about this problem
				if (unit.Variable[MANA_INDEX].Value < spell.ManaCost) {
					unit.Player->Notify(NotifyYellow, unit.tilePos.x, unit.tilePos.y,
						_("%s: not enough mana for spell: %s"),
						unit.Type->Name.c_str(), spell.Name.c_str());
				} else {
					unit.Player->Notify(NotifyYellow, unit.tilePos.x, unit.tilePos.y,
						_("%s: can't cast spell: %s"),
						unit.Type->Name.c_str(), spell.Name.c_str());
				}

				if (unit.Player->AiEnabled) {
					DebugPrint("FIXME: do we need an AI callback?\n");
				}
				return true;
			}
			// FIXME FIXME FIXME: Check if already in range and skip straight to 2(casting)
			if (!spell.IsCasterOnly()) {
				unit.CurrentOrder()->NewResetPath();
			}
			unit.ReCast = 0; // repeat spell on next pass? (defaults to `no')
			unit.SubAction = 1;
			// FALL THROUGH
		case 1:                         // Move to the target.
			if (spell.Range && spell.Range != INFINITE_RANGE) {
				SpellMoveToTarget(unit);
				break;
			} else {
				unit.SubAction = 2;
			}
			// FALL THROUGH
		case 2:                         // Cast spell on the target.
			if (!spell.IsCasterOnly()) {
				AnimateActionSpellCast(unit, *this);
				if (unit.Anim.Unbreakable) {
					return false;
				}
			} else {
				// FIXME: what todo, if unit/goal is removed?
				CUnit *goal = order.GetGoal();
				if (goal && goal != &unit && !goal->IsVisibleAsGoal(*unit.Player)) {
					unit.ReCast = 0;
				} else {
					unit.ReCast = SpellCast(unit, &spell, goal, order.goalPos.x, order.goalPos.y);
				}
			}
			if (!unit.ReCast && unit.CurrentAction() != UnitActionDie) {
				return true;
			}
			break;

		default:
			unit.SubAction = 0; // Reset path, than move to target
			break;
	}
	return false;
}

/* virtual */ void COrder_SpellCast::Cancel(CUnit&)
{
}


/**
**  Unit casts a spell!
**
**  @param unit  Unit, for that the spell cast is handled.
*/
void HandleActionSpellCast(COrder& order, CUnit &unit)
{
	Assert(order.Action == UnitActionSpellCast);

	if (order.Execute(unit)) {
		order.ClearGoal();
		unit.ClearAction();
	}
}

//@}
