//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name new_ai.c	-	The new computer player AI main file. */
//
//      (c) Copyright 2000-2002 by Lutz Sammer
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

#ifdef NEW_AI	// {

//@{

#define noTIMEIT			/// Enable CPU use debugging

//----------------------------------------------------------------------------
//	Documentation
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

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"

#include "player.h"
#include "unit.h"
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

global AiType* AiTypes;			/// List of all AI types.
global AiHelper AiHelpers;		/// AI helper variables

global PlayerAi* AiPlayer;		/// Current AI player

/**
**	W*rCr*ft number to internal ai-type name.
*/
global char** AiTypeWcNames;

/*----------------------------------------------------------------------------
--	Lowlevel functions
----------------------------------------------------------------------------*/

/**
**	Execute the AI Script.
*/
local void AiExecuteScript(void)
{
    PlayerAi* pai;
    SCM value;

    pai=AiPlayer;
    if( !gh_null_p(pai->Script) ) {
	if( pai->ScriptDebug ) {		// display executed command
	    printf("%d:",pai->Player->Player);
	    gh_display(gh_car(pai->Script));
	    gh_newline();
	}
	value=gh_eval(gh_car(pai->Script),NIL);
	if( !gh_eq_p(value,SCM_BOOL_T) ) {
	    pai->Script=gh_cdr(pai->Script);
	}
    }
}

/**
**	Check if everything is fine, send new requests to resource manager.
*/
local void AiCheckUnits(void)
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

    memset(counter,0,sizeof(counter));
    memset(attacking,0,sizeof(attacking));
    //
    //	Count the already made build requests.
    //
    for( queue=AiPlayer->UnitTypeBuilded; queue; queue=queue->Next ) {
	counter[queue->Type->Type]+=queue->Want;
	DebugLevel3Fn("Already in build queue: %s %d/%d\n" _C_
		queue->Type->Ident _C_ queue->Made _C_ queue->Want);
    }

    //
    //	Remove non active units.
    //
    n=AiPlayer->Player->TotalNumUnits;
    for( i=0; i<n; ++i ) {
	if( !AiPlayer->Player->Units[i]->Active ) {
	    counter[AiPlayer->Player->Units[i]->Type->Type]--;
	    DebugLevel3Fn("Removing non active unit: %s\n" _C_
		    AiPlayer->Player->Units[i]->Type->Ident);
	}
    }
    unit_types_count=AiPlayer->Player->UnitTypesCount;

    //
    //	Look if some unit-types are missing.
    //
    n=AiPlayer->UnitTypeRequestsCount;
    for( i=0; i<n; ++i ) {
	t=AiPlayer->UnitTypeRequests[i].Table[0]->Type;
	x=AiPlayer->UnitTypeRequests[i].Count;

	//
	//	Add equivalent units
	//
	e=unit_types_count[t];
	if( t<AiHelpers.EquivCount && AiHelpers.Equiv[t] ) {
	    DebugLevel3Fn("Equivalence for %s\n" _C_
		    AiPlayer->UnitTypeRequests[i].Table[0]->Ident);
	    for( j=0; j<AiHelpers.Equiv[t]->Count; ++j ) {
		e+=unit_types_count[AiHelpers.Equiv[t]->Table[j]->Type];
	    }
	}

	if( x>e+counter[t] ) {	// Request it.
	    DebugLevel3Fn("Need %s *%d\n" _C_
		    AiPlayer->UnitTypeRequests[i].Table[0]->Ident _C_ x);
	    AiAddUnitTypeRequest(AiPlayer->UnitTypeRequests[i].Table[0],
		    x-e-counter[t]);
	    counter[t]+=x-e-counter[t];
	}
	counter[t]-=x;
    }

    //
    //	Look through the forces what is missing.
    //
    for( i=AI_MAX_FORCES; i<AI_MAX_ATTACKING_FORCES; ++i ) {
	const AiUnit* unit;

	for( unit=AiPlayer->Force[i].Units; unit; unit=unit->Next ) {
	    attacking[unit->Unit->Type->Type]++;
	}
    }
    for( i=0; i<AI_MAX_FORCES; ++i ) {
	const AiUnitType* aiut;

	// No troops for attacking force
	if( !AiPlayer->Force[i].Defending
		&& AiPlayer->Force[i].Attacking ) {
	    continue;
	}

	for( aiut=AiPlayer->Force[i].UnitTypes; aiut; aiut=aiut->Next ) {
	    t=aiut->Type->Type;
	    x=aiut->Want;
	    if( x>unit_types_count[t]+counter[t]-attacking[t] ) {	// Request it.
		DebugLevel2Fn("Force %d need %s * %d\n" _C_ i _C_
			aiut->Type->Ident _C_ x);
		AiAddUnitTypeRequest(aiut->Type,
			x-(unit_types_count[t]+counter[t]-attacking[t]));
		counter[t]+=x-(unit_types_count[t]+counter[t]-attacking[t]);
		AiPlayer->Force[i].Completed=0;
	    }
	    counter[t]-=x;
	}
    }

    //
    //	Look if some upgrade-to are missing.
    //
    n=AiPlayer->UpgradeToRequestsCount;
    for( i=0; i<n; ++i ) {
	t=AiPlayer->UpgradeToRequests[i]->Type;
	x=1;

	//
	//	Add equivalent units
	//
	e=unit_types_count[t];
	if( t<AiHelpers.EquivCount && AiHelpers.Equiv[t] ) {
	    DebugLevel3Fn("Equivalence for %s\n" _C_
		    AiPlayer->UpgradeToRequests[i]->Ident);
	    for( j=0; j<AiHelpers.Equiv[t]->Count; ++j ) {
		e+=unit_types_count[AiHelpers.Equiv[t]->Table[j]->Type];
	    }
	}

	if( x>e+counter[t] ) {	// Request it.
	    AiAddUpgradeToRequest(AiPlayer->UpgradeToRequests[i]);
	    counter[t]+=x-e-counter[t];
	}
	counter[t]-=x;
    }

    //
    //	Look if some researches are missing.
    //
    n=AiPlayer->ResearchRequestsCount;
    for( i=0; i<n; ++i ) {
	DebugLevel3Fn("%s - %c\n" _C_
		AiPlayer->ResearchRequests[i]->Ident _C_
		UpgradeIdAllowed(AiPlayer->Player,
		    AiPlayer->ResearchRequests[i]-Upgrades));
	if( UpgradeIdAllowed(AiPlayer->Player,
		    AiPlayer->ResearchRequests[i]-Upgrades)=='A' ) {
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
local void SaveAiTypesWcName(FILE* file)
{
    char** cp;
    int i;

    //
    //	Dump table wc2 race numbers -> internal symbol.
    //
    if( (cp=AiTypeWcNames) ) {
	fprintf(file,"(define-ai-wc-names");

	i=90;
	while( *cp ) {
	    if( i+strlen(*cp)>79 ) {
		i=fprintf(file,"\n ");
	    }
	    i+=fprintf(file," '%s",*cp++);
	}
	fprintf(file,")\n\n");
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
local void SaveAiHelperTable(FILE* file,const char* name,int upgrade,int n,
	AiUnitTypeTable*const * table)
{
    int t;
    int i;
    int j;
    int f;

    for( t=0; t<(upgrade ? UpgradeMax : NumUnitTypes); ++t ) {
	// Look if that unit-type can build something
	for( f=i=0; i<n; ++i ) {
	    if( table[i] ) {
		for( j=0; j<table[i]->Count; ++j ) {
		    if( table[i]->Table[j]->Type==t ) {
			if( !f ) {
			    fprintf(file,"\n  (list '%s '%s\n    ",name,
				    UnitTypes[t].Ident);
			    f=4;
			}
			if( upgrade ) {
			    if( f+strlen(Upgrades[i].Ident)>78 ) {
				f=fprintf(file,"\n    ");
			    }
			    f+=fprintf(file,"'%s ",Upgrades[i].Ident);
			} else {
			    if( f+strlen(UnitTypes[i].Ident)>78 ) {
				f=fprintf(file,"\n    ");
			    }
			    f+=fprintf(file,"'%s ",UnitTypes[i].Ident);
			}
		    }
		}
	    }
	}
	if( f ) {
	    fprintf(file,")");
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
local void SaveAiEquivTable(FILE* file,const char* name,int n,
	AiUnitTypeTable*const * table)
{
    int i;
    int j;
    int f;

    for( i=0; i<n; ++i ) {
	if( table[i] ) {
	    fprintf(file,"\n  (list '%s '%s\n    ",name,
		    UnitTypes[i].Ident);
	    f=4;
	    for( j=0; j<table[i]->Count; ++j ) {
		if( f+strlen(table[i]->Table[j]->Ident)>78 ) {
		    f=fprintf(file,"\n    ");
		}
		f+=fprintf(file,"'%s ",table[i]->Table[j]->Ident);
	    }
	    fprintf(file,")");
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
local void SaveAiCostTable(FILE* file,const char* name,int n,
	AiUnitTypeTable*const * table)
{
    int t;
    int i;
    int j;
    int f;

    for( t=0; t<NumUnitTypes; ++t ) {
	// Look if that unit-type can build something
	for( f=i=0; i<n; ++i ) {
	    if( table[i] ) {
		for( j=0; j<table[i]->Count; ++j ) {
		    if( table[i]->Table[j]->Type==t ) {
			if( !f ) {
			    fprintf(file,"\n  (list '%s '%s\n    ",name,
				    UnitTypes[t].Ident);
			    f=4;
			}
			if( f+strlen(DefaultResourceNames[i])>78 ) {
			    f=fprintf(file,"\n    ");
			}
			f+=fprintf(file,"'%s ",DefaultResourceNames[i]);
		    }
		}
	    }
	}
	if( f ) {
	    fprintf(file,")");
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
local void SaveAiUnitLimitTable(FILE* file,const char* name,int n,
	AiUnitTypeTable*const * table)
{
    int t;
    int i;
    int j;
    int f;

    for( t=0; t<NumUnitTypes; ++t ) {
	// Look if that unit-type can build something
	for( f=i=0; i<n; ++i ) {
	    if( table[i] ) {
		for( j=0; j<table[i]->Count; ++j ) {
		    if( table[i]->Table[j]->Type==t ) {
			if( !f ) {
			    fprintf(file,"\n  (list '%s '%s\n    ",name,
				    UnitTypes[t].Ident);
			    f=4;
			}
			if( f+strlen("food")>78 ) {
			    f=fprintf(file,"\n    ");
			}
			f+=fprintf(file,"'%s ","food");
		    }
		}
	    }
	}
	if( f ) {
	    fprintf(file,")");
	}
    }
}
/**
**	Save AI helper table.
**
**	@param file	Output file.
*/
local void SaveAiHelper(FILE* file)
{
    fprintf(file,"(define-ai-helper");
    //
    //	Save build table
    //
    SaveAiHelperTable(file,"build",0,AiHelpers.BuildCount,AiHelpers.Build);

    //
    //	Save train table
    //
    SaveAiHelperTable(file,"train",0,AiHelpers.TrainCount,AiHelpers.Train);

    //
    //	Save upgrade table
    //
    SaveAiHelperTable(file,"upgrade",0,AiHelpers.UpgradeCount,
	    AiHelpers.Upgrade);

    //
    //	Save research table
    //
    SaveAiHelperTable(file,"research",1,AiHelpers.ResearchCount,
	    AiHelpers.Research);

    //
    //	Save repair table
    //
    SaveAiHelperTable(file,"repair",0,AiHelpers.RepairCount,
	    AiHelpers.Repair);

    //
    //	Save collect table
    //
    SaveAiCostTable(file,"collect",AiHelpers.CollectCount,AiHelpers.Collect);

    //
    //	Save resource table
    //
    SaveAiCostTable(file,"with-goods",AiHelpers.WithGoodsCount,
	    AiHelpers.WithGoods);

    //
    //	Save limits table
    //
    SaveAiUnitLimitTable(file,"unit-limit",AiHelpers.UnitLimitCount,
	    AiHelpers.UnitLimit);

    //
    //	Save equivalence table
    //
    SaveAiEquivTable(file,"unit-equiv",AiHelpers.EquivCount,
	    AiHelpers.Equiv);

    fprintf(file," )\n\n");
}

/**
**	Save the AI type. (recursive)
**
**	@param file	Output file.
**	@param aitype	AI type to save.
*/
local void SaveAiType(FILE* file,const AiType* aitype)
{
    SCM list;

    if( aitype->Next ) {
	SaveAiType(file,aitype->Next);
    }
    DebugLevel3Fn("%s,%s,%s\n" _C_ aitype->Name _C_ aitype->Race _C_ aitype->Class);
    fprintf(file,"(define-ai \"%s\" '%s '%s\n",
	    aitype->Name,aitype->Race ? aitype->Race : "*",aitype->Class);

    fprintf(file,"  '(");
    //	Print the script a little formated
    list=aitype->Script;
    while( !gh_null_p(list) ) {
	fprintf(file,"\n    ");
	lprin1f(gh_car(list),file);
	list=gh_cdr(list);
    }
    fprintf(file," ))\n\n");
}

/**
**	Save the AI types.
**
**	@param file	Output file.
*/
local void SaveAiTypes(FILE* file)
{
    SaveAiType(file,AiTypes);

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
local void SaveAiPlayer(FILE* file,unsigned plynr,const PlayerAi* ai)
{
    SCM script;
    int i;
    const AiBuildQueue* queue;

    fprintf(file,"(define-ai-player %u\n",plynr);
    fprintf(file,"  'ai-type '%s\n",ai->AiType->Name);
    //
    //	Find the script.
    //
    if( !gh_null_p(ai->Script) ) {
	i=0;
	script=ai->AiType->Script;
	do {
	    if( ai->Script==script ) {
		fprintf(file,"  'script '(aitypes %d)\n",i);
		break;
	    }
	    script=gh_cdr(script);
	    ++i;
	} while( !gh_null_p(script) );

	if( gh_null_p(script) ) {	// Not found in ai-types.
	    DebugLevel0Fn("FIXME: not written\n");
	    fprintf(file,"  'script '(FIXME: %d)\n",i);
	}

    }
    fprintf(file,"  'script-debug #%s\n",ai->ScriptDebug ? "t" : "f");
    fprintf(file,"  'sleep-cycles %lu\n",ai->SleepCycles);

    //
    //	All forces
    //
    for( i=0; i<AI_MAX_ATTACKING_FORCES; ++i ) {
	const AiUnitType* aut;
	const AiUnit* aiunit;

	fprintf(file,"  'force '(%d %s%s%s",i,
		ai->Force[i].Completed ? "complete" : "recruit",
		ai->Force[i].Attacking ? " attack" : "",
		ai->Force[i].Defending ? " defend" : "");

	fprintf(file," role ");
	switch( ai->Force[i].Role ) {
	    case AiForceRoleAttack:
		fprintf(file,"attack");
		break;
	    case AiForceRoleDefend:
		fprintf(file,"defend");
		break;
	    default:
		fprintf(file,"unknown");
		break;
	}

	fprintf(file,"\n    types ( ");
	for( aut=ai->Force[i].UnitTypes; aut; aut=aut->Next ) {
	    fprintf(file,"%d %s ",aut->Want,aut->Type->Ident);
	}
	fprintf(file,")\n    units (");
	for( aiunit=ai->Force[i].Units; aiunit; aiunit=aiunit->Next ) {
	    fprintf(file," %d %s",UnitNumber(aiunit->Unit),
		    aiunit->Unit->Type->Ident);
	}
	fprintf(file," ))\n");
    }

    fprintf(file,"  'reserve '(");
    for( i=0; i<MaxCosts; ++i ) {
	fprintf(file,"%s %d ",DefaultResourceNames[i],ai->Reserve[i]);
    }
    fprintf(file,")\n");

    fprintf(file,"  'used '(");
    for( i=0; i<MaxCosts; ++i ) {
	fprintf(file,"%s %d ",DefaultResourceNames[i],ai->Used[i]);
    }
    fprintf(file,")\n");

    fprintf(file,"  'needed '(");
    for( i=0; i<MaxCosts; ++i ) {
	fprintf(file,"%s %d ",DefaultResourceNames[i],ai->Needed[i]);
    }
    fprintf(file,")\n");

    fprintf(file,"  'collect '(");
    for( i=0; i<MaxCosts; ++i ) {
	fprintf(file,"%s %d ",DefaultResourceNames[i],ai->Collect[i]);
    }
    fprintf(file,")\n");

    fprintf(file,"  'need-mask '(");
    for( i=0; i<MaxCosts; ++i ) {
	if( ai->NeededMask&(1<<i) ) {
	    fprintf(file,"%s ",DefaultResourceNames[i]);
	}
    }
    fprintf(file,")\n");

    if( ai->NeedFood ) {
	fprintf(file,"  'need-food\n");
    }

    //
    //	Requests
    //
    fprintf(file,"  'unit-type '(");
    for( i=0; i<ai->UnitTypeRequestsCount; ++i ) {
	fprintf(file,"%s ",ai->UnitTypeRequests[i].Table[0]->Ident);
	fprintf(file,"%d ",ai->UnitTypeRequests[i].Count);
    }
    fprintf(file,")\n");

    fprintf(file,"  'upgrade '(");
    for( i=0; i<ai->UpgradeToRequestsCount; ++i ) {
	fprintf(file,"%s ",ai->UpgradeToRequests[i]->Ident);
    }
    fprintf(file,")\n");

    fprintf(file,"  'research '(");
    for( i=0; i<ai->ResearchRequestsCount; ++i ) {
	fprintf(file,"%s ",ai->ResearchRequests[i]->Ident);
    }
    fprintf(file,")\n");

    //
    //	Building queue
    //
    fprintf(file,"  'building '(");
    for( queue=ai->UnitTypeBuilded; queue; queue=queue->Next ) {
	fprintf(file,"%s %d %d ",queue->Type->Ident,queue->Made,queue->Want);
    }
    fprintf(file,")\n");

    fprintf(file,"  'repair-building %u\n",ai->LastRepairBuilding);

    fprintf(file,"  'repair-workers '(");
    for( i=0; i<UnitMax; ++i ) {
	if( ai->TriedRepairWorkers[i] ) {
	    fprintf(file,"%d %d ",i,ai->TriedRepairWorkers[i]);
	}
    }
    fprintf(file,")");

    fprintf(file,")\n\n");
}

/**
**	Save state of player AIs.
**
**	@param file	Output file.
*/
local void SaveAiPlayers(FILE* file)
{
    unsigned p;

    for( p=0; p<PlayerMax; ++p ) {
	if( Players[p].Ai ) {
	    SaveAiPlayer(file,p,Players[p].Ai);
	}
    }
}

/**
**	Save state of AI to file.
**
**	@param file	Output file.
*/
global void SaveAi(FILE* file)
{
    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: AI $Id$\n\n");

    SaveAiTypesWcName(file);
    SaveAiHelper(file);
    SaveAiTypes(file);
    SaveAiPlayers(file);

    DebugLevel0Fn("FIXME: Saving AI isn't supported\n");
}

/**
**      Setup all at start.
**
**      @param player   The player structure pointer.
*/
global void AiInit(Player* player)
{
    PlayerAi* pai;
    AiType* ait;
    char* ainame;

    DebugLevel0Fn("%d - %s -" _C_ player->Player _C_ player->Name);

    pai=calloc(1,sizeof(PlayerAi));
    if( !pai ) {
	fprintf(stderr,"Out of memory.\n");
	exit(0);
    }
    pai->Player=player;
    ait=AiTypes;

    ainame=AiTypeWcNames[player->AiNum];
    DebugLevel0(" %s\n" _C_ ainame);

    //
    //	Search correct AI type.
    //
    for( ;; ) {
	if( ait->Race && strcmp(ait->Race,player->RaceName) ) {
	    ait=ait->Next;
	    if( !ait && ainame ) {
		ainame=NULL;
		ait=AiTypes;
	    }
	    DebugCheck( !ait );
	    continue;
	}
	if( ainame && strcmp(ainame,ait->Class) ) {
	    ait=ait->Next;
	    if( !ait && ainame ) {
		ainame=NULL;
		ait=AiTypes;
	    }
	    DebugCheck( !ait );
	    continue;
	}
	break;
    }
    if( !ainame ) {
	DebugLevel0Fn("AI: not found!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    DebugLevel0Fn("AI: %s:%s with %s:%s\n" _C_ player->RaceName _C_ ait->Race
	    _C_ ainame _C_ ait->Class );

    pai->AiType=ait;
    pai->Script=ait->Script;

    pai->Collect[TimeCost]=0;
    pai->Collect[GoldCost]=50;
    pai->Collect[WoodCost]=50;
    pai->Collect[OilCost]=0;
    pai->Collect[OreCost]=0;
    pai->Collect[StoneCost]=0;
    pai->Collect[CoalCost]=0;

    player->Ai=pai;
}

/**
**	Cleanup the AI.
*/
global void CleanAi(void)
{
    int i;
    int p;
    PlayerAi* pai;
    void* temp;
    AiType* aitype;
    AiBuildQueue* queue;
    char** cp;

    for( p=0; p<PlayerMax; ++p ) {
	if( (pai=Players[p].Ai) ) {
	    //
	    //	Free forces
	    //
	    for( i=0; i<AI_MAX_ATTACKING_FORCES; ++i ) {
		AiUnitType* aut;
		AiUnit* aiunit;

		for( aut=pai->Force[i].UnitTypes; aut; aut=temp ) {
		    temp=aut->Next;
		    free(aut);
		}
		for( aiunit=pai->Force[i].Units; aiunit; aiunit=temp ) {
		    temp=aiunit->Next;
		    free(aiunit);
		}
	    }
	    //
	    //	Free UnitTypeRequests
	    //
	    free(pai->UnitTypeRequests);
	    //
	    //	Free UpgradeToRequests
	    //
	    free(pai->UpgradeToRequests);
	    //
	    //	Free ResearchRequests
	    //
	    free(pai->ResearchRequests);
	    //
	    //	Free UnitTypeBuilded
	    //
	    for( queue=pai->UnitTypeBuilded; queue; queue=temp ) {
		temp=queue->Next;
		free(queue);
	    }

	    free(pai);
	    Players[p].Ai=NULL;
	}
    }

    //
    //	Free AiTypes.
    //
    for( aitype=AiTypes; aitype; aitype=temp ) {
	DebugLevel3Fn("%s,%s,%s\n" _C_ aitype->Name _C_ aitype->Race _C_ aitype->Class);
	free(aitype->Name);
	free(aitype->Race);
	free(aitype->Class);

	// ai-type->Script freed by ccl

	temp=aitype->Next;
	free(aitype);
    }
    AiTypes=NULL;

    //
    //	Free AiHelpers.
    //
    for( i=0; i<AiHelpers.TrainCount; ++i ) {
	free(AiHelpers.Train[i]);
    }
    free(AiHelpers.Train);

    for( i=0; i<AiHelpers.BuildCount; ++i ) {
	free(AiHelpers.Build[i]);
    }
    free(AiHelpers.Build);

    for( i=0; i<AiHelpers.UpgradeCount; ++i ) {
	free(AiHelpers.Upgrade[i]);
    }
    free(AiHelpers.Upgrade);

    for( i=0; i<AiHelpers.ResearchCount; ++i ) {
	free(AiHelpers.Research[i]);
    }
    free(AiHelpers.Research);

    for( i=0; i<AiHelpers.RepairCount; ++i ) {
	free(AiHelpers.Repair[i]);
    }
    free(AiHelpers.Repair);

    for( i=0; i<AiHelpers.CollectCount; ++i ) {
	free(AiHelpers.Collect[i]);
    }
    free(AiHelpers.Collect);

    for( i=0; i<AiHelpers.WithGoodsCount; ++i ) {
	free(AiHelpers.WithGoods[i]);
    }
    free(AiHelpers.WithGoods);

    for( i=0; i<AiHelpers.UnitLimitCount; ++i ) {
	free(AiHelpers.UnitLimit[i]);
    }
    free(AiHelpers.UnitLimit);

    for( i=0; i<AiHelpers.EquivCount; ++i ) {
	free(AiHelpers.Equiv[i]);
    }
    free(AiHelpers.Equiv);

    memset(&AiHelpers,0,sizeof(AiHelpers));

    //
    //	Mapping original AI numbers in puds to our internal strings
    //
    if( (cp=AiTypeWcNames) ) {			// Free all old names
	while( *cp ) {
	    free(*cp++);
	}
	free(AiTypeWcNames);
	AiTypeWcNames=NULL;
    }
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
local int AiRemoveFromBuilded2(PlayerAi* pai,const UnitType* type)
{
    AiBuildQueue** queue;
    AiBuildQueue* next;

    //
    //	Search the unit-type order.
    //
    for( queue=&pai->UnitTypeBuilded; (next=*queue); queue=&next->Next ) {
	DebugCheck( !next->Want );
	if( type==next->Type && next->Made ) {
	    --next->Made;
	    if( !--next->Want ) {
		*queue=next->Next;
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
local void AiRemoveFromBuilded(PlayerAi* pai,const UnitType* type)
{
    int i;

    if( AiRemoveFromBuilded2(pai,type) ) {
	return;
    }
    //
    //	This could happen if an upgrade is ready, look for equivalent units.
    //
    if( type->Type<AiHelpers.EquivCount && AiHelpers.Equiv[type->Type] ) {
	DebugLevel2Fn("Equivalence for %s\n" _C_ type ->Ident);
	for( i=0; i<AiHelpers.Equiv[type->Type]->Count; ++i ) {
	    if( AiRemoveFromBuilded2(pai,
		    AiHelpers.Equiv[type->Type]->Table[i]) ) {
		return;
	    }
	}
    }

    DebugCheck( 1 );
}

/**
**	Reduce made unit-type from build list.
**
**	@param pai	Computer AI player.
**	@param type	Unit-type which is now available.
**	@return		True if the unit-type could be reduced.
*/
local int AiReduceMadeInBuilded2(const PlayerAi* pai,const UnitType* type)
{
    AiBuildQueue* queue;

    //
    //	Search the unit-type order.
    //
    for( queue=pai->UnitTypeBuilded; queue; queue=queue->Next ) {
	if( type==queue->Type && queue->Made ) {
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
local void AiReduceMadeInBuilded(const PlayerAi* pai,const UnitType* type)
{
    int i;

    if( AiReduceMadeInBuilded2(pai,type) ) {
	return;
    }
    //
    //	This could happen if an upgrade is ready, look for equivalent units.
    //
    if( type->Type<AiHelpers.EquivCount && AiHelpers.Equiv[type->Type] ) {
	DebugLevel2Fn("Equivalence for %s\n" _C_ type ->Ident);
	for( i=0; i<AiHelpers.Equiv[type->Type]->Count; ++i ) {
	    if( AiReduceMadeInBuilded2(pai,
		    AiHelpers.Equiv[type->Type]->Table[i]) ) {
		return;
	    }
	}
    }

    DebugCheck( 1 );
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
global void AiHelpMe(const Unit* attacker,Unit* defender)
{
    PlayerAi* pai;
    AiUnit* aiunit;
    int force;

    DebugLevel0Fn("%d: %d(%s) attacked at %d,%d\n" _C_
	    defender->Player->Player _C_ UnitNumber(defender) _C_
	    defender->Type->Ident _C_ defender->X _C_ defender->Y);

    //
    //	Don't send help to scouts (zeppelin,eye of vision).
    //
    if( !defender->Type->CanAttack && defender->Type->UnitType==UnitTypeFly ) {
	return;
    }

    AiPlayer=pai=defender->Player->Ai;
    if( pai->Force[0].Attacking ) {		// Force 0 busy
	return;
    }

    //
    //	If unit belongs to an attacking force, don't defend it.
    //
    for( force=0; force<AI_MAX_ATTACKING_FORCES; ++force ) {
	if( !pai->Force[force].Attacking ) {	// none attacking
	    // FIXME, send the force for help
	    continue;
	}
	aiunit=pai->Force[force].Units;
	while( aiunit ) {
	    if( defender==aiunit->Unit ) {
		return;
	    }
	    aiunit=aiunit->Next;
	}
    }

    DebugLevel2Fn("Sending force 0 and 1 to defend\n");
    //
    //	Send force 0 defending, also send force 1 if this is home.
    //
    if( attacker ) {
	AiAttackWithForceAt(0,attacker->X,attacker->Y);
	if( !pai->Force[1].Attacking ) {	// none attacking
	    pai->Force[1].Defending=1;
	    AiAttackWithForceAt(1,attacker->X,attacker->Y);
	}
    } else {
	AiAttackWithForceAt(0,defender->X,defender->Y);
	if( !pai->Force[1].Attacking ) {	// none attacking
	    pai->Force[1].Defending=1;
	    AiAttackWithForceAt(1,defender->X,defender->Y);
	}
    }
    pai->Force[0].Defending=1;
}

/**
**	Called if an unit is killed.
**
**	@param unit	Pointer to unit.
*/
global void AiUnitKilled(Unit* unit)
{
    DebugLevel1Fn("%d: %d(%s) killed\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident);

    DebugCheck(unit->Player->Type == PlayerPerson);

    // FIXME: must handle all orders...

    switch( unit->Orders[0].Action ) {
	case UnitActionStill:
	case UnitActionAttack:
	case UnitActionMove:
	    break;
	case UnitActionBuilded:
	    DebugLevel1Fn("%d: %d(%s) killed, under construction!\n" _C_
		    unit->Player->Player _C_ UnitNumber(unit) _C_
		    unit->Type->Ident);
	    AiReduceMadeInBuilded(unit->Player->Ai,unit->Type);
	    break;
	case UnitActionBuild:
	    DebugLevel1Fn("%d: %d(%s) killed, with order %s!\n" _C_
		    unit->Player->Player _C_ UnitNumber(unit) _C_
		    unit->Type->Ident _C_
		    unit->Orders[0].Type->Ident);
	    AiReduceMadeInBuilded(unit->Player->Ai,unit->Orders[0].Type);
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
**	@param unit	Pointer to unit what builds the building.
**	@param what	Pointer to unit building that was build.
*/
global void AiWorkComplete(Unit* unit,Unit* what)
{
    DebugLevel1Fn("%d: %d(%s) build %s at %d,%d completed\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	    what->Type->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerPerson);

    AiRemoveFromBuilded(unit->Player->Ai,what->Type);
}

/**
**	Called if building can't be build.
**
**	@param unit	Pointer to unit what builds the building.
**	@param what	Pointer to unit-type.
*/
global void AiCanNotBuild(Unit* unit,const UnitType* what)
{
    DebugLevel1Fn("%d: %d(%s) Can't build %s at %d,%d\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident
	    _C_ what->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerPerson);

    AiReduceMadeInBuilded(unit->Player->Ai,what);
}

/**
**	Called if building place can't be reached.
**
**	@param unit	Pointer to unit what builds the building.
**	@param what	Pointer to unit-type.
*/
global void AiCanNotReach(Unit* unit,const UnitType* what)
{
    DebugLevel1Fn("%d: %d(%s) Can't reach %s at %d,%d\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	    what->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerPerson);

    AiReduceMadeInBuilded(unit->Player->Ai,what);
}

/**
**	Called if the AI needs more farms.
**
**	@param unit	Point to unit.
**      @param what     Pointer to unit-type.
*/
global void AiNeedMoreFarms(Unit* unit,
	const UnitType* what __attribute__((unused)))
{
    DebugLevel1Fn("%d: %d(%s) need more farms %s at %d,%d\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	    what->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerPerson);

    ((PlayerAi*)unit->Player->Ai)->NeedFood=1;
}

/**
**	Called if training of an unit is completed.
**
**	@param unit	Pointer to unit making.
**	@param what	Pointer to new ready trained unit.
*/
global void AiTrainingComplete(Unit* unit,Unit* what)
{
    DebugLevel1Fn("%d: %d(%s) training %s at %d,%d completed\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	    what->Type->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerPerson);

    AiRemoveFromBuilded(unit->Player->Ai,what->Type);

    AiPlayer=unit->Player->Ai;
    AiCleanForces();
    AiAssignToForce(what);
}

/**
**	Called if upgrading of an unit is completed.
**
**	@param unit	Pointer to unit working.
**	@param what	Pointer to the new unit-type.
*/
global void AiUpgradeToComplete(Unit* unit __attribute__((unused)),
	const UnitType* what __attribute__((unused)))
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
global void AiResearchComplete(Unit* unit __attribute__((unused)),
	const Upgrade* what __attribute__((unused)))
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
global void AiEachCycle(Player* player __attribute__((unused)))
{
}

/**
**	This called for each player, each second.
**
**	@param player	The player structure pointer.
*/
global void AiEachSecond(Player* player)
{
#ifdef TIMEIT
    u_int64_t sv=rdtsc();
    u_int64_t ev;
    static long mv;
    long sx;
#endif

    DebugLevel3Fn("%d:\n" _C_ player->Player);

    AiPlayer=player->Ai;
    IfDebug( if( !AiPlayer ) return; );	// For debug only!
    //
    //	Advance script
    //
    AiExecuteScript();
    //
    //	Look if everything is fine.
    //
    AiCheckUnits();
    //
    //	Handle the resource manager.
    //
    AiResourceManager();
    //
    //	Handle the force manager.
    //
    AiForceManager();
    //
    //	Check for magic actions.
    //
    AiCheckMagic();

#ifdef TIMEIT
    ev=rdtsc();
    sx=(ev-sv);
    mv=(mv+sx)/2;
    DebugLevel1Fn("%ld %ld\n" _C_ sx/1000 _C_ mv/1000);
#endif
}

//@}

#endif // } NEW_AI
