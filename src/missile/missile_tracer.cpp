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
/**@name missile_tracer.cpp - The missile Tracer. */
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

#include <stdio.h>

#include "stratagus.h"

#include "missile.h"

#include "map.h"
#include "unit.h"

/**
**  Handle tracer missile.
**
**  @param missile  Missile pointer.
**
**  @return         1 if goal is reached, 0 else.
*/
static int TracerMissile(Missile &missile)
{
	if (MissileInitMove(missile) == 1) {
		return 1;
	}

	Assert(missile.Type != NULL);
	Assert(missile.TotalStep != 0);
	if (missile.TargetUnit) {
		missile.destination = missile.TargetUnit->GetMapPixelPosCenter();
	}

	const PixelPos diff = (missile.destination - missile.source);
	missile.position = missile.source + diff * missile.CurrentStep / missile.TotalStep;

	if (missile.Type->Smoke.Missile && missile.CurrentStep) {
		const PixelPos position =  missile.position + missile.Type->size / 2;
		MakeMissile(*missile.Type->Smoke.Missile, position, position);
	}
	return 0;
}

/**
**  Missile flies from x,y to the target position, changing direction on the way
*/
void MissileTracer::Action()
{
	this->Wait = this->Type->Sleep;
	if (TracerMissile(*this)) {
		this->MissileHit();
		this->TTL = 0;
	} else {
		this->NextMissileFrame(1, 0);
	}
}

//@}
