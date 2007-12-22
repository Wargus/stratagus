//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name flashparticle.cpp - The flash particle. */
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
//      $Id$

//@{

#include "stratagus.h"
#include "particle.h"
#include "video.h"


CGraphic *CFlashParticle::flash;
int CFlashParticle::numFrames = 20;


CFlashParticle::CFlashParticle(CPosition position) :
	CParticle(position), frame(0), currTicks(0)
{
	// FIXME: smaller explosions shouldn't use all the frames
}

CFlashParticle::~CFlashParticle()
{
}

void CFlashParticle::init()
{
	if (!flash) {
		flash = CGraphic::New("graphics/particle/flash.png", 240, 194);
		flash->Load();
	}
}

void CFlashParticle::exit()
{
	if (flash) {
		CGraphic::Free(flash);
		flash = NULL;
	}
}

void CFlashParticle::draw()
{
	CPosition screenPos = ParticleManager.getScreenPos(pos);
	flash->DrawFrameClip(frame, static_cast<int>(screenPos.x - flash->Width / 2.f),
		static_cast<int>(screenPos.y - flash->Height / 2.f));
}

void CFlashParticle::update(int ticks)
{
	const int ticksPerFrame = 22;
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
