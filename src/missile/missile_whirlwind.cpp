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
/**@name missile_whirlwind.cpp - The missile Whirlwind. */
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

#include "map.h"

/**
**  Whirlwind controller
**
**  @todo do it more configurable.
*/
void MissileWhirlwind::Action()
{
	//
	// Animate, move.
	//
	if (!this->AnimWait--) {
		if (this->NextMissileFrame(1, 0)) {
			this->SpriteFrame = 0;
			PointToPointMissile(*this);
		}
		this->AnimWait = this->Type->Sleep;
	}
	this->Wait = 1;
	//
	// Center of the tornado
	//
	PixelPos center = this->position + this->Type->size / 2;
	center.x = (center.x + PixelTileSize.x / 2) / PixelTileSize.x;
	center.y = (center.y + PixelTileSize.y) / PixelTileSize.y;

#if 0
	CUnit *table[UnitMax];

	if (!(this->TTL % 4)) { // Every 4 cycles 4 points damage in tornado center
		int n = SelectUnitsOnTile(x, y, table);
		for (int i = 0; i < n; ++i) {
			if (table[i]->CurrentAction() != UnitActionDie) {
				// should be missile damage ?
				HitUnit(this->SourceUnit, table[i], WHIRLWIND_DAMAGE1);
			}
		}
	}
	if (!(this->TTL % (CYCLES_PER_SECOND / 10))) { // Every 1/10s 1 points damage on tornado periphery
		// we should parameter this
		int n = SelectUnits(center.x - 1, center.y - 1, center.x + 1, center.y + 1, table);
		for (int i = 0; i < n; ++i) {
			if ((table[i]->X != center.x || table[i]->Y != center.y) && table[i]->CurrentAction() != UnitActionDie) {
				// should be in missile
				HitUnit(this->SourceUnit, table[i], WHIRLWIND_DAMAGE2);
			}
		}
	}
	DebugPrint("Whirlwind: %d, %d, TTL: %d state: %d\n" _C_
			   missile->X _C_ missile->Y _C_ missile->TTL _C_ missile->State);
#else
	if (!(this->TTL % CYCLES_PER_SECOND / 10)) {
		this->MissileHit();
	}
#endif
	// Changes direction every 3 seconds (approx.)
	if (!(this->TTL % 100)) { // missile has reached target unit/spot
		int nx;
		int ny;

		do {
			// find new destination in the map
			nx = center.x + SyncRand() % 5 - 2;
			ny = center.y + SyncRand() % 5 - 2;
		} while (!Map.Info.IsPointOnMap(nx, ny));
		this->destination.x = nx * PixelTileSize.x + PixelTileSize.x / 2;
		this->destination.y = ny * PixelTileSize.y + PixelTileSize.y / 2;
		this->source = this->position;
		this->State = 0;
		DebugPrint("Whirlwind new direction: %d, %d, TTL: %d\n" _C_
				   this->destination.x _C_ this->destination.y _C_ this->TTL);
	}
}

//@}
