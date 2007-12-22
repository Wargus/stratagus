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
//      $Id$

//@{

#include <sstream>

#include "stratagus.h"
#include "particle.h"
#include "video.h"


static const int NumExplosions = 9;
static const int Sizes[][2] = {
	{ 128, 96 },
	{  54, 73 },
	{  13, 18 },
};

static CGraphic *large[NumExplosions];
static CGraphic *medium[NumExplosions];
static CGraphic *small[NumExplosions];


CFlameParticle::CFlameParticle(CPosition position) :
	CParticle(position)
{
	int explosion = MyRand() % NumExplosions;
	// FIXME: use different size explosions
	flame = new GraphicAnimation(large[explosion], 33);
}


CFlameParticle::~CFlameParticle()
{
	delete flame;
}

void CFlameParticle::init()
{
	for (int i = 0; i < NumExplosions; ++i) {
		if (!large[i]) {
			std::ostringstream os;
			os << "graphics/particle/large0" << i + 1 << ".png";
			large[i] = CGraphic::New(os.str(), Sizes[0][0], Sizes[0][1]);
			large[i]->Load();
		}
		if (!medium[i]) {
			std::ostringstream os;
			os << "graphics/particle/medium0" << i + 1 << ".png";
			medium[i] = CGraphic::New(os.str(), Sizes[1][0], Sizes[1][1]);
			medium[i]->Load();
		}
		if (!small[i]) {
			std::ostringstream os;
			os << "graphics/particle/small0" << i + 1 << ".png";
			small[i] = CGraphic::New(os.str(), Sizes[2][0], Sizes[2][1]);
			small[i]->Load();
		}
	}
}

void CFlameParticle::exit()
{
	for (int i = 0; i < NumExplosions; ++i) {
		if (large[i]) {
			CGraphic::Free(large[i]);
			large[i] = NULL;
		}
		if (medium[i]) {
			CGraphic::Free(medium[i]);
			medium[i] = NULL;
		}
		if (small[i]) {
			CGraphic::Free(small[i]);
			small[i] = NULL;
		}
	}
}

void CFlameParticle::draw()
{
	CPosition screenPos = ParticleManager.getScreenPos(pos);
	flame->draw(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y));
}

void CFlameParticle::update(int ticks)
{
	flame->update(ticks);
	if (flame->isFinished()) {
		destroy();
	}
}

//@}
