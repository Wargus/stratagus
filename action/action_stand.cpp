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
/**@name action_stand.c	-	The stand ground action. */
/*
**	(c) Copyright 2000 by Lutz Sammer
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
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "tileset.h"
#include "map.h"
#include "sound_server.h"
#include "missile.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Unit stands ground!
**
**	@param unit	Action handled for this unit pointer.
*/
global void HandleActionStandGround(Unit* unit)
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

    if( !unit->Reset ) {		// animation can't be aborted
	return;
    }

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
		MakeMissile(MissileSmallFire
			,unit->X*TileSizeX
				+(type->TileWidth*TileSizeX)/2
			,unit->Y*TileSizeY
				+(type->TileHeight*TileSizeY)/2
				-TileSizeY
			,0,0);
	    } else {
		MakeMissile(MissileBigFire
			,unit->X*TileSizeX
				+(type->TileWidth*TileSizeX)/2
			,unit->Y*TileSizeY
				+(type->TileHeight*TileSizeY)/2
				-TileSizeY
			,0,0);
	    }
	}
    }

#if 0
    // JOHNS: critters removed here
    //
    //	Critters:	are moving random around.
    //
    if( type->Critter ) {
	// FIXME: critters: skeleton and daemon are also critters??????
	if( type->Type==UnitCritter ) {
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
		unit->Command.Action=UnitActionMove;
		unit->Command.Data.Move.Fast=1;
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
#endif

    //
    //	Workers and mage didn't attack automatic
    //	removed with standground && !type->CowerWorker && !type->CowerMage 
    if( type->CanAttack ) {
	//
	//	Units attacks in attacking range.
	//
	if( (goal=AttackUnitsInRange(unit)) ) {
	    // FIXME: johns, looks wired what I have written here
	    // FIXME: Why have I written such a chaos? (johns)
	    if( !unit->SubAction || unit->Command.Data.Move.Goal!=goal ) {
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

    if( unit->SubAction ) {
	unit->SubAction=unit->State=0;
    }

    //
    //	Land units:	are turning left/right.
    //
    if( type->LandUnit ) {
	switch( (MyRand()>>8)&0x0FF ) {
	    case 0:			// Turn clockwise
#ifdef NEW_HEADING
		unit->Direction+=32;
#else
		unit->Heading=(unit->Heading+1)&7;
#endif
		UnitUpdateHeading(unit);
		if( UnitVisible(unit) ) {
		    MustRedraw|=RedrawMap;
		}
		break;
	    case 1:			// Turn counter clockwise
#ifdef NEW_HEADING
		unit->Direction-=32;
#else
		unit->Heading=(unit->Heading-1)&7;
#endif
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
