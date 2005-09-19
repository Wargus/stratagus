//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//        Stratagus - A free fantasy real time strategy game engine
//
/**@name unitsound.cpp - The unit sounds. */
//
//      (c) Copyright 1999-2005 by Fabrice Rossi and Jimmy Salmon
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

/*----------------------------------------------------------------------------
--  Include
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
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
void LoadUnitSounds(void)
{
}

/**
**  Map animation sounds
*/
static void MapAnimSounds2(Animation* anim)
{
	int i;

	while (anim) {
		if (anim->Type == AnimationSound) {
			anim->D.Sound.Sound = SoundIdForName(anim->D.Sound.Name);
		} else if (anim->Type == AnimationRandomSound) {
			for (i = 0; i < anim->D.RandomSound.NumSounds; ++i) {
				anim->D.RandomSound.Sound[i] = SoundIdForName(anim->D.RandomSound.Name[i]);
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
	MapAnimSounds2(type->Animations->Death);
	MapAnimSounds2(type->Animations->Attack);
	MapAnimSounds2(type->Animations->Move);
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
void MapUnitSounds(void)
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

			if (type->Sound.Selected.Name) {
				type->Sound.Selected.Sound =
					SoundIdForName(type->Sound.Selected.Name);
			}
			if (type->Sound.Acknowledgement.Name) {
				type->Sound.Acknowledgement.Sound =
					SoundIdForName(type->Sound.Acknowledgement.Name);
				/*
				// Acknowledge sounds have infinite range
				SetSoundRange(type->Sound.Acknowledgement.Sound,
					INFINITE_SOUND_RANGE);
				*/
			}
			if (type->Sound.Ready.Name) {
				type->Sound.Ready.Sound =
					SoundIdForName(type->Sound.Ready.Name);
				// Ready sounds have infinite range
				SetSoundRange(type->Sound.Ready.Sound,
					INFINITE_SOUND_RANGE);
			}
			if (type->Sound.Repair.Name) {
				type->Sound.Repair.Sound =
					SoundIdForName(type->Sound.Repair.Name);
			}
			for (j = 0; j < MaxCosts; ++j) {
				if (type->Sound.Harvest[j].Name) {
					type->Sound.Harvest[j].Sound =
						SoundIdForName(type->Sound.Harvest[j].Name);
				}
			}
			if (type->Sound.Help.Name) {
				type->Sound.Help.Sound =
					SoundIdForName(type->Sound.Help.Name);
				// Help sounds have infinite range
				SetSoundRange(type->Sound.Help.Sound,
					INFINITE_SOUND_RANGE);
			}
			if (type->Sound.Dead.Name) {
				type->Sound.Dead.Sound =
					SoundIdForName(type->Sound.Dead.Name);
			}
		}
	}
}

//@}
