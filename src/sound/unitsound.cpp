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
/**@name unitsound.cpp - The unit sounds. */
//
//      (c) Copyright 1999-2007 by Fabrice Rossi and Jimmy Salmon
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
--  Include
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#include "unitsound.h"

#include "animation/animation_randomsound.h"
#include "animation/animation_sound.h"
#include "map.h"
#include "player.h"
#include "sound.h"
#include "sound_server.h"
#include "tileset.h"
#include "unit.h"
#include "unittype.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

bool SoundConfig::MapSound()
{
	if (!this->Name.empty()) {
		this->Sound = SoundForName(this->Name);
	}
	return this->Sound != NULL;
}

void SoundConfig::SetSoundRange(unsigned char range)
{
	if (this->Sound) {
		::SetSoundRange(this->Sound, range);
	}
}

/**
**  Load all sounds for units.
*/
void LoadUnitSounds()
{
}

static void MapAnimSound(CAnimation &anim)
{
	if (anim.Type == AnimationSound) {
		CAnimation_Sound &anim_sound = *static_cast<CAnimation_Sound *>(&anim);

		anim_sound.MapSound();
	} else if (anim.Type == AnimationRandomSound) {
		CAnimation_RandomSound &anim_rsound = *static_cast<CAnimation_RandomSound *>(&anim);

		anim_rsound.MapSound();
	}
}

/**
**  Map animation sounds
*/
static void MapAnimSounds2(CAnimation *anim)
{
	if (anim == NULL) {
		return ;
	}
	MapAnimSound(*anim);
	for (CAnimation *it = anim->Next; it != anim; it = it->Next) {
		MapAnimSound(*it);
	}
}

/**
**  Map animation sounds for a unit type
*/
static void MapAnimSounds(CUnitType &type)
{
	if (!type.Animations) {
		return;
	}
	MapAnimSounds2(type.Animations->Start);
	MapAnimSounds2(type.Animations->Still);
	MapAnimSounds2(type.Animations->Move);
	MapAnimSounds2(type.Animations->Attack);
	MapAnimSounds2(type.Animations->SpellCast);
	for (int i = 0; i <= ANIMATIONS_DEATHTYPES; ++i) {
		MapAnimSounds2(type.Animations->Death[i]);
	}
	MapAnimSounds2(type.Animations->Repair);
	MapAnimSounds2(type.Animations->Train);
	MapAnimSounds2(type.Animations->Research);
	MapAnimSounds2(type.Animations->Upgrade);
	MapAnimSounds2(type.Animations->Build);
	for (int i = 0; i < MaxCosts; ++i) {
		MapAnimSounds2(type.Animations->Harvest[i]);
	}
}

/**
**  Map the sounds of all unit-types to the correct sound id.
**  And overwrite the sound ranges.
**  @todo the sound ranges should be configurable by user with CCL.
*/
void MapUnitSounds()
{
	if (SoundEnabled() == false) {
		return;
	}
	// Parse all units sounds.
	for (std::vector<CUnitType *>::size_type i = 0; i < UnitTypes.size(); ++i) {
		CUnitType &type = *UnitTypes[i];

		MapAnimSounds(type);

		type.Sound.Selected.MapSound();
		type.Sound.Acknowledgement.MapSound();
		// type.Sound.Acknowledgement.SetSoundRange(INFINITE_SOUND_RANGE);
		type.Sound.Attack.MapSound();
		type.Sound.Ready.MapSound();
		type.Sound.Ready.SetSoundRange(INFINITE_SOUND_RANGE);
		type.Sound.Repair.MapSound();
		for (int i = 0; i < MaxCosts; ++i) {
			type.Sound.Harvest[i].MapSound();
		}
		type.Sound.Help.MapSound();
		type.Sound.Help.SetSoundRange(INFINITE_SOUND_RANGE);

		for (int i = 0; i <= ANIMATIONS_DEATHTYPES; ++i) {
			type.Sound.Dead[i].MapSound();
		}
	}
}

//@}
