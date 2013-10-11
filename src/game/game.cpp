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

#include <png.h>

#include "stratagus.h"

#include "game.h"

#include "actions.h"
#include "ai.h"
#include "animation.h"
#include "commands.h"
#include "construct.h"
#include "depend.h"
#include "editor.h"
#include "font.h"
#include "interface.h"
#include "iocompat.h"
#include "iolib.h"
#include "map.h"
#include "minimap.h"
#include "missile.h"
#include "netconnect.h"
#include "network.h"
#include "parameters.h"
#include "pathfinder.h"
#include "player.h"
#include "replay.h"
#include "results.h"
#include "settings.h"
#include "sound.h"
#include "sound_server.h"
#include "spells.h"
#include "tileset.h"
#include "translate.h"
#include "trigger.h"
#include "ui.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"
#include "upgrade.h"
#include "version.h"
#include "video.h"


extern void CleanGame();

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

Settings GameSettings;  /// Game Settings
static int LcmPreventRecurse;   /// prevent recursion through LoadGameMap
GameResults GameResult;                      /// Outcome of the game

std::string GameName;
std::string FullGameName;

unsigned long GameCycle;             /// Game simulation cycle counter
unsigned long FastForwardCycle;      /// Cycle to fastforward to in a replay

bool UseHPForXp = false;              /// true if gain XP by dealing damage, false if by killing.

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern gcn::Gui *Gui;
static std::vector<gcn::Container *> Containers;

/**
**  Save game settings.
**
**  @param file  Save file handle
*/
void SaveGameSettings(CFile &file)
{
	file.printf("\nGameSettings.NetGameType = %d\n", GameSettings.NetGameType);
	for (int i = 0; i < PlayerMax - 1; ++i) {
		file.printf("GameSettings.Presets[%d].Race = %d\n", i, GameSettings.Presets[i].Race);
		file.printf("GameSettings.Presets[%d].Team = %d\n", i, GameSettings.Presets[i].Team);
		file.printf("GameSettings.Presets[%d].Type = %d\n", i, GameSettings.Presets[i].Type);
	}
	file.printf("GameSettings.Resources = %d\n", GameSettings.Resources);
	file.printf("GameSettings.Difficulty = %d\n", GameSettings.Difficulty);
	file.printf("GameSettings.NumUnits = %d\n", GameSettings.NumUnits);
	file.printf("GameSettings.Opponents = %d\n", GameSettings.Opponents);
	file.printf("GameSettings.GameType = %d\n", GameSettings.GameType);
	file.printf("GameSettings.NoFogOfWar = %s\n", GameSettings.NoFogOfWar ? "true" : "false");
	file.printf("GameSettings.RevealMap = %d\n", GameSettings.RevealMap);
	file.printf("GameSettings.MapRichness = %d\n", GameSettings.MapRichness);
	file.printf("\n");
}

void StartMap(const std::string &filename, bool clean)
{
	std::string nc, rc;

	gcn::Widget *oldTop = Gui->getTop();
	gcn::Container *container = new gcn::Container();
	Containers.push_back(container);
	container->setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
	container->setOpaque(false);
	Gui->setTop(container);

	NetConnectRunning = 0;
	InterfaceState = IfaceStateNormal;

	//  Create the game.
	DebugPrint("Creating game with map: %s\n" _C_ filename.c_str());
	if (clean) {
		CleanPlayers();
	}
	GetDefaultTextColors(nc, rc);

	CreateGame(filename, &Map);

	UI.StatusLine.Set(NameLine);
	SetMessage("%s", _("Do it! Do it now!"));

	//  Play the game.
	GameMainLoop();

	//  Clear screen
	Video.ClearScreen();
	Invalidate();

	CleanGame();
	InterfaceState = IfaceStateMenu;
	SetDefaultTextColors(nc, rc);

	Gui->setTop(oldTop);
	Containers.erase(std::find(Containers.begin(), Containers.end(), container));
	delete container;
}

void FreeAllContainers()
{
	for (size_t i = 0; i != Containers.size(); ++i) {
		delete Containers[i];
	}
}

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

// Write a small image of map preview
static void WriteMapPreview(const char *mapname, CMap &map)
{
	FILE *fp = fopen(mapname, "wb");
	if (fp == NULL) {
		return;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_write_struct(&png_ptr, NULL);
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		/* If we get here, we had a problem reading the file */
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return;
	}

	/* set up the output control if you are using standard C streams */
	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, UI.Minimap.W, UI.Minimap.H, 8,
				 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
				 PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);

	const int rectSize = 5; // size of rectange used for player start spots
#if defined(USE_OPENGL) || defined(USE_GLES)
	if (UseOpenGL) {
		unsigned char *pixels = new unsigned char[UI.Minimap.W * UI.Minimap.H * 3];
		if (!pixels) {
			fprintf(stderr, "Out of memory\n");
			exit(1);
		}
		// Copy GL map surface to pixel array
		for (int i = 0; i < UI.Minimap.H; ++i) {
			for (int j = 0; j < UI.Minimap.W; ++j) {
				Uint32 c = ((Uint32 *)MinimapSurfaceGL)[j + i * UI.Minimap.W];
				const int offset = (i * UI.Minimap.W + j) * 3;
				pixels[offset + 0] = ((c & RMASK) >> RSHIFT);
				pixels[offset + 1] = ((c & GMASK) >> GSHIFT);
				pixels[offset + 2] = ((c & BMASK) >> BSHIFT);
			}
		}
		// Add player start spots
		for (int i = 0; i < PlayerMax - 1; ++i) {
			if (Players[i].Type != PlayerNobody) {
				for (int j = -rectSize / 2; j <= rectSize / 2; ++j) {
					for (int k = -rectSize / 2; k <= rectSize / 2; ++k) {
						const int miniMapX = Players[i].StartPos.x * UI.Minimap.W / map.Info.MapWidth;
						const int miniMapY = Players[i].StartPos.y * UI.Minimap.H / map.Info.MapHeight;
						if (miniMapX + j < 0 || miniMapX + j >= UI.Minimap.W) {
							continue;
						}
						if (miniMapY + k < 0 || miniMapY + k >= UI.Minimap.H) {
							continue;
						}
						const int offset = ((miniMapY + k) * UI.Minimap.H + miniMapX + j) * 3;
						pixels[offset + 0] = ((Players[i].Color & RMASK) >> RSHIFT);
						pixels[offset + 1] = ((Players[i].Color & GMASK) >> GSHIFT);
						pixels[offset + 2] = ((Players[i].Color & BMASK) >> BSHIFT);
					}
				}
			}
		}
		// Write everything in PNG
		for (int i = 0; i < UI.Minimap.H; ++i) {
			unsigned char *row = new unsigned char[UI.Minimap.W * 3];
			memcpy(row, pixels + i * UI.Minimap.W * 3, UI.Minimap.W * 3);
			png_write_row(png_ptr, row);
			delete[] row;
		}
		delete[] pixels;
	} else
#endif
	{
		unsigned char *row = new unsigned char[UI.Minimap.W * 3];
		const SDL_PixelFormat *fmt = MinimapSurface->format;
		SDL_Surface *preview = SDL_CreateRGBSurface(SDL_SWSURFACE,
													UI.Minimap.W, UI.Minimap.H, 32, fmt->Rmask, fmt->Gmask, fmt->Bmask, 0);
		SDL_BlitSurface(MinimapSurface, NULL, preview, NULL);

		SDL_LockSurface(preview);

		SDL_Rect rect;
		for (int i = 0; i < PlayerMax - 1; ++i) {
			if (Players[i].Type != PlayerNobody) {
				rect.x = Players[i].StartPos.x * UI.Minimap.W / map.Info.MapWidth - rectSize / 2;
				rect.y = Players[i].StartPos.y * UI.Minimap.H / map.Info.MapHeight - rectSize / 2;
				rect.w = rect.h = rectSize;
				SDL_FillRect(preview, &rect, Players[i].Color);
			}
		}

		for (int i = 0; i < UI.Minimap.H; ++i) {
			switch (preview->format->BytesPerPixel) {
				case 1:
					for (int j = 0; j < UI.Minimap.W; ++j) {
						Uint8 c = ((Uint8 *)preview->pixels)[j + i * UI.Minimap.W];
						row[j * 3 + 0] = fmt->palette->colors[c].r;
						row[j * 3 + 1] = fmt->palette->colors[c].g;
						row[j * 3 + 2] = fmt->palette->colors[c].b;
					}
					break;
				case 3:
					memcpy(row, (char *)preview->pixels + i * UI.Minimap.W, UI.Minimap.W * 3);
					break;
				case 4:
					for (int j = 0; j < UI.Minimap.W; ++j) {
						Uint32 c = ((Uint32 *)preview->pixels)[j + i * UI.Minimap.W];
						row[j * 3 + 0] = ((c & fmt->Rmask) >> fmt->Rshift);
						row[j * 3 + 1] = ((c & fmt->Gmask) >> fmt->Gshift);
						row[j * 3 + 2] = ((c & fmt->Bmask) >> fmt->Bshift);
					}
					break;
			}
			png_write_row(png_ptr, row);
		}
		delete[] row;

		SDL_UnlockSurface(preview);
		SDL_FreeSurface(preview);
	}

	png_write_end(png_ptr, info_ptr);

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
}


// Write the map presentation file
static int WriteMapPresentation(const std::string &mapname, CMap &map)
{
	FileWriter *f = NULL;

	const char *type[] = {"", "", "neutral", "nobody",
						  "computer", "person", "rescue-passive", "rescue-active"
						 };

	int numplayers = 0;
	int topplayer = PlayerMax - 2;

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
		for (int i = 0; i <= topplayer; ++i) {
			f->printf("%s\"%s\"", (i ? ", " : ""), type[map.Info.PlayerType[i]]);
			if (map.Info.PlayerType[i] == PlayerPerson) {
				++numplayers;
			}
		}
		f->printf(")\n");

		f->printf("PresentMap(\"%s\", %d, %d, %d, %d)\n",
				  map.Info.Description.c_str(), numplayers, map.Info.MapWidth, map.Info.MapHeight,
				  map.Info.MapUID + 1);

		if (map.Info.Filename.find(".sms") == std::string::npos && !map.Info.Filename.empty()) {
			f->printf("DefineMapSetup(\"%s\")\n", map.Info.Filename.c_str());
		}
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

	try {
		f = CreateFileWriter(mapSetup);

		f->printf("-- Stratagus Map Setup\n");
		f->printf("-- File generated by the Stratagus V" VERSION " builtin map editor.\n");
		// MAPTODO Copyright notice in generated file
		f->printf("-- File licensed under the GNU GPL version 2.\n\n");

		f->printf("-- player configuration\n");
		for (int i = 0; i < PlayerMax; ++i) {
			if (Players[i].Type == PlayerNobody) {
				continue;
			}
			f->printf("SetStartView(%d, %d, %d)\n", i, Players[i].StartPos.x, Players[i].StartPos.y);
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
		f->printf("LoadTileModels(\"%s\")\n\n", map.TileModelsFileName.c_str());

		if (writeTerrain) {
			f->printf("-- Tiles Map\n");
			for (int i = 0; i < map.Info.MapHeight; ++i) {
				for (int j = 0; j < map.Info.MapWidth; ++j) {
					const CMapField &mf = map.Fields[j + i * map.Info.MapWidth];
					const int tile = mf.getGraphicTile();
					const int n = map.Tileset->findTileIndexByTile(tile);
					const int value = mf.Value;
					f->printf("SetTile(%3d, %d, %d, %d)\n", n, j, i, value);
				}
			}
		}

		f->printf("-- place units\n");
		f->printf("if (MapUnitsInit ~= nil) then MapUnitsInit() end\n");
		for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
			const CUnit &unit = **it;
			f->printf("unit = CreateUnit(\"%s\", %d, {%d, %d})\n",
					  unit.Type->Ident.c_str(),
					  unit.Player->Index,
					  unit.tilePos.x, unit.tilePos.y);
			if (unit.Type->GivesResource) {
				f->printf("SetResourcesHeld(unit, %d)\n", unit.ResourcesHeld);
			}
		}
		f->printf("\n\n");
	} catch (const FileException &) {
		fprintf(stderr, "Can't save map setup : `%s' \n", mapSetup);
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
	if (!map.Info.MapWidth || !map.Info.MapHeight) {
		fprintf(stderr, "%s: invalid Stratagus map\n", mapName.c_str());
		ExitFatal(-1);
	}

	char mapSetup[PATH_MAX];
	strcpy_s(mapSetup, sizeof(mapSetup), mapName.c_str());
	char *setupExtension = strstr(mapSetup, ".smp");
	if (!setupExtension) {
		fprintf(stderr, "%s: invalid Stratagus map filename\n", mapName.c_str());
	}

	char previewName[PATH_MAX];
	strcpy_s(previewName, sizeof(previewName), mapName.c_str());
	char *previewExtension = strstr(previewName, ".smp");
	memcpy(previewExtension, ".png\0", 5 * sizeof(char));
	WriteMapPreview(previewName, map);

	memcpy(setupExtension, ".sms", 4 * sizeof(char));
	if (WriteMapPresentation(mapName, map) == -1) {
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
	const char *name = filename.c_str();
	const char *tmp = strrchr(name, '.');
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
			LoadStratagusMap(filename, map.Info.Filename);
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
		const bool top_i = Players[i].StartPos.y <= middle;

		for (int j = i + 1; j < PlayerMax - 1; ++j) {
			const bool top_j = Players[j].StartPos.y <= middle;

			if (top_i == top_j) {
				CommandDiplomacy(i, DiplomacyAllied, j);
				Players[i].ShareVisionWith(Players[j]);
				CommandDiplomacy(j, DiplomacyAllied, i);
				Players[j].ShareVisionWith(Players[i]);
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
		const bool left_i = Players[i].StartPos.x <= middle;

		for (int j = i + 1; j < PlayerMax - 1; ++j) {
			const bool left_j = Players[j].StartPos.x <= middle;

			if (left_i == left_j) {
				CommandDiplomacy(i, DiplomacyAllied, j);
				Players[i].ShareVisionWith(Players[j]);
				CommandDiplomacy(j, DiplomacyAllied, i);
				Players[j].ShareVisionWith(Players[i]);
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
				Players[i].ShareVisionWith(Players[j]);
				CommandDiplomacy(j, DiplomacyAllied, i);
				Players[j].ShareVisionWith(Players[i]);
			} else {
				CommandDiplomacy(i, DiplomacyEnemy, j);
				CommandDiplomacy(j, DiplomacyEnemy, i);
			}
		}
	}
}

/**
**  Man vs Machine with Humans on a Team
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
					Players[i].ShareVisionWith(Players[j]);
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
	if (SaveGameLoading) {
		SaveGameLoading = false;
		// Load game, already created game with Init/LoadModules
		CommandLog(NULL, NoUnitP, FlushCommands, -1, -1, NoUnitP, NULL, -1);
		return;
	}

	InitPlayers();

	if (Map.Info.Filename.empty() && !filename.empty()) {
		const std::string path = LibraryFileName(filename.c_str());

		if (strcasestr(filename.c_str(), ".smp")) {
			LuaLoadFile(path);
		}
	}

	for (int i = 0; i < PlayerMax; ++i) {
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
		NetworkOnStartGame();
	} else {
		const std::string &localPlayerName = Parameters::Instance.LocalPlayerName;

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
	LoadIcons();

	LoadCursors(PlayerRaces.Name[ThisPlayer->Race]);
	UnitUnderCursor = NoUnitP;

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
	UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(ThisPlayer->StartPos));

	//
	// Various hacks which must be done after the map is loaded.
	//
	// FIXME: must be done after map is loaded
	InitPathfinder();
	//
	// FIXME: The palette is loaded after the units are created.
	// FIXME: This loops fixes the colors of the units.
	//
	for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
		CUnit &unit = **it;
		// I don't really think that there can be any rescued units at this point.
		if (unit.RescuedFrom) {
			unit.Colors = &unit.RescuedFrom->UnitColors;
		} else {
			unit.Colors = &unit.Player->UnitColors;
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

// call the lua function: CleanGame_Lua.
static void CleanGame_Lua()
{
	lua_getglobal(Lua, "CleanGame_Lua");
	if (lua_isfunction(Lua, -1)) {
		LuaCall(0, 1);
	} else {
		lua_pop(Lua, 1);
	}
}

/**
**  Cleanup game.
**
**  Call each module to clean up.
**  Contrary to CleanModules, maps can be restarted
**  without reloading all lua files.
*/
void CleanGame()
{
	EndReplayLog();
	CleanMessages();

	RestoreColorCyclingSurface();
	CleanGame_Lua();
	CleanTriggers();
	CleanAi();
	CleanGroups();
	CleanMissiles();
	CleanUnits();
	CleanSelections();
	Map.Clean();
	CleanReplayLog();
	FreePathfinder();
	CursorBuilding = NULL;
	UnitUnderCursor = NULL;
}

/**
**  Return of game name.
**
**  @param l  Lua state.
*/
static int CclSetGameName(lua_State *l)
{
	const int args = lua_gettop(l);
	if (args > 1 || (args == 1 && (!lua_isnil(l, 1) && !lua_isstring(l, 1)))) {
		LuaError(l, "incorrect argument");
	}
	if (args == 1 && !lua_isnil(l, 1)) {
		GameName = lua_tostring(l, 1);
	}

	if (!GameName.empty()) {
		std::string path = Parameters::Instance.GetUserDirectory() + "/" + GameName;
		makedir(path.c_str(), 0777);
	}
	return 0;
}

static int CclSetFullGameName(lua_State *l)
{
	const int args = lua_gettop(l);
	if (args > 1 || (args == 1 && (!lua_isnil(l, 1) && !lua_isstring(l, 1)))) {
		LuaError(l, "incorrect argument");
	}
	if (args == 1 && !lua_isnil(l, 1)) {
		FullGameName = lua_tostring(l, 1);
	}
	return 0;
}

/**
**  Set God mode.
**
**  @param l  Lua state.
**
**  @return   The old mode.
*/
static int CclSetGodMode(lua_State *l)
{
	LuaCheckArgs(l, 1);
	GodMode = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Get God mode.
**
**  @param l  Lua state.
**
**  @return   God mode.
*/
static int CclGetGodMode(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushboolean(l, GodMode);
	return 1;
}

/**
**  Set resource harvesting speed (deprecated).
**
**  @param l  Lua state.
*/
static int CclSetSpeedResourcesHarvest(lua_State *l)
{
	LuaCheckArgs(l, 3);

	const int player = LuaToNumber(l, 1);
	const std::string resource = LuaToString(l, 2);
	const int resId = GetResourceIdByName(l, resource.c_str());

	Players[player].SpeedResourcesHarvest[resId] = LuaToNumber(l, 3);
	return 0;
}

/**
**  Set resource returning speed (deprecated).
**
**  @param l  Lua state.
*/
static int CclSetSpeedResourcesReturn(lua_State *l)
{
	LuaCheckArgs(l, 3);

	const int player = LuaToNumber(l, 1);
	const std::string resource = LuaToString(l, 2);
	const int resId = GetResourceIdByName(l, resource.c_str());

	Players[player].SpeedResourcesReturn[resId] = LuaToNumber(l, 3);
	return 0;
}

/**
**  Set building speed (deprecated).
**
**  @param l  Lua state.
*/
static int CclSetSpeedBuild(lua_State *l)
{
	LuaCheckArgs(l, 2);
	const int player = LuaToNumber(l, 1);
	Players[player].SpeedBuild = LuaToNumber(l, 2);
	return 0;
}

/**
**  Get building speed (deprecated).
**
**  @param l  Lua state.
**
**  @return   Building speed.
*/
static int CclGetSpeedBuild(lua_State *l)
{
	LuaCheckArgs(l, 1);
	const int player = LuaToNumber(l, 1);
	lua_pushnumber(l, Players[player].SpeedBuild);
	return 1;
}

/**
**  Set training speed (deprecated).
**
**  @param l  Lua state.
*/
static int CclSetSpeedTrain(lua_State *l)
{
	LuaCheckArgs(l, 2);
	const int player = LuaToNumber(l, 1);
	Players[player].SpeedTrain = LuaToNumber(l, 2);
	return 0;
}

/**
**  Get training speed (deprecated).
**
**  @param l  Lua state.
**
**  @return   Training speed.
*/
static int CclGetSpeedTrain(lua_State *l)
{
	LuaCheckArgs(l, 1);
	const int player = LuaToNumber(l, 1);
	lua_pushnumber(l, Players[player].SpeedTrain);
	return 1;
}

/**
**  For debug increase upgrading speed (deprecated).
**
**  @param l  Lua state.
*/
static int CclSetSpeedUpgrade(lua_State *l)
{
	LuaCheckArgs(l, 2);
	const int player = LuaToNumber(l, 1);
	Players[player].SpeedUpgrade = LuaToNumber(l, 2);

	lua_pushnumber(l, Players[player].SpeedUpgrade);
	return 1;
}

/**
**  For debug increase researching speed (deprecated).
**
**  @param l  Lua state.
*/
static int CclSetSpeedResearch(lua_State *l)
{
	LuaCheckArgs(l, 2);
	const int player = LuaToNumber(l, 1);
	Players[player].SpeedResearch = LuaToNumber(l, 2);

	lua_pushnumber(l, Players[player].SpeedResearch);
	return 1;
}

/**
**  For debug increase all speeds (deprecated).
**
**  @param l  Lua state.
*/
static int CclSetSpeeds(lua_State *l)
{
	LuaCheckArgs(l, 1);
	const int speed = LuaToNumber(l, 1);
	for (int i = 0; i < PlayerMax; ++i) {
		for (int j = 0; j < MaxCosts; ++j) {
			Players[i].SpeedResourcesHarvest[j] = speed;
			Players[i].SpeedResourcesReturn[j] = speed;
		}
		Players[i].SpeedBuild = Players[i].SpeedTrain = Players[i].SpeedUpgrade = Players[i].SpeedResearch = speed;
	}

	lua_pushnumber(l, speed);
	return 1;
}

/**
**  Define default incomes for a new player.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultIncomes(lua_State *l)
{
	const int args = lua_gettop(l);
	for (int i = 0; i < MaxCosts && i < args; ++i) {
		DefaultIncomes[i] = LuaToNumber(l, i + 1);
	}
	return 0;
}

/**
**  Define default action for the resources.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultActions(lua_State *l)
{
	for (unsigned int i = 0; i < MaxCosts; ++i) {
		DefaultActions[i].clear();
	}
	const unsigned int args = lua_gettop(l);
	for (unsigned int i = 0; i < MaxCosts && i < args; ++i) {
		DefaultActions[i] = LuaToString(l, i + 1);
	}
	return 0;
}

/**
**  Define default names for the resources.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultResourceNames(lua_State *l)
{
	for (unsigned int i = 0; i < MaxCosts; ++i) {
		DefaultResourceNames[i].clear();
	}
	const unsigned int args = lua_gettop(l);
	for (unsigned int i = 0; i < MaxCosts && i < args; ++i) {
		DefaultResourceNames[i] = LuaToString(l, i + 1);
	}
	return 0;
}

/**
**  Define default names for the resources.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultResourceAmounts(lua_State *l)
{
	const unsigned int args = lua_gettop(l);

	if (args & 1) {
		LuaError(l, "incorrect argument");
	}
	for (unsigned int j = 0; j < args; ++j) {
		const std::string resource = LuaToString(l, j + 1);
		const int resId = GetResourceIdByName(l, resource.c_str());

		++j;
		DefaultResourceAmounts[resId] = LuaToNumber(l, j + 1);
	}
	return 0;
}

/**
**  Define max amounts for the resources.
**
**  @param l  Lua state.
*/
static int CclDefineDefaultResourceMaxAmounts(lua_State *l)
{
	const int args = std::min<int>(lua_gettop(l), MaxCosts);

	for (int i = 0; i < args; ++i) {
		DefaultResourceMaxAmounts[i] = LuaToNumber(l, i + 1);
	}
	for (int i = args; i < MaxCosts; ++i) {
		DefaultResourceMaxAmounts[i] = -1;
	}
	return 0;
}

/**
**  Affect UseHPForXp.
**
**  @param l  Lua state.
**
**  @return 0.
*/
static int ScriptSetUseHPForXp(lua_State *l)
{
	LuaCheckArgs(l, 1);
	UseHPForXp = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Set the local player name
**
**  @param l  Lua state.
*/
static int CclSetLocalPlayerName(lua_State *l)
{
	LuaCheckArgs(l, 1);
	Parameters::Instance.LocalPlayerName = LuaToString(l, 1);
	return 0;
}

/**
**  Get the local player name
**
**  @param l  Lua state.
*/
static int CclGetLocalPlayerName(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushstring(l, Parameters::Instance.LocalPlayerName.c_str());
	return 1;
}

/**
**  Get Stratagus Version
*/
static int CclGetStratagusVersion(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushstring(l, VERSION);
	return 1;
}

/**
**  Get Stratagus Homepage
*/
static int CclGetStratagusHomepage(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushstring(l, HOMEPAGE);
	return 1;
}

static int CclSetMenuRace(lua_State *l)
{
	LuaCheckArgs(l, 1);
	MenuRace = LuaToString(l, 1);
	return 0;
}

/**
**  Load the SavedGameInfo Header
**
**  @param l  Lua state.
*/
static int CclSavedGameInfo(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

	for (lua_pushnil(l); lua_next(l, 1); lua_pop(l, 1)) {
		const char *value = LuaToString(l, -2);

		if (!strcmp(value, "SaveFile")) {
			if (strcpy_s(CurrentMapPath, sizeof(CurrentMapPath), LuaToString(l, -1)) != 0) {
				LuaError(l, "SaveFile too long");
			}
			std::string buf = StratagusLibPath;
			buf += "/";
			buf += LuaToString(l, -1);
			if (LuaLoadFile(buf) == -1) {
				DebugPrint("Load failed: %s\n" _C_ value);
			}
		} else if (!strcmp(value, "SyncHash")) {
			SyncHash = LuaToNumber(l, -1);
		} else if (!strcmp(value, "SyncRandSeed")) {
			SyncRandSeed = LuaToNumber(l, -1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	return 0;
}

void LuaRegisterModules()
{
	lua_register(Lua, "SetGameName", CclSetGameName);
	lua_register(Lua, "SetFullGameName", CclSetFullGameName);

	lua_register(Lua, "SetGodMode", CclSetGodMode);
	lua_register(Lua, "GetGodMode", CclGetGodMode);

	lua_register(Lua, "SetSpeedResourcesHarvest", CclSetSpeedResourcesHarvest);
	lua_register(Lua, "SetSpeedResourcesReturn", CclSetSpeedResourcesReturn);
	lua_register(Lua, "SetSpeedBuild", CclSetSpeedBuild);
	lua_register(Lua, "GetSpeedBuild", CclGetSpeedBuild);
	lua_register(Lua, "SetSpeedTrain", CclSetSpeedTrain);
	lua_register(Lua, "GetSpeedTrain", CclGetSpeedTrain);
	lua_register(Lua, "SetSpeedUpgrade", CclSetSpeedUpgrade);
	lua_register(Lua, "SetSpeedResearch", CclSetSpeedResearch);
	lua_register(Lua, "SetSpeeds", CclSetSpeeds);

	lua_register(Lua, "DefineDefaultIncomes", CclDefineDefaultIncomes);
	lua_register(Lua, "DefineDefaultActions", CclDefineDefaultActions);
	lua_register(Lua, "DefineDefaultResourceNames", CclDefineDefaultResourceNames);
	lua_register(Lua, "DefineDefaultResourceAmounts", CclDefineDefaultResourceAmounts);
	lua_register(Lua, "DefineDefaultResourceMaxAmounts", CclDefineDefaultResourceMaxAmounts);

	lua_register(Lua, "SetUseHPForXp", ScriptSetUseHPForXp);
	lua_register(Lua, "SetLocalPlayerName", CclSetLocalPlayerName);
	lua_register(Lua, "GetLocalPlayerName", CclGetLocalPlayerName);

	lua_register(Lua, "SetMenuRace", CclSetMenuRace);

	lua_register(Lua, "GetStratagusVersion", CclGetStratagusVersion);
	lua_register(Lua, "GetStratagusHomepage", CclGetStratagusHomepage);

	lua_register(Lua, "SavedGameInfo", CclSavedGameInfo);

	AiCclRegister();
	AnimationCclRegister();
	ConstructionCclRegister();
	DecorationCclRegister();
	DependenciesCclRegister();
	EditorCclRegister();
	GroupCclRegister();
	MapCclRegister();
	MissileCclRegister();
	NetworkCclRegister();
	PathfinderCclRegister();
	PlayerCclRegister();
	ReplayCclRegister();
	ScriptRegister();
	SelectionCclRegister();
	SoundCclRegister();
	SpellCclRegister();
	TriggerCclRegister();
	UnitCclRegister();
	UnitTypeCclRegister();
	UpgradesCclRegister();
	UserInterfaceCclRegister();
	VideoCclRegister();
}


//@}
