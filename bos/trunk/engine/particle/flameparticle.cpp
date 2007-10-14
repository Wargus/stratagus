//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name flameparticle.cpp - The flame particle. */
//
//      (c) Copyright 2007 by Jimmy Salmon
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
//      $Id: commands.cpp 9147 2007-09-28 17:18:24Z jim4 $

//@{

#include <sstream>

#include "stratagus.h"
#include "particle.h"
#include "video.h"


static const int NumExplosions = 7;
static const int Sizes[][2] = {
	{ 13, 18 },
	{ 27, 36 },
	{ 41, 54 },
	{ 54, 73 },
	{ 68, 91 },
	{ 82, 109 },
	{ 96, 128 },
};

CGraphic *CFlameParticle::explosion0[NumExplosions];
CGraphic *CFlameParticle::explosion1[NumExplosions];


CFlameParticle::CFlameParticle(CPosition position) :
	CParticle(position), frame(0), currTicks(0)
{
	int size = 2;

	if (MyRand() % 2 == 0) {
		g = explosion0[size];
		numFrames = 16;
	} else {
		g = explosion1[size];
		numFrames = 15;
	}
}


CFlameParticle::~CFlameParticle()
{
}

void CFlameParticle::init()
{
	for (int i = 0; i < NumExplosions; ++i) {
		if (!explosion0[i]) {
			std::ostringstream os;
			os << "graphics/particle/explosion0-" << i + 1 << ".png";
			explosion0[i] = CGraphic::New(os.str(), Sizes[i][0], Sizes[i][1]);
			explosion0[i]->Load();
		}
		if (!explosion1[i]) {
			std::ostringstream os;
			os << "graphics/particle/explosion1-" << i + 1 << ".png";
			explosion1[i] = CGraphic::New(os.str(), Sizes[i][1], Sizes[i][0]);
			explosion1[i]->Load();
		}
	}
}

void CFlameParticle::exit()
{
	for (int i = 0; i < NumExplosions; ++i) {
		if (explosion0[i]) {
			CGraphic::Free(explosion0[i]);
			explosion0[i] = NULL;
		}
		if (explosion1[i]) {
			CGraphic::Free(explosion1[i]);
			explosion1[i] = NULL;
		}
	}
}

void CFlameParticle::draw()
{
	g->DrawFrameClip(frame, static_cast<int>(pos.x - g->Width / 2.f),
		static_cast<int>(pos.y - g->Height / 2.f));
}

void CFlameParticle::update(int ticks)
{
	const int ticksPerFrame = 33;
	currTicks += ticks;
	while (currTicks > ticksPerFrame) {
		currTicks -= ticksPerFrame;
		++frame;
	}
	if (frame >= numFrames) {
		destroy();
	}
}

//@}
