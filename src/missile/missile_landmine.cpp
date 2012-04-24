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
/**@name missile_landmine.cpp - The missile LandMine. */
//
//      (c) Copyright 2012 by Joris Dauphin
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

#include "missile.h"

#include "actions.h"
#include "map.h"
#include "unit.h"

struct LandMineTargetFinder {
	const CUnit *const source;
	int CanHitOwner;
	LandMineTargetFinder(const CUnit *unit, int hit):
		source(unit), CanHitOwner(hit) {}
	inline bool operator()(const CUnit *const unit) const {
		return (!(unit == source && !CanHitOwner)
				&& unit->Type->UnitType != UnitTypeFly
				&& unit->CurrentAction() != UnitActionDie);
	}
	inline CUnit *FindOnTile(const CMapField *const mf) const {
		return mf->UnitCache.find(*this);
	}
};

/**
**  Land mine controller.
**  @todo start-finish-start cyclic animation.(anim scripts!)
**  @todo missile should dissapear for a while.
*/
void MissileLandMine::Action()
{
	const Vec2i pos = Map.MapPixelPosToTilePos(this->position);

	if (LandMineTargetFinder(this->SourceUnit, this->Type->CanHitOwner).FindOnTile(Map.Field(pos)) != NULL) {
		DebugPrint("Landmine explosion at %d,%d.\n" _C_ pos.x _C_ pos.y);
		this->MissileHit();
		this->TTL = 0;
		return;
	}
	if (!this->AnimWait--) {
		this->NextMissileFrame(1, 0);
		this->AnimWait = this->Type->Sleep;
	}
	this->Wait = 1;
}

//@}
