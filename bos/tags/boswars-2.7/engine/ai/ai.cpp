//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name ai.cpp - The computer player AI main file. */
//
//      (c) Copyright 2000-2009 by Lutz Sammer, Ludovic Pollet, and
//                                 Jimmy Salmon
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

//----------------------------------------------------------------------------
// Documentation
//----------------------------------------------------------------------------

/**
** @page AiModule Module - AI
**
** @section aibasics What is it?
**
** @subsection aiscripted What is scripted AI?
**
** The AI script tells the engine build 4 workers, than build 3 footman,
** than attack the player, than sleep 100 frames.
**
** @section API The AI API
**
** @subsection aimanage Management calls
**
** Manage the inititialse and cleanup of the AI players.
**
** ::InitAiModule()
**
** Initialise all global varaibles and structures.
** Called before AiInit, or before game loading.
**
** ::AiInit(::CPlayer)
**
** Called for each player, to setup the AI structures
** CPlayer::Ai in the player structure and possibly call
** AI initialization scripts. It can use CPlayer::AiName to
** select different AI's.
**
** ::CleanAi()
**
** Called to release all the memory for all AI structures.
** Must handle self which players contains AI structures.
**
** ::SaveAi(::FILE *)
**
** Save the AI structures of all players to file.
** Must handle self which players contains AI structures.
**
**
** @subsection aipcall Periodic calls
**
** This functions are called regular for all AI players.
**
** ::AiEachCycle(::Player)
**
** Called each game cycle, to handle quick checks, which needs
** less CPU.
**
** ::AiEachSecond(::Player)
**
** Called each second, to handle more CPU intensive things.
**
**
** @subsection aiecall Event call-backs
**
** This functions are called, when some special events happens.
**
** ::AiHelpMe()
**
** Called if a unit owned by the AI is attacked.
**
** ::AiUnitKilled()
**
** Called if a unit owned by the AI is killed.
**
** ::AiWorkComplete()
**
** Called if a unit has completed its work.
**
** ::AiCanNotBuild()
**
** Called if the AI unit can't build the requested unit-type.
**
** ::AiCanNotReach()
**
** Called if the AI unit can't reach the building place.
**
** ::AiTrainingComplete()
**
** Called if AI unit has completed training a new unit.
*/

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"

#include "player.h"
#include "unit.h"
#include "unittype.h"
#include "upgrade.h"
#include "script.h"
#include "actions.h"
#include "map.h"
#include "pathfinder.h"
#include "ai_local.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/** List of all AI types.
 * DefineAiType keeps this in sync with the AiTypes Lua variable.  */
std::vector<CAiType *> AiTypes;

AiHelper AiHelpers;             /// AI helper variables

PlayerAi *AiPlayer;             /// Current AI player

/*----------------------------------------------------------------------------
-- Lowlevel functions
----------------------------------------------------------------------------*/

/**
**  Execute a Lua function of the AI type.
*/
static void AiExecuteFunction(const char *field)
{
	if (AiPlayer->AiType != NULL)
	{
		lua_getfield(Lua, LUA_GLOBALSINDEX, "AiTypes");
		lua_getfield(Lua, -1, AiPlayer->AiType->Name.c_str());
		lua_getfield(Lua, -1, field);
		if (!lua_isnil(Lua, -1))
		{
			LuaCall(0, 1);	// removes the function from the stack
			lua_pop(Lua, 2); // remove the tables too
		}
		else
		{
			lua_pop(Lua, 3);
		}
	}
}

/**
**  Check if everything is fine, send new requests to resource manager.
*/
static void AiCheckUnits()
{
	int counter[UnitTypeMax];
	int attacking[UnitTypeMax];
	const int *unit_types_count;
	int i;

	memset(counter, 0, sizeof(counter));
	memset(attacking, 0, sizeof(attacking));

	//
	//  Count the already made build requests.
	//
	for (i = 0; i < (int)AiPlayer->UnitTypeBuilt.size(); ++i)
	{
		AiBuildQueue *queue = &AiPlayer->UnitTypeBuilt[i];
		counter[queue->Type->Slot] += queue->Want;
	}

	unit_types_count = AiPlayer->Player->UnitTypesCount;

	//
	//  Look if some unit-types are missing.
	//
	for (i = 0; i < (int)AiPlayer->UnitTypeRequests.size(); ++i)
	{
		int slot = AiPlayer->UnitTypeRequests[i].Type->Slot;
		int count = AiPlayer->UnitTypeRequests[i].Count;
		int e;

		//
		// Add equivalent units
		//
		e = unit_types_count[slot];
		if (slot < (int)AiHelpers.Equiv.size())
		{
			for (int j = 0; j < (int)AiHelpers.Equiv[slot].size(); ++j)
			{
				e += unit_types_count[AiHelpers.Equiv[slot][j]->Slot];
			}
		}

		if (count > e + counter[slot])
		{
			// Request it.
			AiAddUnitTypeRequest(AiPlayer->UnitTypeRequests[i].Type,
				count - e - counter[slot]);
			counter[slot] += count - e - counter[slot];
		}
		counter[slot] -= count;
	}

	//
	// Look through the forces what is missing.
	//
	for (i = AI_MAX_FORCES; i < AI_MAX_ATTACKING_FORCES; ++i)
	{
		for (int j = 0; j < (int)AiPlayer->Force[i].Units.size(); ++j)
		{
			attacking[AiPlayer->Force[i].Units[j]->Type->Slot]++;
		}
	}

	//
	// create missing units
	//
	for (i = 0; i < AI_MAX_FORCES; ++i)
	{
		// No troops for attacking force
		if (!AiPlayer->Force[i].Defending && AiPlayer->Force[i].Attacking)
		{
			continue;
		}

		for (int j = 0; j < (int)AiPlayer->Force[i].UnitTypes.size(); ++j)
		{
			const AiUnitType *aiut = &AiPlayer->Force[i].UnitTypes[j];
			int slot = aiut->Type->Slot;
			int want = aiut->Want;
			if (want > unit_types_count[slot] + counter[slot] - attacking[slot])
			{
				// Request it.
				AiAddUnitTypeRequest(aiut->Type,
					want - (unit_types_count[slot] + counter[slot] - attacking[slot]));
				counter[slot] += want - (unit_types_count[slot] + counter[slot] - attacking[slot]);
				AiPlayer->Force[i].Completed = false;
			}
			counter[slot] -= want;
		}
	}
}

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
**  Save state of player AI.
**
**  @param file   Output file.
**  @param plynr  Player number.
**  @param ai     Player AI.
*/
static void SaveAiPlayer(CFile *file, int plynr, PlayerAi *ai)
{
	int i;

	file->printf("DefineAiPlayer(%d,\n", plynr);
	file->printf("  \"ai-type\", \"%s\",\n", ai->AiType->Name.c_str());

	file->printf("  \"script-debug\", %s,\n", ai->ScriptDebug ? "true" : "false");
	file->printf("  \"sleep-cycles\", %lu,\n", ai->SleepCycles);

	//
	//  All forces
	//
	for (i = 0; i < AI_MAX_ATTACKING_FORCES; ++i)
	{
		int j;

		file->printf("  \"force\", {%d, %s%s%s", i,
			ai->Force[i].Completed ? "\"complete\"," : "\"recruit\",",
			ai->Force[i].Attacking ? " \"attack\"," : "",
			ai->Force[i].Defending ? " \"defend\"," : "");

		file->printf(" \"role\", ");
		switch (ai->Force[i].Role)
		{
			case AiForceRoleAttack:
				file->printf("\"attack\",");
				break;
			case AiForceRoleDefend:
				file->printf("\"defend\",");
				break;
			default:
				file->printf("\"unknown-%d\",", ai->Force[i].Role);
				break;
		}

		file->printf("\n    \"types\", { ");
		for (j = 0; j < (int)ai->Force[i].UnitTypes.size(); ++j)
		{
			const AiUnitType *aut = &ai->Force[i].UnitTypes[j];
			file->printf("%d, \"%s\", ", aut->Want, aut->Type->Ident.c_str());
		}
		file->printf("},\n    \"units\", {");
		for (j = 0; j < (int)ai->Force[i].Units.size(); ++j)
		{
			const CUnit *aiunit = ai->Force[i].Units[j];
			file->printf(" %d, \"%s\",", UnitNumber(aiunit),
				aiunit->Type->Ident.c_str());
		}
		file->printf("},\n    \"state\", %d, \"goalx\", %d, \"goaly\", %d,",
			ai->Force[i].State, ai->Force[i].GoalX, ai->Force[i].GoalY);
		file->printf("},\n");
	}

	file->printf("  \"needed\", {");
	for (i = 0; i < MaxCosts; ++i)
	{
		file->printf("\"%s\", %d, ", DefaultResourceNames[i].c_str(), ai->Needed[i]);
	}
	file->printf("},\n");

	file->printf("  \"need-mask\", {");
	for (i = 0; i < MaxCosts; ++i)
	{
		if (ai->NeededMask & (1 << i))
		{
			file->printf("\"%s\", ", DefaultResourceNames[i].c_str());
		}
	}
	file->printf("},\n");

	//
	//  Requests
	//
	file->printf("  \"last-can-not-move-cycle\", %lu,\n", ai->LastCanNotMoveGameCycle);
	file->printf("  \"unit-type\", {");
	for (i = 0; i < (int)ai->UnitTypeRequests.size(); ++i)
	{
		file->printf("\"%s\", ", ai->UnitTypeRequests[i].Type->Ident.c_str());
		file->printf("%d, ", ai->UnitTypeRequests[i].Count);
	}
	file->printf("},\n");

	//
	//  Building queue
	//
	file->printf("  \"building\", {");
	for (i = 0; i < (int)ai->UnitTypeBuilt.size(); ++i)
	{
		const AiBuildQueue *queue = &ai->UnitTypeBuilt[i];
		file->printf("\"%s\", %d, %d, ", queue->Type->Ident.c_str(), queue->Made, queue->Want);
	}
	file->printf("},\n");

	file->printf("  \"repair-building\", %u,\n", ai->LastRepairBuilding);

	file->printf("  \"repair-workers\", {");
	for (i = 0; i < UnitMax; ++i)
	{
		if (ai->TriedRepairWorkers[i])
		{
			file->printf("%d, %d, ", i, ai->TriedRepairWorkers[i]);
		}
	}
	file->printf("})\n\n");
}

/**
**  Save state of player AIs.
**
**  @param file  Output file.
*/
static void SaveAiPlayers(CFile *file)
{
	for (int p = 0; p < PlayerMax; ++p)
	{
		if (Players[p].Ai)
		{
			SaveAiPlayer(file, p, Players[p].Ai);
		}
	}
}

/**
**  Save state of AI to file.
**
**  @param file  Output file.
*/
void SaveAi(CFile *file)
{
	file->printf("\n--- -----------------------------------------\n");

	SaveAiPlayers(file);
}

/**
**  Setup all at start.
**
**  @param player  The player structure pointer.
*/
void AiInit(CPlayer *player)
{
	PlayerAi *pai;
	CAiType *ait;
	CAiType *passive = NULL;
	size_t i;

	pai = new PlayerAi;
	if (!pai)
	{
		fprintf(stderr, "Out of memory.\n");
		exit(0);
	}

	pai->Player = player;
	ait = NULL;

	DebugPrint("Player %d (%s) looking for \"%s\"\n" _C_
		   player->Index _C_
		   player->Name.c_str() _C_
		   player->AiName.c_str());

	//
	//  Search correct AI type.
	//
	if (AiTypes.empty())
	{
		fprintf(stderr, "AI: Got no scripts at all! You need at least the \"ai-passive\" fallback script.\n");
		fprintf(stderr, "AI: Look at the DefineAi() documentation.\n");
		Exit(1);
	}
	for (i = 0; i < AiTypes.size(); ++i)
	{
		ait = AiTypes[i];
		if (player->AiName.empty() || ait->Name == player->AiName)
		{
			break;
		}

		if (ait->Name == "ai-passive")
		{
			passive = ait;
		}
	}
	if (i == AiTypes.size())
	{
		DebugPrint("AI: Script \"%s\" not found!\n" _C_
			   player->AiName.c_str());

		// Fall back to the passive script only, because it
		// does not require any kind of saved AI state.
		// Furthermore, it does not alter the existing AI
		// state, so the game can be saved with this fallback
		// and then loaded into a build that does support the
		// correct script.
		if (passive == NULL)
		{
			fprintf(stderr, "AI: Script \"ai-passive\" not found either!  Cannot continue.\n");
			Exit(1);
		}

		ait = passive;
		DebugPrint("AI: Using fallback \"%s\".\n" _C_
			   ait->Name.c_str());
	}

	pai->AiType = ait;
	player->Ai = pai;

	// Initialize the AI state of the player if it has not been
	// already initialized, i.e. we're not loading a saved game.
	lua_getfield(Lua, LUA_GLOBALSINDEX, "AiState"); // stack: AiState
	Assert(lua_istable(Lua, -1));
	lua_pushnumber(Lua, player->Index);             // stack: AiState, player
	lua_gettable(Lua, -2);                          // stack: AiState, AiState[player]
	bool needs_init = lua_isnil(Lua, -1);
	lua_pop(Lua, 2);
	if (needs_init)
	{
		AiPlayer = pai;
		AiExecuteFunction("Init");
	}

	// Cause a reproduceable crash if something expected us to
	// preserve the value of AiPlayer.
	AiPlayer = NULL;
}

/**
**  Initialize global structures of the AI
*/
void InitAiModule()
{
	AiResetUnitTypeEquiv();
}

/**
**  Cleanup the AI in order to enable to restart a game.
*/
void CleanAi()
{
	for (int p = 0; p < PlayerMax; ++p)
	{
		if (Players[p].Ai)
		{
			delete Players[p].Ai;
			Players[p].Ai = NULL;
		}
	}
}

/**
**  Free all AI resources.
*/
void FreeAi()
{
	CleanAi();
	
	//
	//  Free AiTypes.
	//
	for (size_t i = 0; i < AiTypes.size(); ++i)
	{
		delete AiTypes[i];
	}
	AiTypes.clear();

	//
	//  Free AiHelpers.
	//
	AiHelpers.Train.clear();
	AiHelpers.Build.clear();
	AiHelpers.Repair.clear();
	AiHelpers.UnitLimit.clear();
	AiHelpers.Equiv.clear();

	AiResetUnitTypeEquiv();
}

/*----------------------------------------------------------------------------
-- Support functions
----------------------------------------------------------------------------*/

/**
**  Remove unit-type from build list.
**
**  @param pai   Computer AI player.
**  @param type  Unit-type which is now available.
**  @return      True, if unit-type was found in list.
*/
static int AiRemoveFromBuilt2(PlayerAi *pai, const CUnitType *type)
{
	std::vector<AiBuildQueue>::iterator i;

	for (i = pai->UnitTypeBuilt.begin(); i != pai->UnitTypeBuilt.end(); ++i)
	{
		Assert((*i).Want);
		if (type == (*i).Type && (*i).Made)
		{
			--(*i).Made;
			if (!--(*i).Want)
			{
				pai->UnitTypeBuilt.erase(i);
			}
			return 1;
		}
	}
	return 0;
}

/**
**  Remove unit-type from build list.
**
**  @param pai   Computer AI player.
**  @param type  Unit-type which is now available.
*/
static void AiRemoveFromBuilt(PlayerAi *pai, const CUnitType *type)
{
	int equivalents[UnitTypeMax + 1];
	int equivalentsCount;

	if (AiRemoveFromBuilt2(pai, type))
	{
		return;
	}

	//
	//  This could happen if an upgrade is ready, look for equivalent units.
	//
	equivalentsCount = AiFindUnitTypeEquiv(type, equivalents);
	for (int i = 0; i < equivalentsCount; ++i)
	{
		if (AiRemoveFromBuilt2(pai, UnitTypes[equivalents[i]]))
		{
			return;
		}
	}

	if (pai->Player == ThisPlayer)
	{
		DebugPrint("My guess is that you built something under ai me. naughty boy!\n");
		return;
	}

	Assert(0);
}

/**
**  Reduce made unit-type from build list.
**
**  @param pai   Computer AI player.
**  @param type  Unit-type which is now available.
**  @return      True if the unit-type could be reduced.
*/
static int AiReduceMadeInBuilt2(PlayerAi *pai, const CUnitType *type)
{
	std::vector<AiBuildQueue>::iterator i;

	for (i = pai->UnitTypeBuilt.begin(); i != pai->UnitTypeBuilt.end(); ++i)
	{
		if (type == (*i).Type && (*i).Made)
		{
			(*i).Made--;
			return 1;
		}
	}
	return 0;
}

/**
**  Reduce made unit-type from build list.
**
**  @param pai   Computer AI player.
**  @param type  Unit-type which is now available.
*/
static void AiReduceMadeInBuilt(PlayerAi *pai, const CUnitType *type)
{
	int equivs[UnitTypeMax + 1];
	int equivnb;

	if (AiReduceMadeInBuilt2(pai, type))
	{
		return;
	}
	//
	//  This could happen if an upgrade is ready, look for equivalent units.
	//
	equivnb = AiFindUnitTypeEquiv(type, equivs);

	for (size_t i = 0; i < (size_t)equivnb; ++i)
	{
		if (AiReduceMadeInBuilt2(pai, UnitTypes[equivs[i]]))
		{
			return;
		}
	}

	Assert(0);
}

/*----------------------------------------------------------------------------
-- Callback Functions
----------------------------------------------------------------------------*/

/**
**  Called if a Unit is Attacked
**
**  @param attacker  Pointer to attacker unit.
**  @param defender  Pointer to unit that is being attacked.
*/
void AiHelpMe(const CUnit *attacker, CUnit *defender)
{
	PlayerAi *pai;
	CUnit *aiunit;
	int force;
	int destx, desty;

	DebugPrint("%d: %d(%s) attacked at %d,%d\n" _C_
		defender->Player->Index _C_ UnitNumber(defender) _C_
		defender->Type->Ident.c_str() _C_ defender->X _C_ defender->Y);

	AiPlayer = pai = defender->Player->Ai;
	if (pai->Force[0].Attacking)
	{
		// Force 0 busy
		return;
	}

	//
	//  If unit belongs to an attacking force, don't defend it.
	//
	for (force = 0; force < AI_MAX_ATTACKING_FORCES; ++force)
	{
		if (!pai->Force[force].Attacking)
		{
			// none attacking
			// FIXME, send the force for help
			continue;
		}

		for (size_t i = 0; i < pai->Force[force].Units.size(); ++i)
		{
			aiunit = pai->Force[force].Units[i];
			if (defender == aiunit)
			{
				return;
			}
		}
	}

	//
	//  Send forces
	//
	destx = attacker ? attacker->X : defender->X;
	desty = attacker ? attacker->Y : defender->Y;

	// Send force 0 defending
	AiAttackWithForceAt(0, destx, desty);
	pai->Force[0].Defending = true;

	// Also send force 1 if not already attacking
	if (!pai->Force[1].Attacking)
	{
		pai->Force[1].Defending = true;
		AiAttackWithForceAt(1, destx, desty);
	}
}

/**
**  Called when a unit is killed.
**
**  @param unit  Pointer to unit.
*/
void AiUnitKilled(CUnit *unit)
{
	DebugPrint("%d: %d(%s) killed\n" _C_
		unit->Player->Index _C_ UnitNumber(unit) _C_ unit->Type->Ident.c_str());

	Assert(unit->Player->Type != PlayerPerson);

	// FIXME: must handle all orders...
	switch (unit->Orders[0]->Action)
	{
		case UnitActionStill:
		case UnitActionAttack:
		case UnitActionMove:
			break;

		case UnitActionBuilt:
			DebugPrint("%d: %d(%s) killed, under construction!\n" _C_
				unit->Player->Index _C_ UnitNumber(unit) _C_ unit->Type->Ident.c_str());
			AiReduceMadeInBuilt(unit->Player->Ai, unit->Type);
			break;

		case UnitActionBuild:
			DebugPrint("%d: %d(%s) killed, with order %s!\n" _C_
				unit->Player->Index _C_ UnitNumber(unit) _C_
				unit->Type->Ident.c_str() _C_ unit->Orders[0]->Type->Ident.c_str());
			if (!unit->Orders[0]->Goal)
			{
				AiReduceMadeInBuilt(unit->Player->Ai, unit->Orders[0]->Type);
			}
			break;

		default:
			DebugPrint("FIXME: %d: %d(%s) killed, with order %d!\n" _C_
				unit->Player->Index _C_ UnitNumber(unit) _C_
				unit->Type->Ident.c_str() _C_ unit->Orders[0]->Action);
			break;
	}
}

/**
**  Called when work complete (Buildings).
**
**  @param unit     The unit that was building the building.
**  @param newUnit  New unit that was built.
*/
void AiWorkComplete(CUnit *unit, CUnit *newUnit)
{
	if (unit)
	{
		DebugPrint("%d: %d(%s) build %s at %d,%d completed\n" _C_
			newUnit->Player->Index _C_ UnitNumber(unit) _C_ unit->Type->Ident.c_str() _C_
			newUnit->Type->Ident.c_str() _C_ unit->X _C_ unit->Y);
	}
	else
	{
		DebugPrint("%d: building %s at %d,%d completed\n" _C_
			newUnit->Player->Index _C_ newUnit->Type->Ident.c_str() _C_ newUnit->X _C_ newUnit->Y);
	}

	Assert(newUnit->Player->Type != PlayerPerson);
	AiRemoveFromBuilt(newUnit->Player->Ai, newUnit->Type);
}

/**
**  Called if building can't be built.
**
**  @param unit      The unit trying to build.
**  @param unitType  Unit type trying to be built.
*/
void AiCanNotBuild(CUnit *unit, const CUnitType *unitType)
{
	DebugPrint("%d: %d(%s) Can't build %s at %d,%d\n" _C_
		unit->Player->Index _C_ UnitNumber(unit) _C_ unit->Type->Ident.c_str() _C_
		unitType->Ident.c_str() _C_ unit->X _C_ unit->Y);

	Assert(unit->Player->Type != PlayerPerson);
	AiReduceMadeInBuilt(unit->Player->Ai, unitType);
}

/**
**  Called if building place can't be reached.
**
**  @param unit      The unit trying to build.
**  @param unitType  Unit type trying to be built.
*/
void AiCanNotReach(CUnit *unit, const CUnitType *unitType)
{
	Assert(unit->Player->Type != PlayerPerson);
	AiReduceMadeInBuilt(unit->Player->Ai, unitType);
}

/**
**  Try to move a unit that's in the way
*/
static void AiMoveUnitInTheWay(CUnit *unit)
{
	static int dirs[8][2] = {{-1,-1},{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1}};
	int ux0;
	int uy0;
	int ux1;
	int uy1;
	int bx0;
	int by0;
	int bx1;
	int by1;
	int x;
	int y;
	int trycount,i;
	CUnit *blocker;
	CUnitType *unittype;
	CUnitType *blockertype;
	CUnit *movableunits[16];
	int movablepos[16][2];
	int movablenb;

	AiPlayer = unit->Player->Ai;
	unittype = unit->Type;

	ux0 = unit->X;
	uy0 = unit->Y;
	ux1 = ux0 + unittype->TileWidth - 1;
	uy1 = uy0 + unittype->TileHeight - 1;

	movablenb = 0;


	// Try to make some unit moves around it
	for (i = 0; i < NumUnits; ++i)
	{
		blocker = Units[i];

		if (blocker->IsUnusable())
		{
			continue;
		}

		if (!blocker->IsIdle())
		{
			continue;
		}

		if (blocker->Player != unit->Player)
		{
			// Not allied
			if (!(blocker->Player->Allied & (1 << unit->Player->Index)))
			{
				continue;
			}
		}

		blockertype = blocker->Type;

		if (blockertype->UnitType != unittype->UnitType)
		{
			continue;
		}

		if (!CanMove(blocker))
		{
			continue;
		}

		bx0 = blocker->X;
		by0 = blocker->Y;
		bx1 = bx0 + blocker->Type->TileWidth - 1;
		by1 = by0 + blocker->Type->TileHeight - 1;;

		// Check for collision
		if (!((ux0 == bx1 + 1 || ux1 == bx0 - 1) &&
				(std::max(by0, uy0) <= std::min(by1, uy1))) &&
			!((uy0 == by1 + 1 || uy1 == by0 - 1) &&
				(std::max(bx0, ux0) <= std::min(bx1, ux1))))
		{
			continue;
		}

		if (unit == blocker)
		{
			continue;
		}

		// Move blocker in a rand dir
		i = SyncRand() & 7;
		trycount = 8;
		while (trycount > 0)
		{
			i = (i + 1) & 7;
			--trycount;

			x = blocker->X + dirs[i][0];
			y = blocker->Y + dirs[i][1];

			// Out of the map => no !
			if (x < 0 || y < 0 || x >= Map.Info.MapWidth || y >= Map.Info.MapHeight)
			{
				continue;
			}
			// move to blocker ? => no !
			if (x == ux0 && y == uy0)
			{
				continue;
			}

			movableunits[movablenb] = blocker;
			movablepos[movablenb][0] = x;
			movablepos[movablenb][1] = y;

			++movablenb;
			trycount = 0;
		}
		if (movablenb >= 16)
		{
			break;
		}
	}

	// Don't move more than 1 unit.
	if (movablenb)
	{
		i = SyncRand() % movablenb;
		CommandMove(movableunits[i], movablepos[i][0], movablepos[i][1], FlushCommands);
	}
}

/**
**  Called if a unit can't move. Try to move unit in the way
**
**  @param unit  Pointer to unit what builds the building.
**
**  @pre         The unit's field flags must have been marked
**               on the map; see MarkUnitFieldFlags.
*/
void AiCanNotMove(CUnit *unit)
{
	// Please do not use UnmarkUnitFieldFlags and MarkUnitFieldFlags
	// around AiCanNotMove calls.
	Assert((Map.Field(unit->X, unit->Y)->Flags & unit->Type->FieldFlags) != 0);

	AiPlayer = unit->Player->Ai;

	if (GameCycle >= AiPlayer->LastCanNotMoveGameCycle + CYCLES_PER_SECOND)
	{
		int gx, gy, gw, gh;
		int minrange, maxrange;

		if (unit->Orders[0]->Goal)
		{
			gw = unit->Orders[0]->Goal->Type->TileWidth;
			gh = unit->Orders[0]->Goal->Type->TileHeight;
			gx = unit->Orders[0]->Goal->X;
			gy = unit->Orders[0]->Goal->Y;
			maxrange = unit->Orders[0]->Range;
			minrange = unit->Orders[0]->MinRange;
		}
		else
		{
			// Take care of non square goals :)
			// If goal is non square, range states a non-existant goal rather
			// than a tile.
			gw = unit->Orders[0]->Width;
			gh = unit->Orders[0]->Height;
			maxrange = unit->Orders[0]->Range;
			minrange = unit->Orders[0]->MinRange;
			gx = unit->Orders[0]->X;
			gy = unit->Orders[0]->Y;
		}

		if (unit->Type->UnitType == UnitTypeFly ||
			PlaceReachable(unit, gx, gy, gw, gh, minrange, maxrange))
		{
			// Path probably closed by unit here
			AiMoveUnitInTheWay(unit);
		}
	}

	AiPlayer->LastCanNotMoveGameCycle = GameCycle;
}

/**
**  Called when training of a unit is completed.
**
**  @param unit     Unit that trained the new unit.
**  @param newUnit  The new unit.
*/
void AiTrainingComplete(CUnit *unit, CUnit *newUnit)
{
	DebugPrint("%d: %d(%s) training %s at %d,%d completed\n" _C_
		unit->Player->Index _C_ UnitNumber(unit) _C_ unit->Type->Ident.c_str() _C_
		newUnit->Type->Ident.c_str() _C_ unit->X _C_ unit->Y);

	Assert(unit->Player->Type != PlayerPerson);

	AiRemoveFromBuilt(unit->Player->Ai, newUnit->Type);

	AiPlayer = unit->Player->Ai;
	AiCleanForces();
	AiAssignToForce(newUnit);
}

/**
**  This is called for each player, each game cycle.
**
**  @param player  The player structure pointer.
*/
void AiEachCycle(CPlayer *player)
{
	AiPlayer = player->Ai;
}

/**
**  This is called for each player each second.
**
**  @param player  The player structure pointer.
*/
void AiEachSecond(CPlayer *player)
{
	AiPlayer = player->Ai;
	Assert(AiPlayer != NULL);

	// Advance script
	AiExecuteFunction("EachSecond");

	// Look if everything is fine.
	AiCheckUnits();

	// Handle the resource manager.
	AiResourceManager();

	// Handle the force manager.
	AiForceManager();

	// Check for magic actions.
	AiCheckMagic();
}

//@}
