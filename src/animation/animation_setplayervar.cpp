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

#include "animation/animation_setplayervar.h"

#include "animation.h"
#include "stratagus.h"
#include "unit.h"

#include <cstdio>
#include <sstream>

/**
**  Gets the player data.
**
**  @param player  Player number.
**  @param prop    Player's property.
**  @param arg     Additional argument (for resource and unit).
**
**  @return  Returning value (only integer).
*/
int GetPlayerData(int player, std::string_view prop, std::string_view arg)
{
	if (prop == "RaceName") {
		return Players[player].Race;
	} else if (prop == "Resources") {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			ErrorPrint("Invalid resource \"%s\"", arg.data());
			Exit(1);
		}
		return Players[player].Resources[resId] + Players[player].StoredResources[resId];
	} else if (prop == "StoredResources") {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			ErrorPrint("Invalid resource \"%s\"", arg.data());
			Exit(1);
		}
		return Players[player].StoredResources[resId];
	} else if (prop == "MaxResources") {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			ErrorPrint("Invalid resource \"%s\"", arg.data());
			Exit(1);
		}
		return Players[player].MaxResources[resId];
	} else if (prop == "Incomes") {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			ErrorPrint("Invalid resource \"%s\"", arg.data());
			Exit(1);
		}
		return Players[player].Incomes[resId];
	} else if (prop == "UnitTypesCount") {
		return Players[player].UnitTypesCount[UnitTypeByIdent(arg).Slot];
	} else if (prop == "UnitTypesAiActiveCount") {
		return Players[player].UnitTypesAiActiveCount[UnitTypeByIdent(arg).Slot];
	} else if (prop == "AiEnabled") {
		return Players[player].AiEnabled;
	} else if (prop == "TotalNumUnits") {
		return Players[player].GetUnitCount();
	} else if (prop == "NumBuildings") {
		return Players[player].NumBuildings;
	} else if (prop == "Supply") {
		return Players[player].Supply;
	} else if (prop == "Demand") {
		return Players[player].Demand;
	} else if (prop == "UnitLimit") {
		return Players[player].UnitLimit;
	} else if (prop == "BuildingLimit") {
		return Players[player].BuildingLimit;
	} else if (prop == "TotalUnitLimit") {
		return Players[player].TotalUnitLimit;
	} else if (prop == "Score") {
		return Players[player].Score;
	} else if (prop == "TotalUnits") {
		return Players[player].TotalUnits;
	} else if (prop == "TotalBuildings") {
		return Players[player].TotalBuildings;
	} else if (prop == "TotalResources") {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			ErrorPrint("Invalid resource \"%s\"", arg.data());
			Exit(1);
		}
		return Players[player].TotalResources[resId];
	} else if (prop == "TotalRazings") {
		return Players[player].TotalRazings;
	} else if (prop == "TotalKills") {
		return Players[player].TotalKills;
	} else {
		ErrorPrint("Invalid field: %s", prop.data());
		Exit(1);
	}
	return 0;
}

/**
**  Sets the player data.
*/
static void SetPlayerData(const int player, std::string_view prop, std::string_view arg, int value)
{
	if (prop == "RaceName") {
		Players[player].Race = value;
	} else if (prop == "Resources") {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			ErrorPrint("Invalid resource \"%s\"", arg.data());
			Exit(1);
		}
		Players[player].SetResource(resId, value, STORE_BOTH);
	} else if (prop == "StoredResources") {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			ErrorPrint("Invalid resource \"%s\"", arg.data());
			Exit(1);
		}
		Players[player].SetResource(resId, value, STORE_BUILDING);
	} else if (prop == "UnitLimit") {
		Players[player].UnitLimit = value;
	} else if (prop == "BuildingLimit") {
		Players[player].BuildingLimit = value;
	} else if (prop == "TotalUnitLimit") {
		Players[player].TotalUnitLimit = value;
	} else if (prop == "Score") {
		Players[player].Score = value;
	} else if (prop == "TotalUnits") {
		Players[player].TotalUnits = value;
	} else if (prop == "TotalBuildings") {
		Players[player].TotalBuildings = value;
	} else if (prop == "TotalResources") {
		const int resId = GetResourceIdByName(arg);
		if (resId == -1) {
			ErrorPrint("Invalid resource \"%s\"", arg.data());
			Exit(1);
		}
		Players[player].TotalResources[resId] = value;
	} else if (prop == "TotalRazings") {
		Players[player].TotalRazings = value;
	} else if (prop == "TotalKills") {
		Players[player].TotalKills = value;
	} else {
		ErrorPrint("Invalid field: %s", prop.data());
		Exit(1);
	}
}

void CAnimation_SetPlayerVar::Action(CUnit &unit,
                                     int & /*move*/,
                                     int /*scale*/) const /* override */
{
	Assert(unit.Anim.CurrAnim);
	Assert((*unit.Anim.CurrAnim)[unit.Anim.Anim].get() == this);

	const int playerId = ParseAnimInt(unit, this->playerStr);
	const int rop = ParseAnimInt(unit, this->valueStr);
	int data = GetPlayerData(playerId, this->varStr, this->argStr);

	modifyValue(this->mod, data, rop);
	SetPlayerData(playerId, this->varStr, this->argStr, data);
}

/*
**  s = "player var mod value [arg2]"
*/
void CAnimation_SetPlayerVar::Init(std::string_view s, lua_State *) /* override */
{
	std::istringstream is{std::string(s)};

	std::string modStr;
	is >> this->playerStr >> this->varStr >> modStr >> this->valueStr >> this->argStr;
	this->mod = toSetVar_ModifyTypes(modStr);
}

//@}
