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
/**@name action_unload.c		-	The unload action. */
//
//	(c) Copyright 1998,2000-2001 by Lutz Sammer
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

#include "stratagus.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "map.h"
#include "interface.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/


#define LandUnitMask ( \
    MapFieldLandUnit | \
    MapFieldBuilding | \
    MapFieldWall | \
    MapFieldRocks | \
    MapFieldForest | \
    MapFieldCoastAllowed | \
    MapFieldWaterAllowed | \
    MapFieldUnpassable)

/**
**	Find a free position close to x, y
**
**	@param x	Original x search position.
**	@param y	Original y search position
**	@param resx	Unload x position.
**	@param resy	Unload y position.
**      @param mask     Movement mask for the unit to be droped.
**
**	@return		True if a position was found, False otherwise.
**      @note           resx and resy are undefined if a position is not found.
**
**	@bug FIXME: Place unit only on fields reachable from the transporter
*/
local int FindUnloadPosition(int x,int y,int *resx,int *resy,int mask)
{
    int i,n,addx,addy;
    addx=addy=1;
    --x;
    for( n=0; n<2; ++n ) {
        //  Nobody: There was some code here to check for unloading units that can
        //  only go on even tiles. It's useless, since we can only unload land units.
	for( i=addy; i--; y++ ) {
	    if( CheckedCanMoveToMask(x,y,mask) ) {
                *resx=x;
                *resy=y;
		return 1;
	    }
	}
	++addx;
	for( i=addx; i--; x++ ) {
	    if( CheckedCanMoveToMask(x,y,mask) ) {
                *resx=x;
                *resy=y;
		return 1;
	    }
	}
	++addy;
	for( i=addy; i--; y-- ) {
	    if( CheckedCanMoveToMask(x,y,mask) ) {
                *resx=x;
                *resy=y;
		return 1;
	    }
	}
	++addx;
	for( i=addx; i--; x-- ) {
	    if( CheckedCanMoveToMask(x,y,mask) ) {
                *resx=x;
                *resy=y;
		return 1;
	    }
	}
	++addy;
    }
    return 0;
}

/**
**	Reappear unit on map.
**
**	@param unit	Unit to drop out.
**	@param addx	Tile size in x.
**	@param addy	Tile size in y.
**
**	@return		True if unit can be unloaded.
**
**	@bug FIXME: Place unit only on fields reachable from the transporter
*/
global int UnloadUnit(Unit* unit)
{
    int x,y;
    DebugCheck( !unit->Removed );
    if ( !FindUnloadPosition(unit->X,unit->Y,&x,&y,UnitMovementMask(unit)) ) {
        return 0;
    }
    unit->X=x;
    unit->Y=y;
    unit->Wait=1;		// should be correct unit has still action
    PlaceUnit(unit,x,y);
    return 1;
}


/**
**      Find the closest piece of coast you can unload units on
**      
**      @param  x       start location for the search
**      @param  y       start location for the search
**      @param  resx    coast x position
**      @param  resy    coast y position
**      @return 1 if a location was found, 0 otherwise
*/
local int ClosestFreeCoast(int x,int y,int* resx,int* resy)
{
    int i,addx,addy,nullx,nully,n;
    addx=addy=1;
    if( CoastOnMap(x,y) &&
        FindUnloadPosition(x,y,&nullx,&nully,LandUnitMask) ) {
        *resx=x;
        *resy=y;
        return 1;
    }
    x--;
    // The maximum distance to the coast. We have to stop somewhere...
    n=20;
    while (n--)
    {
	for( i=addy; i--; y++ ) {
            if( x>=0 && y>=0 && x<TheMap.Width && y<TheMap.Height &&
                CoastOnMap(x,y) && !UnitOnMapTile(x,y) &&
                FindUnloadPosition(x,y,&nullx,&nully,LandUnitMask) ) {
                *resx=x;
                *resy=y;
		return 1;
	    }
	}
	++addx;
	for( i=addx; i--; x++ ) {
            if( x>=0 && y>=0 && x<TheMap.Width && y<TheMap.Height &&
                CoastOnMap(x,y) && !UnitOnMapTile(x,y) &&
                FindUnloadPosition(x,y,&nullx,&nully,LandUnitMask) ) {
                *resx=x;
                *resy=y;
		return 1;
	    }
	}
	++addy;
	for( i=addy; i--; y-- ) {
            if( x>=0 && y>=0 && x<TheMap.Width && y<TheMap.Height &&
                CoastOnMap(x,y) && !UnitOnMapTile(x,y) &&
                FindUnloadPosition(x,y,&nullx,&nully,LandUnitMask) ) {
                *resx=x;
                *resy=y;
		return 1;
	    }
	}
	++addx;
	for( i=addx; i--; x-- ) {
            if( x>=0 && y>=0 && x<TheMap.Width && y<TheMap.Height &&
                CoastOnMap(x,y) && !UnitOnMapTile(x,y) &&
                FindUnloadPosition(x,y,&nullx,&nully,LandUnitMask) ) {
                *resx=x;
                *resy=y;
		return 1;
	    }
	}
	++addy;
    }
    DebugLevel0Fn("Try clicking closer to an actual coast.\n");
    return 0;
}

/**
**	Move to coast.
**
**	@param unit	Pointer to unit.
**	@return		-1 if unreachable, True if reached, False otherwise.
*/
local int MoveToCoast(Unit* unit)
{
    DebugLevel3Fn("%p\n" _C_ unit->Orders[0].Goal);
    
    switch( DoActionMove(unit) ) {	// reached end-point?
	case PF_UNREACHABLE:
	    DebugLevel2Fn("COAST NOT REACHABLE\n");
	    return -1;
	case PF_REACHED:
	    break;
	default:
	    return 0;
    }

    IfDebug(
	if( !CoastOnMap(unit->X,unit->Y) ) {
	    DebugLevel2Fn("COAST NOT REACHED\n");
	    return -1;
	}
    )

    DebugCheck( unit->Orders[0].Action!=UnitActionUnload );
    return 1;
}

/**
**	Leave the transporter.
**
**	@param unit	Pointer to unit.
*/
local void LeaveTransporter(Unit* unit)
{
    int i,stillonboard;
    Unit* goal;
    stillonboard=0;

    goal=unit->Orders[0].Goal;
    DebugLevel3Fn("Goal %p\n" _C_ goal);
    if( goal ) {
	unit->Orders[0].Goal=NoUnitP;
	if( goal->Destroyed ) {
	    DebugLevel0Fn("destroyed unit\n");
	    RefsDebugCheck( !goal->Refs );
	    if( !--goal->Refs ) {
		ReleaseUnit(goal);
	    }
	    return;
	}
	RefsDebugCheck( !goal->Refs );
	--goal->Refs;
	RefsDebugCheck( !goal->Refs );
	for( i=0; i<MAX_UNITS_ONBOARD; ++i ) {
	    if( goal==unit->OnBoard[i] ) {
		goal->X=unit->X;
		goal->Y=unit->Y;
		if( UnloadUnit(goal) ) {
		    unit->OnBoard[i]=NoUnitP;
		    unit->Value--;
		}
		break;
	    }
	}
    } else {
	for( i=0; i<MAX_UNITS_ONBOARD; ++i ) {
	    if( (goal=unit->OnBoard[i]) ) {
		goal->X=unit->X;
		goal->Y=unit->Y;
		if( UnloadUnit(goal) ) {
		    unit->OnBoard[i]=NoUnitP;
		    unit->Value--;
		} else {
                    stillonboard++;
                }
	    }
	}
    }
    if( IsOnlySelected(unit) ) {
	UpdateButtonPanel();
	MustRedraw|=RedrawPanels;
    }
    
    // We still have some units to unload, find a piece of free coast.
    if (stillonboard) {
        //  We tell it to unload at it's current position. This can't be done,
        //  so it will search for a piece of free coast nearby.
        unit->Orders[0].Action=UnitActionUnload;
        unit->Orders[0].Goal=NoUnitP;
        unit->Orders[0].X=unit->X;
        unit->Orders[0].Y=unit->Y;
        unit->SubAction=0;
        unit->Reset=0;
        unit->Wait=0;
    } else {
        unit->Wait=1;
        unit->Orders[0].Action=UnitActionStill;
        unit->SubAction=0;
    }
}

/**
**	The transporter unloads an unit.
**
**	@param unit	Pointer to unit.
*/
global void HandleActionUnload(Unit* unit)
{
    int i,x,y;

    DebugLevel3Fn("%p(%d) SubAction %d\n"
	    _C_ unit _C_ UnitNumber(unit) _C_ unit->SubAction);

    switch( unit->SubAction ) {
	//
	//	Move the transporter
	//
	case 0:
            if ( !unit->Orders[0].Goal ) {
                if ( !ClosestFreeCoast(unit->Orders[0].X,unit->Orders[0].Y,&x,&y) ) {
                    // Sorry... I give up.
		    unit->Orders[0].Action=UnitActionStill;
		    unit->SubAction=0;
                    return;
                }
                unit->Orders[0].X=x;
                unit->Orders[0].Y=y;
            }
            NewResetPath(unit);
            unit->SubAction=1;
	case 1:
            //  The Goal is the unit that we have to unload.
	    if( !unit->Orders[0].Goal ) {
                // We have to unload everything
		if( (i=MoveToCoast(unit)) ) {
		    if( i==-1 ) {
			if( ++unit->SubAction==1 ) {
			    unit->Orders[0].Action=UnitActionStill;
			    unit->SubAction=0;
			}
		    } else {
			unit->SubAction=2;
		    }
		}
		break;
	    }
	//
	//	Leave the transporter
	//
	case 2:
	    // FIXME: show still animations ?
	    LeaveTransporter(unit);
            if ( unit->Orders[0].Action!=UnitActionStill ) {
                HandleActionUnload(unit);
            }
	    break;
    }
}

//@}
