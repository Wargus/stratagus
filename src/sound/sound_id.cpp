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
/*
**	(c) Copyright 1998,2000 by Lutz Sammer and Fabrice Rossi
**
**	$Id$
*/

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
**	display the sound name hash table. 
*/
global void DisplaySoundHashTable(void) {
    fprintf(stderr,"Sound HashTable Begin\n");
    fprintf(stderr,__FUNCTION__": not written\n");
    fprintf(stderr,"Sound HashTable End\n");
}

/**
** Ask the sound server to register a sound ans tore the mapping
** between its name and its id.
*/
global SoundId MakeSound(char* sound_name,char* file[],unsigned char nb) {
    SoundId id;

    // ask the server to register the sound
    id=RegisterSound(file,nb);
    // save the mapping from sound_name to id in the hash table.
    MapSound(sound_name,id);
    return id;
}

/**
** maps a sound name to its id
*/
global SoundId SoundIdForName(const char* sound_name) {
    const SoundId* result;

    result=(const SoundId*)hash_find(SoundIdHash,(char*)sound_name);

    if (result) {
	return *result;
    }
    DebugLevel0("Can't find sound %s in sound table\n",sound_name);
    return NULL;
}

/**
 ** add a new mapping (sound name to sound id) in the hash table
*/
global void MapSound(char* sound_name,SoundId id) {
    *((SoundId*)hash_add(SoundIdHash,(char*)sound_name))=id;
}

/**
 ** ask the sound server to build a special sound group.
*/
global SoundId MakeSoundGroup(char* group_name,SoundId first,SoundId second) {
    SoundId sound;
    
    sound=RegisterTwoGroups(first,second);
    MapSound(group_name,sound);
    return sound;
}

#endif	// } WITH_SOUND

//@}
