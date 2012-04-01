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

#include "animation.h"
#include "map.h"
#include "sound.h"
#include "unit.h"

/* virtual */ void CAnimation_RandomSound::Action(CUnit& unit, int &/*move*/, int /*scale*/) const
{
	Assert(unit.Anim.Anim == this);

	if (unit.IsVisible(*ThisPlayer) || ReplayRevealMap) {
		const size_t index = SyncRand() % this->sounds.size();
		PlayUnitSound(unit, this->sounds[index].Sound);
	}
}

/* virtual */ void CAnimation_RandomSound::Init(const char* s)
{
	// FIXME : Bad const cast.
	char *op2 = const_cast<char*>(s);
	int count = 0;

	while (op2 && *op2) {
		char *next = strchr(op2, ' ');
		if (next) {
			while (*next == ' ') {
				*next++ = '\0';
			}
		}
		++count;
		this->sounds.push_back(SoundConfig(op2));
		op2 = next;
	}
}

void CAnimation_RandomSound::MapSound()
{
	for (size_t i = 0; i != this->sounds.size(); ++i) {
		this->sounds[i].Sound = SoundForName(this->sounds[i].Name);
	}
}


//@}
