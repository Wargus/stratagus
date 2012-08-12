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
/**@name script_unit.cpp - The unit ccl functions. */
//
//      (c) Copyright 2001-2005 by Lutz Sammer and Jimmy Salmon
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
#include "unit.h"

#include "actions.h"
#include "animation.h"
#include "commands.h"
#include "construct.h"
#include "map.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "spells.h"
#include "trigger.h"
#include "unit_find.h"
#include "unit_manager.h"
#include "unittype.h"
#include "upgrade.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Get resource by name
extern unsigned CclGetResourceByName(lua_State *l);

/**
**  Set training queue
**
**  @param l  Lua state.
**
**  @return  The old state of the training queue
*/
static int CclSetTrainingQueue(lua_State *l)
{
	LuaCheckArgs(l, 1);
	EnableTrainingQueue = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Set capture buildings
**
**  @param l  Lua state.
**
**  @return   The old state of the flag
*/
static int CclSetBuildingCapture(lua_State *l)
{
	LuaCheckArgs(l, 1);
	EnableBuildingCapture = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Set reveal attacker
**
**  @param l  Lua state.
**
**  @return   The old state of the flag
*/
static int CclSetRevealAttacker(lua_State *l)
{
	LuaCheckArgs(l, 1);
	RevealAttacker = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Set cost multiplier to RepairCost for buildings additional workers helping (0 = no additional cost)
**
**  @param l  Lua state.
*/
static int CclResourcesMultiBuildersMultiplier(lua_State *l)
{
	LuaCheckArgs(l, 1);
	ResourcesMultiBuildersMultiplier = LuaToNumber(l, 1);
	return 0;
}

/**
**  Get a unit pointer
**
**  @param l  Lua state.
**
**  @return   The unit pointer
*/
static CUnit *CclGetUnit(lua_State *l)
{
	return &UnitManager.GetSlotUnit((int)LuaToNumber(l, -1));
}

/**
**  Get a unit pointer from ref string
**
**  @param l  Lua state.
**
**  @return   The unit pointer
*/
CUnit *CclGetUnitFromRef(lua_State *l)
{
	const char *const value = LuaToString(l, -1);
	unsigned int slot = strtol(value + 1, NULL, 16);
	Assert(slot < UnitManager.GetUsedSlotCount());
	return &UnitManager.GetSlotUnit(slot);
}


bool COrder::ParseGenericData(lua_State *l, int &j, const char *value)
{
	if (!strcmp(value, "finished")) {
		lua_rawgeti(l, -1, j + 1);
		this->Finished = true;
		lua_pop(l, 1);
	} else if (!strcmp(value, "goal")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->Goal = CclGetUnitFromRef(l);
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}



void PathFinderInput::Load(lua_State *l)
{
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument in PathFinderInput::Load");
	}
	const int args = 1 + lua_rawlen(l, -1);
	for (int i = 1; i < args; ++i) {
		lua_rawgeti(l, -1, i);
		const char *tag = LuaToString(l, -1);
		lua_pop(l, 1);
		++i;
		if (!strcmp(tag, "unit-size")) {
			lua_rawgeti(l, -1, i);
			CclGetPos(l, &this->unitSize.x, &this->unitSize.y);
			lua_pop(l, 1);
		} else if (!strcmp(tag, "goalpos")) {
			lua_rawgeti(l, -1, i);
			CclGetPos(l, &this->goalPos.x, &this->goalPos.y);
			lua_pop(l, 1);
		} else if (!strcmp(tag, "goal-size")) {
			lua_rawgeti(l, -1, i);
			CclGetPos(l, &this->goalSize.x, &this->goalSize.y);
			lua_pop(l, 1);
		} else if (!strcmp(tag, "minrange")) {
			lua_rawgeti(l, -1, i);
			this->minRange = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(tag, "maxrange")) {
			lua_rawgeti(l, -1, i);
			this->maxRange = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(tag, "invalid")) {
			this->isRecalculatePathNeeded = true;
			--i;
		} else {
			LuaError(l, "PathFinderInput::Load: Unsupported tag: %s" _C_ tag);
		}
	}
}


void PathFinderOutput::Load(lua_State *l)
{
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument in PathFinderOutput::Load");
	}
	const int args = 1 + lua_rawlen(l, -1);
	for (int i = 1; i < args; ++i) {
		lua_rawgeti(l, -1, i);
		const char *tag = LuaToString(l, -1);
		lua_pop(l, 1);
		++i;
		if (!strcmp(tag, "cycles")) {
			lua_rawgeti(l, -1, i);
			this->Cycles = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(tag, "fast")) {
			this->Fast = 1;
			--i;
		} else if (!strcmp(tag, "path")) {
			lua_rawgeti(l, -1, i);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument _");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				this->Path[k] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
			this->Length = subargs;
			lua_pop(l, 1);
		} else {
			LuaError(l, "PathFinderOutput::Load: Unsupported tag: %s" _C_ tag);
		}
	}
}

/**
**  Parse orders.
**
**  @param l     Lua state.
**  @param unit  Unit pointer which should get the orders.
*/
static void CclParseOrders(lua_State *l, CUnit &unit)
{
	for (std::vector<COrderPtr>::iterator order = unit.Orders.begin();
		 order != unit.Orders.end();
		 ++order) {
		delete *order;
	}
	unit.Orders.clear();
	const int n = lua_rawlen(l, -1);

	for (int j = 0; j < n; ++j) {
		lua_rawgeti(l, -1, j + 1);

		unit.Orders.push_back(NULL);
		COrderPtr *order = &unit.Orders.back();

		CclParseOrder(l, unit, order);
		lua_pop(l, 1);
	}
}

/**
**  Parse unit
**
**  @param l  Lua state.
**
**  @todo  Verify that vision table is always correct (transporter)
**  @todo (PlaceUnit() and host-info).
*/
static int CclUnit(lua_State *l)
{
	const int slot = LuaToNumber(l, 1);

	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	CUnit *unit = NULL;
	CUnitType *type = NULL;
	CUnitType *seentype = NULL;
	CPlayer *player = NULL;

	// Parse the list:
	const int args = lua_rawlen(l, 2);
	for (int j = 0; j < args; ++j) {
		lua_rawgeti(l, 2, j + 1);
		const char *value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;

		if (!strcmp(value, "type")) {
			lua_rawgeti(l, 2, j + 1);
			type = UnitTypeByIdent(LuaToString(l, -1));
			lua_pop(l, 1);
		} else if (!strcmp(value, "seen-type")) {
			lua_rawgeti(l, 2, j + 1);
			seentype = UnitTypeByIdent(LuaToString(l, -1));
			lua_pop(l, 1);
		} else if (!strcmp(value, "player")) {
			lua_rawgeti(l, 2, j + 1);
			player = &Players[(int)LuaToNumber(l, -1)];
			lua_pop(l, 1);

			// During a unit's death animation (when action is "die" but the
			// unit still has its original type, i.e. it's still not a corpse)
			// the unit is already removed from map and from player's
			// unit list (=the unit went through LetUnitDie() which
			// calls RemoveUnit() and UnitLost()).  Such a unit should not
			// be put on player's unit list!  However, this state is not
			// easily detected from this place.  It seems that it is
			// characterized by
			// unit->CurrentAction()==UnitActionDie so we have to wait
			// until we parsed at least Unit::Orders[].
			Assert(type);
			unit = &UnitManager.GetSlotUnit(slot);
			unit->Init(*type);
			unit->Seen.Type = seentype;
			unit->Active = 0;
			unit->Removed = 0;
			Assert(UnitNumber(*unit) == slot);
		} else if (!strcmp(value, "current-sight-range")) {
			lua_rawgeti(l, 2, j + 1);
			unit->CurrentSightRange = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "refs")) {
			lua_rawgeti(l, 2, j + 1);
			unit->Refs = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "host-info")) {
			int w;
			int h;

			lua_rawgeti(l, 2, j + 1);
			if (!lua_istable(l, -1) || lua_rawlen(l, -1) != 4) {
				LuaError(l, "incorrect argument");
			}
			Vec2i pos;
			lua_rawgeti(l, -1, 1);
			pos.x = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			pos.y = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 3);
			w = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 4);
			h = LuaToNumber(l, -1);
			lua_pop(l, 1);
			MapSight(*player, pos, w, h, unit->CurrentSightRange, MapMarkTileSight);
			// Detectcloak works in container
			if (unit->Type->DetectCloak) {
				MapSight(*player, pos, w, h, unit->CurrentSightRange, MapMarkTileDetectCloak);
			}
			// Radar(Jammer) not.
			lua_pop(l, 1);
		} else if (!strcmp(value, "tile")) {
			lua_rawgeti(l, 2, j + 1);
			CclGetPos(l, &unit->tilePos.x , &unit->tilePos.y, -1);
			lua_pop(l, 1);
			unit->Offset = Map.getIndex(unit->tilePos.x, unit->tilePos.y);
		} else if (!strcmp(value, "seen-tile")) {
			lua_rawgeti(l, 2, j + 1);
			CclGetPos(l, &unit->Seen.tilePos.x , &unit->Seen.tilePos.y, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "stats")) {
			lua_rawgeti(l, 2, j + 1);
			unit->Stats = &type->Stats[(int)LuaToNumber(l, -1)];
			lua_pop(l, 1);
		} else if (!strcmp(value, "pixel")) {
			lua_rawgeti(l, 2, j + 1);
			CclGetPos(l, &unit->IX , &unit->IY, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "seen-pixel")) {
			lua_rawgeti(l, 2, j + 1);
			CclGetPos(l, &unit->Seen.IX , &unit->Seen.IY, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "frame")) {
			lua_rawgeti(l, 2, j + 1);
			unit->Frame = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "seen")) {
			lua_rawgeti(l, 2, j + 1);
			unit->Seen.Frame = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "not-seen")) {
			unit->Seen.Frame = UnitNotSeen;
			--j;
		} else if (!strcmp(value, "direction")) {
			lua_rawgeti(l, 2, j + 1);
			unit->Direction = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "damage-type")) {
			lua_rawgeti(l, 2, j + 1);
			unit->DamagedType = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "attacked")) {
			lua_rawgeti(l, 2, j + 1);
			// FIXME : unsigned long should be better handled
			unit->Attacked = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "auto-repair")) {
			unit->AutoRepair = 1;
			--j;
		} else if (!strcmp(value, "burning")) {
			unit->Burning = 1;
			--j;
		} else if (!strcmp(value, "destroyed")) {
			unit->Destroyed = 1;
			--j;
		} else if (!strcmp(value, "removed")) {
			unit->Removed = 1;
			--j;
		} else if (!strcmp(value, "selected")) {
			unit->Selected = 1;
			--j;
		} else if (!strcmp(value, "rescued-from")) {
			lua_rawgeti(l, 2, j + 1);
			unit->RescuedFrom = &Players[(int)LuaToNumber(l, -1)];
			lua_pop(l, 1);
		} else if (!strcmp(value, "seen-by-player")) {
			lua_rawgeti(l, 2, j + 1);
			const char *s = LuaToString(l, -1);
			lua_pop(l, 1);
			unit->Seen.ByPlayer = 0;
			for (int i = 0; i < PlayerMax && *s; ++i, ++s) {
				if (*s == '-' || *s == '_' || *s == ' ') {
					unit->Seen.ByPlayer &= ~(1 << i);
				} else {
					unit->Seen.ByPlayer |= (1 << i);
				}
			}
		} else if (!strcmp(value, "seen-destroyed")) {
			lua_rawgeti(l, 2, j + 1);
			const char *s = LuaToString(l, -1);
			lua_pop(l, 1);
			unit->Seen.Destroyed = 0;
			for (int i = 0; i < PlayerMax && *s; ++i, ++s) {
				if (*s == '-' || *s == '_' || *s == ' ') {
					unit->Seen.Destroyed &= ~(1 << i);
				} else {
					unit->Seen.Destroyed |= (1 << i);
				}
			}
		} else if (!strcmp(value, "constructed")) {
			unit->Constructed = 1;
			--j;
		} else if (!strcmp(value, "seen-constructed")) {
			unit->Seen.Constructed = 1;
			--j;
		} else if (!strcmp(value, "seen-state")) {
			lua_rawgeti(l, 2, j + 1);
			unit->Seen.State = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "active")) {
			unit->Active = 1;
			--j;
		} else if (!strcmp(value, "ttl")) {
			lua_rawgeti(l, 2, j + 1);
			// FIXME : unsigned long should be better handled
			unit->TTL = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "threshold")) {
			lua_rawgeti(l, 2, j + 1);
			// FIXME : unsigned long should be better handled
			unit->Threshold = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "group-id")) {
			lua_rawgeti(l, 2, j + 1);
			unit->GroupId = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "last-group")) {
			lua_rawgeti(l, 2, j + 1);
			unit->LastGroup = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "resources-held")) {
			lua_rawgeti(l, 2, j + 1);
			unit->ResourcesHeld = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "current-resource")) {
			lua_rawgeti(l, 2, j + 1);
			lua_pushvalue(l, -1);
			unit->CurrentResource = CclGetResourceByName(l);
			lua_pop(l, 1);
		} else if (!strcmp(value, "pathfinder-input")) {
			lua_rawgeti(l, 2, j + 1);
			lua_pushvalue(l, -1);
			unit->pathFinderData->input.Load(l);
			lua_pop(l, 1);
		} else if (!strcmp(value, "pathfinder-output")) {
			lua_rawgeti(l, 2, j + 1);
			lua_pushvalue(l, -1);
			unit->pathFinderData->output.Load(l);
			lua_pop(l, 1);
		} else if (!strcmp(value, "wait")) {
			lua_rawgeti(l, 2, j + 1);
			unit->Wait = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "anim-data")) {
			lua_rawgeti(l, 2, j + 1);
			CAnimations::LoadUnitAnim(l, *unit, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "blink")) {
			lua_rawgeti(l, 2, j + 1);
			unit->Blink = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "moving")) {
			unit->Moving = 1;
			--j;
		} else if (!strcmp(value, "re-cast")) {
			unit->ReCast = 1;
			--j;
		} else if (!strcmp(value, "boarded")) {
			unit->Boarded = 1;
			--j;
		} else if (!strcmp(value, "next-worker")) {
			lua_rawgeti(l, 2, j + 1);
			lua_pushvalue(l, -1);
			unit->NextWorker = CclGetUnitFromRef(l);
			lua_pop(l, 1);
		} else if (!strcmp(value, "resource-workers")) {
			lua_rawgeti(l, 2, j + 1);
			lua_pushvalue(l, -1);
			unit->Resource.Workers = CclGetUnitFromRef(l);
			lua_pop(l, 1);
		} else if (!strcmp(value, "resource-assigned")) {
			lua_rawgeti(l, 2, j + 1);
			lua_pushvalue(l, -1);
			unit->Resource.Assigned = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "resource-active")) {
			lua_rawgeti(l, 2, j + 1);
			lua_pushvalue(l, -1);
			unit->Resource.Active = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "rs")) {
			lua_rawgeti(l, 2, j + 1);
			unit->Rs = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "units-boarded-count")) {
			lua_rawgeti(l, 2, j + 1);
			unit->BoardCount = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "units-contained")) {
			int subargs;
			int k;
			lua_rawgeti(l, 2, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_rawlen(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				CUnit *u = CclGetUnitFromRef(l);
				lua_pop(l, 1);
				u->AddInContainer(*unit);
			}
			lua_pop(l, 1);
		} else if (!strcmp(value, "orders")) {
			lua_rawgeti(l, 2, j + 1);
			lua_pushvalue(l, -1);
			CclParseOrders(l, *unit);
			lua_pop(l, 1);
			// now we know unit's action so we can assign it to a player
			Assert(player != NULL);
			unit->AssignToPlayer(*player);
			if (unit->CurrentAction() == UnitActionBuilt) {
				DebugPrint("HACK: the building is not ready yet\n");
				// HACK: the building is not ready yet
				unit->Player->UnitTypesCount[type->Slot]--;
			}
		} else if (!strcmp(value, "critical-order")) {
			lua_rawgeti(l, 2, j + 1);
			lua_pushvalue(l, -1);
			CclParseOrder(l, *unit , &unit->CriticalOrder);
			lua_pop(l, 1);
		} else if (!strcmp(value, "saved-order")) {
			lua_rawgeti(l, 2, j + 1);
			lua_pushvalue(l, -1);
			CclParseOrder(l, *unit, &unit->SavedOrder);
			lua_pop(l, 1);
		} else if (!strcmp(value, "new-order")) {
			lua_rawgeti(l, 2, j + 1);
			lua_pushvalue(l, -1);
			CclParseOrder(l, *unit, &unit->NewOrder);
			lua_pop(l, 1);
		} else if (!strcmp(value, "goal")) {
			lua_rawgeti(l, 2, j + 1);
			unit->Goal = &UnitManager.GetSlotUnit(LuaToNumber(l, -1));
			lua_pop(l, 1);
		} else if (!strcmp(value, "auto-cast")) {
			lua_rawgeti(l, 2, j + 1);
			const char *s = LuaToString(l, -1);
			Assert(SpellTypeByIdent(s));
			if (!unit->AutoCastSpell) {
				unit->AutoCastSpell = new char[SpellTypeTable.size()];
				memset(unit->AutoCastSpell, 0, SpellTypeTable.size());
			}
			unit->AutoCastSpell[SpellTypeByIdent(s)->Slot] = 1;
			lua_pop(l, 1);
		} else {
			const int index = UnitTypeVar.VariableNameLookup[value];// User variables
			if (index != -1) { // Valid index
				lua_rawgeti(l, 2, j + 1);
				DefineVariableField(l, unit->Variable + index, -1);
				lua_pop(l, 1);
				continue;
			}
			LuaError(l, "Unit: Unsupported tag: %s" _C_ value);
		}
	}

	// Unit may not have been assigned to a player before now. If not,
	// do so now. It is only assigned earlier if we have orders.
	// for loading of units from a MAP, and not a savegame, we won't
	// have orders for those units.  They should appear here as if
	// they were just created.
	if (!unit->Player) {
		Assert(player);
		unit->AssignToPlayer(*player);
		UpdateForNewUnit(*unit, 0);
	}

	//  Revealers are units that can see while removed
	if (unit->Removed && unit->Type->Revealer) {
		MapMarkUnitSight(*unit);
	}

	// Fix Colors for rescued units.
	if (unit->RescuedFrom) {
		unit->Colors = &unit->RescuedFrom->UnitColors;
	}

	return 0;
}

/**
**  Move a unit on map.
**
**  @param l  Lua state.
**
**  @return   Returns the slot number of the made placed.
*/
static int CclMoveUnit(lua_State *l)
{
	LuaCheckArgs(l, 2);

	lua_pushvalue(l, 1);
	CUnit *unit = CclGetUnit(l);
	lua_pop(l, 1);

	Vec2i ipos;

	lua_rawgeti(l, 2, 1);
	ipos.x = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 2, 2);
	ipos.y = LuaToNumber(l, -1);
	lua_pop(l, 1);

	if (UnitCanBeAt(*unit, ipos)) {
		unit->Place(ipos);
	} else {
		const int heading = SyncRand() % 256;

		unit->tilePos = ipos;
		DropOutOnSide(*unit, heading, NULL);
	}
	lua_pushvalue(l, 1);
	return 1;
}

/**
**  Create a unit and place it on the map
**
**  @param l  Lua state.
**
**  @return   Returns the slot number of the made unit.
*/
static int CclCreateUnit(lua_State *l)
{
	LuaCheckArgs(l, 3);

	lua_pushvalue(l, 1);
	CUnitType *unittype = CclGetUnitType(l);
	if (unittype == NULL) {
		LuaError(l, "Bad unittype");
	}
	lua_pop(l, 1);
	if (!lua_istable(l, 3) || lua_rawlen(l, 3) != 2) {
		LuaError(l, "incorrect argument !!");
	}
	lua_rawgeti(l, 3, 1);
	Vec2i ipos;
	ipos.x = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 3, 2);
	ipos.y = LuaToNumber(l, -1);
	lua_pop(l, 1);

	lua_pushvalue(l, 2);
	const int playerno = TriggerGetPlayer(l);
	lua_pop(l, 1);
	if (playerno == -1) {
		printf("CreateUnit: You cannot use \"any\" in create-unit, specify a player\n");
		LuaError(l, "bad player");
		return 0;
	}
	if (Players[playerno].Type == PlayerNobody) {
		printf("CreateUnit: player %d does not exist\n", playerno);
		LuaError(l, "bad player");
		return 0;
	}
	CUnit *unit = MakeUnit(*unittype, &Players[playerno]);
	if (unit == NULL) {
		DebugPrint("Unable to allocate unit");
		return 0;
	} else {
		if (UnitCanBeAt(*unit, ipos)
			|| (unit->Type->Building && CanBuildUnitType(NULL, *unit->Type, ipos, 0))) {
			unit->Place(ipos);
		} else {
			const int heading = SyncRand() % 256;

			unit->tilePos = ipos;
			DropOutOnSide(*unit, heading, NULL);
		}
		UpdateForNewUnit(*unit, 0);

		lua_pushnumber(l, UnitNumber(*unit));
		return 1;
	}
}

/**
**  Set resources held by a unit
**
**  @param l  Lua state.
*/
static int CclSetResourcesHeld(lua_State *l)
{
	LuaCheckArgs(l, 2);

	if (lua_isnil(l, 1)) {
		return 0;
	}

	lua_pushvalue(l, 1);
	CUnit *unit = CclGetUnit(l);
	lua_pop(l, 1);
	const int value = LuaToNumber(l, 2);
	unit->ResourcesHeld = value;

	return 0;
}

/**
**  Order a unit
**
**  @param l  Lua state.
**
**  OrderUnit(player, unit-type, sloc, dloc, order)
*/
static int CclOrderUnit(lua_State *l)
{
	LuaCheckArgs(l, 5);

	lua_pushvalue(l, 1);
	const int plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	lua_pushvalue(l, 2);
	const CUnitType *unittype = TriggerGetUnitType(l);
	lua_pop(l, 1);
	if (!lua_istable(l, 3)) {
		LuaError(l, "incorrect argument");
	}
	lua_rawgeti(l, 3, 1);
	Vec2i pos1;
	pos1.x = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 3, 2);
	pos1.y = LuaToNumber(l, -1);
	lua_pop(l, 1);

	Vec2i pos2;
	if (lua_rawlen(l, 3) == 4) {
		lua_rawgeti(l, 3, 3);
		pos2.x = LuaToNumber(l, -1);
		lua_pop(l, 1);
		lua_rawgeti(l, 3, 4);
		pos2.y = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else {
		pos2 = pos1;
	}
	if (!lua_istable(l, 4)) {
		LuaError(l, "incorrect argument");
	}
	Vec2i dpos1;
	Vec2i dpos2;
	lua_rawgeti(l, 4, 1);
	dpos1.x = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 4, 2);
	dpos1.y = LuaToNumber(l, -1);
	lua_pop(l, 1);
	if (lua_rawlen(l, 4) == 4) {
		lua_rawgeti(l, 4, 3);
		dpos2.x = LuaToNumber(l, -1);
		lua_pop(l, 1);
		lua_rawgeti(l, 4, 4);
		dpos2.y = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else {
		dpos2 = dpos1;
	}
	const char *order = LuaToString(l, 5);
	std::vector<CUnit *> table;
	Select(pos1, pos2, table);
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];

		if (unittype == ANY_UNIT
			|| (unittype == ALL_FOODUNITS && !unit.Type->Building)
			|| (unittype == ALL_BUILDINGS && unit.Type->Building)
			|| unittype == unit.Type) {
			if (plynr == -1 || plynr == unit.Player->Index) {
				if (!strcmp(order, "move")) {
					CommandMove(unit, (dpos1 + dpos2) / 2, 1);
				} else if (!strcmp(order, "attack")) {
					CUnit *attack = TargetOnMap(unit, dpos1, dpos2);

					CommandAttack(unit, (dpos1 + dpos2) / 2, attack, 1);
				} else if (!strcmp(order, "patrol")) {
					CommandPatrolUnit(unit, (dpos1 + dpos2) / 2, 1);
				} else {
					LuaError(l, "Unsupported order: %s" _C_ order);
				}
			}
		}
	}
	return 0;
}

class HasSameUnitTypeAs
{
public:
	explicit HasSameUnitTypeAs(const CUnitType *_type) : type(_type) {}
	bool operator()(const CUnit *unit) const {
		return (type == ANY_UNIT || type == unit->Type
				|| (type == ALL_FOODUNITS && !unit->Type->Building)
				|| (type == ALL_BUILDINGS && unit->Type->Building));
	}
private:
	const CUnitType *type;
};


/**
**  Kill a unit
**
**  @param l  Lua state.
**
**  @return   Returns true if a unit was killed.
*/
static int CclKillUnit(lua_State *l)
{
	LuaCheckArgs(l, 2);

	lua_pushvalue(l, 1);
	const CUnitType *unittype = TriggerGetUnitType(l);
	lua_pop(l, 1);
	const int plynr = TriggerGetPlayer(l);
	if (plynr == -1) {
		CUnitManager::Iterator it = std::find_if(UnitManager.begin(), UnitManager.end(), HasSameUnitTypeAs(unittype));

		if (it != UnitManager.end()) {
			LetUnitDie(**it);
			lua_pushboolean(l, 1);
			return 1;
		}
	} else {
		CPlayer &player = Players[plynr];
		std::vector<CUnit *>::iterator it = std::find_if(player.UnitBegin(), player.UnitEnd(), HasSameUnitTypeAs(unittype));

		if (it != player.UnitEnd()) {
			LetUnitDie(**it);
			lua_pushboolean(l, 1);
			return 1;
		}
	}
	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Kill a unit at a location
**
**  @param l  Lua state.
**
**  @return   Returns the number of units killed.
*/
static int CclKillUnitAt(lua_State *l)
{
	LuaCheckArgs(l, 2);

	lua_pushvalue(l, 2);
	int plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	int q = LuaToNumber(l, 3);
	lua_pushvalue(l, 1);
	const CUnitType *unittype = TriggerGetUnitType(l);
	lua_pop(l, 1);
	if (!lua_istable(l, 4)) {
		LuaError(l, "incorrect argument");
	}
	Vec2i pos1;
	Vec2i pos2;
	lua_rawgeti(l, 4, 1);
	pos1.x = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 4, 2);
	pos1.y = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 4, 3);
	pos2.x = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 4, 4);
	pos2.y = LuaToNumber(l, -1);
	lua_pop(l, 1);

	std::vector<CUnit *> table;

	Select(pos1, pos2, table);

	int s = 0;
	for (size_t j = 0; j < table.size() && s < q; ++j) {
		CUnit *unit = table[j];

		if (unittype == ANY_UNIT
			|| (unittype == ALL_FOODUNITS && !unit->Type->Building)
			|| (unittype == ALL_BUILDINGS && unit->Type->Building)
			|| unittype == unit->Type) {
			if (plynr == -1 || plynr == unit->Player->Index) {
				LetUnitDie(*unit);
				++s;
			}
		}
	}
	lua_pushnumber(l, s);
	return 1;
}

/**
**  Get a player's units
**
**  @param l  Lua state.
**
**  @return   Array of units.
*/
static int CclGetUnits(lua_State *l)
{
	LuaCheckArgs(l, 1);

	const int plynr = TriggerGetPlayer(l);

	lua_newtable(l);
	if (plynr == -1) {
		int i = 0;
		for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it, ++i) {
			const CUnit &unit = **it;
			lua_pushnumber(l, UnitNumber(unit));
			lua_rawseti(l, -2, i + 1);
		}
	} else {
		for (int i = 0; i < Players[plynr].GetUnitCount(); ++i) {
			lua_pushnumber(l, UnitNumber(Players[plynr].GetUnit(i)));
			lua_rawseti(l, -2, i + 1);
		}
	}
	return 1;
}

/**
**
**  Get the value of the unit bool-flag.
**
**  @param l  Lua state.
**
**  @return   The value of the bool-flag of the unit.
*/
static int CclGetUnitBoolFlag(lua_State *l)
{
	LuaCheckArgs(l, 2);

	lua_pushvalue(l, 1);
	const CUnit *unit = CclGetUnit(l);
	lua_pop(l, 1);

	const char *const value = LuaToString(l, 2);
	int index = UnitTypeVar.BoolFlagNameLookup[value];// User bool flags
	if (index == -1) {
		LuaError(l, "Bad bool-flag name '%s'\n" _C_ value);
	}
	lua_pushboolean(l, unit->Type->BoolFlag[index].value);
	return 1;
}

/**
**  Get the value of the unit variable.
**
**  @param l  Lua state.
**
**  @return   The value of the variable of the unit.
*/
static int CclGetUnitVariable(lua_State *l)
{
	const int nargs = lua_gettop(l);
	Assert(nargs == 2 || nargs == 3);

	lua_pushvalue(l, 1);
	const CUnit *unit = CclGetUnit(l);
	lua_pop(l, 1);

	const char *const value = LuaToString(l, 2);
	if (!strcmp(value, "RegenerationRate")) {
		lua_pushnumber(l, unit->Variable[HP_INDEX].Increase);
	} else if (!strcmp(value, "Player")) {
		lua_pushnumber(l, unit->Player->Index);
	} else if (!strcmp(value, "Ident")) {
		lua_pushstring(l, unit->Type->Ident.c_str());
	} else if (!strcmp(value, "ResourcesHeld")) {
		lua_pushnumber(l, unit->ResourcesHeld);
	} else {
		int index = UnitTypeVar.VariableNameLookup[value];// User variables
		if (index == -1) {
			LuaError(l, "Bad variable name '%s'\n" _C_ value);
		}
		if (nargs == 2) {
			lua_pushnumber(l, unit->Variable[index].Value);
		} else {
			const char *const type = LuaToString(l, 3);
			if (!strcmp(type, "Value")) {
				lua_pushnumber(l, unit->Variable[index].Value);
			} else if (!strcmp(type, "Max")) {
				lua_pushnumber(l, unit->Variable[index].Max);
			} else if (!strcmp(type, "Increase")) {
				lua_pushnumber(l, unit->Variable[index].Increase);
			} else if (!strcmp(type, "Enable")) {
				lua_pushnumber(l, unit->Variable[index].Enable);
			} else {
				LuaError(l, "Bad variable type '%s'\n" _C_ type);
			}
		}
	}
	return 1;
}

/**
**  Set the value of the unit variable.
**
**  @param l  Lua state.
**
**  @return The new value of the unit.
*/
static int CclSetUnitVariable(lua_State *l)
{
	const int nargs = lua_gettop(l);
	Assert(nargs == 3 || nargs == 4);

	lua_pushvalue(l, 1);
	CUnit *unit = CclGetUnit(l);
	lua_pop(l, 1);
	const char *const name = LuaToString(l, 2);
	int value;
	if (!strcmp(name, "RegenerationRate")) {
		value = LuaToNumber(l, 3);
		if (value > unit->Variable[HP_INDEX].Max) {
			unit->Stats->Variables[HP_INDEX].Increase = unit->Variable[HP_INDEX].Max;
		} else {
			unit->Stats->Variables[HP_INDEX].Increase = value;
		}
	} else {
		const int index = UnitTypeVar.VariableNameLookup[name];// User variables
		if (index == -1) {
			LuaError(l, "Bad variable name '%s'\n" _C_ name);
		}
		value = LuaToNumber(l, 3);
		if (nargs == 3) {
			if (value > unit->Variable[index].Max) {
				unit->Variable[index].Value = unit->Variable[index].Max;
			} else {
				unit->Variable[index].Value = value;
			}
		} else {
			const char *const type = LuaToString(l, 4);
			if (!strcmp(type, "Value")) {
				if (value > unit->Variable[index].Max) {
					unit->Variable[index].Value = unit->Variable[index].Max;
				} else {
					unit->Variable[index].Value = value;
				}
			} else if (!strcmp(type, "Max")) {
				unit->Variable[index].Max = value;
			} else if (!strcmp(type, "Increase")) {
				unit->Variable[index].Increase = value;
			} else if (!strcmp(type, "Enable")) {
				unit->Variable[index].Enable = value;
			} else {
				LuaError(l, "Bad variable type '%s'\n" _C_ type);
			}
		}
	}
	lua_pushnumber(l, value);
	return 1;
}

/**
**  Get the usage of unit slots during load to allocate memory
**
**  @param l  Lua state.
*/
static int CclSlotUsage(lua_State *l)
{
	UnitManager.Load(l);
	return 0;
}

/**
**  Register CCL features for unit.
*/
void UnitCclRegister()
{
	lua_register(Lua, "SetTrainingQueue", CclSetTrainingQueue);
	lua_register(Lua, "SetBuildingCapture", CclSetBuildingCapture);
	lua_register(Lua, "SetRevealAttacker", CclSetRevealAttacker);
	lua_register(Lua, "ResourcesMultiBuildersMultiplier", CclResourcesMultiBuildersMultiplier);

	lua_register(Lua, "Unit", CclUnit);

	lua_register(Lua, "MoveUnit", CclMoveUnit);
	lua_register(Lua, "CreateUnit", CclCreateUnit);
	lua_register(Lua, "SetResourcesHeld", CclSetResourcesHeld);
	lua_register(Lua, "OrderUnit", CclOrderUnit);
	lua_register(Lua, "KillUnit", CclKillUnit);
	lua_register(Lua, "KillUnitAt", CclKillUnitAt);

	lua_register(Lua, "GetUnits", CclGetUnits);

	// unit member access functions
	lua_register(Lua, "GetUnitBoolFlag", CclGetUnitBoolFlag);
	lua_register(Lua, "GetUnitVariable", CclGetUnitVariable);
	lua_register(Lua, "SetUnitVariable", CclSetUnitVariable);

	lua_register(Lua, "SlotUsage", CclSlotUsage);
}

//@}
