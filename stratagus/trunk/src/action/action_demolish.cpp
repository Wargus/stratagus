/*
**	A clone of a famous game.
*/
/**@name action_demolish.c	-	The demolish action. */
/*
**	(c) Copyright 1999 by Vladi Belperchinov-Shabanski
**
**
*/

//@{

#include <stdio.h>
#include <stdlib.h>

#include "clone.h"
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
	    // FIXME: RESET FIRST!!
	    err=HandleActionMove(unit); 
	    if( unit->Reset ) {
		goal=unit->Command.Data.Move.Goal;

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
            x=unit->X;
            y=unit->Y;
            DestroyUnit(unit);
	    // FIXME: Must play explosion sound
            n=SelectUnits(x-2,y-2, x+2, y+2,table);
	    // FIXME: Don't hit flying units!
            for( i=0; i<n; ++i ) {
                HitUnit(table[i],DEMOLISH_DAMAGE);
            }

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
