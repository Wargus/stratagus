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
#include "video.h"
#include "sound.h"
#include "unittype.h"
#include "animation.h"
#include "player.h"
#include "unit.h"
#include "sound_server.h"
#include "tileset.h"
#include "map.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Load all sounds for units.
*/
void LoadUnitSounds()
{
}

/**
**  Map animation sounds
*/
static void MapAnimSounds2(CAnimation *anim)
{
	while (anim) {
		if (anim->Type == AnimationSound) {
			anim->D.Sound.Sound = SoundForName(anim->D.Sound.Name);
		} else if (anim->Type == AnimationRandomSound) {
			for (unsigned int i = 0; i < anim->D.RandomSound.NumSounds; ++i) {
				anim->D.RandomSound.Sound[i] = SoundForName(anim->D.RandomSound.Name[i]);
			}
		}
		anim = anim->Next;
	}
}

/**
**  Map animation sounds for a unit type
*/
static void MapAnimSounds(CUnitType *type)
{
	int i;

	if (!type->Animations) {
		return;
	}

	MapAnimSounds2(type->Animations->Start);
	MapAnimSounds2(type->Animations->Still);
	MapAnimSounds2(type->Animations->Move);
	MapAnimSounds2(type->Animations->Attack);
	for (i = 0; i <= ANIMATIONS_DEATHTYPES; ++i) {
		MapAnimSounds2(type->Animations->Death[i]);
	}
	MapAnimSounds2(type->Animations->Repair);
	MapAnimSounds2(type->Animations->Train);
	MapAnimSounds2(type->Animations->Research);
	MapAnimSounds2(type->Animations->Upgrade);
	MapAnimSounds2(type->Animations->Build);
	for (i = 0; i < MaxCosts; ++i) {
		MapAnimSounds2(type->Animations->Harvest[i]);
	}
}

/**
**  Map the sounds of all unit-types to the correct sound id.
**  And overwrite the sound ranges. @todo the sound ranges should be
**  configurable by user with CCL.
*/
void MapUnitSounds()
{
	CUnitType *type;
	int j;

	if (SoundEnabled()) {
		//
		// Parse all units sounds.
		//
		for (std::vector<CUnitType *>::size_type i = 0; i < UnitTypes.size(); ++i) {
			type = UnitTypes[i];

			MapAnimSounds(type);

			if (!type->Sound.Selected.Name.empty()) {
				type->Sound.Selected.Sound =
					SoundForName(type->Sound.Selected.Name);
			}
			if (!type->Sound.Acknowledgement.Name.empty()) {
				type->Sound.Acknowledgement.Sound =
					SoundForName(type->Sound.Acknowledgement.Name);
				/*
				// Acknowledge sounds have infinite range
				SetSoundRange(type->Sound.Acknowledgement.Sound,
					INFINITE_SOUND_RANGE);
				*/
			}
			if (!type->Sound.Attack.Name.empty()) {
				type->Sound.Attack.Sound =
					SoundForName(type->Sound.Attack.Name);
			}
			if (!type->Sound.Ready.Name.empty()) {
				type->Sound.Ready.Sound =
					SoundForName(type->Sound.Ready.Name);
				// Ready sounds have infinite range
				SetSoundRange(type->Sound.Ready.Sound,
					INFINITE_SOUND_RANGE);
			}
			if (!type->Sound.Repair.Name.empty()) {
				type->Sound.Repair.Sound =
					SoundForName(type->Sound.Repair.Name);
			}
			for (j = 0; j < MaxCosts; ++j) {
				if (!type->Sound.Harvest[j].Name.empty()) {
					type->Sound.Harvest[j].Sound =
						SoundForName(type->Sound.Harvest[j].Name);
				}
			}
			if (!type->Sound.Help.Name.empty()) {
				type->Sound.Help.Sound =
					SoundForName(type->Sound.Help.Name);
				// Help sounds have infinite range
				SetSoundRange(type->Sound.Help.Sound,
					INFINITE_SOUND_RANGE);
			}
			for (j = 0; j <= ANIMATIONS_DEATHTYPES; ++j) {
				if (!type->Sound.Dead[j].Name.empty()) {
					type->Sound.Dead[j].Sound =
						SoundForName(type->Sound.Dead[j].Name);
				}
			}
		}
	}
}

//@}
