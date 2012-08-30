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
	// Animate, move.
	if (!this->AnimWait--) {
		if (this->NextMissileFrame(1, 0)) {
			this->SpriteFrame = 0;
			PointToPointMissile(*this);
		}
		this->AnimWait = this->Type->Sleep;
	}
	this->Wait = 1;

	// Center of the tornado
	const PixelPos pixelCenter = this->position + this->Type->size / 2;
	const PixelPos centerOffset(PixelTileSize.x / 2, PixelTileSize.y);
	const Vec2i center = Map.MapPixelPosToTilePos(pixelCenter + centerOffset);

	if (!(this->TTL % CYCLES_PER_SECOND / 10)) {
		this->MissileHit();
	}
	// Changes direction every 3 seconds (approx.)
	if (!(this->TTL % 100)) { // missile has reached target unit/spot
		Vec2i newPos;

		do {
			// find new destination in the map
			newPos.x = center.x + SyncRand() % 5 - 2;
			newPos.y = center.y + SyncRand() % 5 - 2;
		} while (!Map.Info.IsPointOnMap(newPos));
		this->destination = Map.TilePosToMapPixelPos_Center(newPos);
		this->source = this->position;
		this->State = 0;
		DebugPrint("Whirlwind new direction: %d, %d, TTL: %d\n" _C_
				   this->destination.x _C_ this->destination.y _C_ this->TTL);
	}
}

//@}
