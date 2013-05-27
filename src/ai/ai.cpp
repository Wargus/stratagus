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
/**@name ai.cpp - The computer player AI main file. */
//
//      (c) Copyright 2000-2007 by Lutz Sammer, Ludovic Pollet, and
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
** ::InitAiModule()
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
** ::AiNeedMoreSupply()
**
** Called if an trained unit is ready, but not enough food is
** available for it.
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

#include "stratagus.h"

#include "ai.h"
#include "ai_local.h"

#include "actions.h"
#include "action/action_attack.h"
#include "commands.h"
#include "iolib.h"
#include "map.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"
#include "upgrade.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

int AiSleepCycles;              /// Ai sleeps # cycles

std::vector<CAiType *> AiTypes; /// List of all AI types.
AiHelper AiHelpers;             /// AI helper variables

PlayerAi *AiPlayer;             /// Current AI player

/*----------------------------------------------------------------------------
-- Lowlevel functions
----------------------------------------------------------------------------*/

/**
**  Execute the AI Script.
*/
static void AiExecuteScript()
{
	if (AiPlayer->Script.empty()) {
		return;
	}
	lua_getglobal(Lua, "_ai_scripts_");
	lua_pushstring(Lua, AiPlayer->Script.c_str());
	lua_rawget(Lua, -2);
	LuaCall(0, 1);
	lua_pop(Lua, 1);
}

/**
**  Check if everything is fine, send new requests to resource manager.
*/
static void AiCheckUnits()
{
	//  Count the already made build requests.
	int counter[UnitTypeMax];
	AiGetBuildRequestsCount(*AiPlayer, counter);

	//  Remove non active units.
	const int unitCount = AiPlayer->Player->GetUnitCount();
	for (int i = 0; i < unitCount; ++i) {
		const CUnit &unit = AiPlayer->Player->GetUnit(i);

		if (!unit.Active) {
			counter[unit.Type->Slot]--;
		}
	}
	const int *unit_types_count = AiPlayer->Player->UnitTypesCount;

	//  Look if some unit-types are missing.
	int n = AiPlayer->UnitTypeRequests.size();
	for (int i = 0; i < n; ++i) {
		const unsigned int t = AiPlayer->UnitTypeRequests[i].Type->Slot;
		const int x = AiPlayer->UnitTypeRequests[i].Count;

		// Add equivalent units
		int e = unit_types_count[t];
		if (t < AiHelpers.Equiv.size()) {
			for (unsigned int j = 0; j < AiHelpers.Equiv[t].size(); ++j) {
				e += unit_types_count[AiHelpers.Equiv[t][j]->Slot];
			}
		}
		const int requested = x - e - counter[t];
		if (requested > 0) {  // Request it.
			AiAddUnitTypeRequest(*AiPlayer->UnitTypeRequests[i].Type, requested);
			counter[t] += requested;
		}
		counter[t] -= x;
	}

	AiPlayer->Force.CheckUnits(counter);

	//  Look if some upgrade-to are missing.
	n = AiPlayer->UpgradeToRequests.size();
	for (int i = 0; i < n; ++i) {
		const unsigned int t = AiPlayer->UpgradeToRequests[i]->Slot;
		const int x = 1;

		//  Add equivalent units
		int e = unit_types_count[t];
		if (t < AiHelpers.Equiv.size()) {
			for (unsigned int j = 0; j < AiHelpers.Equiv[t].size(); ++j) {
				e += unit_types_count[AiHelpers.Equiv[t][j]->Slot];
			}
		}

		const int requested = x - e - counter[t];
		if (requested > 0) {  // Request it.
			AiAddUpgradeToRequest(*AiPlayer->UpgradeToRequests[i]);
			counter[t] += requested;
		}
		counter[t] -= x;
	}

	//  Look if some researches are missing.
	n = (int)AiPlayer->ResearchRequests.size();
	for (int i = 0; i < n; ++i) {
		if (UpgradeIdAllowed(*AiPlayer->Player, AiPlayer->ResearchRequests[i]->ID) == 'A') {
			AiAddResearchRequest(AiPlayer->ResearchRequests[i]);
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
static void SaveAiPlayer(CFile &file, int plynr, const PlayerAi &ai)
{
	file.printf("DefineAiPlayer(%d,\n", plynr);
	file.printf("  \"ai-type\", \"%s\",\n", ai.AiType->Name.c_str());

	file.printf("  \"script\", \"%s\",\n", ai.Script.c_str());
	file.printf("  \"script-debug\", %s,\n", ai.ScriptDebug ? "true" : "false");
	file.printf("  \"sleep-cycles\", %lu,\n", ai.SleepCycles);

	//  All forces
	for (size_t i = 0; i < ai.Force.Size(); ++i) {
		file.printf("  \"force\", {%d, %s%s%s", (int) i,
					ai.Force[i].Completed ? "\"complete\"," : "\"recruit\",",
					ai.Force[i].Attacking ? " \"attack\"," : "",
					ai.Force[i].Defending ? " \"defend\"," : "");

		file.printf(" \"role\", ");
		switch (ai.Force[i].Role) {
			case AiForceRoleAttack:
				file.printf("\"attack\",");
				break;
			case AiForceRoleDefend:
				file.printf("\"defend\",");
				break;
			default:
				file.printf("\"unknown-%d\",", ai.Force[i].Role);
				break;
		}

		file.printf("\n    \"types\", { ");
		const size_t unitTypesCounst = ai.Force[i].UnitTypes.size();
		for (size_t j = 0; j != unitTypesCounst; ++j) {
			const AiUnitType &aut = ai.Force[i].UnitTypes[j];
			file.printf("%d, \"%s\", ", aut.Want, aut.Type->Ident.c_str());
		}
		file.printf("},\n    \"units\", {");
		const size_t unitsCount = ai.Force[i].Units.size();
		for (size_t j = 0; j != unitsCount; ++j) {
			const CUnit &aiunit = *ai.Force[i].Units[j];
			file.printf(" %d, \"%s\",", UnitNumber(aiunit),
						aiunit.Type->Ident.c_str());
		}
		file.printf("},\n    \"state\", %d, \"goalx\", %d, \"goaly\", %d,",
					ai.Force[i].State, ai.Force[i].GoalPos.x, ai.Force[i].GoalPos.y);
		file.printf("},\n");
	}

	file.printf("  \"reserve\", {");
	for (int i = 0; i < MaxCosts; ++i) {
		file.printf("\"%s\", %d, ", DefaultResourceNames[i].c_str(), ai.Reserve[i]);
	}
	file.printf("},\n");

	file.printf("  \"used\", {");
	for (int i = 0; i < MaxCosts; ++i) {
		file.printf("\"%s\", %d, ", DefaultResourceNames[i].c_str(), ai.Used[i]);
	}
	file.printf("},\n");

	file.printf("  \"needed\", {");
	for (int i = 0; i < MaxCosts; ++i) {
		file.printf("\"%s\", %d, ", DefaultResourceNames[i].c_str(), ai.Needed[i]);
	}
	file.printf("},\n");

	file.printf("  \"collect\", {");
	for (int i = 0; i < MaxCosts; ++i) {
		file.printf("\"%s\", %d, ", DefaultResourceNames[i].c_str(), ai.Collect[i]);
	}
	file.printf("},\n");

	file.printf("  \"need-mask\", {");
	for (int i = 0; i < MaxCosts; ++i) {
		if (ai.NeededMask & (1 << i)) {
			file.printf("\"%s\", ", DefaultResourceNames[i].c_str());
		}
	}
	file.printf("},\n");
	if (ai.NeedSupply) {
		file.printf("  \"need-supply\",\n");
	}

	//  Requests
	if (!ai.FirstExplorationRequest.empty()) {
		file.printf("  \"exploration\", {");
		const size_t FirstExplorationRequestCount = ai.FirstExplorationRequest.size();
		for (size_t i = 0; i != FirstExplorationRequestCount; ++i) {
			const AiExplorationRequest &ptr = ai.FirstExplorationRequest[i];
			file.printf("{%d, %d, %d}, ", ptr.pos.x, ptr.pos.y, ptr.Mask);
		}
		file.printf("},\n");
	}
	file.printf("  \"last-exploration-cycle\", %lu,\n", ai.LastExplorationGameCycle);
	file.printf("  \"last-can-not-move-cycle\", %lu,\n", ai.LastCanNotMoveGameCycle);
	file.printf("  \"unit-type\", {");
	const size_t unitTypeRequestsCount = ai.UnitTypeRequests.size();
	for (size_t i = 0; i != unitTypeRequestsCount; ++i) {
		file.printf("\"%s\", ", ai.UnitTypeRequests[i].Type->Ident.c_str());
		file.printf("%d, ", ai.UnitTypeRequests[i].Count);
	}
	file.printf("},\n");

	file.printf("  \"upgrade\", {");
	const size_t upgradeToRequestsCount = ai.UpgradeToRequests.size();
	for (size_t i = 0; i != upgradeToRequestsCount; ++i) {
		file.printf("\"%s\", ", ai.UpgradeToRequests[i]->Ident.c_str());
	}
	file.printf("},\n");

	file.printf("  \"research\", {");
	const size_t researchRequestsCount = ai.ResearchRequests.size();
	for (size_t i = 0; i != researchRequestsCount; ++i) {
		file.printf("\"%s\", ", ai.ResearchRequests[i]->Ident.c_str());
	}
	file.printf("},\n");

	//
	//  Building queue
	//
	file.printf("  \"building\", {");
	const size_t UnitTypeBuiltCount = ai.UnitTypeBuilt.size();
	for (size_t i = 0; i != UnitTypeBuiltCount; ++i) {
		const AiBuildQueue &queue = ai.UnitTypeBuilt[i];
		/* rb - for backward compatibility of save format we have to put it first */
		if (queue.Pos.x != -1) {
			file.printf("\"onpos\", %d, %d, ", queue.Pos.x, queue.Pos.y);
		}
		/* */

		file.printf("\"%s\", %d, %d", queue.Type->Ident.c_str(), queue.Made, queue.Want);
		if (i < UnitTypeBuiltCount - 1) {
			file.printf(",\n");
		}
	}
	file.printf("},\n");

	file.printf("  \"repair-building\", %u\n", ai.LastRepairBuilding);

	file.printf(")\n\n");
}

/**
**  Save state of AI to file.
**
**  @param file  Output file.
*/
void SaveAi(CFile &file)
{
	file.printf("\n--- -----------------------------------------\n");

	for (int i = 0; i < PlayerMax; ++i) {
		if (Players[i].Ai) {
			SaveAiPlayer(file, i, *Players[i].Ai);
		}
	}

	DebugPrint("FIXME: Saving lua function definition isn't supported\n");
}

/**
**  Setup all at start.
**
**  @param player  The player structure pointer.
*/
void AiInit(CPlayer &player)
{
	PlayerAi *pai = new PlayerAi;

	if (!pai) {
		fprintf(stderr, "Out of memory.\n");
		exit(0);
	}

	pai->Player = &player;

	DebugPrint("%d - looking for class %s\n" _C_ player.Index _C_ player.AiName.c_str());
	//MAPTODO print the player name (player->Name) instead of the pointer

	//  Search correct AI type.
	if (AiTypes.empty()) {
		DebugPrint("AI: Got no scripts at all! You need at least one dummy fallback script.\n");
		DebugPrint("AI: Look at the DefineAi() documentation.\n");
		Exit(0);
	}
	size_t i;
	CAiType *ait = NULL;

	for (i = 0; i < AiTypes.size(); ++i) {
		ait = AiTypes[i];
		if (!ait->Race.empty() && ait->Race != PlayerRaces.Name[player.Race]) {
			continue;
		}
		if (!player.AiName.empty() && ait->Class != player.AiName) {
			continue;
		}
		break;
	}
	if (i == AiTypes.size()) {
		DebugPrint("AI: Found no matching ai scripts at all!\n");
		// FIXME: surely we can do something better than exit
		exit(0);
	}
	if (player.AiName.empty()) {
		DebugPrint("AI: not found!!!!!!!!!!\n");
		DebugPrint("AI: Using fallback:\n");
	}
	DebugPrint("AI: %s:%s with %s:%s\n" _C_ PlayerRaces.Name[player.Race].c_str() _C_
			   !ait->Race.empty() ? ait->Race.c_str() : "All" _C_ player.AiName.c_str() _C_ ait->Class.c_str());

	pai->AiType = ait;
	pai->Script = ait->Script;

	pai->Collect[GoldCost] = 50;
	pai->Collect[WoodCost] = 50;
	pai->Collect[OilCost] = 0;

	player.Ai = pai;
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
	for (int p = 0; p < PlayerMax; ++p) {
		delete Players[p].Ai;
		Players[p].Ai = NULL;
	}
}


/**
**  Free all AI resources.
*/
void FreeAi()
{
	CleanAi();

	//  Free AiTypes.
	for (unsigned int i = 0; i < AiTypes.size(); ++i) {
		CAiType *aitype = AiTypes[i];

		delete aitype;
	}
	AiTypes.clear();

	//  Free AiHelpers.
	AiHelpers.Train.clear();
	AiHelpers.Build.clear();
	AiHelpers.Upgrade.clear();
	AiHelpers.Research.clear();
	AiHelpers.Repair.clear();
	AiHelpers.UnitLimit.clear();
	AiHelpers.Equiv.clear();
	AiHelpers.Refinery.clear();
	AiHelpers.Depots.clear();

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
static int AiRemoveFromBuilt2(PlayerAi *pai, const CUnitType &type)
{
	std::vector<AiBuildQueue>::iterator i;

	for (i = pai->UnitTypeBuilt.begin(); i != pai->UnitTypeBuilt.end(); ++i) {
		Assert((*i).Want);
		if (&type == (*i).Type && (*i).Made) {
			--(*i).Made;
			if (!--(*i).Want) {
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
static void AiRemoveFromBuilt(PlayerAi *pai, const CUnitType &type)
{
	if (AiRemoveFromBuilt2(pai, type)) {
		return;
	}

	//  This could happen if an upgrade is ready, look for equivalent units.
	int equivalents[UnitTypeMax + 1];
	const int equivalentsCount = AiFindUnitTypeEquiv(type, equivalents);
	for (int i = 0; i < equivalentsCount; ++i) {
		if (AiRemoveFromBuilt2(pai, *UnitTypes[equivalents[i]])) {
			return;
		}
	}
	if (pai->Player == ThisPlayer) {
		DebugPrint("My guess is that you built something under ai me. naughty boy!\n");
		return;
	}
	fprintf(stderr, "Can't reduce %s from build list.\n", type.Ident.c_str());
}

/**
**  Reduce made unit-type from build list.
**
**  @param pai   Computer AI player.
**  @param type  Unit-type which is now available.
**  @return      True if the unit-type could be reduced.
*/
static bool AiReduceMadeInBuilt2(PlayerAi &pai, const CUnitType &type)
{
	std::vector<AiBuildQueue>::iterator i;

	for (i = pai.UnitTypeBuilt.begin(); i != pai.UnitTypeBuilt.end(); ++i) {
		if (&type == (*i).Type && (*i).Made) {
			(*i).Made--;
			return true;
		}
	}
	return false;
}

/**
**  Reduce made unit-type from build list.
**
**  @param pai   Computer AI player.
**  @param type  Unit-type which is now available.
*/
void AiReduceMadeInBuilt(PlayerAi &pai, const CUnitType &type)
{
	if (AiReduceMadeInBuilt2(pai, type)) {
		return;
	}
	//  This could happen if an upgrade is ready, look for equivalent units.
	int equivs[UnitTypeMax + 1];
	const unsigned int equivnb = AiFindUnitTypeEquiv(type, equivs);

	for (unsigned int i = 0; i < equivnb; ++i) {
		if (AiReduceMadeInBuilt2(pai, *UnitTypes[equivs[i]])) {
			return;
		}
	}
	if (pai.Player == ThisPlayer) {
		DebugPrint("My guess is that you built something under ai me. naughty boy!\n");
		return;
	}
	fprintf(stderr, "Can't reduce %s from build list.\n", type.Ident.c_str());
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
void AiHelpMe(const CUnit *attacker, CUnit &defender)
{
	/* Friendly Fire - typical splash */
	if (!attacker || attacker->Player->Index == defender.Player->Index) {
		//FIXME - try react somehow
		return;
	}

	DebugPrint("%d: %d(%s) attacked at %d,%d\n" _C_
			   defender.Player->Index _C_ UnitNumber(defender) _C_
			   defender.Type->Ident.c_str() _C_ defender.tilePos.x _C_ defender.tilePos.y);

	//  Don't send help to scouts (zeppelin,eye of vision).
	if (!defender.Type->CanAttack && defender.Type->UnitType == UnitTypeFly) {
		return;
	}
	// Summoned unit, don't help
	if (defender.Summoned) {
		return;
	}

	PlayerAi &pai = *defender.Player->Ai;
	AiPlayer = &pai;

	//  If unit belongs to an attacking force, check if force members can help.
	if (defender.GroupId) {
		AiForce &aiForce = pai.Force[defender.GroupId - 1];

		//  Unit belongs to an force, check if brothers in arms can help
		for (unsigned int i = 0; i < aiForce.Units.size(); ++i) {
			CUnit &aiunit = *aiForce.Units[i];

			if (&defender == &aiunit) {
				continue;
			}

			// if brother is idle or attack no-agressive target and
			// can attack our attacker then ask for help
			// FIXME ad support for help from Coward type units
			if (aiunit.IsAgressive() && CanTarget(*aiunit.Type, *attacker->Type)
				&& aiunit.CurrentOrder()->GetGoal() != attacker) {
				bool shouldAttack = aiunit.IsIdle() && aiunit.Threshold == 0;

				if (aiunit.CurrentAction() == UnitActionAttack) {
					const COrder_Attack &orderAttack = *static_cast<COrder_Attack *>(aiunit.CurrentOrder());
					const CUnit *oldGoal = orderAttack.GetGoal();

					if (oldGoal == NULL || (ThreatCalculate(defender, *attacker) < ThreatCalculate(defender, *oldGoal)
											&& aiunit.MapDistanceTo(defender) <= aiunit.Stats->Variables[ATTACKRANGE_INDEX].Max)) {
						shouldAttack = true;
					}
				}

				if (shouldAttack) {
					CommandAttack(aiunit, attacker->tilePos, const_cast<CUnit *>(attacker), FlushCommands);
					COrder *savedOrder = COrder::NewActionAttack(aiunit, attacker->tilePos);

					if (aiunit.CanStoreOrder(savedOrder) == false) {
						delete savedOrder;
						savedOrder = NULL;
					} else {
						aiunit.SavedOrder = savedOrder;
					}
				}
			}
		}
		if (!aiForce.Defending && aiForce.State > 0) {
			DebugPrint("%d: %d(%s) belong to attacking force, don't defend it\n" _C_
					   defender.Player->Index _C_ UnitNumber(defender) _C_ defender.Type->Ident.c_str());
			// unit belongs to an attacking force,
			// so don't send others force in such case.
			// FIXME: there may be other attacking the same place force who can help
			return;
		}
	}

	// Send defending forces, also send attacking forces if they are home/traning.
	// This is still basic model where we suspect only one base ;(
	const Vec2i &pos = attacker->tilePos;

	for (unsigned int i = 0; i < pai.Force.Size(); ++i) {
		AiForce &aiForce = pai.Force[i];

		if (aiForce.Size() > 0
			&& ((aiForce.Role == AiForceRoleDefend && !aiForce.Attacking)
				|| (aiForce.Role == AiForceRoleAttack && !aiForce.Attacking && !aiForce.State))) {  // none attacking
			aiForce.Defending = true;
			aiForce.Attack(pos);
		}
	}
}

/**
**  Called if a unit is killed.
**
**  @param unit  Pointer to unit.
*/
void AiUnitKilled(CUnit &unit)
{
	DebugPrint("%d: %d(%s) killed\n" _C_
			   unit.Player->Index _C_ UnitNumber(unit) _C_ unit.Type->Ident.c_str());

	Assert(unit.Player->Type != PlayerPerson);

	if (unit.GroupId) {
		AiForce &force = unit.Player->Ai->Force[unit.GroupId - 1];

		force.Remove(unit);
		if (force.Size() == 0) {
			force.Attacking = false;
			if (!force.Defending && force.State > 0) {
				DebugPrint("%d: Attack force #%lu was destroyed, giving up\n"
						   _C_ unit.Player->Index _C_(long unsigned int)(&force  - & (unit.Player->Ai->Force[0])));
				force.Reset(true);
			}
		}
	}

	unit.CurrentOrder()->AiUnitKilled(unit);
}

/**
**  Called if work complete (Buildings).
**
**  @param unit  Pointer to unit that builds the building.
**  @param what  Pointer to unit building that was built.
*/
void AiWorkComplete(CUnit *unit, CUnit &what)
{
	if (unit) {
		DebugPrint("%d: %d(%s) build %s at %d,%d completed\n" _C_
				   what.Player->Index _C_ UnitNumber(*unit) _C_ unit->Type->Ident.c_str() _C_
				   what.Type->Ident.c_str() _C_ unit->tilePos.x _C_ unit->tilePos.y);
	} else {
		DebugPrint("%d: building %s at %d,%d completed\n" _C_
				   what.Player->Index _C_ what.Type->Ident.c_str() _C_ what.tilePos.x _C_ what.tilePos.y);
	}

	Assert(what.Player->Type != PlayerPerson);
	AiRemoveFromBuilt(what.Player->Ai, *what.Type);
}

/**
**  Called if building can't be build.
**
**  @param unit  Pointer to unit what builds the building.
**  @param what  Pointer to unit-type.
*/
void AiCanNotBuild(const CUnit &unit, const CUnitType &what)
{
	DebugPrint("%d: %d(%s) Can't build %s at %d,%d\n" _C_
			   unit.Player->Index _C_ UnitNumber(unit) _C_ unit.Type->Ident.c_str() _C_
			   what.Ident.c_str() _C_ unit.tilePos.x _C_ unit.tilePos.y);

	Assert(unit.Player->Type != PlayerPerson);
	AiReduceMadeInBuilt(*unit.Player->Ai, what);
}

/**
**  Called if building place can't be reached.
**
**  @param unit  Pointer to unit what builds the building.
**  @param what  Pointer to unit-type.
*/
void AiCanNotReach(CUnit &unit, const CUnitType &what)
{
	Assert(unit.Player->Type != PlayerPerson);
	AiReduceMadeInBuilt(*unit.Player->Ai, what);
}

/**
**  Try to move a unit that's in the way
*/
static void AiMoveUnitInTheWay(CUnit &unit)
{
	static Vec2i dirs[8] = {Vec2i(-1, -1), Vec2i(-1, 0), Vec2i(-1, 1), Vec2i(0, 1), Vec2i(1, 1), Vec2i(1, 0), Vec2i(1, -1), Vec2i(0, -1)};
	CUnit *movableunits[16];
	Vec2i movablepos[16];
	int movablenb;

	AiPlayer = unit.Player->Ai;

	// No more than 1 move per cycle ( avoid stressing the pathfinder )
	if (GameCycle == AiPlayer->LastCanNotMoveGameCycle) {
		return;
	}

	const CUnitType &unittype = *unit.Type;
	const Vec2i u0 = unit.tilePos;
	const Vec2i u1(u0.x + unittype.TileWidth - 1, u0.y + unittype.TileHeight - 1);

	movablenb = 0;

	// Try to make some unit moves around it
	for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
		CUnit &blocker = **it;

		if (blocker.IsUnusable()) {
			continue;
		}
		if (!blocker.IsIdle()) {
			continue;
		}
		if (blocker.Player != unit.Player && blocker.Player->IsAllied(*unit.Player) == false) {
			continue;
		}
		const CUnitType &blockertype = *blocker.Type;

		if (blockertype.UnitType != unittype.UnitType) {
			continue;
		}
		if (!blocker.CanMove()) {
			continue;
		}

		const Vec2i b0 = blocker.tilePos;
		const Vec2i b1(b0.x + blockertype.TileWidth - 1, b0.y + blockertype.TileHeight - 1);

		// Check for collision
		if (!((u0.x == b1.x + 1 || u1.x == b0.x - 1)
			  && (std::max<int>(b0.y, u0.y) <= std::min<int>(b1.y, u1.y)))
			&& !((u0.y == b1.y + 1 || u1.y == b0.y - 1)
				 && (std::max<int>(b0.x, u0.x) <= std::min<int>(b1.x, u1.x)))) {
			continue;
		}

		if (&unit == &blocker) {
			continue;
		}

		// Move blocker in a rand dir
		int r = SyncRand() & 7;
		int trycount = 8;
		while (trycount > 0) {
			r = (r + 1) & 7;
			--trycount;

			const Vec2i pos = blocker.tilePos + dirs[r];

			// Out of the map => no !
			if (!Map.Info.IsPointOnMap(pos)) {
				continue;
			}
			// move to blocker ? => no !
			if (pos == u0) {
				continue;
			}

			movableunits[movablenb] = &blocker;
			movablepos[movablenb] = pos;

			++movablenb;
			trycount = 0;
		}
		if (movablenb >= 16) {
			break;
		}
	}

	// Don't move more than 1 unit.
	if (movablenb) {
		const int index = SyncRand() % movablenb;

		CommandMove(*movableunits[index], movablepos[index], FlushCommands);
		AiPlayer->LastCanNotMoveGameCycle = GameCycle;
	}
}

/**
**  Called if a unit can't move. Try to move unit in the way
**
**  @param unit  Pointer to unit what builds the building.
*/
void AiCanNotMove(CUnit &unit)
{
	const Vec2i &goalPos = unit.pathFinderData->input.GetGoalPos();
	const int minrange = unit.pathFinderData->input.GetMinRange();
	const int maxrange = unit.pathFinderData->input.GetMaxRange();
	const int gw = unit.pathFinderData->input.GetGoalSize().x;
	const int gh = unit.pathFinderData->input.GetGoalSize().y;

	AiPlayer = unit.Player->Ai;
	if (unit.Type->UnitType == UnitTypeFly
		|| PlaceReachable(unit, goalPos, gw, gh, minrange, maxrange)) {
		// Path probably closed by unit here
		AiMoveUnitInTheWay(unit);
	}
}

/**
**  Called if the AI needs more farms.
**
**  @param player  player which need supply.
*/
void AiNeedMoreSupply(const CPlayer &player)
{
	Assert(player.Type != PlayerPerson);
	player.Ai->NeedSupply = true;
}

/**
**  Called if training of a unit is completed.
**
**  @param unit  Pointer to unit making.
**  @param what  Pointer to new ready trained unit.
*/
void AiTrainingComplete(CUnit &unit, CUnit &what)
{
	DebugPrint("%d: %d(%s) training %s at %d,%d completed\n" _C_
			   unit.Player->Index _C_ UnitNumber(unit) _C_ unit.Type->Ident.c_str() _C_
			   what.Type->Ident.c_str() _C_ unit.tilePos.x _C_ unit.tilePos.y);

	Assert(unit.Player->Type != PlayerPerson);

	AiRemoveFromBuilt(unit.Player->Ai, *what.Type);

	unit.Player->Ai->Force.RemoveDeadUnit();
	unit.Player->Ai->Force.Assign(what);

}

/**
**  Called if upgrading of an unit is completed.
**
**  @param unit Pointer to unit working.
**  @param what Pointer to the new unit-type.
*/
void AiUpgradeToComplete(CUnit &unit, const CUnitType &what)
{
	DebugPrint("%d: %d(%s) upgrade-to %s at %d,%d completed\n" _C_
			   unit.Player->Index _C_ UnitNumber(unit) _C_ unit.Type->Ident.c_str() _C_
			   what.Ident.c_str() _C_ unit.tilePos.x _C_ unit.tilePos.y);

	Assert(unit.Player->Type != PlayerPerson);
}

/**
**  Called if reseaching of an unit is completed.
**
**  @param unit  Pointer to unit working.
**  @param what  Pointer to the new upgrade.
*/
void AiResearchComplete(CUnit &unit, const CUpgrade *what)
{
	DebugPrint("%d: %d(%s) research %s at %d,%d completed\n" _C_
			   unit.Player->Index _C_ UnitNumber(unit) _C_ unit.Type->Ident.c_str() _C_
			   what->Ident.c_str() _C_ unit.tilePos.x _C_ unit.tilePos.y);

	Assert(unit.Player->Type != PlayerPerson);

	// FIXME: upgrading knights -> paladins, must rebuild lists!
}

/**
**  This is called for each player, each game cycle.
**
**  @param player  The player structure pointer.
*/
void AiEachCycle(CPlayer &player)
{
	AiPlayer = player.Ai;
}

/**
**  This is called for each player each second.
**
**  @param player  The player structure pointer.
*/
void AiEachSecond(CPlayer &player)
{
	AiPlayer = player.Ai;
#ifdef DEBUG
	if (!AiPlayer) {
		return;
	}
#endif

	//  Advance script
	AiExecuteScript();

	//  Look if everything is fine.
	AiCheckUnits();

	//  Handle the resource manager.
	AiResourceManager();

	//  Handle the force manager.
	AiForceManager();

	//  Check for magic actions.
	AiCheckMagic();

	// At most 1 explorer each 5 seconds
	if (GameCycle > AiPlayer->LastExplorationGameCycle + 5 * CYCLES_PER_SECOND) {
		AiSendExplorers();
	}
}

//@}
