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

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"

#include "player.h"
#include "unit.h"

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
	    DebugLevel3Fn("Equivalence for %s\n",
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
	DebugLevel0Fn("FIXME: %s\n",AiPlayer->UpgradeToRequests[i]->Ident);
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
**	W*rCr*ft number to internal ai-type name.
*/
global char* AiTypeWcNames[] = {
    "land-attack",
    "passive",
};

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
    while( (!ait->Race || strcmp(ait->Race,player->RaceName))
	    && (!ainame || strcmp(ainame,ait->Class)) ) {
	ait=ait->Next;
	DebugCheck( !ait );
    }
    DebugLevel0Fn("AI: %s with %s\n", player->RaceName, ainame );

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
*/
local void AiRemoveFromBuilded(PlayerAi* pai,const UnitType* type)
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
	    return;
	}
    }
    DebugCheck( 1 );
}

/**
**	Reduce made unit-type from build list.
**
**	@param pai	Computer AI player.
**	@param type	Unit-type which is now available.
*/
local void AiReduceMadeInBuilded(const PlayerAi* pai,const UnitType* type)
{
    AiBuildQueue* queue;

    //
    //	Search the unit-type order.
    //
    for( queue=pai->UnitTypeBuilded; queue; queue=queue->Next ) {
	if( type==queue->Type && queue->Made ) {
	    queue->Made--;
	    return;
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
global void AiHelpMe(Unit* unit)
{
    PlayerAi* pai;

    DebugLevel0Fn("%d: %d(%s) attacked at %d,%d\n" _C_ unit->Player->Player _C_
	    UnitNumber(unit) _C_ unit->Type->Ident _C_ unit->X _C_ unit->Y);
    //
    //	Send force 0 defending
    //
    pai=unit->Player->Ai;
    if( !pai->Force[0].Attacking && unit->Type->Building ) {
	AiAttackWithForceAt(0,unit->X,unit->Y);
	pai->Force[0].Defending=1;
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
    DebugLevel1Fn("%d: %d Can't build %s at %d,%d\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit), what->Ident _C_
	    unit->X _C_ unit->Y);

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
**	Called if an unit is killed.
**
**	@param unit	Pointer to unit.
*/
global void AiUnitKilled(Unit* unit)
{
    // FIXME: if the unit builds something for us it must restartet!!!!
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
}

//@}

#endif // } NEW_AI
