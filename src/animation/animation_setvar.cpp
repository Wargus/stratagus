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
/**@name animation_setvar.cpp - The animation SetVar. */
//
//      (c) Copyright 2012 by Joris Dauphin
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "animation/animation_setvar.h"

#include "actions.h"
#include "unit.h"
#include "unit_manager.h"
#include "map.h"

#include <stdio.h>
#include <sstream>

void CAnimation_SetVar::Action(CUnit &unit, int & /*move*/, int /*scale*/) const /* override */
{
	Assert(unit.Anim.Anim == this);

	CUnit *goal = &unit;
	if (this->unitSlotStr.empty() == false) {
		switch (this->unitSlotStr[0]) {
			case 'l': // last created unit
				goal = UnitManager->lastCreatedUnit();
				break;
			case 't': // target unit
				goal = unit.CurrentOrder()->GetGoal();
				break;
			case 's': // unit self (no use)
				goal = &unit;
				break;
		}
	}
	if (!goal) {
		return;
	}
	auto dot_pos = this->varStr.find('.');
	auto arg1 = std::string_view{this->varStr.c_str(), dot_pos};
	if (dot_pos == std::string_view::npos) {
		// Special case for non-CVariable variables
		if (arg1 == "DamageType") {
			int death = ExtraDeathIndex(this->valueStr);
			if (death == ANIMATIONS_DEATHTYPES) {
				ErrorPrint("Incorrect death type: %s\n", this->valueStr.c_str());
				Exit(1);
				return;
			}
			goal->Type->DamageType = this->valueStr;
			return;
		}
		ErrorPrint("Need also specify the variable '%s' tag\n", arg1.data());
		Exit(1);
		return;
	}
	const int index = UnitTypeVar.VariableNameLookup[arg1];// User variables
	if (index == -1) {
		ErrorPrint("Bad variable name '%s'\n", arg1.data());
		Exit(1);
		return;
	}

	const int rop = ParseAnimInt(unit, this->valueStr);
	const auto next = std::string_view{this->varStr.c_str() + dot_pos + 1};
	int value = 0;
	if (next == "Value") {
		value = goal->Variable[index].Value;
		modifyValue(this->mod, value, rop);
		goal->Variable[index].Value = value;
	} else if (next == "Max") {
		value = goal->Variable[index].Max;
		modifyValue(this->mod, value, rop);
		goal->Variable[index].Max = value;
		// Special case: when adjusting the sight range, we need to update the visibility
		if (index == SIGHTRANGE_INDEX) {
			MapUnmarkUnitSight(unit);
			unit.CurrentSightRange = value;
			MapMarkUnitSight(unit);
		}
	} else if (next == "Increase") {
		value = goal->Variable[index].Increase;
		modifyValue(this->mod, value, rop);
		goal->Variable[index].Increase = value;
	} else if (next == "Enable") {
		value = goal->Variable[index].Enable;
		modifyValue(this->mod, value, rop);
		goal->Variable[index].Enable = value;
	} else if (next == "Percent") {
		value = goal->Variable[index].Value * 100 / goal->Variable[index].Max;
		modifyValue(this->mod, value, rop);
		goal->Variable[index].Value = goal->Variable[index].Max * value / 100;
	}
	clamp(&goal->Variable[index].Value, 0, goal->Variable[index].Max);
}

/*
**  s = "var mod value [unitSlot]"
*/
void CAnimation_SetVar::Init(std::string_view s, lua_State *) /* override */
{
	std::istringstream is{std::string(s)};

	std::string modStr;
	is >> this->varStr >> modStr >> this->valueStr >> this->unitSlotStr;
	this->mod = toSetVar_ModifyTypes(modStr);
}

//@}
