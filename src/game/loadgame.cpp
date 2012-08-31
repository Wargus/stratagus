//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name loadgame.cpp - Load game. */
//
//      (c) Copyright 2001-2006 by Lutz Sammer, Andreas Arens
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

#include "stratagus.h"

#include "actions.h"
#include "ai.h"
#include "commands.h"
#include "construct.h"
#include "depend.h"
#include "font.h"
#include "map.h"
#include "minimap.h"
#include "missile.h"
#include "particle.h"
#include "pathfinder.h"
#include "replay.h"
#include "script.h"
#include "sound.h"
#include "sound_server.h"
#include "spells.h"
#include "trigger.h"
#include "ui.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"
#include "upgrade.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

bool SaveGameLoading;                 /// If a Saved Game is Loading

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


/**
**  Cleanup modules.
**
**  Call each module to clean up.
*/
void CleanModules()
{
	EndReplayLog();
	CleanMessages();

	CleanIcons();
	CleanCursors();
	CleanUserInterface();
	CleanFonts();
	CleanTriggers();
	FreeAi();
	PlayerRaces.Clean();
	CleanConstructions();
	CleanDecorations();
	CleanMissiles();
	CleanUnits();
	CleanUnitTypes();
	CleanPlayers();
	CleanSelections();
	CleanGroups();
	CleanUpgrades();
	CleanDependencies();
	CleanButtons();
	CleanMissileTypes();
	CleanTilesets();
	Map.Clean();
	Map.CleanFogOfWar();
	CParticleManager::exit();
	CleanReplayLog();
	CleanSpells();
	FreePathfinder();

	UnitTypeVar.Init(); // internal script. should be to a better place, don't find for restart.
}

/**
**  Initialize all modules.
**
**  Call each module to initialize.
*/
void InitModules()
{
	GameCycle = 0;
	FastForwardCycle = 0;
	SyncHash = 0;

	CallbackMusicOn();
	InitSyncRand();
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
	InitUpgrades();
	InitDependencies();

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
void LoadModules()
{
	LoadFonts();
	LoadIcons();
	LoadCursors(PlayerRaces.Name[ThisPlayer->Race]);
	UI.Load();
#ifndef DYNAMIC_LOAD
	LoadMissileSprites();
#endif
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

static void PlaceUnits()
{
	for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
		CUnit &unit = **it;
		if (!unit.Removed) {
			unit.Removed = 1;
			unit.Place(unit.tilePos);
		}
	}
}

/**
**  Load a game to file.
**
**  @param filename  File name to be loaded.
*/
void LoadGame(const std::string &filename)
{
	// log will be enabled if found in the save game
	CommandLogDisabled = true;
	SaveGameLoading = true;

	SetDefaultTextColors(FontYellow, FontWhite);
	LoadFonts();

	CclGarbageCollect(0);
	InitUnitTypes(1);
	LuaLoadFile(filename);
	CclGarbageCollect(0);

	PlaceUnits();

	const unsigned long game_cycle = GameCycle;
	const unsigned syncrand = SyncRandSeed;
	const unsigned synchash = SyncHash;

	InitModules();
	LoadModules();

	GameCycle = game_cycle;
	SyncRandSeed = syncrand;
	SyncHash = synchash;
	SelectionChanged();
}

//@}
