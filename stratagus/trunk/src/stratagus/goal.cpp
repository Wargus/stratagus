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
/*
**	(c) Copyright 1999,2000 by Lutz Sammer
**
**	$Id$
*/

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
#include "network.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Check if the goals for this game are reached.
*/
global void CheckGoals(void)
{
    int i;
    int n;

    DebugLevel3(__FUNCTION__"\n");

    for( i=n=0; i<NumPlayers; ++i ) {
	if( Players[i].Race!=PlayerRaceNeutral ) {
	    DebugLevel3("%d: %d %d\n",i,Players[i].NumUnits
			,Players[i].NumBuildings);
	    if( Players[i].NumUnits+Players[i].NumBuildings ) {
		++n;
	    }
	}
    }

    if( n==1 ) {			// only one player remains.
	NetworkQuit();
	if( ThisPlayer->NumUnits+ThisPlayer->NumBuildings ) {
	    fprintf(stderr,"You have won!\n");
            SetStatusLine("You have won!");
            GamePaused=1;
	} else {
	    fprintf(stderr,"You have lost!\n");
            SetStatusLine("You have lost!");
            GamePaused=1;
	}
    }
}

//@}
