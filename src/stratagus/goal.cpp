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
/**@name goal.c		-	The game goal. */
//
//	(c) Copyright 1999-2001 by Lutz Sammer
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
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "goal.h"

#include "interface.h"

#include "unit.h"
#include "menus.h"
#include "network.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

// FIXME: must write more goals. Currently only last side wins supported

/**
**	Check if the goals for this game are reached.
*/
global void CheckGoals(void)
{
    int i;
    int n;

    DebugLevel3Fn("\n");

    for( i=n=0; i<NumPlayers; ++i ) {
	if( Players[i].Race!=PlayerRaceNeutral ) {
	    DebugLevel3("%d: %d %d\n",i,Players[i].NumFoodUnits
			,Players[i].NumBuildings);
	    if( Players[i].NumFoodUnits+Players[i].NumBuildings ) {
		++n;
	    }
	}
    }

    if( n==1 ) {			// only one player remains.
	NetworkQuit();
	if( ThisPlayer->NumFoodUnits+ThisPlayer->NumBuildings ) {
	    fprintf(stderr,"You have won!\n");
            // SetStatusLine("You have won!");
            GamePaused=1;
	    ProcessMenu(MENU_VICTORY, 1);
	} else {
	    fprintf(stderr,"You have lost!\n");
            // SetStatusLine("You have lost!");
            GamePaused=1;
	    ProcessMenu(MENU_LOST, 1);
	}
    }
}

//@}
