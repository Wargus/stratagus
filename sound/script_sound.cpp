//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name ccl_sound.c	-	The sound ccl functions. */
//
//	(c) Copyright 1999-2003 by Lutz Sammer and Fabrice Rossi
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
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
**
**	@param name	name
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

/**
**	Create a sound.
**
**	Glue between c and scheme. This function asks the sound system to
**	register a sound under a given name, wiht an associated list of files
**	(the list can be replaced by only one file).
**
**	@param name	the name of the sound
**	@param file	a list of sound file names (or a file name)
**
**	@return		the sound id of the created sound
*/
local SCM CclMakeSound(SCM name, SCM file)
{
    SoundId id;
    char* c_name;
    char* c_file;
    char** c_files;
    int nb;
    int i;
    SCM a_file;

    if (!gh_string_p(name)) {
	fprintf(stderr, "string expected\n");
	return SCM_UNSPECIFIED;
    }
    if (gh_string_p(file)) {
	// only one file
	c_name = gh_scm2newstr(name, NULL);
	c_file = gh_scm2newstr(file, NULL);
	id = MakeSound(c_name, &c_file, 1);
	DebugLevel3("Making sound `%s' from `%s' with id %p\n" _C_ c_name _C_
	    c_file _C_ id);
	// the sound name (c_name) must be kept but the file name can be freed
	// JOHNS: wrong!
	free(c_file);
	free(c_name);
    } else if (gh_list_p(file)) {
	// several files
	c_name = gh_scm2newstr(name, NULL);
	DebugLevel3("Making sound `%s'\n" _C_ c_name);
	nb = gh_length(file);
	c_files = (char **)malloc(sizeof(char *) * nb);
	for (i = 0; i < nb; i++) {
	    a_file = gh_car(file);
	    if (!gh_string_p(name)) {
		fprintf(stderr, "string expected\n");
		// FIXME: memory leak!
		return SCM_UNSPECIFIED;
	    }
	    c_files[i] = gh_scm2newstr(a_file, NULL);
	    DebugLevel3("\tComponent %d: `%s'\n" _C_ i _C_ c_files[i]);
	    file = gh_cdr(file);
	}
	//FIXME: check size before casting
	id = MakeSound(c_name, c_files, (unsigned char)nb);
	for (i = 0; i < nb; i++) {
	    free(c_files[i]);
	}
	free(c_name);
	free(c_files);
    } else {
	fprintf(stderr, "string or list expected\n");
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
local SCM CclMakeSoundGroup(SCM name, SCM first, SCM second)
{
    SoundId id;
    char* c_name;

    if (!gh_string_p(name) && !gh_symbol_p(name)) {
	fprintf(stderr, "string or symbol expected\n");
	return SCM_UNSPECIFIED;
    }
    c_name = gh_scm2newstr(name, NULL);

    id = MakeSoundGroup(c_name, CclGetSoundId(first), CclGetSoundId(second));
    // JOHNS: not anymore: c_name consumed by MakeSoundGroup!
    free(c_name);
    return sound_id_ccl(id);
}

/**
**	Glue between c and scheme. Ask to the sound system to remap a sound id
**	to a given name.
**
**	@param name	the new name for the sound
**	@param sound	the sound object
**
**	@return		the sound object
*/
local SCM CclMapSound(SCM name, SCM sound)
{
    char* sound_name;

    sound_name = gh_scm2newstr(name, NULL);
    MapSound(sound_name, CclGetSoundId(sound));
    free(sound_name);
    return sound;
}

/**
**	Ask the sound system to play the specified sound.
**
**	@param sound	either the sound name or the sound id
**
**	@return		SCM_UNSPECIFIED
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
**	Glue between c and scheme. Ask the sound system to dump on the
**	standard output the mapping between sound names and sound id.
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
	    PrintFunction();
	    fprintf(stdout,"Symbol expected\n");
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
**	Set cd mode
**
**	@param mode	cd mode
*/
local SCM CclSetCdMode(SCM mode)
{
#if defined(USE_SDLCD) || defined(USE_LIBCDA) || defined(USE_CDDA)
    CDModes cdmode;

    if( gh_eq_p(mode,gh_symbol2scm("all")) ) {
	cdmode=CDModeAll;
    } else if( gh_eq_p(mode,gh_symbol2scm("random")) ) {
	cdmode=CDModeRandom;
    } else if( gh_eq_p(mode,gh_symbol2scm("defined")) ) {
	cdmode=CDModeDefined;
    } else if ( gh_eq_p(mode,gh_symbol2scm("off")) ) {
	cdmode=CDModeOff;
    } else {
	cdmode=CDModeOff;
	errl("Unsupported tag",mode);
    }

    PlayCDRom(cdmode);
#endif
    return mode;
}

/**
**	Define play sections
*/
local SCM CclDefinePlaySections(SCM list)
{
    SCM value;
    SCM sublist;
    PlaySection *p;
    int i;

    ++NumPlaySections;
    PlaySections=realloc(PlaySections,NumPlaySections*sizeof(PlaySection));
    p=PlaySections+NumPlaySections-1;
    memset(p,0,sizeof(PlaySection));

    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	if( gh_eq_p(value,gh_symbol2scm("race")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    p->Race=gh_scm2newstr(value,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("type")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    if( gh_eq_p(value, gh_symbol2scm("game")) ) {
		p->Type=PlaySectionGame;
	    } else if( gh_eq_p(value,gh_symbol2scm("briefing")) ) {
		p->Type=PlaySectionBriefing;
	    } else if( gh_eq_p(value,gh_symbol2scm("stats-victory")) ) {
		p->Type=PlaySectionStatsVictory;
	    } else if( gh_eq_p(value,gh_symbol2scm("stats-defeat")) ) {
		p->Type=PlaySectionStatsDefeat;
	    } else if( gh_eq_p(value,gh_symbol2scm("main-menu")) ) {
		p->Type=PlaySectionMainMenu;
	    } else {
		errl("Unsupported tag",value);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("cd")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    while( !gh_null_p(sublist) ) {
		value=gh_car(sublist);
		sublist=gh_cdr(sublist);
		if( gh_eq_p(value,gh_symbol2scm("order")) ) {
		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);
		    if( gh_eq_p(value,gh_symbol2scm("all")) ) {
			p->CDOrder=PlaySectionOrderAll;
		    } else if( gh_eq_p(value,gh_symbol2scm("random")) ) {
			p->CDOrder=PlaySectionOrderRandom;
		    } else {
			errl("Unsupported tag",value);
		    }
		} else if( gh_eq_p(value,gh_symbol2scm("tracks")) ) {
		    SCM temp;

		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);
		    for( i=0; i<gh_vector_length(value); ++i ) {
			temp=gh_vector_ref(value,gh_int2scm(i));
			p->CDTracks|=(1<<gh_scm2int(temp));
		    }
		} else {
		    errl("Unsupported tag",value);
		}
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("no-cd")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    while( !gh_null_p(sublist) ) {
		value=gh_car(sublist);
		sublist=gh_cdr(sublist);
		if( gh_eq_p(value,gh_symbol2scm("order")) ) {
		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);
		    if( gh_eq_p(value,gh_symbol2scm("all")) ) {
			p->FileOrder=PlaySectionOrderAll;
		    } else if( gh_eq_p(value,gh_symbol2scm("random")) ) {
			p->FileOrder=PlaySectionOrderRandom;
		    } else {
			errl("Unsupported tag",value);
		    }
		} else if( gh_eq_p(value,gh_symbol2scm("files")) ) {
		    SCM sublist2;

		    sublist2=gh_car(sublist);
		    sublist=gh_cdr(sublist);
		    i=0;
		    while( !gh_null_p(sublist2) ) {
			value=gh_car(sublist2);
			sublist2=gh_cdr(sublist2);
			++i;
			p->Files=realloc(p->Files,(i+1)*sizeof(char*));
			p->Files[i-1]=gh_scm2newstr(value,NULL);
			p->Files[i]=NULL;
		    }
		} else {
		    errl("Unsupported tag",value);
		}
	    }
	} else {
	    errl("Unsupported tag",value);
	}
    }

    return SCM_UNSPECIFIED;
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
**	Turn Off Music (client side)
*/
local SCM CclMusicOff(void)
{
    StopMusic();
    MusicOff=1;
    return SCM_UNSPECIFIED;
}

/**
**	Turn On Music (client side)
**
**	@return true if and only if the sound is REALLY turned on
**		(uses SoundFildes)
*/
local SCM CclMusicOn(void)
{
    MusicOff=0;
    return SCM_UNSPECIFIED;
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
    if (d>0) {
	DistanceSilent=d;
    }
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

/**
**	Set the range of a given sound.
**
**	@param sound	the sound id or name of the sound
**	@param range	the new range for this sound
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
    DebugLevel3("Range: %u (%d)\n" _C_ TheRange _C_ tmp);
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
**	Play a sound file.
**
**	@param name	Name of the sound file to play.
*/
local SCM CclPlayFile(SCM name)
{
    char* filename;

    filename=gh_scm2newstr(name,NULL);
    PlayFile(filename);
    free(filename);

    return SCM_UNSPECIFIED;
}

/**
**	Stop playing music.
*/
local SCM CclStopMusic(void)
{
    StopMusic();

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
    gh_new_procedure1_0("set-cd-mode!",CclSetCdMode);

    gh_new_procedureN("define-play-sections",CclDefinePlaySections);

    init_subr_0("sound-off",CclSoundOff);
    init_subr_0("sound-on",CclSoundOn);
    init_subr_0("music-off",CclMusicOff);
    init_subr_0("music-on",CclMusicOn);
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
    gh_new_procedure1_0("play-file",CclPlayFile);
    gh_new_procedure0_0("stop-music",CclStopMusic);
}

#else	// }{ WITH_SOUND

#include "ccl.h"

/**
**	Global volume support
**
**	@param volume	new global sound volume
*/
local SCM CclSetSoundVolume(SCM volume)
{
    return volume;
}

/**
**	Music volume support
**
**	@param volume	new global music volume
*/
local SCM CclSetMusicVolume(SCM volume)
{
    return volume;
}

/**
**	Set cd mode
**
**	@param mode	cd mode
*/
local SCM CclSetCdMode(SCM mode)
{
    return mode;
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
**	Turn Off Music (client side)
*/
local SCM CclMusicOff(void)
{
    return SCM_UNSPECIFIED;
}

/**
**	Turn On Music (client side)
**
**	@return true if and only if the sound is REALLY turned on
**		(uses SoundFildes)
*/
local SCM CclMusicOn(void)
{
    return SCM_UNSPECIFIED;
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
**	@param sound	the sound id or name of the sound
**	@param range	the new range for this sound
*/
local SCM CclSetSoundRange(SCM sound,SCM range __attribute__((unused)))
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
local SCM CclSoundForName(SCM name __attribute__((unused)))
{
    return NIL;
}

/**
**	Glue between c and scheme. Allows to specify some global game sounds
**	in a ccl file.
*/
local SCM CclDefineGameSounds(SCM list __attribute__((unused)))
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
**	@return		the sound object
*/
local SCM CclMapSound(SCM name __attribute__((unused)),SCM sound)
{
    return sound;
}

/**
**	Play a music file.
**
**	@param name	Name of the music file to play.
*/
local SCM CclPlayMusic(SCM name __attribute__((unused)))
{
    return SCM_UNSPECIFIED;
}

/**
**	Play a sound file.
**
**	@param name	Name of the sound file to play.
*/
local SCM CclPlayFile(SCM name __attribute__((unused)))
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
    gh_new_procedure1_0("set-cd-mode!",CclSetCdMode);
    gh_new_procedure0_0("sound-off",CclSoundOff);
    gh_new_procedure0_0("sound-on",CclSoundOn);
    gh_new_procedure0_0("music-off",CclMusicOff);
    gh_new_procedure0_0("music-on",CclMusicOn);
    gh_new_procedure0_0("sound-thread",CclSoundThread);
    gh_new_procedure1_0("set-global-sound-range!",CclSetGlobalSoundRange);
    gh_new_procedureN("define-game-sounds",CclDefineGameSounds);
    gh_new_procedure0_0("display-sounds",CclDisplaySounds);
    gh_new_procedure2_0("map-sound",CclMapSound);
    gh_new_procedure1_0("sound-for-name",CclSoundForName);
    gh_new_procedure2_0("set-sound-range!",CclSetSoundRange);

    gh_new_procedure1_0("play-music",CclPlayMusic);
    gh_new_procedure1_0("play-file",CclPlayFile);
}

#endif	// } !WITH_SOUND

//@}
