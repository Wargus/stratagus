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

#include "stratagus.h"
#include "particle.h"
#include "video.h"


CGraphic *CFlameParticle::explosion0;
CGraphic *CFlameParticle::explosion1;


CFlameParticle::CFlameParticle(CPosition position) :
	CParticle(position), frame(0), currTicks(0)
{
	if (MyRand() % 2 == 0) {
		g = explosion0;
		numFrames = 16;
	} else {
		g = explosion1;
		numFrames = 15;
	}
}


CFlameParticle::~CFlameParticle()
{
}

void CFlameParticle::init()
{
	if (!explosion0) {
		explosion0 = CGraphic::New("graphics/particle/explosion0-7.png", 96, 128);
		explosion0->Load();
	}
	if (!explosion1) {
		explosion1 = CGraphic::New("graphics/particle/explosion1-7.png", 128, 96);
		explosion1->Load();
	}
}

void CFlameParticle::exit()
{
	if (explosion0) {
		CGraphic::Free(explosion0);
		explosion0 = NULL;
	}
	if (explosion1) {
		CGraphic::Free(explosion1);
		explosion1 = NULL;
	}
}

void CFlameParticle::draw()
{
	g->DrawFrameClip(frame, pos.x - g->Width / 2, pos.y - g->Height / 2);
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
