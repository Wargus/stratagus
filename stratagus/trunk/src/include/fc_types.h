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
/**@name fc_types.h	-	FreeCraft Types. */
//
//	(c) Copyright 2002 by Lutz Sammer
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

#ifndef __FC_TYPES_H__
#define __FC_TYPES_H__

//@{

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

//	This is needed to have recursive forward references

#if !defined(__STRUCT_PLAYER__) && !defined(DOXYGEN)
#define __STRUCT_PLAYER__
typedef struct _player_ Player;
#endif

#if !defined(__STRUCT_VIEWPORT__) && !defined(DOXYGEN)
#define __STRUCT_VIEWPORT__
typedef struct _viewport_ Viewport;
#endif

#if !defined(__STRUCT_MISSILETYPE__ ) && !defined(DOXYGEN)
#define __STRUCT_MISSILETYPE__
typedef struct _missile_type_ MissileType;
#endif

//@}

#endif	// !__FC_TYPES_H__
