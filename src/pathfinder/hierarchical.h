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
/**@name hierarchical.h	-	The hierarchical pathfinder header file. */
//
//      (c) Copyright 2002 by Latimerius.
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
//	 $Id$

#ifndef __HIERARCHICAL_H__
#define __HIERARCHICAL_H__

#include "types.h"
#include "region.h"

#define MOVEMENT_IMPORTANT_FLAGS  (MapFieldLandAllowed | \
			MapFieldCoastAllowed| MapFieldWaterAllowed | \
			MapFieldUnpassable | MapFieldBuilding)

typedef enum _node_operation_ {
    SET_REGID,
	GET_BEST,
	MARK_BEST
} NodeOperation;

typedef enum _area_neighborship_type_ {
	AREAS_NONCONNECTED,
	AREAS_8CONNECTED,
	AREAS_4CONNECTED
} AreaNeighborshipType;

extern int PfHierShowRegIds;
extern int PfHierShowGroupIds;

extern inline int AreaGetWidth (void);
extern inline int AreaGetHeight (void);
extern int AreaNeighborship (AreaCoords * , AreaCoords * );
extern void AreaSetNumRegions (int , int , int );
extern int AreaGetRegions (int , int , Region *** );
extern void AreaAddRegion (int , int , Region * );
extern int AreaMapWidth (void);
extern int AreaMapHeight (void);
extern void NodePerformOperation (int , int , NodeOperation , int []);
extern int NodeGetAreaOffset (int , int );

/* FIXME check out whether HIERARCHICAL_BACKEND is still needed */
#ifndef HIERARCHICAL_BACKEND

#include "unit.h"

extern int PfHierInitialize (void);
extern void PfHierClean (void);
extern int PfHierComputePath (Unit * , int * , int * );
extern void PfHierMapChangedCallback (int , int , int , int );
extern void PfHierReleaseData (Unit * );

#endif /* HIERARCHICAL_BACKEND */

#endif	// __HIERARCHICAL_H__
