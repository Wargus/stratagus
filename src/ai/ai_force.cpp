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
/**@name ai_force.c	-	AI force functions. */
//
//      (c) Copyright 2001 by Lutz Sammer
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

#include "unit.h"
#include "ai_local.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Ai clean units in a force.
**
**	@param force	Force number.
*/
local void AiCleanForce(int force)
{
    AiUnit** prev;
    AiUnit* aiunit;

    //
    //	Release all killed units.
    //
    prev=&AiPlayer->Force[force].Units;
    while( (aiunit=*prev) ) {
	if( aiunit->Unit->Destroyed ) {
	    RefsDebugCheck( !aiunit->Unit->Refs );
	    if( !--aiunit->Unit->Refs ) {
		ReleaseUnit(aiunit->Unit);
	    }
	    AiPlayer->Force[force].Completed=0;
	    *prev=aiunit->Next;
	    free(aiunit);
	    continue;
	}
	prev=&aiunit->Next;
    }
}

/**
**	Cleanup units in forces.
*/
local void AiCleanForces(void)
{
    int force;

    //
    //	Release all killed units.
    //
    for( force=0; force<AI_MAX_FORCES; ++force ) {
	AiCleanForce(force);
    }
}

/**
**	Check if the units belongs to the force.
**
**	@param force	Force to be checked.
**	@param type	Type to check.
**	@return		Returns true if it fits, false otherwise.
*/
local int AiCheckBelongsToForce(int force,const UnitType* type)
{
    AiUnit* aiunit;
    AiUnitType* aitype;
    int counter[UnitTypeMax];
    int flag;

    memset(counter,0,sizeof(counter));
    //
    //	Count units in force.
    //
    aiunit=AiPlayer->Force[force].Units;
    while( aiunit ) {
	counter[aiunit->Unit->Type->Type]++;
	aiunit=aiunit->Next;
    }

    //
    //	Look what should be in the force.
    //
    flag=0;
    AiPlayer->Force[force].Completed=1;
    aitype=AiPlayer->Force[force].UnitTypes;
    while( aitype ) {
	if( aitype->Want>counter[aitype->Type->Type] ) {
	    if( type==aitype->Type ) {
		if( aitype->Want-1>counter[aitype->Type->Type] ) {
		    AiPlayer->Force[force].Completed=0;
		}
		flag=1;
	    } else {
		AiPlayer->Force[force].Completed=0;
	    }
	}
	aitype=aitype->Next;
    }
    return flag;
}

/**
**	Ai assign unit to force.
*/
global void AiAssignToForce(Unit* unit)
{
    int force;

    AiCleanForces();

    //
    //	Check to which force it belongs
    //
    for( force=0; force<AI_MAX_FORCES; ++force ) {
	if( AiCheckBelongsToForce(force,unit->Type) ) {
	    AiUnit* aiunit;

	    aiunit=malloc(sizeof(aiunit));
	    aiunit->Next=AiPlayer->Force[force].Units;
	    aiunit->Unit=unit;
	    RefsDebugCheck( unit->Destroyed || !unit->Refs );
	    ++unit->Refs;
	    AiPlayer->Force[force].Units=aiunit;
	    break;
	}
    }
}

//@}

#endif // } NEW_AI
