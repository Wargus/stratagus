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
/**@name sound.c	-	The sound. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer and Fabrice Rossi
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "freecraft.h"

#ifdef WITH_SOUND	// {

#ifdef USE_SDLA
#include <SDL/SDL_audio.h>
#endif

#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "sound_server.h"
#include "missile.h"

#include "sound.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int SoundOff;			/// True quiet, sound turned off

/**
**	FIXME: docu
*/
global GameSound GameSounds={
    { "placement error" },
    { "placement sucess" },
    { "click" },
    { "tree chopping"},
    { "transport docking"},
    { "building construction"},
    { "basic human voices work complete" },
    { "peasant work complete" },
    { "basic orc voices work complete" },
    { "rescue (human)" },
    { "rescue (orc)" },
};

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Helper function: insert a sound request in the server side sound FIFO.
**
**	@param unit	Pointer to unit.
**	@param id	Unit identifier, for pointer reuse detection.
**	@param power	How loud to play the sound.
**	@param sound
**	@param fight
**	@param selection
**	@param volume
*/
local void InsertSoundRequest(Unit* unit,unsigned id,unsigned char power,
			      SoundId sound,unsigned char fight,
			      unsigned char selection,unsigned char volume) 
{
#ifdef USE_SDLA
// FIXME: Don't know if this is the right position
    SDL_LockAudio();
#endif
    //FIXME: valid only in a shared memory context...
    if( !SoundOff && sound != NO_SOUND ) {
	if (SoundRequests[NextSoundRequestIn].Used) {
	    DebugLevel0("***** NO FREE SLOT IN SOUND FIFO *****\n");
	} else {
	    SoundRequests[NextSoundRequestIn].Used=1;
	    SoundRequests[NextSoundRequestIn].Source.Base=unit;
	    SoundRequests[NextSoundRequestIn].Source.Id=id;
	    SoundRequests[NextSoundRequestIn].Sound=sound;
	    SoundRequests[NextSoundRequestIn].Power=power;
	    SoundRequests[NextSoundRequestIn].Fight=(fight)?1:0;
	    SoundRequests[NextSoundRequestIn].Selection=(selection)?1:0;
	    SoundRequests[NextSoundRequestIn].IsVolume=(volume)?1:0;
	    DebugLevel3("Source[%p,%s]: registering request %p at slot %d=%d\n",
			unit,unit ? unit->Type->Ident : ""
			,sound,NextSoundRequestIn,power);
#ifdef USE_THREAD
	    // increment semaphore
	    if (SoundThreadRunning) {
		if(sem_post(&SoundThreadChannelSemaphore)) {
		    DebugLevel0("Cannot increment semaphore!\n");
		    //FIXME: need to quit?
		}
	    }
#endif
	    NextSoundRequestIn++;
	    if (NextSoundRequestIn>=MAX_SOUND_REQUESTS)
		NextSoundRequestIn=0;
	}
    }
#ifdef USE_SDLA
// FIXME: Don't know if this is the right position
    SDL_UnlockAudio();
#endif
}

/**
**	Maps a UnitVoiceGroup to a SoundId. 
*/
local SoundId ChooseUnitVoiceSoundId(Unit *unit,UnitVoiceGroup voice) {
    switch (voice) {
    case VoiceAcknowledging:
	return unit->Type->Sound.Acknowledgement.Sound;
    case VoiceReady:
	return unit->Type->Sound.Ready.Sound;
    case VoiceSelected:
	return unit->Type->Sound.Selected.Sound;
    case VoiceAttacking:
	return unit->Type->Weapon.Attack.Sound;
    case VoiceHelpMe:
	return unit->Type->Sound.Help.Sound;
    case VoiceDying:
	return unit->Type->Sound.Dead.Sound;
    case VoiceTreeChopping:
	return GameSounds.TreeChopping.Sound;
    case VoiceWorkCompleted:
	// FIXME: make this more configurable
	if (unit->Type==UnitTypeHumanWorker) 
	    return GameSounds.PeasantWorkComplete.Sound;
	else if( ThisPlayer->Race==PlayerRaceHuman ) {
	    return GameSounds.HumanWorkComplete.Sound;
	} else {
	    return GameSounds.OrcWorkComplete.Sound;
	}
    case VoiceBuilding:
	return GameSounds.BuildingConstruction.Sound;
    case VoiceDocking:
	return GameSounds.Docking.Sound;
    }
    return NULL;
}


/**
**	FIXME: docu
*/
global void PlayUnitSound(Unit* unit,UnitVoiceGroup unit_voice_group)
{
    DebugLevel3("FIXME: fabrice please look\n");
    InsertSoundRequest(unit,
		       unit->Slot,
		       ViewPointDistanceToUnit(unit),
		       ChooseUnitVoiceSoundId(unit,unit_voice_group),
		       unit_voice_group==VoiceAttacking,
		       (unit_voice_group==VoiceSelected
			||unit_voice_group==VoiceBuilding),
		       0);
}

/**
**	FIXME: docu
*/
global void PlayMissileSound(const Missile* missile,SoundId sound) {
    DebugLevel3("Playing %p at volume %u\n",sound,volume);
    InsertSoundRequest(NULL,
		       0,
		       ViewPointDistanceToMissile(missile),
		       sound,
		       1,
		       0,
		       0);
}

/**
**	FIXME: docu
*/
global void PlayGameSound(SoundId sound,unsigned char volume) {
    DebugLevel3("Playing %p at volume %u\n",sound,volume);
    InsertSoundRequest(NULL,
		       0,
		       volume,
		       sound,
		       0,
		       0,
		       1);
}

/**
**	FIXME: docu
*/
global void SetGlobalVolume(int volume) {
    //FIXME: we use here the fact that we are in a shared memory context. This
    // should send a message to the sound server
    // silently discard out of range values
    if ( volume<0 ) {
	GlobalVolume=0;
    } else if ( volume>MaxVolume ) {
	GlobalVolume=MaxVolume;
    } else {
	GlobalVolume=volume;
    }
    DebugLevel3("Sound Volume: %d\n",GlobalVolume);
}

/**
**	Lookup the sound id's for the game sounds.
*/
global void InitSoundClient(void)
{
    // let's map game sounds, look if already setup in ccl.

    if( !GameSounds.PlacementError.Sound ) {
	GameSounds.PlacementError.Sound=
		SoundIdForName(GameSounds.PlacementError.Name);
    }
    if( !GameSounds.PlacementSuccess.Sound ) {
	GameSounds.PlacementSuccess.Sound=
		SoundIdForName(GameSounds.PlacementSuccess.Name);
    }
    if( !GameSounds.Click.Sound ) {
	GameSounds.Click.Sound=SoundIdForName(GameSounds.Click.Name);
    }
    if( !GameSounds.TreeChopping.Sound ) {
	GameSounds.TreeChopping.Sound=
		SoundIdForName(GameSounds.TreeChopping.Name);
    }
    if( !GameSounds.Docking.Sound ) {
	GameSounds.Docking.Sound=
		SoundIdForName(GameSounds.Docking.Name);
    }
    if( !GameSounds.BuildingConstruction.Sound ) {
	GameSounds.BuildingConstruction.Sound=
		SoundIdForName(GameSounds.BuildingConstruction.Name);
    }
    if( !GameSounds.HumanWorkComplete.Sound ) {
	GameSounds.HumanWorkComplete.Sound=
		SoundIdForName(GameSounds.HumanWorkComplete.Name);
    }
    if( !GameSounds.PeasantWorkComplete.Sound ) {
	GameSounds.PeasantWorkComplete.Sound=
		SoundIdForName(GameSounds.PeasantWorkComplete.Name);
    }
    if( !GameSounds.OrcWorkComplete.Sound ) {
	GameSounds.OrcWorkComplete.Sound=
		SoundIdForName(GameSounds.OrcWorkComplete.Name);
    }
    if( !GameSounds.HumanRescue.Sound ) {
	GameSounds.HumanRescue.Sound=
		SoundIdForName(GameSounds.HumanRescue.Name);
    }
    if( !GameSounds.OrcRescue.Sound ) {
	GameSounds.OrcRescue.Sound=
		SoundIdForName(GameSounds.OrcRescue.Name);
    }
}

#endif	// } WITH_SOUND

//@}
