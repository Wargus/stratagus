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


/* static */ void CParticleManager::init()
{
}

/* static */ void CParticleManager::exit()
{
	ParticleManager.clear();
}

void CParticleManager::clear()
{
	particles.clear();
	new_particles.clear();
}

std::vector<CParticle *> CParticleManager::prepareToDraw(const CViewport &vp)
{
	this->vp = &vp;
	std::vector<CParticle *> table;

	for (auto &particle : particles) {
		if (particle->isVisible(vp)) {
			table.push_back(particle.get());
		}
	}

	ranges::sort(table, [](const CParticle *lhs, const CParticle *rhs) {
		return lhs->getDrawLevel() < rhs->getDrawLevel();
	});
	return table;
}

void CParticleManager::endDraw()
{
	this->vp = nullptr;
}

void CParticleManager::update()
{
	unsigned long ticks = GameCycle - lastTicks;

	std::move(new_particles.begin(), new_particles.end(), std::back_inserter(particles));
	new_particles.clear();

	for (auto &particle : particles) {
		particle->update(1000.0f / CYCLES_PER_SECOND * ticks);
	}
	ranges::erase_if(particles, [](const auto &particle) { return particle->isDestroyed(); });

	lastTicks += ticks;
}

void CParticleManager::add(CParticle* particle)
{
	add(std::unique_ptr<CParticle>(particle));
}

void CParticleManager::add(std::unique_ptr<CParticle> particle)
{
	new_particles.push_back(std::move(particle));
}

CPosition CParticleManager::getScreenPos(const CPosition &pos) const
{
	const PixelPos mapPixelPos((int)pos.x, (int)pos.y);
	const PixelPos screenPixelPos = vp->MapToScreenPixelPos(mapPixelPos);

	return CPosition(screenPixelPos.x, screenPixelPos.y);
}

//@}
