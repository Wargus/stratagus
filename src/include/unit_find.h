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
//      (c) Copyright 1998-2012 by Lutz Sammer, Jimmy Salmon and Joris Dauphin
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

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

//
//  Some predicates
//

class HasSameTypeAs
{
public:
	explicit HasSameTypeAs(const CUnitType &_type) : type(&_type) {}
	bool operator()(const CUnit *unit) const { return unit->Type == type; }
private:
	const CUnitType *type;
};

class HasSamePlayerAs
{
public:
	explicit HasSamePlayerAs(const CPlayer &_player) : player(&_player) {}
	bool operator()(const CUnit *unit) const { return unit->Player == player; }
private:
	const CPlayer *player;
};

class HasNotSamePlayerAs
{
public:
	explicit HasNotSamePlayerAs(const CPlayer &_player) : player(&_player) {}
	bool operator()(const CUnit *unit) const { return unit->Player != player; }
private:
	const CPlayer *player;
};

class IsAlliedWith
{
public:
	explicit IsAlliedWith(const CPlayer &_player) : player(&_player) {}
	bool operator()(const CUnit *unit) const { return unit->IsAllied(*player); }
private:
	const CPlayer *player;
};

class IsEnemyWith
{
public:
	explicit IsEnemyWith(const CPlayer &_player) : player(&_player) {}
	bool operator()(const CUnit *unit) const { return unit->IsEnemy(*player); }
private:
	const CPlayer *player;
};

class HasSamePlayerAndTypeAs
{
public:
	explicit HasSamePlayerAndTypeAs(const CUnit &unit) :
		player(unit.Player), type(unit.Type)
	{}
	HasSamePlayerAndTypeAs(const CPlayer &_player, const CUnitType &_type) :
		player(&_player), type(&_type)
	{}

	bool operator()(const CUnit *unit) const {
		return (unit->Player == player && unit->Type == type);
	}

private:
	const CPlayer *player;
	const CUnitType *type;
};

class IsNotTheSameUnitAs
{
public:
	explicit IsNotTheSameUnitAs(const CUnit &unit) : forbidden(&unit) {}
	bool operator()(const CUnit *unit) const { return unit != forbidden; }
private:
	const CUnit *forbidden;
};

class IsBuildingType
{
public:
	bool operator()(const CUnit *unit) const { return unit->Type->Building; }
};


template <typename Pred>
class NotPredicate
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
class AndPredicate
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
	explicit CUnitTypeFinder(const UnitTypeType t) : unitTypeType(t) {}
	bool operator()(const CUnit *const unit) const {
		const CUnitType &type = *unit->Type;
		if (type.Vanishes || (unitTypeType != static_cast<UnitTypeType>(-1) && type.UnitType != unitTypeType)) {
			return false;
		}
		return true;
	}
private:
	const UnitTypeType unitTypeType;
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

void Select(const Vec2i &ltPos, const Vec2i &rbPos, std::vector<CUnit *> &units);
void SelectFixed(const Vec2i &ltPos, const Vec2i &rbPos, std::vector<CUnit *> &units);
void SelectAroundUnit(const CUnit &unit, int range, std::vector<CUnit *> &around);

template <typename Pred>
void SelectFixed(const Vec2i &ltPos, const Vec2i &rbPos, std::vector<CUnit *> &units, Pred pred)
{
	Assert(Map.Info.IsPointOnMap(ltPos));
	Assert(Map.Info.IsPointOnMap(rbPos));
	Assert(units.empty());

	for (Vec2i posIt = ltPos; posIt.y != rbPos.y + 1; ++posIt.y) {
		for (posIt.x = ltPos.x; posIt.x != rbPos.x + 1; ++posIt.x) {
			const CMapField &mf = *Map.Field(posIt);
			const CUnitCache &cache = mf.UnitCache;

			for (size_t i = 0; i != cache.size(); ++i) {
				CUnit &unit = *cache[i];

				if (unit.CacheLock == 0 && pred(&unit)) {
					unit.CacheLock = 1;
					units.push_back(&unit);
				}
			}
		}
	}
	for (size_t i = 0; i != units.size(); ++i) {
		units[i]->CacheLock = 0;
	}
}

template <typename Pred>
void SelectAroundUnit(const CUnit &unit, int range, std::vector<CUnit *> &around, Pred pred)
{
	const Vec2i offset(range, range);
	const Vec2i typeSize(unit.Type->TileWidth - 1, unit.Type->TileHeight - 1);

	Select(unit.tilePos - offset,
		   unit.tilePos + typeSize + offset, around,
		   MakeAndPredicate(IsNotTheSameUnitAs(unit), pred));
}

template <typename Pred>
void Select(const Vec2i &ltPos, const Vec2i &rbPos, std::vector<CUnit *> &units, Pred pred)
{
	Vec2i minPos = ltPos;
	Vec2i maxPos = rbPos;

	Map.FixSelectionArea(minPos, maxPos);
	SelectFixed(minPos, maxPos, units, pred);
}

template <typename Pred>
CUnit *FindUnit_IfFixed(const Vec2i &ltPos, const Vec2i &rbPos, Pred pred)
{
	Assert(Map.Info.IsPointOnMap(ltPos));
	Assert(Map.Info.IsPointOnMap(rbPos));

	for (Vec2i posIt = ltPos; posIt.y != rbPos.y + 1; ++posIt.y) {
		for (posIt.x = ltPos.x; posIt.x != rbPos.x + 1; ++posIt.x) {
			const CMapField &mf = *Map.Field(posIt);
			const CUnitCache &cache = mf.UnitCache;

			CUnitCache::const_iterator it = std::find_if(cache.begin(), cache.end(), pred);
			if (it != cache.end()) {
				return *it;
			}
		}
	}
	return NULL;
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
							   int resource, bool check_usage = false, const CUnit *deposit = NULL);

/// Find nearest deposit
extern CUnit *FindDeposit(const CUnit &unit, int range, int resource);
/// Find the next idle worker
extern CUnit *FindIdleWorker(const CPlayer &player, const CUnit *last);

/// Find the neareast piece of terrain with specific flags.
extern bool FindTerrainType(int movemask, int resmask, int range,
							const CPlayer &player, const Vec2i &startPos, Vec2i *pos);

extern void FindUnitsByType(const CUnitType &type, std::vector<CUnit *> &units, bool everybody = false);

/// Find all units of this type of the player
extern void FindPlayerUnitsByType(const CPlayer &player, const CUnitType &type, std::vector<CUnit *> &units);
/// Return any unit on that map tile
extern CUnit *UnitOnMapTile(const Vec2i &pos, unsigned int type);// = -1);
/// Return possible attack target on that map area
extern CUnit *TargetOnMap(const CUnit &unit, const Vec2i &pos1, const Vec2i &pos2);

/// Return resource, if on map tile
extern CUnit *ResourceOnMap(const Vec2i &pos, int resource, bool mine_on_top = true);
/// Return resource deposit, if on map tile
extern CUnit *ResourceDepositOnMap(const Vec2i &pos, int resource);

/// Find best enemy in numeric range to attack
extern CUnit *AttackUnitsInDistance(const CUnit &unit, int range, bool onlyBuildings = false);
/// Find best enemy in attack range to attack
extern CUnit *AttackUnitsInRange(const CUnit &unit);
/// Find best enemy in reaction range to attack
extern CUnit *AttackUnitsInReactRange(const CUnit &unit);

//@}

#endif // !__UNIT_FIND_H__
