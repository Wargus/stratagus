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
/**@name splitter_zoneset.c	-	Manipulation of zone set. 	*/
//
//	(c) Copyright 1999-2003 by Ludovic Pollet
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

//@{

#ifdef MAP_REGIONS

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stratagus.h"
#include "player.h"
#include "unit.h"
#include "map.h"

#include "pathfinder.h"

#include "splitter_local.h"


/**
**		Clear a ZoneSet structure (make it ready for usage)
**		Only first call eats CPU cycle.
**
**		@param m		pointer to a ZoneSet structure
*/
global void ZoneSetClear(ZoneSet* m)
{
	int i;

	ClearZoneNeedRefresh();

	if (m->Id == 0) {
		for (i = 0; i < MaxZoneNumber; i++) {
			m->Marks[i] = 1;
		}
		m->Id++;
	}

	m->ZoneCount = 0;
	m->Id++;
}

/**
**		Add a zone to a ZoneSet structure
**
**		@param m		pointer to a ZoneSet structure
**		@param zone		zone to add
**		@return 		1 if the zone is new in the set
*/
global int ZoneSetAddZone(ZoneSet * m, int zone)
{
	if (m->Marks[zone] == m->Id) {
		return 0;
	}
	m->Marks[zone] = m->Id;
	m->Zones[m->ZoneCount++] = zone;
	return 1;
}

/**
**		Make a union of two ZoneSet
**
**		@param dst		ZoneSet which will be modifier
**		@param src		ZoneSet which will be added into dst
*/
global void ZoneSetAddSet(ZoneSet* dst, ZoneSet* src)
{
	int i;

	for (i = 0; i < src->ZoneCount; i++) {
		ZoneSetAddZone(dst, src->Zones[i]);
	}
}

/**
**		Check if two ZoneSet have at least one common zone
**
**		@param dst		pointer to a ZoneSet structure
**		@param src		pointer to a ZoneSet structure
**		@return				1 if a common zone was found, 0 else
*/
global int ZoneSetHasIntersect(ZoneSet* dst, ZoneSet* src)
{
	int i;
	for (i = 0; i < dst->ZoneCount; i++) {
		if (src->Marks[dst->Zones[i]] == src->Id) {
			return 1;
		}
	}
	return 0;
}

/**
**		Compute the intersection of two ZoneSet structure
**
**		@param dst		pointer to the ZoneSet which will hold the result
**		@param src		other ZoneSet in the operation
*/
global void ZoneSetIntersect(ZoneSet* dst, ZoneSet* src)
{
	int i;
	int newzonecount;

	newzonecount = dst->ZoneCount;
	i = 0;
	while (i < newzonecount) {
		if (src->Marks[dst->Zones[i]] == src->Id) {
			// Keep it
			i++;
		} else {
			// Remove i in dst
			dst->Zones[i] = dst->Zones[newzonecount - 1];
			newzonecount--;
		}
	}

	ZoneSetClear(dst);
	for (i = 0;i < newzonecount; i++) {
		ZoneSetAddZone(dst, dst->Zones[i]);
	}
}

/**
**		Add the zone of a map cell into a ZoneSet
**
**		@param zs		pointer to a ZoneSet structure
**		@param x		X coord of the map cell
**		@param y		Y coord of the map cell
*/
global void ZoneSetAddCell(ZoneSet * zs,int x,int y)
{
	RegionId region;

	region = RegionMapping(x, y);

	if (region == NoRegion) {
		return;
	}

	ZoneSetAddZone(zs, Regions[region].Zone);
}

/**
**		Check if a ZoneSet contains a given zone
**
**		@param zs		pointer to the ZoneSet
**		@param zone		the zone
**		@return 		1 if zs contains the zone, 0 else
*/
global int ZoneSetContains(ZoneSet * zs,int zone)
{
	return zs->Marks[zone] == zs->Id;
}

/**
**		Add a rectangle (not filled) of tile, if they are accessible regarding the mask
**
**		@param zs		pointer to a ZoneSet
**		@param x0		Rectangle coordinate
**		@param y0		Rectangle coordinate
**		@param x1		Rectangle coordinate
**		@param y1		Rectangle coordinate
**		@param range		Range around the rectangle
**		@param mask		Mask to check cell for
*/
local void ZoneSetAddPassableRange(ZoneSet * zs,int x0,int y0,int x1,int y1,int range,int mask)
{
	static int turn[5][2]={{1,0},{0,1},{-1,0},{0,-1}};
	int x,y;
	int dir;

	x0 -= range;
	y0 -= range;
	x1 += range;
	y1 += range;

	x = x0;
	y = y0;
	dir = -1;
	do {
		if (InMap(x, y) && !(TheMap.Fields[x + y * TheMap.Width].Flags & mask)) {
			ZoneSetAddCell(zs, x, y);
		}

		if ((x == x0 || x == x1) && (y == y0 || y == y1)) {
			dir++;
		}
		x += turn[dir][0];
			y += turn[dir][1];
	} while (dir < 4);
}

/**
**		Add the zone(s) accessible by an unit
**
**		@param source		pointer to a ZoneSet
**		@param src		pointer to an unit
*/
global void ZoneSetAddUnitZones(ZoneSet * source,Unit * src)
{
	int unitmask;
	int start_x0, start_y0, start_x1, start_y1;
	int x,y;

	start_x0 = src->X;
	start_y0 = src->Y;
	start_x1 = start_x0 + src->Type->TileWidth - 1;
	start_y1 = start_y0 + src->Type->TileWidth - 1;

	unitmask = UnitMovementMask(src) & (~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit));

	// For start zone, use zone under unit + zone
	for (y = start_y0; y <= start_y1; y++) {
		for (x = start_x0; x <= start_x1; x++) {
			if (!InMap(x, y)) {
				continue;
			}
			ZoneSetAddCell(source, x, y);
		}
	}

	// Add the passable things around the unit
	ZoneSetAddPassableRange(source, start_x0, start_y0, start_x1, start_y1, 1, unitmask);
}

/**
**		Add a unit's goal to a zoneset. The goal is checked with the unit's movement mask
**
**		@param dest		pointer to a ZoneSet
**		@param src		pointer to an unit
**		@param goal_x		coordinate of the goal
**		@param goal_y		coordinate of the goal
**		@param w		width in cell of the goal
**		@param h		height in cell of the goal
**		@param minrange		minrange to the goal
**		@param maxrange		maxrange to the goal
*/
global void ZoneSetAddGoalZones(ZoneSet* dest,Unit* src, int goal_x, int goal_y,int w,int h,int minrange,int maxrange)
{
	int goal_x0, goal_y0, goal_x1, goal_y1;
	int unitmask;
	int x,y;
	int i;

	if (w < 1) {
		w = 1;
	}

	if (h < 1) {
		h = 1;
	}

	unitmask = UnitMovementMask(src) & (~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit));

	goal_x0 = goal_x;
	goal_y0 = goal_y;
	goal_x1 = goal_x0 + w - 1;
	goal_y1 = goal_y0 + h - 1;


	if (minrange == 0) {
		// For goal zone, check mask as well
		for (y = goal_y0; y <= goal_y1; y++) {
			for (x = goal_x0; x <= goal_x1; x++) {
				if (InMap(x, y) && !(TheMap.Fields[x+y*TheMap.Width].Flags & unitmask)) {
					ZoneSetAddCell(dest, x, y);
				}
			}
		}
	}

	// Add range as well
	for (i = (minrange ? minrange : 1); i <= maxrange; i++) {
		ZoneSetAddPassableRange(dest, goal_x0, goal_y0, goal_x1, goal_y1, i, unitmask);
	}
}

/**
**		Add To a ZoneSet zone which can be reached in one step from zone.
**		Only Water-to-Land connexion are taken into account.
**
**		@param dst		pointer to a ZoneSet
**		@param zone		the zone which adjacent are searched
*/
local void ZoneSetAddZoneAdjacents(ZoneSet * dst, int zone)
{
	int i, j;
	int adj;
	RegionId other;

	for (i = 0; i < RegionMax; i++) {
		if (Regions[i].Zone == zone) {
			for (j = 0; j < Regions[i].ConnectionsNumber; j++) {
				other = Regions[i].Connections[j];
				adj = Regions[other].Zone;
				if (adj != -1 && adj != zone) {
					ZoneSetAddZone(dst, adj);
				}
			}
		}
	}
}

/**
**		Add to a ZoneSet all zone which are directly connected zones
**
**		@param dst		ZoneSet to add zones to
**		@param src		Zones for which connected zones are searched
*/
global void ZoneSetAddConnected(ZoneSet* dst, ZoneSet * src)
{
	int i;
	for (i = 0; i < src->ZoneCount; i++) {
		ZoneSetAddZoneAdjacents(dst, src->Zones[i]);
	}
}

/**
**		Find a path throught zones (for transporters, ...)
**
**		@param src		Starting zones
**		@param dst		Destination zones
**		@param path		will hold the zones
**		@param pathlen		will hold the path length
*/
global int ZoneSetFindPath(ZoneSet* src,ZoneSet* dst,int * path,int * pathlen)
{
	static ZoneSet current={0};
	static ZoneSet newzones={0};
	int zonedst[MaxZoneNumber];
	int zonenext[MaxZoneNumber];
	int i, j, curdst;
	int zone, newzone;
	int shouldcontinue;
	int startzone,bestdst;

	ZoneSetClear(&current);
	for (i = 0; i < dst->ZoneCount; i++) {
		zonedst[dst->Zones[i]] = 0;
		zonenext[dst->Zones[i]] = -1;
		ZoneSetAddZone(&current, dst->Zones[i]);
	}

	curdst = 0;

	while (!ZoneSetHasIntersect(src,&current)) {
		shouldcontinue = 0;
		for (i = 0; i < current.ZoneCount; i++) {
			zone = current.Zones[i];
			if (zonedst[zone] != curdst) {
				continue;
			}

			ZoneSetClear(&newzones);
			ZoneSetAddZoneAdjacents(&newzones, zone);
			for (j = 0; j < newzones.ZoneCount; j++) {
				newzone = newzones.Zones[j];
				if (!ZoneSetAddZone(&current, newzone)) {
					continue;
				}
				zonedst[newzone] = curdst + 1;
				zonenext[newzone] = zone;
				shouldcontinue = 1;
			}
		}
		curdst++;
		if (!shouldcontinue) {
			DebugLevel3Fn("No path between zones.\n");
			return 0;
		}
	}

	bestdst = -1;
	startzone = -1;
	for (i = 0; i < src->ZoneCount; i++) {
		if (bestdst == -1 || zonedst[src->Zones[i]] < bestdst) {
			startzone = src->Zones[i];
			bestdst = zonedst[startzone];
		}
	}

	DebugCheck(bestdst == -1);
	*pathlen = 0;
	do {
		// Add startzone
		path[(*pathlen)++] = startzone;
		if (ZoneSetContains(dst, startzone)) {
			return 1;
		}

		startzone = zonenext[startzone];
		DebugCheck(startzone == -1);
	} while(1);
}

#endif		// MAP_REGIONS
//@}

