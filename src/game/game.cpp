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
/**@name game.cpp - The game set-up and creation. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer, Andreas Arens, and
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

#include "stratagus.h"
#include "map.h"
#include "tileset.h"
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
#include "depend.h"
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
#include "commands.h"
#include "iolib.h"
#include "iocompat.h"
#include "replay.h"

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
static void LoadStratagusMap(const std::string &smpname, const std::string &mapname)
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
		char *p = strrchr(mapfull, '/');
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
static int WriteMapPresentation(const std::string &mapname, CMap &map, char *)
{
	FileWriter *f = NULL;
	int i;
	int topplayer;
	int numplayers;
//	char *mapsetupname;
	const char *type[] = {"", "", "neutral", "nobody",
		"computer", "person", "rescue-passive", "rescue-active"};

	numplayers = 0;
	topplayer = PlayerMax - 2;

	try {
		f = CreateFileWriter(mapname);
		f->printf("-- Stratagus Map Presentation\n");
		f->printf("-- File generated by the Stratagus V" VERSION " builtin map editor.\n");
		// MAPTODO Copyright notice in generated file
		f->printf("-- File licensed under the GNU GPL version 2.\n\n");

		f->printf("DefinePlayerTypes(");
		while (topplayer > 0 && map.Info.PlayerType[topplayer] == PlayerNobody) {
			--topplayer;
		}
		for (i = 0; i <= topplayer; ++i) {
			f->printf("%s\"%s\"", (i ? ", " : ""), type[map.Info.PlayerType[i]]);
			if (map.Info.PlayerType[i] == PlayerPerson) {
				++numplayers;
			}
		}
		f->printf(")\n");

		f->printf("PresentMap(\"%s\", %d, %d, %d, %d)\n",
			map.Info.Description.c_str(), numplayers, map.Info.MapWidth, map.Info.MapHeight,
			map.Info.MapUID + 1);

//		mapsetupname = strrchr(mapsetup, '/');
//		if (!mapsetupname) {
//			mapsetupname = mapsetup;
//		}
//		f->printf("DefineMapSetup(GetCurrentLuaPath()..\"%s\")\n", mapsetupname);
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
**  @param mapsetup      map filename
**  @param map           map to save
**  @param writeTerrain  write the tiles map in the .sms
*/
int WriteMapSetup(const char *mapSetup, CMap &map, int writeTerrain)
{
	FileWriter *f = NULL;
	int i, j;

	try {
		f = CreateFileWriter(mapSetup);

		f->printf("-- Stratagus Map Setup\n");
		f->printf("-- File generated by the Stratagus V" VERSION " builtin map editor.\n");
		// MAPTODO Copyright notice in generated file
		f->printf("-- File licensed under the GNU GPL version 2.\n\n");

		f->printf("-- player configuration\n");
		for (i = 0; i < PlayerMax; ++i) {
			if (Players[i].Type == PlayerNobody) {
				continue;
			}
			f->printf("SetStartView(%d, %d, %d)\n", i, Players[i].StartX, Players[i].StartY);
			f->printf("SetPlayerData(%d, \"Resources\", \"%s\", %d)\n",
				i, DefaultResourceNames[WoodCost].c_str(),
				Players[i].Resources[WoodCost]);
			f->printf("SetPlayerData(%d, \"Resources\", \"%s\", %d)\n",
				i, DefaultResourceNames[GoldCost].c_str(),
				Players[i].Resources[GoldCost]);
			f->printf("SetPlayerData(%d, \"Resources\", \"%s\", %d)\n",
				i, DefaultResourceNames[OilCost].c_str(),
				Players[i].Resources[OilCost]);
			f->printf("SetPlayerData(%d, \"RaceName\", \"%s\")\n",
				i, PlayerRaces.Name[Players[i].Race].c_str());
			f->printf("SetAiType(%d, \"%s\")\n",
				i, Players[i].AiName.c_str());
		}
		f->printf("\n");

		f->printf("-- load tilesets\n");
		f->printf("LoadTileModels(\"%s\")\n\n", map.TileModelsFileName);

		if (writeTerrain) {
			f->printf("-- Tiles Map\n");
			for (i = 0; i < map.Info.MapHeight; ++i) {
				for (j = 0; j < map.Info.MapWidth; ++j) {
					int tile;
					int n;

					tile = map.Fields[j + i * map.Info.MapWidth].Tile;
					for (n = 0; n < map.Tileset.NumTiles && tile != map.Tileset.Table[n]; ++n) {
					}
					const int value = map.Fields[j + i * map.Info.MapWidth].Value;
					f->printf("SetTile(%3d, %d, %d, %d)\n", n, j, i, value);
				}
			}
		}

		f->printf("-- place units\n");
		for (i = 0; i < NumUnits; ++i) {
			f->printf("unit = CreateUnit(\"%s\", %d, {%d, %d})\n",
				Units[i]->Type->Ident.c_str(),
				Units[i]->Player->Index,
				Units[i]->tilePos.x, Units[i]->tilePos.y);
			if (Units[i]->Type->GivesResource) {
				f->printf("SetResourcesHeld(unit, %d)\n", Units[i]->ResourcesHeld);
			}
		}
		f->printf("\n\n");
	} catch (const FileException &) {
		fprintf(stderr,"Can't save map setup : `%s' \n", mapSetup);
		delete f;
		return -1;
	}

	delete f;
	return 1;
}



/**
**  Save a Stratagus map.
**
**  @param mapName   map filename
**  @param map       map to save
**  @param writeTerrain   write the tiles map in the .sms
*/
int SaveStratagusMap(const std::string &mapName, CMap &map, int writeTerrain)
{
	char mapSetup[PATH_MAX];
	char *extension;

	if (!map.Info.MapWidth || !map.Info.MapHeight) {
		fprintf(stderr, "%s: invalid Stratagus map\n", mapName.c_str());
		ExitFatal(-1);
	}

	strcpy_s(mapSetup, sizeof(mapSetup), mapName.c_str());
	extension = strstr(mapSetup, ".smp");
	if (!extension) {
		fprintf(stderr, "%s: invalid Statagus map filename\n", mapName.c_str());
	}
	memcpy(extension, ".sms", 4 * sizeof(char));

	if (WriteMapPresentation(mapName, map, mapSetup) == -1) {
		return -1;
	}

	return WriteMapSetup(mapSetup, map, writeTerrain);
}


/**
**  Load any map.
**
**  @param filename  map filename
**  @param map       map loaded
*/
static void LoadMap(const std::string &filename, CMap &map)
{
	const char *tmp;
	const char *name = filename.c_str();

	tmp = strrchr(name, '.');
	if (tmp) {
#ifdef USE_ZLIB
		if (!strcmp(tmp, ".gz")) {
			while (tmp - 1 > name && *--tmp != '.') {
			}
		}
#endif
#ifdef USE_BZ2LIB
		if (!strcmp(tmp, ".bz2")) {
			while (tmp - 1 > name && *--tmp != '.') {
			}
		}
#endif
		if (!strcmp(tmp, ".smp")
#ifdef USE_ZLIB
				|| !strcmp(tmp, ".smp.gz")
#endif
#ifdef USE_BZ2LIB
				|| !strcmp(tmp, ".smp.bz2")
#endif
		) {
			if (map.Info.Filename.empty()) {
				// The map info hasn't been loaded yet => do it now
				LoadStratagusMapInfo(filename);
			}
			Assert(!map.Info.Filename.empty());
			map.Create();
			LoadStratagusMap(filename, map.Info.Filename.c_str());
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
static void GameTypeFreeForAll()
{
	for (int i = 0; i < PlayerMax - 1; ++i) {
		for (int j = i + 1; j < PlayerMax - 1; ++j) {
			CommandDiplomacy(i, DiplomacyEnemy, j);
			CommandDiplomacy(j, DiplomacyEnemy, i);
		}
	}
}

/**
**  Top vs Bottom
*/
static void GameTypeTopVsBottom()
{
	const int middle = Map.Info.MapHeight / 2;

	for (int i = 0; i < PlayerMax - 1; ++i) {
		const bool top_i = Players[i].StartY <= middle;

		for (int j = i + 1; j < PlayerMax - 1; ++j) {
			const bool top_j = Players[j].StartY <= middle;

			if (top_i == top_j) {
				CommandDiplomacy(i, DiplomacyAllied, j);
				Players[i].SharedVision |= (1 << j);
				CommandDiplomacy(j, DiplomacyAllied, i);
				Players[j].SharedVision |= (1 << i);
			} else {
				CommandDiplomacy(i, DiplomacyEnemy, j);
				CommandDiplomacy(j, DiplomacyEnemy, i);
			}
		}
	}
}

/**
**  Left vs Right
*/
static void GameTypeLeftVsRight()
{
	const int middle = Map.Info.MapWidth / 2;

	for (int i = 0; i < PlayerMax - 1; ++i) {
		const bool left_i = Players[i].StartX <= middle;

		for (int j = i + 1; j < PlayerMax - 1; ++j) {
			const bool left_j = Players[j].StartX <= middle;

			if (left_i ==left_j) {
				CommandDiplomacy(i, DiplomacyAllied, j);
				Players[i].SharedVision |= (1 << j);
				CommandDiplomacy(j, DiplomacyAllied, i);
				Players[j].SharedVision |= (1 << i);
			} else {
				CommandDiplomacy(i, DiplomacyEnemy, j);
				CommandDiplomacy(j, DiplomacyEnemy, i);
			}
		}
	}
}

/**
**  Man vs Machine
*/
static void GameTypeManVsMachine()
{
	for (int i = 0; i < PlayerMax - 1; ++i) {
		if (Players[i].Type != PlayerPerson && Players[i].Type != PlayerComputer) {
			continue;
		}
		for (int j = i + 1; j < PlayerMax - 1; ++j) {
			if (Players[j].Type != PlayerPerson && Players[j].Type != PlayerComputer) {
				continue;
			}
			if (Players[i].Type == Players[j].Type) {
				CommandDiplomacy(i, DiplomacyAllied, j);
				Players[i].SharedVision |= (1 << j);
				CommandDiplomacy(j, DiplomacyAllied, i);
				Players[j].SharedVision |= (1 << i);
			} else {
				CommandDiplomacy(i, DiplomacyEnemy, j);
				CommandDiplomacy(j, DiplomacyEnemy, i);
			}
		}
	}
}

/**
**  Man vs Machine whith Humans on a Team
*/
static void GameTypeManTeamVsMachine()
{
	for (int i = 0; i < PlayerMax - 1; ++i) {
		if (Players[i].Type != PlayerPerson && Players[i].Type != PlayerComputer) {
			continue;
		}
		for (int j = 0; j < PlayerMax - 1; ++j) {
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

	InitPlayers();

	if (Map.Info.Filename.empty() && !filename.empty()) {
		char path[PATH_MAX];

		LibraryFileName(filename.c_str(), path, sizeof(path));
		if (strcasestr(filename.c_str(), ".smp")) {
			LuaLoadFile(path);
		}
	}

	for (i = 0; i < PlayerMax; ++i) {
		int playertype = (PlayerTypes)Map.Info.PlayerType[i];
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
		LoadMap(filename, *map);
	}
	CclCommand("if (MapLoaded ~= nil) then MapLoaded() end");

	GameCycle = 0;
	FastForwardCycle = 0;
	SyncHash = 0;
	InitSyncRand();

	if (IsNetworkGame()) { // Prepare network play
		DebugPrint("Client setup: Calling InitNetwork2\n");
		InitNetwork2();
	} else {
		const std::string& localPlayerName = Parameters::Instance.LocalPlayerName;

		if (!localPlayerName.empty() && localPlayerName != "Anonymous") {
			ThisPlayer->SetName(localPlayerName);
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
	// FIXME: implement more game types
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

			// Future game type ideas
#if 0
			case SettingsGameTypeOneOnOne:
				break;
			case SettingsGameTypeCaptureTheFlag:
				break;
			case SettingsGameTypeGreed:
				break;
			case SettingsGameTypeSlaughter:
				break;
			case SettingsGameTypeSuddenDeath:
				break;
			case SettingsGameTypeTeamMelee:
				break;
			case SettingsGameTypeTeamCaptureTheFlag:
				break;
#endif
		}
	}

	//
	// Graphic part
	//
	SetPlayersPalette();
	InitIcons();
	LoadIcons();

	LoadCursors(PlayerRaces.Name[ThisPlayer->Race]);
	UnitUnderCursor.Reset();

	InitMissileTypes();
#ifndef DYNAMIC_LOAD
	LoadMissileSprites();
#endif
	InitConstructions();
	LoadConstructions();
	LoadUnitTypes();
	LoadDecorations();

	InitSelections();

	InitUserInterface();
	UI.Load();

	Map.Init();
	UI.Minimap.Create();
	PreprocessMap();

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
	// Upgrades
	//
	InitUpgrades();

	//
	// Dependencies
	//
	InitDependencies();

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
	const Vec2i start = {ThisPlayer->StartX, ThisPlayer->StartY};
	UI.SelectedViewport->Center(start, PixelTileSize / 2);

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
void InitSettings()
{
	for (int i = 0; i < PlayerMax; ++i) {
		GameSettings.Presets[i].Race = SettingsPresetMapDefault;
		GameSettings.Presets[i].Team = SettingsPresetMapDefault;
		GameSettings.Presets[i].Type = SettingsPresetMapDefault;
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
