//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name sound_id.c	-	The sound id. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer and Fabrice Rossi
//
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	Stratagus is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include "stratagus.h"

#include <stdlib.h>
#include <string.h>

#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "sound_server.h"

#include "util.h"

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

#ifdef DOXYGEN								// no real code, only for document

/**
**		hash table used to store the mapping between sound name and sound id
*/
local int SoundIdHash[61];

#else

/**
**		hash table used to store the mapping between sound name and sound id
*/
local hashtable(int, 61) SoundIdHash;

#endif

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Display the sound name hash table.
*/
global void DisplaySoundHashTable(void)
{
	struct hash_st st;

	fprintf(stdout,"Sound HashTable Begin\n");
	PrintFunction();
	fprintf(stdout,"FIXME: not written\n");
	fprintf(stdout,"Sound HashTable End\n");

	hash_stat(SoundIdHash, &st);
	printf("nelem   : %d\n", st.nelem);
	printf("hashsize: %d\n", st.hashsize);
	printf("maxdepth: %d\n", st.maxdepth);
	printf("middepth: %d.%03d\n", st.middepth / 1000, st.middepth % 1000);

}

/**
**		Add a new mapping (sound name to sound id) in the hash table
**		Create a new mapping between a name and an already valid sound id.
**
**		@param name		Name of the sound (now freed by caller!).
**		@param id		Sound identifier.
*/
global void MapSound(const char* name, const SoundId id)
{
	*((SoundId*)hash_add(SoundIdHash, (char*)name)) = id;
}

/**
**		Maps a sound name to its id
**
**		@param name		Sound name.
**
**		@return				Sound idenfier for this name.
*/
global SoundId SoundIdForName(const char* name)
{
	const SoundId* result;

	Assert(name);

	if ((result = (const SoundId*)hash_find(SoundIdHash, (char*)name))) {
		return *result;
	}
	DebugPrint("Can't find sound `%s' in sound table\n" _C_ name);
	return NULL;
}

/**
**		Ask the sound server to register a sound and store the mapping
**		between its name and its id.
**		Register a sound group (or an unique sound if nb==1) and get the
**		corresponding sound id.
**
**		@param name		name of this sound group (Freed by caller).
**		@param file		list of sound file names
**		@param nb		number of sounds
**
**		@return the sound id of the created group
*/
global SoundId MakeSound(const char* name, const char* file[], int nb)
{
	SoundId id;
	const SoundId* result;

	Assert(nb <= 255);

	if ((result = (const SoundId*)hash_find(SoundIdHash, (char*)name))) {
		DebugPrint("re-register sound `%s'\n" _C_ name);
		return *result;
	}

	// ask the server to register the sound
	id = RegisterSound(file, nb);
	// save the mapping from name to id in the hash table.
	MapSound(name, id);
	return id;
}

/**
**		Ask the sound server to build a special sound group.
**
**		Register two sound groups together to make a special sound (for
**		selection). Return the corresponding id after registering it under a
**		given name.
**
**		@param name		the name of the group (handled by caller).
**		@param first		id of the first group
**		@param second		id of the second group
**
**		@return				Registered sound identifier.
*/
global SoundId MakeSoundGroup(const char* name, SoundId first, SoundId second)
{
	SoundId sound;
	const SoundId* result;

	if ((result = (const SoundId*)hash_find(SoundIdHash, (char*)name))) {
		DebugPrint("re-register sound `%s'\n" _C_ name);
		return *result;
	}

	sound = RegisterTwoGroups(first, second);
	MapSound(name, sound);

	return sound;
}

//@}
