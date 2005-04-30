//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name splitter_lowlevel.c - Low level funcs for Regions.  */
//
//      (c) Copyright 1999-2005 by Ludovic Pollet
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
//      $Id$

#ifdef MAP_REGIONS

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stratagus.h"
#include "player.h"
#include "unit.h"
#include "map.h"
#include "util.h"

#include "pathfinder.h"

#include "splitter_local.h"

/**
** Remove a segment from a region
**
** @param def    The RegionDefinition structure
** @param seg    The segment to remove
*/
void RegionDelSegment(RegionDefinition* def, RegionSegment* seg)
{
	if (seg->Next) {
		seg->Next->Prev = seg->Prev;
	} else {
		def->LastSegment = seg->Prev;
	}

	if (seg->Prev) {
		seg->Prev->Next = seg->Next;
	} else {
		def->FirstSegment = seg->Next;
	}

	free(seg);
}

/**
** Add a segment to a region
**
** @param def    The RegionDefinition structure
** @param seg    The segment to add
*/
void RegionAddSegment(RegionDefinition* def,int x0,int x1,int y)
{
	RegionSegment* seg;
	seg = malloc(sizeof(RegionSegment));

	seg->Y = y;
	seg->MinX = x0;
	seg->MaxX = x1;
	seg->Next = def->FirstSegment;
	seg->Prev = 0;

	// insert at the beginning
	if (def->FirstSegment) {
		def->FirstSegment->Prev = seg;
	} else {
		def->LastSegment = seg;
	}
	def->FirstSegment = seg;
}

/**
** Add a segment to a region. Eventually, existing segment are collapsed
**
** @param def    The RegionDefinition structure
** @param x0     Minimum x of the segment
** @param x1     Maximum x of the segment
** @param y      Y coord of the segment
*/
void RegionAppendSegment(RegionDefinition* def, int x0, int x1, int y)
{
	RegionSegment* seg;
	seg = def->FirstSegment;
	while (seg) {
		if (seg->Y == y && seg->MaxX + 1 == x0) {
			seg->MaxX = x1;
			return;
		}
		if (seg->Y == y && seg->MinX - 1 == x1) {
			seg->MinX = x0;
			return;
		}
		seg = seg->Next;
	}
	RegionAddSegment(def, x0, x1, y);
}

/**
** Update min & max in a region
**
** @param def    The RegionDefinition structure
** @param x      X coord of a cell
** @param y      Y coord of a cell
*/
void RegionUpdateMinMax(RegionDefinition* adef,int x,int y)
{
	if (adef->TileCount == 0) {
		adef->MinX = x;
		adef->MaxX = x;
		adef->MinY = y;
		adef->MaxY = y;
	} else {
		if (x < adef->MinX) {
			adef->MinX = x;
		}
		if (x > adef->MaxX) {
			adef->MaxX = x;
		}
		if (y < adef->MinY) {
			adef->MinY = y;
		}
		if (y > adef->MaxY) {
			adef->MaxY = y;
		}
	}
}


/**
** Find A point one a region, closest to a given vertical line (x)
**
** @param def    The RegionDefinition structure
** @param x      x coord of the vertical line
** @param vx     X result value
** @param vy     Y result value
*/
void RegionFindPointOnX(RegionDefinition* def,int x,int * vx,int * vy)
{
	RegionSegment *cur;
	int bestx, besty, bestxdelta, bestydelta;
	int xdelta, ydelta;

	int curx, cury;
	int idealy;

	bestx = -1;
	besty = -1;
	bestxdelta = 0x7fffffff;
	bestydelta = 0x7fffffff;

	idealy = (def->MinY + def->MaxY) / 2;

	cur = def->FirstSegment;

	while (cur) {
		cury = cur->Y;
		for (curx = cur->MinX; curx <= cur->MaxX; curx++) {
			if (RegionTempStorage[curx + TheMap.Info.MapWidth * cury]) {
				continue;
			}
			xdelta = abs(curx - x);
			ydelta = abs(idealy - cury);

			if ((xdelta < bestxdelta) || (xdelta == bestxdelta && ydelta < bestydelta)) {
				bestx = curx;
				besty = cury;
				bestxdelta = xdelta;
				bestydelta = ydelta;
			}
		}

		cur = cur->Next;
	}

	// Check one was found !
	Assert(besty != -1);

	*vx = bestx;
	*vy = besty;
}

/**
** Find A point one a region, closest to a given horizontal line (y)
**
** @param def    The RegionDefinition structure
** @param x      y coord of the horizontal line
** @param vx     X result value
** @param vy     Y result value
*/
void RegionFindPointOnY(RegionDefinition* def,int y,int * vx,int * vy)
{
	RegionSegment *cur;
	int bestx, besty, bestxdelta, bestydelta;
	int xdelta, ydelta;

	int curx, cury;
	int idealx;

	bestx = -1;
	besty = -1;
	bestxdelta = 0x7fffffff;
	bestydelta = 0x7fffffff;

	idealx = (def->MinX + def->MaxX) / 2;

	cur = def->FirstSegment;

	while (cur) {
		cury = cur->Y;
		for (curx = cur->MinX; curx <= cur->MaxX; curx++) {
			if (RegionTempStorage[curx + TheMap.Info.MapWidth * cury]) {
				continue;
			}

			xdelta = abs(curx - idealx);
			ydelta = abs(cury - y);

			if ((ydelta < bestydelta) || (ydelta == bestydelta && xdelta < bestxdelta)) {
				bestx = curx;
				besty = cury;
				bestxdelta = xdelta;
				bestydelta = ydelta;
			}
		}
		cur = cur->Next;
	}

	// Check one was found !
	Assert(besty != -1);

	*vx = bestx;
	*vy = besty;
}

/**
** Allocate the temp storage area
*/
void RegionTempStorageAllocate(void)
{
	RegionTempStorage = malloc(TheMap.Info.MapWidth * TheMap.Info.MapHeight * sizeof(int));
}

/**
** Free the temp storage area
*/
void RegionTempStorageFree(void)
{
	free(RegionTempStorage);
}

/**
** Fill a region in the temp storage area
**
** @param adef     The region definition
** @param value    Value to fill with
*/
void RegionTempStorageFillRegion(RegionDefinition* adef,int value)
{
	RegionSegment* cur;
	int * segstart;
	int i;

	cur = adef->FirstSegment;

	while (cur) {
		segstart = RegionTempStorage + cur->MinX + cur->Y * TheMap.Info.MapWidth;
		i = cur->MaxX - cur->MinX + 1;
		while(i) {
			*(segstart++) = value;
			i--;
		}

		cur = cur->Next;
	}
}

/**
** Unmark points of a regions in the temp storage area
**
** @param regid       The regiond ID
** @param markvalue   Cells with value == markvalue will become 0
*/
void RegionTempStorageUnmarkPoints(RegionId regid, int markvalue)
{
	RegionDefinition * adef;
	RegionSegment * seg;
	int * ptr;
	int x,y;

	adef = Regions + regid;
	seg = adef->FirstSegment;

	while (seg) {
		y = seg->Y;
		ptr = RegionTempStorage + seg->MinX + y * TheMap.Info.MapWidth;
		for (x = seg->MinX; x <= seg->MaxX; x++) {
			if (*ptr == markvalue) {
				*ptr = 0;
			}
			ptr++;
		}
		seg = seg->Next;
	}
}

/**
** Mark some points of a region in the temp storage area
**
** @param regid      The regiond ID
** @param points     MapPoint array
** @param nbpoint    MapPoint array size
** @param maxmak     Maximum number of point to mark
** @param markvalue  Points get marked with this value
** @return The number of points marked
*/
static int RegionTempStorageMarkPoints(RegionId regid, MapPoint* points, int nbpoints, int maxmark, int markvalue)
{
	int id;
	int rslt;

	if (nbpoints > maxmark) {
		rslt = maxmark;

		// Mark points in random order
		// FIXME : could avoid syncrand cost here
		while (maxmark) {
			id = SyncRand() % nbpoints;

			RegionTempStorage[points[id].X + TheMap.Info.MapWidth * points[id].Y] = markvalue;

			nbpoints--;
			maxmark--;
			points[id].X = points[nbpoints].X;
			points[id].Y = points[nbpoints].Y;
		}
	} else {
		rslt = nbpoints;

		// Mark all points
		for (id = 0; id < nbpoints; ++id) {
			RegionTempStorage[points[id].X + TheMap.Info.MapWidth * points[id].Y] = markvalue;
		}
	}
	return rslt;
}

/**
** Mark limits of a regions in the temp storage area
**
** @param regid      The regiond ID
** @param maxmak     Maximum number of point to mark
** @param markvalue  Points get marked with this value
*/
int RegionTempStorageMarkObstacle(RegionId regid, int maxmark,int markvalue)
{
	RegionDefinition * adef;
	RegionSegment * seg;
	RegionId oppregion;
	int x, y, i, tx, ty;
	int region_ok[8];
	int lasti;
	int obstacle;

	int markednb;
	MapPoint * marked;


	if (maxmark <= 0) {
		return 0;
	}

	adef = Regions + regid;

	marked = malloc(sizeof(MapPoint) * adef->TileCount);
	markednb = 0;

	seg = adef->FirstSegment;

	while (seg) {
		y = seg->Y;

		for (x = seg->MinX; x <= seg->MaxX; x++) {
			if (RegionTempStorage[x + y * TheMap.Info.MapWidth]) {
				continue;
			}

			// Mark in region_ok, obstacles tiles
			for (i = 0; i < 8; i++) {
				tx = x + adjacents[i][0];
				ty = y + adjacents[i][1];

				// Out = don't pass
				if (tx < adef->MinX || tx > adef->MaxX ||
					ty < adef->MinY || ty > adef->MaxY) {
					region_ok[i] = 1;
					continue;
				}

				oppregion = RegionMapping(tx, ty);
				// Same region => pass
				if (oppregion == regid) {
					region_ok[i] = 1;
					continue;
				}

				// Not mapped = don't pass
				if (oppregion == NoRegion) {
					region_ok[i] = 1;
					continue;
				}

				// Other region ...
				//if (Regions[oppregion].IsWater != Regions[regid].IsWater) {
					region_ok[i] = 1;
				//}
			}

			// Count obstacle when travelling throught adjacents
			lasti = region_ok[7];
			obstacle = 0;
			for (i = 0; i < 8; ++i) {
				if (lasti != region_ok[i]) {
					obstacle++;
				}
				lasti = region_ok[i];
			}

			if (obstacle < 4) {
				continue;
			}

			// Mark the obstacle
			marked[markednb].X = x;
			marked[markednb].Y = y;
			markednb++;
		}
		seg = seg->Next;
	}

	obstacle = RegionTempStorageMarkPoints(regid, marked, markednb, maxmark, markvalue);
	free(marked);
	return obstacle;
}

/**
** Make marked points bigger ( mark their adjacent with markvalue + 1 )
** Multiple invoquation lead to bigger marks
**
** @param regid       The regiond ID
** @param maxmak      Maximum number of point to mark
** @param markvalue   Points marked with this value will be "grown"
*/
int RegionTempStorageEmbossObstacle(RegionId regid, int maxmark,int markvalue)
{
	int markednb;
	RegionDefinition* adef;
	RegionSegment* seg;
	MapPoint* marked;
	int x,y;
	int tx,ty;
	int i;
	int mustmark;
	int obstacle;

	adef = Regions + regid;
	seg = adef->FirstSegment;

	marked = malloc(sizeof(MapPoint) * adef->TileCount);
	markednb = 0;

	while (seg) {
		y = seg->Y;

		for (x = seg->MinX; x <= seg->MaxX; x++) {
			if (RegionTempStorage[x + y * TheMap.Info.MapWidth]) {
				continue;
			}

			mustmark = 0;
			for (i = 0; i < 8; i++) {
				tx = x + adjacents[i][0];
				ty = y + adjacents[i][1];
				if (!InMap(tx, ty)) {
					continue;
				}

				if (RegionTempStorage[tx + ty * TheMap.Info.MapHeight] == markvalue) {
					mustmark = 1;
					break;
				}
			}
			if (!mustmark) {
				continue;
			}

			marked[markednb].X = x;
			marked[markednb].Y = y;
			markednb++;
		}
			seg = seg->Next;
	}

	obstacle = RegionTempStorageMarkPoints(regid, marked, markednb, maxmark, markvalue+1);
	free(marked);
	return obstacle;
}

/**
** Set the connection count in rega for regb to value
**
** @param rega    The region to change connection value
** @param regb    Other region in the connection
** @param value   The new connection count between the two regions
*/
void RegionSetConnection(RegionId rega, RegionId regb, int value)
{
	RegionDefinition* adef;
	int j;

	adef = Regions + rega;

	for (j = 0; j < adef->ConnectionsNumber; j++) {
		if (adef->Connections[j] == regb) {
			if (value) {
					adef->ConnectionsCount[j] = value;
			} else {
				// Remove
				ZoneNeedRefresh = 1;

				adef->ConnectionsNumber--;
				adef->Connections[j] = adef->Connections[adef->ConnectionsNumber];
				adef->ConnectionsCount[j] = adef->ConnectionsCount[adef->ConnectionsNumber];

				adef->Connections = realloc(adef->Connections, sizeof(int) * adef->ConnectionsNumber);
				adef->ConnectionsCount = realloc(adef->ConnectionsCount, sizeof(int) * adef->ConnectionsNumber);
			}
			return;
		}
	}
	// Not found
	if (!value) {
		return;
	}

	ZoneNeedRefresh = 1;

	adef->ConnectionsNumber++;
	adef->Connections = realloc(adef->Connections, sizeof(int) * adef->ConnectionsNumber);
	adef->ConnectionsCount = realloc(adef->ConnectionsCount, sizeof(int) * adef->ConnectionsNumber);

	adef->Connections[adef->ConnectionsNumber - 1] = regb;
	adef->ConnectionsCount[adef->ConnectionsNumber - 1] = value;
}

/**
** Add to the connection count in rega for regb to value
**
** @param rega    The region to change connection value
** @param regb    Other region in the connection
** @param value   The value to add to the connection count between the two regions
*/
void RegionAddConnection(RegionId rega, RegionId regb,int value)
{
	int j;
	RegionDefinition * adef;

	Assert(rega != regb);

	adef = Regions + rega;

	for (j = 0; j < adef->ConnectionsNumber; j++) {
		if (adef->Connections[j] == regb) {
			adef->ConnectionsCount[j] += value;
			if (!adef->ConnectionsCount[j]) {
				ZoneNeedRefresh = 1;

				adef->ConnectionsNumber--;
				adef->Connections[j] = adef->Connections[adef->ConnectionsNumber];
				adef->ConnectionsCount[j] = adef->ConnectionsCount[adef->ConnectionsNumber];
				adef->Connections = realloc(adef->Connections,
					sizeof(int) * (adef->ConnectionsNumber + 1));
				adef->ConnectionsCount = realloc(adef->ConnectionsCount,
					sizeof(int) * (adef->ConnectionsNumber + 1));
			}
			return;
		}
	}

	Assert(value > 0);

	adef->Connections = realloc(adef->Connections, sizeof(int) * (adef->ConnectionsNumber + 1));
	adef->ConnectionsCount = realloc(adef->ConnectionsCount, sizeof(int) * (adef->ConnectionsNumber + 1));
	adef->Connections[adef->ConnectionsNumber] = regb;
	adef->ConnectionsCount[adef->ConnectionsNumber] = value;
	adef->ConnectionsNumber++;

	ZoneNeedRefresh = 1;
}

/**
** Add a value to a connection count between two regions (symetrical)
**
** @param rega    On of the region to change connection value
** @param regb    Other region in the connection
** @param value   The value to add to the connection count between the two regions
*/
void RegionAddBidirConnection(RegionId rega, RegionId regb,int value)
{
	RegionAddConnection(rega, regb, value);
	RegionAddConnection(regb, rega, value);
}

/**
**  Initialise a "CircularFiller", which fill points in the temp storage area.
**
** @param filler    pointer to a CircularFiller structure
** @param region    region to fill
** @param startx    start point x coord
** @param starty    start point y coord
** @param value     value it will fill with
*/
void CircularFillerInit(CircularFiller* filler, RegionId region, int startx, int starty, int value)
{
	filler->NextOne = 0;
	filler->LastOne = 0;
	filler->Points = malloc(sizeof(MapPoint) * Regions[region].TileCount);
	filler->Points[0].X = startx;
	filler->Points[0].Y = starty;
	filler->FillValue = value;
	filler->RestrictTo = region;
	filler->Direction = 0;

	RegionTempStorage[startx + TheMap.Info.MapWidth * starty] = value;
}

/**
** Free a CircularFiller private structures
**
** @param filler the filler to free
*/
void CircularFillerDone(CircularFiller * filler)
{
	free(filler->Points);
}

/**
** Mark one point with a circularefiller
**
** @param filler the filler
** @return  1 if one point marked, 0 else
*/
int CircularFillerStep(CircularFiller * filler)
{
	int fillx,filly;
	int adjx,adjy;
	int ptid, try;

	while (filler->LastOne >= filler->NextOne) {
		ptid = filler->NextOne;

		fillx = filler->Points[ptid].X;
		filly = filler->Points[ptid].Y;

		try = 0;
		while (try < 8) {
			adjx = fillx + adjacents[filler->Direction & 7][0];
			adjy = filly + adjacents[filler->Direction & 7][1];

			filler->Direction++;
			try++;

			if (!InMap(adjx, adjy)) {
				continue;
			}

			if (RegionMapping(adjx, adjy) != filler->RestrictTo) {
				continue;
			}

			if (RegionTempStorage[adjx + TheMap.Info.MapWidth * adjy] != 0) {
				continue;
			}

			RegionTempStorage[adjx + TheMap.Info.MapWidth * adjy] = filler->FillValue;

			filler->LastOne++;

			filler->Points[filler->LastOne].X = adjx;
			filler->Points[filler->LastOne].Y = adjy;

			return 1;
		}
			filler->NextOne++;
	}
	return 0;
}

/**
** Rescan a region to update its connections
**
** @param regid the region to scan
*/
void RegionRescanAdjacents(RegionId regid)
{
	static int Connected[MaxRegionNumber];
	static int LastId=0;

	int ConnectionsCount[MaxRegionNumber];
	int Connections[MaxRegionNumber];
	int ConnectionNumber;

	RegionSegment * seg;
	RegionId adjreg;
	int x,y,ax,ay,i,adj;
	RegionDefinition* adef = Regions + regid;

	LastId++;
	if (LastId == 1) {
		for (i = 0; i < MaxRegionNumber; i++) {
			Connected[i] = 1;
		}
		LastId++;
	}

	ConnectionNumber = 0;

	seg = adef->FirstSegment;

	while (seg) {
		y = seg->Y;

		for (x = seg->MinX; x <= seg->MaxX; x++) {
			for (adj = 0; adj < 8; adj++) {
				ax = x + adjacents[adj][0];
				ay = y + adjacents[adj][1];

				if (!InMap(ax, ay)) {
					continue;
				}

				adjreg = RegionMapping(ax, ay);
				if (adjreg == NoRegion || adjreg == regid) {
					continue;
				}

				if (Connected[adjreg] != LastId) {
					Connected[adjreg] = LastId;
					Connections[ConnectionNumber++] = adjreg;
					ConnectionsCount[adjreg] = 0;
				}
				ConnectionsCount[adjreg]++;
			}
		}

		seg = seg->Next;
	}

	// Broke connections with other cells.
	for (i = 0; i < adef->ConnectionsNumber; i++) {
		adjreg = adef->Connections[i];
		// Not connected anymore to it, remove.
		if (Connected[adjreg] != LastId) {
			RegionSetConnection(adjreg, regid, 0);
		}
	}

	// Set connections for region
	if (adef->ConnectionsNumber) {
		free(adef->Connections);
		free(adef->ConnectionsCount);
	}

	adef->Connections = 0;
	adef->ConnectionsCount = 0;
	adef->ConnectionsNumber = 0;
	for (i = 0; i < ConnectionNumber; i++) {
		adjreg = Connections[i];
		RegionSetConnection(regid, adjreg, ConnectionsCount[adjreg]);
		RegionSetConnection(adjreg, regid, ConnectionsCount[adjreg]);
	}
}

/**
**  Adjust the connections of the given region, when taking cell x,y into account
** This function updates connections and eventually mark the region for a connex check
**
** @param reg      the region
** @param x        X coord of the cell
** @param y        Y coord of the cell
** @param add      1 if cell is added to the region, 0 else
** @param bidir    Operate in both directions
*/
void RegionUpdateConnection(RegionId reg, int x, int y, int add, int bidir)
{
	int adj;
	int ax, ay;
	RegionId adjreg;

	for (adj = 0; adj < 8; adj++) {
			ax = x + adjacents[adj][0];
		ay = y + adjacents[adj][1];

		if (!InMap(ax, ay)) {
			continue;
		}

		adjreg = RegionMapping(ax, ay);
		if (adjreg == NoRegion || adjreg == reg) {
			continue;
		}

		if (bidir) {
			RegionAddBidirConnection(reg, adjreg, add);
		} else {
			RegionAddConnection(reg, adjreg, add);
		}
	}
}

//@}

#endif // MAP_REGIONS

