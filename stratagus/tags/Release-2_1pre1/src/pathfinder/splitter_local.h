//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name splitter_local.h		-	The map headerfile. */
//
//	(c) Copyright 1998-2003 by Ludovic Pollet
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
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

#ifndef __SPLITTER_LOCAL_H__
#define __SPLITTER_LOCAL_H__

/*----------------------------------------------------------------------------
--		Structures
----------------------------------------------------------------------------*/
typedef struct _region_line_ RegionSegment;

struct _region_line_{
	int						Y;				/// y pos of the line
	int 				MinX,MaxX;		/// X bounds (including)
	RegionSegment		*Prev;				/// previous in region
	RegionSegment		*Next;				/// Next in region
};

typedef struct _region_definition_{
	int						TileCount;		/// Nb of tile assigned to it
	int						MinX,MinY;		/// Upper left corner
	int						MaxX,MaxY;		/// Bottom right corner

	long				SumX,SumY;		// May limit map to ~512x512

	int						ConnectionsNumber;
	int* 				Connections;
	int*				ConnectionsCount;

	int						Color;				/// For debugging only.

	char				IsWater;		/// This region is water ?
	int						Dirty;				/// Should be checked for split & joins ?
	char				NeedConnectTest;/// Do we need to test connection for this region ?
	RegionSegment*		FirstSegment;		/// All lines. ( double linked list )
	RegionSegment*		LastSegment;
	int						Zone;				/// 8-connex tile zone
} RegionDefinition;

typedef struct {
	unsigned short int		X;
	unsigned short int		Y;
} MapPoint;

typedef struct _circular_filler_{
	int 				NextOne;
	int 				LastOne;
	int						FillValue;
	MapPoint*				Points;
	RegionId				RestrictTo;
	int						Direction;
}CircularFiller;


/*----------------------------------------------------------------------------
--		Macros
----------------------------------------------------------------------------*/
#define		MaxZone						1024
#define InMap(x,y)				(((unsigned)(x)<(unsigned)TheMap.Width)&&(((unsigned)(y)<(unsigned)TheMap.Height)))
#define MapFlag(x,y)				TheMap.Fields[x + TheMap.Width * y].Flags
#define RegionMapping(x,y) 		(RegionMappingStorage[(x) + TheMap.Width * (y)])
#define TileMappable(x,y)\
	(		(MapFlag(x,y) & (MapFieldLandAllowed | MapFieldWaterAllowed | MapFieldCoastAllowed)) &&\
		(!(MapFlag(x,y) & (MapFieldBuilding | MapFieldWall | MapFieldRocks | MapFieldForest))))
#define TilePassable(x,y)\
	((MapFlag(x,y) & (MapFieldLandUnit + MapFieldSeaUnit)) == 0)

#define TileIsWater(x,y)\
	((TheMap.Fields[x + TheMap.Width * y].Flags & (MapFieldWaterAllowed | MapFieldCoastAllowed)) != 0)



/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

/// Coordinates of adjacents cells of a cell
extern int 				adjacents[8][2];
/// Tile => region mapping
extern RegionId* 		RegionMappingStorage;
/// All regions
extern RegionDefinition		Regions[MaxRegionNumber];
/// Temporary storage
extern int* 				RegionTempStorage;
/// Number of region
extern int				RegionCount;
/// Highest region + 1
extern int				RegionMax;
/// Lowest free region ID
extern int				NextFreeRegion;
/// Zone marked dirty ?
extern int				ZoneNeedRefresh;



/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

// Region handling

/// Allocate a new region
RegionId NewRegion(int iswater);
void RegionFree(RegionId regid);

/// Split a region (when it is too big, malformed)
void RegionSplit(RegionId regid, int updateConnections);
/// Check that a region is connex and split it if necessary
void RegionCheckConnex(RegionId regid);

// Region geometry handling

/// Add a segment to a region
void RegionAppendSegment(RegionDefinition * def,int x0,int x1,int y);
/// Add an existing segment to a region
void RegionAddSegment(RegionDefinition * def,int x0,int x1,int y);
/// Remove a segment from a region
void RegionDelSegment(RegionDefinition* def, RegionSegment* seg);
/// Update Minx & Maxx values
void RegionUpdateMinMax(RegionDefinition* adef,int x,int y);
/// Assign a tile to a region
void RegionAssignTile(RegionId region,int x,int y);
/// Unassign a tile from a region
void RegionUnassignTile(RegionId region,int x,int y);
/// Find the closest point on a region with shortest horizontal distance to x
void RegionFindPointOnX(RegionDefinition * def,int x,int * vx,int * vy);
/// Find the closest point on a region with shortest vertical distance to y
void RegionFindPointOnY(RegionDefinition * def,int y,int * vx,int * vy);

// Region geometry operations

/// Allocate the space for geometric operations
void RegionTempStorageAllocate(void);
/// Free the space for geometric operations
void RegionTempStorageFree(void);
/// Fill a region with a value ( in the TempStorage area )
void RegionTempStorageFillRegion(RegionDefinition* adef,int value);
/// Mark region limits
int RegionTempStorageMarkObstacle(RegionId regid, int maxmark,int markvalue);
/// Make marked points bigger
int RegionTempStorageEmbossObstacle(RegionId regid, int maxmark,int markvalue);
/// Unassigned adjacent cell of markvalue are assigned to markvalue+1
int RegionTempStorageGrow(RegionId regid, int maxmark,int markvalue);
/// Clear all points in the region with the given value
void RegionTempStorageUnmarkPoints(RegionId regid, int markvalue);
/// Split a region according to values in the tempstorage
void RegionSplitUsingTemp(RegionId reg, int nbarea, int updateConnections);

/// Initialise a "circularfiller"
void CircularFillerInit(CircularFiller* filler, RegionId region, int startx, int starty, int value);
/// Free a "circularfiller"
void CircularFillerDone(CircularFiller * filler);
/// One step of filling
int CircularFillerStep(CircularFiller * filler);

// Region connections handling

/// Set The connection count in rega for regb to value
void RegionSetConnection(RegionId rega, RegionId regb, int value);
/// Add to the connection count between two regions
void RegionAddConnection(RegionId rega, RegionId regb,int value);
/// Add to the connection count between two regions (symetrical)
void RegionAddBidirConnection(RegionId rega, RegionId regb,int value);
/// Reduce connection for a region, when the region was reduced
void RegionRescanAdjacents(RegionId regid);
/// Adjust connection for a region, when the given cell changed
void RegionUpdateConnection(RegionId reg,int x,int y,int add,int bidir);
/// Compute all connection ( reset )
void UpdateConnections(void);

/// Ask for a zone recalculation
void ClearZoneNeedRefresh(void);

// Debugging

/// Output tons of debugging
void MapSplitterDebug(void);
/// Verify connexions
void RegionDebugAllConnexions(void);
/// Check coherence between regions and map
void RegionDebugWater(void);


#endif // __SPLITTER_LOCAL_H__
