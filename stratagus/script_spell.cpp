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
/**@name script_spell.c - The spell script functions.. */
//
//      (c) Copyright 1998-2005 by Joris Dauphin and Crestez Leonard
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
//      $Id$
//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "spells.h"
#include "script_sound.h"
#include "script.h"
#include "missile.h"
#include "unittype.h"
#include "upgrade.h"


// **************************************************************************
// Action parsers for spellAction
// **************************************************************************

/**
**  Parse the missile location description for a spell action.
**
**  @param l         Lua state.
**  @param location  Pointer to missile location description.
**
**  @note This is only here to avoid code duplication. You don't have
**        any reason to USE this:)
*/
static void CclSpellMissileLocation(lua_State* l, SpellActionMissileLocation* location)
{
	const char* value;
	int args;
	int j;

	Assert(location != NULL);
	memset(location, 0, sizeof(*location));

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	j = 0;

	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;
		if (!strcmp(value, "base")) {
			lua_rawgeti(l, -1, j + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			if (!strcmp(value, "caster")) {
				location->Base = LocBaseCaster;
			} else if (!strcmp(value, "target")) {
				location->Base = LocBaseTarget;
			} else {
				LuaError(l, "Unsupported missile location base flag: %s" _C_ value);
			}
		} else if (!strcmp(value, "add-x")) {
			lua_rawgeti(l, -1, j + 1);
			location->AddX = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "add-y")) {
			lua_rawgeti(l, -1, j + 1);
			location->AddY = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "add-rand-x")) {
			lua_rawgeti(l, -1, j + 1);
			location->AddRandX = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "add-rand-y")) {
			lua_rawgeti(l, -1, j + 1);
			location->AddRandY = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported missile location description flag: %s" _C_ value);
		}
	}
}

/**
**  Parse the action for spell.
**
**  @param l            Lua state.
**  @param spellaction  Pointer to spellaction.
*/
static void CclSpellAction(lua_State* l, SpellActionType* spellaction)
{
	const char* value;
	int args;
	int j;

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	j = 0;

	lua_rawgeti(l, -1, j + 1);
	value = LuaToString(l, -1);
	lua_pop(l, 1);
	++j;

	if (!strcmp(value, "spawn-missile")) {
		spellaction->CastFunction = CastSpawnMissile;
		spellaction->Data.SpawnMissile.StartPoint.Base = LocBaseCaster;
		spellaction->Data.SpawnMissile.EndPoint.Base = LocBaseTarget;
		spellaction->Data.SpawnMissile.TTL = -1;
		for (; j < args; ++j) {
			lua_rawgeti(l, -1, j + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++j;
			if (!strcmp(value, "damage")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.SpawnMissile.Damage = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "delay")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.SpawnMissile.Delay = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "ttl")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.SpawnMissile.TTL = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "start-point")) {
				lua_rawgeti(l, -1, j + 1);
				CclSpellMissileLocation(l, &spellaction->Data.SpawnMissile.StartPoint);
				lua_pop(l, 1);
			} else if (!strcmp(value, "end-point")) {
				lua_rawgeti(l, -1, j + 1);
				CclSpellMissileLocation(l, &spellaction->Data.SpawnMissile.EndPoint);
				lua_pop(l, 1);
			} else if (!strcmp(value, "missile")) {
				lua_rawgeti(l, -1, j + 1);
				value = LuaToString(l, -1);
				spellaction->Data.SpawnMissile.Missile = MissileTypeByIdent(value);
				if (spellaction->Data.SpawnMissile.Missile == NULL) {
					DebugPrint("in spawn-missile : missile %s does not exist\n" _C_ value);
				}
				lua_pop(l, 1);
			} else {
				LuaError(l, "Unsupported spawn-missile tag: %s" _C_ value);
			}
		}
		// Now, checking value.
		if (spellaction->Data.SpawnMissile.Missile == NULL) {
			LuaError(l, "Use a missile for spawn-missile (with missile)");
		}
	} else if (!strcmp(value, "area-adjust-vitals")) {
		spellaction->CastFunction = CastAreaAdjustVitals;
		for (; j < args; ++j) {
			lua_rawgeti(l, -1, j + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++j;
			if (!strcmp(value, "hit-points")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.AreaAdjustVitals.HP = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "mana-points")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.AreaAdjustVitals.Mana = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else {
				LuaError(l, "Unsupported area-adjust-vitals tag: %s" _C_ value);
			}
		}
	} else if (!strcmp(value, "area-bombardment")) {
		spellaction->CastFunction = CastAreaBombardment;
		for (; j < args; ++j) {
			lua_rawgeti(l, -1, j + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++j;
			if (!strcmp(value, "fields")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.AreaBombardment.Fields = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "shards")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.AreaBombardment.Shards = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "damage")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.AreaBombardment.Damage = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "start-offset-x")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.AreaBombardment.StartOffsetX = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "start-offset-y")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.AreaBombardment.StartOffsetY = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "missile")) {
				lua_rawgeti(l, -1, j + 1);
				value = LuaToString(l, -1);
				spellaction->Data.AreaBombardment.Missile = MissileTypeByIdent(value);
				if (spellaction->Data.AreaBombardment.Missile == NULL) {
					DebugPrint("in area-bombardement : missile %s does not exist\n" _C_ value);
				}
				lua_pop(l, 1);
			} else {
				LuaError(l, "Unsupported area-bombardment tag: %s" _C_ value);
			}
		}
		// Now, checking value.
		if (spellaction->Data.AreaBombardment.Missile == NULL) {
			LuaError(l, "Use a missile for area-bombardment (with missile)");
		}
	} else if (!strcmp(value, "demolish")) {
		spellaction->CastFunction = CastDemolish;
		for (; j < args; ++j) {
			lua_rawgeti(l, -1, j + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++j;
			if (!strcmp(value, "range")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.Demolish.Range = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "damage")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.Demolish.Damage = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else {
				LuaError(l, "Unsupported demolish tag: %s" _C_ value);
			}
		}
	} else if (!strcmp(value, "adjust-variable")) {
		spellaction->CastFunction = CastAdjustVariable;
		lua_rawgeti(l, -1, j + 1);
		if (!lua_istable(l, -1)) {
			LuaError(l, "Table expected for adjust-variable.");
		}
		spellaction->Data.AdjustVariable = calloc(UnitTypeVar.NumberVariable, sizeof (*spellaction->Data.AdjustVariable));
		for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
			int i;

			i = GetVariableIndex(LuaToString(l, -2));
			if (i == -1) {
				LuaError(l, "in adjust-variable : Bad variable index : '%s'" _C_ LuaToString(l, -2));
			}
			if (lua_isnumber(l, -1)) {
				spellaction->Data.AdjustVariable[i].Enable = (LuaToNumber(l, -1) != 0);
				spellaction->Data.AdjustVariable[i].ModifEnable = 1;
				spellaction->Data.AdjustVariable[i].Value = LuaToNumber(l, -1);
				spellaction->Data.AdjustVariable[i].ModifValue = 1;
				spellaction->Data.AdjustVariable[i].Max = LuaToNumber(l, -1);
				spellaction->Data.AdjustVariable[i].ModifMax = 1;
			} else if (lua_istable(l, -1)) {
				for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
					const char *key;

					key = LuaToString(l, -2);
					if (!strcmp(key, "Enable")) {
						spellaction->Data.AdjustVariable[i].Enable = LuaToBoolean(l, -1);
						spellaction->Data.AdjustVariable[i].ModifEnable = 1;
					} else if (!strcmp(key, "Value")) {
						spellaction->Data.AdjustVariable[i].Value = LuaToNumber(l, -1);
						spellaction->Data.AdjustVariable[i].ModifValue = 1;
					} else if (!strcmp(key, "Max")) {
						spellaction->Data.AdjustVariable[i].Max = LuaToNumber(l, -1);
						spellaction->Data.AdjustVariable[i].ModifMax = 1;
					} else if (!strcmp(key, "Increase")) {
						spellaction->Data.AdjustVariable[i].Increase = LuaToNumber(l, -1);
						spellaction->Data.AdjustVariable[i].ModifIncrease = 1;
					} else if (!strcmp(key, "InvertEnable")) {
						spellaction->Data.AdjustVariable[i].InvertEnable = LuaToBoolean(l, -1);
					} else if (!strcmp(key, "AddValue")) {
						spellaction->Data.AdjustVariable[i].AddValue = LuaToNumber(l, -1);
					} else if (!strcmp(key, "AddMax")) {
						spellaction->Data.AdjustVariable[i].AddMax = LuaToNumber(l, -1);
					} else if (!strcmp(key, "AddIncrease")) {
						spellaction->Data.AdjustVariable[i].AddIncrease = LuaToNumber(l, -1);
					} else if (!strcmp(key, "IncreaseTime")) {
						spellaction->Data.AdjustVariable[i].IncreaseTime = LuaToNumber(l, -1);
					} else if (!strcmp(key, "TargetIsCaster")) {
						value = LuaToString(l, -1);
						if (!strcmp(value, "caster")) {
							spellaction->Data.AdjustVariable[i].TargetIsCaster = 1;
						} else if (!strcmp(value, "target")) {
							spellaction->Data.AdjustVariable[i].TargetIsCaster = 0;
						} else { // Error
							LuaError(l, "key '%s' not valid for TargetIsCaster in adjustvariable" _C_ value);
						}
					} else { // Error
						LuaError(l, "key '%s' not valid for adjustvariable" _C_ key);
					}
				}
			} else {
				LuaError(l, "in adjust-variable : Bad variable value");
			}
		}
		lua_pop(l, 1); // pop table
	} else if (!strcmp(value, "summon")) {
		spellaction->CastFunction = CastSummon;
		for (; j < args; ++j) {
			lua_rawgeti(l, -1, j + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++j;
			if (!strcmp(value, "unit-type")) {
				lua_rawgeti(l, -1, j + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				spellaction->Data.Summon.UnitType = UnitTypeByIdent(value);
				if (!spellaction->Data.Summon.UnitType) {
					spellaction->Data.Summon.UnitType = 0;
					DebugPrint("unit type \"%s\" not found for summon spell.\n" _C_ value);
				}
			} else if (!strcmp(value, "time-to-live")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.Summon.TTL = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "require-corpse")) {
				spellaction->Data.Summon.RequireCorpse = 1;
				--j;
			} else {
				LuaError(l, "Unsupported summon tag: %s" _C_ value);
			}
		}
		// Now, checking value.
		if (spellaction->Data.Summon.UnitType == NULL) {
			LuaError(l, "Use a unittype for summon (with unit-type)");
		}
	} else if (!strcmp(value, "spawn-portal")) {
		spellaction->CastFunction = CastSpawnPortal;
		for (; j < args; ++j) {
			lua_rawgeti(l, -1, j + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++j;
			if (!strcmp(value, "portal-type")) {
				lua_rawgeti(l, -1, j + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				spellaction->Data.SpawnPortal.PortalType = UnitTypeByIdent(value);
				if (!spellaction->Data.SpawnPortal.PortalType) {
					spellaction->Data.SpawnPortal.PortalType = 0;
					DebugPrint("unit type \"%s\" not found for spawn-portal.\n" _C_ value);
				}
			} else {
				LuaError(l, "Unsupported spawn-portal tag: %s" _C_ value);
			}
		}
		// Now, checking value.
		if (spellaction->Data.SpawnPortal.PortalType == NULL) {
			LuaError(l, "Use a unittype for spawn-portal (with portal-type)");
		}
	} else if (!strcmp(value, "capture")) {
		spellaction->CastFunction = CastCapture;
		for (; j < args; ++j) {
			lua_rawgeti(l, -1, j + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++j;
			if (!strcmp(value, "sacrifice")) {
				spellaction->Data.Capture.SacrificeEnable = 1;
			} else if (!strcmp(value, "damage")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.Capture.Damage = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "percent")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.Capture.DamagePercent = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else {
				LuaError(l, "Unsupported Capture tag: %s" _C_ value);
			}
		}
	} else if (!strcmp(value, "polymorph")) {
		spellaction->CastFunction = CastPolymorph;
		for (; j < args; ++j) {
			lua_rawgeti(l, -1, j + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++j;
			if (!strcmp(value, "new-form")) {
				lua_rawgeti(l, -1, j + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				spellaction->Data.Polymorph.NewForm = UnitTypeByIdent(value);
				if (!spellaction->Data.Polymorph.NewForm) {
					spellaction->Data.Polymorph.NewForm= 0;
					DebugPrint("unit type \"%s\" not found for polymorph spell.\n" _C_ value);
				}
				// FIXME: temp polymorphs? hard to do.
			} else if (!strcmp(value, "player-neutral")) {
				spellaction->Data.Polymorph.PlayerNeutral = 1;
				--j;
			} else {
				LuaError(l, "Unsupported polymorph tag: %s" _C_ value);
			}
		}
		// Now, checking value.
		if (spellaction->Data.Polymorph.NewForm == NULL) {
			LuaError(l, "Use a unittype for polymorph (with new-form)");
		}
	} else if (!strcmp(value, "adjust-vitals")) {
		spellaction->CastFunction = CastAdjustVitals;
		for (; j < args; ++j) {
			lua_rawgeti(l, -1, j + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++j;
			if (!strcmp(value, "hit-points")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.AdjustVitals.HP = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "mana-points")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.AdjustVitals.Mana = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "max-multi-cast")) {
				lua_rawgeti(l, -1, j + 1);
				spellaction->Data.AdjustVitals.MaxMultiCast = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else {
				LuaError(l, "Unsupported adjust-vitals tag: %s" _C_ value);
			}
		}
	} else {
		LuaError(l, "Unsupported action type: %s" _C_ value);
	}
}

/**
**  Get a condition value from a scm object.
**
**  @param l      Lua state.
**  @param value  scm value to convert.
**
**  @return CONDITION_TRUE, CONDITION_FALSE, CONDITION_ONLY or -1 on error.
**  @note This is a helper function to make CclSpellCondition shorter
**        and easier to understand.
*/
char Ccl2Condition(lua_State* l, const char* value)
{
	if (!strcmp(value, "true")) {
		return CONDITION_TRUE;
	} else if (!strcmp(value, "false")) {
		return CONDITION_FALSE;
	} else if (!strcmp(value, "only")) {
		return CONDITION_ONLY;
	} else {
		LuaError(l, "Bad condition result: %s" _C_ value);
		return -1;
	}
}

/**
**  Parse the Condition for spell.
**
**  @param l          Lua state.
**  @param condition  pointer to condition to fill with data.
**
**  @note Conditions must be allocated. All data already in is LOST.
*/
static void CclSpellCondition(lua_State* l, ConditionInfo* condition)
{
	const char* value;
	int i;
	int args;
	int j;

	//
	// Initializations:
	//

	// Set everything to 0:
	memset(condition, 0, sizeof(ConditionInfo));
	// Flags are defaulted to 0(CONDITION_TRUE)
	condition->BoolFlag = (char*)calloc(UnitTypeVar.NumberBoolFlag, sizeof (*condition->BoolFlag));
	condition->Variable = calloc(UnitTypeVar.NumberVariable, sizeof (*condition->Variable));
	// Initialize min/max stuff to values with no effect.
	for (i = 0; i < UnitTypeVar.NumberVariable; i++) {
		condition->Variable[i].MinValue = -1;
		condition->Variable[i].MaxValue = -1;
		condition->Variable[i].MinMax = -1;
		condition->Variable[i].MinValuePercent = -8;
		condition->Variable[i].MaxValuePercent = 1024;
	}
	//  Now parse the list and set values.
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;
		if (!strcmp(value, "alliance")) {
			lua_rawgeti(l, -1, j + 1);
			condition->Alliance = Ccl2Condition(l, LuaToString(l, -1));
			lua_pop(l, 1);
		} else if (!strcmp(value, "opponent")) {
			lua_rawgeti(l, -1, j + 1);
			condition->Opponent = Ccl2Condition(l, LuaToString(l, -1));
			lua_pop(l, 1);
		} else if (!strcmp(value, "self")) {
			lua_rawgeti(l, -1, j + 1);
			condition->TargetSelf = Ccl2Condition(l, LuaToString(l, -1));
			lua_pop(l, 1);
		} else {
			for (i = 0; i < UnitTypeVar.NumberBoolFlag; i++) { // User defined flags
				if (!strcmp(value, UnitTypeVar.BoolFlagName[i])) {
					lua_rawgeti(l, -1, j + 1);
					condition->BoolFlag[i] = Ccl2Condition(l, LuaToString(l, -1));
					lua_pop(l, 1);
					break;
				}
			}
			if (i != UnitTypeVar.NumberBoolFlag) {
				continue;
			}
			i = GetVariableIndex(value);
			if (i != -1) { // Valid index.
				lua_rawgeti(l, -1, j + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "Table expected in variable in condition");
				}
				for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
					const char *key;

					key = LuaToString(l, -2);
					if (!strcmp(key, "Enable")) {
						condition->Variable[i].Enable = Ccl2Condition(l, LuaToString(l, -1));
					} else if (!strcmp(key, "MinValue")) {
						condition->Variable[i].MinValue = LuaToNumber(l, -1);
					} else if (!strcmp(key, "MaxValue")) {
						condition->Variable[i].MaxValue = LuaToNumber(l, -1);
					} else if (!strcmp(key, "MinMax")) {
						condition->Variable[i].MinMax = LuaToNumber(l, -1);
					} else if (!strcmp(key, "MinValuePercent")) {
						condition->Variable[i].MinValuePercent = LuaToNumber(l, -1);
					} else if (!strcmp(key, "MaxValuePercent")) {
						condition->Variable[i].MaxValuePercent = LuaToNumber(l, -1);
					} else if (!strcmp(key, "ConditionApplyOnCaster")) {
						condition->Variable[i].ConditionApplyOnCaster = LuaToBoolean(l, -1);
					} else { // Error
						LuaError(l, "%s invalid for Variable in condition" _C_ key);
					}
				}
				lua_pop(l, 1); // lua_rawgeti()
				continue;
			}
			LuaError(l, "Unsuported condition tag: %s" _C_ value);
		}
	}
}

/**
**  Parse the Condition for spell.
**
**  @param l         Lua state.
**  @param autocast  pointer to autocast to fill with data.
**
**  @notes: autocast must be allocated. All data already in is LOST.
*/
static void CclSpellAutocast(lua_State* l, AutoCastInfo* autocast)
{
	const char* value;
	int args;
	int j;

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;
		if (!strcmp(value, "range")) {
			lua_rawgeti(l, -1, j + 1);
			autocast->Range = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "combat")) {
			lua_rawgeti(l, -1, j + 1);
			autocast->Combat = Ccl2Condition(l, LuaToString(l, -1));
			lua_pop(l, 1);
		} else if (!strcmp(value, "condition")) {
			if (!autocast->Condition) {
				autocast->Condition = (ConditionInfo*)malloc(sizeof(ConditionInfo));
			}
			lua_rawgeti(l, -1, j + 1);
			CclSpellCondition(l, autocast->Condition);
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported autocast tag: %s" _C_ value);
		}
	}
}

/**
**  Parse Spell.
**
**  @param l  Lua state.
*/
static int CclDefineSpell(lua_State* l)
{
	const char* identname;
	SpellType* spell;
	const char* value;
	SpellActionType* act;
	int args;
	int i;

	args = lua_gettop(l);
	identname = LuaToString(l, 1);
	spell = SpellTypeByIdent(identname);
	if (spell != NULL) {
		DebugPrint("Redefining spell-type `%s'\n" _C_ identname);
	} else {
		SpellTypeTable = (SpellType**)realloc(SpellTypeTable, (1 + SpellTypeCount) * sizeof(SpellType*));
		spell = SpellTypeTable[SpellTypeCount] = (SpellType*)malloc(sizeof(SpellType));
		memset(spell, 0, sizeof(SpellType));
		spell->Slot = SpellTypeCount;
		spell->Ident = strdup(identname);
		spell->DependencyId = -1;
		for (i = 0; i < NumUnitTypes; ++i) { // adjust array for caster already defined
			if (UnitTypes[i]->CanCastSpell) {
				UnitTypes[i]->CanCastSpell = (char*)realloc(UnitTypes[i]->CanCastSpell,
					SpellTypeCount * sizeof((*UnitTypes)->CanCastSpell));
				UnitTypes[i]->CanCastSpell[SpellTypeCount] = 0;
			}
			if (UnitTypes[i]->AutoCastActive) {
				UnitTypes[i]->AutoCastActive = (char*)realloc(UnitTypes[i]->AutoCastActive,
					SpellTypeCount * sizeof((*UnitTypes)->AutoCastActive));
				UnitTypes[i]->AutoCastActive[SpellTypeCount] = 0;
			}
		}
		SpellTypeCount++;
	}
	for (i = 1; i < args; ++i) {
		value = LuaToString(l, i + 1);
		++i;
		if (!strcmp(value, "showname")) {
			if (spell->Name) {
					free(spell->Name);
			}
			spell->Name = strdup(LuaToString(l, i + 1));
		} else if (!strcmp(value, "manacost")) {
			spell->ManaCost = LuaToNumber(l, i + 1);
		} else if (!strcmp(value, "range")) {
			if (!lua_isstring(l, i + 1) && !lua_isnumber(l, i + 1)) {
				LuaError(l, "incorrect argument");
			}
			if (lua_isstring(l, i + 1) && !strcmp(lua_tostring(l, i + 1), "infinite")) {
				spell->Range = INFINITE_RANGE;
			} else if (lua_isnumber(l, i + 1)) {
				spell->Range = lua_tonumber(l, i + 1);
			} else {
				LuaError(l, "Invalid range");
			}
		} else if (!strcmp(value, "repeat-cast")) {
			spell->RepeatCast = 1;
			--i;
		} else if (!strcmp(value, "target")) {
			value = LuaToString(l, i + 1);
			if (!strcmp(value, "self")) {
				spell->Target = TargetSelf;
			} else if (!strcmp(value, "unit")) {
				spell->Target = TargetUnit;
			} else if (!strcmp(value, "position")) {
				spell->Target = TargetPosition;
			} else {
				LuaError(l, "Unsupported spell target type tag: %s" _C_ value);
			}
		} else if (!strcmp(value, "action")) {
			int subargs;
			int k;

			spell->Action = (SpellActionType*)malloc(sizeof(SpellActionType));
			act = spell->Action;
			memset(act, 0, sizeof(SpellActionType));
			if (!lua_istable(l, i + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, i + 1);
			k = 0;
			lua_rawgeti(l, i + 1, k + 1);
			CclSpellAction(l, act);
			lua_pop(l, 1);
			++k;
			for (; k < subargs; ++k) {
				act->Next = (SpellActionType*)malloc(sizeof(SpellActionType));
				act = act->Next;
				memset(act, 0, sizeof(SpellActionType));
				lua_rawgeti(l, i + 1, k + 1);
				CclSpellAction(l, act);
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "condition")) {
			if (!spell->Condition) {
				spell->Condition = (ConditionInfo*)malloc(sizeof(ConditionInfo));
			}
			lua_pushvalue(l, i + 1);
			CclSpellCondition(l, spell->Condition);
			lua_pop(l, 1);
		} else if (!strcmp(value, "autocast")) {
			if (!spell->AutoCast) {
				spell->AutoCast = (AutoCastInfo*)malloc(sizeof(AutoCastInfo));
				memset(spell->AutoCast, 0, sizeof(AutoCastInfo));
			}
			lua_pushvalue(l, i + 1);
			CclSpellAutocast(l, spell->AutoCast);
			lua_pop(l, 1);
		} else if (!strcmp(value, "ai-cast")) {
			if (!spell->AICast) {
				spell->AICast = (AutoCastInfo*)malloc(sizeof(AutoCastInfo));
				memset(spell->AICast, 0, sizeof(AutoCastInfo));
			}
			lua_pushvalue(l, i + 1);
			CclSpellAutocast(l, spell->AICast);
			lua_pop(l, 1);
		} else if (!strcmp(value, "sound-when-cast")) {
			//  Free the old name, get the new one
			if (spell->SoundWhenCast.Name) {
				free(spell->SoundWhenCast.Name);
			}
			spell->SoundWhenCast.Name = strdup(LuaToString(l, i + 1));
			spell->SoundWhenCast.Sound = SoundIdForName(spell->SoundWhenCast.Name);
			//  Check for sound.
			if (!spell->SoundWhenCast.Sound) {
				free(spell->SoundWhenCast.Name);
				spell->SoundWhenCast.Name = 0;
			}
		} else if (!strcmp(value, "depend-upgrade")) {
			value = LuaToString(l, i + 1);
			spell->DependencyId = UpgradeIdByIdent(value);
			if (spell->DependencyId == -1) {
				lua_pushfstring(l, "Bad upgrade name: %s", value);
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	return 0;
}

/**
** Register CCL features for Spell.
*/
void SpellCclRegister(void)
{
	lua_register(Lua, "DefineSpell", CclDefineSpell);
}

#if 0 // Use old ccl config.

/**
** Save a spell action to a file.
**
**  @param file File pointer to save to
** @param action Pointer to action to save.
*/
static void SaveSpellAction(CLFile* file, SpellActionType* action)
{
	SpellActionMissileLocation* loc;

	if (action->CastFunction == CastAreaBombardment) {
		CLprintf(file, "(area-bombardment fields %d shards %d damage %d start-offset-x %d start-offset-y %d)",
				action->Data.AreaBombardment.Fields,
				action->Data.AreaBombardment.Shards,
				action->Data.AreaBombardment.Damage,
				action->Data.AreaBombardment.StartOffsetX,
				action->Data.AreaBombardment.StartOffsetY);
	} else if (action->CastFunction == CastAreaAdjustVitals) {
		CLprintf(file, "(area-adjust-vitals");
		if (action->Data.AreaAdjustVitals.HP) {
			CLprintf(file, " hit-points %d", action->Data.AdjustVitals.HP);
		}
		if (action->Data.AreaAdjustVitals.Mana) {
			CLprintf(file, " mana-points %d", action->Data.AdjustVitals.Mana);
		}
		CLprintf(file, ")\n");
	} else if (action->CastFunction == CastSpawnMissile) {
		CLprintf(file, "(spawn-missile delay %d ttl %d damage %d ",
				action->Data.SpawnMissile.Delay,
				action->Data.SpawnMissile.TTL,
				action->Data.SpawnMissile.Damage);
		//
		// Save start-point
		//
		loc=&action->Data.SpawnMissile.StartPoint;
		CLprintf(file, "start-point (base ");
		if (loc->Base==LocBaseCaster) {
			CLprintf(file, "caster");
		} else {
			CLprintf(file, "target");
		}
		CLprintf(file, " add-x %d add-y %d add-rand-x %d add-rand-y %d) ",
				loc->AddX,loc->AddY,loc->AddRandX,loc->AddRandY);
		//
		// Save end-point
		//
		loc=&action->Data.SpawnMissile.EndPoint;
		CLprintf(file, "end-point (base ");
		if (loc->Base==LocBaseCaster) {
			CLprintf(file, "caster");
		} else {
			CLprintf(file, "target");
		}
		CLprintf(file, " add-x %d add-y %d add-rand-x %d add-rand-y %d)",
				loc->AddX,loc->AddY,loc->AddRandX,loc->AddRandY);
		CLprintf(file, ")");
	} else if (action->CastFunction == CastAdjustVitals) {
		CLprintf(file, "(adjust-vitals");
		if (action->Data.AdjustVitals.HP) {
			CLprintf(file, " hit-points %d", action->Data.AdjustVitals.HP);
		}
		if (action->Data.AdjustVitals.Mana) {
			CLprintf(file, " mana-points %d", action->Data.AdjustVitals.Mana);
		}
		if (action->Data.AdjustVitals.MaxMultiCast) {
			CLprintf(file, " max-multi-cast %d", action->Data.AdjustVitals.MaxMultiCast);
		}
		CLprintf(file, ")\n");
	} else if (action->CastFunction == CastSummon) {
		CLprintf(file, "(summon unit-type %s time-to-live %d",
				action->Data.Summon.UnitType->Ident,
				action->Data.Summon.TTL);
		if (action->Data.Summon.RequireCorpse) {
			CLprintf(file, " require-corpse ");
		}
		CLprintf(file, ")\n");
	} else if (action->CastFunction == CastDemolish) {
		CLprintf(file, "(demolish range %d damage %d)\n",
				action->Data.Demolish.Range,
				action->Data.Demolish.Damage);
	} else if (action->CastFunction == CastPolymorph) {
		CLprintf(file, "(polymorph new-form %s)",
				action->Data.Polymorph.NewForm->Ident);
	} else if (action->CastFunction == CastCapture) {
		CLprintf(file, "(capture damage %d percent %d",
				action->Data.Capture.Damage,
				action->Data.Capture.DamagePercent);
		if (action->Data.Capture.SacrificeEnable) {
			CLprintf(file, " sacrifice");
		}
		CLprintf(file, ")\n");
	} else if (action->CastFunction == CastSpawnPortal) {
		CLprintf(file, "(spawn-portal portal-type %s)",
				action->Data.SpawnPortal.PortalType->Ident);
	}
}

/**
**  Save a spell condition to a file.
**
**  @param file       File pointer to save to
**  @param condition  Pointer to condition to save.
*/
static void SaveSpellCondition(CLFile* file, ConditionInfo* condition)
{
	char condstrings[3][10] = {
		"true", /// CONDITION_TRUE
		"false", /// CONDITION_FALSE
		"only" /// CONDITION_ONLY
	};
	int i;
	Assert(file);
	Assert(condition);

	CLprintf(file, "( ");
	//
	// First save data related to flags.
	// NOTE: (int) is there to keep compilers happy.
	//
	if (condition->Alliance != CONDITION_TRUE) {
		CLprintf(file, "alliance %s ", condstrings[(int)condition->Alliance]);
	}
	if (condition->TargetSelf != CONDITION_TRUE) {
		CLprintf(file, "self %s ", condstrings[(int)condition->TargetSelf]);
	}
	for (i = 0; i < UnitTypeVar.NumberBoolFlag; i++) { // User defined flags
		if (condition->BoolFlag[i] != CONDITION_TRUE) {
			CLprintf(file, "%s %s ",
				UnitTypeVar.BoolFlagName[i], condstrings[(int)condition->BoolFlag[i]]);
		}
	}
	//
	// The end.
	//
	CLprintf(file, ")\n");
}

/**
** Save autocast info to a CCL file
**
** @param file The file to save to.
** @param autocast Auocastinfo to save.
*/
void SaveSpellAutoCast(CLFile* file, AutoCastInfo* autocast)
{
	char condstrings[3][10] = {
		"true", /// CONDITION_TRUE
		"false", /// CONDITION_FALSE
		"only" /// CONDITION_ONLY
	};

	CLprintf(file, "( range %d ", autocast->Range);
	if (autocast->Combat != CONDITION_TRUE) {
		CLprintf(file, "combat %s ", condstrings[(int)autocast->Combat]);
	}
	if (autocast->Condition) {
		CLprintf(file, " condition ");
		SaveSpellCondition(file, autocast->Condition);
	}
	CLprintf(file, " )\n");
}

#endif

//@}
