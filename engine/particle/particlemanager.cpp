//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name particlemanager.cpp - The particle manager. */
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


CParticleManager ParticleManager;


CParticleManager::CParticleManager()
{
}

CParticleManager::~CParticleManager()
{
}

void CParticleManager::init()
{
	CExplosion::init();
	CFlameParticle::init();
	CFlashParticle::init();
	CChunkParticle::init();
	CSmokeParticle::init();
}

void CParticleManager::exit()
{
	CExplosion::exit();
	CFlameParticle::exit();
	CFlashParticle::exit();
	CChunkParticle::exit();
	CSmokeParticle::exit();
}

void CParticleManager::draw()
{
	std::vector<CParticle *>::iterator i;
	for (i = particles.begin(); i != particles.end(); ++i) {
		(*i)->draw();
	}
}

void CParticleManager::update()
{
	static unsigned long lastTicks = 0;
	unsigned long ticks = GetTicks() - lastTicks;
	std::vector<CParticle *>::iterator i;

	i = particles.begin();
	while (i != particles.end()) {
		(*i)->update(ticks);
		if ((*i)->isDestroyed()) {
			delete *i;
			i = particles.erase(i);
		} else {
			++i;
		}
	}

	lastTicks += ticks;
}

void CParticleManager::add(CParticle *particle)
{
	particles.push_back(particle);
}

//@}
