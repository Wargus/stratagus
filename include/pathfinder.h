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
/**@name pathfinder.h	-	The path finder headerfile. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer,Russell Smith
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

#ifndef	__PATH_FINDER_H__
#define	__PATH_FINDER_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "unit.h"
#include "map.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Result codes of the pathfinder.
**
**	@todo
**		Another idea is	SINT_MAX as reached, SINT_MIN as unreachable
**		stop others how far to goal.
*/
enum _move_return_ {
    MUST_REACH_GOAL=-4,
    BEST_GOAL=-5,
    PF_FAILED=-3,			/// This Pathfinder failed, try another
    PF_UNREACHABLE=-2,			/// Unreachable stop
    PF_REACHED=-1,			/// Reached goal stop
    PF_WAIT=0,				/// Wait, no time or blocked
    PF_MOVE=1,				/// On the way moving
};

/**
**	To remove pathfinder internals. Called if path destination changed.
*/
#define NewResetPath(unit) \
    do { unit->Data.Move.Fast=1; unit->Data.Move.Length=0; }while( 0 )

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /// Path matrix
extern unsigned char Matrix[(MaxMapWidth+2)*(MaxMapHeight+3)+2];
    /// are we using A* or the old path finder
extern int AStarOn;
    /// cost associated to move on a tile occupied by a fixed unit
extern int AStarFixedUnitCrossingCost;
    /// cost associated to move on a tile occupied by a moving unit
extern int AStarMovingUnitCrossingCost;

//
//  Convert heading into direction.
//                            //  N NE  E SE  S SW  W NW
extern const int Heading2X[8];
extern const int Heading2Y[8];
extern const int XY2Heading[3][3];

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Create a matrix for the old pathfinder
extern unsigned char* CreateMatrix(void);
    /// Allocate a new matrix and initialize
extern unsigned char* MakeMatrix(void);
    /// Get next element of the way to goal.
extern int NewPath(Unit* unit);
    /// Return distance to place.
extern int PlaceReachable(Unit* unit,int x,int y,int range);
    /// Return distance to unit.
extern int UnitReachable(Unit* unit,const Unit* dest,int range);

//
//	in astar.c
//
    /// Returns the next element of the path
extern int NextPathElement(Unit*,int* xdp,int* ydp);

    /// Init the a* data structures
extern void InitAStar(void);

    /// free the a* data structures
extern void FreeAStar(void);

    /// Find and a* path for a unit
extern int AStarFindPath(Unit* unit, int x1, int y1, int x2, int y2, int best,char* path);
//
//	in ccl_pathfinder.c
//
    /// register ccl features
extern void PathfinderCclRegister(void);

//@}

#ifdef HIERARCHIC_PATHFINDER
#include "../pathfinder/hierarchical.h"
#endif /* HIERARCHIC_PATHFINDER */

#endif	// !__PATH_FINDER_H__
