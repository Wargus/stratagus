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
/**@name ai_force.c	-	AI force functions. */
//
//      (c) Copyright 2001-2003 by Lutz Sammer
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
** 	Count available units by type in a force.
**
**	The returned array will map UnitType=>number of unit
** 
** 	@param force		the force to count unit
**	@param countByType	array[UnitTypeMax+1] of int
*/
global void AiForceCountUnits( int force, int *countByType )
{
    int type;
    AiUnit *aiunit;
    memset( countByType, 0, sizeof ( int ) * ( UnitTypeMax + 1 ) );

   // FIXME: Should I use equivalent unit types?
    aiunit = AiPlayer->Force[force].Units;
    while ( aiunit ) {
	if ( ( !aiunit->Unit->Destroyed ) &&
	     ( aiunit->Unit->HP ) && ( aiunit->Unit->Orders[0].Action != UnitActionDie ) ) {
	    type = aiunit->Unit->Type->Type;

	    DebugCheck( ( type < 0 ) || ( type > UnitTypeMax ) );
	    countByType[type]++;
	}
	aiunit = aiunit->Next;
    }
}

/**
**	Substract wanted unit in a force. ( from the result of AiForceCountUnits )
**
** 	@param force		the force to count unit
**	@param countByType	array[UnitTypeMax+1] of int
**	@return			The number of missing unit
*/
global int AiForceSubstractWant( int force, int *countByType )
{
    int missing;
    const AiUnitType *aitype;

    missing = 0;
    aitype = AiPlayer->Force[force].UnitTypes;
    while ( aitype ) {
	countByType[aitype->Type->Type] -= aitype->Want;
	if ( countByType[aitype->Type->Type] < 0 ) {
	    missing -= countByType[aitype->Type->Type];
	}
	aitype = aitype->Next;
    }

    return missing;
}

/**
**	Complete dst force with units from src force.
**
**	@param src the force from which units are taken
**	@param dst the force into which units go
*/
global void AiForceTransfert( int src, int dst )
{
    AiUnit **prev;
    AiUnit *aiunit;

    int counter[UnitTypeMax + 1];

   //
   //  Count units in dest force.
   //
    AiForceCountUnits( dst, counter );

   //
   //  Check the dest force requirements.
   //    
    if ( AiForceSubstractWant( dst, counter ) == 0 ) {
       // Nothing missing => abort.
	return;
    }
   // Iterate the source force, moving needed units into dest...
    prev = &AiPlayer->Force[src].Units;
    while ( *prev ) {
	aiunit = ( *prev );
	if ( counter[aiunit->Unit->Type->Type] < 0 ) {
	   // move in dest force...
	    *prev = aiunit->Next;

	    aiunit->Next = AiPlayer->Force[dst].Units;
	    AiPlayer->Force[dst].Units = aiunit;

	    counter[aiunit->Unit->Type->Type]++;
	} else {
	   // Just iterate
	    prev = &aiunit->Next;
	}
    }
}

/**
**	Ai clean units in a force.
**
**	@param force	Force number.
*/
global void AiCleanForce( int force )
{
    AiUnit **prev;
    AiUnit *aiunit;
    int counter[UnitTypeMax + 1];
    int unit_released;

   //
   //  Release all killed units.
   //
    prev = &AiPlayer->Force[force].Units;
    while ( ( aiunit = *prev ) ) {
	if ( aiunit->Unit->Destroyed ) {
	    RefsDebugCheck( !aiunit->Unit->Refs );
	    if ( !--aiunit->Unit->Refs ) {
		ReleaseUnit( aiunit->Unit );
	    }
	    *prev = aiunit->Next;
	    free( aiunit );
	    continue;
	} else if ( !aiunit->Unit->HP || aiunit->Unit->Orders[0].Action == UnitActionDie ) {
	    RefsDebugCheck( !aiunit->Unit->Refs );
	    --aiunit->Unit->Refs;
	    RefsDebugCheck( !aiunit->Unit->Refs );
	    *prev = aiunit->Next;
	    free( aiunit );
	    continue;
	}
	prev = &aiunit->Next;
    }

   //
   //  Count units in force.
   //
    AiForceCountUnits( force, counter );

   //
   //  Look if the force is complete.
   //
    AiPlayer->Force[force].Completed = ( AiForceSubstractWant( force, counter ) == 0 );

   // Don't prune the 0 force in any case 
    if ( force > 0 ) {
       //
       //      Release units too much in force.
       //
	unit_released = 0;
	prev = ( &AiPlayer->Force[force].Units );
	while ( ( aiunit = ( *prev ) ) ) {
	    if ( counter[aiunit->Unit->Type->Type] > 0 ) {
		DebugLevel0Fn( "Release unit %s\n" _C_ aiunit->Unit->Type->Ident );
		counter[aiunit->Unit->Type->Type]--;
		RefsDebugCheck( !aiunit->Unit->Refs );
		--aiunit->Unit->Refs;
		RefsDebugCheck( !aiunit->Unit->Refs );
		*prev = aiunit->Next;

	       // Move this unit somewhere else...             
		AiAssignToForce( aiunit->Unit );
		free( aiunit );

		continue;
	    }
	    prev = &aiunit->Next;
	}
    }

    DebugLevel3Fn( "%d complete %d\n" _C_ force _C_ AiPlayer->Force[force].Completed );
}

/**
**
**	Remove everything in the given force
**
**	@param force	the force to erase
*/
global void AiEraseForce( int force )
{
    AiUnitType *aiut, *next;
    AiUnit *aiu, *next_u;

    aiut = AiPlayer->Force[force].UnitTypes;
    while ( aiut ) {
	next = aiut->Next;
	free( aiut );
	aiut = next;
    }
    AiPlayer->Force[force].UnitTypes = 0;

    aiu = AiPlayer->Force[force].Units;
    while ( aiu ) {
	next_u = aiu->Next;
	free( aiu );
	aiu = next_u;
    }
    AiPlayer->Force[force].Units = 0;

    AiAssignFreeUnitsToForce();
}

/**
**	Cleanup units in forces.
*/
global void AiCleanForces( void )
{
    int force;

   //
   //  Release all killed units.
   //
    for ( force = 0; force < AI_MAX_FORCES; ++force ) {
	AiCleanForce( force );
    }
}

/**
**	Check if the units belongs to the force.
**	If ok, update the completed flag
**
**	@param force	Force to be checked.
**	@param type	Type to check.
**	@return		Returns true if it fits & update completed flag, false otherwise.
*/
local int AiCheckBelongsToForce( int force, const UnitType * type )
{
    int counter[UnitTypeMax + 1];
    int missing;

   //
   //  Count units in force.
   //
    AiForceCountUnits( force, counter );

   //
   //  Look what should be in the force.
   //
    missing = AiForceSubstractWant( force, counter );
    AiPlayer->Force[force].Completed = ( missing == 0 );

    if ( counter[type->Type] < 0 ) {
       // Ok we will put this unit in this force !
       // Just one missing...
	if ( ( counter[type->Type] == -1 ) && ( missing == 1 ) ) {
	    AiPlayer->Force[force].Completed = 1;
	}
	return 1;
    }
    return 0;
}

/**
**	Ai assign unit to force.
**
**	@param unit	Unit to assign to force.
*/
global void AiAssignToForce( Unit * unit )
{
    AiUnit *aiunit;
    int force;

   //
   //  Check to which force it belongs
   //
    for ( force = 0; force < AI_MAX_FORCES; ++force ) {
       // care of populate from scratch only.
	if ( AiPlayer->Force[force].PopulateMode != AiForcePopulateFromScratch ) {
	    continue;
	}

	if ( AiCheckBelongsToForce( force, unit->Type ) ) {
	    aiunit = malloc( sizeof ( *aiunit ) );
	    aiunit->Next = AiPlayer->Force[force].Units;
	    AiPlayer->Force[force].Units = aiunit;
	    aiunit->Unit = unit;
	    RefsDebugCheck( unit->Destroyed || !unit->Refs );
	    ++unit->Refs;
	    return;
	}
    }

   // Add to the 0 force !
   // ( we overflow the 0 force here, so completed does not need update )
    aiunit = malloc( sizeof ( *aiunit ) );
    aiunit->Next = AiPlayer->Force[0].Units;
    AiPlayer->Force[0].Units = aiunit;
    aiunit->Unit = unit;
    RefsDebugCheck( unit->Destroyed || !unit->Refs );
    ++unit->Refs;
}

/**
**	Try to complete a force, using all available units
**
**	@param force	the force to complete
*/
global void AiForceComplete( int force )
{
    int j;

    for ( j = 0; j < AI_MAX_FORCES; ++j ) {
       // Don't complete with self ...
	if ( j == force ) {
	    continue;
	}
       // Complete only with "reusable" forces.
	if ( !AiPlayer->Force[j].UnitsReusable ) {
	    continue;
	}
       // Honor "populate from attack" 
	if ( ( AiPlayer->Force[force].PopulateMode == AiForcePopulateFromAttack ) &&
	     ( !AiPlayer->Force[j].Role == AiForceRoleAttack ) ) {
	    continue;
	}
       // Complete the force automatically...
	AiForceTransfert( j, force );

	if ( AiPlayer->Force[force].Completed ) {
	    break;
	}
    }
}

/**
** 	Enrole a unit in the specific force.
**	FIXME : should take units which are closer to the hotspot.
**	FIXME : should ensure that units can move to the hotspot.
**
**	@param force	the force to put units on
**	@param ut	the searched unittype
**	@param count	the number of unit to add
**	@return		the number of unit still missing ( or 0 if successfull )
*/
global int AiEnroleSpecificUnitType( int force, UnitType * ut, int count )
{
    AiForce *dstForce;
    int src_force;
    AiUnit *aiUnit, **prev;;

    dstForce = AiPlayer->Force + force;
    for ( src_force = 0; src_force < AI_MAX_FORCES; src_force++ ) {
	if ( src_force == force ) {
	    continue;
	}
       // Only populate with reserve 
	if ( !AiPlayer->Force[src_force].UnitsReusable ) {
	    continue;
	}
       // Don't populate attack force with defend reserve.
	if ( ( AiPlayer->Force[src_force].Role == AiForceRoleDefend ) &&
	     ( AiPlayer->Force[force].PopulateMode == AiForcePopulateFromAttack ) ) {
	    continue;
	}

	aiUnit = AiPlayer->Force[src_force].Units;
	prev = &AiPlayer->Force[src_force].Units;
	while ( aiUnit ) {
	   // FIXME : comparaison should match equivalent unit as well
	    if ( aiUnit->Unit->Type == ut ) {
		*prev = aiUnit->Next;

	       // Move to dstForce 
		AiPlayer->Force[src_force].Completed = 0;
		aiUnit->Next = dstForce->Units;
		dstForce->Units = aiUnit;

		count--;
		if ( !count ) {
		    return 0;
		}
	    }
	    prev = &aiUnit->Next;
	    aiUnit = aiUnit->Next;
	}
    }
    return count;
}

/**
**	Make sure that current force requirement are superior to actual assigned unit count
**
*/
local void AiFinalizeForce( int force )
{
    int i;
    int unitcount[UnitTypeMax + 1];
    AiUnitType *aitype;

    AiForceCountUnits( force, unitcount );
    aitype = AiPlayer->Force[force].UnitTypes;
    while ( aitype ) {
	if ( unitcount[aitype->Type->Type] > aitype->Want ) {
	    aitype->Want = unitcount[aitype->Type->Type];
	    unitcount[aitype->Type->Type] = 0;
	}
	aitype = aitype->Next;
    }

    for ( i = 0; i <= UnitTypeMax; i++ ) {
	if ( unitcount[i] > 0 ) {
	    aitype = ( AiUnitType * ) malloc( sizeof ( AiUnitType ) );
	    aitype->Want = unitcount[i];
	    aitype->Type = UnitTypes[i];

	   // Insert into force.
	    aitype->Next = AiPlayer->Force[force].UnitTypes;
	    AiPlayer->Force[force].UnitTypes = aitype;
	}
    }
}

/**
**	Create a force full of available units, responding to the powers.
**
**	@param power	Land/Sea/Air power to match
**	@param utypes	array of unittypes to use
**	@param ucount	Size of the utypes array	
**	@return 	-1 if not possible, 0 if force ready.
*/
global int AiCreateSpecificForce( int *power, int *unittypes, int unittypescount )
{
    int id, maxPower, forceUpdated;
    UnitType *ut;
    int curpower[3];

    curpower[0] = power[0];
    curpower[1] = power[1];
    curpower[2] = power[2];
    AiEraseForce( AiScript->ownForce );

    do {
	forceUpdated = 0;
	maxPower = ( curpower[0] > curpower[1] ?
		     ( curpower[0] > curpower[2] ? 0 : 2 ) :
		     ( curpower[1] > curpower[2] ? 1 : 2 ) );

	for ( id = 0; id < unittypescount; id++ ) {
	    ut = UnitTypes[unittypes[id]];
	    if ( !( ut->CanTarget & ( 1 << maxPower ) ) ) {
		continue;
	    }
	   // Try to respond to the most important power ...
	    if ( AiEnroleSpecificUnitType( AiScript->ownForce, ut, 1 ) == 1 ) {
		continue;
	    }

	    curpower[maxPower] -= AiUnittypeForce( ut );
	    forceUpdated = 1;
	    maxPower = ( curpower[0] > curpower[1] ?
			 ( curpower[0] > curpower[2] ? 0 : 2 ) :
			 ( curpower[1] > curpower[2] ? 1 : 2 ) );
	    if ( curpower[maxPower] <= 0 ) {
		AiFinalizeForce( AiScript->ownForce );
		return 0;
	    }
	}
    } while ( forceUpdated );
   // Sth missing...
    AiFinalizeForce( AiScript->ownForce );
    return -1;
}


/**
**	Assign free units to force.
*/
global void AiAssignFreeUnitsToForce( void )
{
    Unit *table[UnitMax];
    int n;
    int f;
    int i;
    Unit *unit;
    const AiUnit *aiunit;

    AiCleanForces();

    n = AiPlayer->Player->TotalNumUnits;
    memcpy( table, AiPlayer->Player->Units, sizeof ( *AiPlayer->Player->Units ) * n );

   //
   //  Remove all units already in forces.
   //
    for ( f = 0; f < AI_MAX_FORCES; ++f ) {
	aiunit = AiPlayer->Force[f].Units;
	while ( aiunit ) {
	    unit = aiunit->Unit;
	    for ( i = 0; i < n; ++i ) {
		if ( table[i] == unit ) {
		    table[i] = table[--n];
		}
	    }
	    aiunit = aiunit->Next;
	}
    }

   //
   //  Try to assign the remaining units.
   //
    for ( i = 0; i < n; ++i ) {
	if ( table[i]->Active ) {
	    AiAssignToForce( table[i] );
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
global void AiAttackWithForceAt( int force, int x, int y )
{
    const AiUnit *aiunit;

    AiCleanForce( force );

    if ( ( aiunit = AiPlayer->Force[force].Units ) ) {
	AiPlayer->Force[force].Attacking = 1;

       //
       //      Send all units in the force to enemy.
       //
	while ( aiunit ) {
	    if ( aiunit->Unit->Type->CanAttack ) {
		CommandAttack( aiunit->Unit, x, y, NULL, FlushCommands );
	    } else {
		CommandMove( aiunit->Unit, x, y, FlushCommands );
	    }

	    aiunit = aiunit->Next;
	}
    }
}

/**
**	Attack opponent with force.
**
**	@param force	Force number to attack with.
*/
global void AiAttackWithForce( int force )
{
    const AiUnit *aiunit;
    const Unit *enemy;
    int x;
    int y;

    AiCleanForce( force );

    AiPlayer->Force[force].Attacking = 0;
    if ( ( aiunit = AiPlayer->Force[force].Units ) ) {
	AiPlayer->Force[force].Attacking = 1;
	DebugLevel3Fn( "FORCE %d started ( AiAttackWithForce )\n" _C_ force );

	enemy = NoUnitP;
	while ( aiunit && !enemy ) {	// Use an unit that can attack
	    if ( aiunit->Unit->Type->CanAttack ) {
		enemy = AttackUnitsInDistance( aiunit->Unit, MaxMapWidth );
	    }
	    aiunit = aiunit->Next;
	}

	if ( !enemy ) {
	    DebugLevel0Fn( "Need to plan an attack with transporter\n" );
	    if ( !AiPlayer->Force[force].State && !AiPlanAttack( &AiPlayer->Force[force] ) ) {
		DebugLevel0Fn( "Can't transport, look for walls\n" );
		if ( !AiFindWall( &AiPlayer->Force[force] ) ) {
		    AiPlayer->Force[force].Attacking = 0;
		}
	    }
	    return;
	}
	AiPlayer->Force[force].State = 0;
	x = enemy->X;
	y = enemy->Y;

       //
       //      Send all units in the force to enemy.
       //
	aiunit = AiPlayer->Force[force].Units;
	while ( aiunit ) {
	    if ( aiunit->Unit->Type->CanAttack ) {
		CommandAttack( aiunit->Unit, x, y, NULL, FlushCommands );
	    } else {
		CommandMove( aiunit->Unit, x, y, FlushCommands );
	    }
	    aiunit = aiunit->Next;
	}
    }
}

/**
**	Try to group units in a force. Units are grouped arround the closest units of the hotspot. 
**
**	@param 	force	the force to send home.
*/
global void AiGroupForceNear( int force, int targetX, int targetY )
{
    const AiUnit *aiunit;
    const AiUnit *groupunit;
    int unitdst, groupdst;

   // Step 1 : find the unit closest to the force hotspot
    AiCleanForce( force );

    groupdst = -1;

    groupunit = 0;

    aiunit = AiPlayer->Force[force].Units;

   // Sanity : don't group force with only one unit !
    if ( ( !aiunit ) || ( !aiunit->Next ) ) {
	return;
    }

    while ( aiunit ) {
	unitdst = abs( aiunit->Unit->X - targetX ) + abs( aiunit->Unit->Y - targetY );
	if ( ( unitdst < groupdst ) || ( !groupunit ) ) {
	    groupunit = aiunit;
	    groupdst = unitdst;
	}

	aiunit = aiunit->Next;
    }

    AiPlayer->Force[force].Attacking = 1;

   // Order units to attack near the "group" unit...
    aiunit = AiPlayer->Force[force].Units;
    while ( aiunit ) {
	if ( aiunit->Unit->Type->CanAttack ) {
	    CommandAttack( aiunit->Unit, groupunit->Unit->X, groupunit->Unit->Y, NULL,
			   FlushCommands );
	} else {
	    CommandMove( aiunit->Unit, groupunit->Unit->X, groupunit->Unit->Y, FlushCommands );
	}
	aiunit = aiunit->Next;
    }
}

/**
** 	Find the closest home batiment.
** 	ground is 0 : land, 1 : air, 2 : water ( as UnitType::UnitType )
**
**	@param ground	ground type ( land/air/water )
**	@param x	X start position
**	@param y	Y start position
**	@param rsltx	X destination
**	@param rslyx	Y destination
*/
local void AiFindHome( int ground, int x, int y, int *rsltx, int *rslty )
{
   // Find in the player unit's the closer to
   // FIXME : TODO
    *rsltx = AiPlayer->Player->StartX;
    *rslty = AiPlayer->Player->StartY;
}

/**
**	Try to send this force home 
**
**	@param 	force	the force to send home.
*/
global void AiSendForceHome( int force )
{
    const AiUnit *aiunit;
    int i, type;
    int x[3], y[3];

    AiCleanForce( force );
    aiunit = AiPlayer->Force[force].Units;

    for ( i = 0; i < 3; i++ ) {
	x[i] = -1;
	y[i] = -1;
    }

    while ( aiunit ) {
	type = aiunit->Unit->Type->UnitType;

	if ( x[type] == -1 ) {
	    AiFindHome( type, aiunit->Unit->X, aiunit->Unit->Y, &x[type], &y[type] );
	}

	CommandMove( aiunit->Unit, x[type], y[type], FlushCommands );

	aiunit = aiunit->Next;
    }
}

//----------------------------------------------------------------------------
//      Handle attack of force with transporter.
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
local void AiLoadForce( AiForce * force )
{
    AiUnit *aiunit;
    Unit *table[UnitMax];
    int n;
    int i;
    int o;
    int f;

   //
   //  Find all transporters.
   //
    n = 0;
    aiunit = force->Units;
    while ( aiunit ) {
	if ( aiunit->Unit->Type->Transporter ) {
	    table[n++] = aiunit->Unit;
	}
	aiunit = aiunit->Next;
    }

    if ( !n ) {
	DebugLevel0Fn( "No transporter, lost or error in code?\n" );
	force->MustTransport = 0;
	force->State = 0;
	return;
    }
   //
   //  Load all on transporter.
   //
    f = o = i = 0;
    aiunit = force->Units;
    while ( aiunit ) {
	Unit *unit;

	unit = aiunit->Unit;
	if ( !unit->Type->Transporter && unit->Type->UnitType == UnitTypeLand ) {
	    if ( !unit->Removed ) {
		f = 1;
		if ( unit->Orders[0].Action != UnitActionBoard ) {
		    if ( UnitIdle( table[i] ) ) {
			DebugLevel0Fn( "Send transporter %d\n" _C_ i );
			CommandFollow( table[i], unit, FlushCommands );
		    }
		    CommandBoard( unit, table[i], FlushCommands );
		    ++o;
		   // FIXME
		    if ( o == table[i]->Type->MaxOnBoard ) {
			DebugLevel0Fn( "FIXME: next transporter for AI boarding\n" );
			return;
		    }
		}
	    }
	}
	aiunit = aiunit->Next;
    }

    if ( !f ) {
	DebugLevel0Fn( "All are loaded\n" );
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
local void AiSendTransporter( AiForce * force )
{
    AiUnit *aiunit;

   //
   //  Find all transporters.
   //
    aiunit = force->Units;
    while ( aiunit ) {
       //      Transporter to unload units
	if ( aiunit->Unit->Type->Transporter ) {
	    CommandUnload( aiunit->Unit, force->GoalX, force->GoalY, NoUnitP, FlushCommands );
	   //      Ships to defend transporter
	} else if ( aiunit->Unit->Type->UnitType == UnitTypeNaval ) {
	    CommandAttack( aiunit->Unit, force->GoalX, force->GoalY, NoUnitP, FlushCommands );
	}
	aiunit = aiunit->Next;
    }
    ++force->State;
}

/**
**	Step 3)
**	Wait for transporters landed.
**
**	@param force	Force pointer.
**
*/
local void AiWaitLanded( AiForce * force )
{
    AiUnit *aiunit;
    int i;

    DebugLevel0Fn( "Waiting\n" );
   //
   //  Find all transporters.
   //
    i = 1;
    aiunit = force->Units;
    while ( aiunit ) {
	if ( aiunit->Unit->Type->Transporter ) {
	    if ( UnitIdle( aiunit->Unit ) ) {
		DebugLevel0Fn( "Unloading\n" );
	       // Don't tell empty transporters to unload.
		if ( aiunit->Unit->InsideCount ) {
		    CommandUnload( aiunit->Unit, force->GoalX, force->GoalY,
				   NoUnitP, FlushCommands );
		    i = 0;
		}
	    } else {
		i = 0;
	    }
	}
	aiunit = aiunit->Next;
    }
    if ( i ) {
	++force->State;		// all unloaded
    }
}

/**
**	Step 4)
**	Force on attack ride. We attack until there is no unit or enemy left.
**
**	@param force	Force pointer.
*/
local void AiForceAttacks( AiForce * force )
{
    const AiUnit *aiunit;

    if ( ( aiunit = force->Units ) ) {
	while ( aiunit ) {
	   // Still some action
	    if ( !UnitIdle( aiunit->Unit ) ) {
		break;
	    }
	    aiunit = aiunit->Next;
	}
       // Must mark the attack as terminated 
	if ( !aiunit ) {
	    DebugLevel3Fn( "FORCE stopped ( AiForceAttacks, unitactionstill )\n" );
	    DebugLevel3Fn( "force target was %d %d\n" _C_ force->GoalX _C_ force->GoalY );
	    DebugLevel3Fn( "unit pos was %d %d\n" _C_ force->Units->Unit->X _C_ force->Units->
			   Unit->Y );

	    force->Attacking = 0;
	   // AiAttackWithForce(force-AiPlayer->Force);
	}
    } else {
	DebugLevel3Fn( "FORCE stopped ( AiAttackWithForce, no unit )\n" );
	force->Attacking = 0;
    }
}

global void AiForceHelpMe( int force, const Unit * attacker, Unit * defender )
{
    AiForce *aiForce;
    AiUnit *rescue;

    aiForce = AiPlayer->Force + force;

   // Don't handle special cases
    if ( aiForce->State > 0 ) {
	return;
    }

    switch ( aiForce->HelpMode ) {
    case AiForceDontHelp:
       // Don't react (easy)
	return;

    case AiForceHelpForce:
       // Send all idles units to help
	rescue = aiForce->Units;
	while ( rescue ) {
	   // TODO : check that dead units does appear there
	    if ( UnitIdle( rescue->Unit ) ) {
	       // This unit attack !
		if ( rescue->Unit->Type->CanAttack ) {
		    CommandAttack( rescue->Unit, attacker->X, attacker->Y, NULL,
				   FlushCommands );
		} else {
		    CommandMove( rescue->Unit, attacker->X, attacker->Y, FlushCommands );
		}
	       // Now the force is attacking ( again )
		aiForce->Attacking = 1;
	    }
	    rescue = rescue->Next;
	}
	break;

    default:
       // the usual way : create a defense force, send it, ...
	AiFindDefendScript( attacker->X, attacker->Y );
	break;
    }
}

/**
**	Handle an attack force.
**
**	@param force	Force pointer.
*/
local void AiGuideAttackForce( AiForce * force )
{
    enum
    { StartState = 1, TransporterLoaded, WaitLanded, AttackNow };

    switch ( force->State ) {
       //
       //  Load units on transporters.
       //
    case StartState:
	AiLoadForce( force );
	break;
    case TransporterLoaded:
	AiSendTransporter( force );
	break;
    case WaitLanded:
	AiWaitLanded( force );
	break;
    case AttackNow:
	force->State = 0;
	AiAttackWithForce( force - AiPlayer->Force );
	break;

       //
       //  Attacking!
       //
    case 0:
	AiForceAttacks( force );
	break;
    }
}

/**
**	Entry point of force manager, perodic called.
*/
global void AiForceManager( void )
{
    int force;

   //
   //  Look if our defenders still have enemies in range.
   //
    for ( force = 0; force < AI_MAX_FORCES; ++force ) {
	if ( AiPlayer->Force[force].Attacking ) {
	    AiCleanForce( force );
	    AiGuideAttackForce( &AiPlayer->Force[force] );
	}
    }
    AiAssignFreeUnitsToForce();
}

//@}
