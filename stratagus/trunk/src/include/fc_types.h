//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name fc_types.h	-	Stratagus Types. */
//
//	(c) Copyright 2002 by Lutz Sammer
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
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
