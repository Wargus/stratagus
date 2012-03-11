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
/**@name build.cpp - The units. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer, Jimmy Salmon and Rafal Bursig
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

#include "stratagus.h"
#include "unit.h"
#include "unittype.h"
#include "map.h"
#include "player.h"
#include "actions.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Find the building restriction that gives me this unit built on top
**  Allows you to define how the restriction is effecting the build
**
**  @param unit    the unit that is "OnTop"
**  @param parent  the parent unit if known. (guess otherwise)
**
**  @return        the BuildingRestrictionDetails
*/
CBuildRestrictionOnTop *OnTopDetails(const CUnit &unit, const CUnitType *parent)
{
	CBuildRestrictionAnd *andb;
	CBuildRestrictionOnTop *ontopb;

	for (std::vector<CBuildRestriction *>::iterator i = unit.Type->BuildingRules.begin();
			i != unit.Type->BuildingRules.end(); ++i) {
		if ((ontopb = dynamic_cast<CBuildRestrictionOnTop *>(*i))) {
			if (!parent) {
				// Guess this is right
				return ontopb;
			}
			if (parent == ontopb->Parent) {
				return ontopb;
			}
		} else if ((andb = dynamic_cast<CBuildRestrictionAnd *>(*i))) {
			for (std::vector<CBuildRestriction *>::iterator i = andb->_or_list.begin();
					i != andb->_or_list.end(); ++i) {
				if ((ontopb = dynamic_cast<CBuildRestrictionOnTop *>(*i))) {
					if (!parent) {
						// Guess this is right
						return ontopb;
					}
					if (parent == ontopb->Parent) {
						return ontopb;
					}
				}
			}
		}
	}
	return NULL;
}

/**
**  Check And Restriction
*/
bool CBuildRestrictionAnd::Check(const CUnitType &type, int x, int y, CUnit *&ontoptarget) const
{
	for (std::vector<CBuildRestriction*>::const_iterator i = _or_list.begin(); i != _or_list.end(); ++i) {
		if (!(*i)->Check(type, x, y, ontoptarget)) {
			return false;
		}
	}
	return true;
}

/**
**  Check Distance Restriction
*/
bool CBuildRestrictionDistance::Check(const CUnitType &type, int x, int y, CUnit *&) const
{
	const Vec2i pos = {x, y};
	Vec2i pos1 = {0, 0};
	Vec2i pos2 = {0, 0};
	int distance = 0;

	if (this->DistanceType == LessThanEqual ||
			this->DistanceType == GreaterThan ||
			this->DistanceType == Equal ||
			this->DistanceType == NotEqual) {
		pos1.x = std::max<int>(x - this->Distance, 0);
		pos1.y = std::max<int>(y - this->Distance, 0);
		pos2.x = std::min<int>(x + type.TileWidth + this->Distance, Map.Info.MapWidth);
		pos2.y = std::min<int>(y + type.TileHeight + this->Distance, Map.Info.MapHeight);
		distance = this->Distance;
	} else if (this->DistanceType == LessThan ||
			this->DistanceType == GreaterThanEqual) {
		pos1.x = std::max<int>(x - this->Distance - 1, 0);
		pos1.y = std::max<int>(y - this->Distance - 1, 0);
		pos2.x = std::min<int>(x + type.TileWidth + this->Distance + 1, Map.Info.MapWidth);
		pos2.y = std::min<int>(y + type.TileHeight + this->Distance + 1, Map.Info.MapHeight);
		distance = this->Distance - 1;
	}
	std::vector<CUnit *> table;
	Map.Select(pos1, pos2, table);

	switch (this->DistanceType) {
		case GreaterThan :
		case GreaterThanEqual :
			for (size_t i = 0; i != table.size(); ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, pos, *table[i]->Type, table[i]->tilePos) <= distance) {
					return false;
				}
			}
			return true;
		case LessThan :
		case LessThanEqual :
			for (size_t i = 0; i != table.size(); ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, pos, *table[i]->Type, table[i]->tilePos) <= distance) {
					return true;
				}
			}
			return false;
		case Equal :
			for (size_t i = 0; i != table.size(); ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, pos, *table[i]->Type, table[i]->tilePos) == distance) {
					return true;
				}
			}
			return false;
		case NotEqual :
			for (size_t i = 0; i != table.size(); ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, pos, *table[i]->Type, table[i]->tilePos) == distance) {
					return false;
				}
			}
			return true;
	}
	return false;
}

inline bool CBuildRestrictionAddOn::functor::operator() (const CUnit *const unit) const
{
	return (unit->Type == Parent && unit->tilePos.x == x && unit->tilePos.y == y);
}

/**
**  Check AddOn Restriction
*/
bool CBuildRestrictionAddOn::Check(const CUnitType &, int x, int y, CUnit *&) const
{
	int x1;
	int y1;

	x1 = x - this->OffsetX < 0 ? -1 : x - this->OffsetX;
	x1 = x1 >= Map.Info.MapWidth ? -1 : x1;
	y1 = y - this->OffsetY < 0 ? -1 : y - this->OffsetY;
	y1 = y1 >= Map.Info.MapHeight ? -1 : y1;
	if (!(x1 == -1 || y1 == -1)) {
		functor f(Parent, x1, y1);
		return (Map.Field(x1,y1)->UnitCache.find(f) != NULL);
	}
	return false;
}

/**
**  Check OnTop Restriction
*/
inline bool CBuildRestrictionOnTop::functor::operator() (CUnit *const unit)
{
	if (unit->tilePos.x == x && unit->tilePos.y == y &&
		!unit->Destroyed &&	unit->Orders[0]->Action != UnitActionDie) {
		if (unit->Type == this->Parent &&
				unit->Orders[0]->Action != UnitActionBuilt) {
			// Available to build on
			ontop = unit;
		} else {
			// Something else is built on this already
			ontop = NULL;
			return false;
		}
	}
	return true;
}

class AliveConstructedAndSameTypeAs
{
public:
	explicit AliveConstructedAndSameTypeAs(const CUnitType& unitType) : type(&unitType) {}
	bool operator () (const CUnit* unit) const
	{
		return unit->IsAlive() && unit->Type == type && unit->CurrentAction() != UnitActionBuilt;
	}
private:
	const CUnitType *type;
};


bool CBuildRestrictionOnTop::Check(const CUnitType &, int x, int y, CUnit *&ontoptarget) const
{
	const Vec2i pos = {x, y};
	Assert(Map.Info.IsPointOnMap(pos));

	ontoptarget = NULL;
	CUnitCache &cache = Map.Field(pos)->UnitCache;

	CUnitCache::iterator it = std::find_if(cache.begin(), cache.end(), AliveConstructedAndSameTypeAs(*this->Parent));

	if (it != cache.end() && (*it)->tilePos == pos) {
		ontoptarget = *it;
		return true;
	}
	return false;
}


/**
**  Can build unit here.
**  Hall too near to goldmine.
**  Refinery or shipyard too near to oil patch.
**
**  @param unit  Unit doing the building
**  @param type  unit-type to be checked.
**  @param pos   Map position.
**
**  @return      OnTop, parent unit, builder on true or 1 if unit==NULL, NULL false.
*/
CUnit *CanBuildHere(const CUnit *unit, const CUnitType &type, const Vec2i &pos)
{
	CUnit *ontoptarget;

	//
	//  Can't build outside the map
	//
	if (pos.x + type.TileWidth > Map.Info.MapWidth) {
		return NULL;
	}
	if (pos.y + type.TileHeight > Map.Info.MapHeight) {
		return NULL;
	}

	// Must be checked before oil!
	if (type.ShoreBuilding) {
		const int width = type.TileWidth;
		int h = type.TileHeight;
		bool success = false;

		// Need at least one coast tile
		unsigned int index = Map.getIndex(pos);
		do {
			CMapField *mf = Map.Field(index);
			int w = width;
			do {
				//if (Map.CoastOnMap(pos)) {
				if((mf->Flags & MapFieldCoastAllowed) == MapFieldCoastAllowed) {
					success = true;
				}
				++mf;
			} while(!success && --w);
			index += Map.Info.MapWidth;
		} while(!success && --h);
		if (!success) {
			return NULL;
		}
	}

	size_t count = type.BuildingRules.size();
	if (count > 0) {
		ontoptarget = NULL;
		for(unsigned int i = 0; i < count; ++i) {
			CBuildRestriction *rule = type.BuildingRules[i];
			// All checks processed, did we really have success
			if (rule->Check(type, pos.x, pos.y, ontoptarget)) {
				// We passed a full ruleset return
				if (unit == NULL) {
					return ontoptarget ? ontoptarget : (CUnit *)1;
				} else {
					return ontoptarget ? ontoptarget : (CUnit *)unit;
				}
			}
		}
		return NULL;
	}
	return (unit == NULL) ? (CUnit *)1 : const_cast<CUnit *>(unit);
}

/**
**  Can build on this point.
**
**  @param pos   tile map position.
**  @param  mask terrain mask
**
**  @return 1 if we can build on this point.
*/
bool CanBuildOn(const Vec2i &pos, int mask)
{
	return (Map.Info.IsPointOnMap(pos) && !Map.CheckMask(pos, mask));
}

/**
**  Can build unit-type at this point.
**
**  @param unit  Worker that want to build the building or NULL.
**  @param type  Building unit-type.
**  @param pos   tile map position.
**  @param real  Really build, or just placement
**
**  @return      OnTop, parent unit, builder on true, NULL false.
**
*/
CUnit *CanBuildUnitType(const CUnit *unit, const CUnitType &type, const Vec2i &pos, int real)
{
	// Terrain Flags don't matter if building on top of a unit.
	CUnit *ontop = CanBuildHere(unit, type, pos);
	if (ontop == NULL) {
		return NULL;
	}
	if (ontop != (CUnit *)1 && ontop != unit) {
		return ontop;
	}

	//  Remove unit that is building!
	if (unit) {
		UnmarkUnitFieldFlags(*unit);
	}

	CPlayer *player = NULL;

	if (unit && unit->Player->Type == PlayerPerson) {
		player = unit->Player;
	}
	int testmask;
	unsigned int index = pos.y * Map.Info.MapWidth;
	for (int h = 0; h < type.TileHeight; ++h) {
		for (int w = type.TileWidth; w--;) {
			/* first part of if (!CanBuildOn(x + w, y + h, testmask)) */
			if (!Map.Info.IsPointOnMap(pos.x + w, pos.y + h)) {
				h = type.TileHeight;
				ontop = NULL;
				break;
			}
			if (player && !real) {
				//testmask = MapFogFilterFlags(player, x + w, y + h, type.MovementMask);
				testmask = MapFogFilterFlags(*player,
						index + pos.x + w, type.MovementMask);
			} else {
				testmask = type.MovementMask;
			}
			/*secound part of if (!CanBuildOn(x + w, y + h, testmask)) */
			if (Map.CheckMask(index + pos.x + w, testmask))
			{
				h = type.TileHeight;
				ontop = NULL;
				break;
			}
			if (player && !Map.IsFieldExplored(*player, index + pos.x + w)) {
				h = type.TileHeight;
				ontop = NULL;
				break;
			}
		}
		index += Map.Info.MapWidth;
	}
	if (unit) {
		MarkUnitFieldFlags(*unit);
	}
	// We can build here: check distance to gold mine/oil patch!
	return ontop;
}

//@}
