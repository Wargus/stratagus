//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________      ______________________
//   T H E   W A R   B E G I N S
//    Stratagus - A free fantasy real time strategy game engine
//
/**@name unitsound.c - The unit sounds. */
//
// (c) Copyright 1999-2001,2003 by Fabrice Rossi and Jimmy Salmon
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
// $Id$

//@{

/*----------------------------------------------------------------------------
-- Include
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "sound_server.h"
#include "tileset.h"
#include "map.h"

/*----------------------------------------------------------------------------
-- Declarations
----------------------------------------------------------------------------*/

/**
** Simple sound definition:
** There is only one sound/voice that could be used for this
** sound identifier.
*/
typedef struct _simple_sound_ {
	char* Name; ///< name of the sound
	char* File; ///< corresponding sound file
} SimpleSound;

/**
** Structure for remaping a sound to a new name.
*/
typedef struct _sound_remap_ {
	char* NewName;  ///< Name in unit-type definition
	char* BaseName; ///< Name used in sound definition
} SoundRemap;

#define MaxSimpleGroups 7 ///< maximal number of sounds pro group

/**
** Sound group definition:
** There is a collection of sounds/voices that could be randomly
** be used fot this sound identifier.
*/
typedef struct _sound_group_ {
	char* Name;                     ///< name of the group
	char* Sounds[MaxSimpleGroups];  ///< list of sound files
} SoundGroup;

/**
** Selection structure:
**
** Special sound structure currently used for the selection of an unit.
** For a special number of the uses the first group is used, after this
** the second groups is played.
*/
typedef struct _selection_group_ {
	char* Name;    ///< name of the selection sound
	char* First;   ///< name of the sound
	char* Second;  ///< name of the annoyed sound
} SelectionGroup;

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/


/**
** Simple sounds currently available.
*/
static SimpleSound* SimpleSounds;

/**
** Sound remaping currently available.
*/
static SoundRemap* SoundRemaps;

/**
** Sound-groups currently available
*/
static SoundGroup* SoundGroups;

/**
** Selection-groups currently available
*/
static SelectionGroup* SelectionGroups;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
** Computes the number of sounds in a sound group
**
** @param group list of file names
**
** @return number of sounds in group
*/
static int NbSoundsInGroup(char* const* const group)
{
	int i;

	for (i = 0; i < MaxSimpleGroups; ++i) {
		if (!group[i]) {
			return i;
		}
	}
	return i;
}


/**
** Loads all simple sounds (listed in the SimpleSounds array).
*/
static void LoadSimpleSounds(void)
{
	int i;

	if (SimpleSounds) {
		for (i = 0; SimpleSounds[i].Name; ++i) {
			MakeSound(SimpleSounds[i].Name, (const char**)&(SimpleSounds[i].File), 1);
		}
	}
}

/**
** Loads all sound groups.
** Special groups are created.
*/
static void LoadSoundGroups(void)
{
	int i;

	if (SoundGroups) {
		for (i = 0; SoundGroups[i].Name; ++i) {
			MakeSound(SoundGroups[i].Name, (const char**)SoundGroups[i].Sounds,
				NbSoundsInGroup(SoundGroups[i].Sounds));
		}
	}
	if (SelectionGroups) {
		for (i = 0; SelectionGroups[i].Name; ++i) {
			//FIXME: might be more efficient
			MakeSoundGroup(SelectionGroups[i].Name,
				SoundIdForName(SelectionGroups[i].First),
				SoundIdForName(SelectionGroups[i].Second));
		}
	}
}

/**
** Performs remaping listed in the Remaps array. Maps also critter
** sounds to their correct values.
*/
static void RemapSounds(void)
{
	int i;

	if (SoundRemaps) {
		for (i = 0; SoundRemaps[i].NewName; ++i) {
			//FIXME: should be more efficient
			MapSound(SoundRemaps[i].NewName,
				SoundIdForName(SoundRemaps[i].BaseName));
		}
	}

	//
	// Make some general sounds.
	//
	// FIXME: move to config CCL
	MapSound("gold-mine-help", SoundIdForName("basic orc voices help 1"));

	// critter mapping FIXME: must support more terrains.

	switch (TheMap.Terrain) {
		case TilesetSummer:
			MakeSoundGroup("critter-selected",
			   SoundIdForName("sheep selected"),
			   SoundIdForName("sheep annoyed"));
			break;
		case TilesetWinter:
			MakeSoundGroup("critter-selected",
			   SoundIdForName("seal selected"),
			   SoundIdForName("seal annoyed"));
			break;
		case TilesetWasteland:
			MakeSoundGroup("critter-selected",
			   SoundIdForName("pig selected"),
			   SoundIdForName("pig annoyed"));
			break;
		case TilesetSwamp:
			MakeSoundGroup("critter-selected",
			   SoundIdForName("warthog selected"),
			   SoundIdForName("warthog annoyed"));
			break;
		default:
			break;
	}
}

/**
** Load all sounds for units.
*/
void LoadUnitSounds(void)
{
	if (SoundFildes != -1) {
		LoadSimpleSounds();
		LoadSoundGroups();
		RemapSounds();
	}
}

/**
** Map the sounds of all unit-types to the correct sound id.
** And overwrite the sound ranges. @todo the sound ranges should be
** configurable by user with CCL.
*/
void MapUnitSounds(void)
{
	UnitType* type;
	int i;
	int j;

	if (SoundFildes != -1) {
		SetSoundRange(SoundIdForName("tree chopping"), 32);

		//
		// Parse all units sounds.
		//
		for (i = 0; i < NumUnitTypes; ++i) {
			type = UnitTypes[i];
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
			// FIXME: will be modified, attack sound be moved to missile/weapon
			if (type->Weapon.Attack.Name) {
				type->Weapon.Attack.Sound =
					SoundIdForName(type->Weapon.Attack.Name);
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
