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
//	(c) Copyright 2001,2002 by Lutz Sammer, Andreas Arens
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

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"
#include "icons.h"
#include "cursor.h"
#include "construct.h"
#include "unittype.h"
#include "upgrade.h"
#include "depend.h"
#include "interface.h"
#include "missile.h"
#include "tileset.h"
#include "map.h"
#include "ccl.h"
#include "ui.h"
#include "ai.h"
#include "campaign.h"
#include "trigger.h"
#include "actions.h"
#include "minimap.h"
#include "commands.h"
#ifdef HIERARCHIC_PATHFINDER
#include "pathfinder.h"
#endif

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
    SCM var;

    CleanMessages();

    CleanIcons();
    CleanCursors();
    // CleanMenus();
    CleanUserInterface();
    CleanCampaign();
    CleanTriggers();
    CleanAi();
    CleanPlayers();
    CleanConstructions();
    CleanDecorations();
    CleanUnitTypes();
    CleanUnits();
    CleanSelections();
    CleanGroups();
    CleanUpgrades();
    CleanDependencies();
    CleanButtons();
    CleanMissileTypes();
    CleanMissiles();
    CleanTilesets();
    CleanMap();
#ifdef HIERARCHIC_PATHFINDER
    PfHierClean ();
#endif

    //
    //	Free our protected objects, AI scripts, unit-type properties.
    //
    var=gh_symbol2scm("*ccl-protect*");
    setvar(var,NIL,NIL);
}

/**
**	Initialize all modules.
**
**	Call each module to initialize.
*/
global void InitModules(void)
{
    GameCycle = 0;
    SyncHash = 0;

    InitSyncRand();
    InitIcons();
    InitVideoCursors();
    InitUserInterface(RaceWcNames ? RaceWcNames[1] : "oops");
    // InitMenus();
    InitPlayers();
    InitMissileTypes();
    InitMissiles();
    InitConstructions();
    // InitDecorations();
    InitUnitTypes();
    InitUnits();
    InitSelections();
    InitGroups();
    InitUpgrades();
    InitDependencies();

    InitButtons();
#ifdef HIERARCHIC_PATHFINDER
    PfHierInitialize();
#endif
    InitMapFogOfWar();			// build tables for fog of war
}

/**
**	Load all.
**
**	Call each module to load additional files (graphics,sounds).
*/
global void LoadModules(void)
{
    LoadIcons();
    LoadCursors(ThisPlayer->RaceName);
    LoadUserInterface();
    // LoadPlayers();
    LoadMissileSprites();
    LoadConstructions();
    LoadDecorations();
    LoadUnitTypes();

    LoadUnitSounds();
    MapUnitSounds();

    LoadTileset();
    CreateMinimap();

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
    int old_siod_verbose_level;
    extern int siod_verbose_level;
    unsigned long game_cycle;

    CleanModules();

    old_siod_verbose_level=siod_verbose_level;
    siod_verbose_level=4;
    user_gc(SCM_BOOL_F);
    siod_verbose_level=old_siod_verbose_level;
    gh_eval_file(filename);
    user_gc(SCM_BOOL_F);

    game_cycle=GameCycle;
    // FIXME: log should be loaded from the save game
    CommandLogDisabled = 1;

    InitModules();
    LoadModules();

    GameCycle=game_cycle;
    //GameCursor=TheUI.Point.Cursor;	// FIXME: just a default.
    GameCursor=CursorTypeByIdent("cursor-point");	// TheUI not cleaned
    UpdateButtonPanel();
    MustRedraw=RedrawEverything;	// redraw everything
}

/**
**	Load all game data.
**
**	Test function for the later load/save functions.
*/
global void LoadAll(void)
{
#if 1
    SaveGame("save_file_of_freecraft0.ccl");
    LoadGame("save_file_of_freecraft0.ccl");
    SaveGame("save_file_of_freecraft1.ccl");
    LoadGame("save_file_of_freecraft1.ccl");
    SaveGame("save_file_of_freecraft2.ccl");
    LoadGame("save_file_of_freecraft2.ccl");
#endif
    //LoadGame ("save_file_of_freecraft.ccl");
}

//@}
