//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name game.cpp - The game set-up and creation. */
//
//      (c) Copyright 1998-2010 by Lutz Sammer, Andreas Arens, and
//                                 Jimmy Salmon
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
#include <string.h>

#include "stratagus.h"
#include "map.h"
#include "minimap.h"
#include "player.h"
#include "unit.h"
#include "unittype.h"
#include "upgrade.h"
#include "pathfinder.h"
#include "ui.h"
#include "font.h"
#include "sound.h"
#include "sound_server.h"
#include "interface.h"
#include "cursor.h"
#include "spells.h"
#include "construct.h"
#include "actions.h"
#include "network.h"
#include "netconnect.h"
#include "missile.h"
#include "settings.h"
#include "results.h"
#include "trigger.h"
#include "replay.h"
#include "iolib.h"
#include "iocompat.h"

#include "script.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

Settings GameSettings;  /// Game Settings
static int LcmPreventRecurse;   /// prevent recursion through LoadGameMap
GameResults GameResult;                      /// Outcome of the game

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static void WriteCreateUnits(FileWriter *f);
static void WriteCreateUnit(FileWriter *f, const CUnit *unit);

/*----------------------------------------------------------------------------
--  Map loading/saving
----------------------------------------------------------------------------*/

/**
**  Load a Stratagus map.
**
**  @param smpname  smp filename
**  @param mapname  map filename
**  @param map      map loaded
*/
static void LoadStratagusMap(const std::string &smpname, const std::string &mapname, CMap *map)
{
	char mapfull[PATH_MAX];
	CFile file;

	// Try the same directory as the smp file first
	strcpy_s(mapfull, sizeof(mapfull), smpname.c_str());
	char *p = strrchr(mapfull, '/');
	if (!p) {
		p = mapfull;
	} else {
		++p;
	}
	strcpy_s(p, sizeof(mapfull) - (p - mapfull), mapname.c_str());

	if (file.open(mapfull, CL_OPEN_READ) == -1) {
		// Not found, try StratagusLibPath and the smp's dir
		strcpy_s(mapfull, sizeof(mapfull), StratagusLibPath.c_str());
		strcat_s(mapfull, sizeof(mapfull), "/");
		strcat_s(mapfull, sizeof(mapfull), smpname.c_str());
		p = strrchr(mapfull, '/');
		if (!p) {
			p = mapfull;
		} else {
			++p;
		}
		strcpy_s(p, sizeof(mapfull) - (p - mapfull), mapname.c_str());
		if (file.open(mapfull, CL_OPEN_READ) == -1) {
			// Not found again, try StratagusLibPath
			strcpy_s(mapfull, sizeof(mapfull), StratagusLibPath.c_str());
			strcat_s(mapfull, sizeof(mapfull), "/");
			strcat_s(mapfull, sizeof(mapfull), mapname.c_str());
			if (file.open(mapfull, CL_OPEN_READ) == -1) {
				// Not found, try mapname by itself as a last resort
				strcpy_s(mapfull, sizeof(mapfull), mapname.c_str());
			} else {
				file.close();
			}
		} else {
			file.close();
		}
	} else {
		file.close();
	}

	if (LcmPreventRecurse) {
		fprintf(stderr, "recursive use of load map!\n");
		ExitFatal(-1);
	}
	InitPlayers();
	LcmPreventRecurse = 1;
	if (LuaLoadFile(mapfull) == -1) {
		fprintf(stderr, "Can't load lua file: %s\n", mapfull);
		ExitFatal(-1);
	}
	LcmPreventRecurse = 0;

#if 0
	// Not true if multiplayer levels!
	if (!ThisPlayer) { /// ARI: bomb if nothing was loaded!
		fprintf(stderr, "%s: invalid map\n", mapname.c_str());
		ExitFatal(-1);
	}
#endif
	if (!Map.Info.MapWidth || !Map.Info.MapHeight) {
		fprintf(stderr, "%s: invalid map\n", mapname.c_str());
		ExitFatal(-1);
	}
	Map.Info.Filename = mapname;
}


// Write the map presentation file
static int WriteMapPresentation(const std::string &mapname, CMap *map)
{
	FileWriter *f = NULL;
	int i;
	int topplayer;
	int numplayers;
	const char *type[] = {"", "", "neutral", "nobody", 
		"computer", "person", "rescue-passive", "rescue-active"};
	
	numplayers = 0;
	topplayer = PlayerMax - 2;
	
	try {
		f = CreateFileWriter(mapname);
		f->printf("-- Stratagus Map Presentation\n");
		f->printf("-- File generated by the Bos Wars builtin editor.\n");
		// MAPTODO Copyright notice in generated file
		f->printf("-- File licensed under the GNU GPL version 2 or later.\n\n");

		f->printf("DefinePlayerTypes(");
		while (topplayer > 0 && map->Info.PlayerType[topplayer] == PlayerNobody) {
			--topplayer;
		}
		for (i = 0; i <= topplayer; ++i) {
			f->printf("%s\"%s\"", (i ? ", " : ""), type[map->Info.PlayerType[i]]);
			if (map->Info.PlayerType[i] == PlayerPerson) {
				++numplayers;
			}
		}
		f->printf(")\n");
	
		f->printf("PresentMap(\"%s\", %d, %d, %d, %d)\n",
			map->Info.Description.c_str(), numplayers, map->Info.MapWidth, map->Info.MapHeight,
			map->Info.MapUID + 1);

		f->printf("DefineMapSetup(GetCurrentLuaPath()..\"/setup.sms\")\n");
	} catch (const FileException &) {
		fprintf(stderr, "ERROR: cannot write the map presentation\n");
		delete f;
		return -1;
	}
	delete f;
	
	return 1;
}


/**
**  Write the map setup file.
**
**  @param mapSetup  map filename
**  @param map       map to save
*/
int WriteMapSetup(const char *mapSetup, CMap *map)
{
	FileWriter *f = NULL;
	int i;

	try {
		f = CreateFileWriter(mapSetup);
	
		f->printf("-- Stratagus Map Setup\n");
		f->printf("-- File generated by the Bos Wars builtin editor.\n");
		// MAPTODO Copyright notice in generated file
		f->printf("-- File licensed under the GNU GPL version 2 or later.\n\n");
	
		f->printf("-- player configuration\n");
		for (i = 0; i < PlayerMax - 1; ++i) {
			f->printf("Players[%d]:SetStartView(%d, %d)\n", i, Players[i].StartX, Players[i].StartY);
			f->printf("Players[%d].EnergyStored = %d\n", i, Players[i].GetEnergyStored());
			f->printf("Players[%d].MagmaStored = %d\n", i, Players[i].GetMagmaStored());
			f->printf("Players[%d].AiName = \"%s\"\n", i, Players[i].AiName.c_str());
		}
		f->printf("\n");

		f->printf("Load(GetCurrentLuaPath() .. \"/terrain.lua\")\n");
		f->printf("\n");

		f->printf("-- place units\n");
		WriteCreateUnits(f);
		f->printf("\n\n");
	} catch (const FileException &) {
		fprintf(stderr,"Can't save map setup: `%s' \n", mapSetup);
		delete f;
		return -1;
	}
	delete f;

	return 1;
}

/**
**  Write Lua CreateUnit calls for all existing units to the map setup
**  file.  When the map is later loaded, these calls will recreate the
**  units.
**
**  @param f     file writer for the map setup file
*/
static void WriteCreateUnits(FileWriter *f)
{
	int i;

	// Hot-spot units must be created before the magma-pump units
	// that are built on top of them.  Otherwise, the map would
	// not be correctly loaded.
	//
	// When the user first adds a hot spot and a magma pump in the
	// editor, they end up in the correct order in the Units
	// array.  However, if the user then deletes some other unit,
	// CUnit::Release moves the last unit in the array to the slot
	// of the deleted unit, and this can move the magma pump in
	// front of the hot spot.  If the units are saved in that order,
	// and the map is then loaded, CclCreateUnit will refuse to
	// place the hot spot on top of the magma pump, and will instead
	// place it somewhere nearby.  This behavior of CclCreateUnit
	// is necessary because it can be called from triggers too.
	//
	// Currently, magma pumps are the only unit type with any
	// building rules, so we just check that.  This code may have
	// to be revised if building rules are added to other unit
	// types, especially if the rules are chained (X on top of Y
	// on top of Z).
	for (i = 0; i < NumUnits; ++i) {
		if (Units[i]->Type->BuildingRules.empty()) {
			WriteCreateUnit(f, Units[i]);
		}
	}
	for (i = 0; i < NumUnits; ++i) {
		if (!Units[i]->Type->BuildingRules.empty()) {
			WriteCreateUnit(f, Units[i]);
		}
	}
}

/**
**  Write a Lua CreateUnit call for the specified unit to the map
**  setup file.  When the map is later loaded, this call will recreate
**  the unit.
**
**  @param f     file writer for the map setup file
**  @param unit  unit to be created when the map is loaded
*/
static void WriteCreateUnit(FileWriter *f, const CUnit *unit)
{
	f->printf("CreateUnit(\"%s\", ", unit->Type->Ident.c_str());
	if (unit->Player->Index == PlayerMax - 1) {
		f->printf("PlayerNumNeutral");
	} else {
		f->printf("%d", unit->Player->Index);
	}
	f->printf(", {%d, %d})\n", unit->X, unit->Y);
}

/**
**  Write a terrain into the given file.
**
**  @param path  pathname of the file to write the terrain into
**  @param map   whose terrain must be saved
*/
static void WriteTerrain(std::string &path, const CMap *map)
{
	FileWriter *f = NULL;

	try
	{
		f = CreateFileWriter(path);
		f->printf("-- terrain\n");
		f->printf("%s", Map.PatchManager.savePatches(true).c_str());
		f->printf("\n");
	}
	catch (const FileException &)
	{
		fprintf(stderr,"Can't save map terrain: `%s' \n", path.c_str());
	}

	delete f;
}


/**
**  Save a Stratagus map.
**
**  @param mapName  map filename
**  @param map      map to save
*/
int SaveStratagusMap(const std::string &mapName, CMap *map)
{
	std::string mapPresentation;
	std::string mapSetup;

	if (!map->Info.MapWidth || !map->Info.MapHeight) {
		fprintf(stderr, "%s: invalid Bos Wars map\n", mapName.c_str());
		ExitFatal(-1);
	}

	if (mapName.find(".map") == std::string::npos) {
		fprintf(stderr, "%s: invalid Bos Wars map filename\n", mapName.c_str());
	}

	makedir(mapName.c_str(), 0777);
	mapPresentation = mapName + std::string("/presentation.smp");
	mapSetup = mapName + std::string("/setup.sms");
	if (WriteMapPresentation(mapPresentation, map) == -1) {
		return -1;
	}
	
	std::string terrainName = mapName + std::string("/terrain.lua");
	WriteTerrain(terrainName, map);

	return WriteMapSetup(mapSetup.c_str(), map);
}


/**
**  Load any map.
**
**  @param filename  map filename
**  @param map       map loaded
*/
static void LoadMap(const std::string &filename, CMap *map)
{
	const char *tmp;
	const char *name = filename.c_str();

	tmp = strrchr(name, '.');
	if (tmp) {
		if (!strcmp(tmp, ".gz")) {
			while (tmp - 1 > name && *--tmp != '.') {
			}
		}
		if (!strcmp(tmp, ".smp") || !strcmp(tmp, ".smp.gz")) {
			if (map->Info.Filename.empty()) {
				// The map info hasn't been loaded yet => do it now
				LoadStratagusMapInfo(filename);
			}
			Assert(!map->Info.Filename.empty());
			map->Create();
			LoadStratagusMap(filename, map->Info.Filename.c_str(), map);
			return;
		}
	}

	fprintf(stderr, "Unrecognized map format\n");
	ExitFatal(-1);
}

/**
**  Set the game paused or unpaused
**
**  @param paused  True to pause game, false to unpause.
*/
void SetGamePaused(bool paused)
{
	GamePaused = paused;
}

/**
**  Get the game paused or unpaused
**
**  @return  True if the game is paused, false otherwise
*/
bool GetGamePaused()
{
	return GamePaused;
}

/**
**  Set the game speed
**
**  @param speed  New game speed.
*/
void SetGameSpeed(int speed)
{
	if (GameCycle == 0 || FastForwardCycle < GameCycle) {
		VideoSyncSpeed = speed * 100 / CYCLES_PER_SECOND;
		SetVideoSync();
	}
}

/**
**  Get the game speed
**
**  @return  Game speed
*/
int GetGameSpeed()
{
	return CYCLES_PER_SECOND * VideoSyncSpeed / 100;
}

/*----------------------------------------------------------------------------
--  Game types
----------------------------------------------------------------------------*/

/**
**  Free for all
*/
static void GameTypeFreeForAll(void)
{
	int i;
	int j;

	for (i = 0; i < PlayerMax - 1; ++i) {
		for (j = 0; j < PlayerMax - 1; ++j) {
			if (i != j) {
				CommandDiplomacy(i, DiplomacyEnemy, j);
			}
		}
	}
}

/**
**  Top vs Bottom
*/
static void GameTypeTopVsBottom(void)
{
	int i;
	int j;
	int top;
	int middle;

	middle = Map.Info.MapHeight / 2;
	for (i = 0; i < PlayerMax - 1; ++i) {
		top = Players[i].StartY <= middle;
		for (j = 0; j < PlayerMax - 1; ++j) {
			if (i != j) {
				if ((top && Players[j].StartY <= middle) ||
						(!top && Players[j].StartY > middle)) {
					CommandDiplomacy(i, DiplomacyAllied, j);
					Players[i].SharedVision |= (1 << j);
				} else {
					CommandDiplomacy(i, DiplomacyEnemy, j);
				}
			}
		}
	}
}

/**
**  Left vs Right
*/
static void GameTypeLeftVsRight(void)
{
	int i;
	int j;
	int left;
	int middle;

	middle = Map.Info.MapWidth / 2;
	for (i = 0; i < PlayerMax - 1; ++i) {
		left = Players[i].StartX <= middle;
		for (j = 0; j < PlayerMax - 1; ++j) {
			if (i != j) {
				if ((left && Players[j].StartX <= middle) ||
						(!left && Players[j].StartX > middle)) {
					CommandDiplomacy(i, DiplomacyAllied, j);
					Players[i].SharedVision |= (1 << j);
				} else {
					CommandDiplomacy(i, DiplomacyEnemy, j);
				}
			}
		}
	}
}

/**
**  Man vs Machine
*/
static void GameTypeManVsMachine(void)
{
	int i;
	int j;

	for (i = 0; i < PlayerMax - 1; ++i) {
		if (Players[i].Type != PlayerPerson && Players[i].Type != PlayerComputer) {
			continue;
		}
		for (j = 0; j < PlayerMax - 1; ++j) {
			if (i != j) {
				if (Players[i].Type == Players[j].Type) {
					CommandDiplomacy(i, DiplomacyAllied, j);
					Players[i].SharedVision |= (1 << j);
				} else {
					CommandDiplomacy(i, DiplomacyEnemy, j);
				}
			}
		}
	}
}

/**
**  Man vs Machine whith Humans on a Team
*/
static void GameTypeManTeamVsMachine(void)
{
	int i;
	int j;

	for (i = 0; i < PlayerMax - 1; ++i) {
		if (Players[i].Type != PlayerPerson && Players[i].Type != PlayerComputer) {
			continue;
		}
		for (j = 0; j < PlayerMax - 1; ++j) {
			if (i != j) {
				if (Players[i].Type == Players[j].Type) {
					CommandDiplomacy(i, DiplomacyAllied, j);
					Players[i].SharedVision |= (1 << j);
				} else {
					CommandDiplomacy(i, DiplomacyEnemy, j);
				}
			}
		}
		if (Players[i].Type == PlayerPerson) {
			Players[i].Team = 2;
		} else {
			Players[i].Team = 1;
		}
	}
}

/*----------------------------------------------------------------------------
--  Game creation
----------------------------------------------------------------------------*/

/**
**  CreateGame.
**
**  Load map, graphics, sounds, etc
**
**  @param filename  map filename
**  @param map       map loaded
**
**  @todo FIXME: use in this function InitModules / LoadModules!!!
*/
void CreateGame(const std::string &filename, CMap *map)
{
	int i;

	if (SaveGameLoading) {
		SaveGameLoading = false;
		// Load game, already created game with Init/LoadModules
		CommandLog(NULL, NoUnitP, FlushCommands, -1, -1, NoUnitP, NULL, -1);
		return;
	}

	InitVisionTable(); // build vision table for fog of war
	InitPlayers();
	
	if (Map.Info.Filename.empty() && !filename.empty()) {
		char path[PATH_MAX];
		
		LibraryFileName(filename.c_str(), path, sizeof(path));
		if (strcasestr(filename.c_str(), ".smp")) {
			LuaLoadFile(path);
		}
	}

	for (i = 0; i < PlayerMax; ++i) {
		PlayerTypes playertype = (PlayerTypes)Map.Info.PlayerType[i];
		// Network games only:
		if (GameSettings.Presets[i].Type != SettingsPresetMapDefault) {
			playertype = GameSettings.Presets[i].Type;
		}
		CreatePlayer(playertype);
	}

	if (!filename.empty()) {
		if (CurrentMapPath != filename) {
			strcpy_s(CurrentMapPath, sizeof(CurrentMapPath), filename.c_str());
		}

		//
		// Load the map.
		//
		InitUnitTypes(1);
		LoadMap(filename, map);
	}

	GameCycle = 0;
	FastForwardCycle = 0;
	SyncHash = 0;
	InitSyncRand();

	if (IsNetworkGame()) { // Prepare network play
		DebugPrint("Client setup: Calling InitNetwork2\n");
		InitNetwork2();
	} else {
		if (!LocalPlayerName.empty() && LocalPlayerName != "Anonymous") {
			ThisPlayer->SetName(LocalPlayerName);
		}
	}

	CallbackMusicOn();

#if 0
	GamePaused = true;
#endif

	if (FlagRevealMap) {
		Map.Reveal();
	}

	//
	// Setup game types
	//
	if (GameSettings.GameType != SettingsGameTypeMapDefault) {
		switch (GameSettings.GameType) {
			case SettingsGameTypeMelee:
				break;
			case SettingsGameTypeFreeForAll:
				GameTypeFreeForAll();
				break;
			case SettingsGameTypeTopVsBottom:
				GameTypeTopVsBottom();
				break;
			case SettingsGameTypeLeftVsRight:
				GameTypeLeftVsRight();
				break;
			case SettingsGameTypeManVsMachine:
				GameTypeManVsMachine();
				break;
			case SettingsGameTypeManTeamVsMachine:
				GameTypeManTeamVsMachine();
		}
	}

	//
	// Graphic part
	//
	SetPlayersPalette();
	InitIcons();
	LoadIcons();

	LoadCursors();
	UnitUnderCursor = NoUnitP;

	InitMissileTypes();
	LoadMissileSprites();
	InitConstructions();
	LoadConstructions();
	LoadUnitTypes();
	LoadDecorations();

	InitSelections();

	InitUserInterface();
	UI.Load();

	Map.Init();
	UI.Minimap.Create();

	//
	// Sound part
	//
	LoadUnitSounds();
	MapUnitSounds();
	if (SoundEnabled()) {
		InitSoundClient();
	}

	//
	// Spells
	//
	InitSpells();

	//
	// Init units' groups
	//
	InitGroups();

	//
	// Init players?
	//
	DebugPlayers();
	PlayersInitAi();

	//
	// Buttons (botpanel)
	//
	InitButtons();

	//
	// Triggers
	//
	InitTriggers();

	SetDefaultTextColors(UI.NormalFontColor, UI.ReverseFontColor);

#if 0
	if (!UI.SelectedViewport) {
		UI.SelectedViewport = UI.Viewports;
	}
#endif
	UI.SelectedViewport->Center(
		ThisPlayer->StartX, ThisPlayer->StartY, TileSizeX / 2, TileSizeY / 2);

	//
	// Various hacks wich must be done after the map is loaded.
	//
	// FIXME: must be done after map is loaded
	InitPathfinder();
	//
	// FIXME: The palette is loaded after the units are created.
	// FIXME: This loops fixes the colors of the units.
	//
	for (i = 0; i < NumUnits; ++i) {
		// I don't really think that there can be any rescued
		// units at this point.
		if (Units[i]->RescuedFrom) {
			Units[i]->Colors = &Units[i]->RescuedFrom->UnitColors;
		} else {
			Units[i]->Colors = &Units[i]->Player->UnitColors;
		}
	}

	GameResult = GameNoResult;

	CommandLog(NULL, NoUnitP, FlushCommands, -1, -1, NoUnitP, NULL, -1);
	Video.ClearScreen();
}

/**
**  Init Game Setting to default values
**
**  @todo  FIXME: this should not be executed for restart levels!
*/
void InitSettings(void)
{
	for (int i = 0; i < PlayerMax; ++i) {
		GameSettings.Presets[i].Team = SettingsPresetMapDefault;
		GameSettings.Presets[i].Type = (PlayerTypes)SettingsPresetMapDefault;
	}
	GameSettings.Resources = SettingsPresetMapDefault;
	GameSettings.NumUnits = SettingsPresetMapDefault;
	GameSettings.Opponents = SettingsPresetMapDefault;
	GameSettings.Difficulty = SettingsPresetMapDefault;
	GameSettings.GameType = SettingsPresetMapDefault;
	GameSettings.MapRichness = SettingsPresetMapDefault;
	GameSettings.NetGameType = SettingsSinglePlayerGame;
}

//@}
