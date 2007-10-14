//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name smokeparticle.cpp - The smoke particle. */
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
#include <iomanip>

#include "stratagus.h"
#include "particle.h"
#include "video.h"


static const int NumSmokes = 12;
static const int Sizes[][2] = {
	{  4,  4 },
	{  8,  8 },
	{ 12, 12 },
	{ 16, 16 },
	{ 20, 20 },
	{ 24, 24 },
	{ 28, 28 },
	{ 32, 32 },
	{ 36, 36 },
	{ 40, 40 },
	{ 44, 44 },
	{ 48, 48 },
};

CGraphic *CSmokeParticle::lightSmoke[NumSmokes];
CGraphic *CSmokeParticle::darkSmoke[NumSmokes];
int CSmokeParticle::numFrames = 26;


CSmokeParticle::CSmokeParticle(CPosition position) :
	CParticle(position), frame(0), currTicks(0)
{
	int size = 2;

	if (MyRand() % 2 == 0) {
		g = lightSmoke[size];
	} else {
		g = darkSmoke[size];
	}
}

CSmokeParticle::~CSmokeParticle()
{
}

void CSmokeParticle::init()
{
	for (int i = 0; i < NumSmokes; ++i) {
		if (!lightSmoke[i]) {
			std::ostringstream os;
			os << "graphics/particle/smokelight" << std::setfill('0') << std::setw(2) << (i + 1) * 4 << ".png";
			lightSmoke[i] = CGraphic::New(os.str(), Sizes[i][0], Sizes[i][1]);
			lightSmoke[i]->Load();
		}
		if (!darkSmoke[i]) {
			std::ostringstream os;
			os << "graphics/particle/smokedark" << std::setfill('0') << std::setw(2) << (i + 1) * 4 << ".png";
			darkSmoke[i] = CGraphic::New(os.str(), Sizes[i][0], Sizes[i][1]);
			darkSmoke[i]->Load();
		}
	}
}

void CSmokeParticle::exit()
{
	for (int i = 0; i < NumSmokes; ++i) {
		if (lightSmoke[i]) {
			CGraphic::Free(lightSmoke[i]);
			lightSmoke[i] = NULL;
		}
		if (darkSmoke[i]) {
			CGraphic::Free(darkSmoke[i]);
			darkSmoke[i] = NULL;
		}
	}
}

void CSmokeParticle::draw()
{
	g->DrawFrameClip(frame, static_cast<int>(pos.x - g->Width / 2.f),
		static_cast<int>(pos.y - g->Height / 2.f));
}

void CSmokeParticle::update(int ticks)
{
	const int ticksPerFrame = 60;
	currTicks += ticks;
	while (currTicks > ticksPerFrame) {
		currTicks -= ticksPerFrame;
		++frame;
	}
	if (frame >= numFrames) {
		destroy();
		return;
	}

	// smoke rises
	const int smokeRisePerSecond = 22;
	pos.y -= ticks / 1000.f * smokeRisePerSecond;
}

//@}
