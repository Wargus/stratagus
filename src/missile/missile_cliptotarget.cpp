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
/**@name missile_cliptotarget.cpp - The missile ClipToTarget. */
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
#include "unit.h"

/**
**  Missile remains clipped to target's current goal and plays his animation once
*/
void MissileClipToTarget::Action()
{
	this->Wait = this->Type->Sleep;

	if (this->TargetUnit != NULL) {
		this->position.x = this->TargetUnit->tilePos.x * PixelTileSize.x + this->TargetUnit->IX;
		this->position.y = this->TargetUnit->tilePos.y * PixelTileSize.y + this->TargetUnit->IY;
	}

	if (this->NextMissileFrame(1, 0)) {
		this->MissileHit();
		this->TTL = 0;
	}
}

//@}
