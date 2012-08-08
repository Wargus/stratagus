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
**  @return         1 if target is reached, 0 otherwise
**
**  @todo Find good values for ZprojToX and Y
*/
static int ParabolicMissile(Missile &missile)
{
	int k;        // Coefficient of the parabol.
	int zprojToX; // Projection of Z axis on axis X.
	int zprojToY; // Projection of Z axis on axis Y.
	int z;        // should be missile.Z later.

	k = -2048; //-1024; // Should be initialised by an other method (computed with distance...)
	zprojToX = 4;
	zprojToY = 1024;
	if (MissileInitMove(missile) == 1) {
		return 1;
	}
	Assert(missile.Type != NULL);
	const PixelPos orig_pos = missile.position;
	Assert(missile.TotalStep != 0);
	const PixelPos diff = (missile.destination - missile.source);
	missile.position = missile.source + diff * missile.CurrentStep / missile.TotalStep;

	Assert(k != 0);
	z = missile.CurrentStep * (missile.TotalStep - missile.CurrentStep) / k;
	// Until Z is used for drawing, modify X and Y.
	missile.position.x += z * zprojToX / 64;
	missile.position.y += z * zprojToY / 64;
	missile.MissileNewHeadingFromXY(missile.position - orig_pos);
	if (missile.Type->Smoke.Missile && missile.CurrentStep) {
		const PixelPos position = missile.position + missile.Type->size / 2;
		MakeMissile(*missile.Type->Smoke.Missile, position, position);
	}
	if (missile.Type->SmokeParticle && missile.CurrentStep) {
		const PixelPos position = missile.position + missile.Type->size / 2;
		missile.Type->SmokeParticle->pushPreamble();
		missile.Type->SmokeParticle->pushInteger(position.x);
		missile.Type->SmokeParticle->pushInteger(position.y);
		missile.Type->SmokeParticle->run();
	}
	if (missile.Type->Pierce) {
		MissileHandlePierce(missile, Map.MapPixelPosToTilePos(missile.position));
	}
	return 0;
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
