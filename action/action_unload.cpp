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
/**@name action_unload.c		-	The unload action. */
//
//	(c) Copyright 1998,2000-2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
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

/**
**	Reappear unit on map.
**
**	@param unit	Unit to drop out.
**	@param heading	Direction in which the unit should appear.
**	@param addx	Tile size in x.
**	@param addy	Tile size in y.
**
**	@return		True if unit can be unloaded.
*/
global int UnloadUnit(Unit* unit,int addx,int addy)
{
    int x;
    int y;
    int i;
    int mask;
    int n;

    DebugCheck( !unit->Removed );

    x=unit->X;
    y=unit->Y;
    mask=UnitMovementMask(unit);

    --x;
    // FIXME: don't search outside of the map
    for( n=0; n<2; ++n ) {
	for( i=addy; i--; y++ ) {
#ifdef NEW_SHIPS
	    if( unit->Type->UnitType!=UnitTypeLand && ((x&1) || (y&1)) ) {
		continue;
	    }
#endif
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		goto found;
	    }
	}
	++addx;
	for( i=addx; i--; x++ ) {
#ifdef NEW_SHIPS
	    if( unit->Type->UnitType!=UnitTypeLand && ((x&1) || (y&1)) ) {
		continue;
	    }
#endif
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		goto found;
	    }
	}
	++addy;
	for( i=addy; i--; y-- ) {
#ifdef NEW_SHIPS
	    if( unit->Type->UnitType!=UnitTypeLand && ((x&1) || (y&1)) ) {
		continue;
	    }
#endif
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		goto found;
	    }
	}
	++addx;
	for( i=addx; i--; x-- ) {
#ifdef NEW_SHIPS
	    if( unit->Type->UnitType!=UnitTypeLand && ((x&1) || (y&1)) ) {
		continue;
	    }
#endif
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		goto found;
	    }
	}
	++addy;
    }
    return 0;

found:
    unit->X=x;
    unit->Y=y;

    unit->Wait=1;		// should be correct unit has still action

    // FIXME: Should I use PlaceUnit here?
    UnitCacheInsert(unit);
    // FIXME: This only works with 1x1 big units
    DebugCheck( unit->Type->TileWidth!=1 || unit->Type->TileHeight!=1 );
    TheMap.Fields[x+y*TheMap.Width].Flags|=UnitFieldFlags(unit);

    unit->Removed=0;

    x+=unit->Type->TileWidth/2;
    y+=unit->Type->TileHeight/2;
#ifdef NEW_FOW
    //
    //	Update fog of war.
    //
    MapMarkSight(unit->Player,x,y,unit->Stats->SightRange);
#else
    //
    //	Update fog of war, if unit belongs to player on this computer
    //
    if( unit->Player==ThisPlayer ) {
	MapMarkSight(x,y,unit->Stats->SightRange);
    }
#endif
    if( unit->Type->CanSeeSubmarine ) {
	MarkSubmarineSeen(unit->Player,x,y,unit->Stats->SightRange);
    }

    MustRedraw|=RedrawMinimap;
    CheckUnitToBeDrawn(unit);

    return 1;
}

/**
**	Move to coast.
**
**	@param unit	Pointer to unit.
**	@return		-1 if unreachable, True if reached, False otherwise.
*/
local int MoveToCoast(Unit* unit)
{
    DebugLevel3Fn("%p\n",unit->Command.Data.Move.Goal);

    switch( DoActionMove(unit) ) {	// reached end-point?
	case PF_UNREACHABLE:
	    DebugLevel2Fn("COAST NOT REACHED\n");
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
    int i;
    Unit* goal;

    goal=unit->Orders[0].Goal;
    DebugLevel3Fn("Goal %p\n",goal);
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
		if( UnloadUnit(goal,
			unit->Type->TileWidth,unit->Type->TileHeight) ) {
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
		if( UnloadUnit(goal,
			unit->Type->TileWidth,unit->Type->TileHeight) ) {
		    unit->OnBoard[i]=NoUnitP;
		    unit->Value--;
		}
	    }
	}
    }
    if( IsOnlySelected(unit) ) {
	UpdateButtonPanel();
	MustRedraw|=RedrawPanels;
    }
    unit->Wait=1;
    unit->Orders[0].Action=UnitActionStill;
    unit->SubAction=0;
}

/**
**	The transporter unloads an unit.
**
**	@param unit	Pointer to unit.
*/
global void HandleActionUnload(Unit* unit)
{
    int i;

    DebugLevel3Fn("%p(%d) SubAction %d\n"
	    ,unit,UnitNumber(unit),unit->SubAction);

    switch( unit->SubAction ) {
	//
	//	Move the transporter
	//
	case 0:
	    NewResetPath(unit);
	    unit->SubAction=1;
	    // FALL THROUGH
	case 1:
	    if( !unit->Orders[0].Goal ) {
		// NOTE: the Move clears the goal!!
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
	    break;
    }
}

//@}
