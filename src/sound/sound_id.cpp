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

#include "stratagus.h"

#include <string>
#include <map>

#include "sound.h"
#include "sound_server.h"
#include "util.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static std::map<std::string, std::shared_ptr<CSound>, std::less<>> SoundMap;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

static std::shared_ptr<CSound> FindSound(const std::string_view &name)
{
	if (auto it = SoundMap.find(name); it != SoundMap.end()) {
		return (*it).second;
	}
	return nullptr;
}

/**
**  Add a new mapping (sound name to sound id) in the hash table
**  Create a new mapping between a name and an already valid sound id.
**
**  @param name  Name of the sound.
**  @param id    Sound identifier.
*/
void MapSound(const std::string &name, std::shared_ptr<CSound> id)
{
	if (!id) {
		DebugPrint("Null Sound for %s is not acceptable by sound table\n", name.c_str());
		return;
	}
	SoundMap[name] = std::move(id);
}

/**
**  Maps a sound name to its id
**
**  @param name  Sound name.
**
**  @return      Sound identifier for this name.
*/
std::shared_ptr<CSound> SoundForName(const std::string_view &name)
{
	Assert(!name.empty());
	if (std::shared_ptr<CSound> sound = FindSound(name)) {
		return sound;
	}
	DebugPrint("Can't find sound '%s' in sound table\n", name.data());
	return nullptr;
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
std::shared_ptr<CSound> MakeSound(const std::string &name, const std::vector<std::string> &files)
{
	if (auto sound = FindSound(name)) {
		DebugPrint("re-register sound '%s'\n", name.c_str());
		return sound;
	}

	if (auto sound = RegisterSound(files)) {
		MapSound(name, sound);
		return sound;
	}
	return nullptr;
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
std::shared_ptr<CSound> MakeSoundGroup(const std::string &name, CSound *first, CSound *second)
{
	if (auto sound = FindSound(name)) {
		DebugPrint("re-register sound '%s'\n", name.c_str());
		return sound;
	}
	if (auto sound = RegisterTwoGroups(first, second)) {
		MapSound(name, sound);
		return sound;
	}
	return nullptr;
}

void FreeSounds()
{
	SoundMap.clear();
}

//@}
