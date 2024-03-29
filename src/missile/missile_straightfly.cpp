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
/**@name missile_straightfly.cpp - The missile StraightFly. */
//
//      (c) Copyright 2013 by cybermind
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
**  Missile flies from x,y to x1,y1 then continues to fly, until incompatible terrain is detected
*/
void MissileStraightFly::Action() /* override */
{
	this->Wait = this->Type->Sleep;
	if (PointToPointMissile(*this)) {
		if (this->DestroyMissile) {
			this->MissileHit();
			this->TTL = 0;
			return;
		}
		if (this->TotalStep) {
			const PixelPos step = (this->destination - this->source);

			this->destination += step * ((PixelTileSize.x + PixelTileSize.y) * 3) / 4 / this->TotalStep;
			this->State++; // !(State & 1) to initialise
			this->source = this->position;
			PointToPointMissile(*this);
		} else {
			this->MissileHit();
			this->TTL = 0;
		}
	} else {
		this->NextMissileFrame(1, 0);
	}
}

//@}
