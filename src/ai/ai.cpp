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
    AiBuildQueue* queue;
    int i;
    int n;
    int t;
    int x;

    memset(counter,0,sizeof(counter));
    //
    //	Count the already made build requests.
    //
    for( queue=AiPlayer->UnitTypeBuilded; queue; queue=queue->Next ) {
	counter[queue->Type->Type]+=queue->Want;
	DebugLevel0Fn("Already in build queue: %s %d/%d\n" _C_
		queue->Type->Ident _C_ queue->Made _C_ queue->Want);
    }

    //
    //	Look if something is missing.
    //
    n=AiPlayer->RequestsCount;
    for( i=0; i<n; ++i ) {
	t=AiPlayer->UnitTypeRequests[i].Table[0]->Type;
	x=AiPlayer->UnitTypeRequests[i].Count;
	if( x>AiPlayer->Player->UnitTypesCount[t]+counter[t] ) {
	    DebugLevel0Fn("Need %s\n" _C_
		    AiPlayer->UnitTypeRequests[i].Table[0]->Ident);
	    // Request it.
	    AiAddUnitTypeRequest(AiPlayer->UnitTypeRequests[i].Table[0],
		    x-AiPlayer->Player->UnitTypesCount[t]-counter[t]);
	}
	counter[t]+=x;
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

    DebugLevel0Fn("%d - %s\n" _C_ player->Player _C_ player->Name);

    pai=calloc(1,sizeof(PlayerAi));
    if( !pai ) {
	fprintf(stderr,"Out of memory.\n");
	exit(0);
    }
    pai->Player=player;
    ait=AiTypes;
    //
    //	Search correct AI type.
    //
    while( ait->Race && strcmp(ait->Race,player->RaceName) ) {
	ait=ait->Next;
	DebugCheck( !ait );
    }
    pai->AiType=ait;
    pai->Script=ait->Script;

    player->Ai=pai;
}

/*----------------------------------------------------------------------------
--	Callback Functions
----------------------------------------------------------------------------*/

/**
**	Called if a Unit is Attacked
**
**	@param unit	Pointer to unit that is being attacked.
**
*/
global void AiHelpMe(Unit* unit)
{
    DebugLevel0Fn("%d %d" _C_ unit->X _C_ unit->Y);
}

/**
**	Called if work complete (Buildings).
**
**	@param unit	Pointer to unit what builds the building.
**	@param what	Pointer to unit building that was build.
*/
global void AiWorkComplete(Unit* unit,Unit* what)
{
    AiBuildQueue** queue;
    AiBuildQueue* next;
    PlayerAi* pai;

    DebugLevel1Fn("AiPlayer %d: %d build %s at %d,%d completed\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit), what->Type->Ident _C_
	    unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerHuman);

    //
    //	Search the unit-type order.
    //
    pai=unit->Player->Ai;
    for( queue=&pai->UnitTypeBuilded; (next=*queue); queue=&next->Next ) {
	if( what->Type==next->Type && next->Want && next->Made ) {
	    if( next->Want==next->Made ) {
		*queue=next->Next;
		free(next);
	    }
	    return;
	}
    }
    DebugCheck( 1 );
}

/**
**	Called if building can't be build.
**
**	@param unit	Pointer to unit what builds the building.
**	@param what	Pointer to unit-type.
*/
global void AiCanNotBuild(Unit* unit,const UnitType* what)
{
    AiBuildQueue* queue;
    const PlayerAi* pai;

    DebugLevel1Fn("AiPlayer %d: %d Can't build %s at %d,%d\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit), what->Ident _C_
	    unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerHuman);

    //
    //	Search the unit-type order.
    //
    pai=unit->Player->Ai;
    for( queue=pai->UnitTypeBuilded; queue; queue=queue->Next ) {
	if( what==queue->Type && queue->Made ) {
	    queue->Made--;
	    return;
	}
    }
    DebugCheck( 1 );
}

/**
**	Called if building place can't be reached.
**
**	@param unit	Pointer to unit what builds the building.
**	@param what	Pointer to unit-type.
*/
global void AiCanNotReach(Unit* unit,const UnitType* what)
{
    AiBuildQueue* queue;
    const PlayerAi* pai;

    DebugLevel1Fn("AiPlayer %d: %d Can't reach %s at %d,%d\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit), what->Ident _C_
	    unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerHuman);

    //
    //	Search the unit-type order.
    //
    pai=unit->Player->Ai;
    for( queue=pai->UnitTypeBuilded; queue; queue=queue->Next ) {
	if( what==queue->Type && queue->Made ) {
	    queue->Made--;
	    return;
	}
    }
    DebugCheck( 1 );
}

/**
**	Called if training of an unit is completed.
**
**	@param unit	Pointer to unit.
**	@param what	Pointer to type.
*/
global void AiTrainingComplete(Unit* unit,Unit* what)
{
    AiBuildQueue** queue;
    AiBuildQueue* next;
    PlayerAi* pai;

    DebugLevel1Fn("AiPlayer %d: %d training %s at %d,%d completed\n" _C_
	    unit->Player->Player _C_ UnitNumber(unit), what->Type->Ident _C_
	    unit->X _C_ unit->Y);

    DebugCheck(unit->Player->Type == PlayerHuman);

    //
    //	Search the unit-type order.
    //
    pai=unit->Player->Ai;
    for( queue=&pai->UnitTypeBuilded; (next=*queue); queue=&next->Next ) {
	if( what->Type==next->Type && next->Want && next->Made ) {
	    if( next->Want==next->Made ) {
		*queue=next->Next;
		free(next);
	    }
	    return;
	}
    }
    DebugCheck( 1 );
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

    DebugLevel0Fn("%d:\n" _C_ player->Player);

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
}

//@}

#endif // } NEW_AI
