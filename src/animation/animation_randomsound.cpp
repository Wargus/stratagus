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

/* virtual */ void CAnimation_RandomSound::Action(CUnit &unit, int &/*move*/, int /*scale*/) const
{
	Assert(unit.Anim.Anim == this);

	if (unit.IsVisible(*ThisPlayer) || ReplayRevealMap) {
		const size_t index = SyncRand() % this->sounds.size();
		PlayUnitSound(unit, this->sounds[index].Sound);
	}
}

/*
**  s = "Sound1 [SoundN ...]"
*/
/* virtual */ void CAnimation_RandomSound::Init(const char *s, lua_State *)
{
	const std::string str(s);
	const size_t len = str.size();

	for (size_t begin = 0; begin != std::string::npos;) {
		const size_t end = std::min(len, str.find(' ', begin));

		this->sounds.push_back(SoundConfig(str.substr(begin, end - begin)));
		begin = str.find_first_not_of(' ', end);
	}
}

void CAnimation_RandomSound::MapSound()
{
	for (size_t i = 0; i != this->sounds.size(); ++i) {
		this->sounds[i].MapSound();
	}
}


//@}
