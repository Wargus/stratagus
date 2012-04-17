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
/**@name missile_fire.cpp - The missile Fire. */
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

/**
**  Missile don't move, than checks the source unit for HP.
*/
void MissileFire::Action()
{
	CUnit &unit = *this->SourceUnit;

	this->Wait = this->Type->Sleep;
	if (unit.Destroyed || unit.CurrentAction() == UnitActionDie) {
		this->TTL = 0;
		return;
	}
	if (this->NextMissileFrame(1, 0)) {
		this->SpriteFrame = 0;
		const int f = (100 * unit.Variable[HP_INDEX].Value) / unit.Variable[HP_INDEX].Max;
		MissileType *fire = MissileBurningBuilding(f);

		if (!fire) {
			this->TTL = 0;
			unit.Burning = 0;
		} else {
			if (this->Type != fire) {
				this->position += this->Type->size / 2;
				this->Type = fire;
				this->position -= this->Type->size / 2;
			}
		}
	}
}

//@}
