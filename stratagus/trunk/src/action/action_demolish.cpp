//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
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
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
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
    Unit* table[UnitMax];
    int i;
    int n;
    int xmin, ymin, xmax, ymax;
    int ix, iy;
    Unit* goal;
    int err;

    DebugLevel3Fn("Demolish %d\n" _C_ UnitNumber(unit));

    switch( unit->SubAction ) {
	//
	//	Move near to target.
	//
	case 0:				// first entry.
	    NewResetPath(unit);
	    unit->SubAction=1;
	    // FALL THROUGH

	case 1:
	    // FIXME: reset first!! why? (johns)
	    err=DoActionMove(unit);
	    if( unit->Reset ) {
		goal=unit->Orders[0].Goal;
		//
		//	Target is dead, stop demolish.
		//	FIXME: what should I do, go back or explode on place?
		//
		if( goal ) {
		    if( goal->Destroyed ) {
			DebugLevel0Fn("Destroyed unit\n");
			RefsDebugCheck( !goal->Refs );
			if( !--goal->Refs ) {
			    ReleaseUnit(goal);
			}
			// FIXME: perhaps I should choose an alternative
			unit->Orders[0].Goal=NoUnitP;
			unit->Orders[0].Action=UnitActionStill;
			unit->SubAction=0;
			return;
		    } else if( goal->Removed || !goal->HP
				|| goal->Orders[0].Action==UnitActionDie ) {
			RefsDebugCheck( !goal->Refs );
			--goal->Refs;
			RefsDebugCheck( !goal->Refs );
			unit->Orders[0].Goal=NoUnitP;
			// FIXME: perhaps I should choose an alternative
			unit->Orders[0].Action=UnitActionStill;
			unit->SubAction=0;
			return;
		    }
		}

		//
		//	Have reached target? FIXME: could use pathfinder result?
		//
		if( goal ) {
		    if( MapDistanceToUnit(unit->X,unit->Y,goal)<=1 ) {
			unit->State=0;
			unit->SubAction=2;
		    }
		} else if( MapDistance(unit->X,unit->Y
			,unit->Orders[0].X,unit->Orders[0].Y)<=1 ) {
		    unit->State=0;
		    unit->SubAction=2;
		} else if( err==PF_UNREACHABLE ) {
		    unit->Orders[0].Action=UnitActionStill;
		    return;
		}
		DebugCheck( unit->Orders[0].Action!=UnitActionDemolish );
	    }
	    break;

	//
	//	Demolish the target.
	//
	case 2:
	    goal=unit->Orders[0].Goal;
	    if( goal ) {
		RefsDebugCheck( !goal->Refs );
		--goal->Refs;
		RefsDebugCheck( !goal->Refs );
		unit->Orders[0].Goal=NoUnitP;
	    }

	    xmin = unit->X - 2;
	    ymin = unit->Y - 2;
	    xmax = unit->X + 2;
	    ymax = unit->Y + 2;
	    if (xmin<0) xmin=0;
	    if (xmax > TheMap.Width-1) xmax = TheMap.Width-1;
	    if (ymin<0) ymin=0;
	    if (ymax > TheMap.Height-1) ymax = TheMap.Height-1;

	    // FIXME: Must play explosion sound

	    //	FIXME: Currently we take the X fields, the original only the O
	    //		XXXXX ..O..
	    //		XXXXX .OOO.
	    //		XX.XX OO.OO
	    //		XXXXX .OOO.
	    //		XXXXX ..O..
	    //

	    //
	    //	 Effect of the explosion on units.
	    //
            n=SelectUnits(xmin,ymin, xmax, ymax,table);
            for( i=0; i<n; ++i ) {
		if( table[i]->Type->UnitType!=UnitTypeFly && table[i]->HP
		    && table[i] != unit ) {
		    // Don't hit flying units!
		    HitUnit(unit,table[i],DEMOLISH_DAMAGE);
		}
            }

	    //
	    //	Terrain effect of the explosion
	    //
            for( ix=xmin; ix<=xmax; ix++ ) {
		for( iy=ymin; iy<=ymax; iy++ ) {
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
            LetUnitDie(unit);
#ifdef HIERARCHIC_PATHFINDER
	    PfHierMapChangedCallback (xmin, ymin, xmax, ymax);
#endif
	    break;
    }
}

//@}
