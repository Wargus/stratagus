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
/**@name action_still.c	-	The stand still action. */
//
//	(c) Copyright 1998,2000 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"

#include "missile.h"
#include "unittype.h"
#include "actions.h"
#include "unit.h"
#include "tileset.h"
#include "map.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Unit stands still or stand ground.
**
**	@param unit	Unit pointer for action.
*/
global void ActionStillGeneric(Unit* unit,int ground)
{
    const UnitType* type;
    Unit* temp;
    Unit* goal;

    DebugLevel3Fn(" %Zd\n",UnitNumber(unit));

    //
    //	Animations
    //

    type=unit->Type;

    if( unit->SubAction ) {
	//
	//	Attacking unit in attack range.
	//
	AnimateActionAttack(unit);
    } else {
	//
	//	Still animation
	//
        DebugCheck( !type->Animations || !type->Animations->Still );

	UnitShowAnimation(unit,type->Animations->Still);

	//
	//	FIXME: this a workaround of a bad code.
	//		UnitShowAnimation resets frame.
	//
	if( unit->State==1 && type->GoldMine ) {
	    unit->Frame=!!unit->Command.Data.GoldMine.Active;
	}
	if( unit->State==1 && type->GivesOil ) {
	    unit->Frame=unit->Command.Data.OilWell.Active ? 2 : 0;
	}

    }

    if( !unit->Reset ) {		// animation can't be aborted here
	return;
    }

    //
    //	Critters:	are moving random around.
    //
    // FIXME: critters: skeleton and daemon are also critters??????
    if( type->Critter && type==UnitTypeCritter ) {
	int x;
	int y;
    
	x=unit->X;
	y=unit->Y;
	switch( (SyncRand()>>12)&15 ) {
	    case 0:	x++;		break;
	    case 1:	y++;		break;
	    case 2:	x--;		break;
	    case 3:	y--;		break;
	    case 4:	x++; y++;	break;
	    case 5:	x--; y++;	break;
	    case 6:	y--; x++;	break;
	    case 7:	x--; y--;	break;
	    default:
		    break;
	}
	if( x<0 ) {
	    x=0;
	} else if( x>=TheMap.Width ) {
	    x=TheMap.Width-1;
	} 
	if( y<0 ) {
	    y=0;
	} else if( y>=TheMap.Height ) {
	    y=TheMap.Height-1;
	}
	if( x!=unit->X || y!=unit->Y ) {
	    if( CheckedCanMoveToMask(x,y,TypeMovementMask(type)) ) {
		// FIXME: Don't use pathfinder for this.
		unit->Command.Action=UnitActionMove;
		ResetPath(unit->Command);
		unit->Command.Data.Move.Goal=NoUnitP;
		unit->Command.Data.Move.Range=0;
		unit->Command.Data.Move.SX=unit->X;
		unit->Command.Data.Move.SY=unit->Y;
		unit->Command.Data.Move.DX=x;
		unit->Command.Data.Move.DY=y;
	    }
	}
	// NOTE: critter couldn't attack automatic through the return
	return;
    }

    //
    //	Workers and mage didn't attack automatic
    //
    if( type->CanAttack && !type->CowerWorker && !type->CowerMage ) {
	//
	//	Normal units react in reaction range.
	//
	if( !type->Tower && !ground ) {
	    if( (goal=AttackUnitsInReactRange(unit)) ) {
		// Weak goal, can choose other unit, come back after attack
		// FIXME: should rewrite command handling
		CommandAttack(unit,unit->X,unit->Y,NULL,FlushCommands);
		unit->SavedCommand=unit->NextCommand[0];
		unit->SavedCommand.Action=UnitActionAttack;
		CommandAttack(unit,goal->X,goal->Y,NULL,FlushCommands);
		DebugLevel3Fn(" %Zd Attacking in range %d\n"
			,UnitNumber(unit),unit->SubAction);
		unit->SubAction|=2;
	    }
	} else if( (goal=AttackUnitsInRange(unit)) ) {
	    DebugLevel3Fn(" %Zd #%d\n",UnitNumber(goal),goal->Refs);
	    //
	    //	Old goal destroyed.
	    //
	    temp=unit->Command.Data.Move.Goal;
	    if( temp && temp->Destroyed ) {
		DebugLevel3Fn(" destroyed unit %Zd #%d\n"
			,UnitNumber(temp),temp->Refs);
		DebugCheck( !temp->Refs );
		if( !--temp->Refs ) {
		    ReleaseUnit(temp);
		}
		unit->Command.Data.Move.Goal=temp=NoUnitP;
	    }
	    if( !unit->SubAction || temp!=goal ) {
		// New target.
		if( temp ) {
		    DebugLevel3Fn(" old unit %Zd #%d\n"
			    ,UnitNumber(temp),temp->Refs);
		    DebugCheck( !temp->Refs );
		    temp->Refs--;
		    DebugCheck( !temp->Refs );
		}
		unit->Command.Data.Move.Goal=goal;
		goal->Refs++;
		unit->State=0;
		unit->SubAction=1;
		if( !type->Tower ) {
		    UnitHeadingFromDeltaXY(unit,goal->X-unit->X
			    ,goal->Y-unit->Y);
		    AnimateActionAttack(unit);
		}
	    }
	    return;
	}
    }

    if( unit->SubAction ) {		// was attacking.
	if( (temp=unit->Command.Data.Move.Goal) ) {
	    DebugCheck( !temp->Refs );
	    temp->Refs--;
	    DebugCheck( !temp->Refs );
	    unit->Command.Data.Move.Goal=NoUnitP;
	}
	unit->SubAction=unit->State=0;
    }

    //
    //	Land units:	are turning left/right.
    //
    if( type->LandUnit ) {
	switch( (MyRand()>>8)&0x0FF ) {
	    case 0:			// Turn clockwise
		unit->Direction+=NextDirection;
		UnitUpdateHeading(unit);
		if( UnitVisible(unit) ) {
		    MustRedraw|=RedrawMap;
		}
		break;
	    case 1:			// Turn counter clockwise
		unit->Direction-=NextDirection;
		UnitUpdateHeading(unit);
		if( UnitVisible(unit) ) {
		    MustRedraw|=RedrawMap;
		}
		break;
	    default:			// does nothing
		break;
	}
	return;
    }

    //
    //	Sea units:	are floating up/down.
    //
    if( type->SeaUnit ) {
	unit->IY=(MyRand()>>15)&1;
	return;
    }

    //
    //	Air units:	are floating up/down.
    //
    if( type->AirUnit ) {
	unit->IY=(MyRand()>>15)&1;
	return;
    }
}


/**
**	Unit stands still!
**
**	@param unit	Unit pointer for still action.
*/
global void HandleActionStill(Unit* unit)
{
    ActionStillGeneric(unit,0);
}

//@}
