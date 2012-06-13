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

#include "ai_local.h"

#include "actions.h"
#include "map.h"
#include "missile.h"
#include "pathfinder.h"
#include "unit.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

class _EnemyOnMapTile
{
public:
	_EnemyOnMapTile(const CUnit &unit, const Vec2i _pos, CUnit **enemy) :
		source(&unit) , pos(_pos), best(enemy) {
	}

	void operator()(CUnit *const unit) const {
		const CUnitType &type = *unit->Type;
		// unusable unit ?
		// if (unit->IsUnusable()) can't attack constructions
		// FIXME: did SelectUnitsOnTile already filter this?
		// Invisible and not Visible
		if (unit->Removed || unit->Variable[INVISIBLE_INDEX].Value
			// || (!UnitVisible(unit, source->Player))
			|| unit->CurrentAction() == UnitActionDie) {
			return;
		}
		if (pos.x < unit->tilePos.x || pos.x >= unit->tilePos.x + type.TileWidth
			|| pos.y < unit->tilePos.y || pos.y >= unit->tilePos.y + type.TileHeight) {
			return;
		}
		if (!CanTarget(source->Type, &type)) {
			return;
		}
		if (!source->Player->IsEnemy(*unit)) { // a friend or neutral
			return;
		}
		// Choose the best target.
		if (!*best || (*best)->Type->Priority < type.Priority) {
			*best = unit;
		}
	}

private:
	const CUnit *const source;
	const Vec2i pos;
	CUnit **best;
};

/**
**  Choose enemy on map tile.
**
**  @param source  Unit which want to attack.
**  @param pos     position on map, tile-based.
**
**  @return        Returns ideal target on map tile.
*/
static CUnit *EnemyOnMapTile(const CUnit &source, const Vec2i &pos)
{
	CUnit *enemy = NULL;

	_EnemyOnMapTile filter(source, pos, &enemy);
	Map.Field(pos)->UnitCache.for_each(filter);
	return enemy;
}

/**
**  Mark all by transporter reachable water tiles.
**
**  @param unit    Transporter
**  @param matrix  Water matrix.
**
**  @note only works for water transporters!
*/
static void AiMarkWaterTransporter(const CUnit &unit, unsigned char *matrix)
{
	const Vec2i offset[] = {{0, -1}, { -1, 0}, {1, 0}, {0, 1}, { -1, -1}, {1, -1}, { -1, 1}, {1, 1}};
	Vec2i pos = unit.tilePos;
	const int w = Map.Info.MapWidth + 2;
	matrix += w + w + 1;
	if (matrix[pos.x + pos.y * w]) { // already marked
		DebugPrint("Done\n");
		return;
	}
	const int size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	std::vector<Vec2i> points;
	points.resize(size);

	// Make movement matrix : Ignore all possible mobile units.
	const int mask = unit.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit);
	points[0] = pos;
	int rp = 0;
	matrix[pos.x + pos.y * w] = 66; // mark start point
	int ep = 1;
	int wp = 1; // start with one point

	// Pop a point from stack, push all neightbors which could be entered.
	for (;;) {
		while (rp != ep) {
			Vec2i rpos = points[rp];
			for (int i = 0; i < 8; ++i) { // mark all neighbors
				pos = rpos + offset[i];
				unsigned char *m = matrix + pos.x + pos.y * w;
				if (*m) { // already checked
					continue;
				}
				if (CanMoveToMask(pos, mask)) { // reachable
					*m = 66;
					points[wp] = pos; // push the point
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
		// Continue with next frame.
		if (rp == wp) { // unreachable, no more points available
			break;
		}
		ep = wp;
	}
}

/**
**  Find possible targets.
**
**  @param unit    Attack.
**  @param matrix  Water matrix.
**  @param dpos    Attack point.
**  @param ds      Attack state.
**
**  @return        True if target found.
*/
static bool AiFindTarget(const CUnit &unit, unsigned char *matrix, Vec2i *dpos, int *ds)
{
	const Vec2i offset[] = {{0, -1}, { -1, 0}, {1, 0}, {0, 1}, { -1, -1}, {1, -1}, { -1, 1}, {1, 1}};
	struct p {
		Vec2i pos;
		unsigned char State;
	} *points;
	enum {
		OnWater,
		OnLand,
		OnIsle
	};
	const int size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	points = new p[size];

	Vec2i pos = unit.tilePos;

	int w = Map.Info.MapWidth + 2;
	// Ignore all possible mobile units.
	const int mask = unit.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit);

	points[0].pos = pos;
	points[0].State = OnLand;
	matrix += w + w + 1;
	int rp = 0;
	matrix[pos.x + pos.y * w] = 1; // mark start point
	int ep = 1;
	int wp = 1; // start with one point

	// Pop a point from stack, push all neightbors which could be entered.
	for (;;) {
		while (rp != ep) {
			const Vec2i rpos = points[rp].pos;
			const unsigned char state = points[rp].State;
			for (int i = 0; i < 8; ++i) { // mark all neighbors
				pos = rpos + offset[i];
				unsigned char *m = matrix + pos.x + pos.y * w;

				if (state != OnWater) {
					if (*m) { // already checked
						if (state == OnLand && *m == 66) { // tansporter?
							DebugPrint("->Water\n");
							*m = 6;
							points[wp].pos = pos; // push the point
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
					if (EnemyOnMapTile(unit, pos)) {
						DebugPrint("Target found %d,%d-%d\n" _C_ pos.x _C_ pos.y _C_ state);
						*dpos = pos;
						*ds = state;
						delete[] points;
						return 1;
					}
					if (CanMoveToMask(pos, mask)) { // reachable

						*m = 1;
						points[wp].pos = pos; // push the point
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
							points[wp].pos = pos; // push the point
							points[wp].State = OnWater;
							if (++wp >= size) { // round about
								wp = 0;
							}
						}
						continue;
					}
					if (CanMoveToMask(pos, mask)) { // reachable
						DebugPrint("->Land\n");
						*m = 1;
						points[wp].pos = pos; // push the point
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
		// Continue with next frame.
		if (rp == wp) { // unreachable, no more points available
			break;
		}
		ep = wp;
	}
	delete[] points;
	return false;
}

/**
**  Find possible walls to target.
**
**  @param force  Attack force.
**
**  @return       True if wall found.
*/
int AiFindWall(AiForce *force)
{
	const Vec2i offset[] = {{0, -1}, { -1, 1}, {1, 0}, {0, 1}, { -1, -1}, {1, -1}, { -1, 1}, {1, 1}};

	// Find a unit to use.  Best choice is a land unit with range 1.
	// Next best choice is any land unit.  Otherwise just use the first.
	CUnit *unit = force->Units[0];
	for (unsigned int i = 0; i < force->Units.size(); ++i) {
		CUnit *aiunit = force->Units[i];
		if (aiunit->Type->UnitType == UnitTypeLand) {
			unit = aiunit;
			if (aiunit->Type->Missile.Missile->Range == 1) {
				break;
			}
		}
	}
	Vec2i pos = unit->tilePos;
	const int size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	std::vector<Vec2i> points;
	points.resize(size);

	unsigned char *matrix = CreateMatrix();
	const int w = Map.Info.MapWidth + 2;
	matrix += w + w + 1;

	points[0] = pos;
	int rp = 0;
	matrix[pos.x + pos.y * w] = 1; // mark start point
	int ep = 1;
	int wp = 1; // start with one point
	const int mask = unit->Type->MovementMask;
	Vec2i dest = { -1, -1};

	// Pop a point from stack, push all neighbors which could be entered.
	for (; dest.x == -1;) {
		while (rp != ep && dest.x == -1) {
			Vec2i rpos = points[rp];
			for (int i = 0; i < 8; ++i) { // mark all neighbors
				pos = rpos + offset[i];
				unsigned char *m = matrix + pos.x + pos.y * w;
				if (*m) {
					continue;
				}
				// Check for a wall
				if (Map.WallOnMap(pos)) {
					DebugPrint("Wall found %d,%d\n" _C_ pos.x _C_ pos.y);
					dest = pos;
					break;
				}
				if (CanMoveToMask(pos, mask)) { // reachable
					*m = 1;
					points[wp] = pos; // push the point
					if (++wp >= size) { // round about
						wp = 0;
					}
				} else { // unreachable
					*m = 99;
				}
			}
			if (++rp >= size) { // round about
				rp = 0;
			}
		}
		// Continue with next frame.
		if (rp == wp) { // unreachable, no more points available
			break;
		}
		ep = wp;
	}
	if (dest.x != -1) {
		force->State = AiForceAttackingState_Waiting;
		for (unsigned int i = 0; i < force->Units.size(); ++i) {
			CUnit &aiunit = *force->Units[i];
			if (aiunit.Type->CanAttack) {
				CommandAttack(aiunit, dest, NULL, FlushCommands);
			} else {
				CommandMove(aiunit, dest, FlushCommands);
			}
		}
		return 1;
	}
	return 0;
}

/**
**  Plan an attack with a force.
**  We know, that we must use a transporter.
**
**  @return       True if target found, false otherwise.
**
**  @todo         Perfect planning.
**                Only works for water transporter!
**  @todo transporter are more selective now (flag with unittypeland).
**         We must manage it.
*/
int AiForce::PlanAttack()
{
	DebugPrint("%d: Planning for force #%lu of player #%d\n"_C_ AiPlayer->Player->Index
			   _C_(long unsigned int)(this - & (AiPlayer->Force[0])) _C_ AiPlayer->Player->Index);

	unsigned char *watermatrix = CreateMatrix();

	// Transporter must be already assigned to the force.
	// NOTE: finding free transporters was too much work for me.
	int state = 1;
	for (unsigned int i = 0; i < Size(); ++i) {
		const CUnit &aiunit = *Units[i];

		if (aiunit.Type->CanTransport()) {
			DebugPrint("%d: Transporter #%d\n" _C_ AiPlayer->Player->Index _C_ UnitNumber(aiunit));
			AiMarkWaterTransporter(aiunit, watermatrix);
			state = 0;
		}
	}

	// No transport that belongs to the force.
	CUnit *transporter = NULL;
	if (state) {
		for (int i = 0; i < AiPlayer->Player->GetUnitCount(); ++i) {
			CUnit &unit = AiPlayer->Player->GetUnit(i);

			if (unit.Type->CanTransport() && unit.IsIdle()) {
				DebugPrint("%d: Assign any transporter\n" _C_ AiPlayer->Player->Index);
				AiMarkWaterTransporter(unit, watermatrix);
				// FIXME: can be the wrong transporter.
				transporter = &unit;
				state = 0;
			}
		}
	}

	if (state) { // Absolute no transporter
		DebugPrint("%d: No transporter available\n" _C_ AiPlayer->Player->Index);
		// FIXME: should tell the resource manager we need a transporter!
		return 0;
	}

	// Find a land unit of the force.
	// FIXME: if force is split over different places -> broken
	CUnit *landUnit = CUnitTypeFinder(UnitTypeLand).Find(Units);
	if (landUnit == NULL) {
		DebugPrint("%d: No land unit in force\n" _C_ AiPlayer->Player->Index);
		return 0;
	}

	Vec2i pos = this->GoalPos;

	if (AiFindTarget(*landUnit, watermatrix, &pos, &state)) {
		if (state != 1) { // Need transporter.
			if (transporter) {
				DebugPrint("%d: Assign any transporter\n" _C_ AiPlayer->Player->Index);
				if (transporter->GroupId) {
					transporter->Player->Ai->Force[transporter->GroupId - 1].Remove(*transporter);
				}
				Insert(*transporter);
				transporter->GroupId = transporter->Player->Ai->Force.getIndex(this) + 1;
			}
			int totalBoardCapacity = 0;
			CUnit *transporterAdded = transporter;

			// Verify we have enough transporter.
			// @note: Minimal check for unitType (flyers...)
			for (unsigned int i = 0; i < Size(); ++i) {
				CUnit &aiunit = *Units[i];

				if (aiunit.Type->CanTransport()) {
					totalBoardCapacity += aiunit.Type->MaxOnBoard - aiunit.BoardCount;
					transporter = &aiunit;
				}
			}
			for (unsigned int i = 0; i < Size(); ++i) {
				CUnit &aiunit = *Units[i];

				if (CanTransport(*transporter, aiunit)) {
					totalBoardCapacity--;
				}
			}
			if (totalBoardCapacity < 0) { // Not enough transporter.
				// Add all other idle transporter.
				for (int i = 0; i < AiPlayer->Player->GetUnitCount(); ++i) {
					CUnit &aiunit = AiPlayer->Player->GetUnit(i);

					if (transporterAdded != &aiunit && aiunit.Type->CanTransport() && aiunit.IsIdle()) {
						DebugPrint("%d: Assign another transporter.\n"_C_ AiPlayer->Player->Index);
						if (aiunit.GroupId) {
							aiunit.Player->Ai->Force[aiunit.GroupId - 1].Remove(aiunit);
						}
						Insert(aiunit);
						aiunit.GroupId = aiunit.Player->Ai->Force.getIndex(this) + 1;
						totalBoardCapacity += aiunit.Type->MaxOnBoard - aiunit.BoardCount;
						if (totalBoardCapacity >= 0) {
							break;
						}
					}
				}
			}
		}
		DebugPrint("%d: Can attack\n" _C_ AiPlayer->Player->Index);
		GoalPos = pos;
		State = AiForceAttackingState_Boarding;
		return 1;
	}
	return 0;
}


static bool ChooseRandomUnexploredPositionNear(const Vec2i &center, Vec2i *pos)
{
	Assert(pos != NULL);

	int ray = 3;
	const int maxTryCount = 8;
	for (int i = 0; i != maxTryCount; ++i) {
		pos->x = center.x + SyncRand() % (2 * ray + 1) - ray;
		pos->y = center.y + SyncRand() % (2 * ray + 1) - ray;

		if (Map.Info.IsPointOnMap(*pos)
			&& Map.IsFieldExplored(*AiPlayer->Player, *pos) == false) {
			return true;
		}
		ray = 3 * ray / 2;
	}
	return false;
}

static CUnit *GetBestExplorer(const AiExplorationRequest &request, Vec2i *pos)
{
	// Choose a target, "near"
	const Vec2i &center = request.pos;
	if (ChooseRandomUnexploredPositionNear(center, pos) == false) {
		return NULL;
	}
	// We have an unexplored tile in sight (pos)

	CUnit *bestunit = NULL;
	// Find an idle unit, responding to the mask
	bool flyeronly = false;
	int bestdistance = -1;
	for (int i = 0; i != AiPlayer->Player->GetUnitCount(); ++i) {
		CUnit &unit = AiPlayer->Player->GetUnit(i);

		if (!unit.IsIdle()) {
			continue;
		}
		if (Map.Info.IsPointOnMap(unit.tilePos) == false) {
			continue;
		}
		if (unit.CanMove() == false) {
			continue;
		}
		const CUnitType &type = *unit.Type;

		if (type.UnitType != UnitTypeFly) {
			if (flyeronly) {
				continue;
			}
			if ((request.Mask & MapFieldLandUnit) && type.UnitType != UnitTypeLand) {
				continue;
			}
			if ((request.Mask & MapFieldSeaUnit) && type.UnitType != UnitTypeNaval) {
				continue;
			}
		} else {
			flyeronly = true;
		}

		const int distance = MapDistance(unit.tilePos, *pos);
		if (bestdistance == -1 || distance <= bestdistance
			|| (bestunit->Type->UnitType != UnitTypeFly && type.UnitType == UnitTypeFly)) {
			bestdistance = distance;
			bestunit = &unit;
		}
	}
	return bestunit;
}


/**
**  Respond to ExplorationRequests
*/
void AiSendExplorers()
{
	// Count requests...
	const int requestcount = AiPlayer->FirstExplorationRequest.size();

	// Nothing => abort
	if (!requestcount) {
		return;
	}
	const int maxTryCount = 5;
	for (int i = 0; i != maxTryCount; ++i) {
		// Choose a request
		const int requestid = SyncRand() % requestcount;
		const AiExplorationRequest &request = AiPlayer->FirstExplorationRequest[requestid];

		Vec2i pos;
		CUnit *bestunit = GetBestExplorer(request, &pos);
		if (bestunit != NULL) {
			CommandMove(*bestunit, pos, FlushCommands);
			AiPlayer->LastExplorationGameCycle = GameCycle;
			break;
		}
	}
	// Remove all requests
	AiPlayer->FirstExplorationRequest.clear();
}

//@}
