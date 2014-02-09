//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name script_ai.cpp - The AI ccl functions. */
//
//      (c) Copyright 2000-2009 by Lutz Sammer, Ludovic Pollet,
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

	if (n >= table.size())
	{
		table.resize(n + 1);
	}

	// Look if already known
	std::vector<CUnitType *>::const_iterator i;
	for (i = table[n].begin(); i != table[n].end(); ++i)
	{
		if (*i == base)
		{
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
	
	if (list == "*")
	{
		return UnitTypes;
	}
	int begin = 1;
	int end = 0;
	std::string unitName;
	end = list.find(",", begin);
	while (end != -1)
	{
		unitName = list.substr(begin, end - begin);
		begin = end + 1;
		end = list.find(",", begin);
		if (!unitName.empty())
		{
			Assert(unitName[0] != ',');
			res.push_back(UnitTypeByIdent(unitName));
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

	for (std::vector<CUnitType *>::const_iterator i = UnitTypes.begin(); i != UnitTypes.end(); ++i)
	{
		CUnitType *type = *i;

		if (type->RepairHP > 0)
		{
			res.push_back(type);
		}
	}
	return res;
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

	for (int i = 0; i < (int)UnitButtonTable.size(); ++i)
	{
		const ButtonAction &button = *UnitButtonTable[i];
		const std::vector<CUnitType *> &unitmask = getUnitTypeFromString(button.UnitMask);

		switch (button.Action)
		{
			case ButtonRepair:
				for (std::vector<CUnitType *>::const_iterator j = unitmask.begin(); j != unitmask.end(); ++j)
				{
					for (std::vector<CUnitType *>::const_iterator k = reparableUnits.begin(); k != reparableUnits.end(); ++k)
					{
						AiHelperInsert(aiHelper.Repair, (*k)->Slot, *j);
					}
				}
				break;
			case ButtonBuild:
			{
				CUnitType *buildingType = UnitTypeByIdent(button.ValueStr);

				for (std::vector<CUnitType *>::const_iterator j = unitmask.begin(); j != unitmask.end(); ++j)
				{
					AiHelperInsert(aiHelper.Build, buildingType->Slot, (*j));
				}
				break;
			}
			case ButtonTrain :
			{
				CUnitType *trainingType = UnitTypeByIdent(button.ValueStr);

				for (std::vector<CUnitType *>::const_iterator j = unitmask.begin(); j != unitmask.end(); ++j)
				{
					AiHelperInsert(aiHelper.Train, trainingType->Slot, (*j));
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
	for (j = 0; j < args; ++j)
	{
		LuaCheckTable(l, j + 1);
		subargs = lua_objlen(l, j + 1);
		k = 0;
		value = LuaToString(l, j + 1, k + 1);
		++k;

		//
		// Type build,train.
		//
		if (!strcmp(value, "build"))
		{
			what = -1;
		}
		else if (!strcmp(value, "train"))
		{
			what = -1;
		}
		else if (!strcmp(value, "unit-limit"))
		{
			what = -1;
		}
		else if (!strcmp(value, "unit-equiv"))
		{
			what = 5;
		}
		else if (!strcmp(value, "repair"))
		{
			what = -1;
		}
		else
		{
			LuaError(l, "unknown tag: %s" _C_ value);
			what = -1;
		}

		if (what == -1)
		{
			continue;
		}

		//
		// Get the base unit type, which could handle the action.
		//

		// FIXME: support value as list!
		value = LuaToString(l, j + 1, k + 1);
		++k;
		base = UnitTypeByIdent(value);
		if (!base)
		{
			LuaError(l, "unknown unittype: %s" _C_ value);
		}

		//
		// Get the unit types, which could be produced
		//
		for (; k < subargs; ++k)
		{
			value = LuaToString(l, j + 1, k + 1);
			type = UnitTypeByIdent(value);
			if (!type)
			{
				LuaError(l, "unknown unittype: %s" _C_ value);
			}
			AiHelperInsert(AiHelpers.Equiv, base->Slot, type);
			AiNewUnitTypeEquiv(base, type);
		}
	}
	return 0;
}

/**
**  Define an AI type.
**
**  @param l  Lua state.
**
**  @par Lua parameters;
**  @b ai_type A table that describes the AI type.
**  See the documentation of the AiTypes Lua variable for details.
*/
static int CclDefineAiType(lua_State *l)
{
	// Index of the ai_type parameter from Lua.
	const int aitype_index = 1;

	CAiType *aitype;

	LuaCheckArgs(l, 1);
	luaL_checktype(l, aitype_index, LUA_TTABLE);
	// The following type checks do not use luaL_checktype because
	// it is intended for arguments of functions only and generates
	// error messages accordingly.

	lua_getfield(l, aitype_index, "Ident");
	const int ident_index = lua_gettop(l);
	if (!lua_isstring(l, ident_index))
	{
		LuaError(l, "incorrect argument: Ident of ai_type must be a string");
	}
	size_t ident_length = 0;
	const char *ident_ptr = lua_tolstring(l, ident_index, &ident_length);
	// Leave it on the stack.

	lua_getfield(l, aitype_index, "Init");
	if (!lua_isfunction(l, -1) && !lua_isnil(l, -1))
	{
		LuaError(l, "incorrect argument: Init of ai_type must be a function or nil");
	}
	lua_pop(l, 1);

	lua_getfield(l, aitype_index, "EachSecond");
	if (!lua_isfunction(l, -1))
	{
		LuaError(l, "incorrect argument: EachSecond must be a function");
	}
	lua_pop(l, 1);

	aitype = new CAiType;
	AiTypes.push_back(aitype);

	//
	// AI Name
	//
	aitype->Name.assign(ident_ptr, ident_ptr + ident_length);

#ifdef DEBUG
	for (size_t i = 0; i < AiTypes.size() - 1; ++i)
	{
		const CAiType *ait = AiTypes[i];
		if (aitype->Name == ait->Name)
		{
			DebugPrint("Warning two or more AI's with the same name '%s'\n" _C_ ait->Name.c_str());
		}
	}
#endif

	//
	// Add to the AiTypes Lua variable.
	//
	lua_getfield(l, LUA_GLOBALSINDEX, "AiTypes");
	lua_pushvalue(l, ident_index);
	lua_pushvalue(l, aitype_index);
	lua_settable(l, -3);

	// Lua will discard the values we left on the stack.
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
	for (size_t i = 0; i < AiPlayer->UnitTypeRequests.size(); ++i)
	{
		if (AiPlayer->UnitTypeRequests[i].Type == type)
		{
			return &AiPlayer->UnitTypeRequests[i];
		}
	}
	return NULL;
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
	for (j = 0; j < args; ++j)
	{
		if (lua_isstring(l, j + 1))
		{
			item = LuaToString(l, j + 1);
		}
		else
		{
			item = NULL;
		}

		if (item && !strcmp(item, "none"))
		{
			for (playerid = 0; playerid < NumPlayers; ++playerid)
			{
				if (!Players[playerid].AiEnabled || !Players[playerid].Ai)
				{
					continue;
				}
				((PlayerAi *)Players[playerid].Ai)->ScriptDebug = 0;
			}
		}
		else
		{
			if (item && !strcmp(item, "self"))
			{
				if (!ThisPlayer)
				{
					continue;
				}
				playerid = ThisPlayer->Index;
			}
			else
			{
				playerid = LuaToNumber(l, j + 1);
			}

			if (!Players[playerid].AiEnabled || !Players[playerid].Ai)
			{
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
	if ((autt = FindInUnitTypeRequests(type)))
	{
		autt->Count = LuaToNumber(l, 2);
		// FIXME: 0 should remove it.
	}
	else
	{
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
	int n;

	LuaCheckArgs(l, 1);
	type = CclGetUnitType(l);
	unit_types_count = AiPlayer->Player->UnitTypesCount;
	if (!(autt = FindInUnitTypeRequests(type)))
	{
		//
		// Look if we have this unit-type.
		//
		if (unit_types_count[type->Slot])
		{
			lua_pushboolean(l, 0);
			return 1;
		}

		//
		// Look if we have equivalent unit-types.
		//
		if (type->Slot < (int)AiHelpers.Equiv.size())
		{
			for (j = 0; j < (int)AiHelpers.Equiv[type->Slot].size(); ++j)
			{
				if (unit_types_count[AiHelpers.Equiv[type->Slot][j]->Slot])
				{
					lua_pushboolean(l, 0);
					return 1;
				}
			}
		}
		DebugPrint("Broken? waiting on %s which wasn't requested.\n" _C_ type->Ident.c_str());
		lua_pushboolean(l, 0);
		return 1;
	}
	//
	// Add equivalent units
	//
	n = unit_types_count[type->Slot];
	if (type->Slot < (int)AiHelpers.Equiv.size())
	{
		for (j = 0; j < (int)AiHelpers.Equiv[type->Slot].size(); ++j)
		{
			n += unit_types_count[AiHelpers.Equiv[type->Slot][j]->Slot];
		}
	}
	// units available?

	if (n >= autt->Count)
	{
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
	size_t i;

	LuaCheckArgs(l, 2);
	LuaCheckTable(l, 2);
	force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCES)
	{
		LuaError(l, "Force out of range: %d" _C_ force);
	}

	args = lua_objlen(l, 2);
	for (j = 0; j < args; ++j)
	{
		lua_rawgeti(l, 2, j + 1);
		type = CclGetUnitType(l);
		lua_pop(l, 1);
		++j;
		count = LuaToNumber(l, 2, j + 1);

		if (!type)
		{
			continue;
		}

		// Use the equivalent unittype.
		type = UnitTypes[UnitTypeEquivs[type->Slot]];

		//
		// Look if already in force.
		//
		for (i = 0; i < AiPlayer->Force[force].UnitTypes.size(); ++i)
		{
			aiut = &AiPlayer->Force[force].UnitTypes[i];
			if (aiut->Type->Slot == type->Slot)
			{
				// found
				if (count)
				{
					aiut->Want = count;
				}
				else
				{
					AiPlayer->Force[force].UnitTypes.erase(
						AiPlayer->Force[force].UnitTypes.begin() + i);
				}
				break;
			}
		}

		//
		// New type append it.
		//
		if (i == AiPlayer->Force[force].UnitTypes.size())
		{
			AiUnitType newaiut;
			newaiut.Want = count;
			newaiut.Type = type;
			AiPlayer->Force[force].UnitTypes.push_back(newaiut);
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
	int force;
	const char *flag;

	LuaCheckArgs(l, 2);
	force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCES)
	{
		LuaError(l, "Force %i out of range" _C_ force);
	}
	flag = LuaToString(l, 2);
	if (!strcmp(flag, "attack"))
	{
		AiPlayer->Force[force].Role = AiForceRoleAttack;
	}
	else if (!strcmp(flag, "defend"))
	{
		AiPlayer->Force[force].Role = AiForceRoleDefend;
	}
	else
	{
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
	int force;

	LuaCheckArgs(l, 1);
	force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCES)
	{
		lua_pushfstring(l, "Force out of range: %d", force);
	}
	if (AiPlayer->Force[force].Completed)
	{
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
	int force;

	LuaCheckArgs(l, 1);
	force = LuaToNumber(l, 1);
	if (force < 0 || force >= AI_MAX_FORCES)
	{
		LuaError(l, "Force out of range: %d" _C_ force);
	}
	if (AiPlayer->Force[force].Completed)
	{
		lua_pushboolean(l, 0);
		return 1;
	}

#if 0
	// Debuging
	AiCleanForces();
	Assert(!AiPlayer->Force[force].Completed);
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
	if (force < 0 || force >= AI_MAX_FORCES)
	{
		LuaError(l, "Force out of range: %d" _C_ force);
	}

	AiAttackWithForce(force);

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
	if (AiPlayer->SleepCycles || i == 0)
	{
		if (AiPlayer->SleepCycles < GameCycle)
		{
			AiPlayer->SleepCycles = 0;
			lua_pushboolean(l, 0);
			return 1;
		}
	}
	else
	{
		AiPlayer->SleepCycles = GameCycle + i;
	}

	lua_pushboolean(l, 1);
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
**  Get the default resource number
**
**  @param name  Resource name.
**
**  @return      The number of the resource in DefaultResourceNames
*/
static int DefaultResourceNumber(const char *name)
{
	for (int i = 0; i < MaxCosts; ++i)
	{
		if (DefaultResourceNames[i] == name)
		{
			return i;
		}
	}
	// Resource not found, should never happen
	Assert(0);
	return -1;
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
	DebugPrint("%p %d\n" _C_ Players[i].Ai _C_ Players[i].AiEnabled );
	// FIXME: lose this:
	// Assert(!Players[i].Ai && Players[i].AiEnabled);

	ai = Players[i].Ai = new PlayerAi;
	ai->Player = &Players[i];

	//
	// Parse the list: (still everything could be changed!)
	//
	for (; j < args; ++j)
	{
		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "ai-type"))
		{
			CAiType *ait = NULL;

			value = LuaToString(l, j + 1);
			for (k = 0; k < (int)AiTypes.size(); ++k)
			{
				ait = AiTypes[k];
				if (ait->Name == value)
				{
					break;
				}
			}
			if (k == (int)AiTypes.size())
			{
				lua_pushfstring(l, "ai-type not found: %s", value);
			}
			ai->AiType = ait;
		}
		else if (!strcmp(value, "script"))
		{
			// Ignore, for compatibility with games saved
			// by earlier versions.
		}
		else if (!strcmp(value, "script-debug"))
		{
			ai->ScriptDebug = LuaToBoolean(l, j + 1);
		}
		else if (!strcmp(value, "sleep-cycles"))
		{
			ai->SleepCycles = LuaToNumber(l, j + 1);
		}
		else if (!strcmp(value, "force"))
		{
			LuaCheckTable(l, j + 1);
			subargs = lua_objlen(l, j + 1);
			k = 0;
			i = LuaToNumber(l, j + 1, k + 1);
			++k;
			for (; k < subargs; ++k)
			{
				value = LuaToString(l, j + 1, k + 1);
				++k;
				if (!strcmp(value, "complete"))
				{
					ai->Force[i].Completed = true;
					--k;
				}
				else if (!strcmp(value, "recruit"))
				{
					ai->Force[i].Completed = false;
					--k;
				}
				else if (!strcmp(value, "attack"))
				{
					ai->Force[i].Attacking = true;
					--k;
				}
				else if (!strcmp(value, "defend"))
				{
					ai->Force[i].Defending = true;
					--k;
				}
				else if (!strcmp(value, "role"))
				{
					value = LuaToString(l, j + 1, k + 1);
					if (!strcmp(value, "attack"))
					{
						ai->Force[i].Role = AiForceRoleAttack;
					}
					else if (!strcmp(value, "defend"))
					{
						ai->Force[i].Role = AiForceRoleDefend;
					}
					else
					{
						LuaError(l, "Unsupported force tag: %s" _C_ value);
					}
				}
				else if (!strcmp(value, "types"))
				{
					int subsubargs;
					int subk;

					lua_rawgeti(l, j + 1, k + 1);
					LuaCheckTable(l, -1);
					subsubargs = lua_objlen(l, -1);
					for (subk = 0; subk < subsubargs; ++subk)
					{
						int num;
						const char *ident;
						AiUnitType queue;

						num = LuaToNumber(l, -1, subk + 1);
						++subk;
						ident = LuaToString(l, -1, subk + 1);
						queue.Want = num;
						queue.Type = UnitTypeByIdent(ident);
						ai->Force[i].UnitTypes.push_back(queue);
					}
					lua_pop(l, 1);
				}
				else if (!strcmp(value, "units"))
				{
					int subsubargs;
					int subk;

					lua_rawgeti(l, j + 1, k + 1);
					LuaCheckTable(l, -1);
					subsubargs = lua_objlen(l, -1);
					for (subk = 0; subk < subsubargs; ++subk)
					{
						int num;

						num = LuaToNumber(l, -1, subk + 1);
						++subk;
						LuaToString(l, -1, subk + 1);
						ai->Force[i].Units.push_back(UnitSlots[num]);
					}
					lua_pop(l, 1);
				}
				else if (!strcmp(value, "state"))
				{
					ai->Force[i].State = LuaToNumber(l, j + 1, k + 1);
				}
				else if (!strcmp(value, "goalx"))
				{
					ai->Force[i].GoalX = LuaToNumber(l, j + 1, k + 1);
				}
				else if (!strcmp(value, "goaly"))
				{
					ai->Force[i].GoalY = LuaToNumber(l, j + 1, k + 1);
				}
				else
				{
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		}
		else if (!strcmp(value, "needed"))
		{
			LuaCheckTable(l, j + 1);
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k)
			{
				const char *type;
				int num;

				type = LuaToString(l, j + 1, k + 1);
				++k;
				num = LuaToNumber(l, j + 1, k + 1);
				ai->Needed[DefaultResourceNumber(type)] = num;
			}
		}
		else if (!strcmp(value, "need-mask"))
		{
			LuaCheckTable(l, j + 1);
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k)
			{
				const char *type;

				type = LuaToString(l, j + 1, k + 1);
				ai->NeededMask |= (1 << DefaultResourceNumber(type));
			}
		}
		else if (!strcmp(value, "last-can-not-move-cycle"))
		{
			ai->LastCanNotMoveGameCycle = LuaToNumber(l, j + 1);
		}
		else if (!strcmp(value, "unit-type"))
		{
			LuaCheckTable(l, j + 1);
			subargs = lua_objlen(l, j + 1);
			i = 0;
			if (subargs)
			{
				ai->UnitTypeRequests.resize(subargs / 2);
			}
			for (k = 0; k < subargs; ++k)
			{
				const char *ident;
				int count;

				ident = LuaToString(l, j + 1, k + 1);
				++k;
				count = LuaToNumber(l, j + 1, k + 1);
				ai->UnitTypeRequests[i].Type = UnitTypeByIdent(ident);
				ai->UnitTypeRequests[i].Count = count;
				++i;
			}
		}
		else if (!strcmp(value, "building"))
		{
			LuaCheckTable(l, j + 1);
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k)
			{
				const char *ident;
				int made;
				int want;
				AiBuildQueue queue;

				ident = LuaToString(l, j + 1, k + 1);
				++k;
				made = LuaToNumber(l, j + 1, k + 1);
				++k;
				want = LuaToNumber(l, j + 1, k + 1);
				queue.Type = UnitTypeByIdent(ident);
				queue.Want = want;
				queue.Made = made;
				ai->UnitTypeBuilt.push_back(queue);
			}
		}
		else if (!strcmp(value, "repair-building"))
		{
			ai->LastRepairBuilding = LuaToNumber(l, j + 1);
		}
		else if (!strcmp(value, "repair-workers"))
		{
			LuaCheckTable(l, j + 1);
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k)
			{
				int num;
				int workers;

				num = LuaToNumber(l, j + 1, k + 1);
				++k;
				workers = LuaToNumber(l, j + 1, k + 1);
				ai->TriedRepairWorkers[num] = workers;
				++i;
			}
		}
		else
		{
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
	lua_register(Lua, "DefineAiType", CclDefineAiType);

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

	lua_register(Lua, "AiPlayer", CclAiPlayer);

	lua_register(Lua, "DefineAiPlayer", CclDefineAiPlayer);
}


//@}
