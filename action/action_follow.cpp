//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name action_follow.c -	The follow action. */
//
//	(c) Copyright 2001-2002 by Lutz Sammer
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "unit.h"
#include "pathfinder.h"
#include "interface.h"
#include "actions.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Function
----------------------------------------------------------------------------*/

/**
**	Unit follow action:
**
**	@param unit	Pointer to unit.
*/
global void HandleActionFollow(Unit* unit)
{
    Unit* goal;

    DebugLevel3Fn("%d: %d %d,%d \n" _C_ UnitNumber(unit) _C_
	    unit->Orders[0].Goal ? UnitNumber(unit->Orders[0].Goal) : -1 _C_
	    unit->Orders[0].X _C_ unit->Orders[0].Y);

    //
    //	Reached target
    //
    if( unit->SubAction==128 ) {
	goal=unit->Orders[0].Goal;
	if( !goal || goal->Destroyed || !goal->HP
		|| goal->Orders[0].Action==UnitActionDie ) {
	    DebugLevel0Fn("Goal dead\n");
	    if( goal ) {
		RefsDebugCheck( !goal->Refs );
		if( !--goal->Refs && goal->Destroyed ) {
		    ReleaseUnit(goal);
		}
	    }
	    unit->Orders[0].Goal=NoUnitP;
	    unit->Wait=1;
	    unit->SubAction=0;
	    unit->Orders[0].Action=UnitActionStill;
	    if( IsOnlySelected(unit) ) { // update display for new action
		UpdateButtonPanel();
	    }
	    return;
	}
	if( goal->X==unit->Orders[0].X && goal->Y==unit->Orders[0].Y ) {
	    unit->Reset=1;
	    unit->Wait=10;
	    return;
	}
	unit->SubAction=0;
    }

    if( !unit->SubAction ) {		// first entry
	unit->SubAction=1;
	NewResetPath(unit);
	DebugCheck( unit->State!=0 );
    }

    switch( DoActionMove(unit) ) {	// reached end-point?
	case PF_UNREACHABLE:
	    //
	    //	Some tries to reach the goal
	    //
	    if( unit->Orders[0].RangeX <= TheMap.Width
	        || unit->Orders[0].RangeX <= TheMap.Height) {
		unit->Orders[0].RangeX++;
		unit->Orders[0].RangeY++;
		break;
            }
	    // FALL THROUGH
	case PF_REACHED:
	    // FIXME: dark portal teleportation: Goal is used for target circle of power
	    // FIXME: teleporting of units should use dark portal's mana
	    if( (goal=unit->Orders[0].Goal) && 
		    goal->Type->Teleporter && goal->Goal &&
		    MapDistanceBetweenUnits(unit,goal)<4 ) {
		Unit* table[UnitMax];
		Unit* dest;
		int n;
		int i;

		RemoveUnit(unit,NULL);
		unit->X = goal->Goal->X;
		unit->Y = goal->Goal->Y;
		DropOutOnSide(unit,unit->Direction,1,1);
		//FIXME: SoundIdForName() should be called once
		PlayGameSound(SoundIdForName("invisibility"),MaxSampleVolume);
		//FIXME: MissileTypeByIdent() should be called once
		MakeMissile(MissileTypeByIdent("missile-normal-spell"),
			unit->X*TileSizeX+TileSizeX/2,
			unit->Y*TileSizeY+TileSizeY/2,
			unit->X*TileSizeX+TileSizeX/2,
			unit->Y*TileSizeY+TileSizeY/2 );

		unit->Wait=1;
		unit->SubAction=0;
		unit->Orders[0].Action=UnitActionStill;

		//
		//	FIXME: we must check if the units supports the new order.
		//
		dest=NoUnitP;
		n=SelectUnitsOnTile(goal->Goal->X,goal->Goal->Y,table);
		for( i=0; i<n; ++i ) {
		    if( table[i]->Type==UnitTypeByIdent("unit-circle-of-power") ) {
			dest=table[i];
		    }
		}

		if( dest ) {
		    if( (dest->NewOrder.Action==UnitActionHaulOil
				&& !unit->Type->Tanker)
			    || (dest->NewOrder.Action==UnitActionAttack
				&& !unit->Type->CanAttack)
			    || (dest->NewOrder.Action==UnitActionBoard
				&& unit->Type->UnitType!=UnitTypeLand) ) {
			DebugLevel0Fn("Wrong order for unit\n");
			unit->Orders[0].Action=UnitActionStill;
		    } else {
			if( dest->NewOrder.Goal ) {
			    if( dest->NewOrder.Goal->Destroyed ) {
				// FIXME: perhaps we should use another dest?
				DebugLevel0Fn("Destroyed unit in teleport unit\n");
				RefsDebugCheck( !dest->NewOrder.Goal->Refs );
				if( !--dest->NewOrder.Goal->Refs ) {
				    ReleaseUnit(dest->NewOrder.Goal);
				}
				dest->NewOrder.Goal=NoUnitP;
				dest->NewOrder.Action=UnitActionStill;
			    }
			}

			unit->Orders[0]=dest->NewOrder;

			//
			// FIXME: Pending command uses any references?
			//
			if( unit->Orders[0].Goal ) {
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    unit->Orders[0].Goal->Refs++;
			}
		    }
		}
		return;
	    }
	
	    if( !(goal=unit->Orders[0].Goal) ) {// goal has died
		unit->Wait=1;
		unit->SubAction=0;
		unit->Orders[0].Action=UnitActionStill;
		if( IsOnlySelected(unit) ) { // update display for new action
		    UpdateButtonPanel();
		}
		return;
	    }
	    unit->Orders[0].X=goal->X;
	    unit->Orders[0].Y=goal->Y;
	    unit->SubAction=128;

	    // FALL THROUGH
	default:
	    break;
    }

    //
    //	Target destroyed?
    //
    if( (goal=unit->Orders[0].Goal) && goal->Destroyed ) {
	DebugLevel0Fn("Goal dead\n");
	unit->Orders[0].X=goal->X+goal->Type->TileWidth/2;
	unit->Orders[0].Y=goal->Y+goal->Type->TileHeight/2;
	unit->Orders[0].Goal=NoUnitP;
	RefsDebugCheck( !goal->Refs );
	if( !--goal->Refs ) {
	    ReleaseUnit(goal);
	}
	goal=NoUnitP;
	NewResetPath(unit);
    }
    //
    //	Target removed?
    //
    if( unit->Type->Transporter && goal && goal->Removed ) {
	DebugLevel0Fn("Goal removed\n");
	unit->Orders[0].X=goal->X+goal->Type->TileWidth/2;
	unit->Orders[0].Y=goal->Y+goal->Type->TileHeight/2;
	unit->Orders[0].Goal=NoUnitP;
	RefsDebugCheck( !goal->Refs );
	if( !--goal->Refs ) {
	    ReleaseUnit(goal);
	}
	goal=NoUnitP;
	NewResetPath(unit);
    }

    if( unit->Reset ) {
	//
	//	If our leader is dead or stops or attacks:
	//	Attack any enemy in reaction range.
	//		If don't set the goal, the unit can than choose a
	//		better goal if moving nearer to enemy.
	//
	if( unit->Type->CanAttack && !unit->Type->Tower
		&& (!goal || goal->Orders[0].Action==UnitActionAttack
		    || goal->Orders[0].Action==UnitActionStill) ) {
	    goal=AttackUnitsInReactRange(unit);
	    if( goal ) {
		DebugLevel2Fn("Follow attack %d\n" _C_ UnitNumber(goal));
		CommandAttack(unit,goal->X,goal->Y,NULL,FlushCommands);
		// Save current command to come back.
		unit->SavedOrder=unit->Orders[0];
		// This stops the follow command and the attack is executed
		unit->Orders[0].Action=UnitActionStill;
		unit->Orders[0].Goal=NoUnitP;
		unit->SubAction=0;
		unit->Wait=1;
	    }
	}
    }
}

//@}
