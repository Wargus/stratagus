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
/**@name loadgame.c	-	Load game. */
//
//	(c) Copyright 2001 by Lutz Sammer, Andreas Arens
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
#include "icons.h"
#include "unittype.h"
#include "upgrade.h"
#include "depend.h"
#include "interface.h"
#include "missile.h"
#include "tileset.h"
#include "map.h"
#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Cleanup modules.
**
**	Call each module to clean up.
*/
global void CleanModules(void)
{
    CleanIcons();
    // CleanUI();
    CleanPlayers();
    CleanUnitTypes();
    CleanUnits();
    CleanUpgrades();
    CleanDependencies();
    CleanButtons();
    CleanMissileTypes();
    CleanMissiles();
    CleanTileset();
    CleanMap();
}

/**
**	Initialize all modules.
**
**	Call each module to initialize.
*/
global void InitModules(void)
{
    InitIcons();
    // InitUI();
    InitPlayers();
    InitUnitTypes();
    InitUnits();
    InitUpgrades();
    InitDependencies();

    InitButtons();
}

/**
**	Load all.
**
**	Call each module to load additional files (graphics,sounds).
*/
global void LoadModules(void)
{
    LoadIcons();
    // LoadUI();
    // LoadPlayers();
    LoadUnitTypes();

    LoadTileset();

    // LoadButtons();
}

/**
**	Load a game to file.
**
**	@param filename	File name to be loaded.
**
**	@note	Later we want to store in a more compact binary format.
*/
global void LoadGame(char* filename)
{
    CleanModules();

    gh_eval_file(filename);

    InitModules();
    LoadModules();

    MustRedraw=RedrawEverything;	// redraw everything
}

/**
**	Load all game data.
**
**	Test function for the later load/save functions.
*/
global void LoadAll(void)
{
    SaveGame("save_file_of_freecraft0.ccl");
    LoadGame("save_file_of_freecraft0.ccl");
    SaveGame("save_file_of_freecraft1.ccl");
}

//@}
