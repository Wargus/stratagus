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
/**@name menus.c - The menu function code. */
//
//      (c) Copyright 1999-2005 by Andreas Arens, Jimmy Salmon, Nehal Mistry
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
-- Includes
----------------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#ifndef _MSC_VER
#include <fcntl.h>
#endif

#include "stratagus.h"

#include "SDL.h"

#include "iocompat.h"

#include "unittype.h"
#include "video.h"
#include "player.h"
#include "font.h"
#include "tileset.h"
#include "map.h"
#include "minimap.h"
#include "interface.h"
#include "menus.h"
#include "cursor.h"
#include "pud.h"
#include "iolib.h"
#include "network.h"
#include "netconnect.h"
#include "settings.h"
#include "ui.h"
#include "campaign.h"
#include "sound_server.h"
#include "sound.h"
#include "script.h"
#include "editor.h"
#include "commands.h"
#include "actions.h"
#include "cdaudio.h"
#include "net_lowlevel.h"
#include "master.h"
#include "construct.h"


/*----------------------------------------------------------------------------
-- Prototypes for local functions
----------------------------------------------------------------------------*/

static void EditorEndMenu(void);

/*----------------------------------------------------------------------------
-- Prototypes for action handlers and helper functions
----------------------------------------------------------------------------*/

// Game menu
static void GameMenuInit(Menu* menu);
static void GameOptionsMenu(void);
static void HelpMenu(void);
static void ObjectivesMenu(void);
static void EndScenarioMenu(void);

// Victory, lost
static void GameMenuEnd(void);
static void VictoryInit(Menu* menu);
static void DefeatedInit(Menu* menu);
static void SaveReplay(void);
static void SaveReplayEnterAction(Menuitem* mi, int key);
static void SaveReplayOk(void);

// Scenario select
static void ScenSelectInit(Menu* menu);
static void ScenSelectExit(Menu* menu);
static void ScenSelectTPMSAction(Menuitem* mi, int i);
static void ScenSelectVSAction(Menuitem* mi);
static void ScenSelectOk(void);
static void ScenSelectCancel(void);
static int ScenSelectRDFilter(char* pathbuf, FileList* fl);

// Program start
static void PrgStartInit(Menu* menu);
static void NameLineDrawFunc(Menuitem* mi);
static void SinglePlayerGameMenu(void);
static void MultiPlayerGameMenu(void);
static void CampaignGameMenu(void);
static void ReplayGameMenu(void);
static void GlobalOptionsMenu(void);
static void StartEditor(void);
static void GameShowCredits(void);
static void GameMenuExit(void);

// Confirm menus
static void SurrenderConfirmMenu(void);

// Global Options
static void GlobalOptionsInit(Menu* menu);
static void GlobalOptionsExit(Menu* menu);
static void GlobalOptionsResolutionCheckbox(Menuitem* mi);
static void GlobalOptionsFullscreenCheckbox(Menuitem* mi);

// Tips
static void TipsInit(Menu* menu);
static void TipsExit(Menu* menu);
static void TipsShowTipsCheckbox(Menuitem* mi);
static void TipsShowTipsText(Menuitem* mi);
static void TipsNextTip(void);
static void TipsPreviousTip(void);

// Custom game setup
static void GameSetupInit(Menu* menu);
static void ScenSelectMenu(void);
static void CustomGameStart(void);
static void GameCancel(void);
static void GameDrawFunc(Menuitem* mi);
static void GameRCSAction(Menuitem* mi, int i);
static void GameRESAction(Menuitem* mi, int i);
static void GameUNSAction(Menuitem* mi, int i);
static void GameTSSAction(Menuitem* mi, int i);
static void GameGATAction(Menuitem* mi, int i);
static void CustomGameOPSAction(Menuitem* mi, int i);

// Enter name
static void EnterNameAction(Menuitem* mi, int key);
static void EnterNameCancel(void);

// Net create join
static void JoinNetGameMenu(void);
static void CreateNetGameMenu(void);
static void CreateInternetGameMenu(void);

// Multi net type
static void MultiPlayerLANGame(void);
static void MultiPlayerInternetGame(void);

// Net multi setup
static void MultiGameSetupInit(Menu* menu);
static void MultiGameSetupExit(Menu* menu);
static void MultiGameDrawFunc(Menuitem* mi);
static void MultiScenSelectMenu(void);
static void MultiGameStart(void);
static void MultiGameCancel(void);
static void MultiGameFWSAction(Menuitem* mi, int i);

// Enter server ip
static void EnterServerIPAction(Menuitem* mi, int key);
static void EnterServerIPCancel(void);

// Net multi client
static void TerminateNetConnect(void);
static void MultiGameClientInit(Menu* menu);
static void MultiGameClientExit(Menu* menu);
static void MultiGameClientDrawFunc(Menuitem* mi);
static void MultiClientReady(void);
static void MultiClientNotReady(void);
static void MultiClientCancel(void);
static void MultiClientRCSAction(Menuitem* mi, int i);
static void MultiClientCheckboxAction(Menuitem* mi);
static void MultiClientUpdate(int initial);

// Net connecting
static void NetConnectingInit(Menu* menu);
static void NetConnectingExit(Menu* menu);
static void NetConnectingCancel(void);

// Campaign select
static void CampaignMenu1(void);
static void CampaignMenu2(void);
static void CampaignMenu3(void);
static void CampaignMenu4(void);
static void SelectCampaignMenu(void);

// End scenario
static void EndScenarioRestart(void);
static void EndScenarioSurrender(void);
static void EndScenarioQuitMenu(void);

// Sound options
static void SoundOptionsInit(Menu* menu);
static void SoundOptionsExit(Menu* menu);
static void MasterVolumeHSAction(Menuitem* mi);
static void SetMasterPower(Menuitem* mi);
static void MusicVolumeHSAction(Menuitem* mi);
static void SetMusicPower(Menuitem* mi);
static void CdVolumeHSAction(Menuitem* mi);
static void SetCdPower(Menuitem* mi);
static void SetCdModeDefined(Menuitem* mi);
static void SetCdModeRandom(Menuitem* mi);

// Preferences
static void PreferencesInit(Menu* menu);
static void PreferencesExit(Menu* menu);
static void SetFogOfWar(Menuitem* mi);
static void SetCommandKey(Menuitem* mi);

// Speed options
static void SpeedOptionsInit(Menu* menu);
static void SpeedOptionsExit(Menu* menu);
static void GameSpeedHSAction(Menuitem* mi);
static void MouseScrollHSAction(Menuitem* mi);
static void KeyboardScrollHSAction(Menuitem* mi);

// Game options

// Diplomacy options
static void DiplomacyInit(Menu* menu);
static void DiplomacyExit(Menu* menu);
static void DiplomacyWait(Menuitem* mi);
static void DiplomacyOk(void);

// Help
static void KeystrokeHelpMenu(void);
static void TipsMenu(void);

// Keystroke help
static void KeystrokeHelpVSAction(Menuitem* mi);
static void KeystrokeHelpDrawFunc(Menuitem* mi);

// Save
static void SaveGameInit(Menu* menu);
static void SaveGameExit(Menu* menu);
static void SaveGameEnterAction(Menuitem* mi, int key);
static void SaveGameVSAction(Menuitem* mi);
static void SaveGameOk(void);
static void DeleteConfirmMenu(void);
static int SaveGameRDFilter(char *pathbuf, FileList *fl);
static void CreateSaveDir(void);

// Load
static void LoadGameInit(Menu* menu);
static void LoadGameExit(Menu* menu);
static void LoadGameVSAction(Menuitem* mi);
static void LoadGameOk(void);

// Confirm save
static void SaveConfirmInit(Menu* menu);
static void SaveConfirmExit(Menu* menu);
static void SaveConfirmOk(void);
static void SaveConfirmCancel(void);

// Confirm delete
static void DeleteConfirmInit(Menu* menu);
static void DeleteConfirmExit(Menu* menu);
static void DeleteConfirmOk(void);
static void DeleteConfirmCancel(void);

// Save replay
static void SaveReplayInit(Menu* menu);
static void SaveReplayExit(Menu* menu);

// Editor select
static void EditorNewMap(void);
static void EditorMainLoadMap(void);
static void EditorSelectCancel(void);

// Editor new
static void EditorNewDrawFunc(Menuitem* mi);
static void EditorNewMapDescriptionEnterAction(Menuitem* mi, int key);
static void EditorNewMapSizeEnterAction(Menuitem* mi, int key);
static void EditorNewOk(void);
static void EditorNewCancel(void);

// Editor main load map
static void EditorMainLoadInit(Menu* menu);
static void EditorMainLoadExit(Menu* menu);
static void EditorMainLoadVSAction(Menuitem* mi);
static void EditorMainLoadOk(void);
static void EditorMainLoadCancel(void);
static int EditorMainLoadRDFilter(char *pathbuf, FileList *fl);

// Editor load map
static void EditorLoadOk(void);
static void EditorLoadCancel(void);

// Editor menu
static void EditorMapPropertiesMenu(void);
static void EditorPlayerPropertiesMenu(void);
static void EditorQuitToMenu(void);

// Editor map properties
static void EditorMapPropertiesEnterAction(Menuitem* mi, int key);
static void EditorMapPropertiesOk(void);

// Editor player properties
static void EditorPlayerPropertiesDrawFunc(Menuitem* mi);
static void EditorPlayerPropertiesEnterAction(Menuitem* mi, int key);

// Editor edit resource
static void EditorEditResourceEnterAction(Menuitem* mi,int key);
static void EditorEditResourceOk(void);
static void EditorEditResourceCancel(void);

// Editor edit ai properties
static void EditorEditAiPropertiesCheckbox(Menuitem* mi);
static void EditorEditAiPropertiesOk(void);
static void EditorEditAiPropertiesCancel(void);

// Editor save
static void EditorSaveInit(Menu* menu);
static void EditorSaveExit(Menu* menu);
static void EditorSaveVSAction(Menuitem* mi);
static void EditorSaveEnterAction(Menuitem* mi, int key);
static void EditorSaveOk(void);
static void EditorSaveCancel(void);
static int EditorSaveRDFilter(char *pathbuf, FileList *fl);

// Editor save confirm
static void EditorSaveConfirmInit(Menu* menu);
static void EditorSaveConfirmOk(void);
static void EditorSaveConfirmCancel(void);

// Replay game
static void ReplayGameInit(Menu* menu);
static void ReplayGameExit(Menu* menu);
static void ReplayGameVSAction(Menuitem* mi);
static void ReplayGameDisableFog(Menuitem* mi);
static void ReplayGameOk(void);
static void ReplayGameCancel(void);
static int ReplayGameRDFilter(char *pathbuf, FileList *fl);

// Metaserver
//static void MultiGameMasterReport(void);
//static void EnterMasterAction(Menuitem* mi, int key);
static void ShowMetaServerList(void); // Addition for Magnant
static void MultiMetaServerGameSetupInit(Menu* menu); // init callback
static void MultiMetaServerGameSetupExit(Menu* menu); // exit callback
static void SelectGameServer(Menuitem* mi); // Game Server selection so that client joins the game
static void AddGameServer(void); //Add Game Server on Meta server
static void ChangeGameServer(void); //Change Game Parameters on Meta server
static int MetaServerConnectError(void); // Display error message
static void MultiMetaServerClose(void); //Close Master Server connection
//others
static void GameMenuReturn(void);
static void NetErrorMenu(char *error);
static void NetworkGamePrepareGameSettings(void);
static void MultiGamePTSAction(Menuitem* mi, int o);
static void NetMultiPlayerDrawFunc(Menuitem* mi);
static void MultiGamePlayerSelectorsUpdate(int initial);


/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

	/// Hash table of all the menus
_MenuHash MenuHash;
	/// Hash table of all the menu functions
_MenuFuncHash MenuFuncHash;

#define HASHADD(x,y) { \
	*(void **)hash_add(MenuFuncHash,(y)) = (void *)(x); \
}

	/// Game started
int GuiGameStarted;
	/// Editor cancel button pressed
static int EditorCancelled;

/**
** Other client and server selection state for Multiplayer clients
*/
ServerSetup ServerSetupState;
ServerSetup LocalSetupState;

static char ScenSelectPath[1024];        /// Scenario selector path
static char ScenSelectDisplayPath[1024]; /// Displayed selector path
static char ScenSelectFileName[128];     /// Scenario selector name
static char ScenSelectPathName[128];     /// Path name.

static char SelectedFileExist;           /// True if ScenSelectFileName is an existing file.

char MenuMapFullPath[1024];              /// Selected map path+name

static char *SaveDir;                    /// Save game directory
static char TempPathBuf[PATH_MAX];       /// Temporary buffer for paths

int nKeyStrokeHelps;                     /// Number of keystroke help lines
char **KeyStrokeHelps;                   /// Array of keystroke help lines

/// FIXME: -> ccl...
static unsigned char *mgptsoptions[] = {
	"Available",
	"Computer",
	"Closed",
};

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

////////////////////
// Map info stuff //
////////////////////

/**
**  Allocate and deep copy a MapInfo structure
**
**  @param orig  source of the copy.
**
**  @return copy of orig.
*/
static MapInfo* DuplicateMapInfo(const MapInfo *orig)
{
	MapInfo* dest;

	Assert(orig);

	dest = malloc(sizeof(MapInfo));
	memcpy(dest, orig, sizeof(MapInfo));
	if (orig->Description) {
		dest->Description = strdup(orig->Description);
	}
	if (orig->MapTerrainName) {
		dest->MapTerrainName = strdup(orig->MapTerrainName);
	}
	if (orig->Filename) {
		dest->Filename = strdup(orig->Filename);
	}

	return dest;
}

/**
**  Free map info data
**
**  @param fl  File list to free.
**  @param n   Size of file list.
*/
static void FreeMapInfos(FileList *fl, int n)
{
	int i;

	Assert(fl);
	Assert(0 <= n);

	for (i = 0; i < n; i++) {
		if (fl[i].xdata) {
			FreeMapInfo(fl[i].xdata);
			free(fl[i].xdata);
		}
		free(fl[i].name);
		memset(fl, 0, sizeof(*fl));
	}
}

//////////////
//  Filter  //
//////////////

/**
** Editor main load read directory filter
**
**  @param pathbuf    Pathname and filename of current file (or directory) to check.
**  @param fl         OUT : Filelist to update.
**  @param suf        suffixe allowed.
**  @param width      Restiction size map.
**  @param height     Restiction size map.
**
**  @return 1 if it is a valid file, else 0.
**
**  @note suffixe supported : ".pud", ".scm", ".chk", ".log", ".sav", ".smp"
**  @todo remove pud support.
*/
static int GenericRDFilter(char *pathbuf, FileList *fl, const char *suf[], int width, int height)
{
	unsigned int i;
	char type;      // type of compression =>  'b':bzip2 'z':gzip 'n':none
	char *filename; // filename
	char *cp;

	Assert(pathbuf && *pathbuf);
	Assert(fl);

	memset(fl, 0, sizeof (*fl));
	filename = strrchr(pathbuf, '/');
	if (filename) {
		filename++;
	} else {
		filename = pathbuf;
	}
	cp = filename;
	for (i = 0; suf[i]; i++) {
		char *lcp;

		lcp = NULL;
		cp = filename;
		while (cp != NULL) {
			lcp = cp;
			cp = strcasestr(cp + 1, suf[i]);
		}
		if (lcp >= filename) {
			cp = lcp + strlen(suf[i]);
			break;
		}
	}
	if (!suf[i]) {
		return 0;
	}

	type = 'n';
#ifdef USE_ZLIB
	if (strcmp(cp, ".gz") == 0) {
		*cp = 0;
		type = 'z';
	}
#endif
#ifdef USE_BZ2LIB
	if (strcmp(cp, ".bz2") == 0) {
		*cp = 0;
		type = 'b';
	}
#endif
	if (*cp != '\0') {
		return 0;
	}

	if (strcasestr(filename, ".scm") || strcasestr(filename, ".chk") ||
			strcasestr(filename, ".smp")) {
		MapInfo* info;

		info = DuplicateMapInfo(&TheMap.Info);
		FreeMapInfo(&TheMap.Info);
		LuaLoadFile(pathbuf);

		if ((width != -1 && TheMap.Info.MapWidth != width) ||
			(height != -1 && TheMap.Info.MapHeight != height)) {
			FreeMapInfo(&TheMap.Info);
			// Restore TheMap.Info
			// We don't strdup string attributs of info. (like DuplicateMapInfo dooe)
			// because we don't free the these in original.
			TheMap.Info = *info;
			free(info);
			return 0;
		}
		fl->type = 1;
		fl->name = strdup(filename);
		fl->xdata = DuplicateMapInfo(&TheMap.Info);
		// Restore TheMap.Info
		// We don't strdup string attributs of info. (like DuplicateMapInfo dooe)
		// because we don't free the these in original.
		TheMap.Info = *info;
		free(info);
	} else if (strcasestr(filename, ".log")) {
		fl->type = 1;
		fl->name = strdup(filename);
	} else if (strcasestr(filename, ".sav")) {
		fl->type = type;
		fl->name = strdup(filename);
#if 1 // FIXME : remove it when remove pud format.
	} else if (strcasestr(filename, ".pud")) {
		MapInfo info;

		if (!GetPudInfo(pathbuf, &info)) {
			return 0;
		}
		if ((width != -1 && info.MapWidth != width) ||
			(height != -1 && info.MapHeight != height)) {
			FreeMapInfo(&info);
			return 0;
		}
		fl->type = 1;
		fl->name = strdup(filename);
		fl->xdata = malloc(sizeof(*fl->xdata));
		*fl->xdata = info;
#endif
	} else {
		DebugPrint("file '%s' unsupported with this extension\n" _C_ filename);
		return 0;
	}
	return 1;
}

/**
** Replay game read directory filter
*/
static int ReplayGameRDFilter(char *pathbuf, FileList *fl)
{
	const char* suf[] = {".log", 0};

	return GenericRDFilter(pathbuf, fl, suf, -1, -1);
}

/**
** Save game read directory filter
*/
static int SaveGameRDFilter(char *pathbuf, FileList *fl)
{
	const char* suf[] = {".sav", 0};

	return GenericRDFilter(pathbuf, fl, suf, -1, -1);
}

/**
** Editor main load read directory filter
*/
static int EditorMainLoadRDFilter(char *pathbuf, FileList *fl)
{
	const char* suf[] = {".pud", ".scm", ".chk", 0};

	return GenericRDFilter(pathbuf, fl, suf, -1, -1);
}

/**
** Editor save read directory filter
*/
static int EditorSaveRDFilter(char *pathbuf, FileList *fl)
{
	const char* suf[] = {".pud", 0};

	return GenericRDFilter(pathbuf, fl, suf, -1, -1);
}

/**
** Scenario select read directory filter
*/
static int ScenSelectRDFilter(char *pathbuf, FileList *fl)
{
	const char *suf[3];
	static int szl[] = {-1, 32, 64, 96, 128, 256, 512, 1024};
	int sz;
	Menu* menu;
	int curopt;

	suf[0] = ".smp";
	suf[1] = NULL;
	suf[2] = NULL;

	menu = FindMenu("menu-select-scenario");
	sz = szl[menu->Items[8].D.Pulldown.curopt];

#if 1 // Remove hardcoded value.
	curopt = menu->Items[6].D.Pulldown.curopt;
	if (curopt == 0) {
		suf[0] = ".smp";
	} else if (curopt == 1) {
		suf[0] = ".pud";
	} else {
		suf[0] = ".scm";
		suf[1] = ".chk";
	}
#endif
	return GenericRDFilter(pathbuf, fl, suf, sz, sz);
}

////////////////
// LBRetrieve //
////////////////

/**
**  Listbox retrieve callback.
**
**  @param mi    Listbox.
**  @param i     Line to retrieve.
**
**  @return string to display in listbox.
*/
static unsigned char* LBRetrieve(const Menuitem* mi, int i)
{
	FileList* fl;
	static char buffer[1024];

	Assert(mi->MiType == MiTypeListbox);
	Assert(i >= 0);

	if (i >= mi->D.Listbox.noptions) {
		return NULL;
	}
	fl = mi->D.Listbox.options;
	if (fl[i].type) {
		strcpy(buffer, "   ");
	} else {
		strcpy(buffer, "\260 ");
	}
	strcat(buffer, fl[i].name);
	return buffer;
}

/**
**  Listbox retrieve callback and draw some info.
**
**  @param mi    Listbox.
**  @param i     Line to retrieve.
**
**  @return string to display in listbox.
*/
static unsigned char* LBRetrieveAndInfo(const Menuitem* mi, int i)
{
	FileList* fl;
	MapInfo* info;

	Assert(mi->MiType == MiTypeListbox);
	Assert(i >= 0);

	if (i >= mi->D.Listbox.noptions) {
		return NULL;
	}
	fl = mi->D.Listbox.options;
	info = fl[i].xdata;
	if (fl[i].type && i == mi->D.Listbox.curopt && info) {
		static char buffer[1024];
		int j;
		int n;

		// Draw optional description.
		if (info->Description) {
			VideoDrawText(mi->Menu->X + 8, mi->Menu->Y + 254, LargeFont, info->Description);
		}
		// Draw map size.
		sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
		VideoDrawText(mi->Menu->X + 8, mi->Menu->Y + 254 + 20, LargeFont, buffer);

		// Draw number of players.
		for (n = j = 0; j < PlayerMax; j++) {
			if (info->PlayerType[j] == PlayerPerson) {
				n++;
			}
		}
		if (n == 1) {
			VideoDrawText(mi->Menu->X + 8, mi->Menu->Y + 254 + 40, LargeFont, "1 player");
		} else {
			sprintf(buffer, "%d players", n);
			VideoDrawText(mi->Menu->X + 8, mi->Menu->Y + 254 + 40, LargeFont, buffer);
		}
	}
	return LBRetrieve(mi, i);
}

////////////////
//  LBAction  //
////////////////

/**
**  Update path and file name with the listbox.
**
**  @param mi    listbox.
**  @param i     line in the listbox.
**
**  @return 0 if nothing selected, 1 for file, 2 for directory.
*/
static int PathLBAction(const Menuitem* mi, int i)
{
	FileList *fl;

	Assert(mi->MiType == MiTypeListbox);
	Assert(i >= 0);

	ScenSelectFileName[0] = '\0';
	ScenSelectPathName[0] = '\0';
	SelectedFileExist = 0;
	if (i >= mi->D.Listbox.noptions) {
		return 0;
	}
	fl = mi->D.Listbox.options;
	if (fl[i].type) {
		Assert(strlen(fl[i].name) < sizeof(ScenSelectFileName));
		strcpy(ScenSelectFileName, fl[i].name);
		SelectedFileExist = 1;
		return 1;
	} else {
		Assert(strlen(fl[i].name) < sizeof(ScenSelectPathName));
		strcpy(ScenSelectPathName, fl[i].name);
		return 2;
	}
}
/**
** Save game listbox action callback
*/
static void SaveGameLBAction(Menuitem* mi, int i)
{
	Assert(mi->MiType == MiTypeListbox);
	Assert(i >= 0);

	PathLBAction(mi, i);
	sprintf(mi->Menu->Items[1].D.Input.buffer, "%s~!_", ScenSelectFileName);
	mi->Menu->Items[1].D.Input.nch = strlen(ScenSelectFileName);
}
/**
**  Editor save listbox action callback
*/
static void EditorSaveLBAction(Menuitem* mi, int i)
{
	Assert(mi->MiType == MiTypeListbox);
	Assert(i >= 0);

	PathLBAction(mi, i);
	sprintf(mi->Menu->Items[3].D.Input.buffer, "%s~!_", ScenSelectFileName);
	mi->Menu->Items[3].D.Input.nch = strlen(ScenSelectFileName);
}

//////////////
//  LBInit  //
//////////////

/**
**  Listbox exit callback.
**
**  @param mi  Listbox to free.
*/
static void LBExit(Menuitem* mi)
{
	FileList *fl;

	Assert(mi->MiType == MiTypeListbox);

	if (!mi->D.Listbox.noptions) {
		return ;
	}
	fl = mi->D.Listbox.options;
	FreeMapInfos(fl, mi->D.Listbox.noptions);
	free(fl);
	mi->D.Listbox.options = NULL;
	mi->D.Listbox.noptions = 0;
	mi[1].Flags |= MI_FLAGS_DISABLED;
}

/**
**  Listbox init callback.
**
**  @param mi  Listbox to free.
*/
static void LBInit(Menuitem* mi, const char* path, int (*filter)(char*, FileList*))
{
	int noptions;

	Assert(mi->MiType == MiTypeListbox);
	Assert(path);

	LBExit(mi);

	mi->D.Listbox.noptions = ReadDataDirectory(path, filter,
		(FileList **) & (mi->D.Listbox.options));

	noptions = mi->D.Listbox.noptions;
	mi->D.Listbox.startline = 0;
	if (mi->D.Listbox.noptions > 0) {
		mi->D.Listbox.curopt = 0;
		mi->D.Listbox.cursel = 0;
	} else {
		mi->D.Listbox.curopt = -1;
		mi->D.Listbox.cursel = -1;
	}
	mi[1].D.VSlider.percent = 0;
	mi[1].D.HSlider.percent = 0;
	PathLBAction(mi, 0);
	if (noptions > mi->D.Listbox.nlines) {
		mi[1].Flags &= ~MI_FLAGS_DISABLED;
	} else {
		mi[1].Flags |= MI_FLAGS_DISABLED;
	}
}

/**
** Init callback for listbox in save game menu
*/
static void SaveGameLBInit(Menuitem* mi)
{
	LBInit(mi, SaveDir, SaveGameRDFilter);
}
/**
** Init callback for listbox in load game menu
*/
static void LoadGameLBInit(Menuitem* mi)
{
	LBInit(mi, SaveDir, SaveGameRDFilter);
}
/**
**  Scenario select listbox init callback
*/
static void ScenSelectLBInit(Menuitem* mi)
{
	LBInit(mi, ScenSelectPath, ScenSelectRDFilter);
}
/**
** Editor main load listbox init callback
*/
static void EditorMainLoadLBInit(Menuitem* mi)
{
	LBInit(mi, ScenSelectPath, EditorMainLoadRDFilter);
}
/**
** Editor save listbox init callback
*/
static void EditorSaveLBInit(Menuitem* mi)
{
	LBInit(mi, ScenSelectPath, EditorSaveRDFilter);
	sprintf(mi->Menu->Items[3].D.Input.buffer, "%s~!_", ScenSelectFileName);
	mi->Menu->Items[3].D.Input.nch = strlen(ScenSelectFileName);
}
/**
** Replay game listbox init callback
*/
static void ReplayGameLBInit(Menuitem* mi)
{
	LBInit(mi, ScenSelectPath, ReplayGameRDFilter);
}

/////////////////////
// Folder.         //
/////////////////////

/**
**  Go to parent folder.
**
**  @return 1 if path have changed.
*/
static int ParentFolder()
{
	char *cp;

	if (!ScenSelectDisplayPath[0]) {
		return 0;
	}
	cp = strrchr(ScenSelectDisplayPath, '/');
	if (cp) {
		*cp = '\0';
	} else {
		ScenSelectDisplayPath[0] = '\0';
	}
	cp = strrchr(ScenSelectPath, '/');
	Assert(cp);
	*cp = 0;
	SelectedFileExist = 0;
	ScenSelectPathName[0] = '\0';
	ScenSelectFileName[0] = '\0';
	return 1;
}

/**
** Scenario select folder button
*/
static void ScenSelectFolder(void)
{
	if (ParentFolder()) {
		ScenSelectLBInit(CurrentMenu->Items + 1);
	}
}
/**
** Editor main load folder button
*/
static void EditorMainLoadFolder(void)
{
	if (ParentFolder()) {
		EditorMainLoadLBInit(CurrentMenu->Items + 1);
	}
}
/**
** Editor save folder button
*/
static void EditorSaveFolder(void)
{
	if (ParentFolder()) {
		EditorSaveLBInit(CurrentMenu->Items + 1);
	}
}
/**
**  Replay game folder button callback
*/
static void ReplayGameFolder(void)
{
	if (ParentFolder()) {
		ReplayGameLBInit(CurrentMenu->Items + 1);
	}
}

/**
** Initialize the hash table of menu functions
*/
void InitMenuFuncHash(void)
{
	HASHADD(NULL, "null");

#if 1 // Must be rename and remove duplicate.
	HASHADD(LBRetrieve, "save-game-lb-retrieve");
	HASHADD(LBRetrieve, "load-game-lb-retrieve");
	HASHADD(LBRetrieve, "editor-save-lb-retrieve");
	HASHADD(LBRetrieve, "replay-game-lb-retrieve");

	HASHADD(LBRetrieveAndInfo, "editor-load-lb-retrieve");
	HASHADD(LBRetrieveAndInfo, "scen-select-lb-retrieve");
	HASHADD(LBRetrieveAndInfo, "editor-main-load-lb-retrieve");

	HASHADD(PathLBAction, "scen-select-lb-action");
	HASHADD(SaveGameLBAction, "save-game-lb-action");
	HASHADD(PathLBAction, "load-game-lb-action");
	HASHADD(PathLBAction, "editor-main-load-lb-action");
	HASHADD(PathLBAction, "editor-load-lb-action");
	HASHADD(EditorSaveLBAction, "editor-save-lb-action");
	HASHADD(PathLBAction, "replay-game-lb-action");

#endif

// Game menu
	HASHADD(GameMenuInit,"game-menu-init");
	HASHADD(SaveGameMenu,"save-game-menu");
	HASHADD(LoadGameMenu,"load-game-menu");
	HASHADD(GameOptionsMenu,"game-options-menu");
	HASHADD(HelpMenu,"help-menu");
	HASHADD(ObjectivesMenu,"objectives-menu");
	HASHADD(EndScenarioMenu,"end-scenario-menu");
	HASHADD(GameMenuReturn,"game-menu-return");
	HASHADD(EndMenu,"end-menu");

// Victory, lost
	HASHADD(GameMenuEnd,"game-menu-end");
	HASHADD(VictoryInit,"victory-init");
	HASHADD(DefeatedInit,"defeated-init");
	HASHADD(SaveReplay,"save-replay");
	HASHADD(SaveReplayEnterAction,"save-replay-enter-action");
	HASHADD(SaveReplayOk,"save-replay-ok");

// Scenario select
	HASHADD(ScenSelectInit,"scen-select-init");
	HASHADD(ScenSelectExit,"scen-select-exit");
	HASHADD(ScenSelectTPMSAction,"scen-select-tpms-action");
	HASHADD(ScenSelectVSAction,"scen-select-vs-action");
	HASHADD(ScenSelectFolder,"scen-select-folder");
	HASHADD(ScenSelectOk,"scen-select-ok");
	HASHADD(ScenSelectCancel,"scen-select-cancel");
	HASHADD(ScenSelectRDFilter,"scen-select-rd-filter");

// Program start
	HASHADD(PrgStartInit,"program-start");
	HASHADD(NameLineDrawFunc,"name-line-draw");
	HASHADD(SinglePlayerGameMenu,"single-player-game-menu");
	HASHADD(MultiPlayerGameMenu,"multi-player-game-menu");
	HASHADD(CampaignGameMenu,"campaign-game-menu");
	HASHADD(ReplayGameMenu,"replay-game-menu");
	HASHADD(GlobalOptionsMenu,"global-options-menu");
	HASHADD(StartEditor,"game-start-editor");
	HASHADD(GameShowCredits,"game-show-credits");
	HASHADD(GameMenuExit,"game-menu-exit");

// Confirm menus
	HASHADD(RestartConfirmMenu,"restart-confirm-menu");
	HASHADD(SurrenderConfirmMenu,"surrender-confirm-menu");
	HASHADD(QuitToMenuConfirmMenu,"quit-to-menu-confirm-menu");
	HASHADD(ExitConfirmMenu,"exit-confirm-menu");

// Global Options
	HASHADD(GlobalOptionsInit,"global-options-init");
	HASHADD(GlobalOptionsExit,"global-options-exit");
	HASHADD(GlobalOptionsResolutionCheckbox,"global-options-resolution-checkbox");
	HASHADD(GlobalOptionsFullscreenCheckbox,"global-options-fullscreen-checkbox");

// Tips
	HASHADD(TipsInit,"tips-init");
	HASHADD(TipsExit,"tips-exit");
	HASHADD(TipsShowTipsCheckbox,"tips-show-tips-checkbox");
	HASHADD(TipsShowTipsText,"tips-show-tips-text");
	HASHADD(TipsNextTip,"tips-next-tip");
	HASHADD(TipsPreviousTip,"tips-previous-tip");

// Custom game setup
	HASHADD(GameSetupInit,"game-setup-init");
	HASHADD(ScenSelectMenu,"scen-select-menu");
	HASHADD(CustomGameStart,"custom-game-start");
	HASHADD(GameCancel,"game-cancel");
	HASHADD(GameDrawFunc,"game-draw-func");
	HASHADD(GameRCSAction,"game-rcs-action");
	HASHADD(GameRESAction,"game-res-action");
	HASHADD(GameUNSAction,"game-uns-action");
	HASHADD(GameTSSAction,"game-tss-action");
	HASHADD(GameGATAction,"game-gat-action");
	HASHADD(CustomGameOPSAction,"custom-game-ops-action");

// Enter name
	HASHADD(EnterNameAction,"enter-name-action");
	HASHADD(EnterNameCancel,"enter-name-cancel");

// Net create join
	HASHADD(JoinNetGameMenu,"net-join-game");
	HASHADD(CreateNetGameMenu,"net-create-game");
	HASHADD(CreateInternetGameMenu,"net-internet-create-game");

// Multi net type
	HASHADD(MultiPlayerLANGame,"net-lan-game");
	HASHADD(MultiPlayerInternetGame,"net-internet-game");

// Net multi setup
	HASHADD(MultiGameSetupInit,"multi-game-setup-init");
	HASHADD(MultiGameSetupExit,"multi-game-setup-exit");
	HASHADD(MultiGameDrawFunc,"multi-game-draw-func");
	HASHADD(MultiScenSelectMenu,"multi-scen-select");
	HASHADD(MultiGameStart,"multi-game-start");
	HASHADD(MultiGameCancel,"multi-game-cancel");
	HASHADD(MultiGamePTSAction,"multi-game-pts-action");
	HASHADD(MultiGameFWSAction,"multi-game-fws-action");

// Enter server ip
	HASHADD(EnterServerIPAction,"enter-server-ip-action");
	HASHADD(EnterServerIPCancel,"enter-server-ip-cancel");

// Net multi client
	HASHADD(TerminateNetConnect,"terminate-net-connect");
	HASHADD(MultiGameClientInit,"multi-game-client-init");
	HASHADD(MultiGameClientExit,"multi-game-client-exit");
	HASHADD(MultiGameClientDrawFunc,"multi-client-draw-func");
	HASHADD(MultiClientReady,"multi-client-ready");
	HASHADD(MultiClientNotReady,"multi-client-not-ready");
	HASHADD(MultiClientCancel,"multi-client-cancel");
	HASHADD(MultiClientRCSAction,"multi-client-rcs-action");
	HASHADD(MultiClientCheckboxAction,"multi-client-checkbox-action");
	HASHADD(NetMultiPlayerDrawFunc,"net-multi-player-draw-func");

// Net connecting
	HASHADD(NetConnectingInit,"net-connecting-init");
	HASHADD(NetConnectingExit,"net-connecting-exit");
	HASHADD(NetConnectingCancel,"net-connecting-cancel");

// Campaign select
	HASHADD(CampaignMenu1,"campaign-1");
	HASHADD(CampaignMenu2,"campaign-2");
	HASHADD(CampaignMenu3,"campaign-3");
	HASHADD(CampaignMenu4,"campaign-4");
	HASHADD(SelectCampaignMenu,"select-campaign-menu");

// End scenario
	HASHADD(EndScenarioRestart,"end-scenario-restart");
	HASHADD(EndScenarioSurrender,"end-scenario-surrender");
	HASHADD(EndScenarioQuitMenu,"end-scenario-quit-to-menu");

// Sound options
	HASHADD(SoundOptionsInit,"sound-options-init");
	HASHADD(SoundOptionsExit,"sound-options-exit");
	HASHADD(MasterVolumeHSAction,"master-volume-hs-action");
	HASHADD(SetMasterPower,"set-master-power");
	HASHADD(MusicVolumeHSAction,"music-volume-hs-action");
	HASHADD(SetMusicPower,"set-music-power");
	HASHADD(CdVolumeHSAction,"cd-volume-hs-action");
	HASHADD(SetCdPower,"set-cd-power");
	HASHADD(SetCdModeDefined,"set-cd-mode-defined");
	HASHADD(SetCdModeRandom,"set-cd-mode-random");

// Preferences
	HASHADD(PreferencesInit,"preferences-init");
	HASHADD(PreferencesExit,"preferences-exit");
	HASHADD(SetFogOfWar,"set-fog-of-war");
	HASHADD(SetCommandKey,"set-command-key");

// Speed options
	HASHADD(SpeedOptionsInit,"speed-options-init");
	HASHADD(SpeedOptionsExit,"speed-options-exit");
	HASHADD(GameSpeedHSAction,"game-speed-hs-action");
	HASHADD(MouseScrollHSAction,"mouse-scroll-hs-action");
	HASHADD(KeyboardScrollHSAction,"keyboard-scroll-hs-action");

// Game options
	HASHADD(SoundOptionsMenu,"sound-options-menu");
	HASHADD(SpeedOptionsMenu,"speed-options-menu");
	HASHADD(PreferencesMenu,"preferences-menu");
	HASHADD(DiplomacyMenu,"diplomacy-menu");

// Diplomacy options
	HASHADD(DiplomacyInit,"diplomacy-init");
	HASHADD(DiplomacyExit,"diplomacy-exit");
	HASHADD(DiplomacyWait,"diplomacy-wait");
	HASHADD(DiplomacyOk,"diplomacy-ok");

// Help
	HASHADD(KeystrokeHelpMenu,"keystroke-help-menu");
	HASHADD(TipsMenu,"tips-menu");

// Keystroke help
	HASHADD(KeystrokeHelpVSAction,"keystroke-help-vs-action");
	HASHADD(KeystrokeHelpDrawFunc,"keystroke-help-draw-func");

// Save
	HASHADD(SaveGameInit,"save-game-init");
	HASHADD(SaveGameExit,"save-game-exit");
	HASHADD(SaveGameEnterAction,"save-game-enter-action");
	HASHADD(SaveGameVSAction,"save-game-vs-action");
	HASHADD(SaveGameOk,"save-game-ok");
	HASHADD(DeleteConfirmMenu,"delete-confirm-menu");

// Load
	HASHADD(LoadGameInit,"load-game-init");
	HASHADD(LoadGameExit,"load-game-exit");
	HASHADD(LoadGameVSAction,"load-game-vs-action");
	HASHADD(LoadGameOk,"load-game-ok");

// Confirm save
	HASHADD(SaveConfirmInit,"save-confirm-init");
	HASHADD(SaveConfirmExit,"save-confirm-exit");
	HASHADD(SaveConfirmOk,"save-confirm-ok");
	HASHADD(SaveConfirmCancel,"save-confirm-cancel");

// Confirm delete
	HASHADD(DeleteConfirmInit,"delete-confirm-init");
	HASHADD(DeleteConfirmExit,"delete-confirm-exit");
	HASHADD(DeleteConfirmOk,"delete-confirm-ok");
	HASHADD(DeleteConfirmCancel,"delete-confirm-cancel");

// Save replay
	HASHADD(SaveReplayInit,"save-replay-init");
	HASHADD(SaveReplayExit,"save-replay-exit");

// Editor select
	HASHADD(EditorNewMap,"editor-new-map");
	HASHADD(EditorMainLoadMap,"editor-main-load-map");
	HASHADD(EditorSelectCancel,"editor-select-cancel");

// Editor new
	HASHADD(EditorNewDrawFunc,"editor-new-draw-func");
	HASHADD(EditorNewMapDescriptionEnterAction,"editor-new-map-description-enter-action");
	HASHADD(EditorNewMapSizeEnterAction,"editor-new-map-size-enter-action");
	HASHADD(EditorNewOk,"editor-new-ok");
	HASHADD(EditorNewCancel,"editor-new-cancel");

// Editor main load map
	HASHADD(EditorMainLoadInit,"editor-main-load-init");
	HASHADD(EditorMainLoadExit,"editor-main-load-exit");
	HASHADD(EditorMainLoadVSAction,"editor-main-load-vs-action");
	HASHADD(EditorMainLoadOk,"editor-main-load-ok");
	HASHADD(EditorMainLoadCancel,"editor-main-load-cancel");
	HASHADD(EditorMainLoadFolder,"editor-main-load-folder");

// Editor load map
	HASHADD(EditorMainLoadInit,"editor-load-init");
	HASHADD(EditorMainLoadExit,"editor-load-exit");
	HASHADD(EditorMainLoadVSAction,"editor-load-vs-action");
	HASHADD(EditorLoadOk,"editor-load-ok");
	HASHADD(EditorLoadCancel,"editor-load-cancel");
	HASHADD(EditorMainLoadFolder,"editor-load-folder");

// Editor menu
	HASHADD(EditorSaveMenu,"editor-save-menu");
	HASHADD(EditorLoadMenu,"editor-load-menu");
	HASHADD(EditorMapPropertiesMenu,"editor-map-properties-menu");
	HASHADD(EditorPlayerPropertiesMenu,"editor-player-properties-menu");
	HASHADD(EditorQuitToMenu,"editor-quit-to-menu");

// Editor map properties
	HASHADD(EditorMapPropertiesEnterAction,"editor-map-properties-enter-action");
	HASHADD(EditorMapPropertiesOk,"editor-map-properties-ok");
	HASHADD(EditorEndMenu,"editor-end-menu");

// Editor player properties
	HASHADD(EditorPlayerPropertiesDrawFunc,"editor-player-properties-draw-func");
	HASHADD(EditorPlayerPropertiesEnterAction,"editor-player-properties-enter-action");

// Editor edit resource
	HASHADD(EditorEditResourceEnterAction,"editor-edit-resource-enter-action");
	HASHADD(EditorEditResourceOk,"editor-edit-resource-ok");
	HASHADD(EditorEditResourceCancel,"editor-edit-resource-cancel");

// Editor edit ai properties
	HASHADD(EditorEditAiPropertiesCheckbox,"editor-edit-ai-properties-checkbox");
	HASHADD(EditorEditAiPropertiesOk,"editor-edit-ai-properties-ok");
	HASHADD(EditorEditAiPropertiesCancel,"editor-edit-ai-properties-cancel");

// Editor save
	HASHADD(EditorSaveInit,"editor-save-init");
	HASHADD(EditorSaveExit,"editor-save-exit");
	HASHADD(EditorSaveFolder,"editor-save-folder");
	HASHADD(EditorSaveVSAction,"editor-save-vs-action");
	HASHADD(EditorSaveEnterAction,"editor-save-enter-action");
	HASHADD(EditorSaveOk,"editor-save-ok");
	HASHADD(EditorSaveCancel,"editor-save-cancel");

// Editor save confirm
	HASHADD(EditorSaveConfirmInit,"editor-save-confirm-init");
	HASHADD(EditorSaveConfirmOk,"editor-save-confirm-ok");
	HASHADD(EditorSaveConfirmCancel,"editor-save-confirm-cancel");

// Replay game
	HASHADD(ReplayGameInit,"replay-game-init");
	HASHADD(ReplayGameExit,"replay-game-exit");
	HASHADD(ReplayGameVSAction,"replay-game-vs-action");
	HASHADD(ReplayGameFolder,"replay-game-folder");
	HASHADD(ReplayGameDisableFog,"replay-game-disable-fog");
	HASHADD(ReplayGameOk,"replay-game-ok");
	HASHADD(ReplayGameCancel,"replay-game-cancel");

// Metaserver
	HASHADD(ShowMetaServerList,"metaserver-list");
	HASHADD(MultiMetaServerGameSetupInit,"metaserver-list-init");
	HASHADD(MultiMetaServerGameSetupExit,"metaserver-list-exit");
	HASHADD(SelectGameServer,"select-game-server");
	HASHADD(MultiMetaServerClose,"menu-internet-end-menu");
}

/*----------------------------------------------------------------------------
--  Button action handler and Init/Exit functions
----------------------------------------------------------------------------*/

/**
**  Draw the version, copyright, and license at bottom of the screen.
*/
static void NameLineDrawFunc(Menuitem* mi __attribute__ ((unused)))
{
	char* nc;
	char* rc;

	GetDefaultTextColors(&nc, &rc);
	SetDefaultTextColors(rc, rc);

	if (!SoundEnabled() && !SoundOff) {
		VideoDrawText(16, 16, LargeFont, "Sound disabled, please check!");
	}

	VideoDrawTextCentered(VideoWidth / 2, TheUI.Offset480Y + 440, GameFont, NameLine);
	VideoDrawTextCentered(VideoWidth / 2, TheUI.Offset480Y + 456, GameFont,
		"Engine distributed under the terms of the GNU General Public License.");
	SetDefaultTextColors(nc, rc);
}

/**
** Start menu master init.
**
** @param mi The menu.
*/
static void PrgStartInit(Menu* menu)
{
	if (NetworkNumInterfaces == 0) {
		menu->Items[2].Flags = MI_FLAGS_DISABLED;
	} else {
		menu->Items[2].Flags = 0;
	}
}

/**
** Leave menus and return to game mode
*/
static void GameMenuReturn(void)
{
	while (CurrentMenu) {
		EndMenu();
	}
	InterfaceState = IfaceStateNormal;
	ClearStatusLine();
	GamePaused = 0;
}

/**
**  Init callback for save game menu
*/
static void SaveGameInit(Menu* menu)
{
	char* buf;

	buf = malloc(64);
	strcpy(buf, "~!_");
	menu->Items[1].D.Input.buffer = buf;
	menu->Items[1].D.Input.nch = 0;
	menu->Items[1].D.Input.maxch = 60;

	menu->Items[4].Flags = MI_FLAGS_DISABLED;
	menu->Items[5].Flags = MI_FLAGS_DISABLED;
	CreateSaveDir();
	SaveGameLBInit(menu->Items + 2);
}

/**
**  Exit callback for save game menu
*/
static void SaveGameExit(Menu* menu)
{
	free(menu->Items[1].D.Input.buffer);
	menu->Items[1].D.Input.buffer = NULL;
	LBExit(menu->Items + 2);
}

/**
** Save game input box callback
*/
static void SaveGameEnterAction(Menuitem* mi, int key)
{
	Assert(mi->MiType == MiTypeInput);

	strncpy(ScenSelectFileName, mi->D.Input.buffer, mi->D.Input.nch);
	ScenSelectFileName[mi->D.Input.nch] = '\0';
	ScenSelectPathName[0] = '\0';
	SelectedFileExist = 0;
	if (mi->D.Input.nch != 0) {
		if (key == 10 || key == 13) {
			SaveGameOk();
			return;
		}
	}
}

/**
** Save game
*/
static void SaveGameOk(void)
{
	const char* name;

	name = ScenSelectFileName;

	if (ScenSelectFileName[0] != '\0') {
		strcpy(TempPathBuf, SaveDir);
		strcat(TempPathBuf, "/");
		strcat(TempPathBuf, name);

		// Strip .gz extension.
		if (!strcmp(TempPathBuf + strlen(TempPathBuf) - 3, ".gz")) {
			TempPathBuf[strlen(TempPathBuf) - 3] = '\0';
		}
		// Strip .bz2 extension.
		if (!strcmp(TempPathBuf + strlen(TempPathBuf) - 4, ".bz2")) {
			TempPathBuf[strlen(TempPathBuf) - 4] = '\0';
		}
		// Add .sav if not already there.
		if (strcmp(TempPathBuf + strlen(TempPathBuf) - 4, ".sav")) {
			strcat(TempPathBuf, ".sav");
		}

		if (access(TempPathBuf,F_OK)) {
			SaveGame(TempPathBuf);
			SetMessage("Saved game to: %s", TempPathBuf);
			EndMenu();
		} else {
			ProcessMenu("menu-save-confirm", 0);
		}
	}
}

/**
** Create the save directory
*/
static void CreateSaveDir(void)
{
	if (SaveDir) {
		free(SaveDir);
	}

#ifdef USE_WIN32
	strcpy(TempPathBuf, GameName);
	mkdir(TempPathBuf);
	strcat(TempPathBuf, "/save");
	SaveDir=strdup(TempPathBuf);
	mkdir(SaveDir);
#else
	strcpy(TempPathBuf, getenv("HOME"));
	strcat(TempPathBuf, "/");
	strcat(TempPathBuf, STRATAGUS_HOME_PATH);
	mkdir(TempPathBuf, 0777);
	strcat(TempPathBuf, "/");
	strcat(TempPathBuf, GameName);
	mkdir(TempPathBuf, 0777);
	strcat(TempPathBuf, "/save");
	mkdir(TempPathBuf, 0777);
	SaveDir = strdup(TempPathBuf);
#endif
}

/**
** Save game menu
*/
void SaveGameMenu(void)
{
	ProcessMenu("menu-save-game", 0);
}

/**
**  Save game vertical slider callback
*/
static void SaveGameVSAction(Menuitem* mi)
{
}

/**
**  Load game init callback
*/
static void LoadGameInit(Menu* menu)
{
	menu->Items[3].Flags = MI_FLAGS_DISABLED;
	CreateSaveDir();
	LoadGameLBInit(menu->Items + 1);
}

/**
**  Load game exit callback
*/
static void LoadGameExit(Menu* menu)
{
	LBExit(menu->Items + 1);
}

/**
**  Load game vertical slider callback
*/
static void LoadGameVSAction(Menuitem* mi)
{
}

/**
** Load game ok button callback
*/
static void LoadGameOk(void)
{
	if (!ScenSelectFileName[0]) {
		return ;
	}
	sprintf(TempPathBuf, "%s/%s", SaveDir, ScenSelectFileName);
	SaveGameLoading = 1;
	LoadGame(TempPathBuf);
	Callbacks = &GameCallbacks;
	SetMessage("Loaded game: %s", TempPathBuf);
	GuiGameStarted = 1;
	GameMenuReturn();
	SelectedFileExist = 0;
	ScenSelectFileName[0] = '\0';
	ScenSelectPathName[0] = '\0';
}

/**
**  Save confirm init callback
*/
static void SaveConfirmInit(Menu* menu)
{
	static char name[PATH_MAX]; // FIXME: much memory wasted
	int fileLength;
	Menu* save_menu;

	save_menu = FindMenu("menu-save-game");
	fileLength = strlen(save_menu->Items[1].D.Input.buffer) - 3;

	strcpy(name, "the file: ");
	strncat(name, save_menu->Items[1].D.Input.buffer, fileLength);
	if (strstr(name, ".sav") == NULL) {
		strcat(name, ".sav");
	}
	menu->Items[2].D.Text.text = NewStringDesc(name);
}

/**
**  Save confirm exit callback
*/
static void SaveConfirmExit(Menu* menu)
{
	FreeStringDesc(menu->Items[2].D.Text.text);
	free(menu->Items[2].D.Text.text);
	menu->Items[2].D.Text.text = NULL;
}

/**
**  Save confirm ok button
*/
static void SaveConfirmOk(void)
{
	int fileLength;
	Menu* menu;

	menu = FindMenu("menu-save-game");
	fileLength = strlen(menu->Items[1].D.Input.buffer) - 3;

	strcpy(TempPathBuf, SaveDir);
	strcat(TempPathBuf, "/");
	strncat(TempPathBuf, menu->Items[1].D.Input.buffer, fileLength);
	if (strstr(TempPathBuf, ".sav") == NULL) {
		strcat(TempPathBuf, ".sav");
	}
	SaveGame(TempPathBuf);
	SetMessage("Saved game to: %s", TempPathBuf);
	GameMenuReturn();
	SelectedFileExist = 0;
	ScenSelectFileName[0] = '\0';
	ScenSelectPathName[0] = '\0';
}

/**
** Save confirm cancel button
*/
static void SaveConfirmCancel(void)
{
	EndMenu();
}

/**
** Delete menu
*/
static void DeleteConfirmMenu(void)
{
	ProcessMenu("menu-delete-confirm", 0);
}

/**
**  Init callback for delete confirm menu
*/
static void DeleteConfirmInit(Menu* menu)
{
	Menu* save_menu;
	static char name[PATH_MAX]; // FIXME: much memory wasted

	save_menu = FindMenu("menu-save-game");
	strcpy(name, "the file: ");
	strcat(name, save_menu->Items[1].D.Input.buffer);
	name[strlen(name) - 3] = '\0';
	menu->Items[2].D.Text.text = NewStringDesc(name);
}

/**
**  Exit callback for delete confirm menu
*/
static void DeleteConfirmExit(Menu* menu)
{
	FreeStringDesc(menu->Items[2].D.Text.text);
	free(menu->Items[2].D.Text.text);
	menu->Items[2].D.Text.text = NULL;
}

/**
**  Delete confirm ok button
*/
static void DeleteConfirmOk(void)
{
	Menu* menu;

	menu = FindMenu("menu-save-game");
	strcpy(TempPathBuf, SaveDir);
	strcat(TempPathBuf, "/");
	strcat(TempPathBuf, menu->Items[1].D.Input.buffer);
	TempPathBuf[strlen(TempPathBuf) - 3] = '\0';
	unlink(TempPathBuf);
	EndMenu();

	// Update list of files and clear input
	SaveGameLBInit(&CurrentMenu->Items[2]);
	strcpy(CurrentMenu->Items[1].D.Input.buffer, "~!_");
	CurrentMenu->Items[1].D.Input.nch = 0;
	SelectedFileExist = 0;
	ScenSelectFileName[0] = '\0';
	ScenSelectPathName[0] = '\0';
}

/**
** Delete confirm cancel button
*/
static void DeleteConfirmCancel(void)
{
	EndMenu();
}

/**
** Load game menu
*/
void LoadGameMenu(void)
{
	ProcessMenu("menu-load-game", 0);
}

/**
** Init callback for game menu
*/
static void GameMenuInit(Menu* menu)
{
	// Disable save menu in multiplayer and replays
	if (IsNetworkGame() || ReplayGameType != ReplayNone) {
		menu->Items[1].Flags |= MI_FLAGS_DISABLED;
	} else {
		menu->Items[1].Flags &= ~MI_FLAGS_DISABLED;
	}

	// Disable load menu in multiplayer
	if (IsNetworkGame()) {
		menu->Items[2].Flags |= MI_FLAGS_DISABLED;
	} else {
		menu->Items[2].Flags &= ~MI_FLAGS_DISABLED;
	}
}

/**
** Sound options menu
*/
void SoundOptionsMenu(void)
{
	ProcessMenu("menu-sound-options", 0);
}

/**
** Init callback for sound options menu
*/
static void SoundOptionsInit(Menu* menu __attribute__((unused)))
{
	if (!menu) {
		menu = FindMenu("menu-sound-options");
	}

	// master volume slider
	if (SoundOff || !SoundEnabled()) {
		menu->Items[2].Flags = MI_FLAGS_DISABLED;
	} else {
		menu->Items[2].Flags = 0;
		menu->Items[2].D.HSlider.percent = (GlobalVolume * 100) / 255;
	}

	// master power
	if (SoundOff || !SoundEnabled()) {
		menu->Items[5].D.Checkbox.Checked = 0;
	} else {
		menu->Items[5].D.Checkbox.Checked = 1;
	}

	// music volume slider
	if (SoundOff || PlayingMusic != 1 || !SoundEnabled()) {
		menu->Items[7].Flags = MI_FLAGS_DISABLED;
	} else {
		menu->Items[7].Flags = 0;
		menu->Items[7].D.HSlider.percent = (MusicVolume * 100) / 255;
	}

	// music power
	if (SoundOff || !SoundEnabled()) {
		menu->Items[10].Flags = MI_FLAGS_DISABLED;
	} else {
		menu->Items[10].Flags = 0;
	}
#ifdef USE_CDAUDIO
	if (CDMode != CDModeStopped && CDMode != CDModeOff) {
		menu->Items[7].Flags = MI_FLAGS_DISABLED;
		menu->Items[10].Flags = MI_FLAGS_DISABLED;
	}
#endif
	if (SoundOff || PlayingMusic != 1 || !SoundEnabled()) {
		menu->Items[10].D.Checkbox.Checked = 0;
	} else {
		menu->Items[10].D.Checkbox.Checked = 1;
	}

	menu->Items[12].Flags = MI_FLAGS_DISABLED; // cd volume slider
	menu->Items[15].Flags = MI_FLAGS_DISABLED; // cd power
	menu->Items[15].D.Checkbox.Checked = 0;
	menu->Items[16].Flags = MI_FLAGS_DISABLED; // all tracks button
	menu->Items[17].Flags = MI_FLAGS_DISABLED; // random tracks button
#ifdef USE_CDAUDIO
	menu->Items[15].Flags = 0; // cd power
	if (CDMode != CDModeStopped && CDMode != CDModeOff) {
#if (!defined(USE_WIN32) && defined(USE_LIBCDA)) || defined(USE_CDDA)
		int i = 0;
		i = GetCDVolume();

		menu->Items[12].Flags = 0;
		menu->Items[12].D.HSlider.percent = (i * 100) / 255;
#endif
		menu->Items[15].D.Checkbox.Checked = 1;
		menu->Items[16].Flags = 0;
		menu->Items[17].Flags = 0;

		if (CDMode == CDModeDefined) {
			menu->Items[16].D.Checkbox.Checked = 1;
			menu->Items[17].D.Checkbox.Checked = 0;
		} else if (CDMode == CDModeRandom) {
			menu->Items[16].D.Checkbox.Checked = 0;
			menu->Items[17].D.Checkbox.Checked = 1;
		}
	}
#endif // cd
}

/**
**  Exit callback for sound options menu
*/
static void SoundOptionsExit(Menu* menu __attribute__((unused)))
{
	// FIXME: Only save if something changed
	SavePreferences();
}

/**
** Global options menu
*/
static void GlobalOptionsMenu(void)
{
	ProcessMenu("menu-global-options", 1);
}

/**
** Init callback for global options menu
*/
static void GlobalOptionsInit(Menu* menu)
{
	menu->Items[2].D.Checkbox.Checked = 0;
	menu->Items[3].D.Checkbox.Checked = 0;
	menu->Items[4].D.Checkbox.Checked = 0;
	menu->Items[5].D.Checkbox.Checked = 0;
	menu->Items[6].D.Checkbox.Checked = 0;
	menu->Items[7].D.Checkbox.Checked = 0;

	if (VideoWidth == 640) {
		menu->Items[2].D.Checkbox.Checked = 1;
	} else if (VideoWidth == 800) {
		menu->Items[3].D.Checkbox.Checked = 1;
	} else if (VideoWidth == 1024) {
		menu->Items[4].D.Checkbox.Checked = 1;
	} else if (VideoWidth == 1280) {
		menu->Items[5].D.Checkbox.Checked = 1;
	} else if (VideoWidth == 1600) {
		menu->Items[6].D.Checkbox.Checked = 1;
	}

	if (VideoFullScreen) {
		menu->Items[7].D.Checkbox.Checked = 1;
	}
}

/**
**  Exit callback for global options menu
*/
static void GlobalOptionsExit(Menu* menu __attribute__((unused)))
{
	// FIXME: Only save if something changed
	SavePreferences();
}

/**
**  Global options resolution gem callback
*/
static void GlobalOptionsResolutionCheckbox(Menuitem* mi)
{
	int res;

	res = VideoWidth;
	switch (mi - mi->Menu->Items) {
		case 2:
			res = 640;
			break;
		case 3:
			res = 800;
			break;
		case 4:
			res = 1024;
			break;
		case 5:
			res = 1280;
			break;
		case 6:
			res = 1600;
			break;
	}

	if (VideoWidth != res) {
		if (VideoValidResolution(res, res * 3 / 4)) {
			Menu* menu;

			VideoWidth = res;
			VideoHeight = res * 3 / 4;
			SavePreferences();
			ExitMenus();
			InitVideo();
			// Force Update Background Size
			SetClipping(0, 0, VideoWidth - 1, VideoHeight - 1);
			CleanModules();
			LoadCcl();
			PreMenuSetup();
			GameCursor = TheUI.Point.Cursor;
			menu = FindMenu("menu-program-start");
			if (menu->InitFunc) {
				menu->InitFunc(menu);
			}
			if (menu->BackgroundG) {
				ResizeGraphic(menu->BackgroundG, VideoWidth, VideoHeight);
			}
			DrawMenu(menu);
			CurrentMenu = FindMenu("menu-global-options");
		} else {
			ErrorMenu("Invalid resolution");
		}
	}
	GlobalOptionsInit(CurrentMenu);
}

/**
** Global options fullscreen gem callback
*/
static void GlobalOptionsFullscreenCheckbox(Menuitem* mi __attribute__((unused)))
{
	ToggleFullScreen();
	GlobalOptionsInit(CurrentMenu);
}

/**
** Master volume gem callback
*/
static void SetMasterPower(Menuitem* mi __attribute__((unused)))
{
	if (SoundEnabled()) {
		SoundOff ^= 1;
		SoundOptionsInit(NULL);
	}
}

/**
** Music volume gem callback
*/
static void SetMusicPower(Menuitem* mi __attribute__((unused)))
{
	if (PlayingMusic) {
		MusicOff = 1;
		StopMusic();
	} else {
		MusicOff = 0;
		if (CallbackMusic) {
			lua_pushstring(Lua, "MusicStopped");
			lua_gettable(Lua, LUA_GLOBALSINDEX);
			if (!lua_isfunction(Lua, -1)) {
				fprintf(stderr, "No MusicStopped in Lua\n");
				MusicOff = 1;
				StopMusic();
			} else {
				LuaCall(0, 1);
			}
		}
	}

	SoundOptionsInit(NULL);
}

/**
** CD volume gem callback
*/
static void SetCdPower(Menuitem* mi __attribute__((unused)))
{
#ifdef USE_CDAUDIO
	// Start Playing CD
	if (CDMode == CDModeOff || CDMode == CDModeStopped) {
		ResumeCD();
	} else {
	// Stop Playing CD
		PauseCD();
	}
#endif
	SoundOptionsInit(NULL);
}

/**
** Toggle the fog of war handling.
** Used in the preference menu.
**
** @param mi Menu item (not used).
*/
static void SetFogOfWar(Menuitem* mi __attribute__((unused)))
{
	if (!TheMap.NoFogOfWar) {
		TheMap.NoFogOfWar = 1;
		UpdateFogOfWarChange();
		CommandLog("input", NoUnitP, FlushCommands, -1, -1, NoUnitP, "fow off", -1);
	} else {
		TheMap.NoFogOfWar = 0;
		UpdateFogOfWarChange();
		CommandLog("input", NoUnitP, FlushCommands, -1, -1, NoUnitP, "fow on", -1);
	}
}

/**
** Toggle showing the command chars on icons.
** Used in the preference menu.
**
** @param mi Menu item (not used).
*/
static void SetCommandKey(Menuitem* mi __attribute__((unused)))
{
	ShowCommandKey ^= 1;
}

/**
** CD play all tracks gem callback
*/
static void SetCdModeDefined(Menuitem* mi __attribute__((unused)))
{
#ifdef USE_CDAUDIO
	CDMode = CDModeDefined;
#endif
	SoundOptionsInit(NULL);
}

/**
** CD play random tracks gem callback
*/
static void SetCdModeRandom(Menuitem* mi __attribute__((unused)))
{
#ifdef USE_CDAUDIO
	CDMode = CDModeRandom;
#endif
	SoundOptionsInit(NULL);
}

/**
** Speed settings menu
*/
void SpeedOptionsMenu(void)
{
	ProcessMenu("menu-speed-options", 0);
}

/**
** Init callback for speed settings menu
*/
void SpeedOptionsInit(Menu* menu)
{
	int i;

	i = 2;

	menu->Items[i].D.HSlider.percent = ((VideoSyncSpeed - MIN_GAME_SPEED) * 100) / (MAX_GAME_SPEED - MIN_GAME_SPEED);
	if (menu->Items[i].D.HSlider.percent < 0) {
		menu->Items[i].D.HSlider.percent = 0;
	}
	if (menu->Items[i].D.HSlider.percent > 100) {
		menu->Items[i].D.HSlider.percent = 100;
	}

	menu->Items[i + 4].D.HSlider.percent = 100 - (SpeedMouseScroll - 1) * 100 / 10;
	if (TheUI.MouseScroll == 0) {
		menu->Items[i + 4].D.HSlider.percent = 0;
	}
	menu->Items[i + 8].D.HSlider.percent = 100 - (SpeedKeyScroll - 1) * 100 / 10;
	if (TheUI.KeyScroll == 0) {
		menu->Items[i + 8].D.HSlider.percent = 0;
	}
}

/**
**  Exit callback for speed settings menu
*/
void SpeedOptionsExit(Menu* menu __attribute__((unused)))
{
	// FIXME: Only save if something changed
	SavePreferences();
}

/**
**  Diplomacy options menu
*/
void DiplomacyMenu(void)
{
	ProcessMenu("menu-diplomacy", 0);
}

/**
**  Diplomacy init callback
*/
static void DiplomacyInit(Menu* menu __attribute__ ((unused)))
{
	int i;
	int j;

	j = 0;

	for (i = 0; i <= PlayerMax - 2; ++i) {
		if (Players[i].Type != PlayerNobody && &Players[i] != ThisPlayer) {
			menu->Items[4 * j + 4].D.Text.text = NewStringDesc(Players[i].Name);
			if (ThisPlayer->Allied&(1<<Players[i].Player)) {
				menu->Items[4 * j + 5].D.Checkbox.Checked = 1;
			} else {
				menu->Items[4 * j + 5].D.Checkbox.Checked = 0;
			}
			if (ThisPlayer->Enemy&(1<<Players[i].Player)) {
				menu->Items[4 * j + 6].D.Checkbox.Checked = 1;
			} else {
				menu->Items[4 * j + 6].D.Checkbox.Checked = 0;
			}
			if (ThisPlayer->SharedVision&(1<<Players[i].Player)) {
				menu->Items[4 * j + 7].D.Checkbox.Checked = 1;
			} else {
				menu->Items[4 * j + 7].D.Checkbox.Checked = 0;
			}

			if (ReplayGameType != ReplayNone || ThisPlayer->Team == Players[i].Team) {
				menu->Items[4 * j + 5].Flags |= MI_FLAGS_DISABLED;
				menu->Items[4 * j + 6].Flags |= MI_FLAGS_DISABLED;
				menu->Items[4 * j + 7].Flags |= MI_FLAGS_DISABLED;
			} else {
				menu->Items[4 * j + 5].Flags &= ~MI_FLAGS_DISABLED;
				menu->Items[4 * j + 6].Flags &= ~MI_FLAGS_DISABLED;
				menu->Items[4 * j + 7].Flags &= ~MI_FLAGS_DISABLED;
			}

			++j;
		}
	}
	for (; j <= PlayerMax - 3; ++j) {
		menu->Items[4 * j + 4].D.Text.text = NULL;
		menu->Items[4 * j + 5].Flags |= MI_FLAGS_INVISIBLE;
		menu->Items[4 * j + 6].Flags |= MI_FLAGS_INVISIBLE;
		menu->Items[4 * j + 7].Flags |= MI_FLAGS_INVISIBLE;
	}
}

/**
**  Diplomacy exit callback
*/
static void DiplomacyExit(Menu* menu)
{
	int i;

	for (i = 0; i <= PlayerMax - 3; ++i) {
		FreeStringDesc(menu->Items[4 * i + 4].D.Text.text);
		free(menu->Items[4 * i + 4].D.Text.text);
		menu->Items[4 * i + 4].D.Text.text = NULL;
	}
}

/**
** Diplomacy gem callback
*/
static void DiplomacyWait(Menuitem* mi)
{
	int player;
	int item;

	item = mi - mi->Menu->Items;
	player = (item - 4) / 4;

	// Don't allow allies and enemies at the same time
	if (item == 4 * player + 5) {
		mi->Menu->Items[4 * player + 5].D.Checkbox.Checked = 1;
		mi->Menu->Items[4 * player + 6].D.Checkbox.Checked = 0;
	} else if (item == 4 * player + 6) {
		mi->Menu->Items[4 * player + 5].D.Checkbox.Checked = 0;
		mi->Menu->Items[4 * player + 6].D.Checkbox.Checked = 1;
	}

	// Don't set diplomacy until clicking ok
}

/**
** Diplomacy Ok button callback
*/
static void DiplomacyOk(void)
{
	Menu* menu;
	int i;
	int j;

	menu = CurrentMenu;
	j = 0;

	for (i=0; i<=PlayerMax - 2; ++i) {
		if (Players[i].Type != PlayerNobody && &Players[i] != ThisPlayer) {
			// Menu says to ally
			if (menu->Items[4 * j + 5].D.Checkbox.Checked &&
				!menu->Items[4 * j + 6].D.Checkbox.Checked) {
				// Are they allied?
				if (!(ThisPlayer->Allied & (1 << Players[i].Player) &&
						!(ThisPlayer->Enemy & (1 << Players[i].Player)))) {
					SendCommandDiplomacy(ThisPlayer->Player, DiplomacyAllied,
						Players[i].Player);
				}
			}
			// Menu says to be enemies
			if (!menu->Items[4 * j + 5].D.Checkbox.Checked &&
				menu->Items[4 * j + 6].D.Checkbox.Checked) {
				// Are they enemies?
				if (!(!(ThisPlayer->Allied & (1 << Players[i].Player)) &&
						ThisPlayer->Enemy & (1 << Players[i].Player))) {
					SendCommandDiplomacy(ThisPlayer->Player, DiplomacyEnemy,
						Players[i].Player);
				}
			}
			// Menu says to be neutral
			if (!menu->Items[4 * j + 5].D.Checkbox.Checked &&
				!menu->Items[4 * j + 6].D.Checkbox.Checked) {
				// Are they neutral?
				if (!(!(ThisPlayer->Allied & (1 << Players[i].Player)) &&
						!(ThisPlayer->Enemy & (1 << Players[i].Player)))) {
					SendCommandDiplomacy(ThisPlayer->Player, DiplomacyNeutral,
						Players[i].Player);
				}
			}
			// Menu says to be crazy
			if (menu->Items[4 * j + 5].D.Checkbox.Checked &&
				menu->Items[4 * j + 6].D.Checkbox.Checked) {
				// Are they crazy?
				if (!(ThisPlayer->Allied & (1 << Players[i].Player) &&
						ThisPlayer->Enemy & (1 << Players[i].Player))) {
					SendCommandDiplomacy(ThisPlayer->Player, DiplomacyCrazy,
						Players[i].Player);
				}
			}
			// Shared vision
			if (menu->Items[4 * j + 7].D.Checkbox.Checked) {
				if (!(ThisPlayer->SharedVision & (1 << Players[i].Player))) {
					SendCommandSharedVision(ThisPlayer->Player, 1,
						Players[i].Player);
				}
			}
			else {
				if (ThisPlayer->SharedVision&(1<<Players[i].Player)) {
					SendCommandSharedVision(ThisPlayer->Player,0,
						Players[i].Player);
				}
			}

			++j;
		}
	}
	EndMenu();
}

/**
** Enter the preferences menu.
** Setup if the options are enabled / disabled.
*/
void PreferencesMenu(void)
{
	ProcessMenu("menu-preferences", 0);
}

/**
**  Preferences menu init callback
*/
static void PreferencesInit(Menu* menu)
{
	if (!TheMap.NoFogOfWar) {
		menu->Items[1].D.Checkbox.Checked = 1;
	} else {
		menu->Items[1].D.Checkbox.Checked = 0;
	}

	// Not available in net games or replays
	if (!IsNetworkGame() && ReplayGameType == ReplayNone) {
		menu->Items[1].Flags = MI_FLAGS_NONE;
	} else {
		menu->Items[1].Flags = MI_FLAGS_DISABLED;
	}

	if (ShowCommandKey) {
		menu->Items[2].D.Checkbox.Checked = 1;
	} else {
		menu->Items[2].D.Checkbox.Checked = 0;
	}
}

/**
**  Preferences menu init callback
*/
static void PreferencesExit(Menu* menu __attribute__((unused)))
{
	// FIXME: Only save if something changed
	SavePreferences();
}

/**
**  Show the game options.
*/
static void GameOptionsMenu(void)
{
	ProcessMenu("menu-game-options", 0);
}

/**
**  Show the game credits.
*/
static void GameShowCredits(void)
{
	ShowCredits(&GameCredits);
}

/**
**  Show the Restart Confirm menu
*/
void RestartConfirmMenu(void)
{
	ProcessMenu("menu-restart-confirm", 0);
}

/**
**  Show the Surrender Confirm menu
*/
static void SurrenderConfirmMenu(void)
{
	ProcessMenu("menu-surrender-confirm", 0);
}

/**
**  Show the Quit To Menu Confirm menu
*/
void QuitToMenuConfirmMenu(void)
{
	ProcessMenu("menu-quit-to-menu-confirm", 0);
}

/**
**  Show the Exit Confirm menu
*/
void ExitConfirmMenu(void)
{
	ProcessMenu("menu-exit-confirm", 0);
}

/**
**  Show the End Scenario menu
*/
static void EndScenarioMenu(void)
{
	Menu* menu;

	menu = FindMenu("menu-end-scenario");
	if (!IsNetworkGame()) {
		menu->Items[1].Flags = MI_FLAGS_NONE;
	} else {
		menu->Items[1].Flags = MI_FLAGS_DISABLED;
	}

	ProcessMenu("menu-end-scenario", 0);
}

/**
**  Restart the scenario
*/
static void EndScenarioRestart(void)
{
	RestartScenario = 1;
	GameRunning = 0;
	GameMenuReturn();
}

/**
**  End the game in defeat
*/
static void EndScenarioSurrender(void)
{
	GameResult = GameDefeat;
	GameRunning = 0;
	GameMenuReturn();
}

/**
**  End the game and return to the menu
*/
static void EndScenarioQuitMenu(void)
{
	QuitToMenu = 1;
	GameRunning = 0;
	GameMenuReturn();
}

/**
**  End the running game from menu.
*/
static void GameMenuEnd(void)
{
	InterfaceState = IfaceStateNormal;
	GameRunning = 0;
	CursorOn = CursorOnUnknown;
	CurrentMenu = NULL;
}

/**
**  Victory menu init callback
*/
static void VictoryInit(Menu* menu)
{
	if (CommandLogDisabled) {
		menu->Items[3].Flags = MI_FLAGS_DISABLED;
	}
}

/**
**  Defeated menu init callback
*/
static void DefeatedInit(Menu* menu)
{
	if (CommandLogDisabled) {
		menu->Items[3].Flags = MI_FLAGS_DISABLED;
	}
}

/**
**  Save replay of completed game.
*/
static void SaveReplay(void)
{
	ProcessMenu("menu-save-replay", 0);
}

/**
**  Save replay menu init callback
*/
static void SaveReplayInit(Menu* menu)
{
	char* buf;

	buf = malloc(32);
	strcpy(buf, "~!_");
	menu->Items[1].D.Input.buffer = buf;
	menu->Items[1].D.Input.nch = 0;
	menu->Items[1].D.Input.maxch = 28;
}

/**
**  Save replay menu exit callback
*/
static void SaveReplayExit(Menu* menu)
{
	free(menu->Items[1].D.Input.buffer);
	menu->Items[1].D.Input.buffer = NULL;
}

/**
**  Input field action of save replay menu.
*/
static void SaveReplayEnterAction(Menuitem* mi, int key)
{
	Assert(mi->MiType == MiTypeInput);

	strncpy(ScenSelectFileName, mi->D.Input.buffer, mi->D.Input.nch);
	ScenSelectFileName[mi->D.Input.nch] = '\0';
	ScenSelectPathName[0] = '\0';
	SelectedFileExist = 0;
	if (mi->D.Input.nch != 0) {
		if (key == 10 || key == 13) {
			SaveReplayOk();
		}
	}
}

/**
** Save replay Ok button.
*/
static void SaveReplayOk(void)
{
	FILE *fd;
	Menu* menu;
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
	sprintf(TempPathBuf, "%s/logs/",GameName);
#else
	sprintf(TempPathBuf, "%s/%s/%s", getenv("HOME"), STRATAGUS_HOME_PATH,GameName);
	strcat(TempPathBuf, "/logs/");
#endif
	ptr = TempPathBuf + strlen(TempPathBuf);
	sprintf(ptr, "log_of_stratagus_%d.log", ThisPlayer->Player);

	stat(TempPathBuf, &s);
	buf = malloc(s.st_size);
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
		free(buf);
		return;
	}
	fwrite(buf, s.st_size, 1, fd);
	fclose(fd);

	free(buf);
	EndMenu();
	SelectedFileExist = 0;
	ScenSelectFileName[0] = '\0';
	ScenSelectPathName[0] = '\0';
}

/**
** Keystroke help menu
*/
static void KeystrokeHelpMenu(void)
{
	ProcessMenu("menu-keystroke-help", 0);
}

/**
** Help menu
*/
static void HelpMenu(void)
{
	ProcessMenu("menu-help", 0);
}

/**
** Tips menu
*/
static void TipsMenu(void)
{
	ProcessMenu("menu-tips", 0);
}

/**
** Cycle to the next tip
*/
static void TipsCycleNextTip(void)
{
	++CurrentTip;
	if (Tips[CurrentTip] == NULL) {
		CurrentTip = 0;
	}
}

/**
**  Cycle to the previous tip
*/
static void TipsCyclePreviousTip(void)
{
	if (CurrentTip != 0) {
		--CurrentTip;
	} else {
		while (Tips[CurrentTip + 1] != NULL) {
			++CurrentTip;
		}
	}
}

/**
**  Tips menu init callback
*/
static void TipsInit(Menu* menu __attribute__((unused)))
{
}

/**
**  Tips menu exit callback
*/
static void TipsExit(Menu* menu __attribute__((unused)))
{
	TipsCycleNextTip();
	SavePreferences();
}

/**
**  Show tips at startup gem callback
*/
static void TipsShowTipsCheckbox(Menuitem* mi)
{
	if (mi->Menu->Items[1].D.Checkbox.Checked) {
		ShowTips = 1;
	} else {
		ShowTips = 0;
	}
}

/**
**  Show tips at startup text callback
*/
static void TipsShowTipsText(Menuitem* mi)
{
	if (!mi->Menu->Items[1].D.Checkbox.Checked) {
		ShowTips = 1;
		mi->Menu->Items[1].D.Checkbox.Checked = 1;
	} else {
		ShowTips = 0;
		mi->Menu->Items[1].D.Checkbox.Checked = 0;
	}
}

/**
** Tips menu next tip button callback
*/
static void TipsNextTip(void)
{
	TipsCycleNextTip();
}

/**
** Tips menu previous tip button callback
*/
static void TipsPreviousTip(void)
{
	TipsCyclePreviousTip();
}

/**
**  Exit the game from menu.
*/
static void GameMenuExit(void)
{
	ExitMenus();
	Exit(0);
}

/**
**  Game objectives menu
*/
static void ObjectivesMenu(void)
{
	ProcessMenu("menu-objectives", 0);
}

/**
**  Get map info from select path+name
*/
static void GetInfoFromSelectPath(void)
{
	int i;

	if (!ScenSelectFileName[0]) {
		return;
	}
	if (ScenSelectPath[0]) {
		i = strlen(ScenSelectPath);
		strcat(ScenSelectPath, "/");
	} else {
		i = 0;
	}
	strcat(ScenSelectPath, ScenSelectFileName); // Final map name with path
	if (strcasestr(ScenSelectFileName, ".pud")) {
		GetPudInfo(ScenSelectPath, &TheMap.Info);
		strcpy(MenuMapFullPath, ScenSelectPath);
	} else if(strcasestr(ScenSelectFileName, ".smp")) {
		LuaLoadFile(ScenSelectPath);
	}
	ScenSelectPath[i] = '\0'; // Remove appended part
}

/**
** Enter select scenario menu.
*/
static void ScenSelectMenu(void)
{
	Menu* menu;
	int n;
	int j;
	int t;

	ProcessMenu("menu-select-scenario", 1);

	GetInfoFromSelectPath();

	menu = FindMenu("menu-custom-game");
	for (n = j = 0; j < PlayerMax; ++j) {
		t = TheMap.Info.PlayerType[j];
		if (t == PlayerPerson || t == PlayerComputer) {
			n++;
		}
	}
	menu->Items[12].D.Pulldown.noptions = n;
	if (menu->Items[12].D.Pulldown.curopt >= n) {
		menu->Items[12].D.Pulldown.curopt = 0;
	}
}

/**
** Enter multiplayer select scenario menu.
*/
static void MultiScenSelectMenu(void)
{
	Menu* menu;
	unsigned flags;

	// FIXME: remove when cm works with multiplayer
	menu = FindMenu("menu-select-scenario");
	flags = menu->Items[6].Flags;
	menu->Items[6].Flags = MI_FLAGS_DISABLED;

	ScenSelectMenu();
	MultiGamePlayerSelectorsUpdate(1);

	menu->Items[6].Flags = MI_FLAGS_DISABLED;
}

/**
** Enter single player menu.
*/
static void SinglePlayerGameMenu(void)
{
	GuiGameStarted = 0;
	ProcessMenu("menu-custom-game", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

/**
**  Show the campaign select menu.
**
**  Look which campaigns are available and how they are called.
*/
static void CampaignGameMenu(void)
{
#ifdef DEBUG
	int i;
#endif
	Menu* menu;

	menu = FindMenu("menu-campaign-select");
	DebugPrint("%d campaigns available\n" _C_ NumCampaigns);
#ifdef DEBUG
	for (i = 0; i < NumCampaigns; ++i) {
		DebugPrint("Campaign %d: %16.16s: %s\n" _C_ i _C_
			Campaigns[i].Ident _C_
			Campaigns[i].Name);
	}
#endif

	GuiGameStarted = 0;
	ProcessMenu("menu-campaign-select", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

/**
** Start campaign from menu.
**
** @param number Number of the compaign.
*/
static void StartCampaignFromMenu(int number)
{
#if 0
	// JOHNS: this is currently not needed:

	// Any Campaign info should be displayed through a DrawFunc() Item
	// in the CAMPAIN_CONT menu processed below...
	ProcessMenu("menu-campaign-continue", 1);
	// Set GuiGameStarted = 1 to actually run a game here...
	// See CustomGameStart() for info...
#endif

	PlayCampaign(Campaigns[number].Ident);
	GuiGameStarted = 1;

	// FIXME: johns otherwise crash in UpdateDisplay -> DrawMinimapCursor
	EndMenu();
}

/**
** Call back for 1st entry of campaign menu.
**
** @note FIXME: Isn't it possible to have an argument in the menu?
*/
static void CampaignMenu1(void)
{
	StartCampaignFromMenu(0);
}

/**
** Call back for 2nd entry of campaign menu.
**
** @note FIXME: Isn't it possible to have an argument in the menu?
*/
static void CampaignMenu2(void)
{
	StartCampaignFromMenu(1);
}

/**
** Call back for 3rd entry of campaign menu.
**
** @note FIXME: Isn't it possible to have an argument in the menu?
*/
static void CampaignMenu3(void)
{
	StartCampaignFromMenu(2);
}

/**
** Call back for 4th entry of campaign menu.
**
** @note FIXME: Isn't it possible to have an argument in the menu?
*/
static void CampaignMenu4(void)
{
	StartCampaignFromMenu(3);
}

/**
** Call back for select entry of campaign menu.
*/
static void SelectCampaignMenu(void)
{
	// FIXME: not written
}

/**
** Cancel button of player name menu pressed.
*/
static void EnterNameCancel(void)
{
	Menu* menu;

	menu = CurrentMenu;
	menu->Items[1].D.Input.nch = 0;
	EndMenu();
}

/**
** Input field action of player name menu.
*/
static void EnterNameAction(Menuitem* mi, int key)
{
	if (mi->D.Input.nch == 0) {
		mi[1].Flags = MI_FLAGS_DISABLED;
	} else {
		mi[1].Flags &= ~MI_FLAGS_DISABLED;
		if (key == 10 || key == 13) {
			EndMenu();
		}
	}
}

/**
** Cancel button of enter server ip/name menu pressed.
*/
static void EnterServerIPCancel(void)
{
	CurrentMenu->Items[1].D.Input.nch = 0;
	EndMenu();
}

/**
** Input field action of server ip/name.
*/
static void EnterServerIPAction(Menuitem* mi, int key)
{
	if (mi->D.Input.nch == 0) {
		mi[1].Flags = MI_FLAGS_DISABLED;
	} else {
		mi[1].Flags &= ~MI_FLAGS_DISABLED;
		if (key == 10 || key == 13) {
			EndMenu();
		}
	}
}

/**
** Start processing join a network game menu.
*/
static void JoinNetGameMenu(void)
{
	char server_host_buffer[64];
	char *port;
	Menu* menu;

	//
	//  Prepare enter ip/hostname menu
	//
	if (NetworkArg) {
		strncpy(server_host_buffer, NetworkArg, 24);
		server_host_buffer[24] = 0;
		if (NetworkPort != NetworkDefaultPort) {
			strcat(server_host_buffer, ":");
			port = malloc(10);
			sprintf(port, "%d", NetworkPort);
			strcat(server_host_buffer, port);
			free(port);
		}
	} else {
		server_host_buffer[0] = '\0';
	}
	menu = FindMenu("menu-enter-server");
	strcat(server_host_buffer, "~!_");
	menu->Items[1].D.Input.buffer = server_host_buffer;
	menu->Items[1].D.Input.nch = strlen(server_host_buffer) - 3;
	menu->Items[1].D.Input.maxch = 60;
	if (menu->Items[1].D.Input.nch) {
		menu->Items[2].Flags &= ~MI_FLAGS_DISABLED;
	} else {
		menu->Items[2].Flags |= MI_FLAGS_DISABLED;
	}

	ProcessMenu("menu-enter-server", 1);

	if (menu->Items[1].D.Input.nch == 0) {
		return;
	}

	if ( (port = strchr(server_host_buffer, ':')) != NULL) {
		NetworkPort = atoi(port + 1);
		port[0] = 0;
	}

	// Now finally here is the address
	server_host_buffer[menu->Items[1].D.Input.nch] = 0;
	if (NetworkSetupServerAddress(server_host_buffer)) {
		NetErrorMenu("Unable to lookup host.");
		return;
	}
	NetworkInitClientConnect();
	if (!NetConnectRunning) {
		TerminateNetConnect();
		return;
	}

	if (NetworkArg) {
		free(NetworkArg);
	}
	NetworkArg = strdup(server_host_buffer);

	// Here we really go...
	ProcessMenu("menu-net-connecting", 1);

	if (GuiGameStarted) {
		EndMenu();
	}
}

/**
**  Network connect menu init.
*/
static void NetConnectingInit(Menu* menu)
{
	menu->Items[1].D.Text.text = NewStringDesc(NetServerText);
	menu->Items[2].D.Text.text = NewStringDesc(NetTriesText);
}

/**
**  Network connect menu exit.
*/
static void NetConnectingExit(Menu* menu)
{
	FreeStringDesc(menu->Items[1].D.Text.text);
	FreeStringDesc(menu->Items[2].D.Text.text);
	free(menu->Items[1].D.Text.text);
	free(menu->Items[2].D.Text.text);
	menu->Items[1].D.Text.text = NULL;
	menu->Items[2].D.Text.text = NULL;
}

/**
**  Cancel button of network connect menu pressed.
*/
static void NetConnectingCancel(void)
{
	NetworkExitClientConnect();
	// Trigger TerminateNetConnect() to call us again and end the menu
	NetLocalState = ccs_usercanceled;
	EndMenu();
}

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

/**
** Start processing network game setup menu (server).
*/
static void CreateNetGameMenu(void)
{
	GuiGameStarted = 0;
	ProcessMenu("menu-multi-setup", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

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

/**
** Multiplayer game start game button pressed.
*/
static void MultiGameStart(void)
{
	if (MetaServerInUse) {
		SendMetaCommand("StartGame", "");
	}

	GameSettings.Presets[0].Race = SettingsPresetMapDefault;

	NetworkServerStartGame();
	NetworkGamePrepareGameSettings();

	CustomGameStart();
}

/**
** Enter multiplayer game menu.
*/
static void MultiPlayerGameMenu(void)
{
	char NameBuf[32];
	Menu* menu;

	menu = FindMenu("menu-enter-name");
	menu->Items[1].D.Input.buffer = NameBuf;
	strcpy(NameBuf, LocalPlayerName);
	strcat(NameBuf, "~!_");
	menu->Items[1].D.Input.nch = strlen(NameBuf) - 3;
	menu->Items[1].D.Input.maxch = 15;
	menu->Items[2].Flags &= ~MI_FLAGS_DISABLED;

	ProcessMenu("menu-enter-name", 1);

	if (menu->Items[1].D.Input.nch == 0) {
		return;
	}

	NameBuf[menu->Items[1].D.Input.nch] = 0; // Now finally here is the name
	memset(LocalPlayerName, 0, 16);
	strcpy(LocalPlayerName, NameBuf);

	// FIXME: Only save if player name changed
	SavePreferences();

	GuiGameStarted = 0;
	if (!MasterHost) {
		// No Metaserver Configured - Go right to LAN Game.
		ProcessMenu("menu-create-join-menu", 1);
	} else {
		ProcessMenu("menu-multi-net-type-menu", 1);
	}


	DebugPrint("GuiGameStarted: %d\n" _C_ GuiGameStarted);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

/**
** Process LAN/P2P game menu
*/
static void MultiPlayerLANGame(void)
{
	ProcessMenu("menu-create-join-menu", 1);
	MetaServerInUse = 0;
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

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

/**
**  Initialize the scenario selector menu.
*/
static void ScenSelectInit(Menu* menu)
{
	ScenSelectLBInit(menu->Items + 1);
}

/**
**  Exit the scenario selector menu.
*/
static void ScenSelectExit(Menu* menu)
{
	LBExit(menu->Items + 1);
}

/**
**  Scenario select map type action callback
*/
static void ScenSelectTPMSAction(Menuitem* mi, int i __attribute__((unused)))
{
	mi = mi->Menu->Items + 1;
	ScenSelectLBInit(mi);
}

/**
**  Scenario select vertical slider action callback
*/
static void ScenSelectVSAction(Menuitem* mi)
{
}

/**
**  Keystroke help vertical slider action callback
*/
static void KeystrokeHelpVSAction(Menuitem* mi)
{
	int j;

	j = ((mi->D.VSlider.percent + 1) * (nKeyStrokeHelps - 11)) / 100;
	if ((mi->D.VSlider.cflags & MI_CFLAGS_DOWN) && j < nKeyStrokeHelps - 11) {
		++j;
	} else if ((mi->D.VSlider.cflags & MI_CFLAGS_UP) && j > 0) {
		--j;
	}
	mi->D.VSlider.percent = j * 100 / (nKeyStrokeHelps - 11);
}

/**
**  Draw the Keystroke Help texts
*/
static void KeystrokeHelpDrawFunc(Menuitem* mi)
{
	int i;
	int j;

	j = ((mi[-2].D.VSlider.percent + 1) * (nKeyStrokeHelps - 11)) / 100;
	for (i = 0; i < 11; ++i) {
		VideoDrawText(mi->Menu->X + mi->XOfs, mi->Menu->Y + mi->YOfs + (i * 20),
			mi->Font, KeyStrokeHelps[j * 2]);
		VideoDrawText(mi->Menu->X + mi->XOfs + 80, mi->Menu->Y + mi->YOfs + (i * 20),
			mi->Font, KeyStrokeHelps[j * 2 + 1]);
		++j;
	}
}

/**
** Game speed horizontal slider action callback
*/
static void GameSpeedHSAction(Menuitem* mi)
{
	VideoSyncSpeed = (mi->D.HSlider.percent * (MAX_GAME_SPEED - MIN_GAME_SPEED)) / 100 + MIN_GAME_SPEED;
	SetVideoSync();
}

/**
** Mouse scroll horizontal slider action callback
*/
static void MouseScrollHSAction(Menuitem* mi)
{
	TheUI.MouseScroll = 1;
	SpeedMouseScroll = 10 - (mi->D.HSlider.percent * 9) / 100;
	if (mi->D.HSlider.percent == 0) {
		TheUI.MouseScroll = 0;
	}
}

/**
** Keyboard scroll horizontal slider action callback
*/
static void KeyboardScrollHSAction(Menuitem* mi)
{
	TheUI.KeyScroll = 1;
	SpeedKeyScroll = 10 - (mi->D.HSlider.percent * 9) / 100;
	if (mi->D.HSlider.percent == 0) {
		TheUI.KeyScroll = 0;
	}
}

/**
** Master volume horizontal slider action callback
*/
static void MasterVolumeHSAction(Menuitem* mi)
{
	SetGlobalVolume((mi->D.HSlider.percent * 255) / 100);
}

/**
** Music volume horizontal slider action callback
*/
static void MusicVolumeHSAction(Menuitem* mi)
{
	SetMusicVolume((mi->D.HSlider.percent * 255) / 100);
}

#ifdef USE_CDAUDIO
/**
** CD volume horizontal slider action callback
*/
static void CdVolumeHSAction(Menuitem* mi)
{
	SetCDVolume((mi->D.HSlider.percent * 255) / 100);
}
#else
/**
** CD volume horizontal slider action callback
*/
static void CdVolumeHSAction(Menuitem* mi __attribute__((unused)))
{
}
#endif

/**
** Scenario select ok button
*/
static void ScenSelectOk(void)
{
	Menu* menu;
	Menuitem* mi;

	menu = CurrentMenu;
	mi = &menu->Items[1];
	if (ScenSelectPathName[0]) {
		strcat(ScenSelectPath, "/");
		strcat(ScenSelectPath, ScenSelectPathName);
		if (ScenSelectDisplayPath[0]) {
			strcat(ScenSelectDisplayPath, "/");
		}
		strcat(ScenSelectDisplayPath, ScenSelectPathName);
		ScenSelectLBInit(mi);
	} else if (ScenSelectFileName[0]){
		strcpy(CurrentMapPath, ScenSelectPath);
		if (CurrentMapPath[0]) {
			strcat(CurrentMapPath, "/");
		}
		strcat(CurrentMapPath, ScenSelectFileName);
		EndMenu();
	}
}

/**
** Scenario select cancel button.
*/
static void ScenSelectCancel(void)
{
	char *s;

	//
	//  Use last selected map.
	//
	DebugPrint("Map   path: %s\n" _C_ CurrentMapPath);
	strcpy(ScenSelectPath, StratagusLibPath);
	if (*ScenSelectPath) {
		strcat(ScenSelectPath, "/");
	}
	strcat(ScenSelectPath, CurrentMapPath);
	if ((s = strrchr(ScenSelectPath, '/'))) {
		strcpy(ScenSelectFileName, s + 1);
		*s = '\0';
	}
	DebugPrint("Start path: %s\n" _C_ ScenSelectPath);
	EndMenu();
}

/**
** Custom game cancel button callback
*/
static void GameCancel(void)
{
	EndMenu();
}

/**
** Custom game start game button pressed.
*/
static void CustomGameStart(void)
{
	int i;
	char *p;

	if (ScenSelectPath[0]) {
		strcat(ScenSelectPath, "/");
		strcat(ScenSelectPath, ScenSelectFileName); // Final map name with path
		p = ScenSelectPath + strlen(StratagusLibPath) + 1;
		strcpy(CurrentMapPath, p);
	} else {
		strcpy(CurrentMapPath, ScenSelectFileName);
		strcat(ScenSelectPath, ScenSelectFileName); // Final map name with path
	}

	for (i = 0; i < MAX_OBJECTIVES; i++) {
		if (GameIntro.Objectives[i]) {
			free(GameIntro.Objectives[i]);
			GameIntro.Objectives[i] = NULL;
		}
	}
	GameIntro.Objectives[0] = strdup(DefaultObjective);

	GuiGameStarted = 1;
	EndMenu();
}

/**
** Single player custom game menu entered.
*/
static void GameSetupInit(Menu* menu __attribute__ ((unused)))
{
	Menu* custom_menu;
	int n;
	int j;
	int t;
	char *s;

	//
	//  No old path, setup the default.
	//
	if (!*CurrentMapPath || *CurrentMapPath == '.' || *CurrentMapPath == '/') {
		strcpy(CurrentMapPath, DefaultMap);
	}

	DebugPrint("Map   path: %s\n" _C_ CurrentMapPath);
	strcpy(ScenSelectPath, StratagusLibPath);
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

	GetInfoFromSelectPath();

	custom_menu = FindMenu("menu-custom-game");
	for (n = j = 0; j < PlayerMax; ++j) {
		t = TheMap.Info.PlayerType[j];
		if (t == PlayerPerson || t == PlayerComputer) {
			++n;
		}
	}
	custom_menu->Items[12].D.Pulldown.noptions = n;
	if (custom_menu->Items[12].D.Pulldown.curopt >= n) {
		custom_menu->Items[12].D.Pulldown.curopt = 0;
	}
}

/**
** FIXME: docu
*/
static void GameDrawFunc(Menuitem* mi __attribute__((unused)))
{
	char* nc;
	char* rc;
	int l;
	char buffer[32];

	GetDefaultTextColors(&nc, &rc);
	SetDefaultTextColors(rc, rc);
	l = VideoTextLength(GameFont, "Scenario:");
	VideoDrawText(TheUI.Offset640X + 16, TheUI.Offset480Y + 380, GameFont, "Scenario:");
	VideoDrawText(TheUI.Offset640X + 16, TheUI.Offset480Y + 380 + 24 , GameFont, ScenSelectFileName);
	if (TheMap.Info.Description) {
		VideoDrawText(TheUI.Offset640X + 16 + l + 8, TheUI.Offset480Y + 380, GameFont, TheMap.Info.Description);
	}
	sprintf(buffer, " (%d x %d)", TheMap.Info.MapWidth, TheMap.Info.MapHeight);
	VideoDrawText(TheUI.Offset640X + 16 + l + 8 + VideoTextLength(GameFont, ScenSelectFileName), TheUI.Offset480Y + 380 + 24, GameFont, buffer);

#if 0
	for (n = j = 0; j < PlayerMax; j++) {
		if (info->PlayerType[j] == PlayerPerson) {
			n++;
		}
	}
	if (n == 1) {
		VideoDrawText(menu->X+8,menu->Y + 254 + 40,LargeFont,"1 player");
	} else {
		sprintf(buffer, "%d players", n);
		VideoDrawText(menu->X+8,menu->Y + 254 + 40,LargeFont,buffer);
	}
#endif
	SetDefaultTextColors(nc, rc);
}

/**
**  Menu setup race pulldown action.
**
**  @note 0 is default-race.
*/
static void GameRCSAction(Menuitem* mi, int i)
{
	int n;
	int x;

	if (mi->D.Pulldown.curopt == i) {
		for (n = 0, x = 0; n < PlayerRaces.Count; ++n) {
			if (PlayerRaces.Visible[n]) {
				if (x + 1 == i) {
					break;
				}
				++x;
			}
		}
		if (i != 0) {
			GameSettings.Presets[0].Race = PlayerRaces.Race[x];
		} else {
			GameSettings.Presets[0].Race = SettingsPresetMapDefault;
		}
		ServerSetupState.Race[0] = mi->D.Pulldown.noptions - 1 - i;
		NetworkServerResyncClients();
	}
}

/**
** Game resources action callback
*/
static void GameRESAction(Menuitem* mi, int i)
{
	int v[] = { SettingsResourcesMapDefault, SettingsResourcesLow,
				SettingsResourcesMedium, SettingsResourcesHigh };

	if (!mi || mi->D.Pulldown.curopt == i) {
		GameSettings.Resources = v[i];
		ServerSetupState.ResOpt = i;
		if (mi) {
			NetworkServerResyncClients();
		}
	}
}

/**
** Game units action callback
*/
static void GameUNSAction(Menuitem* mi, int i)
{
	if (!mi || mi->D.Pulldown.curopt == i) {
		GameSettings.NumUnits = i ? SettingsNumUnits1 : SettingsNumUnitsMapDefault;
		ServerSetupState.UnsOpt = i;
		if (mi) {
			NetworkServerResyncClients();
		}
	}
}

/**
** Game tilesets action callback
*/
static void GameTSSAction(Menuitem* mi, int i)
{
	if (!mi || mi->D.Pulldown.curopt == i) {
		// Subtract 1 for default option.
		GameSettings.Terrain = i - 1;
		ServerSetupState.TssOpt = i;
		if (mi) {
			NetworkServerResyncClients();
		}
	}
}

/**
** Called if the pulldown menu of the game type is changed.
*/
static void GameGATAction(Menuitem* mi, int i)
{
	if (!mi || mi->D.Pulldown.curopt == i) {
		GameSettings.GameType = i ? SettingsGameTypeMelee + i - 1 : SettingsGameTypeMapDefault;
		ServerSetupState.GaTOpt = i;
		if (mi) {
			NetworkServerResyncClients();
		}
	}
}

/**
** Game opponents action callback
*/
static void CustomGameOPSAction(Menuitem* mi __attribute__((unused)), int i)
{
	GameSettings.Opponents = i ? i : SettingsPresetMapDefault;
}

/**
** Menu setup fog-of-war pulldown action.
*/
static void MultiGameFWSAction(Menuitem* mi, int i)
{
	if (!mi || mi->D.Pulldown.curopt == i) {
		DebugPrint("Update fow %d\n" _C_ i);
		switch (i) {
			case 0:
				TheMap.NoFogOfWar = 0;
				FlagRevealMap = 0;
				GameSettings.NoFogOfWar = 0;
				GameSettings.RevealMap = 0;
				break;
			case 1:
				TheMap.NoFogOfWar = 1;
				FlagRevealMap = 0;
				GameSettings.NoFogOfWar = 1;
				GameSettings.RevealMap = 0;
				break;
			case 2:
				TheMap.NoFogOfWar = 0;
				FlagRevealMap = 1;
				GameSettings.NoFogOfWar = 0;
				GameSettings.RevealMap = 1;
				break;
			case 3:
				TheMap.NoFogOfWar = 1;
				FlagRevealMap = 1;
				GameSettings.NoFogOfWar = 1;
				GameSettings.RevealMap = 1;
				break;
		}
		ServerSetupState.FwsOpt = i;
		if (mi) {
			NetworkServerResyncClients();
		}
	}
}

/**
** Multiplayer menu player server states.
*/
static void MultiGamePTSAction(Menuitem* mi, int o)
{
	Menu* menu;
	int i;

	menu = FindMenu("menu-multi-setup");
	i = mi - menu->Items - SERVER_PLAYER_STATE;
	// JOHNS: Must this be always true?
	// ARI: NO! think of client menus!
	// Assert(i >= 0 && i <= PlayerMax - 1);

	if (i > 0 && i < PlayerMax-1) {
		if (mi->D.Pulldown.curopt == o) {
			if (mi->D.Pulldown.noptions == 2) { // computer slot
				ServerSetupState.CompOpt[i] = o + 1;
			} else {
				ServerSetupState.CompOpt[i] = o;
			}
			MultiGamePlayerSelectorsUpdate(3); // Recalc buttons on server
			NetworkServerResyncClients();
		}
	}

}

/**
** Multiplayer server draw func
*/
static void MultiGameDrawFunc(Menuitem* mi)
{
	GameDrawFunc(mi);
}

/**
** Multiplayer client draw func
*/
static void MultiGameClientDrawFunc(Menuitem* mi)
{
	// FIXME: do something better
	GameDrawFunc(mi);
}

/**
** Multiplayer network game final race an player type setup.
*/
static void NetworkGamePrepareGameSettings(void)
{
	int c;
	int h;
	int i;
	int num[PlayerMax];
	int comp[PlayerMax];
	int n;
	int x;
	int v;

	DebugPrint("NetPlayers = %d\n" _C_ NetPlayers);

	GameSettings.NetGameType=SettingsMultiPlayerGame;

#ifdef DEBUG
	for (i = 0; i < PlayerMax-1; i++) {
		printf("%02d: CO: %d   Race: %d   Host: ", i, ServerSetupState.CompOpt[i], ServerSetupState.Race[i]);
		if (ServerSetupState.CompOpt[i] == 0) {
			for (h = 0; h < NetPlayers; h++) {
				if (Hosts[h].PlyNr == i) {
					printf("%s", Hosts[h].PlyName);
				}
			}
		}
		printf("\n");
	}
#endif

	// Make a list of the available player slots.
	for (c = h = i = 0; i < PlayerMax; i++) {
		if (TheMap.Info.PlayerType[i] == PlayerPerson) {
			num[h++] = i;
		}
		if (TheMap.Info.PlayerType[i] == PlayerComputer) {
			comp[c++] = i; // available computer player slots
		}
	}
	for (i = 0; i < h; i++) {
		switch(ServerSetupState.CompOpt[num[i]]) {
			case 0:
				GameSettings.Presets[num[i]].Type = PlayerPerson;
				for (n = 0, v = 0; n < PlayerRaces.Count; ++n) {
					if (PlayerRaces.Visible[n]) {
						++v;
					}
				}
				v -= ServerSetupState.Race[num[i]];
				for (n = 0, x = 0; n < PlayerRaces.Count; ++n) {
					if (PlayerRaces.Visible[n]) {
						if (x + 1 == v) {
							break;
						}
						++x;
					}
				}
				if (v != 0) {
					GameSettings.Presets[num[i]].Race = PlayerRaces.Race[x];
				}
				break;
			case 1:
				GameSettings.Presets[num[i]].Type = PlayerComputer;
				break;
			case 2:
				GameSettings.Presets[num[i]].Type = PlayerNobody;
			default:
				break;
		}
	}
	for (i = 0; i < c; i++) {
		if (ServerSetupState.CompOpt[comp[i]] == 2) { // closed..
			GameSettings.Presets[comp[i]].Type = PlayerNobody;
			DebugPrint("Settings[%d].Type == Closed\n" _C_ comp[i]);
		}
	}

#ifdef DEBUG
	for (i = 0; i < NetPlayers; i++) {
		Assert(GameSettings.Presets[Hosts[i].PlyNr].Type == PlayerPerson);
		;
	}
#endif
}

/**
** Player selectors have changed.
** Caution: Called by map change (initial = 1)!
** Caution: Called by network events from clients (initial = 2)!
** Caution: Called by button action on server (initial = 3)!
*/
static void MultiGamePlayerSelectorsUpdate(int initial)
{
	Menu* menu;
	int i;
	int h;
	int c;
	int avail;
	int ready;
	int plyrs;

	menu = FindMenu("menu-multi-setup");

	// FIXME: What this has to do:
	// Use lag gem as KICK button
	//  Notify clients about MAP change: (initial = 1...)

	// Calculate available slots from map info
	for (c = h = i = 0; i < PlayerMax; i++) {
		if (TheMap.Info.PlayerType[i] == PlayerPerson) {
			h++; // available interactive player slots
		}
		if (TheMap.Info.PlayerType[i] == PlayerComputer) {
			c++; // available computer player slots
		}
	}

	avail = h;
	plyrs = 0;
	// Setup the player menu
	for (ready = i = 1; i < PlayerMax-1; i++) {
		if (initial == 1) {
			if (i < h) {
				ServerSetupState.CompOpt[i] = 0;
			}
			menu->Items[SERVER_PLAYER_READY - 1 + i].Flags = MI_FLAGS_DISABLED;
			menu->Items[SERVER_PLAYER_LAG - 1 + i].Flags = MI_FLAGS_DISABLED;

			// FIXME: don't forget to throw out additional players
			//   without available slots here!

		}
		if (Hosts[i].PlyNr) {
			menu->Items[SERVER_PLAYER_STATE + i].MiType = 0;
			menu->Items[SERVER_PLAYER_TEXT + i].MiType = MiTypeDrawfunc;

			menu->Items[SERVER_PLAYER_READY - 1 + i].Flags = MI_FLAGS_DISABLED;
			++plyrs;
			if (ServerSetupState.Ready[i]) {
				menu->Items[SERVER_PLAYER_READY - 1 + i].D.Checkbox.Checked = 1;
				++ready;
			}

			menu->Items[SERVER_PLAYER_LAG - 1 + i].Flags = MI_FLAGS_DISABLED;
		} else {
			// don't allow network and button events to intercept server player's action on pulldown buttons!
			if (!(menu->Items[SERVER_PLAYER_STATE + i].Flags & MI_FLAGS_CLICKED)) {
				if (initial == 1 ||
					(initial == 2 && menu->Items[SERVER_PLAYER_STATE + i].MiType != MiTypePulldown)) {
					menu->Items[SERVER_PLAYER_STATE + i].MiType = MiTypePulldown;
					menu->Items[SERVER_PLAYER_TEXT + i].MiType = 0;
					menu->Items[SERVER_PLAYER_STATE + i].Flags = 0;
					menu->Items[SERVER_PLAYER_STATE + i].D.Pulldown.curopt = ServerSetupState.CompOpt[i];
				}
			}
			if (i < h && ServerSetupState.CompOpt[i] != 0) {
				avail--;
			}

			menu->Items[SERVER_PLAYER_READY - 1 + i].Flags = MI_FLAGS_DISABLED;
			menu->Items[SERVER_PLAYER_LAG - 1 + i].Flags = MI_FLAGS_DISABLED;
		}


		if (i >= h) {
			// Allow to switch off (close) preset ai-computer slots
			// FIXME: evaluate game type...
			if (initial == 1 && i < h + c) {
				menu->Items[SERVER_PLAYER_STATE + i].Flags &= ~MI_FLAGS_DISABLED;
				menu->Items[SERVER_PLAYER_STATE + i].D.Pulldown.noptions = 2;
				menu->Items[SERVER_PLAYER_STATE + i].D.Pulldown.options = mgptsoptions + 1;
				menu->Items[SERVER_PLAYER_STATE + i].D.Pulldown.curopt = 0;
				ServerSetupState.CompOpt[i] = 1;
				menu->Items[SERVER_PLAYER_STATE + i].D.Pulldown.curopt = ServerSetupState.CompOpt[i] - 1;
			}

			menu->Items[SERVER_PLAYER_READY - 1 + i].Flags = MI_FLAGS_DISABLED;
			menu->Items[SERVER_PLAYER_LAG - 1 + i].Flags = MI_FLAGS_DISABLED;
		}

		if (i >= h + c) {
			menu->Items[SERVER_PLAYER_STATE + i].D.Pulldown.defopt = 2;
			menu->Items[SERVER_PLAYER_STATE + i].D.Pulldown.curopt = 2;
			menu->Items[SERVER_PLAYER_STATE + i].Flags = MI_FLAGS_DISABLED;
		}
	}


	// Tell connect state machines how many interactive players we can have
	NetPlayers = avail;
	// Check if all players are ready.
	DebugPrint("READY to START: AVAIL = %d, READY = %d\n" _C_ avail
			_C_ ready);

	// Disable the select scenario after players have joined.
	if (plyrs) {
		// disable Select Scenario button
		menu->Items[2].Flags = MI_FLAGS_DISABLED;
	} else {
		// enable Select Scenario button
		menu->Items[2].Flags = 0;
	}
	if (ready == avail) {
		if (menu->Items[3].Flags == MI_FLAGS_DISABLED) {
			// enable start game button
			menu->Items[3].Flags = 0;
		}
	} else {
		// disable start game button
		menu->Items[3].Flags = MI_FLAGS_DISABLED;
	}

	if (MetaServerInUse) {
		ChangeGameServer();
	}
}

/**
** Update client network menu.
*/
static void MultiClientUpdate(int initial)
{
	Menu* menu;
	int i;
	int h;
	int c;

	menu = FindMenu("menu-net-multi-client");

	//  Calculate available slots from map info
	for (c = h = i = 0; i < PlayerMax; i++) {
		if (TheMap.Info.PlayerType[i] == PlayerPerson) {
			h++; // available interactive player slots
		}
		if (TheMap.Info.PlayerType[i] == PlayerComputer) {
			c++; // available computer player slots
		}
	}

	//
	// Setup defaults, reset values.
	//
	if (initial) {
		menu->Items[CLIENT_PLAYER_STATE].MiType = 0;
		menu->Items[CLIENT_PLAYER_TEXT].MiType = MiTypeDrawfunc;
		memset(&ServerSetupState, 0, sizeof(ServerSetup));
		memset(&LocalSetupState, 0, sizeof(ServerSetup));
	}
	for (i = 1; i < PlayerMax - 1; i++) {
		//
		// Johns: This works only if initial. Hosts[i].PlyNr is later lost.
		//
		if (Hosts[i].PlyNr || i == NetLocalHostsSlot) {
			menu->Items[CLIENT_PLAYER_STATE + i].MiType = 0;
			menu->Items[CLIENT_PLAYER_TEXT + i].MiType = MiTypeDrawfunc;
			if (i == NetLocalHostsSlot) {
				menu->Items[CLIENT_PLAYER_READY - 1 + i].Flags = MI_FLAGS_DISABLED;
			} else {
				menu->Items[CLIENT_PLAYER_READY - 1 + i].Flags = 0;
			}
		} else {
			menu->Items[CLIENT_PLAYER_STATE + i].MiType = MiTypePulldown;
			menu->Items[CLIENT_PLAYER_TEXT + i].MiType = 0;
			menu->Items[CLIENT_PLAYER_STATE + i].Flags = MI_FLAGS_DISABLED;
			menu->Items[CLIENT_PLAYER_STATE + i].D.Pulldown.curopt =
				ServerSetupState.CompOpt[i];
			menu->Items[CLIENT_PLAYER_READY - 1 + i].Flags = MI_FLAGS_INVISIBLE;
		}

		menu->Items[CLIENT_PLAYER_READY - 1 + i].D.Checkbox.Checked = !!ServerSetupState.Ready[i];

#if 0
		if (i != NetLocalHostsSlot) {
		//if (i >= h) {
			menu->Items[CLIENT_PLAYER_STATE + i].D.Pulldown.curopt =
				ServerSetupState.CompOpt[i];
		}
#endif

		// Unused slots are always disabled.
		if (i >= h + c) {
			menu->Items[CLIENT_PLAYER_READY - 1 + i].Flags =
				MI_FLAGS_INVISIBLE;
			menu->Items[CLIENT_PLAYER_STATE + i].D.Pulldown.defopt =
				menu->Items[CLIENT_PLAYER_STATE + i].D.Pulldown.curopt = 2;
			menu->Items[CLIENT_PLAYER_STATE + i].Flags = MI_FLAGS_DISABLED;
		}
	}
}

/**
**  Multiplayer server menu init callback
*/
static void MultiGameSetupInit(Menu* menu)
{
	int i;
	int h;

	// FIXME: Remove this when .cm is supported
	if (*CurrentMapPath && strstr(CurrentMapPath, ".cm\0")) {
		*CurrentMapPath = '\0';
	}

	GameSetupInit(menu);
	NetworkInitServerConnect();
	menu->Items[SERVER_PLAYER_STATE].MiType = 0;
	MultiGameFWSAction(NULL, menu->Items[27].D.Pulldown.defopt);

	memset(&ServerSetupState, 0, sizeof(ServerSetup));
	// Calculate available slots from map info
	for (h = i = 0; i < PlayerMax; i++) {
		if (TheMap.Info.PlayerType[i] == PlayerPerson) {
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

/**
**  Multiplayer server menu exit callback
*/
static void MultiGameSetupExit(Menu* menu __attribute__((unused)))
{
}

/**
**  Cancel button of server multi player menu pressed.
*/
static void MultiGameCancel(void)
{
	NetworkExitServerConnect();

	if (MetaServerInUse) {
		SendMetaCommand("AbandonGame", "");
	}

	NetPlayers = 0; // Make single player menus work again!
	GameCancel();
}

/**
**  Draw the multi player setup menu.
*/
static void NetMultiPlayerDrawFunc(Menuitem* mi)
{
	Menu* menu;
	int i;
	char* nc;
	char* rc;

	menu = FindMenu("menu-multi-setup");
	i = mi - menu->Items - SERVER_PLAYER_TEXT;
	if (i >= 0 && i < PlayerMax - 1) { // Ugly test to detect server
		if (i > 0) {
			menu->Items[SERVER_PLAYER_READY - 1 + i].Flags &=
				~MI_FLAGS_DISABLED;
			// Note: re-disabled in MultiGamePlayerSelectorsUpdate()
			// for kicked out clients!!
			if (ServerSetupState.Ready[i]) {
				menu->Items[SERVER_PLAYER_READY - 1 + i].Flags |= MI_FLAGS_DISABLED;
				menu->Items[SERVER_PLAYER_READY - 1 + i].D.Checkbox.Checked = 1;
			} else {
				menu->Items[SERVER_PLAYER_READY - 1 + i].Flags |= MI_FLAGS_DISABLED;
				menu->Items[SERVER_PLAYER_READY - 1 + i].D.Checkbox.Checked = 0;
			}
			if (ServerSetupState.LastFrame[i] + 30 > FrameCounter) {
				menu->Items[SERVER_PLAYER_LAG - 1 + i].Flags |= MI_FLAGS_DISABLED;
				menu->Items[SERVER_PLAYER_LAG - 1 + i].D.Checkbox.Checked = 1;
			} else {
				menu->Items[SERVER_PLAYER_LAG - 1 + i].Flags |= MI_FLAGS_DISABLED;
			}

		}
	} else {
		menu = FindMenu("menu-net-multi-client");
		i = mi - menu->Items - CLIENT_PLAYER_TEXT;
		if (i > 0) {
			menu->Items[CLIENT_PLAYER_READY - 1 + i].Flags &=
				~MI_FLAGS_DISABLED;
			if (i == NetLocalHostsSlot) {
				menu->Items[CLIENT_PLAYER_READY - 1 + i].Flags &= ~MI_FLAGS_DISABLED;
			} else {
				menu->Items[CLIENT_PLAYER_READY - 1 + i].Flags |= MI_FLAGS_DISABLED;
			}
		}
	}

	GetDefaultTextColors(&nc, &rc);
	SetDefaultTextColors(rc, rc);
	VideoDrawText(TheUI.Offset640X + mi->XOfs, TheUI.Offset480Y + mi->YOfs, GameFont, Hosts[i].PlyName);

	SetDefaultTextColors(nc, rc);
}

/**
**  Cancel button of multiplayer client menu pressed.
*/
static void MultiClientCancel(void)
{
	NetworkDetachFromServer();
	// GameCancel();
}

/**
**  Multiplayer client menu init callback
*/
static void MultiGameClientInit(Menu* menu)
{
	// GameSetupInit(mi);
	MultiClientUpdate(1);
	if (LocalSetupState.Ready[NetLocalHostsSlot]) {
		menu->Items[2].Flags = MI_FLAGS_DISABLED;
		menu->Items[3].Flags = 0;
		menu->Items[CLIENT_RACE].Flags = MI_FLAGS_DISABLED;
	} else {
		menu->Items[3].Flags = MI_FLAGS_DISABLED;
		menu->Items[2].Flags = 0;
		menu->Items[CLIENT_RACE].Flags = 0;
	}
}

/**
**  Multiplayer client menu exit callback
*/
static void MultiGameClientExit(Menu* menu __attribute__((unused)))
{
}

/**
**  Multiplayer client gem action. Toggles ready flag.
*/
static void MultiClientCheckboxAction(Menuitem* mi)
{
	int i;

	i = mi - mi->Menu->Items - CLIENT_PLAYER_READY + 1;
	if (i == NetLocalHostsSlot) {
		LocalSetupState.Ready[i] = !LocalSetupState.Ready[i];
		if (LocalSetupState.Ready[i]) {
			mi->Menu->Items[2].Flags = MI_FLAGS_DISABLED;
			mi->Menu->Items[3].Flags = 0;
			mi->Menu->Items[CLIENT_RACE].Flags = MI_FLAGS_DISABLED;
		} else {
			mi->Menu->Items[3].Flags = MI_FLAGS_DISABLED;
			mi->Menu->Items[2].Flags = 0;
			mi->Menu->Items[CLIENT_RACE].Flags = 0;
		}
		MultiClientUpdate(0);
	}
}

/**
**  Multiplayer client races action callback
*/
static void MultiClientRCSAction(Menuitem* mi, int i)
{
	if (mi->D.Pulldown.curopt == i) {
		LocalSetupState.Race[NetLocalHostsSlot] = mi->D.Pulldown.noptions - 1 - i;
		MultiClientUpdate(0);
	}
}

/**
** Multiplayer client ready button
*/
static void MultiClientReady(void)
{
	Menu* menu;

	menu = FindMenu("menu-net-multi-client");
	menu->Items[2].Flags = MI_FLAGS_DISABLED;
	menu->Items[3].Flags = 0;
	menu->Items[CLIENT_RACE].Flags = MI_FLAGS_DISABLED;
	LocalSetupState.Ready[NetLocalHostsSlot] = 1;
	MultiClientUpdate(0);
}

/**
** Multiplayer client not ready button
*/
static void MultiClientNotReady(void)
{
	Menu* menu;

	menu = FindMenu("menu-net-multi-client");
	menu->Items[3].Flags = MI_FLAGS_DISABLED;
	menu->Items[2].Flags = 0;
	menu->Items[CLIENT_RACE].Flags = 0;
	LocalSetupState.Ready[NetLocalHostsSlot] = 0;
	MultiClientUpdate(0);
}

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
	char *cp;

	cp = strrchr(MenuMapFullPath, '/');
	if (cp) {
		strcpy(ScenSelectFileName, cp + 1);
		*cp = 0;
		strcpy(ScenSelectPath, MenuMapFullPath);
		*cp = '/';
	} else {
		strcpy(ScenSelectFileName, MenuMapFullPath);
		ScenSelectPath[0] = 0;
	}

	FreeMapInfo(&TheMap.Info);
	if (strcasestr(ScenSelectFileName, ".pud")) {
		GetPudInfo(MenuMapFullPath, &TheMap.Info);
	} else {
		LuaLoadFile(MenuMapFullPath);
	}
	return 0;
}

/**
** FIXME: docu
*/
void NetConnectForceDisplayUpdate(void)
{
	MultiGamePlayerSelectorsUpdate(2);
}

/**
** Update client menu to follow server menu.
*/
void NetClientUpdateState(void)
{
	Menu* menu;

	menu = FindMenu("menu-net-multi-client");

	GameRESAction(NULL, ServerSetupState.ResOpt);
	menu->Items[CLIENT_RESOURCE].D.Pulldown.curopt =
		ServerSetupState.ResOpt;

	GameUNSAction(NULL, ServerSetupState.UnsOpt);
	menu->Items[CLIENT_UNITS].D.Pulldown.curopt =
		ServerSetupState.UnsOpt;

	MultiGameFWSAction(NULL, ServerSetupState.FwsOpt);
	menu->Items[CLIENT_FOG_OF_WAR].D.Pulldown.curopt =
		ServerSetupState.FwsOpt;

	GameTSSAction(NULL, ServerSetupState.TssOpt);
	menu->Items[CLIENT_TILESET].D.Pulldown.curopt =
		ServerSetupState.TssOpt;

	GameGATAction(NULL, ServerSetupState.GaTOpt);
	menu->Items[CLIENT_GAMETYPE].D.Pulldown.curopt =
		ServerSetupState.GaTOpt;

	MultiClientUpdate(0);
	DebugPrint("MultiClientMenuRedraw\n");
}

/**
** Start editor.
*/
static void StartEditor(void)
{
	SetupEditor();

	EditorRunning = EditorStarted;
	EndMenu();
}

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
	strcpy(ScenSelectPath, StratagusLibPath);
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


/**
** Editor select cancel button callback
*/
static void EditorSelectCancel(void)
{
	QuitToMenu = 1;
	EditorRunning = EditorNotRunning;
	EndMenu();
}

/**
** Called from menu, for new editor map.
*/
static void EditorNewMap(void)
{
	Menu* menu;
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
	menu->Items[7].D.Pulldown.noptions = NumTilesets;
	ProcessMenu("menu-editor-new", 1);

	if (EditorCancelled) {
		return;
	}

	description[strlen(description) - 3] = '\0';
	TheMap.Info.Description = strdup(description);
	TheMap.Info.MapTerrain = menu->Items[7].D.Pulldown.curopt;
	TheMap.Info.MapWidth = atoi(width);
	TheMap.Info.MapHeight = atoi(height);

	VideoClearScreen();

	*CurrentMapPath = '\0';

	GuiGameStarted = 1;
	EndMenu();
}

/**
** Editor new map draw func
*/
static void EditorNewDrawFunc(Menuitem* mi __attribute__((unused)))
{
}

/**
** Editor new map, map description input box callback
*/
static void EditorNewMapDescriptionEnterAction(
		Menuitem* mi __attribute__((unused)), int key __attribute__((unused)))
{
}

/**
** Editor new map, size input box callback
*/
static void EditorNewMapSizeEnterAction(Menuitem * mi,
		int key __attribute__((unused)))
{
	if (mi->D.Input.nch > 0
			&& !isdigit(mi->D.Input.buffer[mi->D.Input.nch - 1])) {
		strcpy(mi->D.Input.buffer + (--mi->D.Input.nch), "~!_");
	}
}

/**
** Editor new map ok button
*/
static void EditorNewOk(void)
{
	Menu* menu;
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
		EndMenu();
	}
}

/**
** Editor new map cancel button
*/
static void EditorNewCancel(void)
{
	EditorCancelled = 1;
	EndMenu();
}

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

	VideoClearScreen();

	if (ScenSelectPath[0]) {
		s = ScenSelectPath + strlen(ScenSelectPath);
		*s = '/';
		strcpy(s+1, ScenSelectFileName); // Final map name with path
		p = ScenSelectPath + strlen(StratagusLibPath) + 1;
		strcpy(CurrentMapPath, p);
		*s = '\0';
	} else {
		strcpy(CurrentMapPath, ScenSelectFileName);
	}

	GuiGameStarted = 1;
	EndMenu();
}

/**
** Editor main load init callback
*/
static void EditorMainLoadInit(Menu* menu)
{
	EditorMainLoadLBInit(menu->Items + 1);
}

/**
** Editor main load exit callback
*/
static void EditorMainLoadExit(Menu* menu)
{
	LBExit(menu->Items + 1);
}

/**
** Editor main load ok button
*/
static void EditorMainLoadOk(void)
{
	Menu* menu;
	Menuitem* mi;

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
		EndMenu();
	}
}

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
	strcpy(ScenSelectPath, StratagusLibPath);
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

	EndMenu();
}

/**
** Editor main load vertical slider action callback
*/
static void EditorMainLoadVSAction(Menuitem* mi)
{
}

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

	VideoClearScreen();

	if (ScenSelectPath[0]) {
		s = ScenSelectPath + strlen(ScenSelectPath);
		*s = '/';
		strcpy(s + 1, ScenSelectFileName); // Final map name with path
		p = ScenSelectPath + strlen(StratagusLibPath) + 1;
		strcpy(CurrentMapPath, p);
		*s = '\0';
	} else {
		strcpy(CurrentMapPath, ScenSelectFileName);
	}

	EditorMapLoaded = 1;
	EditorRunning = EditorNotRunning;
	EndMenu();
}

/**
** Editor main load ok button
*/
static void EditorLoadOk(void)
{
	Menu* menu;
	Menuitem* mi;

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
		EndMenu();
	}
}

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
	strcpy(ScenSelectPath, StratagusLibPath);
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

/**
** Editor map properties menu
*/
static void EditorMapPropertiesMenu(void)
{
	Menu* menu;
	char description[36];
	char size[30];

	menu = FindMenu("menu-editor-map-properties");

	menu->Items[2].D.Input.buffer = description;
	strcpy(description, TheMap.Info.Description);
	strcat(description, "~!_");
	menu->Items[2].D.Input.nch = strlen(description) - 3;
	menu->Items[2].D.Input.maxch = 31;

	sprintf(size, "%d x %d", TheMap.Info.MapWidth, TheMap.Info.MapHeight);
	menu->Items[4].D.Text.text = NewStringDesc(size);

	menu->Items[6].D.Pulldown.defopt = TheMap.Terrain;

	// FIXME: Set the correct pud version
	menu->Items[8].D.Pulldown.defopt = 1;
	menu->Items[8].Flags = -1;

	ProcessMenu("menu-editor-map-properties", 1);
	FreeStringDesc(menu->Items[4].D.Text.text);
	free(menu->Items[4].D.Text.text);
	menu->Items[4].D.Text.text = NULL;
}

/**
** Editor map properties input box callback
*/
static void EditorMapPropertiesEnterAction(
		Menuitem* mi __attribute__((unused)), int key __attribute__((unused)))
{
}

/**
** Editor map properties ok button
*/
static void EditorMapPropertiesOk(void)
{
	Menu* menu;
	char *description;
	int old;

	menu = CurrentMenu;

	description = menu->Items[2].D.Input.buffer;
	description[strlen(description)-3] = '\0';
	free(TheMap.Info.Description);
	TheMap.Info.Description = strdup(description);

	// Change the terrain
	old = TheMap.Info.MapTerrain;
	if (old != menu->Items[6].D.Pulldown.curopt) {
		TheMap.Info.MapTerrain = menu->Items[6].D.Pulldown.curopt;
		free(TheMap.Info.MapTerrainName);
		TheMap.Info.MapTerrainName = strdup(TilesetWcNames[TheMap.Info.MapTerrain]);
		TheMap.Terrain = TheMap.Info.MapTerrain;
		free(TheMap.TerrainName);
		TheMap.TerrainName = strdup(TilesetWcNames[TheMap.Info.MapTerrain]);
		TheMap.Tileset = Tilesets[TheMap.Info.MapTerrain];

		LoadTileset();
		ChangeTilesetPud(old, &TheMap);
		SetPlayersPalette();
		PreprocessMap();
		LoadConstructions();
		LoadUnitTypes();
		LoadIcons();
		UpdateMinimapTerrain();
	}


	EditorEndMenu();
}

/**
** Editor player properties draw func
*/
static void EditorPlayerPropertiesDrawFunc(Menuitem* mi __attribute__((unused)))
{
}

/**
** Editor player properties input box callback
*/
static void EditorPlayerPropertiesEnterAction(Menuitem* mi,
		int key __attribute__((unused)))
{
	if (mi->D.Input.nch > 0 && !isdigit(mi->D.Input.buffer[mi->D.Input.nch - 1])) {
		strcpy(mi->D.Input.buffer + (--mi->D.Input.nch), "~!_");
	}
}

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

	/// Player type conversion from menu to internal fc number
static int PlayerTypesMenuToFc[] = {
	PlayerPerson,
	PlayerComputer,
	PlayerRescuePassive,
	PlayerRescueActive,
	PlayerNeutral,
	PlayerNobody,
};

/**
** Convert player ai from internal fc number to menu number
**
** @param num Ai number
*/
static int PlayerAiFcToMenu(int num)
{
	if (num == PlayerAiLand) {
		return 0;
	} else if (num == PlayerAiPassive) {
		return 1;
	} else if (num == PlayerAiSea) {
		return 2;
	} else if (num == PlayerAiAir) {
		return 3;
	}
	DebugPrint("Invalid Ai number: %d\n" _C_ num);
	return -1;
}

/**
** Convert player ai from menu number to internal fc number
**
** @param num Ai number
*/
static int PlayerAiMenuToFc(int num)
{
	if (num == 0) {
		return PlayerAiLand;
	} else if (num == 1) {
		return PlayerAiPassive;
	} else if (num == 2) {
		return PlayerAiSea;
	} else if (num == 3) {
		return PlayerAiAir;
	}
	DebugPrint("Invalid Ai number: %d\n" _C_ num);
	return -1;
}

/**
** Edit player properties menu
*/
static void EditorPlayerPropertiesMenu(void)
{
	Menu* menu;
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
		menu->Items[RACE_POSITION + i].D.Pulldown.defopt = TheMap.Info.PlayerSide[i];
		menu->Items[TYPE_POSITION + i].D.Pulldown.defopt = PlayerTypesFcToMenu[TheMap.Info.PlayerType[i]];
		menu->Items[AI_POSITION + i].D.Pulldown.defopt = PlayerAiFcToMenu(TheMap.Info.PlayerAi[i]);
		sprintf(gold[i], "%d~!_", TheMap.Info.PlayerResources[i][GoldCost]);
		sprintf(lumber[i], "%d~!_", TheMap.Info.PlayerResources[i][WoodCost]);
		sprintf(oil[i], "%d~!_", TheMap.Info.PlayerResources[i][OilCost]);
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
		TheMap.Info.PlayerSide[i] = menu->Items[RACE_POSITION + i].D.Pulldown.curopt;
		TheMap.Info.PlayerType[i] = PlayerTypesMenuToFc[menu->Items[TYPE_POSITION + i].D.Pulldown.curopt];
		TheMap.Info.PlayerAi[i] = PlayerAiMenuToFc(menu->Items[AI_POSITION + i].D.Pulldown.curopt);
		TheMap.Info.PlayerResources[i][GoldCost] = atoi(gold[i]);
		TheMap.Info.PlayerResources[i][WoodCost] = atoi(lumber[i]);
		TheMap.Info.PlayerResources[i][OilCost] = atoi(oil[i]);
	}
}

/**
** Edit resource properties
*/
void EditorEditResource(void)
{
	Menu* menu;
	char buf[13];
	char buf2[32];

	menu = FindMenu("menu-editor-edit-resource");

	sprintf(buf2, "Amount of %s:", DefaultResourceNames[UnitUnderCursor->Type->GivesResource]);
	menu->Items[0].D.Text.text = NewStringDesc(buf2);
	sprintf(buf, "%d~!_", UnitUnderCursor->ResourcesHeld);
	menu->Items[1].D.Input.buffer = buf;
	menu->Items[1].D.Input.nch = strlen(buf) - 3;
	menu->Items[1].D.Input.maxch = 6;
	ProcessMenu("menu-editor-edit-resource", 1);
	FreeStringDesc(menu->Items[0].D.Text.text);
	free(menu->Items[0].D.Text.text);
	menu->Items[0].D.Text.text = NULL;
}

/**
** Key pressed in menu-editor-edit-resource
*/
static void EditorEditResourceEnterAction(Menuitem* mi,int key)
{
	if (mi->D.Input.nch > 0 && !isdigit(mi->D.Input.buffer[mi->D.Input.nch - 1])) {
		strcpy(mi->D.Input.buffer + (--mi->D.Input.nch), "~!_");
	} else if (key==10 || key==13) {
		EditorEditResourceOk();
	}
}

/**
** Ok button from menu-editor-edit-resource
*/
static void EditorEditResourceOk(void)
{
	Menu* menu;
	unsigned value;

	menu = CurrentMenu;
	value = atoi(menu->Items[1].D.Input.buffer);
	if (value < 2500) {
		strcpy(menu->Items[1].D.Input.buffer, "2500~!_");
		menu->Items[1].D.Input.nch = 4;
		menu = FindMenu("menu-editor-error");
		menu->Items[1].D.Text.text = NewStringDesc("Must be greater than 2500");
	} else if (value > 655000) {
		strcpy(menu->Items[1].D.Input.buffer, "655000~!_");
		menu->Items[1].D.Input.nch = 6;
		menu = FindMenu("menu-editor-error");
		menu->Items[1].D.Text.text = NewStringDesc("Must be smaller than 655000");
	} else if (value / 2500 * 2500 != value) {
		value = (value + 1250)/ 2500 * 2500;
		sprintf(menu->Items[1].D.Input.buffer, "%d~!_", value);
		menu->Items[1].D.Input.nch = strlen(menu->Items[1].D.Input.buffer) - 3;
		menu = FindMenu("menu-editor-error");
		menu->Items[1].D.Text.text = NewStringDesc("Must be a multiple of 2500");
	} else {
		UnitUnderCursor->ResourcesHeld = value;
		GameMenuReturn();
		return;
	}
	ProcessMenu("menu-editor-error", 1);
	FreeStringDesc(menu->Items[1].D.Text.text);
	free(menu->Items[1].D.Text.text);
	menu->Items[1].D.Text.text = NULL;
}

/**
** Cancel button from menu-editor-edit-resource
*/
static void EditorEditResourceCancel(void)
{
	GameMenuReturn();
}

/**
** Edit ai properties
*/
void EditorEditAiProperties(void)
{
	Menu* menu;

	menu = FindMenu("menu-editor-edit-ai-properties");
	if (UnitUnderCursor->Active) {
		menu->Items[1].D.Checkbox.Checked = 1;
		menu->Items[3].D.Checkbox.Checked = 0;
	} else {
		menu->Items[1].D.Checkbox.Checked = 0;
		menu->Items[3].D.Checkbox.Checked = 1;
	}

	ProcessMenu("menu-editor-edit-ai-properties", 1);
}

/**
** Active or Passive gem clicked in menu-editor-edit-ai-properties
*/
static void EditorEditAiPropertiesCheckbox(Menuitem* mi)
{
	if (&mi->Menu->Items[1] == mi) {
		mi->D.Checkbox.Checked = 1;
		mi->Menu->Items[3].D.Checkbox.Checked = 0;
	} else {
		mi->D.Checkbox.Checked = 1;
		mi->Menu->Items[1].D.Checkbox.Checked = 0;
	}
}

/**
** Ok button from menu-editor-edit-ai-properties
*/
static void EditorEditAiPropertiesOk(void)
{
	Menu* menu;

	menu = CurrentMenu;
	if (menu->Items[1].D.Checkbox.Checked) {
		UnitUnderCursor->Active = 1;
	} else {
		UnitUnderCursor->Active = 0;
	}
	GameMenuReturn();
}

/**
** Cancel button from menu-editor-edit-ai-properties
*/
static void EditorEditAiPropertiesCancel(void)
{
	GameMenuReturn();
}

/**
** Save map from the editor
**
** @return 0 for success, -1 for error
*/
int EditorSaveMenu(void)
{
	Menu* menu;
	char path[PATH_MAX];
	char *s;
	char *p;
	int ret;

	ret = 0;
	menu = FindMenu("menu-editor-save");

	EditorCancelled = 0;

	menu->Items[3].D.Input.buffer = path;
	menu->Items[3].D.Input.maxch = PATH_MAX - 4;

	ProcessMenu("menu-editor-save", 1);

	if (!EditorCancelled) {
		sprintf(path, "%s/%s.gz", ScenSelectPath, ScenSelectFileName);
		if (EditorSavePud(path) == -1) {
			ret = -1;
		}
		s = ScenSelectPath + strlen(ScenSelectPath);
		*s = '/';
		strcpy(s + 1, ScenSelectFileName); // Final map name with path
		p = ScenSelectPath + strlen(StratagusLibPath) + 1;
		strcpy(CurrentMapPath, p);
		*s = '\0';
	}
	return ret;
}

/**
**  Editor save init callback
*/
static void EditorSaveInit(Menu* menu)
{
	EditorSaveLBInit(menu->Items + 1);
}

/**
**  Editor save exit callback
*/
static void EditorSaveExit(Menu* menu)
{
	LBExit(menu->Items + 1);
}

/**
** Editor save ok button
*/
static void EditorSaveOk(void)
{
	Menu* menu;
	Menuitem* mi;
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
			if (!strcasestr(ScenSelectFileName, ".pud")) {
				strcat(ScenSelectFileName, ".pud");
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

/**
** Editor save cancel button
*/
static void EditorSaveCancel(void)
{
	EditorCancelled = 1;
	EditorEndMenu();
}

/**
** Editor save vertical slider action callback
*/
static void EditorSaveVSAction(Menuitem* mi)
{
}

/**
** Editor save input callback
*/
static void EditorSaveEnterAction(Menuitem* mi, int key)
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

/**
** Editor save confirm init callback
*/
static void EditorSaveConfirmInit(Menu* menu)
{
	menu->Items[2].D.Text.text = NewStringDesc(ScenSelectFileName);
}

/**
** Editor save confirm ok button
*/
static void EditorSaveConfirmOk(void)
{
	FreeStringDesc(CurrentMenu->Items[2].D.Text.text);
	free(CurrentMenu->Items[2].D.Text.text);
	CurrentMenu->Items[2].D.Text.text = NULL;
	EditorEndMenu();
}

/**
** Editor save confirm cancel button
*/
static void EditorSaveConfirmCancel(void)
{
	FreeStringDesc(CurrentMenu->Items[2].D.Text.text);
	free(CurrentMenu->Items[2].D.Text.text);
	CurrentMenu->Items[2].D.Text.text = NULL;
	EditorCancelled = 1;
	EditorEndMenu();
	SelectedFileExist = 0;
	ScenSelectFileName[0] = '\0';
	ScenSelectPathName[0] = '\0';
}

/**
** Called from menu, to quit editor to menu.
**
** @todo Should check if modified file should be saved.
*/
static void EditorQuitToMenu(void)
{
	QuitToMenu = 1;
	EditorRunning = EditorNotRunning;
	EndMenu();
	SelectedFileExist = 0;
	ScenSelectFileName[0] = '\0';
	ScenSelectPathName[0] = '\0';
}

/**
** End menus state of the editor.
*/
static void EditorEndMenu(void)
{
	CursorOn = CursorOnUnknown;
	CurrentMenu = NULL;

	InterfaceState = IfaceStateNormal;
	EditorUpdateDisplay();
	InterfaceState = IfaceStateMenu;
}

/**
** Replay game menu
*/
static void ReplayGameMenu(void)
{
#ifdef USE_WIN32
	strcpy(TempPathBuf, GameName);
	mkdir(TempPathBuf);
	strcat(TempPathBuf, "/logs");
	mkdir(TempPathBuf);

	sprintf(ScenSelectPath, "%s/logs", GameName);
#else
	sprintf(TempPathBuf,"%s/%s", getenv("HOME"), STRATAGUS_HOME_PATH);
	mkdir(TempPathBuf, 0777);
	strcat(TempPathBuf, "/");
	strcat(TempPathBuf, GameName);
	mkdir(TempPathBuf, 0777);
	strcat(TempPathBuf, "/logs");
	mkdir(TempPathBuf, 0777);

	sprintf(ScenSelectPath, "%s/%s/%s/logs", getenv("HOME"), STRATAGUS_HOME_PATH, GameName);
#endif
	*ScenSelectDisplayPath = '\0';

	GuiGameStarted = 0;
	ProcessMenu("menu-replay-game", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

/**
**  Replay game menu init callback
*/
static void ReplayGameInit(Menu* menu)
{
	Assert(*ScenSelectPath);

	menu->Items[6].D.Checkbox.Checked = 0;
	DebugPrint("Start path: %s\n" _C_ ScenSelectPath);
	ReplayGameLBInit(menu->Items + 1);
}

/**
**  Replay game menu exit callback
*/
static void ReplayGameExit(Menu* menu)
{
	LBExit(menu->Items + 1);
}

/**
**  Replay game vertical slider action
*/
static void ReplayGameVSAction(Menuitem* mi)
{
}

/**
** Replay game disable fog gem callback
*/
static void ReplayGameDisableFog(Menuitem* mi __attribute__((unused)))
{
}

/**
** Replay game ok button callback
*/
static void ReplayGameOk(void)
{
	Menu* menu;
	Menuitem* mi;
	int i;

	menu = CurrentMenu;
	mi = &menu->Items[1];
	if (ScenSelectPathName[0]) {
		strcat(ScenSelectPath, "/");
		strcat(ScenSelectPath, ScenSelectPathName);
		if (ScenSelectDisplayPath[0]) {
			strcat(ScenSelectDisplayPath, "/");
		}
		strcat(ScenSelectDisplayPath, ScenSelectPathName);
		ReplayGameLBInit(mi);
	} else if (ScenSelectFileName[0]) {
		if (ScenSelectPath[0]) {
			strcat(ScenSelectPath, "/");
			strcat(ScenSelectPath, ScenSelectFileName);
		} else {
			strcpy(ScenSelectPath, ScenSelectFileName);
		}

		LoadReplay(ScenSelectPath);

		for (i = 0; i < MAX_OBJECTIVES; i++) {
			free(GameIntro.Objectives[i]);
			GameIntro.Objectives[i] = NULL;
		}
		GameIntro.Objectives[0] = strdup(DefaultObjective);

		GuiGameStarted = 1;
		EndMenu();

		if (menu->Items[6].D.Checkbox.Checked) {
			ReplayRevealMap = 1;
		} else {
			ReplayRevealMap = 0;
		}
	}
}

/**
** Replay game cancel button callback
*/
static void ReplayGameCancel(void)
{
	char *s;

	//
	//  Use last selected map.
	//
	DebugPrint("Map   path: %s\n" _C_ CurrentMapPath);
	strcpy(ScenSelectPath, StratagusLibPath);
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

	EndMenu();
}

/**
** Net error menu
**
** @param error Error message
*/
static void NetErrorMenu(char *error)
{
	Menu* menu;

	menu = FindMenu("menu-net-error");
	menu->Items[1].D.Text.text = NewStringDesc(error);
	ProcessMenu("menu-net-error", 1);
	FreeStringDesc(menu->Items[1].D.Text.text);
	free(menu->Items[1].D.Text.text);
	menu->Items[1].D.Text.text = NULL;
}

/**
** Error menu
**
** @param error Error message
*/
void ErrorMenu(char *error)
{
	Menu* menu;
	int oldx;
	int oldy;

	menu = FindMenu("menu-net-error");
	oldx = menu->X;
	oldy = menu->Y;
	menu->X = (VideoWidth - menu->Width) / 2;
	menu->Y = (VideoHeight - menu->Height) / 2;
	menu->Items[1].D.Text.text = NewStringDesc(error);
	ProcessMenu("menu-net-error", 1);
	FreeStringDesc(menu->Items[1].D.Text.text);
	free(menu->Items[1].D.Text.text);
	menu->Items[1].D.Text.text = NULL;
	menu->X = oldx;
	menu->Y = oldy;
}

/*----------------------------------------------------------------------------
--  Init functions
----------------------------------------------------------------------------*/

/**
**  Initialize player races for a menu item
*/
static void InitPlayerRaces(Menuitem* mi)
{
	int i;
	int n;

	for (i = 0, n = 0; i < PlayerRaces.Count; ++i) {
		if (PlayerRaces.Visible[i]) {
			++n;
		}
	}
	++n;
	// Reallocate pulldown options.
	if (mi->D.Pulldown.options) {
		free(mi->D.Pulldown.options);
	}
	mi->D.Pulldown.options = malloc(n * sizeof(unsigned char*));
	mi->D.Pulldown.options[0] = strdup("Map Default");
	for (i = 0, n = 1; i < PlayerRaces.Count; ++i) {
		if (PlayerRaces.Visible[i]) {
			mi->D.Pulldown.options[n++] = strdup(PlayerRaces.Display[i]);
		}
	}
	mi->D.Pulldown.noptions = n;
	mi->D.Pulldown.defopt = 0;
}

/**
**  Initialize tilesets for a menu item
*/
static void InitTilesets(Menuitem* mi, int mapdefault)
{
	int i;
	int n;

	// Reallocate pulldown options.
	if (mi->D.Pulldown.options) {
		free(mi->D.Pulldown.options);
	}
	n = NumTilesets + (mapdefault ? 1 : 0);
	mi->D.Pulldown.options = malloc(n * sizeof(unsigned char*));
	n = 0;
	if (mapdefault) {
		mi->D.Pulldown.options[n++] = strdup("Map Default");
	}
	for (i = 0; i < NumTilesets; ++i) {
		mi->D.Pulldown.options[n++] = strdup(Tilesets[i]->Name);
	}
	mi->D.Pulldown.noptions = n;
	mi->D.Pulldown.defopt = 0;
}

/**
**  Initialize the loaded menu data
*/
void InitMenuData(void)
{
	Menu* menu;

	menu = FindMenu("menu-custom-game");
	InitPlayerRaces(&menu->Items[6]);
	InitTilesets(&menu->Items[14], 1);
	menu = FindMenu("menu-multi-setup");
	InitPlayerRaces(&menu->Items[21]);
	InitTilesets(&menu->Items[29], 1);
	menu = FindMenu("menu-net-multi-client");
	InitPlayerRaces(&menu->Items[21]);
	InitTilesets(&menu->Items[29], 1);
	menu = FindMenu("menu-editor-new");
	InitTilesets(&menu->Items[7], 0);
	menu = FindMenu("menu-editor-map-properties");
	InitTilesets(&menu->Items[6], 0);
}

#if 0
/**
**  FIXME: docu
*/
static void MultiGameMasterReport(void)
{
// EndMenu();

	ProcessMenu("metaserver-list", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}

}
#endif

/**
**  Menu for Mater Server Game list.
*/
static void ShowMetaServerList(void)
{
	EndMenu();

	GuiGameStarted = 0;
	ProcessMenu("metaserver-list", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

/**
**  Multiplayer server menu init callback
**
**  Mohydine: Right now, because I find it simpler, the client is sending
**            n commands, one for each online game.
**  @todo: well, redo this :)
*/
static void MultiMetaServerGameSetupInit(Menu* menu)
{
	int i;
	int j;
	int k;
	int numparams;
	int nummenus;
	char* parameter;
	char* reply;
	char* port;

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
			GetMetaParameter(reply, 3, &parameter);       // IP
			GetMetaParameter(reply, 4, &port);            // port
			sprintf(parameter, "%s:%s", parameter, port); // IP:Port
			menu->Items[j + 1].D.Text.text = NewStringDesc(parameter);
			GetMetaParameter(reply, 6, &parameter);
			menu->Items[j + 2].D.Text.text = NewStringDesc(parameter);
			GetMetaParameter(reply, 7, &parameter);
			menu->Items[j + 3].D.Text.text = NewStringDesc(parameter);
			GetMetaParameter(reply, 8, &parameter);
			menu->Items[j + 4].D.Text.text = NewStringDesc(parameter);
			menu->Items[j + 5].D.Checkbox.Checked = 0;
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

/**
**  Multiplayer server menu exit callback
*/
static void MultiMetaServerGameSetupExit(Menu* menu)
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
			free(menu->Items[i + j].D.Text.text);
			menu->Items[i + j].D.Text.text = NULL;
		}
	}
// EndMenu();
// EndMenu();
}

/**
**  Action taken when a player select an online game
*/
static void SelectGameServer(Menuitem* mi)
{
	char server_host_buffer[64];
	char *port;
	int j;
	char* tmp;

	j = mi - mi->Menu->Items;
	mi->Menu->Items[j].D.Checkbox.Checked = 0;
	EndMenu();

	tmp = EvalString(mi->Menu->Items[j - 4].D.Text.text);
	strcpy(server_host_buffer, tmp);
	free(tmp);

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

	if (NetworkArg) {
		free(NetworkArg);
	}
	NetworkArg = strdup(server_host_buffer);

	// Here we really go...
	ProcessMenu("menu-net-connecting", 1);

	if (GuiGameStarted) {
		EndMenu();
	}
}

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
		if (TheMap.Info.PlayerType[i] == PlayerPerson) {
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

/**
**  FIXME: docu
*/
static int MetaServerConnectError(void)
{
	Invalidate();
	NetErrorMenu("Cannot Connect to Meta-Server");
	return 0;
}

/**
**  Close MetaServer connection
*/
static void MultiMetaServerClose(void)
{
	MetaClose();
	MetaServerInUse = 0;
	EndMenu();
}

/**
**  Update menu item state. (disabled, ...)
**
**  @param Menuitem* Items
*/
void UpdateMenuItemButton(Menuitem* items)
{
	void (*handler)(void);

	Assert(items);
	Assert(items->MiType == MiTypeButton);

	// Enable by default.
	items->Flags &= ~MI_FLAGS_DISABLED;
	handler = items->D.Button.Handler;
	// restrict then
	if (((handler == LoadGameOk || handler == SaveConfirmOk || handler == SaveGameOk) &&
			(!ScenSelectFileName[0])) ||
		(handler == DeleteConfirmOk && !SelectedFileExist) ||
		(handler == DeleteConfirmMenu && !SelectedFileExist) ||
		((handler == SaveGameMenu || handler == SaveGameOk) &&
			(!GameRunning || IsNetworkGame() || ReplayGameType != ReplayNone)) ||
		((handler == LoadGameMenu || handler == EndScenarioRestart) &&
			(GameRunning && IsNetworkGame())) ||
		(handler == SaveReplayOk && (CommandLogDisabled || !ScenSelectFileName[0])) ||
		(handler == RestartConfirmMenu && (!GameRunning || IsNetworkGame())) ||
		//
		// MultiPlayer
		//
		(handler == MultiPlayerGameMenu && NetworkNumInterfaces == 0) ||
		(handler == MultiClientReady && LocalSetupState.Ready[NetLocalHostsSlot]) ||
		(handler == MultiClientNotReady && !LocalSetupState.Ready[NetLocalHostsSlot]) ||
		//
		// Campaign : FIXME make it more configurable (not only 4 campaign)
		//
		(handler == CampaignGameMenu && NumCampaigns < 1) ||
		(handler == CampaignMenu1 && NumCampaigns < 1) ||
		(handler == CampaignMenu2 && NumCampaigns < 2) ||
		(handler == CampaignMenu3 && NumCampaigns < 3) ||
		(handler == CampaignMenu4 && NumCampaigns < 4) ||
		(handler == SelectCampaignMenu && 1 /*FIXME : not implemented */) ||
		//
		// Folder
		//
		((handler == EditorMainLoadFolder || handler == ReplayGameFolder ||
			handler == ScenSelectFolder || handler == EditorSaveFolder) &&
			(!*ScenSelectDisplayPath)) ||
		((handler == ReplayGameOk || handler == EditorLoadOk ||
			handler == ScenSelectOk || handler == EditorSaveOk) &&
			(!ScenSelectFileName[0] && !ScenSelectPathName[0]))
		) {
		items->Flags = MI_FLAGS_DISABLED;
	}

	//
	// Multiplayer
	//
	if (handler == MultiScenSelectMenu || handler == MultiGameStart) {
		char plyrs; // other player has connected.
		char ready; // all player are here.
		int i;

		plyrs = 0;
		ready = 1;


		// Calculate available slots from pudinfo
		for (i = 1; i < PlayerMax; i++) {
			if (TheMap.Info.PlayerType[i] == PlayerPerson) {
				if (Hosts[i].PlyNr) {
					plyrs = 1;
					if (!ServerSetupState.Ready[i]) {
						ready = 0;
						break;
					}
				} else {
					if (ServerSetupState.CompOpt[i] == 0) {
						ready = 0;
					}
				}
			}
		}
		if (handler == MultiScenSelectMenu && plyrs) {
			items->Flags = MI_FLAGS_DISABLED;
		}
		if (handler == MultiGameStart && !ready) {
			items->Flags = MI_FLAGS_DISABLED;
		}
	}

#if 1 // FIXME : Remove it when lua modified.
	//
	//  Text modification.
	//
	if (handler == ReplayGameOk || handler == ScenSelectOk || handler == EditorLoadOk) {
		free(items->D.Button.Text);
		items->D.Button.Text = strdup(ScenSelectPathName[0] ? "Open" : "Ok");
	} else if (handler == EditorSaveOk) {
		free(items->D.Button.Text);
		items->D.Button.Text = strdup(ScenSelectPathName[0] ? "Open" : "Save");
	}
	if (handler == EditorMainLoadFolder || handler == ReplayGameFolder ||
			handler == ScenSelectFolder || handler == EditorSaveFolder) {
		free(items->D.Button.Text);
		items->D.Button.Text = strdup(ScenSelectDisplayPath);
	}

	{
		int i;
		const char* s;
		void (*campaignhandler[])(void) = {CampaignMenu1, CampaignMenu2, CampaignMenu3, CampaignMenu4};

		// Campaign name.
		for (i = 0; i < 4; ++i) {
			if (handler == campaignhandler[i]) {
				free(items->D.Button.Text);
				items->D.Button.Text =
					strdup(i < NumCampaigns ? Campaigns[i].Name : "Not available");
				if ((s = strchr(items->D.Button.Text, '!'))) {
					items->D.Button.HotKey = tolower(s[1]);
				}
			}
		}
	}
#endif
}


//@}
