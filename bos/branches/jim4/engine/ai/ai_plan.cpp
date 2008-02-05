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
//      (c) Copyright 2002-2005 by Lutz Sammer and Jimmy Salmon
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
	for (i = 0; i < n; ++i) {
		unit = table[i];
		// unusable unit ?
		// if (unit->IsUnusable()) can't attack constructions
		// FIXME: did SelectUnitsOnTile already filter this?
		if (unit->Removed ||
				//(!UnitVisible(unit, source->Player)) ||
				unit->Orders[0]->Action == UnitActionDie) {
			continue;
		}
		type = unit->Type;
		if (tx < unit->X || tx >= unit->X + type->TileWidth ||
				ty < unit->Y || ty >= unit->Y + type->TileHeight) {
			continue;
		}
		if (!CanTarget(source->Type, unit->Type)) {
			continue;
		}
		if (!source->Player->IsEnemy(unit)) { // a friend or neutral
			continue;
		}
		//
		// Choose the best target.
		//
		if (!best || best->Type->Priority < unit->Type->Priority) {
			best = unit;
		}
	}
	return best;
}

/**
**  Mark all by transporter reachable water tiles.
**
**  @param unit    Transporter
**  @param matrix  Water matrix.
**
**  @note only works for water transporters!
*/
static void AiMarkWaterTransporter(const CUnit *unit, unsigned char *matrix)
{
	static const int xoffset[] = { 0, -1, +1, 0, -1, +1, -1, +1 };
	static const int yoffset[] = { -1, 0, 0, +1, -1, -1, +1, +1 };
	struct p {
		unsigned short X;
		unsigned short Y;
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
	unsigned char *m;

	x = unit->X;
	y = unit->Y;
	w = Map.Info.MapWidth + 2;
	matrix += w + w + 2;
	if (matrix[x + y * w]) { // already marked
		DebugPrint("Done\n");
		return;
	}

	size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	points = new p[size];

	//
	// Make movement matrix.
	//
	mask = unit->Type->MovementMask;
	// Ignore all possible mobile units.
	mask &= ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit);

	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	matrix[x + y * w] = 66; // mark start point
	ep = wp = 1; // start with one point

	//
	// Pop a point from stack, push all neightbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			for (i = 0; i < 8; ++i) { // mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;
				if (*m) { // already checked
					continue;
				}

				if (CanMoveToMask(x, y, mask)) { // reachable
					*m = 66;
					points[wp].X = x; // push the point
					points[wp].Y = y;
					if (++wp >= size) { // round about
						wp = 0;
					}
					/* Must be checked multiple
					   } else { // unreachable
					   *m=99;
					 */
				}
			}

			if (++rp >= size) { // round about
				rp = 0;
			}
		}

		//
		// Continue with next frame.
		//
		if (rp == wp) { // unreachable, no more points available
			break;
		}
		ep = wp;
	}

	delete[] points;
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
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			state = points[rp].State;
			for (i = 0; i < 8; ++i) { // mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;

				if (state != OnWater) {
					if (*m) { // already checked
						if (state == OnLand && *m == 66) { // tansporter?
							DebugPrint("->Water\n");
							*m = 6;
							points[wp].X = x; // push the point
							points[wp].Y = y;
							points[wp].State = OnWater;
							if (++wp >= size) { // round about
								wp = 0;
							}
						}
						continue;
					}
					// Check targets on tile?
					// FIXME: the move code didn't likes a shore building as
					//  target
					if (EnemyOnMapTile(unit, x, y)) {
						DebugPrint("Target found %d,%d-%d\n" _C_ x _C_ y _C_ state);
						*dx = x;
						*dy = y;
						*ds = state;
						delete[] points;
						return 1;
					}

					if (CanMoveToMask(x, y, mask)) { // reachable

						*m = 1;
						points[wp].X = x; // push the point
						points[wp].Y = y;
						points[wp].State = state;
						if (++wp >= size) { // round about
							wp = 0;
						}
					} else { // unreachable
						*m = 99;
					}
				} else { // On water
					if (*m) { // already checked
						if (*m == 66) { // tansporter?
							*m = 6;
							points[wp].X = x; // push the point
							points[wp].Y = y;
							points[wp].State = OnWater;
							if (++wp >= size) { // round about
								wp = 0;
							}
						}
						continue;
					}
					if (CanMoveToMask(x, y, mask)) { // reachable
						DebugPrint("->Land\n");
						*m = 1;
						points[wp].X = x; // push the point
						points[wp].Y = y;
						points[wp].State = OnIsle;
						if (++wp >= size) { // round about
							wp = 0;
						}
					} else { // unreachable
						*m = 99;
					}
				}
			}

			if (++rp >= size) { // round about
				rp = 0;
			}
		}

		//
		// Continue with next frame.
		//
		if (rp == wp) { // unreachable, no more points available
			break;
		}
		ep = wp;
	}
	delete[] points;
	return 0;
}

/**
**  Plan an attack with a force.
**  We know, that we must use a transporter.
**
**  @param force  Pointer on the force.
**
**  @return       True if target found, false otherwise.
**
**  @todo         Perfect planning.
**                Only works for water transporter!
**  @todo transporter are more selective now (flag with unittypeland).
**         We must manage it.
*/
int AiPlanAttack(AiForce *force)
{
	unsigned char *watermatrix;
	const CUnit *aiunit = NULL;
	int x;
	int y;
	int i;
	int state;
	CUnit *transporter;

	DebugPrint("Planning for force #%lu of player #%d\n" _C_
		static_cast<long unsigned int> (force - AiPlayer->Force) _C_ AiPlayer->Player->Index);

	watermatrix = CreateMatrix();

	//
	// Transporter must be already assigned to the force.
	// NOTE: finding free transportes was too much work for me.
	//
	state = 1;
	for (i = 0; i < (int)force->Units.size(); ++i) {
		aiunit = force->Units[i];
		if (aiunit->Type->CanTransport) {
			DebugPrint("Transporter #%d\n" _C_ UnitNumber(aiunit));
			AiMarkWaterTransporter(aiunit, watermatrix);
			state = 0;
		}
	}

	//
	// No transport that belongs to the force.
	//
	transporter = NULL;
	if (state) {
		for (i = 0; i < AiPlayer->Player->TotalNumUnits; ++i) {
			CUnit *unit;

			unit = AiPlayer->Player->Units[i];
			if (unit->Type->CanTransport && unit->IsIdle()) {
				DebugPrint("Assign any transporter\n");
				AiMarkWaterTransporter(unit, watermatrix);
				// FIXME: can be the wrong transporter.
				transporter = unit;
				state = 0;
			}
		}
	}

	if (state) { // Absolute no transporter
		DebugPrint("No transporter available\n");
		// FIXME: should tell the resource manager we need a transporter!
		return 0;
	}
	//
	// Find a land unit of the force.
	// FIXME: if force is split over different places -> broken
	//
	for (i = 0; i < (int)force->Units.size(); ++i) {
		aiunit = force->Units[i];
		if (aiunit->Type->UnitType == UnitTypeLand) {
			DebugPrint("Land unit %d\n" _C_ UnitNumber(aiunit));
			break;
		}
	}
	if (i == (int)force->Units.size()) {
		DebugPrint("No land unit in force\n");
		return 0;
	}

	if (AiFindTarget(aiunit, watermatrix, &x, &y, &state)) {
		if (transporter) {
			force->Units.insert(force->Units.begin(), transporter);
			transporter->RefsIncrease();
		}

		DebugPrint("Can attack\n");
		force->GoalX = x;
		force->GoalY = y;
		force->MustTransport = state == 2;

		force->State = 1;
		return 1;
	}
	return 0;
}

/**
**  Respond to ExplorationRequests
*/
void AiSendExplorers(void)
{
	AiExplorationRequest *request;
	int requestcount;
	int requestid;
	int centerx;
	int centery;
	int x;
	int y;
	int i;
	int targetok;
	int ray;
	int trycount;
	int outtrycount;

	CUnit **unit;
	CUnitType *type;
	CUnit *bestunit;
	int distance;
	int bestdistance;
	int flyeronly;

	// Count requests...
	requestcount = AiPlayer->FirstExplorationRequest.size();

	// Nothing => abort
	if (!requestcount) {
		return;
	}

	outtrycount = 0;
	do {
		bestunit = 0;
		++outtrycount;

		// Choose a request
		requestid = SyncRand() % requestcount;
		request = &AiPlayer->FirstExplorationRequest[requestid];

		// Choose a target, "near"
		centerx = request->X;
		centery = request->Y;
		ray = 3;
		trycount = 0;

		do {
			targetok = 0;

			x = centerx + SyncRand() % (2 * ray + 1) - ray;
			y = centery + SyncRand() % (2 * ray + 1) - ray;

			if (x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight) {
				targetok = !Map.IsFieldExplored(AiPlayer->Player, x, y);
			}

			ray = 3 * ray / 2;
			++trycount;
		} while (trycount < 8 && !targetok);

		if (!targetok) {
			continue;
		}

		// We have an unexplored tile in sight (x,y)

		// Find an idle unit, responding to the mask
		flyeronly = 0;
		bestdistance = -1;

		unit = AiPlayer->Player->Units;
		for (i = AiPlayer->Player->TotalNumUnits; i > 0; ++unit) {
			--i;

			if (!(*unit)->IsIdle()) {
				continue;
			}

			if ((*unit)->X == -1 || (*unit)->Y == -1) {
				continue;
			}

			type = (*unit)->Type;

			if (!CanMove(*unit)) {
				continue;
			}

			if (type->UnitType != UnitTypeFly) {
				if (flyeronly) {
					continue;
				}
				if ((request->Mask & MapFieldLandUnit) && type->UnitType != UnitTypeLand) {
					continue;
				}
				if ((request->Mask & MapFieldSeaUnit) && type->UnitType != UnitTypeNaval) {
					continue;
				}
			} else {
				flyeronly = 1;
			}

			distance = ((*unit)->X - x) * ((*unit)->X - x) + ((*unit)->Y - y) * ((*unit)->Y - y);
			if (bestdistance == -1 || distance <= bestdistance ||
					(bestunit->Type->UnitType != UnitTypeFly && type->UnitType == UnitTypeFly)) {
				bestdistance = distance;
				bestunit = (*unit);
			}
		}
	} while (outtrycount <= 4 && !bestunit);

	if (bestunit) {
		CommandMove(bestunit, x, y, FlushCommands);
		AiPlayer->LastExplorationGameCycle = GameCycle;
	}

	// Remove all requests
	AiPlayer->FirstExplorationRequest.clear();
}

//@}
