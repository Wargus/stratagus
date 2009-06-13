//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name ai_plan.cpp - AI planning functions. */
//
//      (c) Copyright 2002-2009 by Lutz Sammer and Jimmy Salmon
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "missile.h"
#include "unittype.h"
#include "unit_cache.h"
#include "map.h"
#include "pathfinder.h"
#include "actions.h"
#include "ai_local.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Choose enemy on map tile.
**
**  @param source  Unit which want to attack.
**  @param tx      X position on map, tile-based.
**  @param ty      Y position on map, tile-based.
**
**  @return        Returns ideal target on map tile.
*/
static CUnit *EnemyOnMapTile(const CUnit *source, int tx, int ty)
{
	CUnit *table[UnitMax];
	CUnit *unit;
	CUnit *best;
	const CUnitType *type;
	int n;
	int i;

	n = UnitCache.Select(tx, ty, table, UnitMax);
	best = NoUnitP;

	for (i = 0; i < n; ++i)
	{
		unit = table[i];
		// unusable unit ?
		// if (unit->IsUnusable()) can't attack constructions
		// FIXME: did SelectUnitsOnTile already filter this?
		if (unit->Removed ||
			//(!UnitVisible(unit, source->Player)) ||
			unit->Orders[0]->Action == UnitActionDie)
		{
			continue;
		}
		type = unit->Type;
		if (tx < unit->X || tx >= unit->X + type->TileWidth ||
			ty < unit->Y || ty >= unit->Y + type->TileHeight)
		{
			continue;
		}
		if (!CanTarget(source->Type, unit->Type))
		{
			continue;
		}
		if (!source->Player->IsEnemy(unit)) { // a friend or neutral
			continue;
		}
		//
		// Choose the best target.
		//
		if (!best || best->Type->Priority < unit->Type->Priority)
		{
			best = unit;
		}
	}
	return best;
}

/**
**  Find possible targets.
**
**  @param unit    Attack.
**  @param matrix  Water matrix.
**  @param dx      Attack point X.
**  @param dy      Attack point Y.
**  @param ds      Attack state.
**
**  @return        True if target found.
*/
static int AiFindTarget(const CUnit *unit, unsigned char *matrix, int *dx, int *dy,
	int *ds)
{
	static const int xoffset[] = { 0, -1, +1, 0, -1, +1, -1, +1 };
	static const int yoffset[] = { -1, 0, 0, +1, -1, -1, +1, +1 };
	struct p {
		unsigned short X;
		unsigned short Y;
		unsigned char State;
	} *points;
	int size;
	int x;
	int y;
	int rx;
	int ry;
	int mask;
	int wp;
	int rp;
	int ep;
	int i;
	int w;
	enum {
		OnWater,
		OnLand,
		OnIsle
	};
	unsigned char state;
	unsigned char *m;

	size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	points = new p[size];

	x = unit->X;
	y = unit->Y;

	w = Map.Info.MapWidth + 2;
	mask = unit->Type->MovementMask;
	// Ignore all possible mobile units.
	mask &= ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit);

	points[0].X = x;
	points[0].Y = y;
	points[0].State = OnLand;
	matrix += w + w + 2;
	rp = 0;
	matrix[x + y * w] = 1; // mark start point
	ep = wp = 1; // start with one point

	//
	// Pop a point from stack, push all neightbors which could be entered.
	//
	for (;;)
	{
		while (rp != ep)
		{
			rx = points[rp].X;
			ry = points[rp].Y;
			state = points[rp].State;
			for (i = 0; i < 8; ++i)
			{
				// mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;

				if (state != OnWater)
				{
					if (*m)
					{
						// already checked
						if (state == OnLand && *m == 66)
						{
							// tansporter?
							DebugPrint("->Water\n");
							*m = 6;
							points[wp].X = x; // push the point
							points[wp].Y = y;
							points[wp].State = OnWater;
							if (++wp >= size)
							{
								// round about
								wp = 0;
							}
						}
						continue;
					}
					// Check targets on tile?
					// FIXME: the move code didn't likes a shore building as
					//  target
					if (EnemyOnMapTile(unit, x, y))
					{
						DebugPrint("Target found %d,%d-%d\n" _C_ x _C_ y _C_ state);
						*dx = x;
						*dy = y;
						*ds = state;
						delete[] points;
						return 1;
					}

					if (CanMoveToMask(x, y, mask))
					{
						// reachable
						*m = 1;
						points[wp].X = x; // push the point
						points[wp].Y = y;
						points[wp].State = state;
						if (++wp >= size) { // round about
							wp = 0;
						}
					}
					else
					{
						// unreachable
						*m = 99;
					}
				}
				else
				{
					// On water
					if (*m)
					{
						// already checked
						if (*m == 66)
						{
							// tansporter?
							*m = 6;
							points[wp].X = x; // push the point
							points[wp].Y = y;
							points[wp].State = OnWater;
							if (++wp >= size)
							{
								// round about
								wp = 0;
							}
						}
						continue;
					}
					if (CanMoveToMask(x, y, mask))
					{
						// reachable
						DebugPrint("->Land\n");
						*m = 1;
						points[wp].X = x; // push the point
						points[wp].Y = y;
						points[wp].State = OnIsle;
						if (++wp >= size)
						{
							// round about
							wp = 0;
						}
					}
					else
					{
						// unreachable
						*m = 99;
					}
				}
			}

			if (++rp >= size)
			{
				// round about
				rp = 0;
			}
		}

		//
		// Continue with next frame.
		//
		if (rp == wp)
		{
			// unreachable, no more points available
			break;
		}
		ep = wp;
	}
	delete[] points;
	return 0;
}

//@}
