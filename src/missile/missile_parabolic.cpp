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
/**@name missile_parabolic.cpp - The missile Parabolic. */
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
#include "unit_find.h"

/**
**  Calculate parabolic trajectories.
**
**  @param missile  Missile pointer.
**
**  @return         true if target is reached, false otherwise
**
**  @todo Find good values for ZprojToX and Y
*/
static bool ParabolicMissile(Missile &missile)
{
	// Should be initialised by an other method (computed with distance...)
	const double k = -missile.Type->ParabolCoefficient; // Coefficient of the parabol.
	const double zprojToX = 4.0;    // Projection of Z axis on axis X.
	const double zprojToY = 1024.0; // Projection of Z axis on axis Y.
	double z;        // should be missile.Z later.

	MissileInitMove(missile);
	if (missile.TotalStep == 0) {
		return true;
	}
	Assert(missile.Type != NULL);
	const PixelPos orig_pos = missile.position;
	Assert(missile.TotalStep != 0);
	const PixelPos diff = (missile.destination - missile.source);
	const PixelPrecise sign(diff.x >= 0 ? 1 : -1, diff.y >= 0 ? 1 : -1); // Remember sign to move into correct direction
	PixelPrecise pos(missile.position.x, missile.position.y); // Remember old position
	missile.position = missile.source + diff * missile.CurrentStep / missile.TotalStep;

	Assert(k != 0);
	z = (double)missile.CurrentStep * (missile.TotalStep - missile.CurrentStep) / k;
	// Until Z is used for drawing, modify X and Y.
	missile.position.x += (int)(z * zprojToX / 64.0);
	missile.position.y += (int)(z * zprojToY / 64.0);
	missile.MissileNewHeadingFromXY(missile.position - orig_pos);
	for (; pos.x * sign.x <= missile.position.x * sign.x
		 && pos.y * sign.y <= missile.position.y * sign.y;
		 pos.x += (double)diff.x * missile.Type->SmokePrecision / missile.TotalStep,
		 pos.y += (double)diff.y * missile.Type->SmokePrecision / missile.TotalStep) {

		if (missile.Type->Smoke.Missile && missile.CurrentStep) {
			const PixelPos position((int)pos.x + missile.Type->size.x / 2,
									(int)pos.y + missile.Type->size.y / 2);
			Missile *smoke = MakeMissile(*missile.Type->Smoke.Missile, position, position);
			if (smoke && smoke->Type->NumDirections > 1) {
				smoke->MissileNewHeadingFromXY(diff);
			}
		}

		if (missile.Type->SmokeParticle && missile.CurrentStep) {
			const PixelPos position((int)pos.x + missile.Type->size.x / 2,
									(int)pos.y + missile.Type->size.y / 2);
			missile.Type->SmokeParticle->pushPreamble();
			missile.Type->SmokeParticle->pushInteger(position.x);
			missile.Type->SmokeParticle->pushInteger(position.y);
			missile.Type->SmokeParticle->run();
		}

		if (missile.Type->Pierce) {
			const PixelPos position((int)pos.x, (int)pos.y);
			MissileHandlePierce(missile, Map.MapPixelPosToTilePos(position));
		}
	}

	if (missile.CurrentStep == missile.TotalStep) {
		missile.position = missile.destination;
		return true;
	}
	return false;
}

/**
**  Missile flies from x,y to x1,y1 using a parabolic path
*/
void MissileParabolic::Action()
{
	this->Wait = this->Type->Sleep;
	if (ParabolicMissile(*this)) {
		this->MissileHit();
		this->TTL = 0;
	} else {
		this->NextMissileFrameCycle();
	}
}

//@}
