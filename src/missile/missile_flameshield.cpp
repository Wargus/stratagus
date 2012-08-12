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
/**@name missile_flameshield.cpp - The missile FlameShield. */
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
#include "unit.h"
#include "unit_find.h"

/**
**  FlameShield controller
*/
void MissileFlameShield::Action()
{
	static int fs_dc[] = {
		0, 32, 5, 31, 10, 30, 16, 27, 20, 24, 24, 20, 27, 15, 30, 10, 31,
		5, 32, 0, 31, -5, 30, -10, 27, -16, 24, -20, 20, -24, 15, -27, 10,
		-30, 5, -31, 0, -32, -5, -31, -10, -30, -16, -27, -20, -24, -24, -20,
		-27, -15, -30, -10, -31, -5, -32, 0, -31, 5, -30, 10, -27, 16, -24,
		20, -20, 24, -15, 27, -10, 30, -5, 31, 0, 32
	};

	this->Wait = this->Type->Sleep;
	const int index = this->TTL % 36;  // 36 positions on the circle
	const int dx = fs_dc[index * 2];
	const int dy = fs_dc[index * 2 + 1];
	CUnit *unit = this->TargetUnit;
	//
	// Show around the top most unit.
	// FIXME: conf, do we hide if the unit is contained or not?
	//
	while (unit->Container) {
		unit = unit->Container;
	}
	const Vec2i upos = unit->tilePos;
	const int ix = unit->IX;
	const int iy = unit->IY;
	const int uw = unit->Type->TileWidth;
	const int uh = unit->Type->TileHeight;
	this->position.x = upos.x * PixelTileSize.x + ix + uw * PixelTileSize.x / 2 + dx - 16;
	this->position.y = upos.y * PixelTileSize.y + iy + uh * PixelTileSize.y / 2 + dy - 32;
	if (unit->CurrentAction() == UnitActionDie) {
		this->TTL = index;
	}

	if (unit->Container) {
		this->Hidden = 1;
		return;  // Hidden missile don't do damage.
	} else {
		this->Hidden = 0;
	}

	// Only hit 1 out of 8 frames
	if (this->TTL & 7) {
		return;
	}

	std::vector<CUnit *> table;
	SelectAroundUnit(*unit, 1, table);
	for (size_t i = 0; i != table.size(); ++i) {
		if (table[i]->CurrentAction() != UnitActionDie) {
			HitUnit(this->SourceUnit, *table[i], this->Damage);
		}
	}
}

//@}
