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
/**@name smokeparticle.cpp - The smoke particle. */
//
//      (c) Copyright 2007-2008 by Jimmy Salmon and Francois Beerten
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
#include "stratagus.h"
#include "particle.h"
#include "video.h"




CSmokeParticle::CSmokeParticle(CPosition position, Animation *smoke) :
	CParticle(position), puff(smoke)
{
	Assert(smoke);
}

CSmokeParticle::~CSmokeParticle()
{
	delete puff;
}

void CSmokeParticle::draw()
{
	CPosition screenPos = ParticleManager.getScreenPos(pos);
	puff->draw(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y));
}

void CSmokeParticle::update(int ticks)
{
	puff->update(ticks);
	if (puff->isFinished()) {
		destroy();
		return;
	}

	// smoke rises
	const int smokeRisePerSecond = 22;
	pos.y -= ticks / 1000.f * smokeRisePerSecond;
}

CParticle* CSmokeParticle::clone()
{
	return new CSmokeParticle(pos, puff);
}

//@}
