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
//
//	(c) Copyright 1998-2001 by Lutz Sammer
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
*/
enum _move_return_ {
    PF_UNREACHABLE=-2,			/// Unreachable stop
    PF_REACHED=-1,			/// Reached goal stop
    PF_WAIT=0,				/// Wait, no time or blocked
    PF_MOVE=1,				/// On the way moving
};

/**
**	To remove pathfinder internals. Called if path destination changed.
*/
#ifdef NEW_ORDERS
#define ResetPath(command) // Hope I didn't need this?

#define NewResetPath(unit) \
    do { unit->Data.Move.Fast=1; unit->Data.Move.Length=0; }while( 0 )

#else

#define ResetPath(command) ((command).Data.Move.Fast=1)

#endif

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern unsigned char Matrix[(MaxMapWidth+2)*(MaxMapHeight+2)];  /// Path matrix

extern int AStarOn; /// are we using a* or the old path finder
extern int AStarFixedUnitCrossingCost; /// cost associated to move on a tile
                                       /// occupied by a fixed unit
extern int AStarMovingUnitCrossingCost; /// cost associated to move on a tile
                                        /// occupied by a moving unit


/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Create a matrix for the old pathfinder
extern unsigned char* CreateMatrix(void);
    /// Get next element of the way to goal.
extern int NewPath(Unit* unit,int* xdp,int* ydp);
    /// Return distance to place.
extern int PlaceReachable(const Unit* unit,int x,int y,int range);
    /// Return distance to unit.
extern int UnitReachable(const Unit* unit,const Unit* dest,int range);

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
