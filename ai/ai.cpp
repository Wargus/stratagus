//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name new_ai.c	-	The new computer player AI main file. */
//
//      (c) Copyright 2000,2001 by Lutz Sammer
//
//      $Id$

#ifdef NEW_AI	// {

//@{

#define noTIMEIT			/// Enable cpu use debugging

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"

#include "player.h"
#include "unit.h"
#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif

#include "ai_local.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int AiSleep;			/// Ai sleeps # frames
global int AiTimeFactor = 100;		/// Adjust the AI build times
global int AiCostFactor = 100;		/// Adjust the AI costs

global AiType* AiTypes;			/// List of all AI types.
global AiHelper AiHelpers;		/// AI helper variables

global PlayerAi* AiPlayer;		/// Current AI player

/**
**	W*rCr*ft number to internal ai-type name.
*/
local char* AiTypeWcNames[] = {
    "land-attack",
    "passive",
    "orc-03",
    "hum-04",
    "orc-04",
    "hum-05",
    "orc-05",
    "hum-06",
    "orc-06",
    "hum-07",
    "orc-07",
    "hum-08",
    "orc-08",
    "hum-09",
    "orc-09",
    "hum-10",
    "orc-10",
    "hum-11",
    "orc-11",
    "hum-12",
    "orc-12",
    "hum-13",
    "orc-13",
    "hum-14-orange",
    "orc-14-blue",
    "sea-attack",
    "air-attack",
    "hum-14-red",
    "hum-14-white",
    "hum-14-black",
    "orc-14-green",
    "orc-14-white",
    "orc-exp-04",
    "orc-exp-05",
    "orc-exp-07a",
    "orc-exp-09",
    "orc-exp-10",
    "orc-exp-12",
    "orc-exp-06a",
    "orc-exp-06b",
    "orc-exp-11a",
    "orc-exp-11b",
    "hum-exp-02a-red",
    "hum-exp-02b-black",
    "hum-exp-02c-yellow",
    "hum-exp-03a-orange",
    "hum-exp-03b-red",
    "hum-exp-03c-violet",
    "hum-exp-04a-black",
    "hum-exp-04b-red",
    "hum-exp-04c-white",
    "hum-exp-05a-green",
    "hum-exp-05b-orange",
    "hum-exp-05c-violet",
    "hum-exp-05d-yellow",
    "hum-exp-06a-green",
    "hum-exp-06b-black",
    "hum-exp-06c-orange",
    "hum-exp-06d-red",
    "hum-exp-08a-white",
    "hum-exp-08b-yellow",
    "hum-exp-08c-violet",
    "hum-exp-09a-black",
    "hum-exp-09b-red",
    "hum-exp-09c-green",
    "hum-exp-09d-white",
    "hum-exp-10a-violet",
    "hum-exp-10b-green",
    "hum-exp-10c-black",
    "hum-exp-11a",
    "hum-exp-11b",
    "hum-exp-12a",
    "orc-exp-05b",
    "hum-exp-07a",
    "hum-exp-07b",
    "hum-exp-07c",
    "orc-exp-12a",
    "orc-exp-12b",
    "orc-exp-12c",
    "orc-exp-12d",
    "orc-exp-02",
    "orc-exp-07b",
    "orc-exp-03",
};

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
	value=leval(gh_car(pai->Script),NIL);
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
    const AiBuildQueue* queue;
    const int* unit_types_count;
    int i;
    int j;
    int n;
    int t;
    int x;
    int e;

    memset(counter,0,sizeof(counter));
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
		    AiPlayer->UnitTypeRequests[i].Table[0]->Ident,x);
	    AiAddUnitTypeRequest(AiPlayer->UnitTypeRequests[i].Table[0],
		    x-e-counter[t]);
	    counter[t]+=x-e-counter[t];
	}
	counter[t]-=x;
    }

    //
    //	Look through the forces what is missing.
    //
    for( i=0; i<AI_MAX_FORCES; ++i ) {
	const AiUnitType* aiut;

	for( aiut=AiPlayer->Force[i].UnitTypes; aiut; aiut=aiut->Next ) {
	    t=aiut->Type->Type;
	    x=aiut->Want;
	    if( x>unit_types_count[t]+counter[t] ) {	// Request it.
		DebugLevel3Fn("Force %d need %s * %d\n" _C_ i _C_
			aiut->Type->Ident,x);
		AiAddUnitTypeRequest(aiut->Type,
			x-unit_types_count[t]-counter[t]);
		counter[t]+=x-unit_types_count[t]-counter[t];
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
**      Setup all at start.
**
**      @param player   The player structure pointer.
*/
global void AiInit(Player* player)
{
    PlayerAi* pai;
    AiType* ait;
    char* ainame;

    DebugLevel0Fn("%d - %s\n" _C_ player->Player _C_ player->Name);

    pai=calloc(1,sizeof(PlayerAi));
    if( !pai ) {
	fprintf(stderr,"Out of memory.\n");
	exit(0);
    }
    pai->Player=player;
    ait=AiTypes;

    ainame=NULL;
    if( player->AiNum<sizeof(AiTypeWcNames)/sizeof(*AiTypeWcNames) ) {
	ainame=AiTypeWcNames[player->AiNum];
    }
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
    DebugLevel0Fn("AI: %s:%s with %s:%s\n" _C_ player->RaceName _C_ ait->Race
	    _C_ ainame _C_ ait->Class );

    pai->AiType=ait;
    pai->Script=ait->Script;

    player->Ai=pai;
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
	    if( !--next->Want ) {
		*queue=next->Next;
		free(next);
	    }
	    --next->Made;
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
**	@param unit	Pointer to unit that is being attacked.
*/
global void AiHelpMe(const Unit* attacker,Unit * defender)
{
    PlayerAi* pai;
    AiUnit* aiunit;
    int force;

    DebugLevel0Fn("%d: %d(%s) attacked at %d,%d\n" _C_
	    defender->Player->Player _C_ UnitNumber(defender) _C_
	    defender->Type->Ident _C_ defender->X _C_ defender->Y);

    pai=defender->Player->Ai;
    if( pai->Force[0].Attacking ) {		// Force 0 busy
	return;
    }

    //
    //	If unit belongs to an attacking force, don't defend it.
    //
    for( force=0; force<AI_MAX_FORCES; ++force ) {
	aiunit=pai->Force[force].Units;
	if( !pai->Force[force].Attacking ) {	// none attacking
	    continue;
	}
	while( aiunit ) {
	    if( defender==aiunit->Unit ) {
		return;
	    }
	    aiunit=aiunit->Next;
	}
    }

    //
    //	Send force 0 defending
    //
    if( attacker ) {
	AiAttackWithForceAt(0,attacker->X,attacker->Y);
    } else {
	AiAttackWithForceAt(0,defender->X,defender->Y);
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

    DebugCheck(unit->Player->Type == PlayerHuman);

    // FIXME: if the unit builds something for us it must restartet!!!!
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

    DebugCheck(unit->Player->Type == PlayerHuman);

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

    DebugCheck(unit->Player->Type == PlayerHuman);

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

    DebugCheck(unit->Player->Type == PlayerHuman);

    AiReduceMadeInBuilded(unit->Player->Ai,what);
}

/**
**	Called if the AI needs more farms.
**
**	@param unit	Point to unit.
**      @param what     Pointer to unit-type.
*/
global void AiNeedMoreFarms(Unit* unit,const UnitType* what)
{
    DebugLevel1Fn("%d: %d(%s) need more farms %s at %d,%d\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	    what->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerHuman);

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

    DebugCheck(unit->Player->Type == PlayerHuman);

    AiRemoveFromBuilded(unit->Player->Ai,what->Type);

    AiPlayer=unit->Player->Ai;
    AiAssignToForce(what);
}

/**
**	Called if upgrading of an unit is completed.
**
**	@param unit	Pointer to unit working.
**	@param what	Pointer to the new unit-type.
*/
global void AiUpgradeToComplete(Unit* unit,const UnitType* what)
{
    DebugLevel1Fn("%d: %d(%s) upgrade-to %s at %d,%d completed\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	    what->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerHuman);
}

/**
**	Called if reseaching of an unit is completed.
**
**	@param unit	Pointer to unit working.
**	@param what	Pointer to the new upgrade.
*/
global void AiResearchComplete(Unit* unit,const Upgrade* what)
{
    DebugLevel1Fn("%d: %d(%s) research %s at %d,%d completed\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit) _C_ unit->Type->Ident _C_
	    what->Ident _C_ unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerHuman);

    // FIXME: upgrading knights -> paladins, must rebuild lists!
}

/**
**	This is called for each player, each frame.
**
**	@param player	The player structure pointer.
*/
global void AiEachFrame(Player* player)
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

#ifdef TIMEIT
    ev=rdtsc();
    sx=(ev-sv);
    mv=(mv+sx)/2;
    DebugLevel1Fn("%ld %ld\n",sx/1000,mv/1000);
#endif
}

//@}

#endif // } NEW_AI
