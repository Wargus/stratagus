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

typedef enum _node_operation_ {
    SET_REGID
} NodeOperation;

struct _coordinates_ {
	short X, Y;
};

typedef struct _coordinates_ AreaCoords;
typedef struct _coordinates_ FieldCoords;

typedef enum _area_neighborship_type_ {
	AREAS_NONCONNECTED,
	AREAS_8CONNECTED,
	AREAS_4CONNECTED
} AreaNeighborshipType;

extern int AreaGetWidth (void);
extern int AreaGetHeight (void);
extern int AreaNeighborship (AreaCoords * , AreaCoords * );
extern void NodePerformOperation (int , int , NodeOperation , int []);
extern int NodeGetAreaOffset (int , int );

#ifndef HIERARCHICAL_BACKEND

#include "unit.h"

extern int PfHierInitialize (void);
extern int PfHierComputePath (Unit * , int * , int * );

#endif /* HIERARCHICAL_BACKEND */

#endif	// __HIERARCHICAL_H__
