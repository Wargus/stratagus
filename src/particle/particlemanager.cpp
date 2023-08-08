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
/**@name particlemanager.cpp - The particle manager. */
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

#include "stratagus.h"
#include "particle.h"
#include "ui.h"
#include "video.h"

#include <algorithm>


CParticleManager ParticleManager;


CParticleManager::CParticleManager() :
	vp(nullptr), lastTicks(0)
{
}

CParticleManager::~CParticleManager()
{
}

void CParticleManager::init()
{
}

void CParticleManager::exit()
{
	ParticleManager.clear();
}

void CParticleManager::clear()
{
	for (auto *p : particles) {
		delete p;
	}
	particles.clear();

	for (auto *p : new_particles) {
		delete p;
	}
	new_particles.clear();
}

static inline bool DrawLevelCompare(const CParticle *lhs, const CParticle *rhs)
{
	return lhs->getDrawLevel() < rhs->getDrawLevel();
}

std::vector<CParticle *> CParticleManager::prepareToDraw(const CViewport &vp)
{
	this->vp = &vp;
	std::vector<CParticle *> table;

	for (CParticle *p : particles) {
		CParticle &particle = *p;
		if (particle.isVisible(vp)) {
			table.push_back(&particle);
		}
	}

	ranges::sort(table, DrawLevelCompare);
	return table;
}

void CParticleManager::endDraw()
{
	this->vp = nullptr;
}

void CParticleManager::update()
{
	unsigned long ticks = GameCycle - lastTicks;
	std::vector<CParticle *>::iterator i;

	particles.insert(particles.end(), new_particles.begin(), new_particles.end());
	new_particles.clear();

	i = particles.begin();
	while (i != particles.end()) {
		(*i)->update(1000.0f / CYCLES_PER_SECOND * ticks);
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
	new_particles.push_back(particle);
}

CPosition CParticleManager::getScreenPos(const CPosition &pos) const
{
	const PixelPos mapPixelPos((int)pos.x, (int)pos.y);
	const PixelPos screenPixelPos = vp->MapToScreenPixelPos(mapPixelPos);

	return CPosition(screenPixelPos.x, screenPixelPos.y);
}

//@}