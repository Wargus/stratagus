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
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

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

// FIXME: should combine stand ground and still.


/**
**	Unit stands still!
*/
global void HandleActionStill(Unit* unit)
{
    const UnitType* type;
    Unit* goal;

    DebugLevel3(__FUNCTION__": %Zd\n",UnitNumber(unit));

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

#if 0
    //
    //	Building:	burning FIXME: must moved to general point
    //
    if( type->Building ) {
	if( unit->HP ) {
	    int f;

	    f=(100*unit->HP)/unit->Stats->HitPoints;
	    if( f>75) {
		; // No fire for this
	    } else if( f>50 ) {
		MakeMissile(MissileTypeByIdent("missile-small-fire")
			,unit->X*TileSizeX
				+(type->TileWidth*TileSizeX)/2
			,unit->Y*TileSizeY
				+(type->TileHeight*TileSizeY)/2
				-TileSizeY
			,0,0);
	    } else {
		MakeMissile(MissileTypeByIdent("missile-big-fire")
			,unit->X*TileSizeX
				+(type->TileWidth*TileSizeX)/2
			,unit->Y*TileSizeY
				+(type->TileHeight*TileSizeY)/2
				-TileSizeY
			,0,0);
	    }
	}
    }
#endif

#if 1  // a unit with type->Vanishes is _dying_.
    //
    //	Corpse:		vanishes
    //
    if( type->Vanishes ) {
	//UnitCacheRemove(unit);
	ReleaseUnit(unit);
	return;
    }
#endif

    //
    //	Critters:	are moving random around.
    //
    // FIXME: critters: skeleton and daemon are also critters??????
    if( type->Critter ) {
	static const UnitType* critter;

	if( !critter ) {
	    // FIXME: remove or move the by ident, it is to slow!
	    critter=UnitTypeByIdent("unit-critter");
	}
	if( type==critter ) {
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
		// FIXME: Don't use pathfinder for this.
		// FIXME: atleast prove the field is free.
		unit->Command.Action=UnitActionMove;
		ResetPath(unit->Command);
		unit->Command.Data.Move.Goal=NoUnitP;
		unit->Command.Data.Move.Range=0;
		unit->Command.Data.Move.SX=unit->X;
		unit->Command.Data.Move.SY=unit->Y;
		unit->Command.Data.Move.DX=x;
		unit->Command.Data.Move.DY=y;
		return;
	    }

	}
    }

    //
    //	Workers and mage didn't attack automatic
    //
    if( type->CanAttack && !type->CowerWorker && !type->CowerMage ) {
	//
	// JOHNS: removed Human controlled units attacks in attacking range.
	// JOHNS: use stand ground for old behavior.
	//	Computer controlled units react in reaction range.
	//
	if( /*unit->Player->Type!=PlayerHuman &&*/ !type->Tower ) {
	    if( (goal=AttackUnitsInReactRange(unit)) ) {
		// Weak goal, can choose other unit, come back after attack
		// FIXME: should rewrite command handling
		CommandAttack(unit,unit->X,unit->Y,NULL,FlushCommands);
		unit->SavedCommand=unit->NextCommand[0];
		CommandAttack(unit,goal->X,goal->Y,NULL,FlushCommands);
		DebugLevel3(__FUNCTION__": %Zd Attacking in range %d\n"
			,UnitNumber(unit),unit->SubAction);
		unit->SubAction|=2;
		unit->SavedCommand.Action=UnitActionAttack;
	    }
	} else if( (goal=AttackUnitsInRange(unit)) ) {
	    // FIXME: Johns should rewrite this
	    // FIXME: Applies now only for towers
	    if( !unit->SubAction || unit->Command.Data.Move.Goal!=goal ) {
		// New target.
#ifdef NEW_UNIT
		if( unit->Command.Data.Move.Goal ) {
		    unit->Command.Data.Move.Goal--;
		}
		unit->Command.Data.Move.Goal=goal;
		goal->Refs++;
#else
		unit->Command.Data.Move.Goal=goal;
#endif
		unit->State=0;
		unit->SubAction=1;
		// Turn to target
		if( !type->Tower ) {
		    UnitHeadingFromDeltaXY(unit,goal->X-unit->X,goal->Y-unit->Y);
		    AnimateActionAttack(unit);
		}
	    }
	    return;
	}
    }

    if( unit->SubAction ) {		// was attacking.
#ifdef NEW_UNIT
	if( unit->Command.Data.Move.Goal ) {
	    unit->Command.Data.Move.Goal--;
	}
#endif
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

//@}
