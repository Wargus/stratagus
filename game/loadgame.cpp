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
/**@name savegame.c	-	Save game. */
//
//	(c) Copyright 2001 by Lutz Sammer, Andreas Arens
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"
#include "icons.h"
#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Cleanup all.
**
**	Call each module to clean up.
*/
global void CleanupAll(void)
{
    CleanIcons();
#if 0
    // SaveUI();
    CleanUnitTypes();
    CleanUnits();
    CleanUpgrades();
    CleanDependencies();
    CleanButtons();
    CleanMissileTypes();
    CleanMissiles();
    CleanTileset();
    CleanMap();
#endif
}

/**
**	Load all game data.
*/
global void LoadAll(void)
{
    CleanupAll();

    gh_eval_file("save_file_of_freecraft.ccl");

    MustRedraw=RedrawEverything;	// redraw everything
}

//@}
