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
/**@name action_demolish.c	-	The demolish action. */
/*
**	(c) Copyright 1999,2000 by Vladi Belperchinov-Shabanski
**
**	$Id$
*/

//@{

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"

/*----------------------------------------------------------------------------
--      Demolish
----------------------------------------------------------------------------*/

/**
**	Unit Demolishs
**
**	@param unit	Unit, for that the demolish is handled.
*/
global void HandleActionDemolish(Unit* unit)
{
    Unit* table[MAX_UNITS];
    int i;
    int n;
    int x, y, ix, iy;
    Unit* goal;
    int err;

    DebugLevel3("Demolish %d\n",unit-Units);

    switch( unit->SubAction ) {
	//
	//	Move near to target.
	//
	case 0:
	    // FIXME: reset first!! why? (johns)
	    err=HandleActionMove(unit); 
	    if( unit->Reset ) {
		goal=unit->Command.Data.Move.Goal;
#ifdef NEW_UNIT
		//
		//	Target is dead, stop demolish
		//
		if( goal ) {
		    if( goal->Destroyed ) {
			DebugLevel0(__FUNCTION__": destroyed unit\n");
			if( !--goal->Refs ) {
			    ReleaseUnit(goal);
			}
			unit->Command.Data.Move.Goal=goal=NoUnitP;
			// FIXME: perhaps I should choose an alternative
			unit->Command.Action=UnitActionStill;
			return;
		    } else if( goal->Removed || !goal->HP
				|| goal->Command.Action==UnitActionDie ) {
			--goal->Refs;
			unit->Command.Data.Move.Goal=goal=NoUnitP;
			// FIXME: perhaps I should choose an alternative
			unit->Command.Action=UnitActionStill;
			return;
		    }
		}
#else
		//
		//	Target is dead, stop demolish
		//
		if( goal && (!goal->Type || !goal->HP
			|| goal->Command.Action==UnitActionDie) ) {
		    // FIXME: this can't happen, HandleActionMove resets goal!
		    unit->Command.Data.Move.Goal=NoUnitP;
		    unit->Command.Action=UnitActionStill;
		    return;
		}
#endif

		//
		//	Have reached target?
		//
		if( goal ) {
		    if( MapDistanceToUnit(unit->X,unit->Y,goal)<=1 ) {
			unit->State=0;
			unit->SubAction=1;
		    }
		} else if( MapDistance(unit->X,unit->Y
			,unit->Command.Data.Move.DX
			,unit->Command.Data.Move.DY)<=1 ) {
		    unit->State=0;
		    unit->SubAction=1;
		} else if( err ) {
		    return;
		}
		unit->Command.Action=UnitActionDemolish;
	    }
	    break;

	//
	//	Demolish the target.
	//
	case 1:
#ifdef NEW_UNIT
	    goal=unit->Command.Data.Move.Goal;
	    if( goal ) {
		--goal->Refs;
	    }
#endif

            x=unit->X;
            y=unit->Y;
            DestroyUnit(unit);
	    // FIXME: Must play explosion sound

	    //
	    //	 Effect of the explosion on units.
	    //
            n=SelectUnits(x-2,y-2, x+2, y+2,table);
	    // FIXME: Don't hit flying units!
            for( i=0; i<n; ++i ) {
                HitUnit(table[i],DEMOLISH_DAMAGE);
            }

	    //
	    //	Terrain effect of the explosion
	    //
            for( ix=x-2; ix<=x+2; ix++ ) {
		for( iy=y-2; iy<=y+2; iy++ ) {
		    n=TheMap.Fields[ix+iy*TheMap.Width].Flags;
		    if( n&MapFieldWall ) {
			MapRemoveWall(ix,iy);
		    } else if( n&MapFieldRocks ) {
			MapRemoveRock(ix,iy);
		    } else if( n&MapFieldForest ) {
			MapRemoveWood(ix,iy);
		    }
		}
	    }
	    break;
    }
}

//@}
