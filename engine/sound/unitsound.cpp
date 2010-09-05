//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
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

#include "stratagus.h"
#include "sound.h"
#include "sound_server.h"
#include "unittype.h"
#include "animation.h"

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
static void MapAnimSounds2(CAnimation *anim)
{
	while (anim) {
		if (anim->Type == AnimationSound) {
			anim->D.Sound.Sound = SoundForName(anim->D.Sound.Name);
		} else if (anim->Type == AnimationRandomSound) {
			for (int i = 0; i < anim->D.RandomSound.NumSounds; ++i) {
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
	MapAnimSounds2(type->Animations->Harvest);
}

/**
**  Map the sounds of all unit-types to the correct sound id.
**  And overwrite the sound ranges. @todo the sound ranges should be
**  configurable by user with lua.
*/
void MapUnitSounds(void)
{
	if (SoundEnabled()) {
		//
		// Parse all units sounds.
		//
		for (size_t i = 0; i < UnitTypes.size(); ++i) {
			CUnitType *type = UnitTypes[i];

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
			if (!type->Sound.Ready.Name.empty()) {
				type->Sound.Ready.Sound =
					SoundForName(type->Sound.Ready.Name);
				// Ready sounds have infinite range
				SetSoundRange(type->Sound.Ready.Sound,
					INFINITE_SOUND_RANGE);
			}
			if (!type->Sound.Help.Name.empty()) {
				type->Sound.Help.Sound =
					SoundForName(type->Sound.Help.Name);
				// Help sounds have infinite range
				SetSoundRange(type->Sound.Help.Sound,
					INFINITE_SOUND_RANGE);
			}
			if (!type->Sound.Dead.Name.empty()) {
				type->Sound.Dead.Sound =
					SoundForName(type->Sound.Dead.Name);
			}
		}
	}
}

//@}
