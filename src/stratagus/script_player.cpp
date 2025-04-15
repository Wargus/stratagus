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
#include "script_sol.h"
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
CPlayer *CclGetPlayer(lua_State *l)
{
	if (lua_isnumber(l, -1)) {
		const int plynr = LuaToNumber(l, -1);
		if (plynr < 0 || plynr > PlayerMax) {
			LuaError(l, "bad player: %d", plynr);
		}
		return &Players[plynr];
	}
	const std::string_view player = LuaToString(l, -1);
	if (player == "any") {
		return nullptr;
	} else if (player == "this") {
		Assert(ThisPlayer);
		return ThisPlayer;
	}
	LuaError(l, "bad player: %s", player.data());
	ExitFatal(0);
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
		std::string_view value = LuaToString(l, j + 1);
		++j;

		if (value == "name") {
			this->SetName(std::string{LuaToString(l, j + 1)});
		} else if (value == "type") {
			value = LuaToString(l, j + 1);
			if (value == "neutral") {
				this->Type = PlayerTypes::PlayerNeutral;
			} else if (value == "nobody") {
				this->Type = PlayerTypes::PlayerNobody;
			} else if (value == "computer") {
				this->Type = PlayerTypes::PlayerComputer;
			} else if (value == "person") {
				this->Type = PlayerTypes::PlayerPerson;
			} else if (value == "rescue-passive") {
				this->Type = PlayerTypes::PlayerRescuePassive;
			} else if (value == "rescue-active") {
				this->Type = PlayerTypes::PlayerRescueActive;
			} else {
				LuaError(l, "Unsupported tag: %s", value.data());
			}
		} else if (value == "race") {
			const std::string_view raceName = LuaToString(l, j + 1);
			this->Race = PlayerRaces.GetRaceIndexByName(raceName);
			if (this->Race == -1) {
				LuaError(l, "Unsupported race: %s", raceName.data());
			}
		} else if (value == "ai-name") {
			this->AiName = LuaToString(l, j + 1);
		} else if (value == "team") {
			this->Team = LuaToNumber(l, j + 1);
		} else if (value == "enemy") {
			value = LuaToString(l, j + 1);
			for (unsigned int i = 0; i < PlayerMax && i < value.size(); ++i) {
				if (value[i] == '-' || value[i] == '_' || value[i] == ' ') {
					this->Enemy &= ~(1 << i);
				} else {
					this->Enemy |= (1 << i);
				}
			}
		} else if (value == "allied") {
			value = LuaToString(l, j + 1);
			for (unsigned int i = 0; i < PlayerMax && i < value.size(); ++i) {
				if (value[i] == '-' || value[i] == '_' || value[i] == ' ') {
					this->Allied &= ~(1 << i);
				} else {
					this->Allied |= (1 << i);
				}
			}
		} else if (value == "shared-vision") {
			value = LuaToString(l, j + 1);
			for (unsigned int i = 0; i < PlayerMax && i < value.size(); ++i) {
				if (i == static_cast<unsigned int>(this->Index)) {
					continue;
				}
				if (value[i] == '-' || value[i] == '_' || value[i] == ' ') {
					this->UnshareVisionWith(Players[i]);
				} else {
					this->ShareVisionWith(Players[i]);
				}
			}
		} else if (value == "start") {
			CclGetPos(l, &this->StartPos, j + 1);
		} else if (value == "resources") {
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
		} else if (value == "stored-resources") {
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
		} else if (value == "max-resources") {
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
		} else if (value == "last-resources") {
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
		} else if (value == "incomes") {
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
		} else if (value == "revenue") {
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
		} else if (value == "ai-enabled") {
			this->AiEnabled = true;
			--j;
		} else if (value == "ai-disabled") {
			this->AiEnabled = false;
			--j;
		} else if (value == "revealed") {
			this->SetRevealed(true);
			--j;
		} else if (value == "supply") {
			this->Supply = LuaToNumber(l, j + 1);
		} else if (value == "demand") {
			this->Demand = LuaToNumber(l, j + 1);
		} else if (value == "unit-limit") {
			this->UnitLimit = LuaToNumber(l, j + 1);
		} else if (value == "building-limit") {
			this->BuildingLimit = LuaToNumber(l, j + 1);
		} else if (value == "total-unit-limit") {
			this->TotalUnitLimit = LuaToNumber(l, j + 1);
		} else if (value == "score") {
			this->Score = LuaToNumber(l, j + 1);
		} else if (value == "total-units") {
			this->TotalUnits = LuaToNumber(l, j + 1);
		} else if (value == "total-buildings") {
			this->TotalBuildings = LuaToNumber(l, j + 1);
		} else if (value == "total-razings") {
			this->TotalRazings = LuaToNumber(l, j + 1);
		} else if (value == "total-kills") {
			this->TotalKills = LuaToNumber(l, j + 1);
		} else if (value == "lost-main-facility-timer") {
			this->LostMainFacilityTimer = LuaToNumber(l, j + 1);
		} else if (value == "total-resources") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			if (subargs != MaxCosts) {
				LuaError(l, "Wrong number of total-resources: %d", subargs);
			}
			for (int k = 0; k < subargs; ++k) {
				this->TotalResources[k] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (value == "speed-resource-harvest") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			if (subargs != MaxCosts) {
				LuaError(l, "Wrong number of speed-resource-harvest: %d", subargs);
			}
			for (int k = 0; k < subargs; ++k) {
				this->SpeedResourcesHarvest[k] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (value == "speed-resource-return") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			if (subargs != MaxCosts) {
				LuaError(l, "Wrong number of speed-resource-harvest: %d", subargs);
			}
			for (int k = 0; k < subargs; ++k) {
				this->SpeedResourcesReturn[k] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (value == "speed-build") {
			this->SpeedBuild = LuaToNumber(l, j + 1);
		} else if (value == "speed-train") {
			this->SpeedTrain = LuaToNumber(l, j + 1);
		} else if (value == "speed-upgrade") {
			this->SpeedUpgrade = LuaToNumber(l, j + 1);
		} else if (value == "speed-research") {
			this->SpeedResearch = LuaToNumber(l, j + 1);
		} else if (value == "color") {
			if (!lua_istable(l, j + 1) || lua_rawlen(l, j + 1) != 3) {
				LuaError(l, "incorrect argument");
			}
			const int r = LuaToNumber(l, j + 1, 1);
			const int g = LuaToNumber(l, j + 1, 2);
			const int b = LuaToNumber(l, j + 1, 3);
			this->Color = Video.MapRGB(TheScreen->format, r, g, b);
		} else if (value == "timers") {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, j + 1);
			if (subargs != UpgradeMax) {
				LuaError(l, "Wrong upgrade timer length: %d", subargs);
			}
			for (int k = 0; k < subargs; ++k) {
				this->UpgradeTimers.Upgrades[k] = LuaToNumber(l, j + 1, k + 1);
			}
		} else {
			LuaError(l, "Unsupported tag: %s", value.data());
		}
	}
	// Manage max
	for (int i = 0; i < MaxCosts; ++i) {
		if (this->MaxResources[i] != -1) {
			this->SetResource(i, this->Resources[i] + this->StoredResources[i], EStoreType::Both);
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
    CclGetPos(l, &pos1, 1);
    CclGetPos(l, &pos2, 2);
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
        table = Select(pos1, pos2, HasSamePlayerAs(Players[oldp]));
    } else { //Change only specific units by the type.
        CUnitType &type = UnitTypeByIdent(LuaToString(l, 5));
        table = Select(pos1, pos2, HasSamePlayerAndTypeAs(Players[oldp], type));
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
**   -- Give 2 peasants from player 4 to player 1
**   GiveUnitsToPlayer(2, "unit-peasant", 4, 1)
**   -- Give 2 knights from player 4 to player 1 inside the rectangle 2,2 - 14,14
**   GiveUnitsToPlayer(2, "unit-knight", {2,2}, {14,14}, 4, 1)
**   -- Give any 4 units from player 4 to player 1
**   GiveUnitsToPlayer(2, "any", 4, 1)
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
		std::string_view cntStr = LuaToString(l, 1);
		if (cntStr != "all") {
			LuaError(l, "incorrect 1st argument to GiveUnitsToPlayer. Must be number or 'all'");
		}
		cnt = std::numeric_limits<int>::max();
	}

	const int oldp = LuaToNumber(l, args == 4 ? 3 : 5);
	const int newp = LuaToNumber(l, args == 4 ? 4 : 6);

	std::string_view typestr = LuaToString(l, 2);
	int assignedCnt = 0;

	CUnitType *type = nullptr;
	bool any = false;
	bool onlyUnits = false;
	bool onlyBuildings = false;
	if ((any = (typestr == "any"))) {
	} else if ((onlyUnits = (typestr == "unit"))) {
	} else if ((onlyBuildings = (typestr == "building"))) {
	} else {
		type = &UnitTypeByIdent(typestr);
	}

	if (cnt > 0) {
		if (args == 6) {
			Vec2i pos1;
			Vec2i pos2;
			CclGetPos(l, &pos1, 3);
			CclGetPos(l, &pos2, 4);
			if (pos1.x > pos2.x) {
				std::swap(pos1.x, pos2.x);
			}
			if (pos1.y > pos2.y) {
				std::swap(pos1.y, pos2.y);
			}
			std::vector<CUnit *> table;
			if (any) {
				table = Select(pos1, pos2, HasSamePlayerAs(Players[oldp]));
			} else if (onlyUnits) {
				table = Select(pos1, pos2, AndPredicate(HasSamePlayerAs(Players[oldp]), NotPredicate(IsBuildingType())));
			} else if (onlyBuildings) {
				table = Select(pos1, pos2, AndPredicate(HasSamePlayerAs(Players[oldp]), IsBuildingType()));
			} else {
				table = Select(pos1, pos2, HasSamePlayerAndTypeAs(Players[oldp], *type));
			}
			for (size_t i = 0; i != table.size() && cnt > 0; ++i) {
				table[i]->ChangeOwner(Players[newp]);
				assignedCnt++;
				cnt--;
			}
		} else {
			std::vector<CUnit *> table;
			for (CUnit *unit : Players[oldp].GetUnits()) {
				if (any || (onlyUnits && !unit->Type->Building) || (onlyBuildings && unit->Type->Building) || (type == unit->Type)) {
					table.push_back(unit);
					if (--cnt > 0) {
						break;
					}
				}
			}
			for (auto *unit : table) {
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
	const std::string_view state = LuaToString(l, 2);
	if (auto diplomacy = DiplomacyFromString(state)) {
		SendCommandDiplomacy(base, *diplomacy, plynr);
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
**  Get diplomacy from one player to another.
**  Returns one of "allied", "enemy", "neutral" or "crazy".
*/
static int CclGetDiplomacy(lua_State *l)
{
	LuaCheckArgs(l, 2);
	const int base = LuaToNumber(l, 1);
	const int plynr = LuaToNumber(l, 2);

	if (Players[base].IsEnemy(plynr)) {
		if (Players[base].IsAllied(plynr)) {
			lua_pushstring(l, ToString(EDiplomacy::Crazy).data());
		} else {
			lua_pushstring(l, ToString(EDiplomacy::Enemy).data());
		}
	} else if (Players[base].IsAllied(plynr)) {
		lua_pushstring(l, ToString(EDiplomacy::Allied).data());
	} else {
		lua_pushstring(l, ToString(EDiplomacy::Neutral).data());
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

	const std::string_view revel_type = LuaToString(l, 1);
	if (revel_type == "no-revelation") {
		CPlayer::SetRevelationType(RevealTypes::cNoRevelation);
	} else if (revel_type == "all-units") {
		CPlayer::SetRevelationType(RevealTypes::cAllUnits);
	} else if (revel_type == "buildings-only") {
		CPlayer::SetRevelationType(RevealTypes::cBuildingsOnly);
	} else {
		LuaError(l,
		         "Unknown revelation type '%s'\n"
		         "Accessible types of revelation "
		         "\"no-revelation\", \"all-units\" and \"buildings-only\".\n",
		         revel_type.data());
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
		std::string_view value = LuaToString(l, j + 1);
		if (value == "race") {
			++j;
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			int subargs = lua_rawlen(l, j + 1);
			int i = PlayerRaces.Count++;
			for (int k = 0; k < subargs; ++k) {
				value = LuaToString(l, j + 1, k + 1);
				if (value == "name") {
					++k;
					PlayerRaces.Name[i] = LuaToString(l, j + 1, k + 1);
				} else if (value == "display") {
					++k;
					PlayerRaces.Display[i] = LuaToString(l, j + 1, k + 1);
				} else if (value == "visible") {
					PlayerRaces.Visible[i] = 1;
				} else {
					LuaError(l, "Unsupported tag: %s", value.data());
				}
			}
		} else {
			LuaError(l, "Unsupported tag: %s", value.data());
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
			LuaError(l, "You need to define at least %d colors", PlayerMax - 1);
		}
		if (args / 2 < PlayerMax) {
			defaultNeutralPlayer = true;
		}
	}

	for (int i = 0; i < args; ++i) {
		PlayerColorNames.push_back(std::string{LuaToString(l, 1, i + 1)});
		++i;
		lua_rawgeti(l, 1, i + 1);
		if (!lua_istable(l, -1)) {
			LuaError(l, "incorrect argument");
		}
		const int numcolors = lua_rawlen(l, -1);
		if (numcolors != PlayerColorIndexCount) {
			LuaError(l, "You should use %d colors (See DefinePlayerColorIndex())", PlayerColorIndexCount);
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
	if (p == nullptr) {
		printf("GetPlayerData: You cannot use \"any\", specify a player\n");
		LuaError(l, "bad player");
		return 0;
	}
	const std::string_view data = LuaToString(l, 2);

	if (data == "Name") {
		lua_pushstring(l, p->Name.c_str());
		return 1;
	} else if (data == "RaceName") {
		lua_pushstring(l, PlayerRaces.Name[p->Race].c_str());
		return 1;
	} else if (data == "Resources") {
		LuaCheckArgs(l, 3);

		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		lua_pushnumber(l, p->Resources[resId] + p->StoredResources[resId]);
		return 1;
	} else if (data == "StoredResources") {
		LuaCheckArgs(l, 3);

		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		lua_pushnumber(l, p->StoredResources[resId]);
		return 1;
	} else if (data == "MaxResources") {
		LuaCheckArgs(l, 3);

		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		lua_pushnumber(l, p->MaxResources[resId]);
		return 1;
	} else if (data == "UnitTypesCount") {
		LuaCheckArgs(l, 3);
		CUnitType *type = CclGetUnitType(l);
		Assert(type);
		lua_pushnumber(l, p->UnitTypesCount[type->Slot]);
		return 1;
	} else if (data == "UnitTypesAiActiveCount") {
		LuaCheckArgs(l, 3);
		CUnitType *type = CclGetUnitType(l);
		Assert(type);
		lua_pushnumber(l, p->UnitTypesAiActiveCount[type->Slot]);
		return 1;
	} else if (data == "AiEnabled") {
		lua_pushboolean(l, p->AiEnabled);
		return 1;
	} else if (data == "TotalNumUnits") {
		lua_pushnumber(l, p->GetUnitCount());
		return 1;
	} else if (data == "NumBuildings") {
		lua_pushnumber(l, p->NumBuildings);
		return 1;
	} else if (data == "Supply") {
		lua_pushnumber(l, p->Supply);
		return 1;
	} else if (data == "Demand") {
		lua_pushnumber(l, p->Demand);
		return 1;
	} else if (data == "UnitLimit") {
		lua_pushnumber(l, p->UnitLimit);
		return 1;
	} else if (data == "BuildingLimit") {
		lua_pushnumber(l, p->BuildingLimit);
		return 1;
	} else if (data == "TotalUnitLimit") {
		lua_pushnumber(l, p->TotalUnitLimit);
		return 1;
	} else if (data == "Score") {
		lua_pushnumber(l, p->Score);
		return 1;
	} else if (data == "TotalUnits") {
		lua_pushnumber(l, p->TotalUnits);
		return 1;
	} else if (data == "TotalBuildings") {
		lua_pushnumber(l, p->TotalBuildings);
		return 1;
	} else if (data == "TotalResources") {
		LuaCheckArgs(l, 3);

		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		lua_pushnumber(l, p->TotalResources[resId]);
		return 1;
	} else if (data == "TotalRazings") {
		lua_pushnumber(l, p->TotalRazings);
		return 1;
	} else if (data == "TotalKills") {
		lua_pushnumber(l, p->TotalKills);
		return 1;
	} else if (data == "SpeedResourcesHarvest") {
		LuaCheckArgs(l, 3);

		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		lua_pushnumber(l, p->SpeedResourcesHarvest[resId]);
		return 1;
	} else if (data == "SpeedResourcesReturn") {
		LuaCheckArgs(l, 3);

		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		lua_pushnumber(l, p->SpeedResourcesReturn[resId]);
		return 1;
	} else if (data == "SpeedBuild") {
		lua_pushnumber(l, p->SpeedBuild);
		return 1;
	} else if (data == "SpeedTrain") {
		lua_pushnumber(l, p->SpeedTrain);
		return 1;
	} else if (data == "SpeedUpgrade") {
		lua_pushnumber(l, p->SpeedUpgrade);
		return 1;
	} else if (data == "SpeedResearch") {
		lua_pushnumber(l, p->SpeedResearch);
		return 1;
	} else if (data == "Allow") {
		LuaCheckArgs(l, 3);
		const std::string_view ident = LuaToString(l, 3);
		if (starts_with(ident, "unit-")) {
			int id = UnitTypeByIdent(ident).Slot;
			if (UnitIdAllowed(Players[p->Index], id) > 0) {
				lua_pushstring(l, "A");
			} else if (UnitIdAllowed(Players[p->Index], id) == 0) {
				lua_pushstring(l, "F");
			}
		} else if (starts_with(ident, "upgrade-")) {
			if (UpgradeIdentAllowed(Players[p->Index], ident) == 'A') {
				lua_pushstring(l, "A");
			} else if (UpgradeIdentAllowed(Players[p->Index], ident) == 'R') {
				lua_pushstring(l, "R");
			} else if (UpgradeIdentAllowed(Players[p->Index], ident) == 'F') {
				lua_pushstring(l, "F");
			}
		} else {
			LuaError(l, " wrong ident %s\n", ident.data());
		}
		return 1;
	} else {
		LuaError(l, "Invalid field: %s", data.data());
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
	if (p == nullptr) {
		printf("SetPlayerData: You cannot use \"any\", specify a player\n");
		LuaError(l, "bad player");
		return 0;
	}
	lua_pop(l, 1);
	const std::string_view data = LuaToString(l, 2);

	if (data == "Name") {
		p->SetName(std::string{LuaToString(l, 3)});
	} else if (data == "RaceName") {
		const std::string_view racename = LuaToString(l, 3);
		p->Race = PlayerRaces.GetRaceIndexByName(racename);

		if (p->Race == -1) {
			LuaError(l, "invalid race name '%s'", racename.data());
		}
	} else if (data == "Resources") {
		LuaCheckArgs(l, 4);

		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		p->SetResource(resId, LuaToNumber(l, 4));
	} else if (data == "StoredResources") {
		LuaCheckArgs(l, 4);

		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		p->SetResource(resId, LuaToNumber(l, 4), EStoreType::Building);
		// } else if (data == "UnitTypesCount") {
		// } else if (data == "AiEnabled") {
		// } else if (data == "TotalNumUnits") {
		// } else if (data == "NumBuildings") {
		// } else if (data == "Supply") {
		// } else if (data == "Demand") {
	} else if (data == "UnitLimit") {
		p->UnitLimit = LuaToNumber(l, 3);
	} else if (data == "BuildingLimit") {
		p->BuildingLimit = LuaToNumber(l, 3);
	} else if (data == "TotalUnitLimit") {
		p->TotalUnitLimit = LuaToNumber(l, 3);
	} else if (data == "Score") {
		p->Score = LuaToNumber(l, 3);
	} else if (data == "TotalUnits") {
		p->TotalUnits = LuaToNumber(l, 3);
	} else if (data == "TotalBuildings") {
		p->TotalBuildings = LuaToNumber(l, 3);
	} else if (data == "TotalResources") {
		LuaCheckArgs(l, 4);

		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		p->TotalResources[resId] = LuaToNumber(l, 4);
	} else if (data == "TotalRazings") {
		p->TotalRazings = LuaToNumber(l, 3);
	} else if (data == "TotalKills") {
		p->TotalKills = LuaToNumber(l, 3);
	} else if (data == "SpeedResourcesHarvest") {
		LuaCheckArgs(l, 4);

		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		p->SpeedResourcesHarvest[resId] = LuaToNumber(l, 4);
	} else if (data == "SpeedResourcesReturn") {
		LuaCheckArgs(l, 4);

		const std::string_view res = LuaToString(l, 3);
		const int resId = GetResourceIdByName(l, res);
		p->SpeedResourcesReturn[resId] = LuaToNumber(l, 4);
	} else if (data == "SpeedBuild") {
		p->SpeedBuild = LuaToNumber(l, 3);
	} else if (data == "SpeedTrain") {
		p->SpeedTrain = LuaToNumber(l, 3);
	} else if (data == "SpeedUpgrade") {
		p->SpeedUpgrade = LuaToNumber(l, 3);
	} else if (data == "SpeedResearch") {
		p->SpeedResearch = LuaToNumber(l, 3);
	} else if (data == "Allow") {
		LuaCheckArgs(l, 4);
		const std::string_view ident = LuaToString(l, 3);
		const std::string_view acquire = LuaToString(l, 4);

		if (starts_with(ident, "upgrade-")) {
			if (acquire == "R" && UpgradeIdentAllowed(*p, ident) != 'R') {
				UpgradeAcquire(*p, CUpgrade::Get(ident));
			} else if (acquire == "F" || acquire == "A") {
				if (UpgradeIdentAllowed(*p, ident) == 'R') {
					UpgradeLost(*p, CUpgrade::Get(ident)->ID);
				}
				AllowUpgradeId(*p, UpgradeIdByIdent(ident), acquire[0]);
			}
		} else {
			LuaError(l, " wrong ident %s\n", ident.data());
		}
	} else {
		LuaError(l, "Invalid field: %s", data.data());
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
	if (lua_gettop(l) < 2) {
		LuaError(l, "incorrect argument");
	}
	lua_pushvalue(l, 1);
	CPlayer *p = CclGetPlayer(l);
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
	sol::state_view luaSol(Lua);

	luaSol["Player"] = CclPlayer;
	luaSol["ChangeUnitsOwner"] = CclChangeUnitsOwner;
	luaSol["GiveUnitsToPlayer"] = CclGiveUnitsToPlayer;
	luaSol["GetThisPlayer"] = CclGetThisPlayer;
	luaSol["SetThisPlayer"] = CclSetThisPlayer;

	luaSol["SetMaxSelectable"] = CclSetMaxSelectable;

	luaSol["SetAllPlayersUnitLimit"] = CclSetAllPlayersUnitLimit;
	luaSol["SetAllPlayersBuildingLimit"] = CclSetAllPlayersBuildingLimit;
	luaSol["SetAllPlayersTotalUnitLimit"] = CclSetAllPlayersTotalUnitLimit;

	luaSol["SetDiplomacy"] = CclSetDiplomacy;
	luaSol["GetDiplomacy"] = CclGetDiplomacy;
	luaSol["Diplomacy"] = CclDiplomacy;
	luaSol["SetSharedVision"] = CclSetSharedVision;
	luaSol["SharedVision"] = CclSharedVision;

	luaSol["SetRevelationType"] = CclSetRevelationType;

	luaSol["DefineRaceNames"] = CclDefineRaceNames;
	luaSol["DefineNewRaceNames"] = CclDefineRaceNames;
	luaSol["DefinePlayerColors"] = CclDefinePlayerColors;
	luaSol["DefinePlayerColorIndex"] = CclDefinePlayerColorIndex;

	luaSol["NewColors"] = CclNewPlayerColors;

	// player member access functions
	luaSol["GetPlayerData"] = CclGetPlayerData;
	luaSol["SetPlayerData"] = CclSetPlayerData;
	luaSol["SetAiType"] = CclSetAiType;
}

//@}
