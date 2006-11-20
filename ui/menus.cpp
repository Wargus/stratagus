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
/**@name menus.cpp - The menu function code. */
//
//      (c) Copyright 1999-2006 by Andreas Arens, Jimmy Salmon, Nehal Mistry
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "map.h"
#include "netconnect.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

#if 0
	/// Editor cancel button pressed
static int EditorCancelled;
#endif

/**
** Other client and server selection state for Multiplayer clients
*/
ServerSetup ServerSetupState;
ServerSetup LocalSetupState;

char MenuMapFullPath[1024];              /// Selected map path+name

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

#if 0
/**
** Save replay Ok button.
*/
static void SaveReplayOk(void)
{
	FILE *fd;
	Menu *menu;
	char *buf;
	struct stat s;
	char *ptr;

	menu = FindMenu("menu-save-replay");

	if (strchr(menu->Items[1].D.Input.buffer, '/')) {
		ErrorMenu("Name cannot contain '/'");
		return;
	}
	if (strchr(menu->Items[1].D.Input.buffer, '\\')) {
		ErrorMenu("Name cannot contain '\\'");
		return;
	}

#ifdef WIN32
	sprintf(TempPathBuf, "%s/logs/", GameName.c_str());
#else
	sprintf(TempPathBuf, "%s/%s/%s", getenv("HOME"), STRATAGUS_HOME_PATH, GameName.c_str());
	strcat(TempPathBuf, "/logs/");
#endif
	ptr = TempPathBuf + strlen(TempPathBuf);
	sprintf(ptr, "log_of_stratagus_%d.log", ThisPlayer->Index);

	stat(TempPathBuf, &s);
	buf = new char[s.st_size];
	fd = fopen(TempPathBuf, "rb");
	fread(buf, s.st_size, 1, fd);
	fclose(fd);

	strncpy(ptr, menu->Items[1].D.Input.buffer, menu->Items[1].D.Input.nch);
	ptr[menu->Items[1].D.Input.nch] = '\0';
	if (!strcasestr(menu->Items[1].D.Input.buffer, ".log")) {
		strcat(ptr, ".log");
	}

	fd = fopen(TempPathBuf, "wb");
	if (!fd) {
		ErrorMenu("Cannot write to file");
		delete[] buf;
		return;
	}
	fwrite(buf, s.st_size, 1, fd);
	fclose(fd);

	delete[] buf;
	//CloseMenu();
	SelectedFileExist = 0;
	ScenSelectFileName[0] = '\0';
	ScenSelectPathName[0] = '\0';
}
#endif

#if 0
/**
**  Cancel button of network connect menu pressed.
*/
static void NetConnectingCancel(void)
{
	NetworkExitClientConnect();
	// Trigger TerminateNetConnect() to call us again and end the menu
	NetLocalState = ccs_usercanceled;
	//CloseMenu();
}
#endif

#if 0
/**
**  Call back from menu loop, if network state has changed.
*/
static void TerminateNetConnect(void)
{
	switch (NetLocalState) {
		case ccs_unreachable:
			NetErrorMenu("Cannot reach server.");
			NetConnectingCancel();
			return;
		case ccs_nofreeslots:
			NetErrorMenu("Server is full.");
			NetConnectingCancel();
			return;
		case ccs_serverquits:
			NetErrorMenu("Server gone.");
			NetConnectingCancel();
			return;
		case ccs_incompatibleengine:
			NetErrorMenu("Incompatible engine version.");
			NetConnectingCancel();
			return;
		case ccs_badmap:
			NetErrorMenu("Map not available.");
			NetConnectingCancel();
			return;
		case ccs_incompatiblenetwork:
			NetErrorMenu("Incompatible network version.");
			NetConnectingCancel();
			return;
		case ccs_usercanceled:
			NetConnectingCancel();
			return;
		case ccs_started:
			NetworkGamePrepareGameSettings();
			CustomGameStart();
			return;
		default:
			break;
	}

	DebugPrint("NetLocalState %d\n" _C_ NetLocalState);
	NetConnectRunning = 2;
	GuiGameStarted = 0;
	ProcessMenu("menu-net-multi-client", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	} else {
		NetConnectingCancel();
	}
}
#endif

#if 0
/**
** Menu setup fog-of-war pulldown action.
*/
static void MultiGameFWSAction(Menuitem *mi, int i)
{
	if (!mi || mi->D.Pulldown.curopt == i) {
		DebugPrint("Update fow %d\n" _C_ i);
		switch (i) {
			case 0:
				Map.NoFogOfWar = false;
				FlagRevealMap = 0;
				GameSettings.NoFogOfWar = false;
				GameSettings.RevealMap = 0;
				break;
			case 1:
				Map.NoFogOfWar = true;
				FlagRevealMap = 0;
				GameSettings.NoFogOfWar = true;
				GameSettings.RevealMap = 0;
				break;
			case 2:
				Map.NoFogOfWar = false;
				FlagRevealMap = 1;
				GameSettings.NoFogOfWar = false;
				GameSettings.RevealMap = 1;
				break;
			case 3:
				Map.NoFogOfWar = true;
				FlagRevealMap = 1;
				GameSettings.NoFogOfWar = true;
				GameSettings.RevealMap = 1;
				break;
		}
		ServerSetupState.FogOfWar = i;
		if (mi) {
			NetworkServerResyncClients();
		}
	}
}
#endif

#if 0
/**
**  Multiplayer server menu init callback
*/
static void MultiGameSetupInit(Menu *menu)
{
	int i;
	int h;

	// FIXME: Remove this when .cm is supported
	if (*CurrentMapPath && strstr(CurrentMapPath, ".cm\0")) {
		*CurrentMapPath = '\0';
	}

	GameSetupInit(menu);
	NetworkInitServerConnect();
	menu->Items[SERVER_PLAYER_STATE].Flags |= MI_FLAGS_INVISIBLE;
	MultiGameFWSAction(NULL, menu->Items[27].D.Pulldown.defopt);

	memset(&ServerSetupState, 0, sizeof(ServerSetup));
	// Calculate available slots from map info
	for (h = i = 0; i < PlayerMax; i++) {
		if (Map.Info.PlayerType[i] == PlayerPerson) {
			++h; // available interactive player slots
		}
	}
	for (i = h; i < PlayerMax - 1; ++i) {
		ServerSetupState.CompOpt[i] = 1;
	}
	MultiGamePlayerSelectorsUpdate(1);

	if (MetaServerInUse) {
		ChangeGameServer();
	}

}
#endif

#if 0
/**
**  Cancel button of server multi player menu pressed.
*/
static void MultiGameCancel(void)
{
	NetworkExitServerConnect();

	if (MetaServerInUse) {
		SendMetaCommand("AbandonGame", "");
	}

	FreeMapInfo(&Map.Info);

	NetPlayers = 0; // Make single player menus work again!
	GameCancel();
}
#endif

#if 0
/**
**  Cancel button of multiplayer client menu pressed.
*/
static void MultiClientCancel(void)
{
	NetworkDetachFromServer();
	FreeMapInfo(&Map.Info);
	// GameCancel();
}
#endif

/**
** Callback from netconnect loop in Client-Sync state:
** Compare local state with server's information
** and force update when changes have occured.
*/
void NetClientCheckLocalState(void)
{
	if (LocalSetupState.Ready[NetLocalHostsSlot] != ServerSetupState.Ready[NetLocalHostsSlot]) {
		NetLocalState = ccs_changed;
		return;
	}
	if (LocalSetupState.Race[NetLocalHostsSlot] != ServerSetupState.Race[NetLocalHostsSlot]) {
		NetLocalState = ccs_changed;
		return;
	}
	/* ADD HERE */
}

/**
** FIXME: docu
*/
int NetClientSelectScenario(void)
{
	FreeMapInfo(&Map.Info);
	LoadStratagusMapInfo(MenuMapFullPath);
	return 0;
}

/**
** FIXME: docu
*/
void NetConnectForceDisplayUpdate(void)
{
//	MultiGamePlayerSelectorsUpdate(2);
}

#if 0
/**
** Setup Editor Paths
*/
void SetupEditor(void)
{
	char *s;
	//
	//  Create a default path + map.
	//
	if (!*CurrentMapPath || *CurrentMapPath == '.' || *CurrentMapPath == '/') {
		strcpy(CurrentMapPath, DefaultMap);
	}

	//
	// Use the last path.
	//
	strcpy(ScenSelectPath, StratagusLibPath.c_str());
	if (*ScenSelectPath) {
		strcat(ScenSelectPath, "/");
	}
	strcat(ScenSelectPath, CurrentMapPath);
	if ((s = strrchr(ScenSelectPath, '/'))) {
		strcpy(ScenSelectFileName, s + 1);
		*s = '\0';
	}
	strcpy(ScenSelectDisplayPath, CurrentMapPath);
	if ((s = strrchr(ScenSelectDisplayPath, '/'))) {
		*s = '\0';
	} else {
		*ScenSelectDisplayPath = '\0';
	}

}
#endif


#if 0
/**
** Editor select cancel button callback
*/
static void EditorSelectCancel(void)
{
	//QuitToMenu = 1;
	Editor.Running = EditorNotRunning;
	//CloseMenu();
}
#endif

#if 0
/**
** Called from menu, for new editor map.
*/
static void EditorNewMap(void)
{
	Menu *menu;
	char width[10];
	char height[10];
	char description[36];

	EditorCancelled = 0;

	menu = FindMenu("menu-editor-new");
	menu->Items[2].D.Input.buffer = description;
	strcpy(description, "~!_");
	menu->Items[2].D.Input.nch = strlen(description) - 3;
	menu->Items[2].D.Input.maxch = 31;
	menu->Items[4].D.Input.buffer = width;
	strcpy(width, "128~!_");
	menu->Items[4].D.Input.nch = strlen(width) - 3;
	menu->Items[4].D.Input.maxch = 4;
	menu->Items[5].D.Input.buffer = height;
	strcpy(height, "128~!_");
	menu->Items[5].D.Input.nch = strlen(width) - 3;
	menu->Items[5].D.Input.maxch = 4;
	ProcessMenu("menu-editor-new", 1);

	if (EditorCancelled) {
		return;
	}

	description[strlen(description) - 3] = '\0';
	Map.Info.Description = description;
	Map.Info.MapWidth = atoi(width);
	Map.Info.MapHeight = atoi(height);

	Video.ClearScreen();

	*CurrentMapPath = '\0';

	GuiGameStarted = 1;
	//CloseMenu();
}
#endif

#if 0
/**
** Editor new map, size input box callback
*/
static void EditorNewMapSizeEnterAction(Menuitem * mi, int key)
{
	if (mi->D.Input.nch > 0
			&& !isdigit(mi->D.Input.buffer[mi->D.Input.nch - 1])) {
		strcpy(mi->D.Input.buffer + (--mi->D.Input.nch), "~!_");
	}
}
#endif

#if 0
/**
** Editor new map ok button
*/
static void EditorNewOk(void)
{
	Menu *menu;
	unsigned value1;
	unsigned value2;

	menu = CurrentMenu;
	value1 = atoi(menu->Items[4].D.Input.buffer);
	value2 = atoi(menu->Items[5].D.Input.buffer);

	if (value1 < 32 || value2 < 32) {
		if (value1 < 32) {
			sprintf(menu->Items[4].D.Input.buffer, "32~!_");
			menu->Items[4].D.Input.nch = strlen(menu->Items[4].D.Input.buffer) - 3;
		}
		if (value2 < 32) {
			sprintf(menu->Items[5].D.Input.buffer, "32~!_");
			menu->Items[5].D.Input.nch = strlen(menu->Items[5].D.Input.buffer) - 3;
		}
		ErrorMenu("Size smaller than 32");
	} else if (value1 > 1024 || value2 > 1024) {
		if (value1 == 0) {
			sprintf(menu->Items[4].D.Input.buffer, "1024~!_");
			menu->Items[4].D.Input.nch = strlen(menu->Items[4].D.Input.buffer) - 3;
		}
		if (value2 == 0) {
			sprintf(menu->Items[5].D.Input.buffer, "1024~!_");
			menu->Items[5].D.Input.nch = strlen(menu->Items[5].D.Input.buffer) - 3;
		}
		ErrorMenu("Size larger than 1024");
	} else if (value1 / 32 * 32 != value1 || value2/32*32 != value2) {
		if (value1 / 32 * 32 != value1) {
			sprintf(menu->Items[4].D.Input.buffer, "%d~!_", (value1 + 16) / 32 * 32);
			menu->Items[4].D.Input.nch = strlen(menu->Items[4].D.Input.buffer) - 3;
		}
		if (value2 / 32 * 32 != value2) {
			sprintf(menu->Items[5].D.Input.buffer, "%d~!_", (value2 + 16) / 32 * 32);
			menu->Items[5].D.Input.nch = strlen(menu->Items[5].D.Input.buffer) - 3;
		}
		ErrorMenu("Size must be a multiple of 32");
	}
	else {
		char tilemodel[PATH_MAX];

		sprintf(Map.TileModelsFileName, "scripts/tilesets/%s.lua",
				menu->Items[7].D.Pulldown.options[menu->Items[7].D.Pulldown.curopt]);
		sprintf(tilemodel, "%s/scripts/tilesets/%s.lua", StratagusLibPath.c_str(),
				menu->Items[7].D.Pulldown.options[menu->Items[7].D.Pulldown.curopt]);
		LuaLoadFile(tilemodel);
		//CloseMenu();
	}
}
#endif

#if 0
/**
** Editor main load map menu
*/
static void EditorMainLoadMap(void)
{
	char *p;
	char *s;

	EditorCancelled = 0;
	ProcessMenu("menu-editor-main-load-map", 1);
	GetInfoFromSelectPath();

	if (EditorCancelled) {
		return;
	}

	Video.ClearScreen();

	if (ScenSelectPath[0]) {
		s = ScenSelectPath + strlen(ScenSelectPath);
		*s = '/';
		strcpy(s+1, ScenSelectFileName); // Final map name with path
		p = ScenSelectPath + StratagusLibPath.size() + 1;
		strcpy(CurrentMapPath, p);
		*s = '\0';
	} else {
		strcpy(CurrentMapPath, ScenSelectFileName);
	}
	
	GuiGameStarted = 1;
	//CloseMenu();
}
#endif

#if 0
/**
** Editor main load ok button
*/
static void EditorMainLoadOk(void)
{
	Menu *menu;
	Menuitem *mi;

	menu = CurrentMenu;
	mi = &menu->Items[1];
	if (ScenSelectPathName[0]) {
		strcat(ScenSelectPath, "/");
		strcat(ScenSelectPath, ScenSelectPathName);
		if (ScenSelectDisplayPath[0]) {
			strcat(ScenSelectDisplayPath, "/");
		}
		strcat(ScenSelectDisplayPath, ScenSelectPathName);
		EditorMainLoadLBInit(mi);
	} else if (ScenSelectFileName[0]) {
		//CloseMenu();
	}
}
#endif

#if 0
/**
** Editor main load cancel button
*/
static void EditorMainLoadCancel(void)
{
	char *s;

	EditorCancelled=1;

	//
	//  Use last selected map.
	//
	DebugPrint("Map   path: %s\n" _C_ CurrentMapPath);
	strcpy(ScenSelectPath, StratagusLibPath.c_str());
	if (*ScenSelectPath) {
		strcat(ScenSelectPath, "/");
	}
	strcat(ScenSelectPath, CurrentMapPath);
	if ((s = strrchr(ScenSelectPath, '/'))) {
		strcpy(ScenSelectFileName, s + 1);
		*s = '\0';
	}
	strcpy(ScenSelectDisplayPath, CurrentMapPath);
	if ((s = strrchr(ScenSelectDisplayPath, '/'))) {
		*s = '\0';
	} else {
		*ScenSelectDisplayPath = '\0';
	}

	DebugPrint("Start path: %s\n" _C_ ScenSelectPath);

	//CloseMenu();
}
#endif

#if 0
/**
**  Editor load map menu
*/
void EditorLoadMenu(void)
{
	char *p;
	char *s;

	EditorCancelled = 0;
	ProcessMenu("menu-editor-load", 1);
	GetInfoFromSelectPath();

	if (EditorCancelled) {
		return;
	}

	Video.ClearScreen();

	if (ScenSelectPath[0]) {
		s = ScenSelectPath + strlen(ScenSelectPath);
		*s = '/';
		strcpy(s + 1, ScenSelectFileName); // Final map name with path
		p = ScenSelectPath + StratagusLibPath.size() + 1;
		strcpy(CurrentMapPath, p);
		*s = '\0';
	} else {
		strcpy(CurrentMapPath, ScenSelectFileName);
	}

	Editor.MapLoaded = true;
	Editor.Running = EditorNotRunning;
	//CloseMenu();
}
#endif

#if 0
/**
** Editor main load ok button
*/
static void EditorLoadOk(void)
{
	Menu *menu;
	Menuitem *mi;

	menu = CurrentMenu;
	mi = &menu->Items[1];
	if (ScenSelectPathName[0]) {
		strcat(ScenSelectPath, "/");
		strcat(ScenSelectPath, ScenSelectPathName);
		if (ScenSelectDisplayPath[0]) {
			strcat(ScenSelectDisplayPath, "/");
		}
		strcat(ScenSelectDisplayPath, ScenSelectPathName);
		EditorMainLoadLBInit(mi);
	} else if (ScenSelectFileName[0]) {
		//CloseMenu();
	}
}
#endif

#if 0
/**
** Editor main load cancel button
*/
static void EditorLoadCancel(void)
{
	char *s;

	EditorCancelled = 1;

	//
	//  Use last selected map.
	//
	DebugPrint("Map   path: %s\n" _C_ CurrentMapPath);
	strcpy(ScenSelectPath, StratagusLibPath.c_str());
	if (*ScenSelectPath) {
		strcat(ScenSelectPath, "/");
	}
	strcat(ScenSelectPath, CurrentMapPath);
	if ((s = strrchr(ScenSelectPath, '/'))) {
		strcpy(ScenSelectFileName, s + 1);
		*s = '\0';
	}
	strcpy(ScenSelectDisplayPath, CurrentMapPath);
	if ((s = strrchr(ScenSelectDisplayPath, '/'))) {
		*s = '\0';
	} else {
		*ScenSelectDisplayPath = '\0';
	}

	DebugPrint("Start path: %s\n" _C_ ScenSelectPath);

	EditorEndMenu();
}
#endif

#if 0
/**
** Editor map properties menu
*/
static void EditorMapPropertiesMenu(void)
{
	Menu *menu;
	char description[36];
	char size[30];

	menu = FindMenu("menu-editor-map-properties");

	menu->Items[2].D.Input.buffer = description;
	strcpy(description, Map.Info.Description.c_str());
	strcat(description, "~!_");
	menu->Items[2].D.Input.nch = strlen(description) - 3;
	menu->Items[2].D.Input.maxch = 31;

	sprintf(size, "%d x %d", Map.Info.MapWidth, Map.Info.MapHeight);
	menu->Items[4].D.Text.text = NewStringDesc(size);
	menu->Items[6].D.Pulldown.defopt = 0;

	// FIXME: Remove the version pulldown
	menu->Items[8].D.Pulldown.defopt = 1;
	menu->Items[8].Flags = static_cast<unsigned int>(-1);

	ProcessMenu("menu-editor-map-properties", 1);
	FreeStringDesc(menu->Items[4].D.Text.text);
	delete menu->Items[4].D.Text.text;
	menu->Items[4].D.Text.text = NULL;
}
#endif

#if 0
/**
** Editor map properties ok button
*/
static void EditorMapPropertiesOk(void)
{
	Menu *menu;
	char *description;

	menu = CurrentMenu;

	description = menu->Items[2].D.Input.buffer;
	description[strlen(description)-3] = '\0';
	Map.Info.Description = description;

	#if 0
	// MAPTODO
	// Change the terrain
	old = Map.Info.MapTerrain;
	if (old != menu->Items[6].D.Pulldown.curopt) {
		Map.Info.MapTerrain = menu->Items[6].D.Pulldown.curopt;
		delete[] Map.Info.MapTerrainName;
		Map.Info.MapTerrainName = new_strdup(TilesetWcNames[Map.Info.MapTerrain]);
		delete[] Map.TerrainName;
		Map.TerrainName = new_strdup(TilesetWcNames[Map.Info.MapTerrain]);
		Map.Tileset = Tilesets[Map.Info.MapTerrain];

		LoadTileset();
		SetPlayersPalette();
		PreprocessMap();
		LoadConstructions();
		LoadUnitTypes();
		LoadIcons();
		UpdateMinimapTerrain();
	}
	#endif


	EditorEndMenu();
}
#endif

#if 0
/**
** Editor player properties input box callback
*/
static void EditorPlayerPropertiesEnterAction(Menuitem *mi, int key)
{
	if (mi->D.Input.nch > 0 && !isdigit(mi->D.Input.buffer[mi->D.Input.nch - 1])) {
		strcpy(mi->D.Input.buffer + (--mi->D.Input.nch), "~!_");
	}
}
#endif

#if 0
	/// Player type conversion from internal fc to menu number
static int PlayerTypesFcToMenu[] = {
	0,
	0,
	4,
	5,
	1,
	0,
	2,
	3,
};
#endif

#if 0
	/// Player type conversion from menu to internal fc number
static int PlayerTypesMenuToFc[] = {
	PlayerPerson,
	PlayerComputer,
	PlayerRescuePassive,
	PlayerRescueActive,
	PlayerNeutral,
	PlayerNobody,
};
#endif

#if 0
/**
**  Convert player ai from internal fc number to menu number
**
**  @param ainame  Ai name
**  @param menu    Pulldown menu item
*/
static int PlayerSetAiToMenu(char *ainame, MenuitemPulldown *menu)
{
	int i;

	menu->defopt = 0;
	for (i = 0; i < menu->noptions; ++i) {
		if(!strcmp(menu->options[i], ainame)) {
			menu->defopt = i;
		}
	}
	DebugPrint("Invalid Ai number: %s\n" _C_ ainame);
	return i;
}
#endif

#if 0
/**
**  Get the ai ident from the pulldown menu
**
**  @param menu  Pulldown menu item
*/
static char *PlayerGetAiFromMenu(MenuitemPulldown *menu)
{
	return menu->options[menu->curopt];
}
#endif

#if 0
/**
** Edit player properties menu
*/
static void EditorPlayerPropertiesMenu(void)
{
	Menu *menu;
	char gold[PlayerMax][15];
	char lumber[PlayerMax][15];
	char oil[PlayerMax][15];
	int i;

	menu = FindMenu("menu-editor-player-properties");

#define RACE_POSITION 21
#define TYPE_POSITION 38
#define AI_POSITION 55
#define GOLD_POSITION 72
#define LUMBER_POSITION 89
#define OIL_POSITION 106

	for (i = 0; i < PlayerMax; ++i) {
		menu->Items[RACE_POSITION + i].D.Pulldown.defopt = Map.Info.PlayerSide[i];
		menu->Items[TYPE_POSITION + i].D.Pulldown.defopt = PlayerTypesFcToMenu[Map.Info.PlayerType[i]];
		PlayerSetAiToMenu(Players[i].AiName.c_str(), &menu->Items[AI_POSITION + i].D.Pulldown);
		sprintf(gold[i], "%d~!_", Players[i].Resources[GoldCost]);
		sprintf(lumber[i], "%d~!_", Players[i].Resources[WoodCost]);
		sprintf(oil[i], "%d~!_", Players[i].Resources[OilCost]);
		menu->Items[GOLD_POSITION + i].D.Input.buffer = gold[i];
		menu->Items[GOLD_POSITION + i].D.Input.nch = strlen(gold[i]) - 3;
		menu->Items[GOLD_POSITION + i].D.Input.maxch = 7;
		menu->Items[LUMBER_POSITION + i].D.Input.buffer = lumber[i];
		menu->Items[LUMBER_POSITION + i].D.Input.nch = strlen(lumber[i]) - 3;
		menu->Items[LUMBER_POSITION + i].D.Input.maxch = 7;
		menu->Items[OIL_POSITION + i].D.Input.buffer = oil[i];
		menu->Items[OIL_POSITION + i].D.Input.nch = strlen(oil[i]) - 3;
		menu->Items[OIL_POSITION + i].D.Input.maxch = 7;
	}

	ProcessMenu("menu-editor-player-properties", 1);

	for (i = 0; i < PlayerMax; ++i) {
		Map.Info.PlayerSide[i] = menu->Items[RACE_POSITION + i].D.Pulldown.curopt;
		Map.Info.PlayerType[i] = PlayerTypesMenuToFc[menu->Items[TYPE_POSITION + i].D.Pulldown.curopt];
		Players[i].AiName = PlayerGetAiFromMenu(&menu->Items[AI_POSITION + i].D.Pulldown));
		Players[i].Resources[GoldCost] = atoi(gold[i]);
		Players[i].Resources[WoodCost] = atoi(lumber[i]);
		Players[i].Resources[OilCost] = atoi(oil[i]);
	}
}
#endif

#if 0
/**
** Save map from the editor
**
** @return 0 for success, -1 for error
*/
int EditorSaveMenu(void)
{
	Menu *menu;
	char path[PATH_MAX];
	char *s;
	char *p;
	int ret;

	ret = 0;
	menu = FindMenu("menu-editor-save");

	EditorCancelled = 0;

	strcpy(path, "~!_");
	menu->Items[3].D.Input.buffer = path;
	menu->Items[3].D.Input.maxch = PATH_MAX - 4;

	ProcessMenu("menu-editor-save", 1);

	if (!EditorCancelled) {
		if (Editor.WriteCompressedMaps && !strstr(path, ".gz")) {
			sprintf(path, "%s/%s.gz", ScenSelectPath, ScenSelectFileName);
		} else {
			sprintf(path, "%s/%s", ScenSelectPath, ScenSelectFileName);
		}
		if (EditorSaveMap(path) == -1) {
			ret = -1;
		} else {
			// Only change map path if we were able to save the map
			s = ScenSelectPath + strlen(ScenSelectPath);
			*s = '/';
			strcpy(s + 1, ScenSelectFileName); // Final map name with path
			p = ScenSelectPath + StratagusLibPath.size() + 1;
			strcpy(CurrentMapPath, p);
			*s = '\0';
		}
	}
	return ret;
}
#endif

#if 0
/**
** Editor save ok button
*/
static void EditorSaveOk(void)
{
	Menu *menu;
	Menuitem *mi;
	int i;

	menu = CurrentMenu;
	mi = &menu->Items[1];
	i = mi->D.Listbox.curopt;
	if (i < mi->D.Listbox.noptions) {
		if (mi->Menu->Items[3].D.Input.nch == 0 && ScenSelectPathName[0]) {
			strcat(ScenSelectPath, "/");
			strcat(ScenSelectPath, ScenSelectPathName);
			if (ScenSelectDisplayPath[0]) {
				strcat(ScenSelectDisplayPath, "/");
			}
			strcat(ScenSelectDisplayPath, ScenSelectPathName);
			EditorSaveLBInit(mi);
		} else {
			strcpy(ScenSelectFileName, menu->Items[3].D.Input.buffer); // Final map name
			ScenSelectFileName[strlen(ScenSelectFileName) - 3] = '\0';
			if (!strcasestr(ScenSelectFileName, ".smp")) {
				strcat(ScenSelectFileName, ".smp");
			}
			sprintf(TempPathBuf, "%s/%s.gz", ScenSelectPath, ScenSelectFileName);
			if (!access(TempPathBuf, F_OK)) {
				ProcessMenu("menu-editor-save-confirm", 1);
				if (EditorCancelled) {
					EditorCancelled = 0;
					return;
				}
			}
			EditorEndMenu();
		}
	}
}
#endif

#if 0
/**
** Editor save input callback
*/
static void EditorSaveEnterAction(Menuitem *mi, int key)
{
	Assert(mi->MiType == MiTypeInput);

	strncpy(ScenSelectFileName, mi->D.Input.buffer, mi->D.Input.nch);
	ScenSelectFileName[mi->D.Input.nch] = '\0';
	ScenSelectPathName[0] = '\0';
	SelectedFileExist = 0;
	if (mi->D.Input.nch != 0) {
		if (key == 10 || key == 13) {
			EditorSaveOk();
		}
	}
}
#endif

#if 0
/**
** Called from menu, to quit editor to menu.
**
** @todo Should check if modified file should be saved.
*/
static void EditorQuitToMenu(void)
{
	//QuitToMenu = 1;
	Editor.Running = EditorNotRunning;
	//CloseMenu();
	SelectedFileExist = 0;
	ScenSelectFileName[0] = '\0';
	ScenSelectPathName[0] = '\0';
}
#endif

/*----------------------------------------------------------------------------
--  Metaserver
----------------------------------------------------------------------------*/

#if 0
/**
** Start process network game setup menu (server).
** Internet game, register with meta server
*/
static void CreateInternetGameMenu(void)
{
	GuiGameStarted = 0;
	AddGameServer();
	ProcessMenu("menu-multi-setup", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}

}
#endif

#if 0
/**
** Process Internet game menu
*/
static void MultiPlayerInternetGame(void)
{
	//Connect to Meta Server
	if (MetaInit() == -1 ) {
		MetaServerInUse = 0;
		MetaServerConnectError();
		return;
	}
	MetaServerInUse = 1;
	ProcessMenu("menu-internet-create-join-menu", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}
#endif

#if 0
/**
**  FIXME: docu
*/
static void MultiGameMasterReport(void)
{
// CloseMenu();

	ProcessMenu("metaserver-list", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}

}
#endif

#if 0
/**
**  Menu for Mater Server Game list.
*/
static void ShowMetaServerList(void)
{
	//CloseMenu();

	GuiGameStarted = 0;
	ProcessMenu("metaserver-list", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}
#endif

#if 0
/**
**  Multiplayer server menu init callback
**
**  Mohydine: Right now, because I find it simpler, the client is sending
**            n commands, one for each online game.
**  @todo: well, redo this :)
*/
static void MultiMetaServerGameSetupInit(Menu *menu)
{
	int i;
	int j;
	int k;
	int numparams;
	int nummenus;
	char *parameter;
	char *reply;
	char *port;

	SendMetaCommand("NumberOfGames", "");

	reply = NULL;
	// receive
	// check okay
	if (RecvMetaReply(&reply) == -1) {
		//TODO: Notify player that connection was aborted...
		nummenus = 1;
	} else {
		for (i = 0; i < 3; ++i) {
			GetMetaParameter(reply, 0, &parameter);
			nummenus = atoi(parameter);
			delete[] parameter;
			if (nummenus == 0) {
				RecvMetaReply(&reply);
			}
			else {
				break;
			}
		}

	}

	--nummenus;
	// Meta server only sends matching version
	// Only Displays games from Matching version

	i = 1;
	k = 0;
	numparams = 5; // TODO: To be changed if more params are sent

	// Retrieve list of online game from the meta server
	for (j = 4; j <= nummenus * (numparams + 1); j += numparams + 1) { // loop over the number of items in the menu
		// TODO: hard coded.
		// Check if connection to meta server is there.

		SendMetaCommand("GameNumber","%d\n",k + 1);
		i = RecvMetaReply(&reply);
		if (i == 0) {
			// fill the menus with the right info.
			menu->Items[j].D.Text.text = NULL;
			menu->Items[j + 1].D.Text.text = NULL;
			menu->Items[j + 2].D.Text.text = NULL;
			menu->Items[j + 3].D.Text.text = NULL;
			menu->Items[j + 4].D.Text.text = NULL;
			menu->Items[j + 5].Flags = MI_FLAGS_INVISIBLE;
		} else {
			GetMetaParameter(reply, 0, &parameter);       // Player Name
			menu->Items[j].D.Text.text = NewStringDesc(parameter);
			delete[] parameter;
			GetMetaParameter(reply, 3, &parameter);       // IP
			GetMetaParameter(reply, 4, &port);            // port
			sprintf(parameter, "%s:%s", parameter, port); // IP:Port
			menu->Items[j + 1].D.Text.text = NewStringDesc(parameter);
			delete[] parameter;
			delete[] port;
			GetMetaParameter(reply, 6, &parameter);
			menu->Items[j + 2].D.Text.text = NewStringDesc(parameter);
			delete[] parameter;
			GetMetaParameter(reply, 7, &parameter);
			menu->Items[j + 3].D.Text.text = NewStringDesc(parameter);
			delete[] parameter;
			GetMetaParameter(reply, 8, &parameter);
			menu->Items[j + 4].D.Text.text = NewStringDesc(parameter);
			menu->Items[j + 5].D.Checkbox.Checked = 0;
			delete[] parameter;
		}
		++k;
	}

	// Don't display slots not in use
	// FIXME: HardCoded Number of Items in list
	// 5 is the hardcoded value
	for (; j <= numparams * 5; j += numparams + 1) {
		// fill the menus with the right info.
		menu->Items[j].D.Text.text = NULL;
		menu->Items[j + 1].D.Text.text = NULL;
		menu->Items[j + 2].D.Text.text = NULL;
		menu->Items[j + 3].D.Text.text = NULL;
		menu->Items[j + 4].D.Text.text = NULL;
		menu->Items[j + 5].Flags = MI_FLAGS_DISABLED;
	}
}
#endif

#if 0
/**
**  Multiplayer server menu exit callback
*/
static void MultiMetaServerGameSetupExit(Menu *menu)
{
	int i;
	int j;
	int numparam;
	int nummenu;

	numparam = 5;
	nummenu = 6;
	for (j = 4; j <= numparam * nummenu; ++j) {
		for (i = 0; i < numparam; ++i) {
			FreeStringDesc(menu->Items[i + j].D.Text.text);
			delete menu->Items[i + j].D.Text.text;
			menu->Items[i + j].D.Text.text = NULL;
		}
	}
// CloseMenu();
// CloseMenu();
}
#endif

#if 0
/**
**  Action taken when a player select an online game
*/
static void SelectGameServer(Menuitem *mi)
{
	char server_host_buffer[64];
	char *port;
	int j;
	char *tmp;

	j = mi - mi->Menu->Items;
	mi->Menu->Items[j].D.Checkbox.Checked = 0;
	//CloseMenu();

	tmp = EvalString(mi->Menu->Items[j - 4].D.Text.text);
	strcpy(server_host_buffer, tmp);
	delete[] tmp;

	// Launch join directly
	if ((port = strchr(server_host_buffer, ':')) != NULL) {
		NetworkPort = atoi(port + 1);
		port[0] = 0;
	}

	// Now finally here is the address
// server_host_buffer[menu->Items[1].D.Input.nch] = 0;
	if (NetworkSetupServerAddress(server_host_buffer)) {
		NetErrorMenu("Unable to lookup host.");
		ProcessMenu("metaserver-list", 1);
		return;
	}
	NetworkInitClientConnect();
	if (!NetConnectRunning) {
		TerminateNetConnect();
		return;
	}

	NetworkArg = server_host_buffer;

	// Here we really go...
	ProcessMenu("menu-net-connecting", 1);

	if (GuiGameStarted) {
		//CloseMenu();
	}
}
#endif

#if 0
/**
**  Action to add a game server on the meta-server.
*/
static void AddGameServer(void)
{
	// send message to meta server. meta server will detect IP address.
	// Meta-server will return "BUSY" if the list of online games is busy.

	SendMetaCommand("AddGame", "%s\n%d\n%s\n%s\n%s\n%s\n",
		"IP", NetworkPort, "Name", "Map", "Players", "Free");

	// FIXME: Get Reply from Queue
}
#endif

#if 0
/**
**  Action to add a game server on the meta-server.
*/
static void ChangeGameServer(void)
{
	int i;
	int freespots;
	int players;

	// send message to meta server. meta server will detect IP address.
	// Meta-server will return "ERR" if the list of online games is busy.

	freespots = 0;
	players = 0;
	for (i = 0; i < PlayerMax - 1; ++i) {
		if (Map.Info.PlayerType[i] == PlayerPerson) {
			++players;
		}
		if (ServerSetupState.CompOpt[i] == 0) {
			++freespots;
		}
	}
	SendMetaCommand("ChangeGame", "%s\n%s\n%d\n%d\n",
		"Name", ScenSelectFileName, players, freespots - 1);

	// FIXME: Get Reply from Queue
}
#endif

#if 0
/**
**  FIXME: docu
*/
static int MetaServerConnectError(void)
{
	Invalidate();
	NetErrorMenu("Cannot Connect to Meta-Server");
	return 0;
}
#endif

#if 0
/**
**  Close MetaServer connection
*/
static void MultiMetaServerClose(void)
{
	MetaClose();
	MetaServerInUse = 0;
	//CloseMenu();
}
#endif

//@}
