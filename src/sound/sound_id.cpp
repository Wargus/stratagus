//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name sound_id.c	-	The sound id. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer and Fabrice Rossi
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include "freecraft.h"

#ifdef WITH_SOUND	// {

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "sound_server.h"

#include "etlib/hash.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	hash table used to store the mapping between sound name and sound id
*/
local hashtable(int,61) SoundIdHash;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Display the sound name hash table.
*/
global void DisplaySoundHashTable(void)
{
    fprintf(stderr,"Sound HashTable Begin\n");
    fprintf(stderr,__FUNCTION__": not written\n");
    fprintf(stderr,"Sound HashTable End\n");
}

/**
**	Ask the sound server to register a sound ans tore the mapping
**	between its name and its id.
**	Register a sound group (or an unique sound if nb==1) and get the
**	corresponding sound id.
**
**	@param name	name of this sound group. MUST BE A PERMAMNENT STRING.
**	@param file	list of sound file names
**	@param nb	number of sounds
**
**	@return the sound id of the created group
*/
global SoundId MakeSound(char* name,char* file[],unsigned char nb)
{
    SoundId id;
    const SoundId* result;

    if ( (result=(const SoundId*)hash_find(SoundIdHash,(char*)name)) ) {
	DebugLevel0Fn("re-register sound `%s'\n",name);
	return *result;
    }
    // ask the server to register the sound
    id=RegisterSound(file,nb);
    // save the mapping from name to id in the hash table.
    MapSound(name,id);
    return id;
}

/**
**	Maps a sound name to its id
**
**	@param name	Sound name.
**
**	@return		Sound idenfier for this name.
*/
global SoundId SoundIdForName(const char* name)
{
    const SoundId* result;
    
    DebugCheck( !name );

    if( (result=(const SoundId*)hash_find(SoundIdHash,(char*)name)) ) {
	return *result;
    }
    DebugLevel0("Can't find sound `%s' in sound table\n",name);
    return NULL;
}

/**
**	Add a new mapping (sound name to sound id) in the hash table
**
**	@param name	Name of the sound (constant or malloced).
**	@param id	Sound identifier.
*/
global void MapSound(const char* name,const SoundId id)
{
    *((SoundId*)hash_add(SoundIdHash,(char*)name))=id;
}

/**
**	Ask the sound server to build a special sound group.
**
**	@param name	the name of the group. MUST BE A PERMANENT STRING.
**	@param first	id of the first group
**	@param second	id of the second group
**
**	@return		Registered sound identifier.
*/
global SoundId MakeSoundGroup(char* group_name,SoundId first,SoundId second)
{
    SoundId sound;

    sound=RegisterTwoGroups(first,second);
    MapSound(group_name,sound);
    return sound;
}

#endif	// } WITH_SOUND

//@}
