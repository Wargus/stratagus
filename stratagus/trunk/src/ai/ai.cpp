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
/**@name new_ai.c	-	The new computer player AI main file. */
//
//      (c) Copyright 2000-2003 by Lutz Sammer and Ludovic Pollet
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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

#define noTIMEIT			/// Enable CPU use debugging

//----------------------------------------------------------------------------
//      Documentation
//----------------------------------------------------------------------------

/**
**      @page AiModule Module - AI
**
**      @section aibasics What is it?
**
**	Stratagus uses a very simple scripted AI. There are no optimizations
**	yet. The complete AI was written on one weekend.
**	Until no AI specialist joins, I keep this AI.
**
**	@subsection aiscripted What is scripted AI?
**
**	The AI script tells the engine build 4 workers, than build 3 footman,
**	than attack the player, than sleep 100 frames.
**
**      @section API The AI API
**
**	@subsection aimanage Management calls
**
**	Manage the inititialse and cleanup of the AI players.
**
**	::InitAiModule(void)
**
**		Initialise all global varaibles and structures.
**		Called before AiInit, or before game loading.
**
**	::AiInit(::Player)
**
**		Called for each player, to setup the AI structures
**		Player::Aiin the player structure. It can use Player::AiNum to
**		select different AI's.
**
**	::CleanAi(void)
**
**		Called to release all the memory for all AI structures.
**		Must handle self which players contains AI structures.
**
**	::SaveAi(::FILE*)
**
**		Save the AI structures of all players to file.
**		Must handle self which players contains AI structures.
**
**
**	@subsection aipcall Periodic calls
**
**	This functions are called regular for all AI players.
**
**	::AiEachCycle(::Player)
**
**		Called each game cycle, to handle quick checks, which needs
**		less CPU.
**
**	::AiEachSecond(::Player)
**
**		Called each second, to handle more CPU intensive things.
**
**
**	@subsection aiecall Event call-backs
**
**		This functions are called, when some special events happens.
**
**	::AiHelpMe()
**
**		Called if an unit owned by the AI is attacked.
**
**	::AiUnitKilled()
**
**		Called if an unit owned by the AI is killed.
**
**	::AiNeedMoreFarms()
**
**		Called if an trained unit is ready, but not enough food is
**		available.
**
**	::AiWorkComplete()
**
**		Called if an unit has completed its work.
**
**	::AiCanNotBuild()
**
**		Called if the AI unit can't build the requested unit-type.
**
**	::AiCanNotReach()
**
**		Called if the AI unit can't reach the building place.
**
**	::AiTrainingComplete()
**
**		Called if AI unit has completed training a new unit.
**
**	::AiUpgradeToComplete()
**
**		Called if AI unit has completed upgrade to new unit-type.
**
**	::AiResearchComplete()
**
**		Called if AI unit has completed research of an upgrade or spell.
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"

#include "player.h"
#include "unit.h"
#include "ccl.h"
#include "ccl_helpers.h"
#include "actions.h"
#include "map.h"

#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif

#include "ai_local.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int AiSleepCycles;		/// Ai sleeps # cycles
global int AiTimeFactor = 100;		/// Adjust the AI build times
global int AiCostFactor = 100;		/// Adjust the AI costs

global AiType *AiTypes;			/// List of all AI types.
global AiHelper AiHelpers;		/// AI helper variables
global int AiScriptActionNum = 0;	/// number of action script ( FIXME : initialized only once )
global AiScriptAction AiScriptActions[MaxAiScriptActions];	/// definitions of action scripts

global PlayerAi *AiPlayer;		/// Current AI player
global AiRunningScript *AiScript;	/// Current AI script
/**
**	W*rCr*ft number to internal ai-type name.
*/
global char **AiTypeWcNames;

/*----------------------------------------------------------------------------
--	Lowlevel functions
----------------------------------------------------------------------------*/

local void debugForces(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    const AiActionEvaluation * aiaction;
    int force, i;
    int count[UnitTypeMax+1];
    int want[UnitTypeMax+1];
    char * str;

    DebugLevel2Fn(" AI MEMORY (%d)\n" _C_ AiPlayer->EvaluationCount);
    aiaction = AiPlayer->FirstEvaluation;
    while (aiaction) {
	str = gh_scm2newstr(gh_car(gh_car(aiaction->aiScriptAction->Action)),NULL);
	DebugLevel2(" %8d: (%3d,%3d) => points:%9d, needs: %9d ( %s )\n" _C_
	    aiaction->gamecycle _C_
	    aiaction->hotSpotX _C_
	    aiaction->hotSpotY _C_
	    aiaction->hotSpotValue _C_
	    aiaction->value _C_
	    str);
	free(str);
	aiaction = aiaction->Next;
    }
    DebugLevel2Fn(" AI FORCES      ! : completed    A/D : attacking/defending\n");
    for (force = 0; force < AI_MAX_FORCES; force++) {
	DebugLevel2("force %5d %c%c :" _C_
	    force _C_
	    (AiPlayer->Force[force].Role == AiForceRoleAttack ? 'A' : 'D') _C_
	    (AiPlayer->Force[force].Completed ? '!' : ' '));

	AiForceCountUnits(force, count);
	
	for (i = 0; i <= UnitTypeMax; i++) {
	    want[i] = 0;
	}
	AiForceSubstractWant(force, want);
	
	for (i = 0; i < UnitTypeMax; i++) {
	    if (count[i] || want[i]) {
		DebugLevel2(" %s(%d/%d)" _C_ UnitTypes[i]->Ident _C_ count[i] _C_ (-want[i]));
	    }
	}

	if (force > AI_GENERIC_FORCES || force == 0) {
	    if (!gh_null_p(AiPlayer->Scripts[force ? force - AI_GENERIC_FORCES : 0].Script)) {
	    	DebugLevel2(" => ");
		fflush(stdout);
	    	gh_display(gh_car(AiPlayer->Scripts[force ? force - AI_GENERIC_FORCES : 0].Script));
		CclFlushOutput();
	    }
	}
	DebugLevel2("\n");
    }
#elif defined(USE_LUA)
#endif
}

/**
**	Execute the AI Script.
*/
local void AiExecuteScripts(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    int i;
    PlayerAi *pai;
    SCM value;

    pai = AiPlayer;

    // Debugging
    if (pai->ScriptDebug) {
	debugForces();
    }

    for (i = 0; i < AI_MAX_RUNNING_SCRIPTS; i++) {
	AiScript = pai->Scripts + i;
	if (!gh_null_p(AiScript->Script)) {
	    /*DebugLevel3Fn("%d.%d (%12s) @ %3d.%3d :" _C_ pai->Player->Player _C_ i _C_ AiScript->ident _C_ AiScript->HotSpot_X _C_ AiScript->HotSpot_Y);
	    gh_display(AiScript->Script);
	    gh_newline();*/

	    value = gh_eval(gh_car(AiScript->Script), NIL);
	    if (!gh_eq_p(value, SCM_BOOL_T)) {
		CclGcProtectedAssign(&AiScript->Script, gh_cdr(AiScript->Script));
	    }

	    if ((gh_null_p(AiScript->Script)) && (AiScript->ownForce)) {
		AiEraseForce(AiScript->ownForce);
	    }
	}
    }
#elif defined(USE_LUA)
#endif
}

/**
**	Check if everything is fine, send new requests to resource manager.
*/
local void AiCheckUnits(void)
{
    int counter[UnitTypeMax];
    const AiBuildQueue *queue;
    const int *unit_types_count;
    int i;
    int j;
    int n;
    int t;
    int x;
    int e;

    memset(counter, 0, sizeof (counter));

    //
    //  Count the already made build requests.
    //
    for (queue = AiPlayer->UnitTypeBuilded; queue; queue = queue->Next) {
	counter[queue->Type->Type] += queue->Want;
	DebugLevel3Fn("Already in build queue: %s %d/%d\n" _C_
	    queue->Type->Ident _C_ queue->Made _C_ queue->Want);
    }

    //
    //  Remove non active units.
    //
    n = AiPlayer->Player->TotalNumUnits;
    for (i = 0; i < n; ++i) {
	if (!AiPlayer->Player->Units[i]->Active) {
	    counter[AiPlayer->Player->Units[i]->Type->Type]--;
	    DebugLevel3Fn("Removing non active unit: %s\n" _C_
		AiPlayer->Player->Units[i]->Type->Ident);
	}
    }
    unit_types_count = AiPlayer->Player->UnitTypesCount;

    //
    //  Look if some unit-types are missing.
    //
    n = AiPlayer->UnitTypeRequestsCount;
    for (i = 0; i < n; ++i) {
	t = AiPlayer->UnitTypeRequests[i].Table[0]->Type;
	x = AiPlayer->UnitTypeRequests[i].Count;

	//
	//      Add equivalent units
	//
	e = unit_types_count[t];
	if (t < AiHelpers.EquivCount && AiHelpers.Equiv[t]) {
	    DebugLevel3Fn("Equivalence for %s\n" _C_
		AiPlayer->UnitTypeRequests[i].Table[0]->Ident);
	    for (j = 0; j < AiHelpers.Equiv[t]->Count; ++j) {
		e += unit_types_count[AiHelpers.Equiv[t]->Table[j]->Type];
	    }
	}

	if (x > e + counter[t]) {	// Request it.
	    DebugLevel3Fn("Need %s *%d\n" _C_
		AiPlayer->UnitTypeRequests[i].Table[0]->Ident _C_ x);
	    AiAddUnitTypeRequest(AiPlayer->UnitTypeRequests[i].Table[0],
		x - e - counter[t]);
	    counter[t] += x - e - counter[t];
	}
	counter[t] -= x;
    }

    //
    // Magically complete all forces
    //
    for (i = 0; i < AI_MAX_FORCES; ++i) {
	if ((!AiPlayer->Force[i].Completed) &&
	    ((AiPlayer->Force[i].PopulateMode == AiForcePopulateFromAttack) ||
		(AiPlayer->Force[i].PopulateMode == AiForcePopulateAny))) {

	    // This force should be completed from other forces.
	    AiForceComplete(i);
	}
    }

    //
    // create missing units
    //
    for (i = 0; i < AI_MAX_FORCES; ++i) {
	const AiUnitType *aiut;

	// Create units only for AiForceCreateFromScratch forces
	if (AiPlayer->Force[i].PopulateMode != AiForcePopulateFromScratch) {
	    continue;
	}

	for (aiut = AiPlayer->Force[i].UnitTypes; aiut; aiut = aiut->Next) {
	    t = aiut->Type->Type;
	    x = aiut->Want;
	    if (x > unit_types_count[t] + counter[t]) {	// Request it.
		DebugLevel2Fn("Force %d need %s * %d\n" _C_ i _C_ aiut->Type->
		    Ident _C_ x);
		AiAddUnitTypeRequest(aiut->Type, x - unit_types_count[t] - counter[t]);
		counter[t] += x - unit_types_count[t] - counter[t];
		AiPlayer->Force[i].Completed = 0;
	    }
	    counter[t] -= x;
	}
    }

    //
    //  Look if some upgrade-to are missing.
    //
    n = AiPlayer->UpgradeToRequestsCount;
    for (i = 0; i < n; ++i) {
	t = AiPlayer->UpgradeToRequests[i]->Type;
	x = 1;

	//
	//      Add equivalent units
	//
	e = unit_types_count[t];
	if (t < AiHelpers.EquivCount && AiHelpers.Equiv[t]) {
	    DebugLevel3Fn("Equivalence for %s\n" _C_ AiPlayer->UpgradeToRequests[i]->
		Ident);
	    for (j = 0; j < AiHelpers.Equiv[t]->Count; ++j) {
		e += unit_types_count[AiHelpers.Equiv[t]->Table[j]->Type];
	    }
	}

	if (x > e + counter[t]) {	// Request it.
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
	DebugLevel3Fn("%s - %c\n" _C_
	    AiPlayer->ResearchRequests[i]->Ident _C_
	    UpgradeIdAllowed(AiPlayer->Player,
		AiPlayer->ResearchRequests[i] - Upgrades));
	if (UpgradeIdAllowed(AiPlayer->Player,
		AiPlayer->ResearchRequests[i] - Upgrades) == 'A') {
	    AiAddResearchRequest(AiPlayer->ResearchRequests[i]);
	}
    }
}

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Save the mapping of pud numbers of the AI to internal symbols.
**
**	@param file	Output file.
*/
local void SaveAiTypesWcName(CLFile * file)
{
    char **cp;
    int i;

    //
    //  Dump table wc2 race numbers -> internal symbol.
    //
    if ((cp = AiTypeWcNames)) {
	CLprintf(file, "(define-ai-wc-names");

	i = 90;
	while (*cp) {
	    if (i + strlen(*cp) > 79) {
		i = CLprintf(file, "\n ");
	    }
	    i += CLprintf(file, " '%s", *cp++);
	}
	CLprintf(file, ")\n\n");
    }
}

/**
**	Save AI helper sub table.
**
**	@param file	Output file.
**	@param name	Table action name.
**	@param upgrade	True if is an upgrade.
**	@param n	Number of elements in table
**	@param table	unit-type table.
*/
local void SaveAiHelperTable(CLFile * file, const char *name, int upgrade, int n,
    AiUnitTypeTable * const *table)
{
    int t;
    int i;
    int j;
    int f;

    for (t = 0; t < (upgrade ? UpgradeMax : NumUnitTypes); ++t) {
	// Look if that unit-type can build something
	for (f = i = 0; i < n; ++i) {
	    if (table[i]) {
		for (j = 0; j < table[i]->Count; ++j) {
		    if (table[i]->Table[j]->Type == t) {
			if (!f) {
			    CLprintf(file, "\n  (list '%s '%s\n    ", name,
				UnitTypes[t]->Ident);
			    f = 4;
			}
			if (upgrade) {
			    if (f + strlen(Upgrades[i].Ident) > 78) {
				f = CLprintf(file, "\n    ");
			    }
			    f += CLprintf(file, "'%s ", Upgrades[i].Ident);
			} else {
			    if (f + strlen(UnitTypes[i]->Ident) > 78) {
				f = CLprintf(file, "\n    ");
			    }
			    f += CLprintf(file, "'%s ", UnitTypes[i]->Ident);
			}
		    }
		}
	    }
	}
	if (f) {
	    CLprintf(file, ")");
	}
    }
}

/**
**	Save AI helper sub table.
**
**	@param file	Output file.
**	@param name	Table action name.
**	@param n	Number of elements in table
**	@param table	unit-type table.
*/
local void SaveAiEquivTable(CLFile * file, const char *name, int n,
    AiUnitTypeTable * const *table)
{
    int i;
    int j;
    int f;

    for (i = 0; i < n; ++i) {
	if (table[i]) {
	    CLprintf(file, "\n  (list '%s '%s\n    ", name, UnitTypes[i]->Ident);
	    f = 4;
	    for (j = 0; j < table[i]->Count; ++j) {
		if (f + strlen(table[i]->Table[j]->Ident) > 78) {
		    f = CLprintf(file, "\n    ");
		}
		f += CLprintf(file, "'%s ", table[i]->Table[j]->Ident);
	    }
	    CLprintf(file, ")");
	}
    }
}

/**
**	Save AI helper sub table.
**
**	@param file	Output file.
**	@param name	Table action name.
**	@param n	Number of elements in table
**	@param table	unit-type table.
*/
local void SaveAiCostTable(CLFile * file, const char *name, int n,
    AiUnitTypeTable * const *table)
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
		    if (table[i]->Table[j]->Type == t) {
			if (!f) {
			    CLprintf(file, "\n  (list '%s '%s\n    ", name,
				UnitTypes[t]->Ident);
			    f = 4;
			}
			if (f + strlen(DefaultResourceNames[i]) > 78) {
			    f = CLprintf(file, "\n    ");
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

/**
**	Save AI helper sub table.
**
**	@param file	Output file.
**	@param name	Table action name.
**	@param n	Number of elements in table
**	@param table	unit-type table.
*/
local void SaveAiUnitLimitTable(CLFile * file, const char *name, int n,
    AiUnitTypeTable * const *table)
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
		    if (table[i]->Table[j]->Type == t) {
			if (!f) {
			    CLprintf(file, "\n  (list '%s '%s\n    ", name,
				UnitTypes[t]->Ident);
			    f = 4;
			}
			if (f + strlen("food") > 78) {
			    f = CLprintf(file, "\n    ");
			}
			f += CLprintf(file, "'%s ", "food");
		    }
		}
	    }
	}
	if (f) {
	    CLprintf(file, ")");
	}
    }
}

/**
**	Save AI helper table.
**
**	@param file	Output file.
*/
local void SaveAiHelper(CLFile * file)
{
    CLprintf(file, "(define-ai-helper");
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

/**
**	Save all the AiScriptAction defined
**
**	@param file	Output file
*/
local void SaveAiScriptActions(CLFile * file)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    AiScriptAction *aiScriptAction;
    int i;

    // FIXME : should import the built-in lambda as well ( really needed ? )
    for (i = 0; i < AiScriptActionNum; i++) {
	aiScriptAction = AiScriptActions + i;

	CLprintf(file, "(define-ai-action '(%s%s)\n  '",
	    (aiScriptAction->Defensive ? " defense " : ""),
	    (aiScriptAction->Offensive ? " attack " : ""));

	lprin1CL(aiScriptAction->Action, file);
	CLprintf(file, "\n)\n");
    }
#elif defined(USE_LUA)
#endif
}

/**
**	Save the AI type. (recursive)
**
**	@param file	Output file.
**	@param aitype	AI type to save.
*/
local void SaveAiType(CLFile * file, const AiType * aitype)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    SCM list;

    if (aitype->Next) {
	SaveAiType(file, aitype->Next);
    }
    DebugLevel3Fn("%s,%s,%s\n" _C_ aitype->Name _C_ aitype->Race _C_ aitype->Class);
    CLprintf(file, "(define-ai \"%s\" '%s '%s\n",
	aitype->Name, aitype->Race ? aitype->Race : "*", aitype->Class);

    CLprintf(file, "  '(");
    //  Print the script a little formated
    list = aitype->Script;
    while (!gh_null_p(list)) {
	CLprintf(file, "\n    ");
	//lprin1CL(gh_car(list),file);
	list = gh_cdr(list);
    }
    CLprintf(file, " ))\n\n");
#elif defined(USE_LUA)
#endif
}

/**
**	Save the AI types.
**
**	@param file	Output file.
*/
local void SaveAiTypes(CLFile * file)
{
    SaveAiType(file, AiTypes);

    // FIXME: Must save references to other scripts - scheme functions
    // Perhaps we should dump the complete scheme state
}

/**
**	Save state of player AI.
**
**	@param file	Output file.
**	@param plynr	Player number.
**	@param ai	Player AI.
*/
local void SaveAiPlayer(CLFile * file, unsigned plynr, PlayerAi * ai)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    IOOutFile = file;
    IOLoadingMode = 0;
    IOTabLevel = 1;

    CLprintf(IOOutFile, "(define-ai-player '");
    IOPlayerAiFullPtr(SCM_UNSPECIFIED, &ai, 0);
    CLprintf(IOOutFile, ")\n");
#elif defined(USE_LUA)
#endif
}

/**
**	Save state of player AIs.
**
**	@param file	Output file.
*/
local void SaveAiPlayers(CLFile * file)
{
    unsigned p;

    for (p = 0; p < PlayerMax; ++p) {
	if (Players[p].Ai) {
	    SaveAiPlayer(file, p, Players[p].Ai);
	}
    }
}

/**
**	Save state of AI to file.
**
**	@param file	Output file.
*/
global void SaveAi(CLFile * file)
{
    CLprintf(file, "\n;;; -----------------------------------------\n");
    CLprintf(file,
	";;; MODULE: AI $Id$\n\n");

    SaveAiTypesWcName(file);
    SaveAiHelper(file);
    SaveAiTypes(file);
    SaveAiScriptActions(file);
    SaveAiPlayers(file);

    DebugLevel0Fn("FIXME: Saving AI isn't supported\n");
}

/**
**      Setup all at start.
**
**      @param player   The player structure pointer.
*/
global void AiInit(Player * player)
{
    int i;
    PlayerAi *pai;
    AiType *ait;
    char *ainame;

    DebugLevel0Fn("%d - %s -" _C_ player->Player _C_ player->Name);

    pai = calloc(1, sizeof (PlayerAi));
    if (!pai) {
	fprintf(stderr, "Out of memory.\n");
	exit(0);
    }
    pai->Player = player;
    ait = AiTypes;

    for (i = 0; i < AI_MAX_RUNNING_SCRIPTS; i++) {
	pai->Scripts[i].ownForce = AI_GENERIC_FORCES + i;
	pai->Scripts[i].HotSpot_X = -1;
	pai->Scripts[i].HotSpot_Y = -1;
	pai->Scripts[i].HotSpot_Ray = -1;
	pai->Scripts[i].gauges = 0;
	pai->Scripts[i].SleepCycles = 0;
#if defined(USE_GUILE) || defined(USE_SIOD)
	pai->Scripts[i].Script = NIL;
	CclGcProtect(&pai->Scripts[i].Script);
#elif defined(USE_LUA)
#endif
	snprintf(pai->Scripts[i].ident, 10, "Empty");
    }

    // Set autoattack to 1 as default
    pai->AutoAttack = 1;

    for (i = 0; i < AI_GENERIC_FORCES; i++) {
	// First force defend, others are attacking...
	pai->Force[i].Role = (i ? AiForceRoleAttack : AiForceRoleDefend);

	// Theses forces should be built from scratch
	pai->Force[i].PopulateMode = AiForcePopulateFromScratch;
	pai->Force[i].UnitsReusable = 1;
	pai->Force[i].HelpMode = AiForceHelpFull;
    }


    ainame = AiTypeWcNames[player->AiNum];
    DebugLevel0(" looking for class %s\n" _C_ ainame);

    //
    //  Search correct AI type.
    //
    if (!ait) {
	DebugLevel0Fn
	    ("AI: Got no scripts at all! You need at least one dummy fallback script.\n");
	DebugLevel0Fn("AI: Look at the (define-ai) documentation.\n");
	exit(0);
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
	DebugLevel0Fn("AI: Found no matching ai scripts at all!\n");
	exit(0);
    }
    if (!ainame) {
	DebugLevel0Fn("AI: not found!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	DebugLevel0Fn("AI: Using fallback:\n");
    }
    DebugLevel0Fn("AI: %s:%s with %s:%s\n" _C_ player->RaceName _C_ ait->Race
	_C_ ainame _C_ ait->Class);

    pai->AiType = ait;
#if defined(USE_GUILE) || defined(USE_SIOD)
    CclGcProtectedAssign(&pai->Scripts[0].Script, ait->Script);
#elif defined(USE_LUA)
#endif

    pai->Collect[TimeCost] = 0;
    pai->Collect[GoldCost] = 50;
    pai->Collect[WoodCost] = 50;
    pai->Collect[OilCost] = 0;
    pai->Collect[OreCost] = 0;
    pai->Collect[StoneCost] = 0;
    pai->Collect[CoalCost] = 0;

    player->Ai = pai;
}

/**
**	Initialise global structures of the AI
*/
global void InitAiModule(void)
{
    AiResetUnitTypeEquiv();
}

/**
**	Cleanup the AI.
*/
global void CleanAi(void)
{
    int i;
    int p;
    PlayerAi *pai;
    void *temp;
    AiType *aitype;
    AiBuildQueue *queue;
    AiExplorationRequest* request;
    char **cp;

    for (p = 0; p < PlayerMax; ++p) {
	if ((pai = Players[p].Ai)) {
	    //
	    //  Free forces
	    //
	    for (i = 0; i < AI_MAX_FORCES; ++i) {
		AiUnitType *aut;
		AiUnit *aiunit;

		for (aut = pai->Force[i].UnitTypes; aut; aut = temp) {
		    temp = aut->Next;
		    free(aut);
		}
		for (aiunit = pai->Force[i].Units; aiunit; aiunit = temp) {
		    temp = aiunit->Next;
		    free(aiunit);
		}
	    }

	    for (i = 0; i < AI_MAX_RUNNING_SCRIPTS; ++i) {
#if defined(USE_GUILE) || defined(USE_SIOD)
		CclGcUnprotect(&pai->Scripts[i].Script);
#elif defined(USE_LUA)
#endif
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
	    //  Free UnitTypeBuilded
	    //
	    for (queue = pai->UnitTypeBuilded; queue; queue = temp) {
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
	DebugLevel3Fn("%s,%s,%s\n" _C_ aitype->Name _C_ aitype->Race _C_ aitype->Class);
	free(aitype->Name);
	free(aitype->Race);
	free(aitype->Class);

	// ai-type->Script freed by ccl
#if defined(USE_GUILE) || defined(USE_SIOD)
	CclGcUnprotect(&aitype->Script);
#elif defined(USE_LUA)
#endif

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

    //
    //  Mapping original AI numbers in puds to our internal strings
    //
    if ((cp = AiTypeWcNames)) {		// Free all old names
	while (*cp) {
	    free(*cp++);
	}
	free(AiTypeWcNames);
	AiTypeWcNames = NULL;
    }

    // Free script action scm...
    for (i = 0; i < AiScriptActionNum; i++) {
#if defined(USE_GUILE) || defined(USE_SIOD)
	CclGcUnprotect(&AiScriptActions[i].Action);
#elif defined(USE_LUA)
#endif
    }

    AiResetUnitTypeEquiv();

    AiScriptActionNum = 0;
}

/*----------------------------------------------------------------------------
--	Support functions
----------------------------------------------------------------------------*/

/**
**	Remove unit-type from build list.
**
**	@param pai	Computer AI player.
**	@param type	Unit-type which is now available.
**	@return		True, if unit-type was found in list.
*/
local int AiRemoveFromBuilded2(PlayerAi * pai, const UnitType * type)
{
    AiBuildQueue **queue;
    AiBuildQueue *next;

    //
    //  Search the unit-type order.
    //
    for (queue = &pai->UnitTypeBuilded; (next = *queue); queue = &next->Next) {
	DebugCheck(!next->Want);
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
**	Remove unit-type from build list.
**
**	@param pai	Computer AI player.
**	@param type	Unit-type which is now available.
*/
local void AiRemoveFromBuilded(PlayerAi * pai, const UnitType * type)
{
    int i;
    int equivalents[UnitTypeMax+1];
    int equivalentsCount;

    if (AiRemoveFromBuilded2(pai, type)) {
	return;
    }

    //
    //  This could happen if an upgrade is ready, look for equivalent units.
    //
    equivalentsCount = AiFindUnitTypeEquiv(type, equivalents);
    for (i = 0; i < equivalentsCount; i++) {
	if (AiRemoveFromBuilded2(pai, UnitTypes[equivalents[i]])) {
	    return;
	}
    }

    if (pai->Player == ThisPlayer) {
	DebugLevel0Fn
	    ("My guess is that you built something under ai me. naughty boy!\n");
	return;
    }

    DebugCheck(1);
}

/**
**	Reduce made unit-type from build list.
**
**	@param pai	Computer AI player.
**	@param type	Unit-type which is now available.
**	@return		True if the unit-type could be reduced.
*/
local int AiReduceMadeInBuilded2(const PlayerAi * pai, const UnitType * type)
{
    AiBuildQueue *queue;

    //
    //  Search the unit-type order.
    //
    for (queue = pai->UnitTypeBuilded; queue; queue = queue->Next) {
	if (type == queue->Type && queue->Made) {
	    queue->Made--;
	    return 1;
	}
    }
    return 0;
}

/**
**	Reduce made unit-type from build list.
**
**	@param pai	Computer AI player.
**	@param type	Unit-type which is now available.
*/
local void AiReduceMadeInBuilded(const PlayerAi * pai, const UnitType * type)
{
    int i;
    int equivs[UnitTypeMax + 1];
    int equivnb;

    if (AiReduceMadeInBuilded2(pai, type)) {
	return;
    }
    //
    //  This could happen if an upgrade is ready, look for equivalent units.
    //
    equivnb = AiFindUnitTypeEquiv(type, equivs);

    for (i = 0; i < AiHelpers.Equiv[type->Type]->Count; ++i) {
	if (AiReduceMadeInBuilded2(pai, UnitTypes[equivs[i]])) {
	    return;
	}
    }

    DebugCheck(1);
}

/*----------------------------------------------------------------------------
--	Callback Functions
----------------------------------------------------------------------------*/

/**
**	Called if a Unit is Attacked
**
**	@param attacker	Pointer to attacker unit.
**	@param defender	Pointer to unit that is being attacked.
*/
global void AiHelpMe(const Unit * attacker, Unit * defender)
{
    PlayerAi *pai;
    AiUnit *aiunit;
    int force;

    DebugLevel0Fn("%d: %d(%s) attacked at %d,%d\n" _C_
	defender->Player->Player _C_ UnitNumber(defender) _C_
	defender->Type->Ident _C_ defender->X _C_ defender->Y);

    //
    //  Don't send help to scouts (zeppelin,eye of vision).
    //
    if (!defender->Type->CanAttack && defender->Type->UnitType == UnitTypeFly) {
	return;
    }

    AiPlayer = pai = defender->Player->Ai;

    //
    //  If unit belongs to an attack/defend force, don't defend it.
    //
    for (force = 1; force < AI_MAX_FORCES; ++force) {
	aiunit = pai->Force[force].Units;

	while (aiunit) {
	    if (defender == aiunit->Unit) {
		AiForceHelpMe(force, attacker, defender);
		return;
	    }
	    aiunit = aiunit->Next;
	}
    }

    // Unit can't be found in forces, consider it's in force 0
    AiForceHelpMe(0, attacker, defender);
}

/**
**	Called if an unit is killed.
**
**	@param unit	Pointer to unit.
*/
global void AiUnitKilled(Unit * unit)
{
    DebugLevel1Fn("%d: %d(%s) killed\n" _C_
	unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident);

    DebugCheck(unit->Player->Type == PlayerPerson);

    // FIXME: must handle all orders...
    switch (unit->Orders[0].Action) {
	case UnitActionStill:
	case UnitActionAttack:
	case UnitActionMove:
	    break;
	case UnitActionBuilded:
	    DebugLevel1Fn("%d: %d(%s) killed, under construction!\n" _C_
		unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident);
	    AiReduceMadeInBuilded(unit->Player->Ai, unit->Type);
	    break;
	case UnitActionBuild:
	    DebugLevel1Fn("%d: %d(%s) killed, with order %s!\n" _C_
		unit->Player->Player _C_ UnitNumber(unit) _C_
		unit->Type->Ident _C_ unit->Orders[0].Type->Ident);
	    AiReduceMadeInBuilded(unit->Player->Ai, unit->Orders[0].Type);
	    break;
	default:
	    DebugLevel1Fn("FIXME: %d: %d(%s) killed, with order %d!\n" _C_
		unit->Player->Player _C_ UnitNumber(unit) _C_
		unit->Type->Ident _C_ unit->Orders[0].Action);
	    break;
    }
}

/**
**	Called if work complete (Buildings).
**
**	@param unit	Pointer to unit that builds the building.
**	@param what	Pointer to unit building that was built.
*/
global void AiWorkComplete(Unit * unit, Unit * what)
{
    if (unit) {
	DebugLevel1Fn("%d: %d(%s) build %s at %d,%d completed\n" _C_
	    what->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	    what->Type->Ident _C_ unit->X _C_ unit->Y);
    } else {
	DebugLevel1Fn("%d: building %s at %d,%d completed\n" _C_
	    what->Player->Player _C_ what->Type->Ident _C_ what->X _C_ what->Y);
    }

    DebugCheck(what->Player->Type == PlayerPerson);

    AiRemoveFromBuilded(what->Player->Ai, what->Type);
}

/**
**	Called if building can't be build.
**
**	@param unit	Pointer to unit what builds the building.
**	@param what	Pointer to unit-type.
*/
global void AiCanNotBuild(Unit * unit, const UnitType * what)
{
    DebugLevel0Fn("%d: %d(%s) Can't build %s at %d,%d\n" _C_
	unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident
	_C_ what->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerPerson);

    AiReduceMadeInBuilded(unit->Player->Ai, what);
}

/**
**	Called if building place can't be reached.
**
**	@param unit	Pointer to unit what builds the building.
**	@param what	Pointer to unit-type.
*/
global void AiCanNotReach(Unit * unit, const UnitType * what)
{
    DebugLevel3Fn("%d: %d(%s) Can't reach %s at %d,%d\n" _C_
	unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	what->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerPerson);

    AiReduceMadeInBuilded(unit->Player->Ai, what);
}

/**
**	Called if an unit can't move. Try to move unit in the way
**
**	@param unit	Pointer to unit what builds the building.
**	@param what	Pointer to unit-type.
*/
global void AiCanNotMove(Unit * unit)
{
    static int dirs[8][2]={{-1,-1},{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1}};    
    int ux0,uy0,ux1,uy1;
    int bx0,by0,bx1,by1;
    int x,y;
    int trycount,i;
    Unit * blocker;
    UnitType * unittype;
    UnitType * blockertype;
    Unit * movableunits[16];
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

    DebugLevel2Fn("AiCanNotMove : %s at %d %d\n" _C_ unittype->Ident _C_ ux0 _C_ uy0);

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

	
	if (!blockertype->_Speed || blockertype->Building) {
	    continue;
	}

	bx0 = blocker->X;
	by0 = blocker->Y;
	bx1 = bx0;
	by1 = by0;

	// Check for collision
#define int_min(a,b)  ((a)<(b)?(a):(b))
#define int_max(a,b)  ((a)>(b)?(a):(b))
	if (!((ux0 == bx1 + 1 || ux1 == bx0 - 1) && 
	    	(int_max(by0, uy0) <= int_min(by1, uy1)))
	    && !((uy0 == by1 + 1 || uy1 == by0 - 1) && 
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
	    i = (i + 1) &7;
	    trycount--;
	    
	    x = blocker->X + dirs[i][0];
	    y = blocker->Y + dirs[i][1];

	    // Out of the map => no !
	    if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		continue;
	    }
	    // move to blocker ? => no !
	    if (x == ux0 && y == uy0) {
		continue;
	    }

	    movableunits[movablenb] = blocker;
	    movablepos[movablenb][0] = x;
	    movablepos[movablenb][1] = y;

	    movablenb++;
	    trycount = 0;
	}
	if (movablenb >= 16) {
	    break;
	}
    }

    // Don't move more than 1 unit.
    if (movablenb) {
	i = SyncRand() % movablenb;
    	CommandMove(movableunits[i], movablepos[i][0], movablepos[i][1], FlushCommands);
	AiPlayer->LastCanNotMoveGameCycle = GameCycle;
    }
}

/**
**	Called if the AI needs more farms.
**
**	@param unit	Point to unit.
**      @param what     Pointer to unit-type.
*/
global void AiNeedMoreFarms(Unit * unit, const UnitType * what __attribute__ ((unused)))
{
    DebugLevel3Fn("%d: %d(%s) need more farms %s at %d,%d\n" _C_
	unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	what->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerPerson);

    ((PlayerAi *) unit->Player->Ai)->NeedFood = 1;
}

/**
**	Called if training of an unit is completed.
**
**	@param unit	Pointer to unit making.
**	@param what	Pointer to new ready trained unit.
*/
global void AiTrainingComplete(Unit * unit, Unit * what)
{
    DebugLevel1Fn("%d: %d(%s) training %s at %d,%d completed\n" _C_
	unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	what->Type->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerPerson);

    AiRemoveFromBuilded(unit->Player->Ai, what->Type);

    AiPlayer = unit->Player->Ai;
    AiCleanForces();
    AiAssignToForce(what);
}

/**
**	Called if upgrading of an unit is completed.
**
**	@param unit	Pointer to unit working.
**	@param what	Pointer to the new unit-type.
*/
global void AiUpgradeToComplete(Unit * unit __attribute__ ((unused)),
    const UnitType * what __attribute__ ((unused)))
{
    DebugLevel1Fn("%d: %d(%s) upgrade-to %s at %d,%d completed\n" _C_
	unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	what->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerPerson);
}

/**
**	Called if reseaching of an unit is completed.
**
**	@param unit	Pointer to unit working.
**	@param what	Pointer to the new upgrade.
*/
global void AiResearchComplete(Unit * unit __attribute__ ((unused)),
    const Upgrade * what __attribute__ ((unused)))
{
    DebugLevel1Fn("%d: %d(%s) research %s at %d,%d completed\n" _C_
	unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	what->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerPerson);

    // FIXME: upgrading knights -> paladins, must rebuild lists!
}

/**
**	This is called for each player, each game cycle.
**
**	@param player	The player structure pointer.
*/
global void AiEachCycle(Player * player __attribute__ ((unused)))
{
}

/**
**	This called for each player, each second.
**
**	@param player	The player structure pointer.
*/
global void AiEachSecond(Player * player)
{
#ifdef TIMEIT
    u_int64_t sv = rdtsc();
    u_int64_t ev;
    static long mv;
    long sx;
#endif

    DebugLevel3Fn("%d:\n" _C_ player->Player);

    AiPlayer = player->Ai;
#ifdef DEBUG
    if (!AiPlayer) {
	return;
    }
#endif

    //
    //  Advance script
    //
    AiExecuteScripts();

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

    if (AiPlayer->AutoAttack) {
	AiPeriodicAttack();
    }

    // At most 1 explorer each 5 seconds
    if (GameCycle > AiPlayer->LastExplorationGameCycle + 5 * CYCLES_PER_SECOND) {
    	AiSendExplorers();
    }

#ifdef TIMEIT
    ev = rdtsc();
    sx = (ev - sv);
    mv = (mv + sx) / 2;
    DebugLevel1Fn("%ld %ld\n" _C_ sx / 1000 _C_ mv / 1000);
#endif
}

//@}
