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
/**@name unit.h - The unit headerfile. */
//
//      (c) Copyright 1998-2015 by Lutz Sammer, Jimmy Salmon, Joris Dauphin
//		and Andrettin
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

#ifndef __UNIT_FIND_H__
#define __UNIT_FIND_H__

//@{

#include "map.h"
#include "pathfinder.h"
#include "unit.h"
#include "unittype.h"

#include <optional>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

//
//  Some predicates
//

class CUnitFilter
{
public:
	bool operator()(const CUnit *) const { return true; }
};

class NoFilter : public CUnitFilter
{
public:
	bool operator()(const CUnit *) const { return true; }
};

class HasSameTypeAs : public CUnitFilter
{
public:
	explicit HasSameTypeAs(const CUnitType &_type) : type(&_type) {}
	bool operator()(const CUnit *unit) const { return unit->Type == type; }
private:
	const CUnitType *type;
};

class HasSamePlayerAs : public CUnitFilter
{
public:
	explicit HasSamePlayerAs(const CPlayer &_player) : player(&_player) {}
	bool operator()(const CUnit *unit) const { return unit->Player == player; }
private:
	const CPlayer *player;
};

class HasNotSamePlayerAs : public CUnitFilter
{
public:
	explicit HasNotSamePlayerAs(const CPlayer &_player) : player(&_player) {}
	bool operator()(const CUnit *unit) const { return unit->Player != player; }
private:
	const CPlayer *player;
};

class IsAlliedWith : public CUnitFilter
{
public:
	explicit IsAlliedWith(const CPlayer &_player) : player(&_player) {}
	bool operator()(const CUnit *unit) const { return unit->IsAllied(*player); }
private:
	const CPlayer *player;
};

class IsEnemyWith : public CUnitFilter
{
public:
	explicit IsEnemyWith(const CPlayer &_player) : player(&_player) {}
	bool operator()(const CUnit *unit) const { return unit->IsEnemy(*player); }
private:
	const CPlayer *player;
};

class HasSamePlayerAndTypeAs : public CUnitFilter
{
public:
	explicit HasSamePlayerAndTypeAs(const CUnit &unit) :
		player(unit.Player), type(unit.Type)
	{}
	HasSamePlayerAndTypeAs(const CPlayer &_player, const CUnitType &_type) :
		player(&_player), type(&_type)
	{}

	bool operator()(const CUnit *unit) const
	{
		return (unit->Player == player && unit->Type == type);
	}

private:
	const CPlayer *player;
	const CUnitType *type;
};

class IsNotTheSameUnitAs : public CUnitFilter
{
public:
	explicit IsNotTheSameUnitAs(const CUnit &unit) : forbidden(&unit) {}
	bool operator()(const CUnit *unit) const { return unit != forbidden; }
private:
	const CUnit *forbidden;
};

class IsBuildingType : public CUnitFilter
{
public:
	bool operator()(const CUnit *unit) const { return unit->Type->Building; }
};

class IsSameMovementType : public CUnitFilter
{
public:
	explicit IsSameMovementType(const CUnit &unit) : Mask(unit.Type->MovementMask) {}
	bool operator()(const CUnit *unit) const { return unit->Type->MovementMask == Mask; }
private:
	const unsigned Mask;
};

class IsAggresiveUnit : public CUnitFilter
{
public:
	bool operator()(const CUnit *unit) const { return unit->IsAgressive(); }
};

class OutOfMinRange : public CUnitFilter
{
public:
	explicit OutOfMinRange(const int range, const Vec2i pos) : range(range), pos(pos) {}
	bool operator()(const CUnit *unit) const { return unit->MapDistanceTo(pos) >= range; }
private:
	int range;
	Vec2i pos;
};


template <typename Pred>
class NotPredicate : public CUnitFilter
{
public:
	explicit NotPredicate(Pred _pred) : pred(_pred) {}
	bool operator()(const CUnit *unit) const { return pred(unit) == false; }
private:
	Pred pred;
};

template <typename Pred>
NotPredicate<Pred> MakeNotPredicate(Pred pred) { return NotPredicate<Pred>(pred); }

template <typename Pred1, typename Pred2>
class AndPredicate : public CUnitFilter
{
public:
	AndPredicate(Pred1 _pred1, Pred2 _pred2) : pred1(_pred1), pred2(_pred2) {}
	bool operator()(const CUnit *unit) const { return pred1(unit) && pred2(unit); }
private:
	Pred1 pred1;
	Pred2 pred2;
};

template <typename Pred1, typename Pred2>
AndPredicate<Pred1, Pred2> MakeAndPredicate(Pred1 pred1, Pred2 pred2) { return AndPredicate<Pred1, Pred2>(pred1, pred2); }


//unit_find
class CUnitTypeFinder
{
public:
	explicit CUnitTypeFinder(const std::optional<EMovement> t) : moveType(t) {}
	bool operator()(const CUnit *const unit) const
	{
		const CUnitType &type = *unit->Type;
		if (type.BoolFlag[VANISHES_INDEX].value || (moveType != std::nullopt && type.MoveType != moveType)) {
			return false;
		}
		return true;
	}
private:
	const std::optional<EMovement> moveType;
};


class UnitFinder
{
public:
	UnitFinder(const CPlayer &player, const std::vector<CUnit *> &units, int maxDist, int movemask, CUnit **unitP) :
		player(player), units(units), maxDist(maxDist), movemask(movemask), unitP(unitP) {}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	CUnit *FindUnitAtPos(const Vec2i &pos) const;
private:
	const CPlayer &player;
	const std::vector<CUnit *> &units;
	int maxDist;
	int movemask;
	CUnit **unitP;
};

std::vector<CUnit *> Select(const Vec2i &ltPos, const Vec2i &rbPos);
std::vector<CUnit *> SelectFixed(const Vec2i &ltPos, const Vec2i &rbPos);
std::vector<CUnit *> SelectAroundUnit(const CUnit &unit, int range);

template <int selectMax = 0, typename Pred>
std::vector<CUnit *> SelectFixed(const Vec2i &ltPos, const Vec2i &rbPos, Pred pred)
{
	Assert(Map.Info.IsPointOnMap(ltPos));
	Assert(Map.Info.IsPointOnMap(rbPos));

	std::vector<CUnit *> units;
	units.reserve(selectMax << 1);
	int max = selectMax ? selectMax : INT_MAX;

	for (Vec2i posIt = ltPos; posIt.y != rbPos.y + 1; ++posIt.y) {
		for (posIt.x = ltPos.x; posIt.x != rbPos.x + 1; ++posIt.x) {
			const CMapField &mf = *Map.Field(posIt);

			for (CUnit *unit : mf.UnitCache) {
				if ((selectMax == 1 || unit->CacheLock == 0) && pred(unit)) {
					if constexpr (selectMax == 1) {
						return {unit};
					} else {
						unit->CacheLock = 1;
						units.push_back(unit);
						if (--max == 0) {
							break;
						}
					}
				}
			}
		}
	}
	for (auto* unit : units) {
		unit->CacheLock = 0;
	}
	return units;
}

template <int selectMax = 0, typename Pred>
std::vector<CUnit *> Select(const Vec2i &ltPos, const Vec2i &rbPos, Pred pred)
{
	Vec2i minPos = ltPos;
	Vec2i maxPos = rbPos;

	Map.FixSelectionArea(minPos, maxPos);
	return SelectFixed<selectMax>(minPos, maxPos, pred);
}

template <int selectMax = 0, typename Pred>
std::vector<CUnit *> SelectAroundUnit(const CUnit &unit, int range, Pred pred)
{
	const Vec2i offset(range, range);
	const Vec2i typeSize(unit.Type->TileWidth - 1, unit.Type->TileHeight - 1);

	return Select<selectMax>(unit.tilePos - offset,
	                         unit.tilePos + typeSize + offset,
	                         MakeAndPredicate(IsNotTheSameUnitAs(unit), pred));
}

template <typename Pred>
CUnit *FindUnit_IfFixed(const Vec2i &ltPos, const Vec2i &rbPos, Pred pred)
{
	Assert(Map.Info.IsPointOnMap(ltPos));
	Assert(Map.Info.IsPointOnMap(rbPos));

	for (Vec2i posIt = ltPos; posIt.y != rbPos.y + 1; ++posIt.y) {
		for (posIt.x = ltPos.x; posIt.x != rbPos.x + 1; ++posIt.x) {
			const CMapField &mf = *Map.Field(posIt);
			const auto &units = mf.UnitCache;

			const auto it = ranges::find_if(units, pred);
			if (it != units.end()) {
				return *it;
			}
		}
	}
	return nullptr;
}

template <typename Pred>
CUnit *FindUnit_If(const Vec2i &ltPos, const Vec2i &rbPos, Pred pred)
{
	Vec2i minPos = ltPos;
	Vec2i maxPos = rbPos;

	Map.FixSelectionArea(minPos, maxPos);
	return FindUnit_IfFixed(minPos, maxPos, pred);
}

/// Find resource
extern CUnit *UnitFindResource(const CUnit &unit, const CUnit &startUnit, int range,
							   int resource, bool check_usage = false, const CUnit *deposit = nullptr);

/// Find nearest deposit
extern CUnit *FindDeposit(const CUnit &unit, int range, int resource);
/// Find the next idle worker
extern CUnit *FindIdleWorker(const CPlayer &player, const CUnit *last);

/// Find the neareast piece of terrain with specific flags.
extern std::optional<Vec2i>
FindTerrainType(int movemask, int resmask, int range, const CPlayer &player, const Vec2i &startPos);

extern std::vector<CUnit *> FindUnitsByType(const CUnitType &type, bool everybody = false);

/// Find all units of this type of the player
extern std::vector<CUnit *> FindPlayerUnitsByType(const CPlayer &player, const CUnitType &type, bool ai_active = false);
/// Return any unit on that map tile
extern CUnit *UnitOnMapTile(const Vec2i &pos, std::optional<EMovement> type);
/// Return possible attack target on that map area
extern CUnit *TargetOnMap(const CUnit &unit, const Vec2i &pos1, const Vec2i &pos2);

/// Return resource, if on map tile
extern CUnit *ResourceOnMap(const Vec2i &pos, int resource, bool mine_on_top = true);
/// Return resource deposit, if on map tile
extern CUnit *ResourceDepositOnMap(const Vec2i &pos, int resource);

/// Check map for obstacles in a line between 2 tiles
extern bool CheckObstaclesBetweenTiles(const Vec2i &unitPos, const Vec2i &goalPos, unsigned short flags, int *distance = nullptr);
/// Find best enemy in numeric range to attack
extern CUnit *AttackUnitsInDistance(const CUnit &unit, int range, CUnitFilter pred);
extern CUnit *AttackUnitsInDistance(const CUnit &unit, int range);
/// Find best enemy in attack range to attack
extern CUnit *AttackUnitsInRange(const CUnit &unit, CUnitFilter pred);
extern CUnit *AttackUnitsInRange(const CUnit &unit);
/// Find best enemy in reaction range to attack
extern CUnit *AttackUnitsInReactRange(const CUnit &unit, CUnitFilter pred);
extern CUnit *AttackUnitsInReactRange(const CUnit &unit);



//@}

#endif // !__UNIT_FIND_H__
