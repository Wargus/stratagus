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
/**@name ccl_sound.h	-	The Ccl sound header file. */
/*
**	(c) Copyright 1999,2000 by Lutz Sammer and Fabrice Rossi
**
**	$Id$
*/

#ifndef __CCL_SOUND_H__
#define __CCL_SOUND_H__

//@{

#if defined(USE_CCL) && defined(WITH_SOUND)	// {

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <guile/gh.h>			// for SCM

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern int ccl_sound_p(SCM sound);	/// is it a ccl sound?

extern SoundId ccl_sound_id(SCM sound);	/// scheme -> sound id

extern void SoundCclRegister(void);	/// register ccl features

#endif 	// } !defined(USE_CCL) && defined(WITH_SOUND)

#if defined(USE_CCL2) && defined(WITH_SOUND)	// {

#include "siod.h"			// for SCM
#define SCM	LISP

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern int ccl_sound_p(SCM sound);	/// is it a ccl sound?

extern SoundId ccl_sound_id(SCM sound);	/// scheme -> sound id

extern void SoundCclRegister(void);	/// register ccl features

#endif 	// { defined(USE_CCL2) && defined(WITH_SOUND)

//-----------------------------------------------------------------------------

#if !defined(USE_CCL) && !defined(USE_CCL2) || !defined(WITH_SOUND) // {

extern void SoundCclRegister(void);	/// register ccl features

#endif	// } !defined(USE_CCL) && !defined(USE_CCL2) || !defined(WITH_SOUND)
//@}

#endif	// !__CCL_SOUND_H__
