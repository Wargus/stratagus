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

//Modify types
#define MOD_ADD 1
#define MOD_SUB 2
#define MOD_MUL 3
#define MOD_DIV 4
#define MOD_MOD 5


/* virtual */ void CAnimation_SetVar::Action(CUnit &unit, int &/*move*/, int /*scale*/) const
{
	Assert(unit.Anim.Anim == this);

	char arg1[128];
	CUnit *goal = &unit;

	strcpy(arg1, this->varStr.c_str());
	const int rop = ParseAnimInt(&unit, this->valueStr.c_str());

	char *next = strchr(arg1, '.');
	if (next == NULL) {
		fprintf(stderr, "Need also specify the variable '%s' tag \n" _C_ arg1);
		Exit(1);
	} else {
		*next = '\0';
	}
	const int index = UnitTypeVar.VariableNameLookup[arg1];// User variables
	if (index == -1) {
		fprintf(stderr, "Bad variable name '%s'\n" _C_ arg1);
		Exit(1);
	}
	if (this->unitSlotStr.empty() == false) {
		switch (this->unitSlotStr[0]) {
			case 'l': // last created unit
				goal = Units[NumUnits - 1];
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
	int value = 0;
	if (!strcmp(next + 1, "Value")) {
		value = goal->Variable[index].Value;
	} else if (!strcmp(next + 1, "Max")) {
		value = goal->Variable[index].Max;
	} else if (!strcmp(next + 1, "Increase")) {
		value = goal->Variable[index].Increase;
	} else if (!strcmp(next + 1, "Enable")) {
		value = goal->Variable[index].Enable;
	}
	switch (this->mod) {
		case MOD_ADD:
			value += rop;
			break;
		case MOD_SUB:
			value -= rop;
			break;
		case MOD_MUL:
			value *= rop;
			break;
		case MOD_DIV:
			if (!rop) {
				fprintf(stderr, "Division by zero in AnimationSetVar\n");
				Exit(1);
			}
			value /= rop;
			break;
		case MOD_MOD:
			if (!rop) {
				fprintf(stderr, "Division by zero in AnimationSetVar\n");
				Exit(1);
			}
			value %= rop;
			break;
		default:
			value = rop;
	}
	if (!strcmp(next + 1, "Value")) {
		goal->Variable[index].Value = value;
	} else if (!strcmp(next + 1, "Max")) {
		goal->Variable[index].Max = value;
	} else if (!strcmp(next + 1, "Increase")) {
		goal->Variable[index].Increase = value;
	} else if (!strcmp(next + 1, "Enable")) {
		goal->Variable[index].Enable = value;
	}
}

/*
**  s = "var mod value [unitSlot]"
*/
/* virtual */ void CAnimation_SetVar::Init(const char *s)
{
	const std::string str(s);
	const size_t len = str.size();

	size_t begin = 0;
	size_t end = str.find(' ', begin);
	this->varStr.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	const std::string modStr(str, begin, end - begin);
	this->mod = atoi(modStr.c_str());

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	this->valueStr.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	this->unitSlotStr.assign(str, begin, end - begin);
}

//@}
