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
/**@name sound.h	-	The sound header file. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer and Fabrice Rossi
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

#ifndef __SOUND_H__
#define __SOUND_H__

//@{

#ifdef WITH_SOUND	// {

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "sound_id.h"
#include "missile.h"

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

#define MaxSampleVolume 255		/// Maximum sample volume
#define NO_SOUND 0			/// No valid sound ID

/**
**	Global game sounds, not associated to any unit-type
*/
typedef struct _game_sound_ {
    SoundConfig PlacementError;		/// used by ui
    SoundConfig PlacementSuccess;	/// used by ui
    SoundConfig Click;			/// used by ui

    SoundConfig TreeChopping;		/// FIXME: currently unused
    SoundConfig Docking;		/// ship reaches coast
    SoundConfig BuildingConstruction;	/// building under construction

//FIXME: (Fabrice) I don't think it's the correct place to put this
    SoundConfig HumanWorkComplete;	/// building ready
    SoundConfig PeasantWorkComplete;	/// building ready
    SoundConfig OrcWorkComplete;	/// building ready

    SoundConfig Repair;			/// repair sound

    SoundConfig HumanRescue;		/// rescue units
    SoundConfig OrcRescue;		/// rescue units
} GameSound;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /**
    **	Client side sound control. Can be used to turn on/off sound without
    **	really turning it off on the server side.
    */
extern int SoundOff;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern GameSound GameSounds;		/// Game sound configuration

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/** Ask to the sound server to play a sound attached to an unit. The
    sound server may discard the sound if needed (e.g., when the same
    unit is already speaking).
    @param unit the unit speaking
    @param unit_voice_group the sound to play
*/
extern void PlayUnitSound(const Unit* unit,UnitVoiceGroup unit_voice_group);

/** Ask to the sound server to play a sound associated to a missile.
    @param missile the missile (origin of the sound)
    @param sound the sound to play
*/
extern void PlayMissileSound(const Missile* missile,SoundId sound);

/** Ask to the sound server to play a sound: low level call.
    @param sound the sound to play.
    @param volume volume of the sound
*/
extern void PlayGameSound(SoundId sound,unsigned char volume);

    /// Set global volume
extern void SetGlobalVolume(int volume);
    /// Set music volume
extern void SetMusicVolume(int volume);

/** Initialize client side of the sound layer.
*/
extern void InitSoundClient(void);

    /// Play a music file
extern void PlayMusic(const char* name);

#else	// }{ WITH_SOUND

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

#define SoundOff	1		/// Dummy macro for without sound

#define PlayUnitSound(u,g)		/// Dummy macro for without sound
#define PlayMissileSound(s,v)		/// Dummy macro for without sound
#define PlayGameSound(s,v)		/// Dummy macro for without sound
#define SetGlobalVolume(v)		/// Dummy macro for without sound
#define SetMusicVolume(v)		/// Dummy macro for without sound
#define InitSoundClient()		/// Dummy macro for without sound

#define PlayMusic(v)			/// Dummy macro for without sound

#endif	// } WITH_SOUND

//@}

#endif	// !__SOUND_H__
