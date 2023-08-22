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
/**@name script_spell.cpp - The spell script functions.. */
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
//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "spells.h"

#include "spell/spell_adjustvariable.h"
#include "spell/spell_adjustvital.h"
#include "spell/spell_areaadjustvital.h"
#include "spell/spell_areabombardment.h"
#include "spell/spell_capture.h"
#include "spell/spell_demolish.h"
#include "spell/spell_luacallback.h"
#include "spell/spell_polymorph.h"
#include "spell/spell_spawnmissile.h"
#include "spell/spell_spawnportal.h"
#include "spell/spell_summon.h"
#include "spell/spell_teleport.h"
#include "luacallback.h"
#include "script_sound.h"
#include "script.h"
#include "unittype.h"
#include "upgrade.h"


// **************************************************************************
// Action parsers for spellAction
// **************************************************************************

/**
**  Parse the action for spell.
**
**  @param l  Lua state.
*/
static SpellActionType *CclSpellAction(lua_State *l)
{
	if (!lua_istable(l, -1)) {
		LuaError(l, "expected a table of tables as spell action");
	}
	const int args = lua_rawlen(l, -1);

	const std::string_view value = LuaToString(l, -1, 1);

	SpellActionType *spellaction = nullptr;
	if (value == "adjust-variable") {
		spellaction = new Spell_AdjustVariable;
	} else if (value == "adjust-vitals") {
		spellaction = new Spell_AdjustVital;
	} else if (value == "area-adjust-vitals") {
		spellaction = new Spell_AreaAdjustVital;
	} else if (value == "area-bombardment") {
		spellaction = new Spell_AreaBombardment;
	} else if (value == "capture") {
		spellaction = new Spell_Capture;
	} else if (value == "demolish") {
		spellaction = new Spell_Demolish;
	} else if (value == "lua-callback") {
		spellaction = new Spell_LuaCallback;
	} else if (value == "polymorph") {
		spellaction = new Spell_Polymorph;
	} else if (value == "spawn-missile") {
		spellaction = new Spell_SpawnMissile;
	} else if (value == "spawn-portal") {
		spellaction = new Spell_SpawnPortal;
	} else if (value == "summon") {
		spellaction = new Spell_Summon;
	} else if (value == "teleport") {
		spellaction = new Spell_Teleport;
	} else {
		LuaError(l, "Unsupported action type: %s" _C_ value.data());
	}
	spellaction->Parse(l, 1, args);
	return spellaction;
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
char Ccl2Condition(lua_State *l, const char *value)
{
	if (!strcmp(value, "true")) {
		return CONDITION_TRUE;
	} else if (!strcmp(value, "false")) {
		return CONDITION_FALSE;
	} else if (!strcmp(value, "only")) {
		return CONDITION_ONLY;
	} else if (value[0] == '<') {
		int v = to_number(value + 1);
		if (v > 100) {
			LuaError(l, "Can only encode condition '<' up to 100%%, got %d" _C_ v);
		}
		return -v;
	} else if (value[0] == '>') {
		int v = to_number(value + 1);
		if (v > 100) {
			LuaError(l, "Can only encode condition '<' up to 100%%, got %d" _C_ v);
		}
		return v + CONDITION_ONLY;
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
static void CclSpellCondition(lua_State *l, ConditionInfo *condition)
{
	// Flags are defaulted to 0(CONDITION_TRUE)
	size_t new_bool_size = UnitTypeVar.GetNumberBoolFlag();

	condition->BoolFlag = new char[new_bool_size];
	memset(condition->BoolFlag, 0, new_bool_size * sizeof(char));

	condition->Variable = new ConditionInfoVariable[UnitTypeVar.GetNumberVariable()];
	// Initialize min/max stuff to values with no effect.
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); i++) {
		condition->Variable[i].Check = false;
		condition->Variable[i].ExactValue = -1;
		condition->Variable[i].ExceptValue = -1;
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
	const int args = lua_rawlen(l, -1);
	for (int j = 0; j < args; ++j) {
		const std::string_view value = LuaToString(l, -1, j + 1);
		++j;
		if (value == "alliance") {
			condition->Alliance = Ccl2Condition(l, LuaToString(l, -1, j + 1));
		} else if (value == "opponent") {
			condition->Opponent = Ccl2Condition(l, LuaToString(l, -1, j + 1));
		} else if (value == "self") {
			condition->TargetSelf = Ccl2Condition(l, LuaToString(l, -1, j + 1));
		} else if (value == "callback") {
			lua_rawgeti(l, -1, j + 1);
			condition->CheckFunc = new LuaCallback(l, -1);
			lua_pop(l, 1);
		} else {
			int index = UnitTypeVar.BoolFlagNameLookup[value];
			if (index != -1) {
				condition->BoolFlag[index] = Ccl2Condition(l, LuaToString(l, -1, j + 1));
				continue;
			}
			index = UnitTypeVar.VariableNameLookup[value];
			if (index != -1) { // Valid index.
				lua_rawgeti(l, -1, j + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "Table expected in variable in condition");
				}
				for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
					const std::string_view key = LuaToString(l, -2);
					condition->Variable[index].Check = true;
					if (key == "Enable") {
						condition->Variable[index].Enable = Ccl2Condition(l, LuaToString(l, -1));
					} else if (key == "ExactValue") {
						condition->Variable[index].ExactValue = LuaToNumber(l, -1);
					} else if (key == "ExceptValue") {
						condition->Variable[index].ExceptValue = LuaToNumber(l, -1);
					} else if (key == "MinValue") {
						condition->Variable[index].MinValue = LuaToNumber(l, -1);
					} else if (key == "MaxValue") {
						condition->Variable[index].MaxValue = LuaToNumber(l, -1);
					} else if (key == "MinMax") {
						condition->Variable[index].MinMax = LuaToNumber(l, -1);
					} else if (key == "MinValuePercent") {
						condition->Variable[index].MinValuePercent = LuaToNumber(l, -1);
					} else if (key == "MaxValuePercent") {
						condition->Variable[index].MaxValuePercent = LuaToNumber(l, -1);
					} else if (key == "ConditionApplyOnCaster") {
						condition->Variable[index].ConditionApplyOnCaster = LuaToBoolean(l, -1);
					} else { // Error
						LuaError(l, "%s invalid for Variable in condition" _C_ key.data());
					}
				}
				lua_pop(l, 1); // lua_rawgeti()
				continue;
			}
			LuaError(l, "Unsuported condition tag: %s" _C_ value.data());
		}
	}
}

/**
**  Parse the Condition for spell.
**
**  @param l         Lua state.
**  @param autocast  pointer to autocast to fill with data.
**
**  @note: autocast must be allocated. All data already in is LOST.
*/
static void CclSpellAutocast(lua_State *l, AutoCastInfo *autocast)
{
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	const int args = lua_rawlen(l, -1);
	for (int j = 0; j < args; ++j) {
		const std::string_view value = LuaToString(l, -1, j + 1);
		++j;
		if (value == "range") {
			autocast->Range = LuaToNumber(l, -1, j + 1);
		} else if (value == "min-range") {
			autocast->MinRange = LuaToNumber(l, -1, j + 1);
		} else if (value == "position-autocast") {
			lua_rawgeti(l, -1, j + 1);
			autocast->PositionAutoCast = new LuaCallback(l, -1);
			lua_pop(l, 1);
		} else if (value == "combat") {
			autocast->Combat = Ccl2Condition(l, LuaToString(l, -1, j + 1));
		} else if (value == "attacker") {
			autocast->Attacker = Ccl2Condition(l, LuaToString(l, -1, j + 1));
		} else if (value == "corpse") {
			autocast->Corpse = Ccl2Condition(l, LuaToString(l, -1, j + 1));
		} else if (value == "priority") {
			lua_rawgeti(l, -1, j + 1);
			if (!lua_istable(l, -1) || lua_rawlen(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			const std::string_view var = LuaToString(l, -1);
			int index = UnitTypeVar.VariableNameLookup[var];// User variables
			if (index == -1) {
				if (var == "Distance") {
					index = ACP_DISTANCE;
				} else {
					fprintf(stderr, "Bad variable name '%s'\n", var.data());
					Exit(1);
				}
			}
			autocast->PriorytyVar = index;
			lua_pop(l, 1);
			autocast->ReverseSort = LuaToBoolean(l, -1, 2);

			lua_pop(l, 1);
		} else if (value == "condition") {
			if (!autocast->Condition) {
				autocast->Condition = new ConditionInfo;
			}
			lua_rawgeti(l, -1, j + 1);
			CclSpellCondition(l, autocast->Condition);
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported autocast tag: %s" _C_ value.data());
		}
	}
}

/**
**  Parse Spell.
**
**  @param l  Lua state.
*/
static int CclDefineSpell(lua_State *l)
{
	const int args = lua_gettop(l);
	const std::string identname = LuaToString(l, 1);
	SpellType *spell = SpellTypeByIdent(identname);
	if (spell != nullptr) {
		DebugPrint("Redefining spell-type '%s'\n" _C_ identname.c_str());
	} else {
		spell = new SpellType(SpellTypeTable.size(), identname);
		for (std::vector<CUnitType *>::size_type i = 0; i < UnitTypes.size(); ++i) { // adjust array for caster already defined
			if (UnitTypes[i]->CanCastSpell) {
				char *newc = new char[(SpellTypeTable.size() + 1) * sizeof(char)];
				memcpy(newc, UnitTypes[i]->CanCastSpell, SpellTypeTable.size() * sizeof(char));
				delete[] UnitTypes[i]->CanCastSpell;
				UnitTypes[i]->CanCastSpell = newc;
				UnitTypes[i]->CanCastSpell[SpellTypeTable.size()] = 0;
			}
			if (UnitTypes[i]->AutoCastActive) {
				char *newc = new char[(SpellTypeTable.size() + 1) * sizeof(char)];
				memcpy(newc, UnitTypes[i]->AutoCastActive, SpellTypeTable.size() * sizeof(char));
				delete[] UnitTypes[i]->AutoCastActive;
				UnitTypes[i]->AutoCastActive = newc;
				UnitTypes[i]->AutoCastActive[SpellTypeTable.size()] = 0;
			}
		}
		SpellTypeTable.push_back(spell);
	}
	for (int i = 1; i < args; ++i) {
		std::string_view value = LuaToString(l, i + 1);
		++i;
		if (value == "showname") {
			spell->Name = LuaToString(l, i + 1);
		} else if (value == "manacost") {
			spell->ManaCost = LuaToNumber(l, i + 1);
		} else if (value == "cooldown") {
			spell->CoolDown = LuaToNumber(l, i + 1);
		} else if (value == "res-cost") {
			lua_pushvalue(l, i + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int len = lua_rawlen(l, -1);
			if (len != MaxCosts) {
				LuaError(l, "resource table size isn't correct");
			}
			for (int j = 1; j < len; ++j) { // exclude the time
				spell->Costs[j] = LuaToNumber(l, -1, j + 1);
			}
			lua_pop(l, 1);
		} else if (value == "range") {
			if (!lua_isstring(l, i + 1) && !lua_isnumber(l, i + 1)) {
				LuaError(l, "incorrect argument");
			}
			if (lua_isstring(l, i + 1) && !strcmp(lua_tostring(l, i + 1), "infinite")) {
				spell->Range = INFINITE_RANGE;
			} else if (lua_isnumber(l, i + 1)) {
				spell->Range = static_cast<int>(lua_tonumber(l, i + 1));
			} else {
				LuaError(l, "Invalid range");
			}
		} else if (value == "repeat-cast") {
			spell->RepeatCast = 1;
			--i;
		} else if (value == "force-use-animation") {
			spell->ForceUseAnimation = true;
			--i;
		} else if (value == "target") {
			value = LuaToString(l, i + 1);
			if (value == "self") {
				spell->Target = TargetSelf;
			} else if (value == "unit") {
				spell->Target = TargetUnit;
			} else if (value == "position") {
				spell->Target = TargetPosition;
			} else {
				LuaError(l, "Unsupported spell target type tag: %s" _C_ value.data());
			}
		} else if (value == "action") {
			if (!lua_istable(l, i + 1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, i + 1);
			for (int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, i + 1, k + 1);
				spell->Action.push_back(CclSpellAction(l));
				lua_pop(l, 1);
			}
		} else if (value == "condition") {
			if (!spell->Condition) {
				spell->Condition = new ConditionInfo;
			}
			lua_pushvalue(l, i + 1);
			CclSpellCondition(l, spell->Condition);
			lua_pop(l, 1);
		} else if (value == "autocast") {
			if (!spell->AutoCast) {
				spell->AutoCast = new AutoCastInfo();
			}
			lua_pushvalue(l, i + 1);
			CclSpellAutocast(l, spell->AutoCast);
			lua_pop(l, 1);
		} else if (value == "ai-cast") {
			if (!spell->AICast) {
				spell->AICast = new AutoCastInfo();
			}
			lua_pushvalue(l, i + 1);
			CclSpellAutocast(l, spell->AICast);
			lua_pop(l, 1);
		} else if (value == "sound-when-cast") {
			//  Free the old name, get the new one
			spell->SoundWhenCast.Name = LuaToString(l, i + 1);
			spell->SoundWhenCast.MapSound();
			//  Check for sound.
			if (!spell->SoundWhenCast.Sound) {
				spell->SoundWhenCast.Name.clear();
			}
		} else if (value == "depend-upgrade") {
			value = LuaToString(l, i + 1);
			spell->DependencyId = UpgradeIdByIdent(value);
			if (spell->DependencyId == -1) {
				lua_pushfstring(l, "Bad upgrade name: %s", value.data());
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value.data());
		}
	}
	return 0;
}

/**
** Register CCL features for Spell.
*/
void SpellCclRegister()
{
	lua_register(Lua, "DefineSpell", CclDefineSpell);
}

//@}
