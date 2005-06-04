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
/**@name ai.c - The computer player AI main file. */
//
//      (c) Copyright 2000-2005 by Lutz Sammer and Ludovic Pollet
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

//----------------------------------------------------------------------------
// Documentation
//----------------------------------------------------------------------------

/**
** @page AiModule Module - AI
**
** @section aibasics What is it?
**
** Stratagus uses a very simple scripted AI. There are no optimizations
** yet. The complete AI was written on one weekend.
** Until no AI specialist joins, I keep this AI.
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
** ::InitAiModule(void)
**
** Initialise all global varaibles and structures.
** Called before AiInit, or before game loading.
**
** ::AiInit(::Player)
**
** Called for each player, to setup the AI structures
** Player::Aiin the player structure. It can use Player::AiName to
** select different AI's.
**
** ::CleanAi(void)
**
** Called to release all the memory for all AI structures.
** Must handle self which players contains AI structures.
**
** ::SaveAi(::FILE*)
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
** Called if an unit owned by the AI is attacked.
**
** ::AiUnitKilled()
**
** Called if an unit owned by the AI is killed.
**
** ::AiNeedMoreSupply()
**
** Called if an trained unit is ready, but not enough food is
** available for it.
**
** ::AiWorkComplete()
**
** Called if an unit has completed its work.
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
**
** ::AiUpgradeToComplete()
**
** Called if AI unit has completed upgrade to new unit-type.
**
** ::AiResearchComplete()
**
** Called if AI unit has completed research of an upgrade or spell.
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

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

int AiSleepCycles; /// Ai sleeps # cycles

AiType* AiTypes; /// List of all AI types.
AiHelper AiHelpers; /// AI helper variables

PlayerAi* AiPlayer; /// Current AI player

/*----------------------------------------------------------------------------
-- Lowlevel functions
----------------------------------------------------------------------------*/

/**
**  Execute the AI Script.
*/
static void AiExecuteScript(void)
{
	PlayerAi* pai;

	pai = AiPlayer;
	if (pai->Script) {
		lua_pushstring(Lua, "_ai_scripts_");
		lua_gettable(Lua, LUA_GLOBALSINDEX);
		lua_pushstring(Lua, pai->Script);
		lua_rawget(Lua, 1);
		LuaCall(0, 1);
		lua_pop(Lua, 1);
	}
}

/**
**  Check if everything is fine, send new requests to resource manager.
*/
static void AiCheckUnits(void)
{
	int counter[UnitTypeMax];
	int attacking[UnitTypeMax];
	const AiBuildQueue* queue;
	const int* unit_types_count;
	int i;
	int j;
	int n;
	int t;
	int x;
	int e;

	memset(counter, 0, sizeof (counter));
	memset(attacking,0,sizeof(attacking));

	//
	//  Count the already made build requests.
	//
	for (queue = AiPlayer->UnitTypeBuilt; queue; queue = queue->Next) {
		counter[queue->Type->Slot] += queue->Want;
	}

	//
	//  Remove non active units.
	//
	n = AiPlayer->Player->TotalNumUnits;
	for (i = 0; i < n; ++i) {
		if (!AiPlayer->Player->Units[i]->Active) {
			counter[AiPlayer->Player->Units[i]->Type->Slot]--;
		}
	}
	unit_types_count = AiPlayer->Player->UnitTypesCount;

	//
	//  Look if some unit-types are missing.
	//
	n = AiPlayer->UnitTypeRequestsCount;
	for (i = 0; i < n; ++i) {
		t = AiPlayer->UnitTypeRequests[i].Table[0]->Slot;
		x = AiPlayer->UnitTypeRequests[i].Count;

		//
		// Add equivalent units
		//
		e = unit_types_count[t];
		if (t < AiHelpers.EquivCount && AiHelpers.Equiv[t]) {
			for (j = 0; j < AiHelpers.Equiv[t]->Count; ++j) {
				e += unit_types_count[AiHelpers.Equiv[t]->Table[j]->Slot];
			}
		}

		if (x > e + counter[t]) {  // Request it.
			AiAddUnitTypeRequest(AiPlayer->UnitTypeRequests[i].Table[0],
				x - e - counter[t]);
			counter[t] += x - e - counter[t];
		}
		counter[t] -= x;
	}

	//
	// Look through the forces what is missing.
	//
	for (i = AI_MAX_FORCES; i < AI_MAX_ATTACKING_FORCES; ++i) {
		const AiUnit* unit;

		for (unit = AiPlayer->Force[i].Units; unit; unit = unit->Next) {
			attacking[unit->Unit->Type->Slot]++;
		}
	}

	//
	// create missing units
	//
	for (i = 0; i < AI_MAX_FORCES; ++i) {
		const AiUnitType* aiut;

		// No troops for attacking force
		if (!AiPlayer->Force[i].Defending &&
				AiPlayer->Force[i].Attacking) {
			continue;
		}

		for (aiut = AiPlayer->Force[i].UnitTypes; aiut; aiut = aiut->Next) {
			t = aiut->Type->Slot;
			x = aiut->Want;
			if (x > unit_types_count[t] + counter[t] - attacking[t]) {    // Request it.
				AiAddUnitTypeRequest(aiut->Type,
					x - (unit_types_count[t] + counter[t] - attacking[t]));
				counter[t] += x - (unit_types_count[t] + counter[t] - attacking[t]);
				AiPlayer->Force[i].Completed=0;
			}
			counter[t] -= x;
		}
	}

	//
	//  Look if some upgrade-to are missing.
	//
	n = AiPlayer->UpgradeToRequestsCount;
	for (i = 0; i < n; ++i) {
		t = AiPlayer->UpgradeToRequests[i]->Slot;
		x = 1;

		//
		//  Add equivalent units
		//
		e = unit_types_count[t];
		if (t < AiHelpers.EquivCount && AiHelpers.Equiv[t]) {
			for (j = 0; j < AiHelpers.Equiv[t]->Count; ++j) {
				e += unit_types_count[AiHelpers.Equiv[t]->Table[j]->Slot];
			}
		}

		if (x > e + counter[t]) {  // Request it.
			AiAddUpgradeToRequest(AiPlayer->UpgradeToRequests[i]);
			counter[t] += x - e - counter[t];
		}
		counter[t] -= x;
	}

	//
	//  Look if some researches are missing.
	//
	n = AiPlayer->ResearchRequestsCount;
	for (i = 0; i < n; ++i) {
		if (UpgradeIdAllowed(AiPlayer->Player,
				AiPlayer->ResearchRequests[i] - Upgrades) == 'A') {
			AiAddResearchRequest(AiPlayer->ResearchRequests[i]);
		}
	}
}

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

#if 0
/**
**  Save AI helper sub table.
**
**  @param file     Output file.
**  @param name     Table action name.
**  @param upgrade  True if is an upgrade.
**  @param n        Number of elements in table
**  @param table    unit-type table.
*/
static void SaveAiHelperTable(CLFile* file, const char* name, int upgrade, int n,
	AiUnitTypeTable* const* table)
{
	int t;
	int i;
	int j;
	int f;
	int max;

	max = (upgrade ? UpgradeMax : NumUnitTypes);
	for (t = 0; t < max; ++t) {
		// Look if that unit-type can build something
		for (f = i = 0; i < n; ++i) {
			if (table[i]) {
				for (j = 0; j < table[i]->Count; ++j) {
					if (table[i]->Table[j]->Slot == t) {
						if (!f) {
							CLprintf(file, "\n  {\"%s\", \"%s\"\n\t", name,
								UnitTypes[t]->Ident);
							f = 4;
						}
						if (upgrade) {
							if (f + strlen(Upgrades[i].Ident) > 78) {
								f = CLprintf(file, "\n\t");
							}
							f += CLprintf(file, ", \"%s\"", Upgrades[i].Ident);
						} else {
							if (f + strlen(UnitTypes[i]->Ident) > 78) {
								f = CLprintf(file, "\n\t");
							}
							f += CLprintf(file, ", \"%s\"", UnitTypes[i]->Ident);
						}
					}
				}
			}
		}
		if (f) {
			CLprintf(file, "},");
		}
	}
}
#endif

#if 0
/**
**  Save AI helper sub table.
**
**  @param file   Output file.
**  @param name   Table action name.
**  @param n      Number of elements in table
**  @param table  unit-type table.
*/
static void SaveAiEquivTable(CLFile* file, const char* name, int n,
	AiUnitTypeTable* const* table)
{
	int i;
	int j;
	int f;

	for (i = 0; i < n; ++i) {
		if (table[i]) {
			CLprintf(file, "\n  {\"%s\", \"%s\"\n\t", name, UnitTypes[i]->Ident);
			f = 4;
			for (j = 0; j < table[i]->Count; ++j) {
				if (f + strlen(table[i]->Table[j]->Ident) > 78) {
					f = CLprintf(file, "\n\t");
				}
				f += CLprintf(file, ", \"%s\"", table[i]->Table[j]->Ident);
			}
			if (i == n - 1) {
				CLprintf(file, "}");
			} else {
				CLprintf(file, "},");
			}
		}
	}
}
#endif

#if 0 // Not used. Same as SaveAiUnitLimitTable with Defautresource instead of food

/**
**  Save AI helper sub table.
**
**  @param file   Output file.
**  @param name   Table action name.
**  @param n      Number of elements in table
**  @param table  unit-type table.
*/
static void SaveAiCostTable(CLFile* file, const char* name, int n,
	AiUnitTypeTable* const* table)
{
	int t;
	int i;
	int j;
	int f;

	for (t = 0; t < NumUnitTypes; ++t) {
		// Look if that unit-type can build something
		for (f = i = 0; i < n; ++i) {
			if (table[i]) {
				for (j = 0; j < table[i]->Count; ++j) {
					if (table[i]->Table[j]->Slot == t) {
						if (!f) {
							CLprintf(file, "\n  (list '%s '%s\n\t", name,
								UnitTypes[t]->Ident);
							f = 4;
						}
						if (f + strlen(DefaultResourceNames[i]) > 78) {
							f = CLprintf(file, "\n\t");
						}
						f += CLprintf(file, "'%s ", DefaultResourceNames[i]);
					}
				}
			}
		}
		if (f) {
			CLprintf(file, ")");
		}
	}
}

#endif

#if 0
/**
**  Save AI helper sub table.
**
**  @param file   Output file.
**  @param name   Table action name.
**  @param n      Number of elements in table
**  @param table  unit-type table.
*/
static void SaveAiUnitLimitTable(CLFile* file, const char* name, int n,
	AiUnitTypeTable* const* table)
{
	int t;
	int i;
	int j;
	int f;

	for (t = 0; t < NumUnitTypes; ++t) {
		// Look if that unit-type can build something
		for (f = i = 0; i < n; ++i) {
			if (table[i]) {
				for (j = 0; j < table[i]->Count; ++j) {
					if (table[i]->Table[j]->Slot == t) {
						if (!f) {
							CLprintf(file, "\n  {\"%s\", \"%s\"\n\t", name,
								UnitTypes[t]->Ident);
							f = 4;
						}
						if (f + strlen("food") > 78) {
							f = CLprintf(file, "\n\t");
						}
						f += CLprintf(file, ",\"%s\" ", "food");
					}
				}
			}
		}
		if (f) {
			CLprintf(file, "},");
		}
	}
}
#endif

#if 0
/**
**  Save AI helper table.
**
**  @param file  Output file.
**  @todo manage correctly ","
*/
static void SaveAiHelper(CLFile* file)
{
	CLprintf(file, "DefineAiHelper(");
	//
	//  Save build table
	//
	SaveAiHelperTable(file, "build", 0, AiHelpers.BuildCount, AiHelpers.Build);

	//
	//  Save train table
	//
	SaveAiHelperTable(file, "train", 0, AiHelpers.TrainCount, AiHelpers.Train);

	//
	//  Save upgrade table
	//
	SaveAiHelperTable(file, "upgrade", 0, AiHelpers.UpgradeCount, AiHelpers.Upgrade);

	//
	//  Save research table
	//
	SaveAiHelperTable(file, "research", 1, AiHelpers.ResearchCount, AiHelpers.Research);

	//
	//  Save repair table
	//
	SaveAiHelperTable(file, "repair", 0, AiHelpers.RepairCount, AiHelpers.Repair);

	//
	//  Save limits table
	//
	SaveAiUnitLimitTable(file, "unit-limit", AiHelpers.UnitLimitCount,
		AiHelpers.UnitLimit);

	//
	//  Save equivalence table
	//
	SaveAiEquivTable(file, "unit-equiv", AiHelpers.EquivCount, AiHelpers.Equiv);

	CLprintf(file, " )\n\n");
}
#endif

/**
**  Save the AI type. (recursive)
**
**  @param file    Output file.
**  @param aitype  AI type to save.
*/
static void SaveAiType(CLFile* file, const AiType* aitype)
{
	return;
	if (aitype->Next) {
		SaveAiType(file, aitype->Next);
	}
	CLprintf(file, "DefineAi(\"%s\", \"%s\", \"%s\", %s)\n\n",
		aitype->Name, aitype->Race ? aitype->Race : "*",
		aitype->Class, aitype->FunctionName);
}

#if 0
/**
**  Save the AI types.
**
**  @param file  Output file.
*/
static void SaveAiTypes(CLFile* file)
{
	SaveAiType(file, AiTypes);

	// FIXME: Must save references to other scripts - scheme functions
	// Perhaps we should dump the complete scheme state
}
#endif

/**
**  Save state of player AI.
**
**  @param file   Output file.
**  @param plynr  Player number.
**  @param ai     Player AI.
*/
static void SaveAiPlayer(CLFile* file, int plynr, PlayerAi* ai)
{
	int i;
	const AiBuildQueue* queue;

	CLprintf(file, "DefineAiPlayer(%d,\n", plynr);
	CLprintf(file, "  \"ai-type\", \"%s\",\n",ai->AiType->Name);

	CLprintf(file, "  \"script\", \"%s\",\n", ai->Script);
	CLprintf(file, "  \"script-debug\", %s,\n", ai->ScriptDebug ? "true" : "false");
	CLprintf(file, "  \"sleep-cycles\", %lu,\n", ai->SleepCycles);

	//
	//  All forces
	//
	for (i = 0; i < AI_MAX_ATTACKING_FORCES; ++i) {
		const AiUnitType* aut;
		const AiUnit* aiunit;

		CLprintf(file, "  \"force\", {%d, %s%s%s", i,
			ai->Force[i].Completed ? "\"complete\"," : "\"recruit\",",
			ai->Force[i].Attacking ? " \"attack\"," : "",
			ai->Force[i].Defending ? " \"defend\"," : "");

		CLprintf(file, " \"role\", ");
		switch (ai->Force[i].Role) {
			case AiForceRoleAttack:
				CLprintf(file, "\"attack\",");
				break;
			case AiForceRoleDefend:
				CLprintf(file, "\"defend\",");
				break;
			default:
				CLprintf(file, "\"unknown-%d\",", ai->Force[i].Role);
				break;
		}

		CLprintf(file, "\n    \"types\", { ");
		for (aut = ai->Force[i].UnitTypes; aut; aut = aut->Next) {
			CLprintf(file, "%d, \"%s\", ", aut->Want, aut->Type->Ident);
		}
		CLprintf(file, "},\n    \"units\", {");
		for (aiunit = ai->Force[i].Units; aiunit; aiunit = aiunit->Next) {
			CLprintf(file, " %d, \"%s\",", UnitNumber(aiunit->Unit),
				aiunit->Unit->Type->Ident);
		}
		CLprintf(file, "},\n    \"state\", %d, \"goalx\", %d, \"goaly\", %d, \"must-transport\", %d,",
			ai->Force[i].State, ai->Force[i].GoalX, ai->Force[i].GoalY, ai->Force[i].MustTransport);
		CLprintf(file, "},\n");
	}

	CLprintf(file, "  \"reserve\", {");
	for (i = 0; i < MaxCosts; ++i) {
		CLprintf(file, "\"%s\", %d, ", DefaultResourceNames[i], ai->Reserve[i]);
	}
	CLprintf(file, "},\n");

	CLprintf(file, "  \"used\", {");
	for (i = 0; i < MaxCosts; ++i) {
		CLprintf(file, "\"%s\", %d, ", DefaultResourceNames[i], ai->Used[i]);
	}
	CLprintf(file, "},\n");

	CLprintf(file, "  \"needed\", {");
	for (i = 0; i < MaxCosts; ++i) {
		CLprintf(file, "\"%s\", %d, ", DefaultResourceNames[i], ai->Needed[i]);
	}
	CLprintf(file, "},\n");

	CLprintf(file, "  \"collect\", {");
	for (i = 0; i < MaxCosts; ++i) {
		CLprintf(file, "\"%s\", %d, ", DefaultResourceNames[i], ai->Collect[i]);
	}
	CLprintf(file, "},\n");

	CLprintf(file,"  \"need-mask\", {");
	for (i = 0; i < MaxCosts; ++i) {
		if (ai->NeededMask & (1 << i)) {
			CLprintf(file, "\"%s\", ", DefaultResourceNames[i]);
		}
	}
	CLprintf(file, "},\n");
	if (ai->NeedSupply) {
		CLprintf(file, "  \"need-supply\",\n");
	}

	//
	//  Requests
	//
	if (ai->FirstExplorationRequest) {
		AiExplorationRequest* ptr;

		CLprintf(file, "  \"exploration\", {");
		ptr = ai->FirstExplorationRequest;
		while (ptr) {
			CLprintf(file, "{%d, %d, %d}, ",
				ptr->X, ptr->Y, ptr->Mask);
			ptr = ptr->Next;
		}
		CLprintf(file, "},\n");
	}
	CLprintf(file, "  \"last-exploration-cycle\", %lu,\n", ai->LastExplorationGameCycle);
	if (ai->TransportRequests) {
		AiTransportRequest* ptr;

		CLprintf(file, "  \"transport\", {");
		ptr = ai->TransportRequests;
		while (ptr) {
			CLprintf(file, "{%d, ", UnitNumber(ptr->Unit));
			SaveOrder(&ptr->Order, file);
			CLprintf(file, "}, ");
			ptr = ptr->Next;
		}
		CLprintf(file, "},\n");
	}
	CLprintf(file, "  \"last-can-not-move-cycle\", %lu,\n", ai->LastCanNotMoveGameCycle);
	CLprintf(file, "  \"unit-type\", {");
	for (i = 0; i < ai->UnitTypeRequestsCount; ++i) {
		CLprintf(file, "\"%s\", ", ai->UnitTypeRequests[i].Table[0]->Ident);
		CLprintf(file, "%d, ", ai->UnitTypeRequests[i].Count);
	}
	CLprintf(file, "},\n");

	CLprintf(file, "  \"upgrade\", {");
	for (i = 0; i < ai->UpgradeToRequestsCount; ++i) {
		CLprintf(file, "\"%s\", ", ai->UpgradeToRequests[i]->Ident);
	}
	CLprintf(file, "},\n");

	CLprintf(file, "  \"research\", {");
	for (i = 0; i < ai->ResearchRequestsCount; ++i) {
		CLprintf(file, "\"%s\", ", ai->ResearchRequests[i]->Ident);
	}
	CLprintf(file, "},\n");

	//
	//  Building queue
	//
	CLprintf(file, "  \"building\", {");
	for (queue = ai->UnitTypeBuilt; queue; queue = queue->Next) {
		CLprintf(file, "\"%s\", %d, %d, ", queue->Type->Ident, queue->Made, queue->Want);
	}
	CLprintf(file, "},\n");

	CLprintf(file, "  \"repair-building\", %u,\n", ai->LastRepairBuilding);

	CLprintf(file, "  \"repair-workers\", {");
	for (i = 0; i < UnitMax; ++i) {
		if (ai->TriedRepairWorkers[i]) {
			CLprintf(file, "%d, %d, ", i, ai->TriedRepairWorkers[i]);
		}
	}
	CLprintf(file,"})\n\n");
}

/**
**  Save state of player AIs.
**
**  @param file  Output file.
*/
static void SaveAiPlayers(CLFile* file)
{
	int p;

	for (p = 0; p < PlayerMax; ++p) {
		if (Players[p].Ai) {
			SaveAiPlayer(file, p, Players[p].Ai);
		}
	}
}

/**
**  Save state of AI to file.
**
**  @param file  Output file.
*/
void SaveAi(CLFile* file)
{
	CLprintf(file, "\n--- -----------------------------------------\n");
	CLprintf(file,
		"--- MODULE: AI $Id$\n\n");

#if 0
	SaveAiHelper(file);
	SaveAiTypes(file);
#endif
	SaveAiPlayers(file);

	DebugPrint("FIXME: Saving lua function definition isn't supported\n");
}

/**
**  Setup all at start.
**
**  @param player  The player structure pointer.
*/
void AiInit(Player* player)
{
	PlayerAi* pai;
	AiType* ait;
	char* ainame;

	pai = calloc(1, sizeof (PlayerAi));
	if (!pai) {
		fprintf(stderr, "Out of memory.\n");
		exit(0);
	}
	pai->Player = player;
	ait = AiTypes;

	ainame = player->AiName;
	DebugPrint("%d - %p - looking for class %s\n" _C_
		player->Player _C_ player _C_ ainame);
	//MAPTODO print the player name (player->Name) instead of the pointer

	//
	//  Search correct AI type.
	//
	if (!ait) {
		DebugPrint("AI: Got no scripts at all! You need at least one dummy fallback script.\n");
		DebugPrint("AI: Look at the DefineAi() documentation.\n");
		Exit(0);
	}
	for (;;) {
		if (ait->Race && strcmp(ait->Race, player->RaceName)) {
			ait = ait->Next;
			if (!ait && ainame) {
				ainame = NULL;
				ait = AiTypes;
			}
			if (!ait) {
				break;
			}
			continue;
		}
		if (ainame && strcmp(ainame, ait->Class)) {
			ait = ait->Next;
			if (!ait && ainame) {
				ainame = NULL;
				ait = AiTypes;
			}
			if (!ait) {
				break;
			}
			continue;
		}
		break;
	}
	if (!ait) {
		DebugPrint("AI: Found no matching ai scripts at all!\n");
		exit(0);
	}
	if (!ainame) {
		DebugPrint("AI: not found!!!!!!!!!!\n");
		DebugPrint("AI: Using fallback:\n");
	}
	DebugPrint("AI: %s:%s with %s:%s\n" _C_ player->RaceName _C_ 
		ait->Race ? ait->Race : "All" _C_ ainame _C_ ait->Class);

	pai->AiType = ait;
	pai->Script = ait->Script;

	pai->Collect[GoldCost] = 50;
	pai->Collect[WoodCost] = 50;
	pai->Collect[OilCost] = 0;

	player->Ai = pai;
}

/**
**  Initialise global structures of the AI
*/
void InitAiModule(void)
{
	AiResetUnitTypeEquiv();
}

/**
**  Cleanup the AI.
*/
void CleanAi(void)
{
	int i;
	int p;
	PlayerAi* pai;
	void* temp;
	AiType* aitype;
	AiBuildQueue* queue;
	AiExplorationRequest* request;

	for (p = 0; p < PlayerMax; ++p) {
		if ((pai = Players[p].Ai)) {
			//
			//  Free forces
			//
			for (i = 0; i < AI_MAX_ATTACKING_FORCES; ++i) {
				AiUnitType* aut;
				AiUnit* aiunit;

				for (aut = pai->Force[i].UnitTypes; aut; aut = temp) {
					temp = aut->Next;
					free(aut);
				}
				for (aiunit = pai->Force[i].Units; aiunit; aiunit = temp) {
					temp = aiunit->Next;
					free(aiunit);
				}
			}

			//
			//  Free UnitTypeRequests
			//
			free(pai->UnitTypeRequests);
			//
			//  Free UpgradeToRequests
			//
			free(pai->UpgradeToRequests);
			//
			//  Free ResearchRequests
			//
			free(pai->ResearchRequests);
			//
			//  Free UnitTypeBuilt
			//
			for (queue = pai->UnitTypeBuilt; queue; queue = temp) {
				temp = queue->Next;
				free(queue);
			}

			//
			// Free ExplorationRequest list
			//
			while (pai->FirstExplorationRequest) {
				request = pai->FirstExplorationRequest->Next;
				free(pai->FirstExplorationRequest);
				pai->FirstExplorationRequest = request;
			}

			free(pai);
			Players[p].Ai = NULL;
		}
	}

	//
	//  Free AiTypes.
	//
	for (aitype = AiTypes; aitype; aitype = temp) {
		free(aitype->Name);
		free(aitype->Race);
		free(aitype->Class);
		free(aitype->Script);
		free(aitype->FunctionName);

		temp = aitype->Next;
		free(aitype);
	}
	AiTypes = NULL;

	//
	//  Free AiHelpers.
	//
	for (i = 0; i < AiHelpers.TrainCount; ++i) {
		free(AiHelpers.Train[i]);
	}
	free(AiHelpers.Train);

	for (i = 0; i < AiHelpers.BuildCount; ++i) {
		free(AiHelpers.Build[i]);
	}
	free(AiHelpers.Build);

	for (i = 0; i < AiHelpers.UpgradeCount; ++i) {
		free(AiHelpers.Upgrade[i]);
	}
	free(AiHelpers.Upgrade);

	for (i = 0; i < AiHelpers.ResearchCount; ++i) {
		free(AiHelpers.Research[i]);
	}
	free(AiHelpers.Research);

	for (i = 0; i < AiHelpers.RepairCount; ++i) {
		free(AiHelpers.Repair[i]);
	}
	free(AiHelpers.Repair);

	for (i = 0; i < AiHelpers.UnitLimitCount; ++i) {
		free(AiHelpers.UnitLimit[i]);
	}
	free(AiHelpers.UnitLimit);

	for (i = 0; i < AiHelpers.EquivCount; ++i) {
		free(AiHelpers.Equiv[i]);
	}
	free(AiHelpers.Equiv);

	memset(&AiHelpers, 0, sizeof (AiHelpers));

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
static int AiRemoveFromBuilt2(PlayerAi* pai, const UnitType* type)
{
	AiBuildQueue** queue;
	AiBuildQueue* next;

	//
	//  Search the unit-type order.
	//
	for (queue = &pai->UnitTypeBuilt; (next = *queue); queue = &next->Next) {
		Assert(next->Want);
		if (type == next->Type && next->Made) {
			--next->Made;
			if (!--next->Want) {
				*queue = next->Next;
				free(next);
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
static void AiRemoveFromBuilt(PlayerAi* pai, const UnitType* type)
{
	int i;
	int equivalents[UnitTypeMax + 1];
	int equivalentsCount;

	if (AiRemoveFromBuilt2(pai, type)) {
		return;
	}

	//
	//  This could happen if an upgrade is ready, look for equivalent units.
	//
	equivalentsCount = AiFindUnitTypeEquiv(type, equivalents);
	for (i = 0; i < equivalentsCount; ++i) {
		if (AiRemoveFromBuilt2(pai, UnitTypes[equivalents[i]])) {
			return;
		}
	}

	if (pai->Player == ThisPlayer) {
		DebugPrint
			("My guess is that you built something under ai me. naughty boy!\n");
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
static int AiReduceMadeInBuilt2(const PlayerAi* pai, const UnitType* type)
{
	AiBuildQueue* queue;
	//
	//  Search the unit-type order.
	//
	for (queue = pai->UnitTypeBuilt; queue; queue = queue->Next) {
		if (type == queue->Type && queue->Made) {
			queue->Made--;
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
static void AiReduceMadeInBuilt(const PlayerAi* pai, const UnitType* type)
{
	int i;
	int equivs[UnitTypeMax + 1];
	int equivnb;

	if (AiReduceMadeInBuilt2(pai, type)) {
		return;
	}
	//
	//  This could happen if an upgrade is ready, look for equivalent units.
	//
	equivnb = AiFindUnitTypeEquiv(type, equivs);

	for (i = 0; i < AiHelpers.Equiv[type->Slot]->Count; ++i) {
		if (AiReduceMadeInBuilt2(pai, UnitTypes[equivs[i]])) {
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
void AiHelpMe(const Unit* attacker, Unit* defender)
{
	PlayerAi* pai;
	AiUnit* aiunit;
	int force;

	DebugPrint("%d: %d(%s) attacked at %d,%d\n" _C_
		defender->Player->Player _C_ UnitNumber(defender) _C_
		defender->Type->Ident _C_ defender->X _C_ defender->Y);

	//
	//  Don't send help to scouts (zeppelin,eye of vision).
	//
	if (!defender->Type->CanAttack && defender->Type->UnitType == UnitTypeFly) {
		return;
	}

	AiPlayer = pai = defender->Player->Ai;
	if (pai->Force[0].Attacking) {  // Force 0 busy
		return;
	}

	//
	//  If unit belongs to an attacking force, don't defend it.
	//
	for (force = 0; force < AI_MAX_ATTACKING_FORCES; ++force) {
		if (!pai->Force[force].Attacking) {  // none attacking
			// FIXME, send the force for help
			continue;
		}
		aiunit = pai->Force[force].Units;
		while (aiunit) {
			if (defender == aiunit->Unit) {
				return;
			}
			aiunit = aiunit->Next;
		}
	}

	//
	//  Send force 0 defending, also send force 1 if this is home.
	//
	if (attacker) {
		AiAttackWithForceAt(0, attacker->X, attacker->Y);
		if (!pai->Force[1].Attacking) {  // none attacking
			pai->Force[1].Defending = 1;
			AiAttackWithForceAt(1, attacker->X, attacker->Y);
		}
	} else {
		AiAttackWithForceAt(0, defender->X, defender->Y);
		if (!pai->Force[1].Attacking) {  // none attacking
			pai->Force[1].Defending = 1;
			AiAttackWithForceAt(1, defender->X, defender->Y);
		}
	}
	pai->Force[0].Defending = 1;
}

/**
**  Called if an unit is killed.
**
**  @param unit  Pointer to unit.
*/
void AiUnitKilled(Unit* unit)
{
	DebugPrint("%d: %d(%s) killed\n" _C_
		unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident);

	Assert(unit->Player->Type != PlayerPerson);

	// FIXME: must handle all orders...
	switch (unit->Orders[0].Action) {
		case UnitActionStill:
		case UnitActionAttack:
		case UnitActionMove:
			break;
		case UnitActionBuilt:
			DebugPrint("%d: %d(%s) killed, under construction!\n" _C_
				unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident);
			AiReduceMadeInBuilt(unit->Player->Ai, unit->Type);
			break;
		case UnitActionBuild:
			DebugPrint("%d: %d(%s) killed, with order %s!\n" _C_
				unit->Player->Player _C_ UnitNumber(unit) _C_
				unit->Type->Ident _C_ unit->Orders[0].Type->Ident);
			AiReduceMadeInBuilt(unit->Player->Ai, unit->Orders[0].Type);
			break;
		default:
			DebugPrint("FIXME: %d: %d(%s) killed, with order %d!\n" _C_
				unit->Player->Player _C_ UnitNumber(unit) _C_
				unit->Type->Ident _C_ unit->Orders[0].Action);
			break;
	}
}

/**
**  Called if work complete (Buildings).
**
**  @param unit  Pointer to unit that builds the building.
**  @param what  Pointer to unit building that was built.
*/
void AiWorkComplete(Unit* unit, Unit* what)
{
	if (unit) {
		DebugPrint("%d: %d(%s) build %s at %d,%d completed\n" _C_
			what->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
			what->Type->Ident _C_ unit->X _C_ unit->Y);
	} else {
		DebugPrint("%d: building %s at %d,%d completed\n" _C_
			what->Player->Player _C_ what->Type->Ident _C_ what->X _C_ what->Y);
	}

	Assert(what->Player->Type != PlayerPerson);

	AiRemoveFromBuilt(what->Player->Ai, what->Type);
}

/**
**  Called if building can't be build.
**
**  @param unit  Pointer to unit what builds the building.
**  @param what  Pointer to unit-type.
*/
void AiCanNotBuild(Unit* unit, const UnitType* what)
{
	DebugPrint("%d: %d(%s) Can't build %s at %d,%d\n" _C_
		unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
		what->Ident _C_ unit->X _C_ unit->Y);

	Assert(unit->Player->Type != PlayerPerson);

	AiReduceMadeInBuilt(unit->Player->Ai, what);
}

/**
**  Called if building place can't be reached.
**
**  @param unit  Pointer to unit what builds the building.
**  @param what  Pointer to unit-type.
*/
void AiCanNotReach(Unit* unit, const UnitType* what)
{
	Assert(unit->Player->Type != PlayerPerson);

	AiReduceMadeInBuilt(unit->Player->Ai, what);
}

/**
**  FIXME: docu
*/
static void AiMoveUnitInTheWay(Unit* unit)
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
	Unit* blocker;
	UnitType* unittype;
	UnitType* blockertype;
	Unit* movableunits[16];
	int movablepos[16][2];
	int movablenb;

	AiPlayer = unit->Player->Ai;

	// No more than 1 move per cycle ( avoid stressing the pathfinder )
	if (GameCycle == AiPlayer->LastCanNotMoveGameCycle) {
		return;
	}

	unittype = unit->Type;

	ux0 = unit->X;
	uy0 = unit->Y;
	ux1 = ux0 + unittype->TileWidth - 1;
	uy1 = uy0 + unittype->TileHeight - 1;

	movablenb = 0;


	// Try to make some unit moves around it
	for (i = 0; i < NumUnits; ++i) {
		blocker = Units[i];

		if (UnitUnusable(blocker)) {
			continue;
		}

		if (!UnitIdle(blocker)) {
			continue;
		}

		if (blocker->Player != unit->Player) {
			// Not allied
			if (!(blocker->Player->Allied & (1 << unit->Player->Player))) {
				continue;
			}
		}

		blockertype = blocker->Type;

		if (blockertype->UnitType != unittype->UnitType) {
			continue;
		}

		if (!CanMove(blocker)) {
			continue;
		}

		bx0 = blocker->X;
		by0 = blocker->Y;
		bx1 = bx0 + blocker->Type->TileWidth - 1;
		by1 = by0 + blocker->Type->TileHeight - 1;;

		// Check for collision
#define int_min(a,b)  ((a)<(b)?(a):(b))
#define int_max(a,b)  ((a)>(b)?(a):(b))
		if (!((ux0 == bx1 + 1 || ux1 == bx0 - 1) &&
					(int_max(by0, uy0) <= int_min(by1, uy1))) &&
				!((uy0 == by1 + 1 || uy1 == by0 - 1) &&
					(int_max(bx0, ux0) <= int_min(bx1, ux1)))) {
			continue;
		}
#undef int_min
#undef int_max

		if (unit == blocker) {
			continue;
		}

		// Move blocker in a rand dir
		i = SyncRand() & 7;
		trycount = 8;
		while (trycount > 0) {
			i = (i + 1) & 7;
			--trycount;

			x = blocker->X + dirs[i][0];
			y = blocker->Y + dirs[i][1];

			// Out of the map => no !
			if (x < 0 || y < 0 || x >= TheMap.Info.MapWidth || y >= TheMap.Info.MapHeight) {
				continue;
			}
			// move to blocker ? => no !
			if (x == ux0 && y == uy0) {
				continue;
			}

			movableunits[movablenb] = blocker;
			movablepos[movablenb][0] = x;
			movablepos[movablenb][1] = y;

			++movablenb;
			trycount = 0;
		}
		if (movablenb >= 16) {
			break;
		}
	}

	// Don't move more than 1 unit.
	if (movablenb) {
		i = SyncRand() % movablenb;
		CommandMove(movableunits[i], movablepos[i][0], movablepos[i][1],
			FlushCommands);
		AiPlayer->LastCanNotMoveGameCycle = GameCycle;
	}
}


#ifdef MAP_REGIONS
/**
** Return : 0 if nothing available
** 1 if exists (unit may still be null if no one ready)
*/
static int FindTransporterOnZone(int waterzone, ZoneSet* destzones,
	int x, int y, unsigned unitType, Unit** bestunit)
{
	static ZoneSet TransporterZones = {0};
	Unit** allunits;
	Unit* unit;
	int i;
	int unitdst;
	int unitok;
	int result;
	int bestunitdst;
	int unitX;
	int unitY;

	result = 0;
	*bestunit = 0;
	bestunitdst = -1;
	unitdst = -1;

	// Travel through all units
	allunits = AiPlayer->Player->Units;
	for (i = 0; i < AiPlayer->Player->TotalNumUnits; ++i) {
		unit = allunits[i];

		if (UnitUnusable(unit)) {
			continue;
		}
		if ((unsigned)unit->Type->UnitType != unitType || !unit->Type->CanTransport) {
			continue;
		}

		result = 1;

		if (unit->BoardCount >= unit->Type->MaxOnBoard) {
			continue;
		}

		// check that it is in the region
		ZoneSetClear(&TransporterZones);
		ZoneSetAddUnitZones(&TransporterZones, unit);
		if (!ZoneSetContains(&TransporterZones, waterzone)) {
			continue;
		}

		unitok = UnitIdle(unit);
		unitX = unit->X;
		unitY = unit->Y;

		// If transporter is moving, check if it is moving on our coast
		if (!unitok &&
				unit->OrderCount + (unit->OrderFlush ? 1 : 0) >= 2 &&
				unit->Orders[unit->OrderFlush ? 1 : 0].Action == UnitActionFollow &&
				unit->Orders[unit->OrderCount - 1].Action == UnitActionUnload &&
				unit->BoardCount + unit->OrderCount - (unit->OrderFlush ? 1 : 0) <= unit->Type->MaxOnBoard) {

			// Check that it will unload in the dest zone
			ZoneSetClear(&TransporterZones);
			ZoneSetAddCell(&TransporterZones,
				unit->Orders[unit->OrderCount - 1].X, unit->Orders[unit->OrderCount - 1].Y);

			unitok = ZoneSetHasIntersect(&TransporterZones, destzones);
			if (unitok) {
				if (unit->Orders[unit->OrderFlush ? 1 : 0].Goal) {
					unitX = unit->Orders[unit->OrderFlush ? 1 : 0].Goal->X;
					unitY = unit->Orders[unit->OrderFlush ? 1 : 0].Goal->Y;
				} else {
					unitX = unit->Orders[unit->OrderFlush ? 1 : 0].X;
					unitY = unit->Orders[unit->OrderFlush ? 1 : 0].Y;
				}
			}
		}

		if (!unitok) {
			continue;
		}

		unitdst = (unitX - x) * (unitX - x) + (unitY - y) * (unitY - y);
		if (bestunitdst != -1 && unitdst > bestunitdst) {
			continue;
		}

		bestunitdst = unitdst;
		*bestunit = unit;
	}

	return result;
}

/**
**  FIXME: docu
*/
static void HelpWithTransporter(Unit* unit, Unit* transporter,
	int transporterzone, int destzone)
{
	int x;
	int y;

	// Order temp;

	if (UnitIdle(transporter)) {
		CommandFollow(transporter, unit, FlushCommands);
		ZoneFindConnexion(destzone, transporterzone, unit->X, unit->Y, &x, &y);
		CommandUnload(transporter, x, y ,NoUnitP, 0);
	} else {
		CommandFollow(transporter, unit, 0);
		// We need to swap last with order 1
		CommandMoveOrder(transporter, transporter->OrderCount - 1, 1);
	}
	// FIXME: save order & restore it when unloaded
	CommandBoard(unit, transporter, FlushCommands);
}
#endif // MAP_REGIONS

/**
**  Called if an unit can't move. Try to move unit in the way
**
**  @param unit  Pointer to unit what builds the building.
*/
void AiCanNotMove(Unit* unit)
{
#ifdef MAP_REGIONS
	AiTransportRequest* aitr;
#endif
	int gx;
	int gy;
	int gw;
	int gh;
	int minrange;
	int maxrange;

	AiPlayer = unit->Player->Ai;

	if (unit->Orders[0].Goal) {
		gw = unit->Orders[0].Goal->Type->TileWidth;
		gh = unit->Orders[0].Goal->Type->TileHeight;
		gx = unit->Orders[0].Goal->X;
		gy = unit->Orders[0].Goal->Y;
		maxrange = unit->Orders[0].Range;
		minrange = unit->Orders[0].MinRange;
	} else {
		// Take care of non square goals :)
		// If goal is non square, range states a non-existant goal rather
		// than a tile.
		gw = unit->Orders[0].Width;
		gh = unit->Orders[0].Height;
		maxrange = unit->Orders[0].Range;
		minrange = unit->Orders[0].MinRange;
		gx = unit->Orders[0].X;
		gy = unit->Orders[0].Y;
	}

	if (PlaceReachable(unit, gx, gy, gw, gh, minrange, maxrange) ||
			unit->Type->UnitType == UnitTypeFly) {
		// Path probably closed by unit here
		AiMoveUnitInTheWay(unit);
		return;
	}

#ifdef MAP_REGIONS
	aitr = AiPlayer->TransportRequests;
	while (aitr) {
		if (aitr->Unit == unit) {
			return;
		}
		aitr = aitr->Next;
	}

	aitr = malloc(sizeof(AiTransportRequest));
	aitr->Next = AiPlayer->TransportRequests;
	aitr->Unit = unit;
	aitr->Order = unit->Orders[0];
	RefsIncrease(unit);
	if (aitr->Order.Goal) {
		RefsIncrease(aitr->Order.Goal);
	}
	AiPlayer->TransportRequests = aitr;
	return;
#endif // MAP_REGIONS
}

#ifdef MAP_REGIONS
/**
**  FIXME: docu
*/
static void HandleTransportRequests(AiTransportRequest* aitr)
{
	static ZoneSet UnitZones = {0};
	static ZoneSet DestZones = {0};

	Unit* transporter;
	int zonepath[MaxZoneNumber];
	int zonepathlen;
	int gx;
	int gy;
	int gw;
	int gh;
	int maxrange;
	int minrange;

	if (aitr->Unit->Removed) {
		return;
	}

	if (aitr->Order.Goal) {
		// Check for dead goal here (?)
		if (aitr->Order.Goal->Removed) {
			return;
		}

		gw = aitr->Order.Goal->Type->TileWidth;
		gh = aitr->Order.Goal->Type->TileHeight;
		gx = aitr->Order.Goal->X;
		gy = aitr->Order.Goal->Y;
		maxrange = aitr->Order.Range;
		minrange = aitr->Order.MinRange;
	} else {
		// Take care of non square goals :)
		// If goal is non square, range states a non-existant goal rather
		// than a tile.
		gw = aitr->Order.Width;
		gh = aitr->Order.Height;
		maxrange = aitr->Order.Range;
		minrange = aitr->Order.MinRange;
		gx = aitr->Order.X;
		gy = aitr->Order.Y;
	}

	// Check if we have an idle air transporter.

	// Check if we have an idle water tranporter
	ZoneSetClear(&UnitZones);
	ZoneSetAddUnitZones(&UnitZones, aitr->Unit);

	ZoneSetClear(&DestZones);
	ZoneSetAddGoalZones(&DestZones, aitr->Unit, gx, gy, gw, gh, minrange, maxrange);

	if (ZoneSetHasIntersect(&UnitZones, &DestZones)) {
		// Can go, nothing to do.
		return;
	}

	if (!ZoneSetFindPath(&UnitZones, &DestZones, zonepath, &zonepathlen)) {
		return;
	}

	Assert(zonepathlen >= 3);

	if (FindTransporterOnZone(zonepath[1], &DestZones,
			aitr->Unit->X, aitr->Unit->Y, UnitTypeNaval, &transporter)) {
		if (transporter) {
			HelpWithTransporter(aitr->Unit, transporter, zonepath[1], zonepath[2]);
			CommandAnyOrder(aitr->Unit, &aitr->Order, 0);
		} else {
			DebugPrint("All transporters are busy, waits.\n");
		}
	} else {
		// FIXME : Find or build transporter builder in the zone
	}
}
#endif // MAP_REGIONS

/**
**  Called if the AI needs more farms.
**
**  @param unit  Point to unit.
**  @param what  Pointer to unit-type.
*/
void AiNeedMoreSupply(Unit* unit, const UnitType* what __attribute__((unused)))
{
	Assert(unit->Player->Type != PlayerPerson);

	((PlayerAi*)unit->Player->Ai)->NeedSupply = 1;
}

/**
**  Called if training of an unit is completed.
**
**  @param unit  Pointer to unit making.
**  @param what  Pointer to new ready trained unit.
*/
void AiTrainingComplete(Unit* unit, Unit* what)
{
	DebugPrint("%d: %d(%s) training %s at %d,%d completed\n" _C_
		unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
		what->Type->Ident _C_ unit->X _C_ unit->Y);

	Assert(unit->Player->Type != PlayerPerson);

	AiRemoveFromBuilt(unit->Player->Ai, what->Type);

	AiPlayer = unit->Player->Ai;
	AiCleanForces();
	AiAssignToForce(what);
}

/**
**  Called if upgrading of an unit is completed.
**
**  @param unit Pointer to unit working.
**  @param what Pointer to the new unit-type.
*/
void AiUpgradeToComplete(Unit* unit __attribute__((unused)),
	const UnitType* what __attribute__((unused)))
{
	DebugPrint("%d: %d(%s) upgrade-to %s at %d,%d completed\n" _C_
		unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
		what->Ident _C_ unit->X _C_ unit->Y);

	Assert(unit->Player->Type != PlayerPerson);
}

/**
**  Called if reseaching of an unit is completed.
**
**  @param unit  Pointer to unit working.
**  @param what  Pointer to the new upgrade.
*/
void AiResearchComplete(Unit* unit __attribute__((unused)),
	const Upgrade* what __attribute__((unused)))
{
	DebugPrint("%d: %d(%s) research %s at %d,%d completed\n" _C_
		unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
		what->Ident _C_ unit->X _C_ unit->Y);

	Assert(unit->Player->Type != PlayerPerson);

	// FIXME: upgrading knights -> paladins, must rebuild lists!
}

/**
**  This is called for each player, each game cycle.
**
**  @param player  The player structure pointer.
*/
void AiEachCycle(Player* player __attribute__((unused)))
{
	AiTransportRequest* aitr;
	AiTransportRequest* next;

	AiPlayer = player->Ai;

	aitr = AiPlayer->TransportRequests;
	while (aitr) {
		next = aitr->Next;

#ifdef MAP_REGIONS
		HandleTransportRequests(aitr);
#endif // MAP_REGIONS

		RefsDecrease(aitr->Unit);
		if (aitr->Order.Goal) {
			RefsDecrease(aitr->Order.Goal);
		}
		free(aitr);

		aitr = next;
	}
	AiPlayer->TransportRequests = 0;
}

/**
**  This called for each player, each second.
**
**  @param player  The player structure pointer.
*/
void AiEachSecond(Player* player)
{
	AiPlayer = player->Ai;
#ifdef DEBUG
	if (!AiPlayer) {
		return;
	}
#endif

	//
	//  Advance script
	//
	AiExecuteScript();

	//
	//  Look if everything is fine.
	//
	AiCheckUnits();
	//
	//  Handle the resource manager.
	//
	AiResourceManager();
	//
	//  Handle the force manager.
	//
	AiForceManager();
	//
	//  Check for magic actions.
	//
	AiCheckMagic();

	// At most 1 explorer each 5 seconds
	if (GameCycle > AiPlayer->LastExplorationGameCycle + 5 * CYCLES_PER_SECOND) {
		AiSendExplorers();
	}
}

//@}
