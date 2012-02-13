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
/**@name script_ai.cpp - The AI ccl functions. */
//
//      (c) Copyright 2000-2007 by Lutz Sammer, Ludovic Pollet,
//                                 and Jimmy Salmon
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unit_manager.h"
#include "unittype.h"
#include "upgrade.h"
#include "script.h"
#include "ai.h"
#include "pathfinder.h"
#include "ai_local.h"
#include "player.h"
#include "interface.h"

/**
**  Insert new unit-type element.
**
**  @param table  Table with elements.
**  @param n      Index to insert new into table
**  @param base   Base type to insert into table.
*/
static void AiHelperInsert(std::vector<std::vector<CUnitType *> > &table,
	unsigned int n, CUnitType *base)
{
	Assert(base != NULL);

	if (n >= table.size()) {
		table.resize(n + 1);
	}

	// Look if already known
	std::vector<CUnitType *>::const_iterator i;
	for (i = table[n].begin(); i != table[n].end(); ++i) {
		if (*i == base) {
			return;
		}
	}
	table[n].push_back(base);
}

/**
**  Transform list of unit separed with coma to a true list.
*/
static std::vector<CUnitType *> getUnitTypeFromString(const std::string& list)
{
	std::vector<CUnitType *> res;

	if (list == "*") {
		return UnitTypes;
	}
	int begin = 1;
	int end = 0;
	std::string unitName;
	end = list.find(",", begin);
	while (end != -1) {
		unitName = list.substr(begin, end - begin);
		begin = end + 1;
		end = list.find(",", begin);
		if (!unitName.empty()) {
			Assert(unitName[0] != ',');
			res.push_back(UnitTypeByIdent(unitName.c_str()));
		}
	}
	return res;
}

/**
**  Get list of unittype which can be repared.
*/
static std::vector<CUnitType *> getReparableUnits()
{
	std::vector<CUnitType *> res;

	for (std::vector<CUnitType *>::const_iterator i = UnitTypes.begin(); i != UnitTypes.end(); ++i) {
		CUnitType *type = *i;

		if (type->RepairHP > 0) {
			res.push_back(type);
		}
	}
	return res;
}

/**
**  Get sorted list of unittype with Supply not null.
**
**  @note Better (supply / cost) first.
*/
static std::vector<CUnitType *> getSupplyUnits()
{
	std::vector<CUnitType *> res;
	std::vector<CUnitType *> sorted_res;


	for (std::vector<CUnitType *>::const_iterator i = UnitTypes.begin(); i != UnitTypes.end(); ++i) {
		CUnitType *type = *i;

		if (type->Supply > 0) {
			res.push_back(type);
		}
	}
	// Now, sort them, best first.
	while (!res.empty()) {
		float bestscore;
		CUnitType *besttype;

		bestscore = 0;
		for (std::vector<CUnitType *>::const_iterator i = res.begin(); i != res.end(); ++i) {
			CUnitType *type = *i;
			float score;
			unsigned int cost = 0;

			for (unsigned j = 0; j < MaxCosts; ++j) {
				cost += type->_Costs[j];
			}
			score = ((float) type->Supply) / cost;
			if (score > bestscore) {
				bestscore = score;
				besttype = type;
			}
		}
		sorted_res.push_back(besttype);
		for (std::vector<CUnitType *>::iterator i = res.begin(); i != res.end(); ++i) {
			if (*i == besttype) {
				i = res.erase(i);
				break;
			}
		}
	}
	return sorted_res;
}

/**
**  Get sorted list of unittype with CanHarvest not null.
**
**  @note Better (MaxOnBoard / cost) first.
*/
static std::vector<CUnitType *> getRefineryUnits()
{
	std::vector<CUnitType *> res;

	for (std::vector<CUnitType *>::const_iterator i = UnitTypes.begin(); i != UnitTypes.end(); ++i) {
		CUnitType *type = *i;

		if (type->GivesResource > 0 && type->CanHarvest) {
			res.push_back(type);
		}
	}
#if 0
	std::vector<CUnitType *> sorted_res;
	// Now, sort them, best first.
	while (!res.empty()) {
		float bestscore;
		CUnitType *besttype;

		bestscore = 0;
		for (std::vector<CUnitType *>::const_iterator i = res.begin(); i != res.end(); ++i) {
			CUnitType *type = *i;
			float score;
			unsigned int cost = 0;

			for (unsigned j = 0; j < MaxCosts; ++j) {
				cost += type->_Costs[j];
			}
			score = ((float) type->MaxOnBoard) / cost;
			if (score > bestscore) {
				bestscore = score;
				besttype = type;
			}
		}
		sorted_res.push_back(besttype);
		for (std::vector<CUnitType *>::iterator i = res.begin(); i != res.end(); ++i) {
			if (*i == besttype) {
				i = res.erase(i);
				break;
			}
		}
	}
	return sorted_res;
#else
	return res;
#endif

}



/**
**  Init AiHelper.
**
**  @param aiHelper  variable to initialise.
**
**  @todo missing Equiv initialisation.
*/
static void InitAiHelper(AiHelper &aiHelper)
{
	extern std::vector<ButtonAction *> UnitButtonTable;

	std::vector<CUnitType *> reparableUnits = getReparableUnits();
	std::vector<CUnitType *> supplyUnits = getSupplyUnits();
	std::vector<CUnitType *> mineUnits = getRefineryUnits();

	for (std::vector<CUnitType *>::const_iterator i = supplyUnits.begin(); i != supplyUnits.end(); ++i) {
		AiHelperInsert(aiHelper.UnitLimit, 0, *i);
	}

	for (int i = 1; i < MaxCosts; ++i) {
		for (std::vector<CUnitType *>::const_iterator j = mineUnits.begin(); j != mineUnits.end(); ++j) {
			if ((*j)->GivesResource == i) {
				/* HACK : we can't mine TIME then use 0 as 1 */
				AiHelperInsert(aiHelper.Refinery, i - 1, *j);
			}
		}

		for (std::vector<CUnitType *>::const_iterator d = UnitTypes.begin();
			d != UnitTypes.end(); ++d) {
			CUnitType *type = *d;

			if (type->CanStore[i] > 0) {
				/* HACK : we can't store TIME then use 0 as 1 */
				AiHelperInsert(aiHelper.Depots, i - 1, *d);
			}
		}

	}

	for (int i = 0; i < (int)UnitButtonTable.size(); ++i) {
		const ButtonAction &button = *UnitButtonTable[i];
		const std::vector<CUnitType *> &unitmask = getUnitTypeFromString(button.UnitMask);

		switch (button.Action) {
			case ButtonRepair :
				for (std::vector<CUnitType *>::const_iterator j = unitmask.begin(); j != unitmask.end(); ++j) {
					for (std::vector<CUnitType *>::const_iterator k = reparableUnits.begin(); k != reparableUnits.end(); ++k) {
						AiHelperInsert(aiHelper.Repair, (*k)->Slot, *j);
					}
				}
				break;
			case ButtonBuild:
			{
				CUnitType *buildingType = UnitTypeByIdent(button.ValueStr);

				for (std::vector<CUnitType *>::const_iterator j = unitmask.begin(); j != unitmask.end(); ++j) {
					AiHelperInsert(aiHelper.Build, buildingType->Slot, (*j));
				}
				break;
			}
			case ButtonTrain :
			{
				CUnitType *trainingType = UnitTypeByIdent(button.ValueStr);

				for (std::vector<CUnitType *>::const_iterator j = unitmask.begin(); j != unitmask.end(); ++j) {
					AiHelperInsert(aiHelper.Train, trainingType->Slot, (*j));
				}
				break;
			}
			case ButtonUpgradeTo :
			{
				CUnitType *upgradeToType = UnitTypeByIdent(button.ValueStr);

				for (std::vector<CUnitType *>::const_iterator j = unitmask.begin(); j != unitmask.end(); ++j) {
					AiHelperInsert(aiHelper.Upgrade, upgradeToType->Slot, *j);
				}
				break;
			}
			case ButtonResearch :
			{
				int researchId = UpgradeIdByIdent(button.ValueStr);

				for (std::vector<CUnitType *>::const_iterator j = unitmask.begin(); j != unitmask.end(); ++j) {
					AiHelperInsert(aiHelper.Research, researchId, *j);
				}
				break;
			}
			default:
				break;
		}
	}
}

/**
**  Define helper for AI.
**
**  @param l  Lua state.
**
**  @todo  FIXME: the first unit could be a list see ../doc/ccl/ai.html
*/
static int CclDefineAiHelper(lua_State *l)
{
	const char *value;
	int what;
	CUnitType *base;
	CUnitType *type = NULL;
	int args;
	int j;
	int subargs;
	int k;

	InitAiHelper(AiHelpers);

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			LuaError(l, "incorrect argument");
		}
		subargs = lua_objlen(l, j + 1);
		k = 0;
		lua_rawgeti(l, j + 1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;

		//
		// Type build,train,research/upgrade.
		//
		if (!strcmp(value, "build")) {
			what = -1;
		} else if (!strcmp(value, "train")) {
			what = -1;
		} else if (!strcmp(value, "upgrade")) {
			what = -1;
		} else if (!strcmp(value, "research")) {
			what = -1;
		} else if (!strcmp(value, "unit-limit")) {
			what = -1;
		} else if (!strcmp(value, "unit-equiv")) {
			what = 5;
		} else if (!strcmp(value, "repair")) {
			what = -1;
		} else {
			LuaError(l, "unknown tag: %s" _C_ value);
			what = -1;
		}
		if (what == -1) {
			continue;
		}
		//
		// Get the base unit type, which could handle the action.
		//

		// FIXME: support value as list!
		lua_rawgeti(l, j + 1, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;
		base = UnitTypeByIdent(value);
		if (!base) {
			LuaError(l, "unknown unittype: %s" _C_ value);
		}

		//
		// Get the unit types, which could be produced
		//
		for (; k < subargs; ++k) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			type = UnitTypeByIdent(value);
			if (!type) {
				LuaError(l, "unknown unittype: %s" _C_ value);
			}
			AiHelperInsert(AiHelpers.Equiv, base->Slot, type);
			AiNewUnitTypeEquiv(base, type);
		}
	}
	return 0;
}

/**
**  Define an AI engine.
**
**  @param l  Lua state.
**
**  @return   FIXME: docu
*/
static int CclDefineAi(lua_State *l)
{
	const char *value;
	CAiType *aitype;
#ifdef DEBUG
	const CAiType *ait;
#endif

	LuaCheckArgs(l, 4);
	if (!lua_isfunction(l, 4)) {
		LuaError(l, "incorrect argument");
	}

	aitype = new CAiType;
	AiTypes.insert(AiTypes.begin(), aitype);

	//
	// AI Name
	//
	aitype->Name = LuaToString(l, 1);

#ifdef DEBUG
	for (int i = 1; i < (int)AiTypes.size(); ++i) {
		ait = AiTypes[i];
		if (aitype->Name == ait->Name) {
			DebugPrint("Warning two or more AI's with the same name '%s'\n" _C_ ait->Name.c_str());
		}
	}
#endif

	//
	// AI Race
	//
	value = LuaToString(l, 2);
	if (*value != '*') {
		aitype->Race = value;
	} else {
		aitype->Race.clear();
	}

	//
	// AI Class
	//
	aitype->Class = LuaToString(l, 3);

	//
	// AI Script
	//
	lua_pushstring(l, "_ai_scripts_");
	lua_gettable(l, LUA_GLOBALSINDEX);
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_pushstring(l, "_ai_scripts_");
		lua_newtable(l);
		lua_settable(l, LUA_GLOBALSINDEX);
		lua_pushstring(l, "_ai_scripts_");
		lua_gettable(l, LUA_GLOBALSINDEX);
	}
	aitype->Script = aitype->Name + aitype->Race + aitype->Class;
	lua_pushstring(l, aitype->Script.c_str());
	lua_pushvalue(l, 4);
	lua_rawset(l, 5);
	lua_pop(l, 1);

	return 0;
}

/*----------------------------------------------------------------------------
--  AI script functions
----------------------------------------------------------------------------*/

/**
**  Append unit-type to request table.
**
**  @param type   Unit-type to be appended.
**  @param count  How many unit-types to build.
*/
static void InsertUnitTypeRequests(CUnitType *type, int count)
{
	AiRequestType ait;

	ait.Type = type;
	ait.Count = count;

	AiPlayer->UnitTypeRequests.push_back(ait);
}

/**
**  Find unit-type in request table.
**
**  @param type  Unit-type to be found.
*/
static AiRequestType *FindInUnitTypeRequests(const CUnitType *type)
{
	int i;
	int n;

	n = AiPlayer->UnitTypeRequests.size();
	for (i = 0; i < n; ++i) {
		if (AiPlayer->UnitTypeRequests[i].Type == type) {
			return &AiPlayer->UnitTypeRequests[i];
		}
	}
	return NULL;
}

/**
**  Find unit-type in upgrade-to table.
**
**  @param type  Unit-type to be found.
*/
static int FindInUpgradeToRequests(const CUnitType *type)
{
	int i;
	int n;

	n = AiPlayer->UpgradeToRequests.size();
	for (i = 0; i < n; ++i) {
		if (AiPlayer->UpgradeToRequests[i] == type) {
			return 1;
		}
	}
	return 0;
}

/**
**  Append unit-type to request table.
**
**  @param type  Unit-type to be appended.
*/
static void InsertUpgradeToRequests(CUnitType *type)
{
	AiPlayer->UpgradeToRequests.push_back(type);
}

/**
**  Append unit-type to request table.
**
**  @param upgrade  Upgrade to be appended.
*/
static void InsertResearchRequests(CUpgrade *upgrade)
{
	AiPlayer->ResearchRequests.push_back(upgrade);
}

//----------------------------------------------------------------------------

/**
**  Get the race of the current AI player.
**
**  @param l  Lua state.
*/
static int CclAiGetRace(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushstring(l, PlayerRaces.Name[AiPlayer->Player->Race].c_str());
	return 1;
}

/**
**  Get the number of cycles to sleep.
**
**  @param l  Lua state
**
**  @return   Number of return values
*/
static int CclAiGetSleepCycles(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushnumber(l, AiSleepCycles);
	return 1;
}

//----------------------------------------------------------------------------

/**
**  Set debugging flag of AI script
**
**  @param l  Lua state
**
**  @return   Number of return values
*/
static int CclAiDebug(lua_State *l)
{
	LuaCheckArgs(l, 1);
	AiPlayer->ScriptDebug = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Activate AI debugging for the given player(s)
**  Player can be a number for a specific player
**    "self" for current human player (ai me)
**    "none" to disable
**
**  @param l  Lua State
**
**  @return   Number of return values
*/
static int CclAiDebugPlayer(lua_State *l)
{
	const char *item;
	int playerid;
	int args;
	int j;

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		if (lua_isstring(l, j + 1)) {
			item = LuaToString(l, j + 1);
		} else {
			item = NULL;
		}

		if (item && !strcmp(item, "none")) {
			for (playerid = 0; playerid < NumPlayers; ++playerid) {
				if (!Players[playerid].AiEnabled || !Players[playerid].Ai) {
					continue;
				}
				((PlayerAi *)Players[playerid].Ai)->ScriptDebug = 0;
			}
		} else {
			if (item && !strcmp(item, "self")) {
				if (!ThisPlayer) {
					continue;
				}
				playerid = ThisPlayer->Index;
			} else {
				playerid = LuaToNumber(l, j + 1);
			}

			if (!Players[playerid].AiEnabled || !Players[playerid].Ai) {
				continue;
			}
			((PlayerAi *)Players[playerid].Ai)->ScriptDebug = 1;
		}
	}
	return 0;
}

/**
**  Need a unit.
**
**  @param l  Lua state.
**
**  @return   Number of return values
*/
static int CclAiNeed(lua_State *l)
{
	LuaCheckArgs(l, 1);
	InsertUnitTypeRequests(CclGetUnitType(l), 1);

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Set the number of units.
**
**  @param l  Lua state
**
**  @return   Number of return values
*/
static int CclAiSet(lua_State *l)
{
	AiRequestType *autt;
	CUnitType *type;

	LuaCheckArgs(l, 2);
	lua_pushvalue(l, 1);
	type = CclGetUnitType(l);
	lua_pop(l, 1);
	if ((autt = FindInUnitTypeRequests(type))) {
		autt->Count = LuaToNumber(l, 2);
		// FIXME: 0 should remove it.
	} else {
		InsertUnitTypeRequests(type, LuaToNumber(l, 2));
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Wait for a unit.
**
**  @param l  Lua State.
**
**  @return   Number of return values
*/
static int CclAiWait(lua_State *l)
{
	const AiRequestType *autt;
	const CUnitType *type;
	const int *unit_types_count;
	int j;
	unsigned int n;

	LuaCheckArgs(l, 1);
	type = CclGetUnitType(l);
	unit_types_count = AiPlayer->Player->UnitTypesCount;
	if (!(autt = FindInUnitTypeRequests(type))) {
		//
		// Look if we have this unit-type.
		//
		if (unit_types_count[type->Slot]) {
			lua_pushboolean(l, 0);
			return 1;
		}

		//
		// Look if we have equivalent unit-types.
		//
		if (type->Slot < (int)AiHelpers.Equiv.size()) {
			for (j = 0; j < (int)AiHelpers.Equiv[type->Slot].size(); ++j) {
				if (unit_types_count[AiHelpers.Equiv[type->Slot][j]->Slot]) {
					lua_pushboolean(l, 0);
					return 1;
				}
			}
		}
		//
		// Look if we have an upgrade-to request.
		//
		if (FindInUpgradeToRequests(type)) {
			lua_pushboolean(l, 1);
			return 1;
		}
		DebugPrint("Broken? waiting on %s which wasn't requested.\n" _C_ type->Ident.c_str());
		lua_pushboolean(l, 0);
		return 1;
	}
	//
	// Add equivalent units
	//
	n = unit_types_count[type->Slot];
	if (type->Slot < (int)AiHelpers.Equiv.size()) {
		for (j = 0; j < (int)AiHelpers.Equiv[type->Slot].size(); ++j) {
			n += unit_types_count[AiHelpers.Equiv[type->Slot][j]->Slot];
		}
	}
	// units available?

	if (n >= autt->Count) {
		lua_pushboolean(l, 0);
		return 1;
	}

	lua_pushboolean(l, 1);
	return 1;
}

/**
**  Define a force, a groups of units.
**
**  @param l  Lua state.
*/
static int CclAiForce(lua_State *l)
{
	AiUnitType *aiut;
	CUnitType *type;
	int count;
	int force;
	int args;
	int j;
	int i;

	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}
	force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCES) {
		LuaError(l, "Force out of range: %d" _C_ force);
	}
	AiForce &aiforce = AiPlayer->Force[AiPlayer->Force.getScriptForce(force)];

	args = lua_objlen(l, 2);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, 2, j + 1);
		type = CclGetUnitType(l);
		lua_pop(l, 1);
		++j;
		lua_rawgeti(l, 2, j + 1);
		count = LuaToNumber(l, -1);
		lua_pop(l, 1);

		if (!type) { // bulletproof
			continue;
		}

		// Use the equivalent unittype.
		type = UnitTypes[UnitTypeEquivs[type->Slot]];

		//
		// Look if already in force.
		//
		for (i = 0; i < (int)aiforce.UnitTypes.size(); ++i) {
			aiut = &aiforce.UnitTypes[i];
			if (aiut->Type->Slot == type->Slot) { // found
				if (count) {
					aiut->Want = count;
				} else {
					aiforce.UnitTypes.erase(aiforce.UnitTypes.begin() + i);
				}
				break;
			}
		}

		//
		// New type append it.
		//
		if (i == (int)aiforce.UnitTypes.size()) {
			AiUnitType newaiut;
			newaiut.Want = count;
			newaiut.Type = type;
			aiforce.UnitTypes.push_back(newaiut);
		}
	}

	AiAssignFreeUnitsToForce();

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Define the role of a force.
**
**  @param l  Lua state.
*/
static int CclAiForceRole(lua_State *l)
{

	LuaCheckArgs(l, 2);
	int force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCES) {
		LuaError(l, "Force %i out of range" _C_ force);
	}

	AiForce &aiforce = AiPlayer->Force[AiPlayer->Force.getScriptForce(force)];

	const char *flag = LuaToString(l, 2);
	if (!strcmp(flag, "attack")) {
		aiforce.Role = AiForceRoleAttack;
	} else if (!strcmp(flag, "defend")) {
		aiforce.Role = AiForceRoleDefend;
	} else {
		LuaError(l, "Unknown force role '%s'" _C_ flag);
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Check if a force ready.
**
**  @param l  Lua state.
*/
static int CclAiCheckForce(lua_State *l)
{

	LuaCheckArgs(l, 1);
	int force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCES) {
		lua_pushfstring(l, "Force out of range: %d", force);
	}
	if (AiPlayer->Force[AiPlayer->Force.getScriptForce(force)].Completed) {
		lua_pushboolean(l, 1);
		return 1;
	}
	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Wait for a force ready.
**
**  @param l  Lua state.
*/
static int CclAiWaitForce(lua_State *l)
{

	LuaCheckArgs(l, 1);
	int force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCES) {
		lua_pushfstring(l, "Force out of range: %d", force);
	}
	if (AiPlayer->Force[AiPlayer->Force.getScriptForce(force)].Completed) {
		lua_pushboolean(l, 0);
		return 1;
	}

#if 0
	// Debuging
	AiCleanForces();
	Assert(!AiPlayer->Force.getScriptForce(f).Completed);
#endif

	lua_pushboolean(l, 1);
	return 1;
}

/**
**  Attack with force.
**
**  @param l  Lua state.
*/
static int CclAiAttackWithForce(lua_State *l)
{
	int force;

	LuaCheckArgs(l, 1);
	force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCES) {
		LuaError(l, "Force out of range: %d" _C_ force);
	}

	AiAttackWithForce(AiPlayer->Force.getScriptForce(force));

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Wait for a forces ready.
**
**  @param l  Lua state.
*/
static int CclAiWaitForces(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	int force, args = lua_objlen(l, 1);
	for (int i = 0; i < args; ++i) {
		lua_rawgeti(l, 1, i + 1);
		force = LuaToNumber(l, -1);
		lua_pop(l, 1);
		if (force < 0 || force >= AI_MAX_FORCES) {
			lua_pushfstring(l, "Force out of range: %d", force);
		}
		if (!AiPlayer->Force[AiPlayer->Force.getScriptForce(force)].Completed) {
			lua_pushboolean(l, 1);
			return 1;
		}
	}
	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Attack with forces.
**
**  @param l  Lua state.
*/
static int CclAiAttackWithForces(lua_State *l)
{
	int Forces[AI_MAX_FORCES + 1];

	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	int force, args = lua_objlen(l, 1);
	for (int i = 0; i < args; ++i) {
		lua_rawgeti(l, 1, i + 1);
		force = LuaToNumber(l, -1);
		lua_pop(l, 1);
		if (force < 0 || force >= AI_MAX_FORCES) {
			lua_pushfstring(l, "Force out of range: %d", force);
		}
		Forces[i] = AiPlayer->Force.getScriptForce(force);
	}
	Forces[args] = -1;
	AiAttackWithForces(Forces);
	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Sleep n cycles.
**
**  @param l  Lua state.
*/
static int CclAiSleep(lua_State *l)
{
	int i;

	LuaCheckArgs(l, 1);
	i = LuaToNumber(l, 1);
	if (AiPlayer->SleepCycles || i == 0) {
		if (AiPlayer->SleepCycles < GameCycle) {
			AiPlayer->SleepCycles = 0;
			lua_pushboolean(l, 0);
			return 1;
		}
	} else {
		AiPlayer->SleepCycles = GameCycle + i;
	}

	lua_pushboolean(l, 1);
	return 1;
}

/**
**  Research an upgrade.
**
**  @param l  Lua state.
*/
static int CclAiResearch(lua_State *l)
{
	const char *str;
	CUpgrade *upgrade;

	LuaCheckArgs(l, 1);
	if ((str = LuaToString(l, 1))) {
		upgrade = CUpgrade::Get(str);
	} else {
		LuaError(l, "Upgrade needed");
		upgrade = 0;
	}

	InsertResearchRequests(upgrade);

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Upgrade an unit to an new unit-type.
**
**  @param l  Lua state.
*/
static int CclAiUpgradeTo(lua_State *l)
{
	CUnitType *type;

	LuaCheckArgs(l, 1);
	type = CclGetUnitType(l);
	InsertUpgradeToRequests(type);

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Return the player of the running AI.
**
**  @param l  Lua state.
**
**  @return  Player number of the AI.
*/
static int CclAiPlayer(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushnumber(l, AiPlayer->Player->Index);
	return 1;
}

/**
**  Set AI player resource reserve.
**
**  @param l  Lua state.
**
**  @return     Old resource vector
*/
static int CclAiSetReserve(lua_State *l)
{
	int i;

	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	lua_newtable(l);
	for (i = 0; i < MaxCosts; ++i) {
		lua_pushnumber(l, AiPlayer->Reserve[i]);
		lua_rawseti(l, -2, i + 1);
	}
	for (i = 0; i < MaxCosts; ++i) {
		lua_rawgeti(l, 1, i + 1);
		AiPlayer->Reserve[i] = LuaToNumber(l, -1);
		lua_pop(l, 1);
	}
	return 1;
}

/**
**  Set AI player resource collect percent.
**
**  @param l  Lua state.
*/
static int CclAiSetCollect(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	for (int i = 0; i < MaxCosts; ++i) {
		lua_rawgeti(l, 1, i + 1);
		AiPlayer->Collect[i] = LuaToNumber(l, -1);
		lua_pop(l, 1);
	}
	return 0;
}

/**
**  Dump some AI debug informations.
**
**  @param l  Lua state.
*/
static int CclAiDump(lua_State *l)
{
	int i;
	int n;
	const AiUnitType *aut;
	const AiBuildQueue *queue;

	LuaCheckArgs(l, 0);
	//
	// Script
	//
	printf("------\n");
	for (i = 0; i < MaxCosts; ++i) {
		printf("%s(%4d) ", DefaultResourceNames[i].c_str(), AiPlayer->Player->Resources[i]);
	}
	printf("\n");
	printf("%d:", AiPlayer->Player->Index);
#if 0
	gh_display(gh_car(AiPlayer->Script));
#endif
	//
	// Requests
	//
	n = (int)AiPlayer->UnitTypeRequests.size();
	printf("UnitTypeRequests(%d):\n", n);
	for (i = 0; i < n; ++i) {
		printf("%s ", AiPlayer->UnitTypeRequests[i].Type->Ident.c_str());
	}
	printf("\n");
	n = (int)AiPlayer->UpgradeToRequests.size();
	printf("UpgradeToRequests(%d):\n", n);
	for (i = 0; i < n; ++i) {
		printf("%s ", AiPlayer->UpgradeToRequests[i]->Ident.c_str());
	}
	printf("\n");
	n = (int)AiPlayer->ResearchRequests.size();
	printf("ResearchRequests(%d):\n", n);
	for (i = 0; i < n; ++i) {
		printf("%s ", AiPlayer->ResearchRequests[i]->Ident.c_str());
	}
	printf("\n");

	//
	// Building queue
	//
	printf("Building queue:\n");
	for (i = 0; i < (int)AiPlayer->UnitTypeBuilt.size(); ++i) {
		queue = &AiPlayer->UnitTypeBuilt[i];
		printf("%s(%d/%d) ", queue->Type->Ident.c_str(), queue->Made, queue->Want);
	}
	printf("\n");

	//
	// PrintForce
	//
	for (i = 0; i < (int)AiPlayer->Force.Size(); ++i) {
		printf("Force(%d%s%s):\n", i,
			AiPlayer->Force[i].Completed ? ",complete" : ",recruit",
			AiPlayer->Force[i].Attacking ? ",attack" : "");
		for (int j = 0; j < (int)AiPlayer->Force[i].UnitTypes.size(); ++j) {
			aut = &AiPlayer->Force[i].UnitTypes[j];
			printf("%s(%d) ", aut->Type->Ident.c_str(), aut->Want);
		}
		printf("\n");
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Get the default resource number
**
**  @param name  Resource name.
**
**  @return      The number of the resource in DefaultResourceNames
*/
static int DefaultResourceNumber(const char *name)
{
	for (unsigned int i = 0; i < MaxCosts; ++i) {
		if (DefaultResourceNames[i].compare(name) == 0) {
			return i;
		}
	}
	// Resource not found, should never happen
	Assert(0);
	return -1;
}

/**
**  Parse AiBuildQueue builing list
**
**  @param l     Lua state.
**  @param ai  PlayerAi pointer which should be filled with the data.
*/
static void CclParseBuildQueue(lua_State *l, PlayerAi *ai, int offset)
{
	if (!lua_istable(l, offset)) {
		LuaError(l, "incorrect argument");
	}

	const char *value;
	int made;
	int want;
	int x = -1,y = -1;

	const int args = lua_objlen(l, offset);
	for (int k = 0; k < args; ++k) {
		lua_rawgeti(l, offset, k + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++k;

		if (!strcmp(value, "onpos")) {
				lua_rawgeti(l, offset, k + 1);
				x = LuaToNumber(l, -1);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, offset, k + 1);
				y = LuaToNumber(l, -1);
				lua_pop(l, 1);
		} else {

				//lua_rawgeti(l, j + 1, k + 1);
				//ident = LuaToString(l, -1);
				//lua_pop(l, 1);
				//++k;
				lua_rawgeti(l, offset, k + 1);
				made = LuaToNumber(l, -1);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, offset, k + 1);
				want = LuaToNumber(l, -1);
				lua_pop(l, 1);

				AiBuildQueue queue;
				queue.Type = UnitTypeByIdent(value);
				queue.Want = want;
				queue.Made = made;
				queue.X = x;
				queue.Y = y;

				ai->UnitTypeBuilt.push_back(queue);
				x = -1;
				y = -1;
		}
	}
}


/**
** Define an AI player.
**
**  @param l  Lua state.
*/
static int CclDefineAiPlayer(lua_State *l)
{
	const char *value;
	int i;
	PlayerAi *ai;
	int args;
	int j;
	int subargs;
	int k;

	args = lua_gettop(l);
	j = 0;

	i = LuaToNumber(l, j + 1);
	++j;

	Assert(i >= 0 && i <= PlayerMax);
	DebugPrint("%p %d\n" _C_ (void *)Players[i].Ai _C_ Players[i].AiEnabled );
	// FIXME: lose this:
	// Assert(!Players[i].Ai && Players[i].AiEnabled);

	ai = Players[i].Ai = new PlayerAi;
	ai->Player = &Players[i];

	//
	// Parse the list: (still everything could be changed!)
	//
	for (; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "ai-type")) {
			CAiType *ait = NULL;

			value = LuaToString(l, j + 1);
			for (k = 0; k < (int)AiTypes.size(); ++k) {
				ait = AiTypes[k];
				if (ait->Name == value) {
					break;
				}
			}
			if (k == (int)AiTypes.size()) {
				lua_pushfstring(l, "ai-type not found: %s", value);
			}
			ai->AiType = ait;
			ai->Script = ait->Script;
		} else if (!strcmp(value, "script")) {
			ai->Script = LuaToString(l, j + 1);
		} else if (!strcmp(value, "script-debug")) {
			ai->ScriptDebug = LuaToBoolean(l, j + 1);
		} else if (!strcmp(value, "sleep-cycles")) {
			ai->SleepCycles = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "force")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, j + 1);
			k = 0;
			lua_rawgeti(l, j + 1, k + 1);
			i = LuaToNumber(l, -1);
			lua_pop(l, 1);
			++k;
			for (; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "complete")) {
					ai->Force[i].Completed = true;
					--k;
				} else if (!strcmp(value, "recruit")) {
					ai->Force[i].Completed = false;
					--k;
				} else if (!strcmp(value, "attack")) {
					ai->Force[i].Attacking = true;
					--k;
				} else if (!strcmp(value, "defend")) {
					ai->Force[i].Defending = true;
					--k;
				} else if (!strcmp(value, "role")) {
					lua_rawgeti(l, j + 1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					if (!strcmp(value, "attack")) {
						ai->Force[i].Role = AiForceRoleAttack;
					} else if (!strcmp(value, "defend")) {
						ai->Force[i].Role = AiForceRoleDefend;
					} else {
						LuaError(l, "Unsupported force tag: %s" _C_ value);
					}
				} else if (!strcmp(value, "types")) {
					int subsubargs;
					int subk;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					subsubargs = lua_objlen(l, -1);
					for (subk = 0; subk < subsubargs; ++subk) {
						int num;
						const char *ident;
						AiUnitType queue;

						lua_rawgeti(l, -1, subk + 1);
						num = LuaToNumber(l, -1);
						lua_pop(l, 1);
						++subk;
						lua_rawgeti(l, -1, subk + 1);
						ident = LuaToString(l, -1);
						lua_pop(l, 1);
						queue.Want = num;
						queue.Type = UnitTypeByIdent(ident);
						ai->Force[i].UnitTypes.push_back(queue);
					}
					lua_pop(l, 1);
				} else if (!strcmp(value, "units")) {
					int subsubargs;
					int subk;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					subsubargs = lua_objlen(l, -1);
					for (subk = 0; subk < subsubargs; ++subk) {
						lua_rawgeti(l, -1, subk + 1);
						const int num = LuaToNumber(l, -1);
						lua_pop(l, 1);
						++subk;
#if 0
						lua_rawgeti(l, -1, subk + 1);
						const char *ident = LuaToString(l, -1);
						UNUSED(ident);
						lua_pop(l, 1);
#endif
						ai->Force[i].Units.Insert(UnitSlots[num]);
					}
					lua_pop(l, 1);
				} else if (!strcmp(value, "state")) {
					lua_rawgeti(l, j + 1, k + 1);
					ai->Force[i].State = AiForceAttackingState(LuaToNumber(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "goalx")) {
					lua_rawgeti(l, j + 1, k + 1);
					ai->Force[i].GoalPos.x = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "goaly")) {
					lua_rawgeti(l, j + 1, k + 1);
					ai->Force[i].GoalPos.y = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "must-transport")) {
					// Keep for backward compatibility
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "reserve")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				const char *type;
				int num;

				lua_rawgeti(l, j + 1, k + 1);
				type = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, j + 1, k + 1);
				num = LuaToNumber(l, -1);
				lua_pop(l, 1);
				ai->Reserve[DefaultResourceNumber(type)] = num;
			}
		} else if (!strcmp(value, "used")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				const char *type;
				int num;

				lua_rawgeti(l, j + 1, k + 1);
				type = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, j + 1, k + 1);
				num = LuaToNumber(l, -1);
				lua_pop(l, 1);
				ai->Used[DefaultResourceNumber(type)] = num;
			}
		} else if (!strcmp(value, "needed")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				const char *type;
				int num;

				lua_rawgeti(l, j + 1, k + 1);
				type = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, j + 1, k + 1);
				num = LuaToNumber(l, -1);
				lua_pop(l, 1);
				ai->Needed[DefaultResourceNumber(type)] = num;
			}
		} else if (!strcmp(value, "collect")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				const char *type;
				int num;

				lua_rawgeti(l, j + 1, k + 1);
				type = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, j + 1, k + 1);
				num = LuaToNumber(l, -1);
				lua_pop(l, 1);
				ai->Collect[DefaultResourceNumber(type)] = num;
			}
		} else if (!strcmp(value, "need-mask")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				const char *type;

				lua_rawgeti(l, j + 1, k + 1);
				type = LuaToString(l, -1);
				lua_pop(l, 1);
				ai->NeededMask |= (1 << DefaultResourceNumber(type));
			}
		} else if (!strcmp(value, "need-supply")) {
			ai->NeedSupply = true;
			--j;
		} else if (!strcmp(value, "exploration")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				Vec2i pos;

				lua_rawgeti(l, j + 1, k + 1);
				if (!lua_istable(l, -1) || lua_objlen(l, -1) != 3) {
					LuaError(l, "incorrect argument");
				}
				lua_rawgeti(l, -1, 1);
				pos.x = LuaToNumber(l, -1);
				lua_pop(l, 1);
				lua_rawgeti(l, -1, 2);
				pos.y = LuaToNumber(l, -1);
				lua_pop(l, 1);
				lua_rawgeti(l, -1, 3);
				const int mask = LuaToNumber(l, -1);
				lua_pop(l, 1);
				lua_pop(l, 1);
				AiExplorationRequest queue(pos, mask);
				ai->FirstExplorationRequest.push_back(queue);
			}
		} else if (!strcmp(value, "last-exploration-cycle")) {
			ai->LastExplorationGameCycle = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "last-can-not-move-cycle")) {
			ai->LastCanNotMoveGameCycle = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "unit-type")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, j + 1);
			i = 0;
			if (subargs) {
				ai->UnitTypeRequests.resize(subargs / 2);
			}
			for (k = 0; k < subargs; ++k) {
				const char *ident;
				int count;

				lua_rawgeti(l, j + 1, k + 1);
				ident = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, j + 1, k + 1);
				count = LuaToNumber(l, -1);
				lua_pop(l, 1);
				ai->UnitTypeRequests[i].Type = UnitTypeByIdent(ident);
				ai->UnitTypeRequests[i].Count = count;
				++i;
			}
		} else if (!strcmp(value, "upgrade")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				const char *ident;

				lua_rawgeti(l, j + 1, k + 1);
				ident = LuaToString(l, -1);
				lua_pop(l, 1);
				ai->UpgradeToRequests.push_back(UnitTypeByIdent(ident));
			}
		} else if (!strcmp(value, "research")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				const char *ident;

				lua_rawgeti(l, j + 1, k + 1);
				ident = LuaToString(l, -1);
				lua_pop(l, 1);
				ai->ResearchRequests.push_back(CUpgrade::Get(ident));
			}
		} else if (!strcmp(value, "building")) {
			CclParseBuildQueue(l, ai, j+1);
		} else if (!strcmp(value, "repair-building")) {
			ai->LastRepairBuilding = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "repair-workers")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				int num;
				int workers;

				lua_rawgeti(l, j + 1, k + 1);
				num = LuaToNumber(l, -1);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, j + 1, k + 1);
				workers = LuaToNumber(l, -1);
				lua_pop(l, 1);
				ai->TriedRepairWorkers[num] = workers;
				++i;
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	return 0;
}

/**
**  Register CCL features for unit-type.
*/
void AiCclRegister()
{
	// FIXME: Need to save memory here.
	// Loading all into memory isn't necessary.

	lua_register(Lua, "DefineAiHelper", CclDefineAiHelper);
	lua_register(Lua, "DefineAi", CclDefineAi);

	lua_register(Lua, "AiGetRace", CclAiGetRace);
	lua_register(Lua, "AiGetSleepCycles", CclAiGetSleepCycles);

	lua_register(Lua, "AiDebug", CclAiDebug);
	lua_register(Lua, "AiDebugPlayer", CclAiDebugPlayer);
	lua_register(Lua, "AiNeed", CclAiNeed);
	lua_register(Lua, "AiSet", CclAiSet);
	lua_register(Lua, "AiWait", CclAiWait);

	lua_register(Lua, "AiForce", CclAiForce);

	lua_register(Lua, "AiForceRole", CclAiForceRole);
	lua_register(Lua, "AiCheckForce", CclAiCheckForce);
	lua_register(Lua, "AiWaitForce", CclAiWaitForce);

	lua_register(Lua, "AiAttackWithForce", CclAiAttackWithForce);
	lua_register(Lua, "AiSleep", CclAiSleep);
	lua_register(Lua, "AiResearch", CclAiResearch);
	lua_register(Lua, "AiUpgradeTo", CclAiUpgradeTo);

	lua_register(Lua, "AiPlayer", CclAiPlayer);
	lua_register(Lua, "AiSetReserve", CclAiSetReserve);
	lua_register(Lua, "AiSetCollect", CclAiSetCollect);

	lua_register(Lua, "AiDump", CclAiDump);

	lua_register(Lua, "DefineAiPlayer", CclDefineAiPlayer);
	lua_register(Lua, "AiAttackWithForces", CclAiAttackWithForces);
	lua_register(Lua, "AiWaitForces", CclAiWaitForces);

}


//@}
