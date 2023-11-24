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
//      (c) Copyright 2000-2015 by Lutz Sammer, Ludovic Pollet,
//      Jimmy Salmon and Andrettin
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

#include "network.h"
#include "net_lowlevel.h"
#include "stratagus.h"

#include "ai.h"
#include "ai_local.h"

#include "interface.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"
#include "upgrade.h"

/**
**  Insert new unit-type element.
**
**  @param table  Table with elements.
**  @param n      Index to insert new into table
**  @param base   Base type to insert into table.
*/
static void AiHelperInsert(std::vector<std::vector<CUnitType *> > &table,
						   unsigned int n, CUnitType &base)
{
	if (n >= table.size()) {
		table.resize(n + 1);
	}
	// Look if already known
	std::vector<CUnitType *>::const_iterator it = ranges::find(table[n], &base);
	if (it != table[n].end()) {
		return;
	}
	table[n].push_back(&base);
}

/**
**  Transform list of unit separed with coma to a true list.
*/
static std::vector<CUnitType *> getUnitTypeFromString(std::string_view list)
{
	std::vector<CUnitType *> res;

	if (list == "*") {
		return UnitTypes;
	}
	size_t begin = 1;
	size_t end = list.find(",", begin);
	while (end != std::string::npos) {
		std::string_view unitName = list.substr(begin, end - begin);
		begin = end + 1;
		end = list.find(",", begin);
		if (starts_with(unitName, "unit-")) {
			res.push_back(&UnitTypeByIdent(unitName));
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

	for (CUnitType *type : UnitTypes) {
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

	for (CUnitType *type : UnitTypes) {
		if (type->DefaultStat.Variables[SUPPLY_INDEX].Value > 0) { //supply units are identified as being those with a default stat supply of 1 or more; so if a unit has a supply default stat of 0, but through an upgrade ends up having 1 or more supply, it won't be included here
			res.push_back(type);
		}
	}
	// Now, sort them, best first.
	while (!res.empty()) {
		float bestscore = 0;
		CUnitType *besttype = nullptr;

		for (CUnitType *type : res) {
			unsigned int cost = 0;

			for (unsigned j = 0; j < MaxCosts; ++j) {
				cost += type->DefaultStat.Costs[j]; //this cannot be MapDefaultStat because this function is called when the AiHelper is defined, rather than when a game is started
			}
			const float score = ((float) type->DefaultStat.Variables[SUPPLY_INDEX].Value) / cost;
			if (score > bestscore) {
				bestscore = score;
				besttype = type;
			}
		}
		sorted_res.push_back(besttype);
		auto it = ranges::find(res, besttype);
		if (it != res.end()) {
			res.erase(it);
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

	for (CUnitType *type : UnitTypes) {
		if (type->GivesResource > 0 && type->BoolFlag[CANHARVEST_INDEX].value) {
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
		for (CUnitType *type : res) {
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
		auto it = ranges::find(res, besttype);
		if (it != res.end()) {
			res.erase(it);
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
	std::vector<CUnitType *> reparableUnits = getReparableUnits();
	std::vector<CUnitType *> supplyUnits = getSupplyUnits();
	std::vector<CUnitType *> mineUnits = getRefineryUnits();

	for (CUnitType *type : supplyUnits) {
		AiHelperInsert(aiHelper.UnitLimit(), 0, *type);
	}

	for (int i = 1; i < MaxCosts; ++i) {
		for (CUnitType *type : mineUnits) {
			if (type->GivesResource == i) {
				/* HACK : we can't mine TIME then use 0 as 1 */
				AiHelperInsert(aiHelper.Refinery(), i - 1, *type);
			}
		}
		for (CUnitType *type : UnitTypes) {
			if (type->CanStore[i] > 0) {
				/* HACK : we can't store TIME then use 0 as 1 */
				AiHelperInsert(aiHelper.Depots(), i - 1, *type);
			}
		}
	}

	for (const auto &button : UnitButtonTable) {
		const std::vector<CUnitType *> &unitmask = getUnitTypeFromString(button->UnitMask);

		switch (button->Action) {
			case ButtonCmd::Repair :
				for (CUnitType *type : unitmask) {
					for (CUnitType *reparableUnit : reparableUnits) {
						AiHelperInsert(aiHelper.Repair(), reparableUnit->Slot, *type);
					}
				}
				break;
			case ButtonCmd::Build: {
				CUnitType &buildingType = UnitTypeByIdent(button->ValueStr);

				for (CUnitType *type : unitmask) {
					AiHelperInsert(aiHelper.Build(), buildingType.Slot, *type);
				}
				break;
			}
			case ButtonCmd::Train : {
				CUnitType &trainingType = UnitTypeByIdent(button->ValueStr);

				for (CUnitType *type : unitmask) {
					AiHelperInsert(aiHelper.Train(), trainingType.Slot, *type);
				}
				break;
			}
			case ButtonCmd::UpgradeTo : {
				CUnitType &upgradeToType = UnitTypeByIdent(button->ValueStr);

				for (CUnitType *type : unitmask) {
					AiHelperInsert(aiHelper.Upgrade(), upgradeToType.Slot, *type);
				}
				break;
			}
			case ButtonCmd::Research : {
				int researchId = UpgradeIdByIdent(button->ValueStr);

				if (button->Allowed == ButtonCheckSingleResearch) {
					for (CUnitType *type : unitmask) {
						AiHelperInsert(aiHelper.SingleResearch(), researchId, *type);
					}
				} else {
					for (CUnitType *type : unitmask) {
						AiHelperInsert(aiHelper.Research(), researchId, *type);
					}
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
	InitAiHelper(AiHelpers);

	const int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			LuaError(l, "incorrect argument");
		}
		const int subargs = lua_rawlen(l, j + 1);
		const std::string_view value = LuaToString(l, j + 1, 1);
		if (value == "build"
			|| value == "train"
			|| value == "upgrade"
			|| value == "research"
			|| value == "unit-limit"
			|| value == "repair") {
			DebugPrint("DefineAiHelper: Relation is computed from buttons, "
			           "you may remove safely the block beginning with '\"%s\"'\n",
			           value.data());
			continue;
		} else if (value == "unit-equiv") {
		} else {
			LuaError(l, "unknown tag: %s", value.data());
		}
		// Get the base unit type, which could handle the action.
		const std::string_view baseTypeName = LuaToString(l, j + 1, 2);
		const CUnitType &base = UnitTypeByIdent(baseTypeName);

		// Get the unit types, which could be produced
		for (int k = 2; k < subargs; ++k) {
			const std::string_view equivTypeName = LuaToString(l, j + 1, k + 1);
			CUnitType &type = UnitTypeByIdent(equivTypeName);

			AiHelperInsert(AiHelpers.Equiv(), base.Slot, type);
			AiNewUnitTypeEquiv(base, type);
		}
	}
	return 0;
}

static CAiType *GetAiTypesByName(const std::string_view name)
{
	for (auto &ait : AiTypes) {
		if (ait->Name == name) {
			return ait.get();
		}
	}
	return nullptr;
}

/**
** <b>Description</b>
**
**  Define an AI engine. Every game should define at least two AIs: one called 'ai-passive' and
**  one called 'ai-active'. These two are the default AI names used by the games. 'ai-passive'
**  is used almost everywhere when no other AI is selected by a map, and 'ai-active' is the default
**  AI used when adding AI players to a multiplayer game when no other AI is selected by the map.
**
** Example:
**
** <div class="example"><code>-- Those instructions will be executed forever
**		local simple_ai_loop = {
**			-- Sleep for 5 in game minutes
**			function() return AiSleep(9000) end,
**			-- Repeat the functions from start.
**			function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**		}
**
**		-- The initial instructions the A.I has to execute
**		local simple_ai = {
**			function() return AiSleep(1800) end, -- Sleep for 1 in game minute
**			function() return AiNeed("unit-town-hall") end, -- One Town Hall is needed
**			function() return AiWait("unit-town-hall") end, -- Wait until the Town Hall is completed
**			function() return AiSet("unit-peasant", 4) end, -- Make 4 peasants
**			-- Basic buildings
**			function() return AiSet("unit-farm", 4) end, -- Make 4 farms
**			function() return AiWait("unit-farm") end, -- Wait until all 4 farms are build.
**			function() return AiNeed("unit-human-barracks") end, -- Need a Barracks
**			function() return AiWait("unit-human-barracks") end, -- Wait until the Barracks has been built
**			-- Defense force for the base
**			function() return AiForce(1, {"unit-footman", 3}) end, -- Make a force of 3 footmen
**			function() return AiWaitForce(1) end, -- Wait until the 3 footmen are trained
**			function() return AiForceRole(1,"defend") end, -- Make this force as a defense force
**			-- Execute the instructions in simple_ai_loop
**			function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**		}
**
**		-- function that calls the instructions in simple_ai inside DefineAi
**		function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**		-- Make an A.I for the human race that calls the function custom_ai
**		<strong>DefineAi</strong>("ai-name","human","ai-class",custom_ai)</code></div>
*/
static int CclDefineAi(lua_State *l)
{
	LuaCheckArgs(l, 4);
	if (!lua_isfunction(l, 4)) {
		LuaError(l, "incorrect argument");
	}

	// AI Name
	const std::string_view aiName = LuaToString(l, 1);

#ifdef DEBUG
	if (GetAiTypesByName(aiName)) {
		DebugPrint("Warning two or more AI's with the same name '%s'\n", aiName.data());
	}
#endif
	AiTypes.insert(AiTypes.begin(), std::make_unique<CAiType>());
	CAiType *aitype = AiTypes.front().get();
	aitype->Name = aiName;

	// AI Race
	const std::string_view value = LuaToString(l, 2);
	if (!value.empty() && value[0] != '*') {
		aitype->Race = value;
	} else {
		aitype->Race.clear();
	}

	// AI Class
	aitype->Class = LuaToString(l, 3);

	// AI Script
	lua_getglobal(l, "_ai_scripts_");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_setglobal(l, "_ai_scripts_");
		lua_getglobal(l, "_ai_scripts_");
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
	const auto it = ranges::find(AiPlayer->UnitTypeRequests, type, &AiRequestType::Type);
	return it != AiPlayer->UnitTypeRequests.end() ? &*it : nullptr;
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
** <b>Description</b>
**
**  Get the number of cycles to sleep.
**
**  @param l  Lua state
**
**  @return   Number of return values
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**			-- Sleep for 5 in game minutes
**			function() return AiSleep(9000) end,
**			-- Repeat the instructions forever.
**			function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**		}
**
**		local simple_ai = {
**			-- Get the number of cycles to sleep.
**			function() return AiSleep(<strong>AiGetSleepCycles()</strong>) end,
**			-- Execute the instructions in simple_ai_loop
**			function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**		}
**
**		function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**		DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
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
	const int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		std::string_view item;

		if (lua_isstring(l, j + 1)) {
			item = LuaToString(l, j + 1);
		}
		if (item == "none") {
			for (int i = 0; i != NumPlayers; ++i) {
				if (!Players[i].AiEnabled || !Players[i].Ai) {
					continue;
				}
				Players[i].Ai->ScriptDebug = false;
			}
		} else {
			int playerid;
			if (item == "self") {
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
			Players[playerid].Ai->ScriptDebug = true;
		}
	}
	return 0;
}

/**
** <b>Description</b>
**
**  Need a unit.
**
**  @param l  Lua state.
**
**  @return   Number of return values
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**			-- Sleep for 5 in game minutes
**			function() return AiSleep(9000) end,
**			-- Repeat the instructions forever.
**			function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**		}
**
**		local simple_ai = {
**			function() return AiSleep(AiGetSleepCycles()) end,
**			-- Tell to the A.I that it needs a Town Hall.
**			function() return <strong>AiNeed</strong>("unit-town-hall") end,
**			-- Execute the instructions in simple_ai_loop
**			function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**		}
**
**		function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**		DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiNeed(lua_State *l)
{
	LuaCheckArgs(l, 1);
	InsertUnitTypeRequests(CclGetUnitType(l), 1);

	lua_pushboolean(l, 0);
	return 1;
}

/**
** <b>Description</b>
**
**  Set the number of units.
**
**  @param l  Lua state
**
**  @return   Number of return values
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		-- Sleep for 5 in game minutes
**		function() return AiSleep(9000) end,
**		-- Repeat the instructions forever.
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		function() return AiSleep(AiGetSleepCycles()) end,
**		-- Tell to the A.I that it needs a Town Hall.
**		function() return AiNeed("unit-town-hall") end,
**		-- Wait until the town-hall has been built.
**		function() return AiWait("unit-town-hall") end,
**		-- Tell to the A.I that it needs 4 peasants
**		function() return <strong>AiSet</strong>("unit-peasant",4) end,
**		-- Execute the instructions in simple_ai_loop
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiSet(lua_State *l)
{
	LuaCheckArgs(l, 2);
	lua_pushvalue(l, 1);
	CUnitType *type = CclGetUnitType(l);
	lua_pop(l, 1);

	AiRequestType *autt = FindInUnitTypeRequests(type);
	if (autt) {
		autt->Count = LuaToNumber(l, 2);
		// FIXME: 0 should remove it.
	} else {
		InsertUnitTypeRequests(type, LuaToNumber(l, 2));
	}
	lua_pushboolean(l, 0);
	return 1;
}

/**
** <b>Description</b>
**
**  Wait for a unit.
**
**  @param l  Lua State.
**
**  @return   Number of return values
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		-- Sleep for 5 in game minutes
**		function() return AiSleep(9000) end,
**		-- Repeat the instructions forever.
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		function() return AiSleep(AiGetSleepCycles()) end,
**		-- Tell to the A.I that it needs a Town Hall.
**		function() return AiNeed("unit-town-hall") end,
**		-- Wait until the Town Hall has been built.
**		function() return <strong>AiWait</strong>("unit-town-hall") end,
**		-- Execute the instructions in simple_ai_loop
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiWait(lua_State *l)
{
	LuaCheckArgs(l, 1);
	const CUnitType *type = CclGetUnitType(l);
	const int *unit_types_count = AiPlayer->Player->UnitTypesAiActiveCount;
	const AiRequestType *autt = FindInUnitTypeRequests(type);
	if (!autt) {
		// Look if we have this unit-type.
		if (unit_types_count[type->Slot]) {
			lua_pushboolean(l, 0);
			return 1;
		}

		// Look if we have equivalent unit-types.
		if (type->Slot < (int)AiHelpers.Equiv().size()) {
			for (size_t j = 0; j < AiHelpers.Equiv()[type->Slot].size(); ++j) {
				if (unit_types_count[AiHelpers.Equiv()[type->Slot][j]->Slot]) {
					lua_pushboolean(l, 0);
					return 1;
				}
			}
		}
		// Look if we have an upgrade-to request.
		if (ranges::contains(AiPlayer->UpgradeToRequests, type)) {
			lua_pushboolean(l, 1);
			return 1;
		}
		DebugPrint("Broken? waiting on %s which wasn't requested.\n", type->Ident.c_str());
		lua_pushboolean(l, 0);
		return 1;
	}
	//
	// Add equivalent units
	//
	unsigned int n = unit_types_count[type->Slot];
	if (type->Slot < (int)AiHelpers.Equiv().size()) {
		for (size_t j = 0; j < AiHelpers.Equiv()[type->Slot].size(); ++j) {
			n += unit_types_count[AiHelpers.Equiv()[type->Slot][j]->Slot];
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
**  Get number of active build requests for a unit type.
**
**  @param l  Lua State.
**
**  @return   Number of return values
*/
static int CclAiPendingBuildCount(lua_State *l)
{
	LuaCheckArgs(l, 1);
	const CUnitType *type = CclGetUnitType(l);

	// The assumption of this function is that UnitTypeBuilt will be always be
	// fairly small so we iterate each time instead of doing any caching
	for (auto b : AiPlayer->UnitTypeBuilt) {
		if (b.Type == type) {
			lua_pushinteger(l, b.Want);
			return 1;
		}
	}
	lua_pushinteger(l, 0);
	return 1;
}

/**
** <b>Description</b>
**
**  Define a force, a groups of units.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		-- Sleep for 5 in game minutes
**		function() return AiSleep(9000) end,
**		-- Repeat the instructions forever.
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		function() return AiSleep(AiGetSleepCycles()) end,
**		-- Tell to the A.I that it needs a Town Hall.
**		function() return AiNeed("unit-town-hall") end,
**		-- Wait until the Town Hall has been built.
**		function() return AiWait("unit-town-hall") end,
**		-- Tell to the A.I that it needs 4 peasants
**		function() return AiSet("unit-peasant",4) end,
**		-- Tell to the A.I that it needs a Barracks.
**		function() return AiNeed("unit-human-barracks") end,
**		-- Tell to the A.I that it needs a Lumbermill.
**		function() return AiNeed("unit-elven-lumber-mill") end,
**		-- Wait until the Barracks has been built.
**		function() return AiWait("unit-human-barracks") end,
**		-- Wait until the Lumbermill has been built.
**		function() return AiWait("unit-elven-lumber-mill") end,
**		-- Specify the force number 1 made only of 3 footmen
**		function() return <strong>AiForce</strong>(1,{"unit-footman", 3}) end,
**		-- Specify the force number 2 made only of 2 archers
**		function() return <strong>AiForce</strong>(2,{"unit-archer", 2}) end,
**		-- Specify the force number 3 made of 2 footmen and 1 archer
**		function() return <strong>AiForce</strong>(3,{"unit-footman", 2, "unit-archer", 1}) end,
**		-- Wait for all three forces
**		function() return AiWaitForce(1) end,
**		function() return AiWaitForce(2) end,
**		function() return AiWaitForce(3) end,
**		-- Execute the instructions in simple_ai_loop
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiForce(lua_State *l)
{
	bool resetForce = false;
	const int arg = lua_gettop(l);
	Assert(0 < arg && arg <= 3);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}
	if (arg == 3) {
		resetForce = LuaToBoolean(l, 3);
	}
	int force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCE_INTERNAL) {
		LuaError(l, "Force out of range: %d", force);
	}
	AiForce &aiforce = AiPlayer->Force[AiPlayer->Force.getScriptForce(force)];
	if (resetForce) {
		aiforce.Reset(true);
	}
	AiForceRole role = aiforce.Role;
	aiforce.State = AiForceAttackingState::Waiting;
	aiforce.Role = role;

	int args = lua_rawlen(l, 2);
	for (int j = 0; j < args; ++j) {
		lua_rawgeti(l, 2, j + 1);
		CUnitType *type = CclGetUnitType(l);
		lua_pop(l, 1);
		++j;
		int count = LuaToNumber(l, 2, j + 1);

		if (!type) { // bulletproof
			continue;
		}

		// Use the equivalent unittype.
		type = UnitTypes[UnitTypeEquivs[type->Slot]];

		if (resetForce) {
			// Append it.
			AiUnitType newaiut;
			newaiut.Want = count;
			newaiut.Type = type;
			aiforce.UnitTypes.push_back(newaiut);
		} else {
			// Look if already in force.
			size_t i;
			for (i = 0; i < aiforce.UnitTypes.size(); ++i) {
				AiUnitType *aiut = &aiforce.UnitTypes[i];
				if (aiut->Type->Slot == type->Slot) { // found
					if (count) {
						aiut->Want = count;
					} else {
						aiforce.UnitTypes.erase(aiforce.UnitTypes.begin() + i);
					}
					break;
				}
			}
			// New type append it.
			if (i == aiforce.UnitTypes.size()) {
				AiUnitType newaiut;
				newaiut.Want = count;
				newaiut.Type = type;
				aiforce.UnitTypes.push_back(newaiut);
			}
		}

	}
	AiAssignFreeUnitsToForce(force);
	lua_pushboolean(l, 0);
	return 1;
}

/**
** <b>Description</b>
**
**  Define the role of a force.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		function() return AiSleep(9000) end,
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		function() return AiSleep(AiGetSleepCycles()) end,
**		function() return AiNeed("unit-town-hall") end,
**		function() return AiWait("unit-town-hall") end,
**		function() return AiSet("unit-peasant",4) end,
**		function() return AiNeed("unit-human-barracks") end,
**		function() return AiNeed("unit-elven-lumber-mill") end,
**		-- Force 1 has the role of attacker
**		function() return <strong>AiForceRole</strong>(1,"<u>attack</u>") end,
**		-- Force 2 has the role of defender
**		function() return <strong>AiForceRole</strong>(2,"<u>defend</u>") end,
**		function() return AiForce(1,{"unit-footman", 3}) end,
**		function() return AiForce(2,{"unit-archer", 2}) end,
**		function() return AiWaitForce(1) end,
**		function() return AiWaitForce(2) end,
**		-- Execute the instructions in simple_ai_loop
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiForceRole(lua_State *l)
{
	LuaCheckArgs(l, 2);
	int force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCE_INTERNAL) {
		LuaError(l, "Force %i out of range", force);
	}

	AiForce &aiforce = AiPlayer->Force[AiPlayer->Force.getScriptForce(force)];

	const std::string_view flag = LuaToString(l, 2);
	if (flag == "attack") {
		aiforce.Role = AiForceRole::Attack;
	} else if (flag == "defend") {
		aiforce.Role = AiForceRole::Defend;
	} else {
		LuaError(l, "Unknown force role '%s'", flag.data());
	}
	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Release force.
**
**  @param l  Lua state.
*/
static int CclAiReleaseForce(lua_State *l)
{
	LuaCheckArgs(l, 1);
	int force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCE_INTERNAL) {
		LuaError(l, "Force out of range: %d", force);
	}
	for (int i = AI_MAX_FORCE_INTERNAL; i < AI_MAX_FORCES; ++i) {
		if (AiPlayer->Force[i].FormerForce == force) {
			for (CUnit *aiunit : AiPlayer->Force[i].Units) {
				aiunit->GroupId = 0;
			}
			AiPlayer->Force[i].Units.clear();
			AiPlayer->Force[i].Reset(false);
		}
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
	if (force < 0 || force >= AI_MAX_FORCE_INTERNAL) {
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
** <b>Description</b>
**
**  Wait for a force ready.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		function() return AiSleep(9000) end,
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		function() return AiSleep(AiGetSleepCycles()) end,
**		function() return AiNeed("unit-town-hall") end,
**		function() return AiWait("unit-town-hall") end,
**		function() return AiSet("unit-peasant",4) end,
**		function() return AiNeed("unit-human-barracks") end,
**		function() return AiWait("unit-human-barracks") end,
**		function() return AiForce(1,{"unit-footman", 3}) end,
**		-- Wait until force 1 is completed
**		function() return <strong>AiWaitForce</strong>(1) end,
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiWaitForce(lua_State *l)
{
	LuaCheckArgs(l, 1);
	int force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCE_INTERNAL) {
		lua_pushfstring(l, "Force out of range: %d", force);
	}
	if (AiPlayer->Force[AiPlayer->Force.getScriptForce(force)].Completed) {
		lua_pushboolean(l, 0);
		return 1;
	}
#if 0
	// Debuging
	AiRemoveDeadUnitInForces();
	Assert(!AiPlayer->Force.getScriptForce(f).Completed);
#endif
	lua_pushboolean(l, 1);
	return 1;
}

/**
** <b>Description</b>
**
**  Attack with one single force.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		function() return AiSleep(9000) end,
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		function() return AiSleep(AiGetSleepCycles()) end,
**		function() return AiNeed("unit-town-hall") end,
**		function() return AiWait("unit-town-hall") end,
**		function() return AiSet("unit-peasant",4) end,
**		function() return AiNeed("unit-human-barracks") end,
**		function() return AiWait("unit-human-barracks") end,
**		function() return AiForce(1,{"unit-footman", 3}) end,
**		function() return AiWaitForce(1) end,
**		-- Attack with Force 1
**		function() return <strong>AiAttackWithForce</strong>(<u>1</u>) end,
**
**		function() return AiForce(2,{"unit-footman",5}) end,
**		function() return AiWaitForce(2) end,
**		-- Attack with Force 2
**		function() return <strong>AiAttackWithForce</strong>(<u>2</u>) end,
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiAttackWithForce(lua_State *l)
{
	LuaCheckArgs(l, 1);
	int force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCE_INTERNAL) {
		LuaError(l, "Force out of range: %d", force);
	}
	AiAttackWithForce(force);
	lua_pushboolean(l, 0);
	return 1;
}

/**
** <b>Description</b>
**
**  Wait for a forces ready.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		function() return AiSleep(9000) end,
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		function() return AiSleep(AiGetSleepCycles()) end,
**		function() return AiNeed("unit-town-hall") end,
**		function() return AiWait("unit-town-hall") end,
**		function() return AiSet("unit-peasant",4) end,
**		function() return AiNeed("unit-human-barracks") end,
**		function() return AiWait("unit-human-barracks") end,
**		function() return AiNeed("unit-elven-lumber-mill") end,
**		function() return AiWait("unit-elven-lumber-mill") end,
**		function() return AiForce(1,{"unit-footman", 2}) end,
**		function() return AiForce(2,{"unit-archer",1}) end,
**		function() return AiForce(3,{"unit-archer",1,"unit-footman",3}) end,
**		-- Wait all three forces to be ready
**		function() return <strong>AiWaitForces</strong>({1,2,3}) end,
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiWaitForces(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	const int args = lua_rawlen(l, 1);
	for (int i = 0; i < args; ++i) {
		const int force = LuaToNumber(l, 1, i + 1);

		if (force < 0 || force >= AI_MAX_FORCE_INTERNAL) {
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
** <b>Description</b>
**
**  Attack with forces.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		function() return AiSleep(9000) end,
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		function() return AiSleep(AiGetSleepCycles()) end,
**		function() return AiNeed("unit-town-hall") end,
**		function() return AiWait("unit-town-hall") end,
**		function() return AiSet("unit-peasant",4) end,
**		function() return AiNeed("unit-human-barracks") end,
**		function() return AiWait("unit-human-barracks") end,
**		function() return AiNeed("unit-elven-lumber-mill") end,
**		function() return AiWait("unit-elven-lumber-mill") end,
**		function() return AiForce(1,{"unit-footman", 2}) end,
**		function() return AiForce(2,{"unit-archer",1}) end,
**		function() return AiForce(3,{"unit-archer",1,"unit-footman",3}) end,
**		-- Wait all three forces to be ready
**		function() return AiWaitForces({1,2,3}) end,
**		-- Attack with all three forces
**		function() return <strong>AiAttackWithForces</strong>({1,2,3}) end,
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiAttackWithForces(lua_State *l)
{
	int Forces[AI_MAX_FORCE_INTERNAL + 1];

	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	int args = lua_rawlen(l, 1);
	for (int i = 0; i < args; ++i) {
		const int force = LuaToNumber(l, 1, i + 1);

		if (force < 0 || force >= AI_MAX_FORCE_INTERNAL) {
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
** <b>Description</b>
**
**  Sleep for n cycles.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**			-- Sleep for 5 in game minutes
**			function() return <strong>AiSleep</strong>(9000) end,
**			function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**		}
**
**		local simple_ai = {
**			-- Get the number of cycles to sleep
**			function() return <strong>AiSleep</strong>(AiGetSleepCycles()) end,
**			function() return AiNeed("unit-town-hall") end,
**			function() return AiWait("unit-town-hall") end,
**			function() return AiSet("unit-peasant",4) end,
**			-- Sleep for 1 in game minute
**			function() return <strong>AiSleep</strong>(1800) end,
**			function() return AiNeed("unit-human-blacksmith") end,
**			function() return AiWait("unit-human-blacksmith") end,
**			function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**		}
**
**		function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**		DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiSleep(lua_State *l)
{
	LuaCheckArgs(l, 1);
	int i = LuaToNumber(l, 1);
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
** <b>Description</b>
**
**  Research an upgrade.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		function() return AiSleep(9000) end,
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		-- Get the number of cycles to sleep
**		function() return AiSleep(AiGetSleepCycles()) end,
**		function() return AiNeed("unit-town-hall") end,
**		function() return AiWait("unit-town-hall") end,
**		function() return AiSet("unit-peasant",4) end,
**		function() return AiNeed("unit-human-barracks") end,
**		function() return AiWait("unit-human-barracks") end,
**		function() return AiNeed("unit-elven-lumber-mill") end,
**		function() return AiWait("unit-elven-lumber-mill") end,
**		function() return AiForce(1,{"unit-footman", 1, "unit-archer", 2}) end,
**		function() return AiWaitForce(1) end,
**		-- Upgrade the archers arrow.
**		function() return <strong>AiResearch</strong>("upgrade-arrow1") end,
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiResearch(lua_State *l)
{
	LuaCheckArgs(l, 1);
	const std::string_view str = LuaToString(l, 1);
	CUpgrade *upgrade;

	if (!str.empty()) {
		upgrade = CUpgrade::Get(str);
	} else {
		LuaError(l, "Upgrade needed");
		upgrade = nullptr;
	}
	InsertResearchRequests(upgrade);
	lua_pushboolean(l, 0);
	return 1;
}

/**
** <b>Description</b>
**
**  Upgrade an unit to an new unit-type.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		function() return AiSleep(9000) end,
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		function() return AiSleep(AiGetSleepCycles()) end,
**		function() return AiNeed("unit-town-hall") end,
**		function() return AiWait("unit-town-hall") end,
**		function() return AiSet("unit-peasant",4) end,
**		function() return AiNeed("unit-human-barracks") end,
**		function() return AiWait("unit-human-barracks") end,
**		function() return AiNeed("unit-elven-lumber-mill") end,
**		function() return AiWait("unit-elven-lumber-mill") end,
**		function() return AiSleep(1800) end,
**		-- Upgrade the Town Hall to Keep
**		function() return <strong>AiUpgradeTo</strong>("unit-keep") end,
**		function() return AiWait("unit-keep") end,
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiUpgradeTo(lua_State *l)
{
	LuaCheckArgs(l, 1);
	CUnitType *type = CclGetUnitType(l);
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
	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	lua_newtable(l);
	for (int i = 0; i < MaxCosts; ++i) {
		lua_pushnumber(l, AiPlayer->Reserve[i]);
		lua_rawseti(l, -2, i + 1);
	}
	for (int i = 0; i < MaxCosts; ++i) {
		AiPlayer->Reserve[i] = LuaToNumber(l, 1, i + 1);
	}
	return 1;
}

/**
** <b>Description</b>
**
**  Set AI player resource collect percent.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		function() return AiSleep(9000) end,
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		function() return AiSleep(AiGetSleepCycles()) end,
**		function() return AiNeed("unit-town-hall") end,
**		function() return AiWait("unit-town-hall") end,
**		-- The peasants will focus only on gathering gold
**		function() return <strong>AiSetCollect</strong>({0,100,0,0,0,0,0}) end,
**		function() return AiSet("unit-peasant",4) end,
**		function() return AiWait("unit-peasant") end,
**		function() return AiSleep(1800) end,
**		-- The peasants will now focus 50% on gathering gold and 50% on gathering lumber
**		function() return <strong>AiSetCollect</strong>({0,50,50,0,0,0,0}) end,
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiSetCollect(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	for (int i = 0; i < MaxCosts; ++i) {
		AiPlayer->Collect[i] = LuaToNumber(l, 1, i + 1);
	}
	return 0;
}

/**
**  Set AI player build.
**
**  @param l  Lua state.
*/
static int CclAiSetBuildDepots(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (!lua_isboolean(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	AiPlayer->BuildDepots = LuaToBoolean(l, 1);
	return 0;
}

/**
** <b>Description</b>
**
**  Dump some AI debug information.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>local simple_ai_loop = {
**		function() return AiSleep(3600) end,
**		-- Get the information from all the A.I players
**		function() return <strong>AiDump</strong>() end,
**		function() stratagus.gameData.AIState.loop_index[1 + AiPlayer()] = 0; return false; end,
**	}
**
**	local simple_ai = {
**		function() return AiSleep(AiGetSleepCycles()) end,
**		function() return AiNeed("unit-town-hall") end,
**		function() return AiWait("unit-town-hall") end,
**		function() return AiSet("unit-peasant",4) end,
**		function() return AiWait("unit-peasant") end,
**		function() return AiLoop(simple_ai_loop, stratagus.gameData.AIState.loop_index) end,
**	}
**
**	function custom_ai() return AiLoop(simple_ai,stratagus.gameData.AIState.index) end
**	DefineAi("example_ai","human","class_ai",custom_ai)</code></div>
*/
static int CclAiDump(lua_State *l)
{
	LuaCheckArgs(l, 0);
	for (int p = 0; p < PlayerMax - 1; ++p) {
		CPlayer &aip = Players[p];
		if (aip.AiEnabled) {
			//
			// Script
			//

			printf("------\n");
			for (int i = 0; i < MaxCosts; ++i) {
				printf("%s(%4d, %4d/%4d) ", DefaultResourceNames[i].c_str(),
					   aip.Resources[i], aip.StoredResources[i], aip.MaxResources[i]);
			}
			printf("\n");
			printf("Player %d:", aip.Index);
#if 0
			gh_display(gh_car(AiPlayer->Script));
#endif
			//
			// Requests
			//
			printf("UnitTypeRequests(%u):\n", static_cast<unsigned int>(aip.Ai->UnitTypeRequests.size()));
			for (const auto &requestType : aip.Ai->UnitTypeRequests) {
				printf("%s ", requestType.Type->Ident.c_str());
			}
			printf("\n");
			printf("UpgradeToRequests(%u):\n", static_cast<unsigned int>(aip.Ai->UpgradeToRequests.size()));
			for (const auto *unittype : aip.Ai->UpgradeToRequests) {
				printf("%s ", unittype->Ident.c_str());
			}
			printf("\n");
			printf("ResearchRequests(%u):\n", static_cast<unsigned int>(aip.Ai->ResearchRequests.size()));
			for (const auto *upgrade : aip.Ai->ResearchRequests) {
				printf("%s ", upgrade->Ident.c_str());
			}
			printf("\n");

			// Building queue
			printf("Building queue:\n");
			for (const AiBuildQueue &queue : aip.Ai->UnitTypeBuilt) {
				printf("%s(%d/%d) ", queue.Type->Ident.c_str(), queue.Made, queue.Want);
			}
			printf("\n");

			// PrintForce
			for (size_t i = 0; i < aip.Ai->Force.Size(); ++i) {
				printf("Force(%u%s%s):\n", static_cast<unsigned int>(i),
					   aip.Ai->Force[i].Completed ? ",complete" : ",recruit",
					   aip.Ai->Force[i].Attacking ? ",attack" : "");
				for (const AiUnitType &aut : aip.Ai->Force[i].UnitTypes) {
					printf("%s(%d) ", aut.Type->Ident.c_str(), aut.Want);
				}
				printf("\n");
			}
			printf("\n");
		}
	}
	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Parse AiBuildQueue builing list
**
**  @param l     Lua state.
**  @param ai  PlayerAi pointer which should be filled with the data.
*/
static void CclParseBuildQueue(lua_State *l, PlayerAi &ai, int offset)
{
	if (!lua_istable(l, offset)) {
		LuaError(l, "incorrect argument");
	}

	Vec2i pos(-1, -1);

	const int args = lua_rawlen(l, offset);
	for (int k = 0; k < args; ++k) {
		const std::string_view value = LuaToString(l, offset, k + 1);
		++k;

		if (value == "onpos") {
			pos.x = LuaToNumber(l, offset, k + 1);
			++k;
			pos.y = LuaToNumber(l, offset, k + 1);
		} else {
			//ident = LuaToString(l, j + 1, k + 1);
			//++k;
			const int made = LuaToNumber(l, offset, k + 1);
			++k;
			const int want = LuaToNumber(l, offset, k + 1);

			AiBuildQueue queue;
			queue.Type = &UnitTypeByIdent(value);
			queue.Want = want;
			queue.Made = made;
			queue.Pos = pos;

			ai.UnitTypeBuilt.push_back(queue);
			pos.x = -1;
			pos.y = -1;
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
	const unsigned int playerIdx = LuaToNumber(l, 0 + 1);

	Assert(playerIdx <= PlayerMax);
	DebugPrint("%p %d\n", (void *)Players[playerIdx].Ai.get(), Players[playerIdx].AiEnabled);
	// FIXME: lose this:
	// Assert(!Players[playerIdx].Ai && Players[playerIdx].AiEnabled);

	Players[playerIdx].Ai = std::make_unique<PlayerAi>();
	PlayerAi &ai = *Players[playerIdx].Ai;
	ai.Player = &Players[playerIdx];

	// Parse the list: (still everything could be changed!)
	const int args = lua_gettop(l);
	for (int j = 1; j < args; ++j) {
		const std::string_view value = LuaToString(l, j + 1);
		++j;

		if (value == "ai-type") {
			const std::string_view aiName = LuaToString(l, j + 1);
			CAiType *ait = GetAiTypesByName(aiName);
			if (ait == nullptr) {
				LuaError(l, "ai-type not found: %s", aiName.data());
			}
			ai.AiType = ait;
			ai.Script = ait->Script;
		} else if (value == "script") {
			ai.Script = LuaToString(l, j + 1);
		} else if (value == "script-debug") {
			ai.ScriptDebug = LuaToBoolean(l, j + 1);
		} else if (value == "sleep-cycles") {
			ai.SleepCycles = LuaToNumber(l, j + 1);
		} else if (value == "force") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			[[maybe_unused]]const int cclforceIdx = LuaToNumber(l, j + 1, 1);
			const int forceIdx = ai.Force.FindFreeForce(AiForceRole::Default);

			for (int k = 1; k < subargs; ++k) {
				std::string_view value = LuaToString(l, j + 1, k + 1);
				++k;
				if (value == "complete") {
					ai.Force[forceIdx].Completed = true;
					--k;
				} else if (value == "recruit") {
					ai.Force[forceIdx].Completed = false;
					--k;
				} else if (value == "attack") {
					ai.Force[forceIdx].Attacking = true;
					--k;
				} else if (value == "defend") {
					ai.Force[forceIdx].Defending = true;
					--k;
				} else if (value == "role") {
					value = LuaToString(l, j + 1, k + 1);
					if (value == "attack") {
						ai.Force[forceIdx].Role = AiForceRole::Attack;
					} else if (value == "defend") {
						ai.Force[forceIdx].Role = AiForceRole::Defend;
					} else {
						LuaError(l, "Unsupported force tag: %s", value.data());
					}
				} else if (value == "types") {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					const int subsubargs = lua_rawlen(l, -1);
					for (int subk = 0; subk < subsubargs; ++subk) {
						const int num = LuaToNumber(l, -1, subk + 1);
						++subk;
						const std::string_view ident = LuaToString(l, -1, subk + 1);
						AiUnitType queue;

						queue.Want = num;
						queue.Type = &UnitTypeByIdent(ident);
						ai.Force[forceIdx].UnitTypes.push_back(queue);
					}
					lua_pop(l, 1);
				} else if (value == "units") {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					const int subsubargs = lua_rawlen(l, -1);
					for (int subk = 0; subk < subsubargs; ++subk) {
						const int num = LuaToNumber(l, -1, subk + 1);
						++subk;
#if 0
						[[maybe_unused]]const std::string_view ident = LuaToString(l, -1, subk + 1);
#endif
						ai.Force[forceIdx].Units.push_back(&UnitManager->GetSlotUnit(num));
					}
					lua_pop(l, 1);
				} else if (value == "state") {
					ai.Force[forceIdx].State = AiForceAttackingState(LuaToNumber(l, j + 1, k + 1));
				} else if (value == "goalx") {
					ai.Force[forceIdx].GoalPos.x = LuaToNumber(l, j + 1, k + 1);
				} else if (value == "goaly") {
					ai.Force[forceIdx].GoalPos.y = LuaToNumber(l, j + 1, k + 1);
				} else if (value == "must-transport") {
					// Keep for backward compatibility
				} else {
					LuaError(l, "Unsupported tag: %s", value.data());
				}
			}
		} else if (value == "reserve") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				const std::string_view type = LuaToString(l, j + 1, k + 1);
				++k;
				int num = LuaToNumber(l, j + 1, k + 1);
				const int resId = GetResourceIdByName(l, type);
				ai.Reserve[resId] = num;
			}
		} else if (value == "used") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				const std::string_view type = LuaToString(l, j + 1, k + 1);
				++k;
				const int num = LuaToNumber(l, j + 1, k + 1);
				const int resId = GetResourceIdByName(l, type);
				ai.Used[resId] = num;
			}
		} else if (value == "needed") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				const std::string_view type = LuaToString(l, j + 1, k + 1);
				++k;
				const int num = LuaToNumber(l, j + 1, k + 1);
				const int resId = GetResourceIdByName(l, type);
				ai.Needed[resId] = num;
			}
		} else if (value == "collect") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				const std::string_view type = LuaToString(l, j + 1, k + 1);
				++k;
				const int num = LuaToNumber(l, j + 1, k + 1);
				const int resId = GetResourceIdByName(l, type);
				ai.Collect[resId] = num;
			}
		} else if (value == "need-mask") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				const std::string_view type = LuaToString(l, j + 1, k + 1);
				const int resId = GetResourceIdByName(l, type);
				ai.NeededMask |= (1 << resId);
			}
		} else if (value == "need-supply") {
			ai.NeedSupply = true;
			--j;
		} else if (value == "exploration") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				Vec2i pos;

				lua_rawgeti(l, j + 1, k + 1);
				if (!lua_istable(l, -1) || lua_rawlen(l, -1) != 3) {
					LuaError(l, "incorrect argument");
				}
				pos.x = LuaToNumber(l, -1, 1);
				pos.y = LuaToNumber(l, -1, 2);
				const int mask = LuaToNumber(l, -1, 3);
				lua_pop(l, 1);
				AiExplorationRequest queue(pos, mask);
				ai.FirstExplorationRequest.push_back(queue);
			}
		} else if (value == "last-exploration-cycle") {
			ai.LastExplorationGameCycle = LuaToNumber(l, j + 1);
		} else if (value == "last-can-not-move-cycle") {
			ai.LastCanNotMoveGameCycle = LuaToNumber(l, j + 1);
		} else if (value == "unit-type") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			int i = 0;
			if (subargs) {
				ai.UnitTypeRequests.resize(subargs / 2);
			}
			for (int k = 0; k < subargs; ++k) {
				const std::string_view ident = LuaToString(l, j + 1, k + 1);
				++k;
				const int count = LuaToNumber(l, j + 1, k + 1);
				ai.UnitTypeRequests[i].Type = &UnitTypeByIdent(ident);
				ai.UnitTypeRequests[i].Count = count;
				++i;
			}
		} else if (value == "upgrade") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				const std::string_view ident = LuaToString(l, j + 1, k + 1);
				ai.UpgradeToRequests.push_back(&UnitTypeByIdent(ident));
			}
		} else if (value == "research") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				const std::string_view ident = LuaToString(l, j + 1, k + 1);
				ai.ResearchRequests.push_back(CUpgrade::Get(ident));
			}
		} else if (value == "building") {
			CclParseBuildQueue(l, ai, j + 1);
		} else if (value == "repair-building") {
			ai.LastRepairBuilding = LuaToNumber(l, j + 1);
		} else {
			LuaError(l, "Unsupported tag: %s", value.data());
		}
	}
	return 0;
}

/**
 * AiProcessorSetup(host, port, number_of_state_variables, number_of_actions)
 *
 * Connect to an AI agent running at host:port, that will consume
 * number_of_state_variables every step and select one of number_of_actions.
 */
static int CclAiProcessorSetup(lua_State *l)
{
	InitNetwork1();
	LuaCheckArgs(l, 4);
	std::string host = std::string{LuaToString(l, 1)};
	int port = LuaToNumber(l, 2);
	int stateDim = LuaToNumber(l, 3);
	int actionDim = LuaToNumber(l, 4);

	CHost h(host, port);
	CTCPSocket *s = new CTCPSocket();
	s->Open(CHost());
	if (s->Connect(h)) {
		char buf[3];
		buf[0] = 'I';
		buf[1] = (uint8_t)stateDim;
		buf[2] = (uint8_t)actionDim;
		s->Send(buf, 3);
		lua_pushlightuserdata(l, s);
		return 1;
	}

	delete s;
	lua_pushnil(l);
	return 1;
}

static CTCPSocket * AiProcessorSendState(lua_State *l, char prefix)
{
	LuaCheckArgs(l, 3);
	CTCPSocket *s = (CTCPSocket *)lua_touserdata(l, 1);
	if (s == nullptr) {
		LuaError(l, "first argument must be valid handle returned from a previous AiProcessorSetup call");
	}

	uint32_t reward = htonl(LuaToNumber(l, 2));
	if (!lua_istable(l, 3)) {
		LuaError(l, "3rd argument to AiProcessorStep must be table");
	}

	char stepBuf[1029] = {'\0'}; // room for prefix + uint32 reward + 256 uint32 variables
	stepBuf[0] = prefix;
	int i = 1;

	memcpy(stepBuf + i, &reward, sizeof(uint32_t));
	i += sizeof(uint32_t);

	for (lua_pushnil(l); lua_next(l, 3); lua_pop(l, 1)) {
		// idx is ignored
		uint32_t var = htonl(LuaToNumber(l, -1));
		memcpy(stepBuf + i, &var, sizeof(uint32_t));
		i += sizeof(uint32_t);
		if (i + sizeof(uint32_t) > 1025) {
			LuaError(l, "too many state variables");
		}
	}
	s->Send(stepBuf, i);

	return s;
}

/**
 * AiProcessorStep(handle, reward_since_last_call, table_of_state_variables)
 */
static int CclAiProcessorStep(lua_State *l)
{
	// A single step in a reinforcement learning network

	// We receive the current env and current reward in the arguments

	// We need to return the next action.

	// The next call to this function will be the updated state, reward for the
	// last action

	CTCPSocket *s = AiProcessorSendState(l, 'S');
	int action = 0;
	s->Recv(&action, 1);
	lua_pushnumber(l, action + 1); // +1 since lua tables are 1-indexed
	return 1;
}

static int CclAiProcessorEnd(lua_State *l)
{
	CTCPSocket *s = AiProcessorSendState(l, 'E');
	s->Close();
	delete s;
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
	lua_register(Lua, "AiGetPendingBuilds", CclAiPendingBuildCount);

	lua_register(Lua, "AiForce", CclAiForce);

	lua_register(Lua, "AiReleaseForce", CclAiReleaseForce);
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

	lua_register(Lua, "AiSetBuildDepots", CclAiSetBuildDepots);

	lua_register(Lua, "AiDump", CclAiDump);

	lua_register(Lua, "DefineAiPlayer", CclDefineAiPlayer);
	lua_register(Lua, "AiAttackWithForces", CclAiAttackWithForces);
	lua_register(Lua, "AiWaitForces", CclAiWaitForces);

	// for external AI processors
	lua_register(Lua, "AiProcessorSetup", CclAiProcessorSetup);
	lua_register(Lua, "AiProcessorStep", CclAiProcessorStep);
	lua_register(Lua, "AiProcessorEnd", CclAiProcessorEnd);
}

//@}
