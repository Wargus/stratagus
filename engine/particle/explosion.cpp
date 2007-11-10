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

#include "stratagus.h"
#include "particle.h"


CExplosion::CExplosion(CPosition position) :
	CParticle(position)
{
	ParticleManager.setLowDetail(true);
	if (!ParticleManager.getLowDetail()) {
		CFlashParticle *flash = new CFlashParticle(position);
		ParticleManager.add(flash);
	}

	CFlameParticle *flame = new CFlameParticle(position);
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
}

void CExplosion::exit()
{
}

void CExplosion::draw()
{
}

void CExplosion::update(int ticks)
{
}


//@}
