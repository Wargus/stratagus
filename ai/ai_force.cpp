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
#include <string.h>

#include "freecraft.h"

#include "unittype.h"
#include "unit.h"
#include "ai_local.h"
#include "actions.h"

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
	} else if( !aiunit->Unit->HP
		|| aiunit->Unit->Orders[0].Action==UnitActionDie ) {
	    RefsDebugCheck( !aiunit->Unit->Refs );
	    --aiunit->Unit->Refs;
	    RefsDebugCheck( !aiunit->Unit->Refs );
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

/**
**	Assign free units to force.
*/
global void AiAssignFreeUnitsToForce(void)
{
    Unit* table[UnitMax];
    int n;
    int f;
    int i;
    Unit* unit;
    const AiUnit* aiunit;

    n=AiPlayer->Player->TotalNumUnits;
    memcpy(table,AiPlayer->Player->Units,sizeof(*AiPlayer->Player->Units)*n);

    AiCleanForces();

    //
    //	Remove all units already in forces.
    //
    for( f=0; f<AI_MAX_FORCES; ++f ) {
	aiunit=AiPlayer->Force[f].Units;
	while( aiunit ) {
	    unit=aiunit->Unit;
	    for( i=0; i<n; ++i ) {
		if( table[i]==unit ) {
		    table[i]=table[--n];
		}
	    }
	    aiunit=aiunit->Next;
	}
    }

    //
    //	Try to assign the remaining units.
    //
    for( i=0; i<n; ++i ) {
	if( table[i]->Active ) {
	    AiAssignToForce(table[i]);
	}
    }
}

/**
**	Attack at position with force.
**
**	@param force	Force number to attack with.
**	@param x	X tile map position to be attacked.
**	@param y	Y tile map position to be attacked.
*/
global void AiAttackWithForceAt(int force,int x,int y)
{
    const AiUnit* aiunit;

    AiCleanForce(force);

    if( (aiunit=AiPlayer->Force[force].Units) ) {
	AiPlayer->Force[force].Attacking=1;

	//
	//	Send all units in the force to enemy.
	//
	while( aiunit ) {
	    if( aiunit->Unit->Type->CanAttack ) {
		CommandAttack(aiunit->Unit, x, y, NULL,FlushCommands);
	    } else {
		CommandMove(aiunit->Unit, x, y, FlushCommands);
	    }
	    aiunit=aiunit->Next;
	}
    }
}

/**
**	Attack opponent with force.
**
**	@param force	Force number to attack with.
*/
global void AiAttackWithForce(int force)
{
    const AiUnit* aiunit;
    const Unit* enemy;
    int x;
    int y;

    AiCleanForce(force);

    if( (aiunit=AiPlayer->Force[force].Units) ) {
	AiPlayer->Force[force].Attacking=1;

	enemy=NoUnitP;
	while( aiunit && !enemy ) {	// Use an unit that can attack
	    if( aiunit->Unit->Type->CanAttack ) {
		enemy = AttackUnitsInDistance(aiunit->Unit, 1000);
	    }
	    aiunit=aiunit->Next;
	}

	if (!enemy) {
	    return;
	}
	x = enemy->X;
	y = enemy->Y;

	//
	//	Send all units in the force to enemy.
	//
	aiunit=AiPlayer->Force[force].Units;
	while( aiunit ) {
	    if( aiunit->Unit->Type->CanAttack ) {
		CommandAttack(aiunit->Unit, x, y, NULL,FlushCommands);
	    } else {
		CommandMove(aiunit->Unit, x, y, FlushCommands);
	    }
	    aiunit=aiunit->Next;
	}
    }
}

/**
**	Entry point of force manager, perodic called.
*/
global void AiForceManager(void)
{
    int force;

    //
    //	Look if our defenders still have enemies in range.
    //
    for( force=0; force<AI_MAX_FORCES; ++force ) {
	if( AiPlayer->Force[force].Defending ) {
	    const AiUnit* aiunit;

	    AiCleanForce(force);
	    //
	    //	Look if still enemies in attack range.
	    //
	    aiunit=AiPlayer->Force[force].Units;
	    while( aiunit ) {
		if( aiunit->Unit->Type->CanAttack &&
			AttackUnitsInReactRange(aiunit->Unit) ) {
		    break;
		}
		aiunit=aiunit->Next;
	    }
	    if( !aiunit ) {		// No enemies go home.
		DebugLevel0Fn("FIXME: not written, should send force home\n");
		AiPlayer->Force[force].Defending=0;
		AiPlayer->Force[force].Attacking=0;
	    }
	}
    }

    // AiAssignFreeUnitsToForce();
}

//@}

#endif // } NEW_AI
