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
/**@name sound_id.h	-	Sound identifier client side header file. */
//
//	(c) Copyright 1999-2001 by Lutz Sammer and Fabrice Rossi
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __SOUND_ID_H__
#define __SOUND_ID_H__

//@{

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Sound referencing.
**
**	Client side representation of the sound id.
**	Passed to the sound server API.
*/
typedef void* SoundId;			/// sound identifier

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/** Register a sound group (or an unique sound if nb==1) and get the
    corresponding sound id.
    @param sound_name name of this sound group. MUST BE A PERMAMNENT STRING.
    @param file list of sound file names
    @param nb number of sounds
    @return the sound id of the created group
*/
extern SoundId MakeSound(char* sound_name,char* file[],unsigned char nb);

/** Get a sound id for a given sound name. Returns NULL when no sound is
    found.
    @param sound_name name of the sound.
    @return the corresponding sound id
*/
extern SoundId SoundIdForName(const char* sound_name);

/** Create a new mapping between a name and an already valid sound id
    @param sound_name the name. MUST BE A PERMANENT STRING.
    @param id the sound id.
 */
extern void MapSound(const char* sound_name,const SoundId id);

/** Register two sound groups together to make a special sound (for
    selection). Return the corresponding id after registering it under a given
    name.
    @param group_name the name of the group. MUST BE A PERMANENT STRING.
    @param first id of the first group.
    @param second id of the second group.
    @return id of the created group.
*/
extern SoundId MakeSoundGroup(char* group_name,SoundId first,SoundId second);

/** Helper function, displays the mapping between sound names and sound ids
 */
extern void DisplaySoundHashTable(void);


#ifndef WITH_SOUND	// {

#define SoundIdForName(n)	NULL	/// Dummy macro for without sound

#endif	// } WITH_SOUND

//@}

#endif	// !__SOUND_ID_H__
