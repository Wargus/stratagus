//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name build.cpp - The units. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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
#include "unit_cache.h"
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
			for (std::vector<CBuildRestriction *>::iterator it = andb->_or_list.begin();
					it != andb->_or_list.end(); ++it) {
				if ((ontopb = dynamic_cast<CBuildRestrictionOnTop *>(*it))) {
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
	int x1 = 0;
	int x2 = 0;
	int y1 = 0;
	int y2 = 0;
	int distance = 0;

	if (this->DistanceType == LessThanEqual ||
			this->DistanceType == GreaterThan ||
			this->DistanceType == Equal ||
			this->DistanceType == NotEqual) {
		x1 = std::max(x - this->Distance, 0);
		y1 = std::max(y - this->Distance, 0);
		x2 = std::min(x + type->TileWidth + this->Distance, Map.Info.MapWidth);
		y2 = std::min(y + type->TileHeight + this->Distance, Map.Info.MapHeight);
		distance = this->Distance;
	} else if (this->DistanceType == LessThan ||
			this->DistanceType == GreaterThanEqual) {
		x1 = std::max(x - this->Distance - 1, 0);
		y1 = std::max(y - this->Distance - 1, 0);
		x2 = std::min(x + type->TileWidth + this->Distance + 1, Map.Info.MapWidth);
		y2 = std::min(y + type->TileHeight + this->Distance + 1, Map.Info.MapHeight);
		distance = this->Distance - 1;
	}
	n = UnitCache.Select(x1, y1, x2, y2, table, UnitMax);

	switch (this->DistanceType) {
		case GreaterThan :
		case GreaterThanEqual :
			for (int i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) <= distance) {
					return false;
				}
			}
			return true;
		case LessThan :
		case LessThanEqual :
			for (int i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) <= distance) {
					return true;
				}
			}
			return false;
		case Equal :
			for (int i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) == distance) {
					return true;
				}
			}
			return false;
		case NotEqual :
			for (int i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) == distance) {
					return false;
				}
			}
			return true;
	}
	return false;
}

/**
**  Check AddOn Restriction
*/
bool CBuildRestrictionAddOn::Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const
{
	CUnit *table[UnitMax];
	int n;
	int i;
	int x1;
	int y1;

	x1 = x - this->OffsetX < 0 ? -1 : x - this->OffsetX;
	x1 = x1 >= Map.Info.MapWidth ? -1 : x1;
	y1 = y - this->OffsetY < 0 ? -1 : y - this->OffsetY;
	y1 = y1 >= Map.Info.MapHeight ? -1 : y1;
	if (!(x1 == -1 || y1 == -1)) {
		n = UnitCache.Select(x1, y1, table, UnitMax);
		for (i = 0; i < n; ++i) {
			if (table[i]->Type == this->Parent &&
					table[i]->X == x1 && table[i]->Y == y1) {
				return true;
			}
		}
	}
	return false;
}

/**
**  Check OnTop Restriction
*/
bool CBuildRestrictionOnTop::Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const
{
	CUnit *table[UnitMax];
	int n;
	int i;

	ontoptarget = NULL;
	n = UnitCache.Select(x, y, table, UnitMax);
	for (i = 0; i < n; ++i) {
		if (table[i]->X == x && table[i]->Y == y && !table[i]->Destroyed &&
				table[i]->Orders[0]->Action != UnitActionDie) {
			if (table[i]->Type == this->Parent &&
					table[i]->Orders[0]->Action != UnitActionBuilt) {
				// Available to build on
				ontoptarget = table[i];
			} else {
				// Something else is built on this already
				ontoptarget = NULL;
				return false;
			}
		}
	}
	return ontoptarget != NULL;
}

/**
**  Check Terrain Restriction
*/
bool CBuildRestrictionTerrain::Check(const CUnitType *type, int xMin, int yMin, CUnit *&ontoptarget) const
{
	Assert(xMin >= 0);
	Assert(xMin + type->TileWidth <= Map.Info.MapWidth);
	Assert(yMin >= 0);
	Assert(yMin + type->TileHeight <= Map.Info.MapHeight);

	int count = 0;
	for (int y = yMin; y < yMin + type->TileHeight; y++) {
		for (int x = xMin; x < xMin + type->TileWidth; x++) {
			// In principle, if the player hasn't explored
			// the map field yet, this function should not
			// leak any information about it.  However,
			// CanBuildHere seems to be the only caller,
			// and after Check returns true, it verifies
			// that the tiles have been explored, and
			// prevents building if not.
			if (Map.Field(x, y)->Flags & this->FieldFlags) {
				count++;
			}
		}
	}

	return count >= this->Min && count <= this->Max;
}

/**
**  Can build unit here.
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
		bool success = false;

		// Need at least one coast tile
		for (int h = type->TileHeight; h--;) {
			for (int w = type->TileWidth; w--;) {
				if (Map.Field(x + w, y + h)->Flags & MapFieldCoastAllowed) {
					h = w = 0;
					success = true;
					break;
				}
			}
		}
		if (!success) {
			return NULL;
		}
	}

	if (type->BuildingRules.empty()) {
		return (unit == NULL) ? (CUnit *)1 : const_cast<CUnit *>(unit);
	}

	ontoptarget = NULL;
	for (std::vector<CBuildRestriction *>::const_iterator ib = type->BuildingRules.begin();
			ib != type->BuildingRules.end(); ++ib) {
		// All checks processed, did we really have success
		if ((*ib)->Check(type, x, y, ontoptarget)) {
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

/**
**  Can build on this point.
**
**  @param x     X tile map position.
**  @param y     Y tile map position.
**  @param mask  terrain mask
**
**  @return      if we can build on this point.
*/
bool CanBuildOn(int x, int y, int mask)
{
	if (x < 0 || y < 0 || x >= Map.Info.MapWidth || y >= Map.Info.MapHeight) {
		return false;
	}
	return (Map.Field(x, y)->Flags & mask) ? false : true;
}

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
*/
CUnit *CanBuildUnitType(const CUnit *unit, const CUnitType *type, int x, int y, int real)
{
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
	if (unit) {
		UnmarkUnitFieldFlags(unit);
	}

	player = NULL;

	if (unit && unit->Player->Type == PlayerPerson) {
		player = unit->Player;
	}

	for (int h = type->TileHeight; h--;) {
		for (int w = type->TileWidth; w--;) {
			if (player && !real) {
				testmask = MapFogFilterFlags(player, x + w, y + h, type->MovementMask);
			} else {
				testmask = type->MovementMask;
			}
			if (!CanBuildOn(x + w, y + h, testmask)) {
				if (unit) {
					MarkUnitFieldFlags(unit);
				}
				return NULL;
			}
			if (player && !Map.IsFieldExplored(player, x + w, y + h)) {
				if (unit) {
					MarkUnitFieldFlags(unit);
				}
				return NULL;
			}
		}
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
