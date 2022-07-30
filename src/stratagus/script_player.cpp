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
/**@name script_player.cpp - The player ccl functions. */
//
//      (c) Copyright 2001-2007 by Lutz Sammer and Jimmy Salmon
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

#include "player.h"

#include "actions.h"
#include "ai.h"
#include "commands.h"
#include "map.h"
#include "script.h"
#include "unittype.h"
#include "unit.h"
#include "unit_find.h"
#include "upgrade.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern CUnit *CclGetUnitFromRef(lua_State *l);

/**
**  Get a player pointer
**
**  @param l  Lua state.
**
**  @return   The player pointer
*/
static CPlayer *CclGetPlayer(lua_State *l)
{
	return &Players[LuaToNumber(l, -1)];
}

/**
**  Parse the player configuration.
**
**  @param l  Lua state.
*/
static int CclPlayer(lua_State *l)
{
	int i = LuaToNumber(l, 1);

	CPlayer &player = Players[i];
	player.Index = i;

	if (NumPlayers <= i) {
		NumPlayers = i + 1;
	}

	player.Load(l);
	return 0;
}

void CPlayer::Load(lua_State *l)
{
	const int args = lua_gettop(l);

	this->Units.clear();
	this->FreeWorkers.clear();

	// j = 0 represent player Index.
	for (int j = 1; j < args; ++j) {
		const char *value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "name")) {
			this->SetName(LuaToString(l, j + 1));
		} else if (!strcmp(value, "type")) {
			value = LuaToString(l, j + 1);
			if (!strcmp(value, "neutral")) {
				this->Type = PlayerTypes::PlayerNeutral;
			} else if (!strcmp(value, "nobody")) {
				this->Type = PlayerTypes::PlayerNobody;
			} else if (!strcmp(value, "computer")) {
				this->Type = PlayerTypes::PlayerComputer;
			} else if (!strcmp(value, "person")) {
				this->Type = PlayerTypes::PlayerPerson;
			} else if (!strcmp(value, "rescue-passive")) {
				this->Type = PlayerTypes::PlayerRescuePassive;
			} else if (!strcmp(value, "rescue-active")) {
				this->Type = PlayerTypes::PlayerRescueActive;
			} else {
				LuaError(l, "Unsupported tag: %s" _C_ value);
			}
		} else if (!strcmp(value, "race")) {
			const char *raceName = LuaToString(l, j + 1);
			this->Race = PlayerRaces.GetRaceIndexByName(raceName);
			if (this->Race == -1) {
				LuaError(l, "Unsupported race: %s" _C_ raceName);
			}
		} else if (!strcmp(value, "ai-name")) {
			this->AiName = LuaToString(l, j + 1);
		} else if (!strcmp(value, "team")) {
			this->Team = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "enemy")) {
			value = LuaToString(l, j + 1);
			for (int i = 0; i < PlayerMax && *value; ++i, ++value) {
				if (*value == '-' || *value == '_' || *value == ' ') {
					this->Enemy &= ~(1 << i);
				} else {
					this->Enemy |= (1 << i);
				}
			}
		} else if (!strcmp(value, "allied")) {
			value = LuaToString(l, j + 1);
			for (int i = 0; i < PlayerMax && *value; ++i, ++value) {
				if (*value == '-' || *value == '_' || *value == ' ') {
					this->Allied &= ~(1 << i);
				} else {
					this->Allied |= (1 << i);
				}
			}
		} else if (!strcmp(value, "shared-vision")) {
			value = LuaToString(l, j + 1);
			for (int i = 0; i < PlayerMax && *value; ++i, ++value) {
				if (i == this->Index) {
					continue;
				}
				if (*value == '-' || *value == '_' || *value == ' ') {
					this->UnshareVisionWith(Players[i]);
				} else {
					this->ShareVisionWith(Players[i]);
				}
			}
		} else if (!strcmp(value, "start")) {
			CclGetPos(l, &this->StartPos.x, &this->StartPos.y, j + 1);
		} else if (!strcmp(value, "resources")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, j + 1, k + 1);
				++k;
				const int resId = GetResourceIdByName(l, value);
				this->Resources[resId] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (!strcmp(value, "stored-resources")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, j + 1, k + 1);
				++k;

				const int resId = GetResourceIdByName(l, value);
				this->StoredResources[resId] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (!strcmp(value, "max-resources")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, j + 1, k + 1);
				++k;
				const int resId = GetResourceIdByName(l, value);
				this->MaxResources[resId] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (!strcmp(value, "last-resources")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, j + 1, k + 1);
				++k;
				const int resId = GetResourceIdByName(l, value);
				this->LastResources[resId] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (!strcmp(value, "incomes")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, j + 1, k + 1);
				++k;

				const int resId = GetResourceIdByName(l, value);
				this->Incomes[resId] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (!strcmp(value, "revenue")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, j + 1, k + 1);
				++k;

				const int resId = GetResourceIdByName(l, value);
				this->Revenue[resId] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (!strcmp(value, "ai-enabled")) {
			this->AiEnabled = true;
			--j;
		} else if (!strcmp(value, "ai-disabled")) {
			this->AiEnabled = false;
			--j;
		} else if (!strcmp(value, "revealed")) {
			this->SetRevealed(true); 
			--j;	
		} else if (!strcmp(value, "supply")) {
			this->Supply = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "demand")) {
			this->Demand = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "unit-limit")) {
			this->UnitLimit = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "building-limit")) {
			this->BuildingLimit = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-unit-limit")) {
			this->TotalUnitLimit = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "score")) {
			this->Score = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-units")) {
			this->TotalUnits = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-buildings")) {
			this->TotalBuildings = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-razings")) {
			this->TotalRazings = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-kills")) {
			this->TotalKills = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "lost-main-facility-timer")) {
			this->LostMainFacilityTimer = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-resources")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			if (subargs != MaxCosts) {
				LuaError(l, "Wrong number of total-resources: %d" _C_ subargs);
			}
			for (int k = 0; k < subargs; ++k) {
				this->TotalResources[k] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (!strcmp(value, "speed-resource-harvest")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			if (subargs != MaxCosts) {
				LuaError(l, "Wrong number of speed-resource-harvest: %d" _C_ subargs);
			}
			for (int k = 0; k < subargs; ++k) {
				this->SpeedResourcesHarvest[k] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (!strcmp(value, "speed-resource-return")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			if (subargs != MaxCosts) {
				LuaError(l, "Wrong number of speed-resource-harvest: %d" _C_ subargs);
			}
			for (int k = 0; k < subargs; ++k) {
				this->SpeedResourcesReturn[k] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (!strcmp(value, "speed-build")) {
			this->SpeedBuild = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "speed-train")) {
			this->SpeedTrain = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "speed-upgrade")) {
			this->SpeedUpgrade = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "speed-research")) {
			this->SpeedResearch = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "color")) {
			if (!lua_istable(l, j + 1) || lua_rawlen(l, j + 1) != 3) {
				LuaError(l, "incorrect argument");
			}
			const int r = LuaToNumber(l, j + 1, 1);
			const int g = LuaToNumber(l, j + 1, 2);
			const int b = LuaToNumber(l, j + 1, 3);
			this->Color = Video.MapRGB(TheScreen->format, r, g, b);
		} else if (!strcmp(value, "timers")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			if (subargs != UpgradeMax) {
				LuaError(l, "Wrong upgrade timer length: %d" _C_ subargs);
			}
			for (int k = 0; k < subargs; ++k) {
				this->UpgradeTimers.Upgrades[k] = LuaToNumber(l, j + 1, k + 1);
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	// Manage max
	for (int i = 0; i < MaxCosts; ++i) {
		if (this->MaxResources[i] != -1) {
			this->SetResource(i, this->Resources[i] + this->StoredResources[i], STORE_BOTH);
		}
	}
}

/**
** <b>Description</b>
**
**  Change all units owned by one player or change only specific units owned by one player
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>-- Changes all units owned by player 0 and give to player 1
**		<strong>ChangeUnitsOwner</strong>({16, 17}, {30, 32}, 0, 1)
**		-- Changes all farms owned by player 0 and give to player 1
**		<strong>ChangeUnitsOwner</strong>({16, 17}, {30, 32}, 0, 1, "unit-farm")</code></div>
**
*/
static int CclChangeUnitsOwner(lua_State *l)
{
    int args = lua_gettop(l);
    if (args != 4 && args != 5) {
        LuaError(l, "incorrect argument count, need 4 or 5 args");
    }

    Vec2i pos1;
    Vec2i pos2;
    CclGetPos(l, &pos1.x, &pos1.y, 1);
    CclGetPos(l, &pos2.x, &pos2.y, 2);
	if (pos1.x > pos2.x) {
		std::swap(pos1.x, pos2.x);
	}
	if (pos1.y > pos2.y) {
		std::swap(pos1.y, pos2.y);
	}

    const int oldp = LuaToNumber(l, 3);
    const int newp = LuaToNumber(l, 4);
    std::vector<CUnit *> table;
	// Change all units
    if (args == 4) {
        Select(pos1, pos2, table, HasSamePlayerAs(Players[oldp]));
    } else { //Change only specific units by the type.
        CUnitType *type = UnitTypeByIdent(LuaToString(l, 5));
        Select(pos1, pos2, table, HasSamePlayerAndTypeAs(Players[oldp], *type));
    }
    for (auto unit : table) {
        unit->ChangeOwner(Players[newp]);
    }
    return 0;
}

/**
** <b>Description</b>
**
**  <strong>GiveUnitsToPlayer(amount, type, fromPlayer, toPlayer)</strong>
**  <strong>GiveUnitsToPlayer(amount, type, topLeft, bottomRight, fromPlayer, toPlayer)</strong>
**  Give some units of a specific type from a player to another player. Optionally only inside a rectangle.
**  Returns number of units actually assigned. This can be smaller than the requested amount if the
**  <code>fromPlayer</code> did not have enough units.<br/>
**
**  Instead of a number you can pass "all" as the first argument, to hand over all units.<br/>
**  
**  Instead of a unit type name, you can pass "any", "unit", "building" as the second argument,
**  to hand over anything, and unit, or any building.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>
**   -- Give 2 peasants from player 4 to player 2
**   GiveUnitsToPlayer(2, "unit-peasant", 4, 2)
**   -- Give 4 knights from player 5 to player 1 inside the rectangle 2,2 - 14,14
**   GiveUnitsToPlayer(2, "unit-peasant", {2,2}, {14,14}, 4, 2)
**   -- Give any 4 units from player 5 to player 1 inside the rectangle 2,2 - 14,14
**   GiveUnitsToPlayer(2, "any", 4, 2)
** </code></div>
**
*/
static int CclGiveUnitsToPlayer(lua_State *l)
{
	int args = lua_gettop(l);
	if (args != 4 && args != 6) {
		LuaError(l, "incorrect argument count for GiveUnitsToPlayer, need 4 or 6 args");
	}

	int cnt;
	if (lua_isnumber(l, 1)) {
		cnt = LuaToNumber(l, 1);
	} else {
		std::string cntStr = std::string(LuaToString(l, 1));
		if (cntStr != "all") {
			LuaError(l, "incorrect 1st argument to GiveUnitsToPlayer. Must be number or 'all'");
		}
		cnt = std::numeric_limits<int>::max();
	}

	const int oldp = LuaToNumber(l, args == 4 ? 3 : 5);
	const int newp = LuaToNumber(l, args == 4 ? 4 : 6);

	std::string typestr = std::string(LuaToString(l, 2));
	int assignedCnt = 0;

	CUnitType *type = nullptr;
	bool any = false;
	bool onlyUnits = false;
	bool onlyBuildings = false;
	if ((any = (typestr == "any"))) {
	} else if ((onlyUnits = (typestr == "unit"))) {
	} else if ((onlyBuildings = (typestr == "building"))) {
	} else {
		type = UnitTypeByIdent(LuaToString(l, 2));
		if (!type) {
			LuaError(l, "incorrect 2nd argument to GiveUnitsToPlayer. Must be a unit type or 'any', 'unit', or 'building'");
		}
	}

	if (cnt > 0) {
		std::vector<CUnit *> table;
		if (args == 6) {
			Vec2i pos1;
			Vec2i pos2;
			CclGetPos(l, &pos1.x, &pos1.y, 3);
			CclGetPos(l, &pos2.x, &pos2.y, 4);
			if (pos1.x > pos2.x) {
				std::swap(pos1.x, pos2.x);
			}
			if (pos1.y > pos2.y) {
				std::swap(pos1.y, pos2.y);
			}
			if (any) {
				Select(pos1, pos2, table, HasSamePlayerAs(Players[oldp]));
			} else if (onlyUnits) {
				Select(pos1, pos2, table, AndPredicate(HasSamePlayerAs(Players[oldp]), NotPredicate(IsBuildingType())));
			} else if (onlyBuildings) {
				Select(pos1, pos2, table, AndPredicate(HasSamePlayerAs(Players[oldp]), IsBuildingType()));
			} else {
				Select(pos1, pos2, table, HasSamePlayerAndTypeAs(Players[oldp], *type));
			}
			for (size_t i = 0; i != table.size() && cnt > 0; ++i) {
				table[i]->ChangeOwner(Players[newp]);
				assignedCnt++;
				cnt--;
			}
		} else {
			std::vector<CUnit *> table;
			for (std::vector<CUnit *>::const_iterator it = Players[oldp].UnitBegin(); it != Players[oldp].UnitEnd() && cnt > 0; ++it) {
				CUnit *unit = *it;
				if (any || (onlyUnits && !unit->Type->Building) || (onlyBuildings && unit->Type->Building) || (type == unit->Type)) {
					table.push_back(unit);
				}
			}
			for (auto unit : table) {
				unit->ChangeOwner(Players[newp]);
			}
			assignedCnt = table.size();
		}
	}

	lua_pushnumber(l, assignedCnt);
	return 1;
}

/**
** <b>Description</b>
**
**  Get ThisPlayer.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>GetThisPlayer</strong>()</code></div>
*/
static int CclGetThisPlayer(lua_State *l)
{
	LuaCheckArgs(l, 0);
	if (ThisPlayer) {
		lua_pushnumber(l, ThisPlayer - Players);
	} else {
		lua_pushnumber(l, 0);
	}
	return 1;
}

/**
** <b>Description</b>
**
**  Set ThisPlayer.
**
**  @param l  Lua state.
*/
static int CclSetThisPlayer(lua_State *l)
{
	LuaCheckArgs(l, 1);
	int plynr = LuaToNumber(l, 1);
	ThisPlayer = &Players[plynr];

	lua_pushnumber(l, plynr);
	return 1;
}

/**
** <b>Description</b>
**
**  Set the maximum amount of units that can be selected.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>-- 9 units can be selected together.
**		  <strong>SetMaxSelectable</strong>(9)
**		  -- 18 units can be selected together.
**		  <strong>SetMaxSelectable</strong>(18)
**		  -- 50 units can be selected together.
**		  <strong>SetMaxSelectable</strong>(50)</code></div>
*/
static int CclSetMaxSelectable(lua_State *l)
{
	LuaCheckArgs(l, 1);
	MaxSelectable = LuaToNumber(l, 1);

	lua_pushnumber(l, MaxSelectable);
	return 1;
}

/**
** <b>Description</b>
**
**  Set players units limit.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>SetAllPlayersUnitLimit</strong>(200)</code></div>
*/
static int CclSetAllPlayersUnitLimit(lua_State *l)
{
	LuaCheckArgs(l, 1);
	for (int i = 0; i < PlayerMax; ++i) {
		Players[i].UnitLimit = LuaToNumber(l, 1);
	}

	lua_pushnumber(l, lua_tonumber(l, 1));
	return 1;
}

/**
** <b>Description</b>
**
**  Set players buildings limit.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>SetAllPlayersBuildingLimit</strong>(200)</code></div>
*/
static int CclSetAllPlayersBuildingLimit(lua_State *l)
{
	LuaCheckArgs(l, 1);
	for (int i = 0; i < PlayerMax; ++i) {
		Players[i].BuildingLimit = LuaToNumber(l, 1);
	}

	lua_pushnumber(l, lua_tonumber(l, 1));
	return 1;
}

/**
** <b>Description</b>
**
**  Set players total units limit.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>SetAllPlayersTotalUnitLimit</strong>(400)</code></div>
*/
static int CclSetAllPlayersTotalUnitLimit(lua_State *l)
{
	LuaCheckArgs(l, 1);
	for (int i = 0; i < PlayerMax; ++i) {
		Players[i].TotalUnitLimit = LuaToNumber(l, 1);
	}

	lua_pushnumber(l, lua_tonumber(l, 1));
	return 1;
}

/**
** <b>Description</b>
**
**  Change the diplomacy from player to another player.
**
**  @param l  Lua state.
**
**  @return FIXME: should return old state.
**
** Example:
**
** <div class="example"><code><strong>SetDiplomacy</strong>(0,"allied",1)
**		<strong>SetDiplomacy</strong>(1,"allied",0)
**		
**		<strong>SetDiplomacy</strong>(0,"enemy",2)
**		<strong>SetDiplomacy</strong>(1,"enemy",2)</code></div>
*/
static int CclSetDiplomacy(lua_State *l)
{
	LuaCheckArgs(l, 3);
	const int base = LuaToNumber(l, 1);
	const int plynr = LuaToNumber(l, 3);
	const char *state = LuaToString(l, 2);

	if (!strcmp(state, "allied")) {
		SendCommandDiplomacy(base, DiplomacyAllied, plynr);
	} else if (!strcmp(state, "neutral")) {
		SendCommandDiplomacy(base, DiplomacyNeutral, plynr);
	} else if (!strcmp(state, "crazy")) {
		SendCommandDiplomacy(base, DiplomacyCrazy, plynr);
	} else if (!strcmp(state, "enemy")) {
		SendCommandDiplomacy(base, DiplomacyEnemy, plynr);
	}
	return 0;
}

/**
**  Change the diplomacy from ThisPlayer to another player.
**
**  @param l  Lua state.
*/
static int CclDiplomacy(lua_State *l)
{
	lua_pushnumber(l, ThisPlayer->Index);
	lua_insert(l, 1);
	return CclSetDiplomacy(l);
}

/**
**  Get diplomacy from one player to another. Returns the strings "allied",
**  "enemy", "neutral", or "crazy".
*/
static int CclGetDiplomacy(lua_State *l)
{
	LuaCheckArgs(l, 2);
	const int base = LuaToNumber(l, 1);
	const int plynr = LuaToNumber(l, 2);

	if (Players[base].IsEnemy(plynr)) {
		if (Players[base].IsAllied(plynr)) {
			lua_pushstring(l, "crazy");
		} else {
			lua_pushstring(l, "enemy");
		}
	} else if (Players[base].IsAllied(plynr)) {
		lua_pushstring(l, "allied");
	} else {
		lua_pushstring(l, "neutral");
	}
	return 1;
}

/**
** <b>Description</b>
**
**  Change the shared vision from player to another player.
**
**
**  @param l  Lua state.
**
**  @return FIXME: should return old state.
**
** Example:
**
** <div class="example"><code><strong>SetSharedVision</strong>(0,true,1)
**		<strong>SetSharedVision</strong>(1,true,0)
**		
**		<strong>SetSharedVision</strong>(0,false,2)
**		<strong>SetSharedVision</strong>(1,false,2)</code></div>
*/
static int CclSetSharedVision(lua_State *l)
{
	LuaCheckArgs(l, 3);

	const int base = LuaToNumber(l, 1);
	const bool shared = LuaToBoolean(l, 2);
	const int plynr = LuaToNumber(l, 3);

	SendCommandSharedVision(base, shared, plynr);

	return 0;
}

/**
**  Change the shared vision from ThisPlayer to another player.
**
**  @param l  Lua state.
*/
static int CclSharedVision(lua_State *l)
{
	lua_pushnumber(l, ThisPlayer->Index);
	lua_insert(l, 1);
	return CclSetSharedVision(l);
}

/**
** <b>Description</b>
**
**  Change the players revelation type - reveal all units, only buidings or don't reveal anything
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>SetRevelationType</strong>("no-revelation")
** 		<strong>SetRevelationType</strong>("buildings-only")
** 		<strong>SetRevelationType</strong>("all-units")</code></div>
**
*/
static int CclSetRevelationType(lua_State *l)
{
	LuaCheckArgs(l, 1);

	const char *revel_type = LuaToString(l, 1);
	if (!strcmp(revel_type, "no-revelation")) {
		CPlayer::SetRevelationType(RevealTypes::cNoRevelation);
	} else if (!strcmp(revel_type, "all-units")) {
		CPlayer::SetRevelationType(RevealTypes::cAllUnits);
	} else if (!strcmp(revel_type, "buildings-only")) {
		CPlayer::SetRevelationType(RevealTypes::cBuildingsOnly);
	} else {
		PrintFunction();
		fprintf(stdout, "Accessible types of revelation \"no-revelation\", \"all-units\" and \"buildings-only\".\n");
		return 1;
	}
	return 0;
}

/**
** <b>Description</b>
**
**  Define race names in addition to those already there.
**
**  @param l  Lua state.
*/
static int CclDefineNewRaceNames(lua_State *l)
{
	int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		const char *value = LuaToString(l, j + 1);
		if (!strcmp(value, "race")) {
			++j;
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			int subargs = lua_rawlen(l, j + 1);
			int i = PlayerRaces.Count++;
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, j + 1, k + 1);
				if (!strcmp(value, "name")) {
					++k;
					PlayerRaces.Name[i] = LuaToString(l, j + 1, k + 1);
				} else if (!strcmp(value, "display")) {
					++k;
					PlayerRaces.Display[i] = LuaToString(l, j + 1, k + 1);
				} else if (!strcmp(value, "visible")) {
					PlayerRaces.Visible[i] = 1;
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	return 0;
}

/**
** Define race names
**
** @param l Lua state.
*/
static int CclDefineRaceNames(lua_State *l)
{
	PlayerRaces.Clean();
	return CclDefineNewRaceNames(l);
}

/**
** <b>Description</b>
**
**  Define player colors. Pass "false" as an optional second
**  argument to add the colors to the existing ones.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>DefinePlayerColors</strong>({
**		"red", {{164, 0, 0}, {124, 0, 0}, {92, 4, 0}, {68, 4, 0}},
**		"blue", {{12, 72, 204}, {4, 40, 160}, {0, 20, 116}, {0, 4, 76}},
**		"green", {{44, 180, 148}, {20, 132, 92}, {4, 84, 44}, {0, 40, 12}},
**		"violet", {{152, 72, 176}, {116, 44, 132}, {80, 24, 88}, {44, 8, 44}},
**		"orange", {{248, 140, 20}, {200, 96, 16}, {152, 60, 16}, {108, 32, 12}},
**		"black", {{40, 40, 60}, {28, 28, 44}, {20, 20, 32}, {12, 12, 20}},
**		"white", {{224, 224, 224}, {152, 152, 180}, {84, 84, 128}, {36, 40, 76}},
**		"yellow", {{252, 252, 72}, {228, 204, 40}, {204, 160, 16}, {180, 116, 0}},
**		"red", {{164, 0, 0}, {124, 0, 0}, {92, 4, 0}, {68, 4, 0}},
**		"blue", {{12, 72, 204}, {4, 40, 160}, {0, 20, 116}, {0, 4, 76}},
**		"green", {{44, 180, 148}, {20, 132, 92}, {4, 84, 44}, {0, 40, 12}},
**		"violet", {{152, 72, 176}, {116, 44, 132}, {80, 24, 88}, {44, 8, 44}},
**		"orange", {{248, 140, 20}, {200, 96, 16}, {152, 60, 16}, {108, 32, 12}},
**		"black", {{40, 40, 60}, {28, 28, 44}, {20, 20, 32}, {12, 12, 20}},
**		"white", {{224, 224, 224}, {152, 152, 180}, {84, 84, 128}, {36, 40, 76}},
**		"yellow", {{252, 252, 72}, {228, 204, 40}, {204, 160, 16}, {180, 116, 0}},
**	})</code></div>
*/
static int CclDefinePlayerColors(lua_State *l)
{
	int nargs = lua_gettop(l);
	if (nargs < 1 || nargs > 2) {
		LuaError(l, "wrong number of arguments");
	}
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	bool defaultNeutralPlayer = false;
	const int args = lua_rawlen(l, 1);
	if (nargs < 2 || LuaToBoolean(l, 2)) {
		PlayerColorNames.clear();
		PlayerColorsRGB.clear();
		PlayerColorsSDL.clear();
		if (args / 2 < PlayerMax - 1) { // accept no color for neutral player
			LuaError(l, "You need to define at least %d colors" _C_ PlayerMax - 1);
		}
		if (args / 2 < PlayerMax) {
			defaultNeutralPlayer = true;
		}
	}

	for (int i = 0; i < args; ++i) {
		PlayerColorNames.push_back(LuaToString(l, 1, i + 1));
		++i;
		lua_rawgeti(l, 1, i + 1);
		if (!lua_istable(l, -1)) {
			LuaError(l, "incorrect argument");
		}
		const int numcolors = lua_rawlen(l, -1);
		if (numcolors != PlayerColorIndexCount) {
			LuaError(l, "You should use %d colors (See DefinePlayerColorIndex())" _C_ PlayerColorIndexCount);
		}
		std::vector<CColor> newColors;
		for (int j = 0; j < numcolors; ++j) {
			lua_rawgeti(l, -1, j + 1);
			CColor newColor;
			newColor.Parse(l);
			newColors.push_back(newColor);
			lua_pop(l, 1);
		}
		PlayerColorsRGB.push_back(newColors);
		PlayerColorsSDL.push_back(std::vector<SDL_Color>(newColors.begin(), newColors.end()));
	}

	if (defaultNeutralPlayer) {
		PlayerColorNames.push_back("neutral-black");
		std::vector<CColor> neutralColors;
		for (int j = 0; j < PlayerColorIndexCount; ++j) {
			CColor neutralColor;
			neutralColors.push_back(neutralColor);
		}
		PlayerColorsRGB.push_back(neutralColors);
		PlayerColorsSDL.push_back(std::vector<SDL_Color>(neutralColors.begin(), neutralColors.end()));
	}

	return 0;
} 

/**
**  Make new player colors
**
**  @param l  Lua state.
*/
static int CclNewPlayerColors(lua_State *l)
{
	LuaCheckArgs(l, 0);
	SetPlayersPalette();

	return 0;
}

/**
**  Define player color indexes
**
**  @param l  Lua state.
*/
static int CclDefinePlayerColorIndex(lua_State *l)
{
	LuaCheckArgs(l, 2);
	PlayerColorIndexStart = LuaToNumber(l, 1);
	PlayerColorIndexCount = LuaToNumber(l, 2);

	PlayerColorsRGB.clear();
	PlayerColorsSDL.clear();
	return 0;
}

// ----------------------------------------------------------------------------

/**
** <b>Description</b>
**
**  Get player data.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>GetPlayerData</strong>(0,"TotalNumUnits")</code></div>
*/
static int CclGetPlayerData(lua_State *l)
{
	if (lua_gettop(l) < 2) {
		LuaError(l, "incorrect argument");
	}
	lua_pushvalue(l, 1);
	const CPlayer *p = CclGetPlayer(l);
	lua_pop(l, 1);
	const char *data = LuaToString(l, 2);

	if (!strcmp(data, "Name")) {
		lua_pushstring(l, p->Name.c_str());
		return 1;
	} else if (!strcmp(data, "RaceName")) {
		lua_pushstring(l, PlayerRaces.Name[p->Race].c_str());
		return 1;
	} else if (!strcmp(data, "Resources")) {
		LuaCheckArgs(l, 3);

		const std::string res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res.c_str());
		lua_pushnumber(l, p->Resources[resId] + p->StoredResources[resId]);
		return 1;
	} else if (!strcmp(data, "StoredResources")) {
		LuaCheckArgs(l, 3);

		const std::string res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res.c_str());
		lua_pushnumber(l, p->StoredResources[resId]);
		return 1;
	} else if (!strcmp(data, "MaxResources")) {
		LuaCheckArgs(l, 3);

		const std::string res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res.c_str());
		lua_pushnumber(l, p->MaxResources[resId]);
		return 1;
	} else if (!strcmp(data, "UnitTypesCount")) {
		LuaCheckArgs(l, 3);
		CUnitType *type = CclGetUnitType(l);
		Assert(type);
		lua_pushnumber(l, p->UnitTypesCount[type->Slot]);
		return 1;
	} else if (!strcmp(data, "UnitTypesAiActiveCount")) {
		LuaCheckArgs(l, 3);
		CUnitType *type = CclGetUnitType(l);
		Assert(type);
		lua_pushnumber(l, p->UnitTypesAiActiveCount[type->Slot]);
		return 1;
	} else if (!strcmp(data, "AiEnabled")) {
		lua_pushboolean(l, p->AiEnabled);
		return 1;
	} else if (!strcmp(data, "TotalNumUnits")) {
		lua_pushnumber(l, p->GetUnitCount());
		return 1;
	} else if (!strcmp(data, "NumBuildings")) {
		lua_pushnumber(l, p->NumBuildings);
		return 1;
	} else if (!strcmp(data, "Supply")) {
		lua_pushnumber(l, p->Supply);
		return 1;
	} else if (!strcmp(data, "Demand")) {
		lua_pushnumber(l, p->Demand);
		return 1;
	} else if (!strcmp(data, "UnitLimit")) {
		lua_pushnumber(l, p->UnitLimit);
		return 1;
	} else if (!strcmp(data, "BuildingLimit")) {
		lua_pushnumber(l, p->BuildingLimit);
		return 1;
	} else if (!strcmp(data, "TotalUnitLimit")) {
		lua_pushnumber(l, p->TotalUnitLimit);
		return 1;
	} else if (!strcmp(data, "Score")) {
		lua_pushnumber(l, p->Score);
		return 1;
	} else if (!strcmp(data, "TotalUnits")) {
		lua_pushnumber(l, p->TotalUnits);
		return 1;
	} else if (!strcmp(data, "TotalBuildings")) {
		lua_pushnumber(l, p->TotalBuildings);
		return 1;
	} else if (!strcmp(data, "TotalResources")) {
		LuaCheckArgs(l, 3);

		const std::string res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res.c_str());
		lua_pushnumber(l, p->TotalResources[resId]);
		return 1;
	} else if (!strcmp(data, "TotalRazings")) {
		lua_pushnumber(l, p->TotalRazings);
		return 1;
	} else if (!strcmp(data, "TotalKills")) {
		lua_pushnumber(l, p->TotalKills);
		return 1;
	} else if (!strcmp(data, "SpeedResourcesHarvest")) {
		LuaCheckArgs(l, 3);

		const std::string res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res.c_str());
		lua_pushnumber(l, p->SpeedResourcesHarvest[resId]);
		return 1;
	} else if (!strcmp(data, "SpeedResourcesReturn")) {
		LuaCheckArgs(l, 3);

		const std::string res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res.c_str());
		lua_pushnumber(l, p->SpeedResourcesReturn[resId]);
		return 1;
	} else if (!strcmp(data, "SpeedBuild")) {
		lua_pushnumber(l, p->SpeedBuild);
		return 1;
	} else if (!strcmp(data, "SpeedTrain")) {
		lua_pushnumber(l, p->SpeedTrain);
		return 1;
	} else if (!strcmp(data, "SpeedUpgrade")) {
		lua_pushnumber(l, p->SpeedUpgrade);
		return 1;
	} else if (!strcmp(data, "SpeedResearch")) {
		lua_pushnumber(l, p->SpeedResearch);
		return 1;
	} else if (!strcmp(data, "Allow")) {
		LuaCheckArgs(l, 3);
		const char *ident = LuaToString(l, 3);
		if (!strncmp(ident, "unit-", 5)) {
			int id = UnitTypeIdByIdent(ident);
			if (UnitIdAllowed(Players[p->Index], id) > 0) {
				lua_pushstring(l, "A");
			} else if (UnitIdAllowed(Players[p->Index], id) == 0) {
				lua_pushstring(l, "F");
			}
		} else if (!strncmp(ident, "upgrade-", 8)) {
			if (UpgradeIdentAllowed(Players[p->Index], ident) == 'A') {
				lua_pushstring(l, "A");
			} else if (UpgradeIdentAllowed(Players[p->Index], ident) == 'R') {
				lua_pushstring(l, "R");
			} else if (UpgradeIdentAllowed(Players[p->Index], ident) == 'F') {
				lua_pushstring(l, "F");
			}
		} else {
			DebugPrint(" wrong ident %s\n" _C_ ident);
		}
		return 1;
	} else {
		LuaError(l, "Invalid field: %s" _C_ data);
	}

	return 0;
}

/**
** <b>Description</b>
**
**  Set player data.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>
**  <strong>SetPlayerData</strong>(0,"Name","Nation of Stromgarde") -- set the name of this player
**	<strong>SetPlayerData</strong>(0,"RaceName","human") -- the the race to human
** 	<strong>SetPlayerData</strong>(0,"Resources","gold",1700) -- set the player to have 1700 gold
**  <strong>SetPlayerData</strong>(0, "Allow", "upgrade-paladin", "R") -- give the player the Paladin upgrade
** </code></div>
*/
static int CclSetPlayerData(lua_State *l)
{
	if (lua_gettop(l) < 3) {
		LuaError(l, "incorrect argument");
	}
	lua_pushvalue(l, 1);
	CPlayer *p = CclGetPlayer(l);
	lua_pop(l, 1);
	const char *data = LuaToString(l, 2);

	if (!strcmp(data, "Name")) {
		p->SetName(LuaToString(l, 3));
	} else if (!strcmp(data, "RaceName")) {
		const char *racename = LuaToString(l, 3);
		p->Race = PlayerRaces.GetRaceIndexByName(racename);

		if (p->Race == -1) {
			LuaError(l, "invalid race name '%s'" _C_ racename);
		}
	} else if (!strcmp(data, "Resources")) {
		LuaCheckArgs(l, 4);

		const std::string res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res.c_str());
		p->SetResource(resId, LuaToNumber(l, 4));
	} else if (!strcmp(data, "StoredResources")) {
		LuaCheckArgs(l, 4);

		const std::string res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res.c_str());
		p->SetResource(resId, LuaToNumber(l, 4), STORE_BUILDING);
		// } else if (!strcmp(data, "UnitTypesCount")) {
		// } else if (!strcmp(data, "AiEnabled")) {
		// } else if (!strcmp(data, "TotalNumUnits")) {
		// } else if (!strcmp(data, "NumBuildings")) {
		// } else if (!strcmp(data, "Supply")) {
		// } else if (!strcmp(data, "Demand")) {
	} else if (!strcmp(data, "UnitLimit")) {
		p->UnitLimit = LuaToNumber(l, 3);
	} else if (!strcmp(data, "BuildingLimit")) {
		p->BuildingLimit = LuaToNumber(l, 3);
	} else if (!strcmp(data, "TotalUnitLimit")) {
		p->TotalUnitLimit = LuaToNumber(l, 3);
	} else if (!strcmp(data, "Score")) {
		p->Score = LuaToNumber(l, 3);
	} else if (!strcmp(data, "TotalUnits")) {
		p->TotalUnits = LuaToNumber(l, 3);
	} else if (!strcmp(data, "TotalBuildings")) {
		p->TotalBuildings = LuaToNumber(l, 3);
	} else if (!strcmp(data, "TotalResources")) {
		LuaCheckArgs(l, 4);

		const std::string res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res.c_str());
		p->TotalResources[resId] = LuaToNumber(l, 4);
	} else if (!strcmp(data, "TotalRazings")) {
		p->TotalRazings = LuaToNumber(l, 3);
	} else if (!strcmp(data, "TotalKills")) {
		p->TotalKills = LuaToNumber(l, 3);
	} else if (!strcmp(data, "SpeedResourcesHarvest")) {
		LuaCheckArgs(l, 4);

		const std::string res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res.c_str());
		p->SpeedResourcesHarvest[resId] = LuaToNumber(l, 4);
	} else if (!strcmp(data, "SpeedResourcesReturn")) {
		LuaCheckArgs(l, 4);

		const std::string res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res.c_str());
		p->SpeedResourcesReturn[resId] = LuaToNumber(l, 4);
	} else if (!strcmp(data, "SpeedBuild")) {
		p->SpeedBuild = LuaToNumber(l, 3);
	} else if (!strcmp(data, "SpeedTrain")) {
		p->SpeedTrain = LuaToNumber(l, 3);
	} else if (!strcmp(data, "SpeedUpgrade")) {
		p->SpeedUpgrade = LuaToNumber(l, 3);
	} else if (!strcmp(data, "SpeedResearch")) {
		p->SpeedResearch = LuaToNumber(l, 3);
	} else if (!strcmp(data, "Allow")) {
		LuaCheckArgs(l, 4);
		const char *ident = LuaToString(l, 3);
		const std::string acquire = LuaToString(l, 4);

		if (!strncmp(ident, "upgrade-", 8)) {
			if (acquire == "R" && UpgradeIdentAllowed(*p, ident) != 'R') {
				UpgradeAcquire(*p, CUpgrade::Get(ident));
			} else if (acquire == "F" || acquire == "A") {
				if (UpgradeIdentAllowed(*p, ident) == 'R') {
					UpgradeLost(*p, CUpgrade::Get(ident)->ID);
				}
				AllowUpgradeId(*p, UpgradeIdByIdent(ident), acquire[0]);
			}
		} else {
			LuaError(l, " wrong ident %s\n" _C_ ident);
		}
	} else {
		LuaError(l, "Invalid field: %s" _C_ data);
	}

	return 0;
}

/**
** <b>Description</b> 
** 
**  Set ai player algo.
**
**  @param l  Lua state.
** 
** Example: 
** 
** <div class="example"> <code> -- Player 1 has a passive A.I 
**		  <strong>SetAiType</strong>(1, "Passive")</code></div>
*/
static int CclSetAiType(lua_State *l)
{
	CPlayer *p;

	if (lua_gettop(l) < 2) {
		LuaError(l, "incorrect argument");
	}
	lua_pushvalue(l, 1);
	p = CclGetPlayer(l);
	lua_pop(l, 1);

	p->AiName = LuaToString(l, 2);

	return 0;
}

// ----------------------------------------------------------------------------

/**
**  Register CCL features for players.
*/
void PlayerCclRegister()
{
	lua_register(Lua, "Player", CclPlayer);
	lua_register(Lua, "ChangeUnitsOwner", CclChangeUnitsOwner);
	lua_register(Lua, "GiveUnitsToPlayer", CclGiveUnitsToPlayer);
	lua_register(Lua, "GetThisPlayer", CclGetThisPlayer);
	lua_register(Lua, "SetThisPlayer", CclSetThisPlayer);

	lua_register(Lua, "SetMaxSelectable", CclSetMaxSelectable);

	lua_register(Lua, "SetAllPlayersUnitLimit", CclSetAllPlayersUnitLimit);
	lua_register(Lua, "SetAllPlayersBuildingLimit", CclSetAllPlayersBuildingLimit);
	lua_register(Lua, "SetAllPlayersTotalUnitLimit", CclSetAllPlayersTotalUnitLimit);

	lua_register(Lua, "SetDiplomacy", CclSetDiplomacy);
	lua_register(Lua, "GetDiplomacy", CclGetDiplomacy);
	lua_register(Lua, "Diplomacy", CclDiplomacy);
	lua_register(Lua, "SetSharedVision", CclSetSharedVision);
	lua_register(Lua, "SharedVision", CclSharedVision);
	
	lua_register(Lua, "SetRevelationType", CclSetRevelationType);

	lua_register(Lua, "DefineRaceNames", CclDefineRaceNames);
	lua_register(Lua, "DefineNewRaceNames", CclDefineRaceNames);
	lua_register(Lua, "DefinePlayerColors", CclDefinePlayerColors);
	lua_register(Lua, "DefinePlayerColorIndex", CclDefinePlayerColorIndex);

	lua_register(Lua, "NewColors", CclNewPlayerColors);

	// player member access functions
	lua_register(Lua, "GetPlayerData", CclGetPlayerData);
	lua_register(Lua, "SetPlayerData", CclSetPlayerData);
	lua_register(Lua, "SetAiType", CclSetAiType);
}

//@}
