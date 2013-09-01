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

#include "luacallback.h"
#include "map.h"
#include "unit.h"
#include "unit_find.h"

/**
**  Handle tracer missile.
**
**  @param missile  Missile pointer.
**
**  @return         true if goal is reached, false else.
*/
static bool TracerMissile(Missile &missile)
{
	MissileInitMove(missile);
	if (missile.TotalStep == 0) {
		return true;
	}

	Assert(missile.Type != NULL);
	Assert(missile.TotalStep != 0);
	if (missile.TargetUnit) {
		missile.destination = missile.TargetUnit->GetMapPixelPosTopLeft();
	}

	const PixelPos diff = (missile.destination - missile.source);
	const PixelPrecise sign(diff.x >= 0 ? 1 : -1, diff.y >= 0 ? 1 : -1); // Remember sign to move into correct direction
	const PixelPrecise oldPos((double)missile.position.x, (double)missile.position.y); // Remember old position
	PixelPrecise pos(oldPos);
	missile.position = missile.source + diff * missile.CurrentStep / missile.TotalStep;

	for (; pos.x * sign.x <= missile.position.x * sign.x
		 && pos.y * sign.y <= missile.position.y * sign.y;
		 pos.x += (double)diff.x * missile.Type->SmokePrecision / missile.TotalStep,
		 pos.y += (double)diff.y * missile.Type->SmokePrecision / missile.TotalStep) {
		const PixelPos position((int)pos.x + missile.Type->size.x / 2,
								(int)pos.y + missile.Type->size.y / 2);
		if (missile.Type->Smoke.Missile && missile.CurrentStep) {
			Missile *smoke = MakeMissile(*missile.Type->Smoke.Missile, position, position);
			if (smoke && smoke->Type->NumDirections > 1) {
				smoke->MissileNewHeadingFromXY(diff);
			}
		}

		if (missile.Type->SmokeParticle && missile.CurrentStep) {
			missile.Type->SmokeParticle->pushPreamble();
			missile.Type->SmokeParticle->pushInteger(position.x);
			missile.Type->SmokeParticle->pushInteger(position.y);
			missile.Type->SmokeParticle->run();
		}

		if (missile.Type->Pierce) {
			const PixelPos posInt((int)pos.x, (int)pos.y);
			MissileHandlePierce(missile, Map.MapPixelPosToTilePos(posInt));
		}
	}

	// Handle wall blocking
	for (pos = oldPos; pos.x * sign.x <= missile.position.x * sign.x
		 && pos.y * sign.y <= missile.position.y * sign.y;
		 pos.x += (double)diff.x / missile.TotalStep,
		 pos.y += (double)diff.y / missile.TotalStep) {
		const PixelPos position((int)pos.x + missile.Type->size.x / 2,
								(int)pos.y + missile.Type->size.y / 2);
		if (MissileHandleBlocking(missile, position)) {
			return true;
		}
	}

	if (missile.CurrentStep == missile.TotalStep) {
		missile.position = missile.destination;
		return true;
	}
	return false;
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
