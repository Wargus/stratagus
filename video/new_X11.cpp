//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name new_X11.c		-	XWindows support. */
//
//	(c) Copyright 1998-2000 by Lutz Sammer and Valery Shchedrin
//
//	Stratagus is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	Stratagus is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#ifdef USE_X11	// {

// FIXME: move the cleanup X11 source to here!

#include "video.h"


/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Initialze the video part for X11.
*/
global void InitVideoX11(void)
{
    extern void GameInitDisplay(void);

    GameInitDisplay();			// Fall back to old function
}

#endif // } USE_X11

//@}
