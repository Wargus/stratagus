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
/**@name sound_server.h	-	The sound server header file. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer and Fabrice Rossi
**
**	$Id$
*/

#ifndef __SOUND_SERVER_H__
#define __SOUND_SERVER_H__

//@{

#ifdef WITH_SOUND	// {

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#ifdef USE_THREAD
#include <pthread.h>
#include <semaphore.h>
extern sem_t SoundThreadChannelSemaphore;
#endif

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

#define MaxVolume 255

/**
**	RAW samples.
*/
typedef struct _sample_ {
    unsigned char	Channels;	/// mono or stereo
    unsigned char	SampleSize;	/// sample size in bits
    unsigned int	Frequency;	/// frequency in hz
    unsigned int	Length;		/// sample length
    char		Data[0];	/// sample bytes
} Sample;

/**
** Sound double group: a sound that groups two sounds, used to implement
** the annoyed/selected sound system of WC
*/
typedef struct _two_groups_ {
    struct _sound_ *First;		/// first group: selected sound
    struct _sound_ *Second;		/// second group: annoyed sound
} TwoGroups;

/**
 ** A possible value for Number in the Sound struct: means a simple sound
 */
#define ONE_SOUND	0
/**
 ** A possible value for Number in the Sound struct: means a double group (for
 ** selection/annoyed sounds)
 */
#define TWO_GROUPS	1

/**
 ** the range value that makes a sound volume distance independent
 */
#define INFINITE_SOUND_RANGE 255
/**
 ** the maximum range value 
 */
#define MAX_SOUND_RANGE 254
/**
**	Sound definition.
*/
typedef struct _sound_ {
    unsigned char Range; /// Range is a multiplier for DistanceSilent
    /// 255 means infinite range
    unsigned Number ; /// 0 means 1, 1 means two groups and > 1 is a number
    union {
	Sample*     OneSound; /// if it's only a simple sound
	Sample**    OneGroup; /// when it's a simple group
	TwoGroups*  TwoGroups; /// when it's a double group
    } Sound;
} Sound;

/**
**	Sound unique identifier
*/
typedef Sound* ServerSoundId;

/**
**	Origin of a sound
*/
typedef struct _origin_ {
    void* Base;		/// pointer on a Unit
    unsigned Id;	/// unique identifier (if the pointer has been shared)
} Origin;

/**
**	sound request FIFO
*/
typedef struct _sound_request {
    Origin Source;			/// origin of sound
    unsigned short Power;		/// Volume or Distance
    SoundId Sound;			/// which sound
    unsigned Used : 1;			/// flag for used/unused
    unsigned Fight : 1;			/// is it a fight sound?
    unsigned Selection : 1;		/// is it a selection sound?
    unsigned IsVolume : 1;		/// how to interpret power (as a
                                        ///volume or as a distance?) 
} SoundRequest;

#define MAX_SOUND_REQUESTS 32		/// maximal number of sound requests

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/// sound file descriptor, if -1 no sound available
extern int SoundFildes;

/// sound volume (from 0 to MaxVolume, acts as a multiplier)
extern int GlobalVolume;

/// global range control (max cut off distance for sound)
extern int DistanceSilent;

/// FIFO for sound requests
extern SoundRequest SoundRequests[MAX_SOUND_REQUESTS];
/// FIFO index in
extern int NextSoundRequestIn;
/// FIFO index out
extern int NextSoundRequestOut;

/// are we using a sound thread? (default is zero -> no thread)
#ifdef USE_THREAD
extern int WithSoundThread;
#endif

extern int SoundThreadRunning;

#ifdef DEBUG
/// allocated memory for sound samples
extern unsigned AllocatedSoundMemory;
/// allocated memory for compressed sound samples
extern unsigned CompressedSoundMemory;
#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/** Ask the sound server to register a sound (and currently to load it) and to
    return an unique identifier for it.
    @param file the wav file.
    @return the sound unique identifier
*/
extern SoundId RegisterSound(char* file[],unsigned char number);

/** Ask the sound server to put together to sounds to form a special sound.
    @param first first part of the group
    @param second second part of the group
    @return the special sound unique identifier
*/
extern SoundId RegisterTwoGroups(SoundId first,SoundId second);

/** Ask the sound server to change the range of a sound.
    @param sound the id of the sound to modify.
    @param range the new range for this sound.
*/
extern void SetSoundRange(SoundId sound,unsigned char range);


/** Initialize the sound card.
 */
extern int InitSound(void);

/** Initialize the sound server.
 */
extern int InitSoundServer(void);

/** Ask the sound layer to write the content of its buffer to the sound
    device. To be used only in the unthreaded version.
 */
extern void WriteSound(void);

#else	// }{ WITH_SOUND

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

#define SoundFildes	-1		/// Dummy macro for without sound
#define SoundThreadRunning	0	/// Dummy macro for without sound

#define InitSound()	0		/// Dummy macro for without sound
#define WriteSound()			/// Dummy macro for without sound

#endif	// { WITH_SOUND

//@}

#endif	// !__SOUND_SERVER_H__
