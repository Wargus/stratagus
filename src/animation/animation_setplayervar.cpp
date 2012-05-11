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
/**@name animation_setplayervar.cpp - The animation SetPlayerVar. */
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

#include "animation/animation_setplayervar.h"

#include "animation.h"
#include "unit.h"

//Modify types
#define MOD_ADD 1
#define MOD_SUB 2
#define MOD_MUL 3
#define MOD_DIV 4
#define MOD_MOD 5

/**
**  Gets the player data.
**
**  @param player  Player number.
**  @param prop    Player's property.
**  @param arg     Additional argument (for resource and unit).
**
**  @return  Returning value (only integer).
*/
int GetPlayerData(int player, const char *prop, const char *arg)
{
	if (!strcmp(prop, "RaceName")) {
		return Players[player].Race;
	} else if (!strcmp(prop, "Resources")) {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			fprintf(stderr, "Invalid resource \"%s\"", arg);
			Exit(1);
		}
		return Players[player].Resources[resId] + Players[player].StoredResources[resId];
	} else if (!strcmp(prop, "StoredResources")) {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			fprintf(stderr, "Invalid resource \"%s\"", arg);
			Exit(1);
		}
		return Players[player].StoredResources[resId];
	} else if (!strcmp(prop, "MaxResources")) {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			fprintf(stderr, "Invalid resource \"%s\"", arg);
			Exit(1);
		}
		return Players[player].MaxResources[resId];
	} else if (!strcmp(prop, "UnitTypesCount")) {
		const std::string unit(arg);
		CUnitType *type = UnitTypeByIdent(unit);
		return Players[player].UnitTypesCount[type->Slot];
	} else if (!strcmp(prop, "AiEnabled")) {
		return Players[player].AiEnabled;
	} else if (!strcmp(prop, "TotalNumUnits")) {
		return Players[player].GetUnitCount();
	} else if (!strcmp(prop, "NumBuildings")) {
		return Players[player].NumBuildings;
	} else if (!strcmp(prop, "Supply")) {
		return Players[player].Supply;
	} else if (!strcmp(prop, "Demand")) {
		return Players[player].Demand;
	} else if (!strcmp(prop, "UnitLimit")) {
		return Players[player].UnitLimit;
	} else if (!strcmp(prop, "BuildingLimit")) {
		return Players[player].BuildingLimit;
	} else if (!strcmp(prop, "TotalUnitLimit")) {
		return Players[player].TotalUnitLimit;
	} else if (!strcmp(prop, "Score")) {
		return Players[player].Score;
	} else if (!strcmp(prop, "TotalUnits")) {
		return Players[player].TotalUnits;
	} else if (!strcmp(prop, "TotalBuildings")) {
		return Players[player].TotalBuildings;
	} else if (!strcmp(prop, "TotalResources")) {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			fprintf(stderr, "Invalid resource \"%s\"", arg);
			Exit(1);
		}
		return Players[player].TotalResources[resId];
	} else if (!strcmp(prop, "TotalRazings")) {
		return Players[player].TotalRazings;
	} else if (!strcmp(prop, "TotalKills")) {
		return Players[player].TotalKills;
	} else {
		fprintf(stderr, "Invalid field: %s" _C_ prop);
		Exit(1);
	}
	return 0;
}

/**
**  Sets the player data.
*/
static void SetPlayerData(int player, const char *prop, const char *arg, int value)
{
	if (!strcmp(prop, "RaceName")) {
		Players[player].Race = value;
	} else if (!strcmp(prop, "Resources")) {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			fprintf(stderr, "Invalid resource \"%s\"", arg);
			Exit(1);
		}
		Players[player].SetResource(resId, value);
	} else if (!strcmp(prop, "StoredResources")) {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			fprintf(stderr, "Invalid resource \"%s\"", arg);
			Exit(1);
		}
		Players[player].SetResource(resId, value, true);
	} else if (!strcmp(prop, "UnitLimit")) {
		Players[player].UnitLimit = value;
	} else if (!strcmp(prop, "BuildingLimit")) {
		Players[player].BuildingLimit = value;
	} else if (!strcmp(prop, "TotalUnitLimit")) {
		Players[player].TotalUnitLimit = value;
	} else if (!strcmp(prop, "Score")) {
		Players[player].Score = value;
	} else if (!strcmp(prop, "TotalUnits")) {
		Players[player].TotalUnits = value;
	} else if (!strcmp(prop, "TotalBuildings")) {
		Players[player].TotalBuildings = value;
	} else if (!strcmp(prop, "TotalResources")) {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			fprintf(stderr, "Invalid resource \"%s\"", arg);
			Exit(1);
		}
		Players[player].TotalResources[resId] = value;
	} else if (!strcmp(prop, "TotalRazings")) {
		Players[player].TotalRazings = value;
	} else if (!strcmp(prop, "TotalKills")) {
		Players[player].TotalKills = value;
	} else {
		fprintf(stderr, "Invalid field: %s" _C_ prop);
		Exit(1);
	}
}


/* virtual */ void CAnimation_SetPlayerVar::Action(CUnit &unit, int &/*move*/, int /*scale*/) const
{
	Assert(unit.Anim.Anim == this);

	const char *var = this->varStr.c_str();
	const char *arg = this->argStr.c_str();
	int playerId = ParseAnimInt(&unit, this->playerStr.c_str());
	int rop = ParseAnimInt(&unit, this->valueStr.c_str());
	int data = GetPlayerData(playerId, var, arg);

	switch (this->mod) {
		case MOD_ADD:
			data += rop;
			break;
		case MOD_SUB:
			data -= rop;
			break;
		case MOD_MUL:
			data *= rop;
			break;
		case MOD_DIV:
			if (!rop) {
				fprintf(stderr, "Division by zero in AnimationSetPlayerVar\n");
				Exit(1);
			}
			data /= rop;
			break;
		case MOD_MOD:
			if (!rop) {
				fprintf(stderr, "Division by zero in AnimationSetPlayerVar\n");
				Exit(1);
			}
			data %= rop;
			break;
		default:
			data = rop;
	}
	rop = data;
	SetPlayerData(playerId, var, arg, rop);
}

/*
**  s = "player var mod value [arg2]"
*/
/* virtual */ void CAnimation_SetPlayerVar::Init(const char *s)
{
	const std::string str(s);
	const size_t len = str.size();

	size_t begin = 0;
	size_t end = str.find(' ', begin);
	this->playerStr.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
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
	this->argStr.assign(str, begin, end - begin);
}

//@}
