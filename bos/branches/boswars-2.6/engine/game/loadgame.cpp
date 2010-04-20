//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name loadgame.cpp - Load game. */
//
//      (c) Copyright 2001-2008 by Lutz Sammer, Andreas Arens
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "icons.h"
#include "cursor.h"
#include "construct.h"
#include "unittype.h"
#include "upgrade.h"
#include "interface.h"
#include "missile.h"
#include "map.h"
#include "script.h"
#include "ui.h"
#include "ai.h"
#include "results.h"
#include "trigger.h"
#include "actions.h"
#include "minimap.h"
#include "replay.h"
#include "sound.h"
#include "sound_server.h"
#include "font.h"
#include "pathfinder.h"
#include "spells.h"
#include "particle.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

void InitDefinedVariables();

/**
**  Cleanup modules.
**
**  Call each module to clean up.
*/
void CleanModules(void)
{
	EndReplayLog();
	CleanMessages();

	CleanIcons();
	CleanCursors();
	CleanUserInterface();
	CleanFonts();
	CleanTriggers();
	FreeAi();
	CleanPlayers();
	CleanConstructions();
	CleanDecorations();
	CleanUnitTypes();
	CleanUnits();
	CleanSelections();
	CleanGroups();
	CleanButtons();
	CleanMissileTypes();
	CleanMissiles();
	Map.Clean();
	Map.CleanFogOfWar();
	CParticleManager::exit();
	CleanReplayLog();
	CleanSpells();
	FreeVisionTable();
	FreePathfinder();
}

/**
**  Initialize all modules.
**
**  Call each module to initialize.
*/
void InitModules(void)
{
	GameCycle = 0;
	FastForwardCycle = 0;
	SyncHash = 0;

	CallbackMusicOn();
	InitSyncRand();
	InitIcons();
	InitVideoCursors();
	InitUserInterface();
	InitPlayers();
	InitMissileTypes();
	InitMissiles();
	InitConstructions();

	// LUDO : 0 = don't reset player stats ( units level , upgrades, ... ) !
	InitUnitTypes(0);

	InitUnits();
	InitSelections();
	InitGroups();
	InitSpells();

	InitButtons();
	InitTriggers();

	InitAiModule();

	Map.Init();
}

/**
**  Load all.
**
**  Call each module to load additional files (graphics,sounds).
*/
void LoadModules(void)
{
	LoadFonts();
	LoadIcons();
	LoadCursors();
	UI.Load();
	LoadMissileSprites();
	LoadConstructions();
	LoadDecorations();
	LoadUnitTypes();

	InitPathfinder();

	LoadUnitSounds();
	MapUnitSounds();
	if (SoundEnabled()) {
		InitSoundClient();
	}

	SetPlayersPalette();
	UI.Minimap.Create();

	SetDefaultTextColors(UI.NormalFontColor, UI.ReverseFontColor);
}

/**
**  Load a game to file.
**
**  @param filename  File name to be loaded.
*/
void LoadGame(const std::string &filename)
{
	unsigned long game_cycle;
	unsigned syncrand;
	unsigned synchash;
	int i;

	SaveGameLoading = true;

	SetDefaultTextColors(FontYellow, FontWhite);
	LoadFonts();

	CclGarbageCollect(0);
	InitVisionTable();
	InitUnitTypes(1);
	LuaLoadFile(filename);
	CclGarbageCollect(0);

	for (i = 0; i < NumPlayers; ++i) {
		Players[i].RebuildUnitsConsumingResourcesList();
	}

	// Place units
	for (i = 0; i < NumUnits; ++i) {
		CUnit *unit = Units[i];
		if (!unit->Removed) {
			unit->Removed = 1;
			unit->Place(unit->X, unit->Y);
		}
	}

	game_cycle = GameCycle;
	syncrand = SyncRandSeed;
	synchash = SyncHash;

	InitModules();
	LoadModules();

	GameCycle = game_cycle;
	SyncRandSeed = syncrand;
	SyncHash = synchash;
	SelectionChanged();
}

//@}
