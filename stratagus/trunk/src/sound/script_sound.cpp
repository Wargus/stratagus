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
/*
**	(c) Copyright 1999,2000 by Lutz Sammer and Fabrice Rossi
**
**	$Id$
*/

//@{

#include <stdio.h>

#include "freecraft.h"

#ifdef WITH_SOUND	// {

#ifdef USE_CCL	// {

#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "sound.h"
#include "sound_server.h"

#include <guile/gh.h>			// I use guile for a quick hack
#include <libguile.h>

/**
 ** C representation for the guile sound type
 ** ALPHA VERSION!!!!!!!!!
 */
local long GuileSoundTag;

#define CCL_SOUNDP(x) ( SCM_CAR(x)==GuileSoundTag )
#define CCL_SOUND_ID(x) ( (SoundId)SCM_CDR(x) )

/** Test whether a scheme object is a clone sound id
    @param sound the scheme object
    @return true is sound is a clone sound id
*/
global int ccl_sound_p(SCM sound) {
    return SCM_NIMP(sound) && CCL_SOUNDP(sound);
}

/** Cast a scheme object to a clone sound id
    @param sound the scheme object
    @return the clone sound id
*/
global SoundId ccl_sound_id(SCM sound) {
    return CCL_SOUND_ID(sound);
}

/** Cast a clone sound id to its scheme version
    @param id the sound id
    @return its guile version
*/
local SCM sound_id_ccl(SoundId id) {
     SCM sound_smob;

     SCM_NEWCELL(sound_smob);
     SCM_SETCDR(sound_smob,id); 
     SCM_SETCAR(sound_smob,GuileSoundTag);
     return sound_smob;
}

/** Glue between c and scheme. Ask the sound system to associate a sound id to
    a sound name.
*/
local SCM CclSoundForName(SCM name) {
    SoundId id;
    char* sound_name;
    SCM_ASSERT(SCM_NIMP(name) && SCM_STRINGP(name),
	       name,SCM_ARG1,__FUNCTION__);
    sound_name=gh_scm2newstr(name,NULL);
    id=SoundIdForName(sound_name);
    free(sound_name);
    return sound_id_ccl(id);
}


/** Get a Game Sound Id from either a guile sound id or a sound name
 */
local SoundId CclGetSoundId(SCM sound) {
    if (SCM_NIMP(sound) && CCL_SOUNDP(sound)) {
	// if we've got the sound id
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

    SCM_ASSERT(SCM_NIMP(name) && SCM_STRINGP(name),
	       name,SCM_ARG1,__FUNCTION__);
    if (SCM_NIMP(file)) {
	if (SCM_STRINGP(file)) {
	    // only one file
	    c_name=gh_scm2newstr(name,NULL);
	    c_file=gh_scm2newstr(file,NULL);
	    id=MakeSound(c_name,&c_file,1);
	    DebugLevel3("Making sound `%s' from `%s' with id %d\n",c_name,c_file,id);
	    // the sound name (c_name) must be kept but the file name can be freed
	    free(c_file);
	} else if (gh_list_p(file)) {
	    // several files
	    c_name=gh_scm2newstr(name,NULL);
	    DebugLevel3("Making sound `%s'\n",c_name);
	    nb=gh_length(file);
	    c_files=(char **)malloc(sizeof(char*)*nb);
	    for(i=0;i<nb;i++) {
		a_file=SCM_CAR(file);
		SCM_ASSERT(SCM_NIMP(a_file) && SCM_STRINGP(a_file),
			   file,SCM_ARG2,__FUNCTION__);
		c_files[i]=gh_scm2newstr(a_file,NULL);
		DebugLevel3("\tComponent %d: `%s'\n",i,c_files[i]);
		file=SCM_CDR(file);
	    }
	    //FIXME: check size before casting
	    id=MakeSound(c_name,c_files,(unsigned char)nb);
	    for(i=0;i<nb;i++) 
		free(c_files[i]);
	    free(c_files);
	} else {
	    SCM_ASSERT(0,file,SCM_ARG2,__FUNCTION__);
	    return SCM_UNDEFINED;
	}
	return sound_id_ccl(id);
    }
    SCM_ASSERT(0,file,SCM_ARG2,__FUNCTION__);
    return SCM_UNDEFINED;
}

/** Glue between c and scheme. Ask to the sound system to remap a sound id to
    a given name.
    @param name the new name for the sound
    @param sound the sound object
    @return the sound object
*/
local SCM CclMapSound(SCM name,SCM sound) {
    char* sound_name;

    SCM_ASSERT(SCM_NIMP(name) && SCM_STRINGP(name),
	       name,SCM_ARG1,__FUNCTION__);
    SCM_ASSERT(SCM_NIMP(sound) && CCL_SOUNDP(sound),
	       sound,SCM_ARG2,__FUNCTION__);
    sound_name=gh_scm2newstr(name,NULL);
    MapSound(sound_name,CCL_SOUND_ID(sound));
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

/** Support for the guile version of a sound id: mark function for garbage
    collecting. 
    @param sound_smob the object to gc
*/
local SCM MarkGuileSound(SCM sound_smob) {
    // nothing to mark, a sound is a pair
    return SCM_BOOL_F;
}

/** Support for the guile version of a sound id: deallocator.
*/
local scm_sizet FreeGuileSound(SCM sound_smob) {
    // nothing to free
    return 0;
}

/** Support for the guile version of a sound id: printing function.
*/
local int PrintGuileSound(SCM sound_smob,SCM port,scm_print_state *pstate) {
    scm_puts("#<sound ",port);
    //FIXME: dirty trick. How to print a pointer? Maybe should we keep the
    //name of the sound somewhere...
    scm_display(gh_long2scm((long)CCL_SOUND_ID(sound_smob)),port);
    scm_puts(">",port);
    return 1;
}

/** Ask to guile to register a new scheme type: the guile version of a sound
    id
*/
local void DefineGuileSound(void) {
    scm_smobfuns guile_sound_funs = {
	MarkGuileSound,FreeGuileSound,PrintGuileSound,0
    };
    GuileSoundTag=scm_newsmob(&guile_sound_funs);
    scm_make_gsubr ("make-sound", 2, 0, 0, CclMakeSound);
    scm_make_gsubr ("play-sound", 1, 0, 0, CclPlaySound);
}

/** Glue between c and scheme. Ask the sound system to dump on the standard
    output the mapping between sound names and sound id.
*/
local SCM CclDisplaySounds(void) {
    DisplaySoundHashTable();
    return SCM_UNSPECIFIED;
}

/** Glue between c and scheme. Allows to specify some global game sounds in a
    ccl file.
*/
local SCM CclDefineGameSounds(SCM list) {
    //based on Johns CclGameMap function
    //FIXME: need much more error checking and handling
    //FIXME: should allow to define ALL the game sounds
    SCM name;
    SCM data;

    while( !gh_null_p(list) ) {

	name=gh_car(list);
	if( !gh_symbol_p(name) ) {
	    fprintf(stderr,"symbol expected\n");
	    return list;
	}
	list=gh_cdr(list);
	data=gh_car(list);
	if (! CCL_SOUNDP(data) ) {
	    fprintf(stderr,"Sound id expected\n");
	    return list;
	}
	// prepare for next iteration
	list=gh_cdr(list);
	// let's handle now the different cases
	if( gh_eq_p(name,gh_symbol2scm("click")) ) {
	    GameSounds.Click.Sound=CCL_SOUND_ID(data);
	    DebugLevel3("SoundClick %d\n",SoundClick);
	} else if ( gh_eq_p(name,gh_symbol2scm("placement-error")) ) {
	    GameSounds.PlacementError.Sound=CCL_SOUND_ID(data);
	    DebugLevel3("SoundPlacementError %d\n",SoundPlacementError);
	} else if ( gh_eq_p(name,gh_symbol2scm("placement-success")) ) {
	    GameSounds.PlacementSuccess.Sound=CCL_SOUND_ID(data);
	    DebugLevel3("SoundPlacementSuccess %d\n",SoundPlacementSuccess);
	} else if ( gh_eq_p(name,gh_symbol2scm("human-rescue")) ) {
	    GameSounds.HumanRescue.Sound=CCL_SOUND_ID(data);
	} else if ( gh_eq_p(name,gh_symbol2scm("orc-rescue")) ) {
	    GameSounds.OrcRescue.Sound=CCL_SOUND_ID(data);
	} else {
   	    fprintf(stderr,"Incorrect symbol\n");
	    return list;
	}
    }
    return SCM_UNSPECIFIED;
}

/**
**	Global volume support
**
**	@param volume new global sound volume
*/
local SCM CclSetSoundVolume(SCM volume) {
    SetGlobalVolume(gh_scm2int(volume));
    return volume;
}

/**
**	Turn Off Sound (client side)
*/
local SCM CclSoundOff(void) {
    SoundOff=1;
    return SCM_UNSPECIFIED;
}

/**
**	Turn On Sound (client side)
**
**	@return true if and only if the sound is REALLY turned on
**		(uses SoundFildes)
*/
local SCM CclSoundOn(void) {
    if (SoundFildes != -1) 
	return SCM_BOOL_T;
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
**	Ask clone to use a sound thread
*/
local SCM CclSoundThread(void) 
{
#ifdef USE_THREAD
    WithSoundThread=1;			// only if compiled with
#endif
    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for sound.
*/
global void SoundCclRegister(void)
{
    gh_new_procedure1_0("set-sound-volume",CclSetSoundVolume);
    gh_new_procedure0_0("sound-off",CclSoundOff);
    gh_new_procedure0_0("sound-on",CclSoundOn);
    gh_new_procedure0_0("sound-thread",CclSoundThread);
    gh_new_procedure1_0("set-global-sound-range",CclSetGlobalSoundRange);
    gh_new_procedure("define-game-sounds",CclDefineGameSounds,0,0,1);
    DefineGuileSound();
    gh_new_procedure0_0("display-sounds",CclDisplaySounds);
    gh_new_procedure2_0("map-sound",CclMapSound);
    gh_new_procedure1_0("sound-for-name",CclSoundForName);
    gh_new_procedure2_0("set-sound-range",CclSetSoundRange);
}

#endif	// } USE_CCL

#ifdef USE_CCL2	// {

#include <stdlib.h>

#include "ccl.h"
#include "sound_id.h"
#include "sound.h"
#include "sound_server.h"

/**
** C representation for the siod sound type
** ALPHA VERSION!!!!!!!!!
*/
local long SiodSoundTag;

#define CCL_SOUNDP(x)	TYPEP(x,SiodSoundTag)
#define CCL_SOUND_ID(x) ( (SoundId)CDR(x) )

/** Cast a clone sound id to its scheme version

    @param id the sound id

    @return its siod version
*/
local LISP sound_id_ccl(SoundId id) {
     LISP sound_id;

     sound_id=cons(NIL,NIL);
     sound_id->type=SiodSoundTag;
     sound_id->storage_as.cons.cdr=(LISP)id;
     return sound_id;
}

/** Glue between c and scheme. Ask the sound system to associate a sound id to
    a sound name.
*/
local SCM CclSoundForName(SCM name) {
    SoundId id;
    char* sound_name;

    sound_name=gh_scm2newstr(name,NULL);
    id=SoundIdForName(sound_name);
    free(sound_name);
    return sound_id_ccl(id);
}


/** Get a Game Sound Id from either a guile sound id or a sound name
 */
local SoundId CclGetSoundId(SCM sound) {
    if (CCL_SOUNDP(sound)) {
	// if we've got the sound id
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

/** Glue between c and scheme. Ask to the sound system to remap a sound id to
    a given name.
    @param name the new name for the sound
    @param sound the sound object
    @return the sound object
*/
local SCM CclMapSound(SCM name,SCM sound) {
    char* sound_name;

    sound_name=gh_scm2newstr(name,NULL);
    MapSound(sound_name,CCL_SOUND_ID(sound));
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

/** Glue between c and scheme. Allows to specify some global game sounds in a
    ccl file.
*/
local SCM CclDefineGameSounds(SCM list) {
    //based on Johns CclGameMap function
    //FIXME: need much more error checking and handling
    //FIXME: should allow to define ALL the game sounds
    SCM name;
    SCM data;

    while( !gh_null_p(list) ) {

	name=gh_car(list);
	if( !gh_symbol_p(name) ) {
	    fprintf(stderr,"symbol expected\n");
	    return list;
	}
	list=gh_cdr(list);
	data=gh_car(list);
	if (! CCL_SOUNDP(data) ) {
	    fprintf(stderr,"Sound id expected\n");
	    return list;
	}
	// prepare for next iteration
	list=gh_cdr(list);
	// let's handle now the different cases
	if( gh_eq_p(name,gh_symbol2scm("click")) ) {
	    GameSounds.Click.Sound=CCL_SOUND_ID(data);
	    DebugLevel3("SoundClick %p\n",GameSounds.Click.Sound);
	} else if ( gh_eq_p(name,gh_symbol2scm("placement-error")) ) {
	    GameSounds.PlacementError.Sound=CCL_SOUND_ID(data);
	    DebugLevel3("SoundPlacementError %p\n",
		    GameSounds.PlacementError.Sound);
	} else if ( gh_eq_p(name,gh_symbol2scm("placement-success")) ) {
	    GameSounds.PlacementSuccess.Sound=CCL_SOUND_ID(data);
	    DebugLevel3("SoundPlacementSuccess %p\n",
		    GameSounds.PlacementSuccess.Sound);
	} else {
   	    fprintf(stderr,"Incorrect symbol\n");
	    return list;
	}
    }
    return SCM_UNSPECIFIED;
}

/**
**	Global volume support
**
**	@param volume new global sound volume
*/
local SCM CclSetSoundVolume(SCM volume) {
    SetGlobalVolume(gh_scm2int(volume));
    return volume;
}

/**
**	Turn Off Sound (client side)
*/
local SCM CclSoundOff(void) {
    SoundOff=1;
    return SCM_UNSPECIFIED;
}

/**
**	Turn On Sound (client side)
**
**	@return true if and only if the sound is REALLY turned on
**		(uses SoundFildes)
*/
local SCM CclSoundOn(void) {
    if (SoundFildes != -1) 
	return SCM_BOOL_T;
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
**	Register CCL features for sound.
*/
global void SoundCclRegister(void)
{
    SiodSoundTag=allocate_user_tc();

    init_subr_1("set-sound-volume",CclSetSoundVolume);
    init_subr_0("sound-off",CclSoundOff);
    init_subr_0("sound-on",CclSoundOn);
    init_subr_0("sound-thread",CclSoundThread);
    init_subr_1("set-global-sound-range",CclSetGlobalSoundRange);
    init_lsubr("define-game-sounds",CclDefineGameSounds);
    init_subr_0("display-sounds",CclDisplaySounds);
    init_subr_2("map-sound",CclMapSound);
    init_subr_1("sound-for-name",CclSoundForName);
    init_subr_2("set-sound-range",CclSetSoundRange);
    init_subr_2("make-sound",CclMakeSound);
    init_subr_1("play-sound",CclPlaySound);

}

#endif	// } USE_CCL2

#endif	// } WITH_SOUND

//@}
