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
/**@name ai_force.c	-	AI force functions. */
//
//      (c) Copyright 2001-2003 by Lutz Sammer
//
//	Stratagus is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	Stratagus is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
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

#include "stratagus.h"

#include "unittype.h"
#include "unit.h"
#include "ai_local.h"
#include "actions.h"
#include "map.h"

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
    const AiUnitType* aitype;
    int counter[UnitTypeMax];

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
	    *prev=aiunit->Next;
	    free(aiunit);
	    continue;
	} else if( !aiunit->Unit->HP
		|| aiunit->Unit->Orders[0].Action==UnitActionDie ) {
	    RefsDebugCheck( !aiunit->Unit->Refs );
	    --aiunit->Unit->Refs;
	    RefsDebugCheck( !aiunit->Unit->Refs );
	    *prev=aiunit->Next;
	    free(aiunit);
	    continue;
	}
	prev=&aiunit->Next;
    }

    //
    //	Count units in force.
    //
    memset(counter,0,sizeof(counter));
    aiunit=AiPlayer->Force[force].Units;
    while( aiunit ) {
	// FIXME: Should I use equivalent unit types?
	counter[aiunit->Unit->Type->Type]++;
	aiunit=aiunit->Next;
    }

    //
    //	Look if the force is complete.
    //
    AiPlayer->Force[force].Completed=1;
    aitype=AiPlayer->Force[force].UnitTypes;
    while( aitype ) {
	if( aitype->Want>counter[aitype->Type->Type] ) {
	    DebugLevel3Fn("%d: missing %s.\n" _C_ force _C_ aitype->Type->Ident);
	    AiPlayer->Force[force].Completed=0;
	}
	counter[aitype->Type->Type]-=aitype->Want;
	aitype=aitype->Next;
    }

    //
    //	Release units too much in force.
    //
    if( !AiPlayer->Force[force].Attacking ) {
	prev=&AiPlayer->Force[force].Units;
	while( (aiunit=*prev) ) {
	    if( counter[aiunit->Unit->Type->Type]>0 ) {
		DebugLevel0Fn("Release unit %s\n"
			_C_ aiunit->Unit->Type->Ident);
		counter[aiunit->Unit->Type->Type]--;
		RefsDebugCheck( !aiunit->Unit->Refs );
		--aiunit->Unit->Refs;
		RefsDebugCheck( !aiunit->Unit->Refs );
		*prev=aiunit->Next;
		free(aiunit);
		continue;
	    }
	    prev=&aiunit->Next;
	}
    }

    DebugLevel3Fn("%d complete %d\n" _C_ force
	    _C_ AiPlayer->Force[force].Completed);
}

/**
**	Cleanup units in forces.
*/
global void AiCleanForces(void)
{
    int force;

    //
    //	Release all killed units.
    //
    for( force=0; force<AI_MAX_ATTACKING_FORCES; ++force ) {
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
	// FIXME: Should I use equivalent unit types?
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
**
**	@param unit	Unit to assign to force.
*/
global void AiAssignToForce(Unit* unit)
{
    int force;

    //
    //	Check to which force it belongs
    //
    for( force=0; force<AI_MAX_FORCES; ++force ) {
	// No troops for attacking force
	if( !AiPlayer->Force[force].Defending
		&& AiPlayer->Force[force].Attacking ) {
	    continue;
	}

	if( AiCheckBelongsToForce(force,unit->Type) ) {
	    AiUnit* aiunit;

	    aiunit=malloc(sizeof(*aiunit));
	    aiunit->Next=AiPlayer->Force[force].Units;
	    AiPlayer->Force[force].Units=aiunit;
	    aiunit->Unit=unit;
	    RefsDebugCheck( unit->Destroyed || !unit->Refs );
	    ++unit->Refs;
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

    AiCleanForces();

    n=AiPlayer->Player->TotalNumUnits;
    memcpy(table,AiPlayer->Player->Units,sizeof(*AiPlayer->Player->Units)*n);

    //
    //	Remove all units already in forces.
    //
    for( f=0; f<AI_MAX_ATTACKING_FORCES; ++f ) {
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
    int f;

    // Move the force to a free position so it can be used for a new
    // attacking party
    if( force<AI_MAX_FORCES ) {
	AiUnitType *aiut;
	AiUnitType *temp;
	AiUnitType **aiut2;

	f=AI_MAX_FORCES;
	while( AiPlayer->Force[f].Attacking ) {
	    ++f;
	    if( f==AI_MAX_ATTACKING_FORCES ) {
		DebugLevel0Fn("No free attacking forces\n");
		f=force;
		break;
	    }
	}
	if( f!=AI_MAX_ATTACKING_FORCES ) {
	    for( aiut=AiPlayer->Force[f].UnitTypes; aiut; aiut=temp ) {
		temp=aiut->Next;
		free(aiut);
	    }

	    AiPlayer->Force[f]=AiPlayer->Force[force];
	    memset(&AiPlayer->Force[force],0,sizeof(AiForce));
	    aiut=AiPlayer->Force[force].UnitTypes;
	    aiut2=&AiPlayer->Force[force].UnitTypes;
	    while( aiut ) {
		*aiut2=malloc(sizeof(**aiut2));
		(*aiut2)->Next=NULL;
		(*aiut2)->Want=aiut->Want;
		(*aiut2)->Type=aiut->Type;
		aiut=aiut->Next;
		aiut2=&(*aiut2)->Next;
	    }
	}

	force=f;
    }

    AiCleanForce(force);

    AiPlayer->Force[force].Attacking=0;
    if( (aiunit=AiPlayer->Force[force].Units) ) {
	AiPlayer->Force[force].Attacking=1;

	enemy=NoUnitP;
	while( aiunit && !enemy ) {	// Use an unit that can attack
	    if( aiunit->Unit->Type->CanAttack ) {
		enemy = AttackUnitsInDistance(aiunit->Unit, MaxMapWidth);
	    }
	    aiunit=aiunit->Next;
	}

	if (!enemy) {
	    DebugLevel0Fn("Need to plan an attack with transporter\n");
	    if( !AiPlayer->Force[force].State 
		    && !AiPlanAttack(&AiPlayer->Force[force]) ) {
		DebugLevel0Fn("Can't transport, look for walls\n");
		if( !AiFindWall(&AiPlayer->Force[force]) ) {
		    AiPlayer->Force[force].Attacking=0;
		}
	    }
	    return;
	}
	AiPlayer->Force[force].State=0;
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

//----------------------------------------------------------------------------
//	Handle attack of force with transporter.
//----------------------------------------------------------------------------

/**
**	Step 1)
**	Load force on transporters.
**
**	@param force	Force pointer.
**
**	@todo	If an unit can't reach the transporter the code hangs.
**		We must the transporter land on a new position.
**		Or the board action will be better written.
*/
local void AiLoadForce(AiForce* force)
{
    AiUnit* aiunit;
    Unit* table[UnitMax];
    int n;
    int i;
    int o;
    int f;

    //
    //	Find all transporters.
    //
    n=0;
    aiunit=force->Units;
    while( aiunit ) {
	if( aiunit->Unit->Type->Transporter ) {
	    table[n++]=aiunit->Unit;
	}
	aiunit=aiunit->Next;
    }

    if( !n ) {
	DebugLevel0Fn("No transporter, lost or error in code?\n");
	force->MustTransport=0;
	force->State=0;
	return;
    }

    //
    //	Load all on transporter.
    //
    f=o=i=0;
    aiunit=force->Units;
    while( aiunit ) {
	Unit* unit;

	unit=aiunit->Unit;
	if( !unit->Type->Transporter
		&& unit->Type->UnitType==UnitTypeLand ) {
	    if( !unit->Removed ) {
		f=1;
		if( unit->Orders[0].Action!=UnitActionBoard ) {
		    if( table[i]->Orders[0].Action==UnitActionStill
			    && table[i]->OrderCount==1 ) {
			DebugLevel0Fn("Send transporter %d\n" _C_ i);
			CommandFollow(table[i],unit,FlushCommands);
		    }
		    CommandBoard(unit,table[i],FlushCommands);
		    ++o;
		    if( o==MAX_UNITS_ONBOARD ) {
			DebugLevel0Fn("FIXME: next transporter\n");
			return;
		    }
		}
	    }
	}
	aiunit=aiunit->Next;
    }

    if( !f ) {
	DebugLevel0Fn("All are loaded\n");
	++force->State;
    }
}

/**
**	Step 2)
**	Send force awaay in transporters, to unload at target position.
**
**	@param force	Force pointer.
**
**	@todo	The transporter should avoid enemy contact and should land
**		at an unfortified coast. If we send more transporters they
**		should land on different positions.
*/
local void AiSendTransporter(AiForce* force)
{
    AiUnit* aiunit;

    //
    //	Find all transporters.
    //
    aiunit=force->Units;
    while( aiunit ) {
	//	Transporter to unload units
	if( aiunit->Unit->Type->Transporter ) {
	    CommandUnload(aiunit->Unit, force->GoalX, force->GoalY, NoUnitP,
		    FlushCommands);
	//	Ships to defend transporter
	} else if( aiunit->Unit->Type->UnitType==UnitTypeNaval ) {
	    CommandAttack(aiunit->Unit, force->GoalX, force->GoalY, NoUnitP,
		    FlushCommands);
	}
	aiunit=aiunit->Next;
    }
    ++force->State;
}

/**
**	Step 3)
**	Wait for transporters landed.
**
**	@param force	Force pointer.
**
**	@todo	If units blocks the unload process we should move them away.
**		FIXME: hangs if the unit can't unloaded.
*/
local void AiWaitLanded(AiForce* force)
{
    AiUnit* aiunit;
    int i;
    int j;

    DebugLevel0Fn("Waiting\n");
    //
    //	Find all transporters.
    //
    i=1;
    aiunit=force->Units;
    while( aiunit ) {
	if( aiunit->Unit->Type->Transporter ) {
	    if( aiunit->Unit->Orders[0].Action==UnitActionStill ) {
		DebugLevel0Fn("Unloading\n");
		for( j=0; j<MAX_UNITS_ONBOARD; ++j ) {
		    if( aiunit->Unit->OnBoard[j] ) {
			CommandUnload(aiunit->Unit,force->GoalX,force->GoalY,
			    NoUnitP,FlushCommands);
			i=0;
			break;
		    }
		}
	    } else {
		i=0;
	    }
	}
	aiunit=aiunit->Next;
    }
    if( i ) {
	++force->State;			// all unloaded
    }
}

/**
**	Step 4)
**	Force on attack ride. We attack until there is no unit or enemy left.
**
**	@param force	Force pointer.
*/
local void AiForceAttacks(AiForce* force)
{
    const AiUnit* aiunit;

    if( (aiunit=force->Units) ) {
	while( aiunit ) {
	    // Still some action
	    if( aiunit->Unit->Orders[0].Action!=UnitActionStill ) {
		break;
	    }
	    aiunit=aiunit->Next;
	}
	if( !aiunit ) {
	    AiAttackWithForce(force-AiPlayer->Force);
	}
    } else {
	force->Attacking=0;
    }
}

/**
**	Handle an attack force.
**
**	@param force	Force pointer.
*/
local void AiGuideAttackForce(AiForce* force)
{
    enum { StartState=1, TransporterLoaded, WaitLanded, AttackNow };

    switch( force->State ) {
	    //
	    //	Load units on transporters.
	    //
	case StartState:
	    AiLoadForce(force);
	    break;
	case TransporterLoaded:
	    AiSendTransporter(force);
	    break;
	case WaitLanded:
	    AiWaitLanded(force);
	    break;
	case AttackNow:
	    force->State=0;
	    AiAttackWithForce(force-AiPlayer->Force);
	    break;

	    //
	    //	Attacking!
	    //
	case 0:
	    AiForceAttacks(force);
	    break;
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
    for( force=0; force<AI_MAX_ATTACKING_FORCES; ++force ) {
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
	if( AiPlayer->Force[force].Attacking ) {
	    AiCleanForce(force);
	    AiGuideAttackForce(&AiPlayer->Force[force]);
	}
    }
    AiAssignFreeUnitsToForce();
}

//@}

#endif // } NEW_AI
