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
/**@name pathfinder.h	-	The path finder headerfile. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

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
*/
enum _move_return_ {
    PF_UNREACHABLE=-2,			/// Unreachable stop
    PF_REACHED=-1,			/// Reached goal stop
    PF_WAIT=0,				/// Wait, no time or blocked
    PF_MOVE=1,				/// On the way moving
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern unsigned char Matrix[(MaxMapWidth+1)*(MaxMapHeight+1)];  /// Path matrix

extern int AStarOn; /// are we using a* or the old path finder
extern int AStarFixedUnitCrossingCost; /// cost associated to move on a tile
                                       /// occupied by a fixed unit
extern int AStarMovingUnitCrossingCost; /// cost associated to move on a tile
                                        /// occupied by a moving unit


/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    ///
extern unsigned char* CreateMatrix(void);
    ///
extern int NewPath(Unit* unit,int* xdp,int* ydp);
    ///
extern int PlaceReachable(const Unit* unit,int x,int y);
    ///
extern int UnitReachable(const Unit* unit,const Unit* dest);

//
//	in astar.c
//
    /// Returns the next element of the path
extern int NextPathElement(Unit*,int* xdp,int* ydp);

    /// Init the a* data structures
extern void InitAStar(void);

    /// free the a* data structures
extern void FreeAStar(void);

//
//	in ccl_pathfinder.c
//
    /// register ccl features
extern void PathfinderCclRegister(void);

//@}

#endif	// !__PATH_FINDER_H__
