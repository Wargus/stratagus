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
/**@name ccl_sound.c	-	The sound ccl functions. */
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

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"

#ifdef WITH_SOUND	// {

#include <stdlib.h>

#include "ccl.h"
#include "sound_id.h"
#include "sound.h"
#include "sound_server.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
** C representation for the siod sound type
** ALPHA VERSION!!!!!!!!!
*/
local long SiodSoundTag;

#define CCL_SOUNDP(x)	TYPEP(x,SiodSoundTag)
#define CCL_SOUND_ID(x) ( (SoundId)CDR(x) )

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Cast a FreeCraft sound id to its scheme version
**
**	@param id	the sound id
**
**	@return		its siod version
*/
local SCM sound_id_ccl(SoundId id)
{
     SCM sound_id;

     sound_id=cons(NIL,NIL);
     sound_id->type=SiodSoundTag;
     sound_id->storage_as.cons.cdr=(SCM)id;

     return sound_id;
}

/**
**	Glue between c and scheme. Ask the sound system to associate a
**	sound id to a sound name.
*/
local SCM CclSoundForName(SCM name)
{
    SoundId id;
    char* sound_name;

    sound_name=gh_scm2newstr(name,NULL);
    id=SoundIdForName(sound_name);
    free(sound_name);

    return sound_id_ccl(id);
}


/**
**	Get a Game Sound Id from either a siod sound id or a sound name
**
**	@param sound	Lisp cell, SoundID or string or symbol.
**	@return		The C sound id.
*/
local SoundId CclGetSoundId(SCM sound)
{
    if (CCL_SOUNDP(sound)) {	// if we've got the sound id
	return CCL_SOUND_ID(sound);
    } else {
	return CCL_SOUND_ID(CclSoundForName(sound));
    }
}

/** Glue between c and scheme. This function asks the sound system to register
    a sound under a given name, wiht an associated list of files (the list can
    be replaced by only one file).
    @param name the name of the sound
    @param file a list of sound file names (or a file name)
    @return the sound id of the created sound

*/
local SCM CclMakeSound(SCM name,SCM file) {
    SoundId id;
    char* c_name;
    char* c_file;
    char** c_files;
    int nb,i;
    SCM a_file;

    if( !gh_string_p(name) ) {
	fprintf(stderr,"string expected\n");
	return SCM_UNSPECIFIED;
    }
    if ( gh_string_p(file)) {
	// only one file
	c_name=gh_scm2newstr(name,NULL);
	c_file=gh_scm2newstr(file,NULL);
	id=MakeSound(c_name,&c_file,1);
	DebugLevel3("Making sound `%s' from `%s' with id %p\n",c_name,c_file,id);
	// the sound name (c_name) must be kept but the file name can be freed
	free(c_file);
    } else if (gh_list_p(file)) {
	// several files
	c_name=gh_scm2newstr(name,NULL);
	DebugLevel3("Making sound `%s'\n",c_name);
	nb=gh_length(file);
	c_files=(char **)malloc(sizeof(char*)*nb);
	for(i=0;i<nb;i++) {
	    a_file=gh_car(file);
	    if( !gh_string_p(name) ) {
		fprintf(stderr,"string expected\n");
		return SCM_UNSPECIFIED;
	    }
	    c_files[i]=gh_scm2newstr(a_file,NULL);
	    DebugLevel3("\tComponent %d: `%s'\n",i,c_files[i]);
	    file=gh_cdr(file);
	}
	//FIXME: check size before casting
	id=MakeSound(c_name,c_files,(unsigned char)nb);
	for(i=0;i<nb;i++)
	    free(c_files[i]);
	free(c_files);
    } else {
	fprintf(stderr,"string or list expected\n");
	return SCM_UNSPECIFIED;
    }
    return sound_id_ccl(id);
}

/**
**	Glue between c and scheme. This function asks the sound system to
**	build a special sound group.
**
**	@param name	the name of the sound
**	@param first	first group played (sound-id or string)
**	@param second	second group played (sound-id or string)
**
**	@return		The sound id of the created sound
*/
local SCM CclMakeSoundGroup(SCM name,SCM first,SCM second)
{
    char* c_name;

    if( !gh_string_p(name) && !gh_symbol_p(name) ) {
	fprintf(stderr,"string or symbol expected\n");
	return SCM_UNSPECIFIED;
    }
    c_name=gh_scm2newstr(name,NULL);

    return sound_id_ccl(MakeSoundGroup(c_name,
	    CclGetSoundId(first),CclGetSoundId(second)));
    // c_name consumed by MakeSoundGroup!
}

/** Glue between c and scheme. Ask to the sound system to remap a sound id to
    a given name.
    @param name the new name for the sound
    @param sound the sound object
    @return the sound object
*/
local SCM CclMapSound(SCM name,SCM sound) {
    char* sound_name;

    sound_name=gh_scm2newstr(name,NULL);
    MapSound(sound_name,CclGetSoundId(sound));
    return sound;
}

/** Ask to the sound system to play the specified sound.
    @param sound either the sound name or the sound id
    @return SCM_UNSPECIFIED
*/
local SCM CclPlaySound(SCM sound) {
    SoundId id;

    id=CclGetSoundId(sound);
    PlayGameSound(id,MaxSampleVolume);
    return SCM_UNSPECIFIED;
}

/**
**	Test whether a scheme object is a clone sound id
**
**	@param sound	the scheme object
**
**	@return		true is sound is a clone sound id
*/
global int ccl_sound_p(SCM sound)
{
    return CCL_SOUNDP(sound);
}

/**
**	Cast a scheme object to a clone sound id
**
**	@param sound	the scheme object
**
**	@return		the clone sound id
*/
global SoundId ccl_sound_id(SCM sound)
{
    return CCL_SOUND_ID(sound);
}

/**
**	Glue between c and scheme. Ask the sound system to dump on the standard
**	output the mapping between sound names and sound id.
*/
local SCM CclDisplaySounds(void) {
    DisplaySoundHashTable();
    return SCM_UNSPECIFIED;
}

/**
**	Glue between c and scheme. Allows to specify some global game sounds
**	in a ccl file.
*/
local SCM CclDefineGameSounds(SCM list)
{
    //based on Johns CclGameMap function
    //FIXME: need much more error checking and handling
    //FIXME: should allow to define ALL the game sounds
    SCM name;
    SCM data;
    char* str;

    while( !gh_null_p(list) ) {

	name=gh_car(list);
	list=gh_cdr(list);
	if( !gh_symbol_p(name) ) {
	    fprintf(stderr,__FUNCTION__": symbol expected\n");
	    return list;
	}
	// prepare for next iteration
	data=gh_car(list);
	list=gh_cdr(list);

	// let's handle now the different cases
	if( gh_eq_p(name,gh_symbol2scm("click")) ) {
	    if ( !CCL_SOUNDP(data) ) {
		fprintf(stderr,"Sound id expected\n");
		return list;
	    }
	    GameSounds.Click.Sound=CCL_SOUND_ID(data);
	} else if ( gh_eq_p(name,gh_symbol2scm("placement-error")) ) {
	    if ( !CCL_SOUNDP(data) ) {
		fprintf(stderr,"Sound id expected\n");
		return list;
	    }
	    GameSounds.PlacementError.Sound=CCL_SOUND_ID(data);
	} else if ( gh_eq_p(name,gh_symbol2scm("placement-success")) ) {
	    if ( !CCL_SOUNDP(data) ) {
		fprintf(stderr,"Sound id expected\n");
		return list;
	    }
	    GameSounds.PlacementSuccess.Sound=CCL_SOUND_ID(data);
	} else if ( gh_eq_p(name,gh_symbol2scm("repair")) ) {
	    if ( !CCL_SOUNDP(data) ) {
		GameSounds.Repair.Sound=(void*)-1;
	    } else {
		GameSounds.Repair.Sound=CCL_SOUND_ID(data);
	    }
	} else if ( gh_eq_p(name,gh_symbol2scm("human-rescue")) ) {
	    if ( !CCL_SOUNDP(data) ) {
		fprintf(stderr,"Sound id expected\n");
		return list;
	    }
	    GameSounds.HumanRescue.Sound=CCL_SOUND_ID(data);
	} else if ( gh_eq_p(name,gh_symbol2scm("orc-rescue")) ) {
	    if ( !CCL_SOUNDP(data) ) {
		fprintf(stderr,"Sound id expected\n");
		return list;
	    }
	    GameSounds.OrcRescue.Sound=CCL_SOUND_ID(data);
	} else {
	    fprintf(stderr,"Incorrect symbol %s\n",
		    str=gh_scm2newstr(name,NULL));
	    free(str);
	    return list;
	}
    }
    return SCM_UNSPECIFIED;
}

/**
**	Global volume support
**
**	@param volume	new global sound volume
*/
local SCM CclSetSoundVolume(SCM volume)
{
    SetGlobalVolume(gh_scm2int(volume));
    return volume;
}

/**
**	Music volume support
**
**	@param volume	new global music volume
*/
local SCM CclSetMusicVolume(SCM volume)
{
    SetMusicVolume(gh_scm2int(volume));
    return volume;
}

/**
**	Turn Off Sound (client side)
*/
local SCM CclSoundOff(void)
{
    SoundOff=1;
    return SCM_UNSPECIFIED;
}

/**
**	Turn On Sound (client side)
**
**	@return true if and only if the sound is REALLY turned on
**		(uses SoundFildes)
*/
local SCM CclSoundOn(void)
{
    if (SoundFildes != -1) {
	return SCM_BOOL_T;
    }
    SoundOff=0;
    return SCM_BOOL_F;
}

/**
**	Set the cut off distance.
**
**	@param distance new cut off distance for sounds
*/
local SCM CclSetGlobalSoundRange(SCM distance) {
    int d;
    //FIXME check for errors
    d=gh_scm2int(distance);
    if (d>0)
	DistanceSilent=d;
    return distance;
}

/**
**	Ask clone to use a sound thread
*/
local SCM CclSoundThread(void)
{
#ifdef USE_THREAD
    WithSoundThread=1;
#endif
    return SCM_UNSPECIFIED;
}

/** Set the range of a given sound.
    @param sound the sound id or name of the sound
    @range the new range for this sound
 */
local SCM CclSetSoundRange(SCM sound,SCM range) {
    //FIXME check for errors
    unsigned char TheRange;
    int tmp;
    SoundId id;

    tmp=gh_scm2int(range);
    if(tmp<0) {
	TheRange=0;
    } else if (tmp>255) {
	TheRange=255;
    } else {
	TheRange=(unsigned char)tmp;
    }
    DebugLevel3("Range: %u (%d)\n",TheRange,tmp);
    id=CclGetSoundId(sound);
    SetSoundRange(id,TheRange);
    return sound;
}

/**
**	Play a music file.
**
**	@param name	Name of the music file to play.
*/
local SCM CclPlayMusic(SCM name)
{
    char* music_name;

    music_name=gh_scm2newstr(name,NULL);
    PlayMusic(music_name);
    free(music_name);

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for sound.
*/
global void SoundCclRegister(void)
{
    SiodSoundTag=allocate_user_tc();

    gh_new_procedure1_0("set-sound-volume!",CclSetSoundVolume);
    gh_new_procedure1_0("set-music-volume!",CclSetMusicVolume);

    init_subr_0("sound-off",CclSoundOff);
    init_subr_0("sound-on",CclSoundOn);
    init_subr_0("sound-thread",CclSoundThread);
    init_subr_1("set-global-sound-range!",CclSetGlobalSoundRange);
    init_lsubr("define-game-sounds",CclDefineGameSounds);
    init_subr_0("display-sounds",CclDisplaySounds);
    init_subr_2("map-sound",CclMapSound);
    init_subr_1("sound-for-name",CclSoundForName);
    init_subr_2("set-sound-range!",CclSetSoundRange);
    init_subr_2("make-sound",CclMakeSound);
    init_subr_3("make-sound-group",CclMakeSoundGroup);
    init_subr_1("play-sound",CclPlaySound);

    gh_new_procedure1_0("play-music",CclPlayMusic);
}

#else	// }{ WITH_SOUND

#include "ccl.h"

/**
**	Global volume support
**
**	@param volume new global sound volume
*/
local SCM CclSetSoundVolume(SCM volume)
{
    return volume;
}

/**
**	Turn Off Sound (client side)
*/
local SCM CclSoundOff(void)
{
    return SCM_UNSPECIFIED;
}

/**
**	Turn On Sound (client side)
**
**	@return true if and only if the sound is REALLY turned on
**		(uses SoundFildes)
*/
local SCM CclSoundOn(void)
{
    return SCM_BOOL_T;
}

/**
**	Set the cut off distance.
**
**	@param distance new cut off distance for sounds
*/
local SCM CclSetGlobalSoundRange(SCM distance)
{
    return distance;
}

/**
**	Set the range of a given sound.
**
**	@param sound the sound id or name of the sound
*	@range the new range for this sound
*/
local SCM CclSetSoundRange(SCM sound,SCM range)
{
    return sound;
}

/**
**	Ask clone to use a sound thread
*/
local SCM CclSoundThread(void)
{
    return SCM_UNSPECIFIED;
}

/**
**	Glue between c and scheme. Ask the sound system to dump on the
**	standard output the mapping between sound names and sound id.
*/
local SCM CclDisplaySounds(void)
{
    return SCM_UNSPECIFIED;
}

/**
**	Glue between c and scheme. Ask the sound system to associate a sound
**	id to a sound name.
*/
local SCM CclSoundForName(SCM name)
{
    return NIL;
}

/**
**	Glue between c and scheme. Allows to specify some global game sounds
**	in a ccl file.
*/
local SCM CclDefineGameSounds(SCM list)
{
    return NIL;
}

/**
**	Glue between c and scheme. Ask to the sound system to remap a sound id
**	to a given name.
**
**	@param name	the new name for the sound
**	@param sound	the sound object
**
**	@return the sound object
*/
local SCM CclMapSound(SCM name,SCM sound)
{
    return sound;
}

/**
**	Play a music file.
**
**	@param name	Name of the music file to play.
*/
local SCM CclPlayMusic(SCM name)
{
    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for sound. Dummy version.
*/
global void SoundCclRegister(void)
{
    gh_new_procedure1_0("set-sound-volume!",CclSetSoundVolume);
    gh_new_procedure1_0("set-music-volume!",CclSetMusicVolume);
    gh_new_procedure0_0("sound-off",CclSoundOff);
    gh_new_procedure0_0("sound-on",CclSoundOn);
    gh_new_procedure0_0("sound-thread",CclSoundThread);
    gh_new_procedure1_0("set-global-sound-range!",CclSetGlobalSoundRange);
    gh_new_procedureN("define-game-sounds",CclDefineGameSounds);
    gh_new_procedure0_0("display-sounds",CclDisplaySounds);
    gh_new_procedure2_0("map-sound",CclMapSound);
    gh_new_procedure1_0("sound-for-name",CclSoundForName);
    gh_new_procedure2_0("set-sound-range!",CclSetSoundRange);

    gh_new_procedure1_0("play-music",CclPlayMusic);
}

#endif	// } !WITH_SOUND

//@}
