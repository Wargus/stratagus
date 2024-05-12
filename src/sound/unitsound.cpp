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
//      (c) Copyright 1999-2015 by Fabrice Rossi, Jimmy Salmon and Andrettin
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

#include "stratagus.h"

#include "unitsound.h"

#include "animation/animation_randomsound.h"
#include "animation/animation_sound.h"
#include "map.h"
#include "player.h"
#include "sound.h"
#include "sound_server.h"
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
	return this->Sound != nullptr;
}

void SoundConfig::SetSoundRange(unsigned char range)
{
	if (this->Sound) {
		this->Sound->Range = range;
	}
}

/**
**  Load all sounds for units.
*/
void LoadUnitSounds()
{
}

/**
**  Map animation sounds
*/
static void MapAnimSounds2(std::vector<std::unique_ptr<CAnimation>>& anims)
{
	for (auto &anim : anims) {
		anim->MapSound();
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
	MapAnimSounds2(type.Animations->RangedAttack);
	MapAnimSounds2(type.Animations->SpellCast);
	for (auto& soundConfig : type.Animations->Death) {
		MapAnimSounds2(soundConfig);
	}
	MapAnimSounds2(type.Animations->Repair);
	MapAnimSounds2(type.Animations->Train);
	MapAnimSounds2(type.Animations->Research);
	MapAnimSounds2(type.Animations->Upgrade);
	MapAnimSounds2(type.Animations->Build);
	for (auto& soundConfig : type.Animations->Harvest) {
		MapAnimSounds2(soundConfig);
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
	for (CUnitType *typePtr : getUnitTypes()) {
		CUnitType &type = *typePtr;

		MapAnimSounds(type);

		type.MapSound.Selected.MapSound();
		type.MapSound.Acknowledgement.MapSound();
		// type.Sound.Acknowledgement.SetSoundRange(INFINITE_SOUND_RANGE);
		type.MapSound.Attack.MapSound();
		type.MapSound.Build.MapSound();
		type.MapSound.Ready.MapSound();
		type.MapSound.Ready.SetSoundRange(INFINITE_SOUND_RANGE);
		type.MapSound.Repair.MapSound();
		for (auto &soundConfig : type.MapSound.Harvest) {
			soundConfig.MapSound();
		}
		type.MapSound.Help.MapSound();
		type.MapSound.Help.SetSoundRange(INFINITE_SOUND_RANGE);
		type.MapSound.WorkComplete.MapSound();

		for (auto &soundConfig : type.MapSound.Dead) {
			soundConfig.MapSound();
		}
	}
}

//@}
