//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name explosion.cpp - The explosion particle. */
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
#include <sstream>

#include "stratagus.h"
#include "particle.h"
#include "video.h"

static CGraphic *flashgraphic;

static void InitFlashGraphics()
{
	if (!flashgraphic) {
		flashgraphic = CGraphic::New("graphics/particle/flash.png", 240, 194);
		flashgraphic->Load();
	}
}

static void FreeFlashGraphics()
{
	if (flashgraphic) {
		CGraphic::Free(flashgraphic);
		flashgraphic = NULL;
	}
}


static const int NumExplosions = 9;
static CGraphic *large[NumExplosions];

static void InitFlameGraphics()
{
	for (int i = 0; i < NumExplosions; ++i) {
		if (!large[i]) {
			std::ostringstream os;
			os << "graphics/particle/large0" << i + 1 << ".png";
			large[i] = CGraphic::New(os.str(), 128, 96);
			large[i]->Load();
		}
	}
}

void FreeFlameGraphics()
{
	for (int i = 0; i < NumExplosions; ++i) {
		if (large[i]) {
			CGraphic::Free(large[i]);
			large[i] = NULL;
		}
	}
}

CExplosion::CExplosion(CPosition position) :
	CParticle(position)
{
	if (!ParticleManager.getLowDetail()) {
		Animation *flashanim = new GraphicAnimation(flashgraphic, 22);
		StaticParticle *flash = new StaticParticle(position, flashanim);
		ParticleManager.add(flash);
	}

	int explosion = MyRand() % NumExplosions;
	Animation *flameanim = new GraphicAnimation(large[explosion], 33);
	StaticParticle *flame = new StaticParticle(position, flameanim);
	ParticleManager.add(flame);

	int numChunks = 8;
	if (ParticleManager.getLowDetail()) {
		numChunks /= 2;
	}
	for (int i = 0; i < numChunks; ++i) {
		CChunkParticle *chunk = new CChunkParticle(position);
		ParticleManager.add(chunk);
	}

	destroy();
}

CExplosion::~CExplosion()
{
}

void CExplosion::init()
{
	InitFlashGraphics();
	InitFlameGraphics();
}

void CExplosion::exit()
{
	FreeFlashGraphics();
	FreeFlameGraphics();
}

void CExplosion::draw()
{
}

void CExplosion::update(int ticks)
{
}


//@}
