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
/**@name sound_id.cpp - The sound id. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Fabrice Rossi
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#include <string>
#include <map>

#include "sound.h"
#include "sound_server.h"
#include "util.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static std::map<std::string, CSound *> SoundMap;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

static CSound *FindSound(const std::string &name)
{
	std::map<std::string, CSound *>::iterator ret = SoundMap.find(name);
	if (ret != SoundMap.end()) {
		return (*ret).second;
	}
	return NULL;
}

/**
**  Add a new mapping (sound name to sound id) in the hash table
**  Create a new mapping between a name and an already valid sound id.
**
**  @param name  Name of the sound (now freed by caller!).
**  @param id    Sound identifier.
*/
void MapSound(const std::string &name, CSound *id)
{
	if (!id) {
		DebugPrint("Null Sound for %s is not acceptable by sound table\n" _C_ name.c_str());
		return;
	}
	id->Mapref++;
	SoundMap[name] = id;
}

/**
**  Maps a sound name to its id
**
**  @param name  Sound name.
**
**  @return      Sound identifier for this name.
*/
CSound *SoundForName(const std::string &name)
{
	Assert(!name.empty());
	CSound *sound = FindSound(name);
	if (sound) {
		return sound;
	}
	DebugPrint("Can't find sound `%s' in sound table\n" _C_ name.c_str());
	return NULL;
}

/**
**  Ask the sound server to register a sound and store the mapping
**  between its name and its id.
**  Register a sound group (or an unique sound if nb==1) and get the
**  corresponding sound id.
**
**  @param name  name of this sound group (Freed by caller).
**  @param file  list of sound file names
**  @param nb    number of sounds
**
**  @return      the sound id of the created group
*/
CSound *MakeSound(const std::string &name, const char *file[], int nb)
{
	CSound *sound;

	Assert(nb <= 255);

	if ((sound = FindSound(name))) {
		DebugPrint("re-register sound `%s'\n" _C_ name.c_str());
		return sound;
	}

	sound = RegisterSound(file, nb);
	if (sound != NO_SOUND) {
		MapSound(name, sound);
	}

	return sound;
}

/**
**  Ask the sound server to build a special sound group.
**
**  Register two sound groups together to make a special sound (for
**  selection). Return the corresponding id after registering it under a
**  given name.
**
**  @param name    the name of the group (handled by caller).
**  @param first   id of the first group
**  @param second  id of the second group
**
**  @return        Registered sound identifier.
*/
CSound *MakeSoundGroup(const std::string &name, CSound *first, CSound *second)
{
	CSound *sound;

	if ((sound = FindSound(name))) {
		DebugPrint("re-register sound `%s'\n" _C_ name.c_str());
		return sound;
	}

	sound = RegisterTwoGroups(first, second);
	if (sound != NO_SOUND) {
		MapSound(name, sound);
	}

	return sound;
}

#ifdef DEBUG
void FreeSounds()
{
	std::map<std::string, CSound *>::iterator i;
	for (i = SoundMap.begin(); i != SoundMap.end(); ++i) {
		CSound *sound = (*i).second;
		Assert(sound && sound->Mapref != 0);
		if (sound && !--sound->Mapref) {
			delete sound;
		}
	}
}
#endif

//@}
