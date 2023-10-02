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
/**@name animation_randomsound.cpp - The animation RandomSound. */
//
//      (c) Copyright 2012 by Joris Dauphin
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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "animation/animation_randomsound.h"

#include "map.h"
#include "sound.h"
#include "unit.h"

#include <iterator>
#include <sstream>

/* virtual */ void CAnimation_RandomSound::Action(CUnit &unit, int &/*move*/, int /*scale*/) const
{
	Assert(unit.Anim.Anim == this);

	if (unit.IsVisible(*ThisPlayer) || ReplayRevealMap) {
		const size_t index = MyRand() % this->sounds.size();
		PlayUnitSound(unit, this->sounds[index].Sound);
	}
}

/*
**  s = "Sound1 [SoundN ...]"
*/
void CAnimation_RandomSound::Init(std::string_view s, lua_State *) /* override */
{
	std::istringstream is{std::string(s)};
	std::vector<std::string> args;
	std::copy(std::istream_iterator<std::string>(is),
	          std::istream_iterator<std::string>(),
	          std::back_inserter(args));

	for (auto& arg : args) {
		this->sounds.push_back(SoundConfig(std::move(arg)));
	}
}

void CAnimation_RandomSound::MapSound()
{
	for (auto& sound : this->sounds) {
		sound.MapSound();
	}
}


//@}
