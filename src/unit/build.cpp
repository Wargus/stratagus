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
CBuildRestrictionOnTop *OnTopDetails(const CUnit *unit, const CUnitType *parent)
{
	CBuildRestrictionAnd *andb;
	CBuildRestrictionOnTop *ontopb;

	for (std::vector<CBuildRestriction *>::iterator i = unit->Type->BuildingRules.begin();
			i != unit->Type->BuildingRules.end(); ++i) {
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
bool CBuildRestrictionAnd::Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const
{
	for (std::vector<CBuildRestriction*>::const_iterator i = _or_list.begin();
		i != _or_list.end(); ++i) {
		if (!(*i)->Check(type, x, y, ontoptarget)) {
			return false;
		}
	}
	return true;
}

/**
**  Check Distance Restriction
*/
bool CBuildRestrictionDistance::Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const
{
	CUnit *table[UnitMax];
	int n;
	int i;
	int x1 = 0;
	int x2 = 0;
	int y1 = 0;
	int y2 = 0;
	int distance = 0;

	if (this->DistanceType == LessThanEqual ||
			this->DistanceType == GreaterThan ||
			this->DistanceType == Equal ||
			this->DistanceType == NotEqual) {
		x1 = std::max<int>(x - this->Distance, 0);
		y1 = std::max<int>(y - this->Distance, 0);
		x2 = std::min<int>(x + type->TileWidth + this->Distance, Map.Info.MapWidth);
		y2 = std::min<int>(y + type->TileHeight + this->Distance, Map.Info.MapHeight);
		distance = this->Distance;
	} else if (this->DistanceType == LessThan ||
			this->DistanceType == GreaterThanEqual) {
		x1 = std::max<int>(x - this->Distance - 1, 0);
		y1 = std::max<int>(y - this->Distance - 1, 0);
		x2 = std::min<int>(x + type->TileWidth + this->Distance + 1, Map.Info.MapWidth);
		y2 = std::min<int>(y + type->TileHeight + this->Distance + 1, Map.Info.MapHeight);
		distance = this->Distance - 1;
	}
	n = Map.Select(x1, y1, x2, y2, table);

	switch (this->DistanceType) {
		case GreaterThan :
		case GreaterThanEqual :
			for (i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) <= distance) {
					return false;
				}
			}
			return true;
		case LessThan :
		case LessThanEqual :
			for (i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) <= distance) {
					return true;
				}
			}
			return false;
		case Equal :
			for (i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) == distance) {
					return true;
				}
			}
			return false;
		case NotEqual :
			for (i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) == distance) {
					return false;
				}
			}
			return true;
	}
	return false;
}

inline bool CBuildRestrictionAddOn::functor::operator() (const CUnit *const unit) const
{
	return (unit->Type == Parent &&
				unit->X == x && unit->Y == y);
}

/**
**  Check AddOn Restriction
*/
bool CBuildRestrictionAddOn::Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const
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
	if (unit->X == x && unit->Y == y &&
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

bool CBuildRestrictionOnTop::Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const
{
#if 0
	CUnit *table[UnitMax];
	int n;
	int i;

	ontoptarget = NULL;
	n = Map.Select(x, y, table, UnitMax);
	for (i = 0; i < n; ++i) {
		if (table[i]->X == x && table[i]->Y == y && !table[i]->Destroyed &&
				table[i]->CurrentAction() != UnitActionDie) {
			if (table[i]->Type == this->Parent &&
					table[i]->CurrentAction() != UnitActionBuilt) {
				// Available to build on
				ontoptarget = table[i];
			} else {
				// Something else is built on this already
				ontoptarget = NULL;
				return false;
			}
		}
	}
#else
		functor f(Parent, x, y);
		Map.Field(x,y)->UnitCache.for_each_if(f);
		ontoptarget = f.ontop;
#endif

	return ontoptarget != NULL;
}


/**
**  Can build unit here.
**  Hall to near to goldmine.
**  Refinery or shipyard to near to oil patch.
**
**  @param unit  Unit doing the building
**  @param type  unit-type to be checked.
**  @param x     Map X position.
**  @param y     Map Y position.
**
**  @return      OnTop, parent unit, builder on true or 1 if unit==NULL, NULL false.
*/
CUnit *CanBuildHere(const CUnit *unit, const CUnitType *type, int x, int y)
{
	CUnit *ontoptarget;

	//
	//  Can't build outside the map
	//
	if (x + type->TileWidth > Map.Info.MapWidth) {
		return NULL;
	}
	if (y + type->TileHeight > Map.Info.MapHeight) {
		return NULL;
	}

	// Must be checked before oil!
	if (type->ShoreBuilding) {
		const int width = type->TileWidth;
		int w, h = type->TileHeight;
		bool success = false;
		CMapField *mf;

		// Need at least one coast tile
		unsigned int index = Map.getIndex(x, y);
		do {
			mf = Map.Field(index);
			w = width;
			do {
				//if (Map.CoastOnMap(x ,y)) {
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

	size_t count = type->BuildingRules.size();
	if (count > 0) {
		ontoptarget = NULL;
		for(unsigned int i = 0; i < count; ++i) {
			CBuildRestriction *rule = type->BuildingRules[i];
			// All checks processed, did we really have success
			if (rule->Check(type, x, y, ontoptarget)) {
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
**  @param x     X tile map position.
**  @param y     Y tile map position.
**  @param  mask terrain mask
**
**  @return 1 if we can build on this point.
*/
bool CanBuildOn(int x, int y, int mask)
{
	return (Map.Info.IsPointOnMap(x,y) && !Map.CheckMask(x, y,mask));
}

extern int
MapFogFilterFlags(CPlayer *player, const unsigned int index, int mask);

/**
**  Can build unit-type at this point.
**
**  @param unit  Worker that want to build the building or NULL.
**  @param type  Building unit-type.
**  @param x     X tile map position.
**  @param y     Y tile map position.
**  @param real  Really build, or just placement
**
**  @return      OnTop, parent unit, builder on true, NULL false.
**
*/
CUnit *CanBuildUnitType(const CUnit *unit,
	 const CUnitType *type, int x, int y, int real)
{
	int w;
	int h;
	int j;
	int testmask;
	CPlayer *player;
	CUnit *ontop;

	// Terrain Flags don't matter if building on top of a unit.
	ontop = CanBuildHere(unit, type, x, y);
	if (ontop == NULL) {
		return NULL;
	}
	if (ontop != (CUnit *)1 && ontop != unit) {
		return ontop;
	}

	//
	//  Remove unit that is building!
	//
	j = 0;
	if (unit) {
		UnmarkUnitFieldFlags(unit);
	}

	player = NULL;

	if (unit && unit->Player->Type == PlayerPerson) {
		player = unit->Player;
	}

	unsigned int index = y * Map.Info.MapWidth;
	for (h = 0; h < type->TileHeight; ++h) {
		for (w = type->TileWidth; w--;) {
			/* first part of if (!CanBuildOn(x + w, y + h, testmask)) */
			if(!Map.Info.IsPointOnMap(x + w, y + h)) {
				h = type->TileHeight;
				ontop = NULL;
				break;
			}
			if (player && !real) {
				//testmask = MapFogFilterFlags(player, x + w, y + h, type->MovementMask);
				testmask = MapFogFilterFlags(player,
						index + x + w, type->MovementMask);
			} else {
				testmask = type->MovementMask;
			}
			/*secound part of if (!CanBuildOn(x + w, y + h, testmask)) */
			if(Map.CheckMask(index + x + w,testmask))
			{
				h = type->TileHeight;
				ontop = NULL;
				break;
			}
			if (player && !Map.IsFieldExplored(player, index + x + w)) {
				h = type->TileHeight;
				ontop = NULL;
				break;
			}
		}
		index += Map.Info.MapWidth;
	}
	if (unit) {
		MarkUnitFieldFlags(unit);
	}

	//
	// We can build here: check distance to gold mine/oil patch!
	//
	return ontop;
}

//@}
