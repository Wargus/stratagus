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
/**@name chunkparticle.cpp - The chunk particle. */
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

#include <math.h>

#include "stratagus.h"
#include "particle.h"
#include "video.h"


static const int gravity = 32 * 12;

static inline float deg2rad(int degrees) {
	return degrees * (3.1415926535f / 180);
}


CChunkParticle::CChunkParticle(CPosition position, Animation *smokeAnimation) :
	CParticle(position), initialPos(position), nextSmokeTicks(0), age(0),
	height(0.f)
{
	float radians = deg2rad(MyRand() % 360);
	direction.x = cos(radians);
	direction.y = sin(radians);

	const int maxVelocity = 400;
	initialVelocity = MyRand() % maxVelocity;

	int minTrajectoryAngle = 77;
	trajectoryAngle = deg2rad(MyRand() % (90 - minTrajectoryAngle) + minTrajectoryAngle);

	lifetime = (int)(1000 * (initialVelocity * sin(trajectoryAngle) / gravity) * 2);
	this->smokeAnimation = smokeAnimation->clone();
}

CChunkParticle::~CChunkParticle()
{
	delete smokeAnimation;
}

static float calculateScreenPos(float posy, float height)
{
	return posy - height * 0.2f;
}

void CChunkParticle::draw()
{
	CPosition screenPos = ParticleManager.getScreenPos(pos);
	Uint32 color = ColorBlack;

	Video.DrawRectangleClip(color, (int)screenPos.x - 1,
		(int)calculateScreenPos(screenPos.y, height) - 1, 2, 2);
}

static float getHorizontalPosition(int initialVelocity,
	float trajectoryAngle, float time)
{
    return (initialVelocity * cos(trajectoryAngle)) * time;
}

static float getVerticalPosition(int initialVelocity,
	float trajectoryAngle, float time)
{
    return (initialVelocity * sin(trajectoryAngle)) * time -
    	(gravity / 2.0f) * (time * time);
}

void CChunkParticle::update(int ticks)
{
	age += ticks;
	if (age >= lifetime) {
		destroy();
		return;
	}

	const int minSmokeTicks = 150;
	const int randSmokeTicks = 50;

	if (age > nextSmokeTicks) {
		CPosition p(pos.x, calculateScreenPos(pos.y, height));
		Animation *animation = smokeAnimation->clone();
		CSmokeParticle *smoke = new CSmokeParticle(p, animation);
		ParticleManager.add(smoke);

		nextSmokeTicks += MyRand() % randSmokeTicks + minSmokeTicks;
	}

	float time = age / 1000.f;

	float distance =
		getHorizontalPosition(initialVelocity, trajectoryAngle, time);
	pos.x = initialPos.x + distance * direction.x;
	pos.y = initialPos.y + distance * direction.y;

	height = getVerticalPosition(initialVelocity, trajectoryAngle, time);
}


CParticle* CChunkParticle::clone()
{
	return new CChunkParticle(pos, smokeAnimation);
}

//@}
