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
//
//	(c) Copyright 1999-2001 by Vladi Belperchinov-Shabanski
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--      Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "sound.h"
#include "map.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Functions
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

    DebugLevel3Fn("Demolish %d\n",unit-Units);

    switch( unit->SubAction ) {
	//
	//	Move near to target.
	//
	case 0:
	    // FIXME: reset first!! why? (johns)
	    err=HandleActionMove(unit); 
	    if( unit->Reset ) {
		goal=unit->Command.Data.Move.Goal;
		//
		//	Target is dead, stop demolish
		//
		if( goal ) {
		    if( goal->Destroyed ) {
			DebugLevel0Fn("Destroyed unit\n");
#ifdef REFS_DEBUG
			DebugCheck( !goal->Refs );
#endif
			if( !--goal->Refs ) {
			    ReleaseUnit(goal);
			}
			unit->Command.Data.Move.Goal=goal=NoUnitP;
			// FIXME: perhaps I should choose an alternative
			unit->Command.Action=UnitActionStill;
			return;
		    } else if( goal->Removed || !goal->HP
				|| goal->Command.Action==UnitActionDie ) {
#ifdef REFS_DEBUG
			DebugCheck( !goal->Refs );
#endif
			--goal->Refs;
#ifdef REFS_DEBUG
			DebugCheck( !goal->Refs );
#endif
			unit->Command.Data.Move.Goal=goal=NoUnitP;
			// FIXME: perhaps I should choose an alternative
			unit->Command.Action=UnitActionStill;
			return;
		    }
		}

		//
		//	Have reached target? FIXME: could use result?
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
		} else if( err==PF_UNREACHABLE ) {
		    return;
		}
		unit->Command.Action=UnitActionDemolish;
	    }
	    break;

	//
	//	Demolish the target.
	//
	case 1:
	    goal=unit->Command.Data.Move.Goal;
	    if( goal ) {
#ifdef REFS_DEBUG
		DebugCheck( !goal->Refs );
#endif
		--goal->Refs;
#ifdef REFS_DEBUG
		DebugCheck( !goal->Refs );
#endif
		unit->Command.Data.Move.Goal=NoUnitP;
	    }

            x=unit->X;
            y=unit->Y;
            DestroyUnit(unit);
	    // FIXME: Must play explosion sound

	    // 	FIXME: Currently we take the X fields, the original only the O
	    //		XXXXX ..O..
	    //		XXXXX .OOO.
	    //		XX.XX OO.OO
	    //		XXXXX .OOO.
	    //		XXXXX ..O..
	    //

	    //
	    //	 Effect of the explosion on units.
	    //
            n=SelectUnits(x-2,y-2, x+2, y+2,table);
	    // FIXME: Don't hit flying units!
            for( i=0; i<n; ++i ) {
	    	if ( table[i]->Type->LandUnit )
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
