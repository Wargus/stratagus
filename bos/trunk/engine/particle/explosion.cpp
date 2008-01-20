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
//      (c) Copyright 2007-2008 by Jimmy Salmon
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
#include <iomanip>

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

static void FreeFlameGraphics()
{
	for (int i = 0; i < NumExplosions; ++i) {
		if (large[i]) {
			CGraphic::Free(large[i]);
			large[i] = NULL;
		}
	}
}


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

CGraphic *lightSmoke[NumSmokes];
CGraphic *darkSmoke[NumSmokes];

static void InitSmokeGraphics()
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

static void FreeSmokeGraphics()
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


	CGraphic *smoke;
	int size = 2;

	if (MyRand() % 2 == 0) {
		smoke = lightSmoke[size];
	} else {
		smoke = darkSmoke[size];
	}

	int numChunks = 8;
	if (ParticleManager.getLowDetail()) {
		numChunks /= 2;
	}
	for (int i = 0; i < numChunks; ++i) {
		Animation *smokeanimation = new GraphicAnimation(smoke, 60);
		CChunkParticle *chunk = new CChunkParticle(position, smokeanimation);
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
	InitSmokeGraphics();
}

void CExplosion::exit()
{
	FreeFlashGraphics();
	FreeFlameGraphics();
	FreeSmokeGraphics();
}

void CExplosion::draw()
{
}

void CExplosion::update(int ticks)
{
}

CParticle* CExplosion::clone()
{
	Assert(0);
	return 0;
}

//@}
