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
/**@name menus.c	-	The menu buttons. */
//
//	(c) Copyright 1999-2002 by Andreas Arens
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#ifndef _MSC_VER
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#else
#ifndef _WIN32_WCE
#include <direct.h>
#include <io.h>
#define F_OK 4
#define PATH_MAX _MAX_PATH
#endif
#endif

#include "freecraft.h"
#include "video.h"
#include "player.h"
#include "font.h"
#include "tileset.h"
#include "map.h"
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
#include "ccl.h"

#if defined(USE_SDLCD) || defined(USE_SDLA)
#include "SDL.h"
#endif

#ifdef USE_LIBCDA
#include "libcda.h"
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

/*----------------------------------------------------------------------------
--	Prototypes for local functions
----------------------------------------------------------------------------*/

local void EndMenu(void);

/*----------------------------------------------------------------------------
--	Prototypes for action handlers and helper functions
----------------------------------------------------------------------------*/

local void GameMenuEnd(void);
local void GameMenuExit(void);
local void GameMenuReturn(void);
local void GameGlobalOptionsMenu(void);
local void GameShowCredits(void);
local void GameMenuObjectives(void);
local void GameMenuEndScenario(void);
local void GameOptions(void);

local void HelpMenu(void);
local void KeystrokeHelpMenu(void);
local void ShowTipsMenu(void);
local void InitTips(Menuitem *mi);
local void TipsMenuEnd(void);
local void SetTips(Menuitem *mi);
local void ShowNextTip(void);

local void InitGameMenu(Menuitem *mi);

local void SetMasterPower(Menuitem *mi);
local void SetMusicPower(Menuitem *mi);
local void SetCdPower(Menuitem *mi);
local void SetFogOfWar(Menuitem *mi);
local void SetCommandKey(Menuitem *mi);
local void SetCdModeAll(Menuitem *mi);
local void SetCdModeRandom(Menuitem *mi);

local void EndScenarioRestart(void);
local void EndScenarioSurrender(void);
local void EndScenarioQuitMenu(void);

local void PrgStartInit(Menuitem *mi);		// master init
local void StartMenusSetBackground(Menuitem *mi);
local void NameLineDrawFunc(Menuitem *mi);
local void EnterNameAction(Menuitem *mi, int key);
local void EnterNameCancel(void);
local void EnterServerIPAction(Menuitem *mi, int key);
local void EnterServerIPCancel(void);

local void EnterSaveGameAction(Menuitem *mi, int key);
local void SaveAction(void);
local void CreateSaveDir(Menuitem *mi);

local void SaveLBExit(Menuitem *mi);
local void SaveLBInit(Menuitem *mi);
local unsigned char *SaveLBRetrieve(Menuitem *mi, int i);
local void SaveLBAction(Menuitem *mi, int i);
local void SaveVSAction(Menuitem *mi, int i);
local void SaveOk(void);

local void LoadLBExit(Menuitem *mi);
local void LoadLBInit(Menuitem *mi);
local unsigned char *LoadLBRetrieve(Menuitem *mi, int i);
local void LoadLBAction(Menuitem *mi, int i);
local void LoadVSAction(Menuitem *mi, int i);
local void LoadOk(void);
local int SaveRDFilter(char *pathbuf, FileList *fl);

local void FcDeleteMenu(void);
local void FcDeleteInit(Menuitem *mi __attribute__((unused)));
local void FcDeleteFile(void);

//local void ConfirmSaveMenu(void);
local void ConfirmSaveInit(Menuitem *mi __attribute__((unused)));
local void ConfirmSaveFile(void);

local void LoadAction(void);

local void JoinNetGameMenu(void);
local void CreateNetGameMenu(void);

local void SinglePlayerGameMenu(void);
local void MultiPlayerGameMenu(void);
local void CampaignGameMenu(void);
local void ScenSelectMenu(void);

local void CampaignMenu1(void);
local void CampaignMenu2(void);
local void CampaignMenu3(void);
local void CampaignMenu4(void);
local void SelectCampaignMenu(void);

local void ScenSelectLBExit(Menuitem *mi);
local void ScenSelectLBInit(Menuitem *mi);
local unsigned char *ScenSelectLBRetrieve(Menuitem *mi, int i);
local void ScenSelectLBAction(Menuitem *mi, int i);
local void ScenSelectTPMSAction(Menuitem *mi, int i);
local void ScenSelectVSAction(Menuitem *mi, int i);
local void ScenSelectFolder(void);
local void ScenSelectInit(Menuitem *mi);	// master init
local void ScenSelectOk(void);
local void ScenSelectCancel(void);
local int ScenSelectRDFilter(char *pathbuf, FileList *fl);

local void KeystrokeHelpVSAction(Menuitem *mi, int i);
local void KeystrokeHelpDrawFunc(Menuitem *mi);

local void GameSpeedHSAction(Menuitem *mi, int i);
local void MouseScrollHSAction(Menuitem *mi, int i);
local void KeyboardScrollHSAction(Menuitem *mi, int i);
local void MasterVolumeHSAction(Menuitem *mi, int i);
local void MusicVolumeHSAction(Menuitem *mi, int i);
local void CdVolumeHSAction(Menuitem *mi, int i);

local void GameSetupInit(Menuitem *mi);		// master init
local void GameDrawFunc(Menuitem *mi);

local void GameRCSAction(Menuitem *mi, int i);
local void GameRESAction(Menuitem *mi, int i);
local void GameUNSAction(Menuitem *mi, int i);
local void GameTSSAction(Menuitem *mi, int i);
local void GameGATAction(Menuitem *mi, int i);

local void GameCancel(void);

local void CustomGameStart(void);
local void CustomGameOPSAction(Menuitem *mi, int i);

local void MultiClientReady(void);
local void MultiClientNotReady(void);
local void MultiClientGemAction(Menuitem *mi);
local void MultiClientCancel(void);
local void MultiClientRCSAction(Menuitem *mi, int i);

local void MultiGameStart(void);
local void MultiGameCancel(void);
local void NetworkGamePrepareGameSettings(void);
local void MultiGameSetupInit(Menuitem *mi);	// master init
local void MultiGameSetupExit(Menuitem *mi);	// master exit
local void MultiGameDrawFunc(Menuitem *mi);
local void MultiGameFWSAction(Menuitem *mi, int i);
local void MultiGamePTSAction(Menuitem *mi, int o);
local void NetMultiPlayerDrawFunc(Menuitem *mi);
local void MultiGamePlayerSelectorsUpdate(int initial);
local void MultiScenSelectMenu(void);

local void MultiGameClientInit(Menuitem *mi);	// master init
local void MultiGameClientDrawFunc(Menuitem *mi);
local void MultiClientUpdate(int initial);

local void NetConnectingCancel(void);
local void TerminateNetConnect(void);

local void StartEditor(void);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /// Name, Version, Copyright
extern char NameLine[];

local EventCallback callbacks;

// FIXME: Johns: this must be all be configured from ccl some time.

    /// private struct which specifies the buttons gfx
local struct {
	/// resource filename one for each race
    const char*	File[PlayerMaxRaces];
	/// Width of button
    int		Width, Height;
	/// sprite : FILLED
    Graphic*	Sprite;
} MenuButtonGfx
#ifndef laterUSE_CCL
= {
    { "ui/buttons 1.png" ,"ui/buttons 2.png" },
    300, 7632,

    NULL
}
#endif
    ;

/**
**	The currently processed menu
*/
global int CurrentMenu = -1;

/**
**	X, Y, Width, and Height of menu are to redraw
*/
global int MenuRedrawX;
global int MenuRedrawY;
global int MenuRedrawW;
global int MenuRedrawH;

local int MenuButtonUnderCursor = -1;
local int MenuButtonCurSel = -1;

local int GameLoaded;

/**
**	Offsets from top and left, used for different resolutions
*/
local int OffsetX = 0;
local int OffsetY = 0;

/**
**	Other client and server selection state for Multiplayer clients
*/
global ServerSetup ServerSetupState, LocalSetupState;

/**
**	The background picture used by start menues
*/
local Graphic* Menusbgnd;

/**
**	Items for the Game Menu
**
**	@todo FIXME: Configure with CCL.
*/
local Menuitem GameMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, InitGameMenu, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16 + 12 + 106, 40, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36 + 36, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36 + 36 + 36, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitGameMenuItems() {
    MenuitemText   i0 = { "Game Menu", MI_TFLAGS_CENTERED};
    MenuitemButton i1 = { "Save (~<F11~>)", 106, 27, MBUTTON_GM_HALF, GameMenuSave, KeyCodeF11};
    MenuitemButton i2 = { "Load (~<F12~>)", 106, 27, MBUTTON_GM_HALF, GameMenuLoad, KeyCodeF12};
    MenuitemButton i3 = { "Options (~<F5~>)", 224, 27, MBUTTON_GM_FULL, GameOptions, KeyCodeF5};
    MenuitemButton i4 = { "Help (~<F1~>)", 224, 27, MBUTTON_GM_FULL, HelpMenu, KeyCodeF1};
    MenuitemButton i5 = { "Scenario ~!Objectives", 224, 27, MBUTTON_GM_FULL, GameMenuObjectives, 'o'};
    MenuitemButton i6 = { "~!End Scenario", 224, 27, MBUTTON_GM_FULL, GameMenuEndScenario, 'e'};
    MenuitemButton i7 = { "Return to Game (~<Esc~>)", 224, 27, MBUTTON_GM_FULL, GameMenuReturn, '\033'};
    GameMenuItems[0].d.text   = i0;
    GameMenuItems[1].d.button = i1;
    GameMenuItems[2].d.button = i2;
    GameMenuItems[3].d.button = i3;
    GameMenuItems[4].d.button = i4;
    GameMenuItems[5].d.button = i5;
    GameMenuItems[6].d.button = i6;
    GameMenuItems[7].d.button = i7;
}

/**
**	Items for the Victory Menu
**	@todo FIXME: Configure with CCL.
*/
local Menuitem VictoryMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 90, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 56, MenuButtonDisabled, LargeFont, NULL, NULL, {{NULL,0}} },
};

local void InitVictoryMenuItems() {
    MenuitemText   i0 = { "Congratulations!", MI_TFLAGS_CENTERED};
    MenuitemText   i1 = { "You are victorious!", MI_TFLAGS_CENTERED};
    MenuitemButton i2 = { "~!Victory", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'v'};
    MenuitemButton i3 = { "Save Game (~<F11~>)", 224, 27, MBUTTON_GM_FULL, NULL, KeyCodeF11};
    VictoryMenuItems[0].d.text   = i0;
    VictoryMenuItems[1].d.text   = i1;
    VictoryMenuItems[2].d.button = i2;
    VictoryMenuItems[3].d.button = i3;
}

/**
**	Items for the Lost Menu
**	@todo FIXME: Configure with CCL.
*/
local Menuitem LostMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 90, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitLostMenuItems() {
    MenuitemText   i0 = { "You failed to", MI_TFLAGS_CENTERED};
    MenuitemText   i1 = { "achieve victory!", MI_TFLAGS_CENTERED};
    MenuitemButton i2 = { "~!OK", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'o'};
    LostMenuItems[0].d.text   = i0;
    LostMenuItems[1].d.text   = i1;
    LostMenuItems[2].d.button = i2;
}

local Menuitem TipsMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, InitTips, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 14, 256-75, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14+22, 256-75+4, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 14, 256-40, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 168, 256-40, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*0, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*1, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*2, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*3, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*4, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*5, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*6, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*7, 0, GameFont, NULL, NULL, {{NULL,0}} },
};
local void InitTipsMenuItems() {
    MenuitemText   i0  = { "Freecraft Tips", MI_TFLAGS_CENTERED};
    MenuitemGem    i1  = { MI_GSTATE_CHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetTips};
    MenuitemText   i2  = { "Show tips at startup", MI_TFLAGS_LALIGN};
    MenuitemButton i3  = { "~!Next Tip", 106, 27, MBUTTON_GM_HALF, ShowNextTip, 'n'};
    MenuitemButton i4  = { "~!Close", 106, 27, MBUTTON_GM_HALF, TipsMenuEnd, 'c'};
    MenuitemText   i5  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i6  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i7  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i8  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i9  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i10 = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i11 = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i12 = { NULL, MI_TFLAGS_LALIGN};
    TipsMenuItems[0].d.text   = i0;
    TipsMenuItems[1].d.gem    = i1;
    TipsMenuItems[2].d.text   = i2;
    TipsMenuItems[3].d.button = i3;
    TipsMenuItems[4].d.button = i4;
    TipsMenuItems[5].d.text   = i5;
    TipsMenuItems[6].d.text   = i6;
    TipsMenuItems[7].d.text   = i7;
    TipsMenuItems[8].d.text   = i8;
    TipsMenuItems[9].d.text   = i9;
    TipsMenuItems[10].d.text  = i10;
    TipsMenuItems[11].d.text  = i11;
    TipsMenuItems[12].d.text  = i12;
}

local Menuitem ObjectivesMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*0, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*1, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*2, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*3, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*4, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*6, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*7, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*8, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitObjectivesMenuItems() {
    MenuitemText   i0  = { "Objectives", MI_TFLAGS_CENTERED};
    MenuitemText   i1  = { NULL, 0};
    MenuitemButton i10 = { "~!OK", 224, 27, MBUTTON_GM_FULL, EndMenu, 'o'};
    ObjectivesMenuItems[0].d.text    = i0;
    ObjectivesMenuItems[1].d.text    = i1;
    ObjectivesMenuItems[2].d.text    = i1;
    ObjectivesMenuItems[3].d.text    = i1;
    ObjectivesMenuItems[4].d.text    = i1;
    ObjectivesMenuItems[5].d.text    = i1;
    ObjectivesMenuItems[6].d.text    = i1;
    ObjectivesMenuItems[7].d.text    = i1;
    ObjectivesMenuItems[8].d.text    = i1;
    ObjectivesMenuItems[9].d.text    = i1;
    ObjectivesMenuItems[10].d.button = i10;
}

local Menuitem EndScenarioMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*0, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*1, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*2, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*3, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitEndScenarioMenuItems() {
    MenuitemText   i0 = { "End Scenario", MI_TFLAGS_CENTERED};
    MenuitemButton i1 = { "~!Restart Scenario", 224, 27, MBUTTON_GM_FULL, EndScenarioRestart, 'r'};
    MenuitemButton i2 = { "~!Surrender", 224, 27, MBUTTON_GM_FULL, EndScenarioSurrender, 's'};
    MenuitemButton i3 = { "~!Quit to Menu", 224, 27, MBUTTON_GM_FULL, EndScenarioQuitMenu, 'q'};
    MenuitemButton i4 = { "E~!xit Program", 224, 27, MBUTTON_GM_FULL, GameMenuExit, 'x'};
    MenuitemButton i5 = { "Previous (~!E~!s~!c)", 224, 27, MBUTTON_GM_FULL, EndMenu, '\033'};
    EndScenarioMenuItems[0].d.text  = i0;
    EndScenarioMenuItems[1].d.button = i1;
    EndScenarioMenuItems[2].d.button = i2;
    EndScenarioMenuItems[3].d.button = i3;
    EndScenarioMenuItems[4].d.button = i4;
    EndScenarioMenuItems[5].d.button = i5;
}

/**
**	Items for the SelectScen Menu
*/
local unsigned char *ssmtoptions[] = {
    // "All scenarios (cm+pud)",
    "Freecraft scenario (cm)",
    "Foreign scenario (pud)"
    // FIXME: what is about levels in zips?
};

local unsigned char *ssmsoptions[] = {
    "Any size",
    "32 x 32",
    "64 x 64",
    "96 x 96",
    "128 x 128",
    "256 x 256",
    "512 x 512",
    "1024 x 1024",
    // FIXME: We support bigger sizes
};

local int GuiGameStarted;

local char ScenSelectPath[1024];		/// Scenario selector path
local char ScenSelectDisplayPath[1024];		/// Displayed selector path
local char ScenSelectFileName[128];		/// Scenario selector name
global char ScenSelectFullPath[1024];		/// Scenario selector path+name

global MapInfo *ScenSelectPudInfo;		/// Selected pud info

/**
**	Scenario selection menu.
**
**	@todo FIXME: Configure with CCL.
*/
local Menuitem ScenSelectMenuItems[] = {
    { MI_TYPE_TEXT, 176, 8, 0, LargeFont, ScenSelectInit, NULL, {{NULL,0}} },

    { MI_TYPE_LISTBOX, 24, 140, 0, GameFont, ScenSelectLBInit, ScenSelectLBExit, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 312, 140, 0, 0, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 48, 318, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 198, 318, 0, LargeFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 132, 40, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 140, 40, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 132, 80, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 140, 80, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 22, 112, 0, GameFont, NULL, NULL, {{NULL,0}} },
};
local void InitScenSelectMenuItems() {
    MenuitemText    i0 = { "Select scenario", MI_TFLAGS_CENTERED};

    MenuitemListbox  i1 = { NULL, 288, 6*18, MBUTTON_PULLDOWN, ScenSelectLBAction, 0, 0, 0, 0, 6, 0,
			    (void *)ScenSelectLBRetrieve, ScenSelectOk};
    MenuitemVslider  i2 = { 0, 18, 6*18, ScenSelectVSAction, -1, 0, 0, 0, ScenSelectOk};

    MenuitemButton   i3 = { "OK", 106, 27, MBUTTON_GM_HALF, ScenSelectOk, 0};
    MenuitemButton   i4 = { "Cancel", 106, 27, MBUTTON_GM_HALF, ScenSelectCancel, 0};

    MenuitemText     i5 = { "Type:", MI_TFLAGS_RALIGN};
    MenuitemPulldown i6 = { ssmtoptions, 192, 20, MBUTTON_PULLDOWN, ScenSelectTPMSAction, 2, 1, 1, 0, 0};
    MenuitemText     i7 = { "Map size:", MI_TFLAGS_RALIGN};
    MenuitemPulldown i8 = { ssmsoptions, 192, 20, MBUTTON_PULLDOWN, ScenSelectTPMSAction, 8, 0, 0, 0, 0};
    MenuitemButton   i9 = { NULL, 36, 24, MBUTTON_FOLDER, ScenSelectFolder, 0};
    ScenSelectMenuItems[0].d.text     = i0;
    ScenSelectMenuItems[1].d.listbox  = i1;
    ScenSelectMenuItems[2].d.vslider  = i2;
    ScenSelectMenuItems[3].d.button   = i3;
    ScenSelectMenuItems[4].d.button   = i4;
    ScenSelectMenuItems[5].d.text     = i5;
    ScenSelectMenuItems[6].d.pulldown = i6;
    ScenSelectMenuItems[7].d.text     = i7;
    ScenSelectMenuItems[8].d.pulldown = i8;
    ScenSelectMenuItems[9].d.button   = i9;
}

/**
**	Items for the Program Start Menu
*/
local Menuitem PrgStartMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, PrgStartInit, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 0, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 1, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 2, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 3, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 4, MenuButtonDisabled, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 5, MenuButtonDisabled, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 6, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 7, 0, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitPrgStartMenuItems() {
    MenuitemDrawfunc i0 = { NameLineDrawFunc };
    MenuitemButton   i1 = { "~!Single Player Game", 224, 27, MBUTTON_GM_FULL, SinglePlayerGameMenu, 's'};
    MenuitemButton   i2 = { "~!Multi Player Game", 224, 27, MBUTTON_GM_FULL, MultiPlayerGameMenu, 'm'};
    MenuitemButton   i3 = { "~!Campaign Game", 224, 27, MBUTTON_GM_FULL, CampaignGameMenu, 'c'};
    MenuitemButton   i4 = { "~!Load Game", 224, 27, MBUTTON_GM_FULL, GameMenuLoad, 'l'};
    MenuitemButton   i5 = { "~!Options", 224, 27, MBUTTON_GM_FULL, GameGlobalOptionsMenu, 'o'};
    MenuitemButton   i6 = { "~!Editor", 224, 27, MBUTTON_GM_FULL, StartEditor, 'e'};
    MenuitemButton   i7 = { "S~!how Credits", 224, 27, MBUTTON_GM_FULL, GameShowCredits, 'h'};
    MenuitemButton   i8 = { "E~!xit Program", 224, 27, MBUTTON_GM_FULL, GameMenuExit, 'x'};
    PrgStartMenuItems[0].d.drawfunc = i0;
    PrgStartMenuItems[1].d.button   = i1;
    PrgStartMenuItems[2].d.button   = i2;
    PrgStartMenuItems[3].d.button   = i3;
    PrgStartMenuItems[4].d.button   = i4;
    PrgStartMenuItems[5].d.button   = i5;
    PrgStartMenuItems[6].d.button   = i6;
    PrgStartMenuItems[7].d.button   = i7;
    PrgStartMenuItems[8].d.button   = i8;
}

/**
**	Items for the Custom Game Setup Menu
*/
local unsigned char *rcsoptions[] = {
    "Human",
    "Orc",
    "Map Default",
    // FIXME: we support more and other race names
};

local unsigned char *resoptions[] = {
    "Map Default",
    "Low",
    "Medium",
    "High",
};

local unsigned char *unsoptions[] = {
    "Map Default",
    "One Peasant Only",
    // FIXME: we support more and other unit names
};

// FIXME: Must load this from some place.
local unsigned char *tssoptions[] = {
    "Map Default",
    "Forest",
    "Winter",
    "Wasteland",
    "Orc Swamp",	// hmm. need flag if XPN-GFX is present! - tmp-hack in InitMenus()..
			// FIXME: Some time later _all_ tilesets should be a dynamic (ccl-defineable) resource..
};

/**
**	Game type options.
**
**	@todo Needs to be configurable from CCL.
*/
local unsigned char* gatoptions[] = {
    "Use map settings",
    "Melee",
    "Free for all",
    "One on one",
    "Capture the flag",
    "Greed",
    "Slaughter",
    "Sudden death",
    "Team melee",
    "Team free for all",
    "Team capture the flag",
    "Top vs bottom",
    "Left vs right",
};

local unsigned char *cgopsoptions[] = {
    "Map Default",
    "1 Opponent",
    "2 Opponents",
    "3 Opponents",
    "4 Opponents",
    "5 Opponents",
    "6 Opponents",
    "7 Opponents",
    // FIXME: We support upto 15 opponents
};

local unsigned char *mgfwsoptions[] = {
    "On",
    "Off",
};

local unsigned char *mgptsoptions[] = {
    "Available",
    "Computer",
    "Closed",
};

/**
**	Single player custom game menu.
*/
local Menuitem CustomGameMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, GameSetupInit, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640/2+12, 192, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36+36, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+240-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+240, 0, GameFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 220, 10+240-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+240, 0, GameFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 640-224-16, 10+240-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 640-224-16, 10+240, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+300-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+300, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+300-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+300, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640-224-16, 10+300-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 640-224-16, 10+300, 0, GameFont, NULL, NULL, {{NULL,0}} },
};
local void InitCustomGameMenuItems() {
    MenuitemDrawfunc i0  = { GameDrawFunc };
    MenuitemText     i1  = { "~<Single Player Game Setup~>", MI_TFLAGS_CENTERED};
    MenuitemButton   i2  = { "S~!elect Scenario", 224, 27, MBUTTON_GM_FULL, ScenSelectMenu, 'e'};
    MenuitemButton   i3  = { "~!Start Game", 224, 27, MBUTTON_GM_FULL, CustomGameStart, 's'};
    MenuitemButton   i4  = { "~!Cancel Game", 224, 27, MBUTTON_GM_FULL, GameCancel, 'c'};
    MenuitemText     i5  = { "~<Your Race:~>", 0};
    MenuitemPulldown i6  = { rcsoptions, 152, 20, MBUTTON_PULLDOWN, GameRCSAction, 3, 2, 2, 0, 0};
    MenuitemText     i7  = { "~<Resources:~>", 0};
    MenuitemPulldown i8  = { resoptions, 152, 20, MBUTTON_PULLDOWN, GameRESAction, 4, 0, 0, 0, 0};
    MenuitemText     i9  = { "~<Units:~>", 0};
    MenuitemPulldown i10 = { unsoptions, 190, 20, MBUTTON_PULLDOWN, GameUNSAction, 2, 0, 0, 0, 0};
    MenuitemText     i11 = { "~<Opponents:~>", 0};
    MenuitemPulldown i12 = { cgopsoptions, 152, 20, MBUTTON_PULLDOWN, CustomGameOPSAction, 8, 0, 0, 0, 0};
    MenuitemText     i13 = { "~<Map Tileset:~>", 0};
    MenuitemPulldown i14 = { tssoptions, 152, 20, MBUTTON_PULLDOWN, GameTSSAction, 5, 0, 0, 0, 0};
    MenuitemText     i15 = { "~<Game Type:~>", 0};
    MenuitemPulldown i16 = { gatoptions, 190, 20, MBUTTON_PULLDOWN, GameGATAction, 13, 0, 0, 0, 0};
    CustomGameMenuItems[0].d.drawfunc  = i0;
    CustomGameMenuItems[1].d.text      = i1;
    CustomGameMenuItems[2].d.button    = i2;
    CustomGameMenuItems[3].d.button    = i3;
    CustomGameMenuItems[4].d.button    = i4;
    CustomGameMenuItems[5].d.text      = i5;
    CustomGameMenuItems[6].d.pulldown  = i6;
    CustomGameMenuItems[7].d.text      = i7;
    CustomGameMenuItems[8].d.pulldown  = i8;
    CustomGameMenuItems[9].d.text      = i9;
    CustomGameMenuItems[10].d.pulldown = i10;
    CustomGameMenuItems[11].d.text     = i11;
    CustomGameMenuItems[12].d.pulldown = i12;
    CustomGameMenuItems[13].d.text     = i13;
    CustomGameMenuItems[14].d.pulldown = i14;
    CustomGameMenuItems[15].d.text     = i15;
    CustomGameMenuItems[16].d.pulldown = i16;
}

/**
**	Items for the Enter Name Menu
*/
local Menuitem EnterNameMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_INPUT, 40, 38, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 24, 80, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 154, 80, 0, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitEnterNameMenuItems() {
    MenuitemText   i0 = { "Enter your name:", MI_TFLAGS_CENTERED};
    MenuitemInput  i1 = { NULL, 212, 20, MBUTTON_PULLDOWN, EnterNameAction, 0, 0};
    MenuitemButton i2 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};
    MenuitemButton i3 = { "~!Cancel", 106, 27, MBUTTON_GM_HALF, EnterNameCancel, 'c'};
    EnterNameMenuItems[0].d.text   = i0;
    EnterNameMenuItems[1].d.input  = i1;
    EnterNameMenuItems[2].d.button = i2;
    EnterNameMenuItems[3].d.button = i3;
}

/**
**	Items for the Enter Server Menu
*/
local Menuitem EnterServerIPMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_INPUT, 40, 38, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 24, 80, MenuButtonDisabled, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 154, 80, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitEnterServerIPMenuItems() {
    MenuitemText   i0 = { "Enter server IP-address:", MI_TFLAGS_CENTERED};
    MenuitemInput  i1 = { NULL, 212, 20, MBUTTON_PULLDOWN, EnterServerIPAction, 0, 0};
    MenuitemButton i2 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};
    MenuitemButton i3 = { "~!Cancel", 106, 27, MBUTTON_GM_HALF, EnterServerIPCancel, 'c'};
    EnterServerIPMenuItems[0].d.text   = i0;
    EnterServerIPMenuItems[1].d.input  = i1;
    EnterServerIPMenuItems[2].d.button = i2;
    EnterServerIPMenuItems[3].d.button = i3;
}

/**
**	Items for the Net Create Join Menu
*/
local Menuitem NetCreateJoinMenuItems[] = {
    { MI_TYPE_BUTTON, 208, 320, 0, LargeFont, NULL/*StartMenusSetBackground*/, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 320 + 36, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 320 + 36 + 36, 0, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitNetCreateJoinMenuItems() {
    MenuitemButton i0 = { "~!Join Game", 224, 27, MBUTTON_GM_FULL, JoinNetGameMenu, 'j'};
    MenuitemButton i1 = { "~!Create Game", 224, 27, MBUTTON_GM_FULL, CreateNetGameMenu, 'c'};
    MenuitemButton i2 = { "~!Previous Menu", 224, 27, MBUTTON_GM_FULL, EndMenu, 'p'};
    NetCreateJoinMenuItems[0].d.button = i0;
    NetCreateJoinMenuItems[1].d.button = i1;
    NetCreateJoinMenuItems[2].d.button = i2;
}


/**
**	Items for the Net Multiplayer Setup Menu
*/
local Menuitem NetMultiButtonStorage[] = {
    { MI_TYPE_PULLDOWN, 40, 32, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_DRAWFUNC, 40, 32, 0, GameFont, NULL, NULL, {{NULL,0}} },
};
local void InitNetMultiButtonStorage() {
    MenuitemPulldown i0 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, MultiGamePTSAction, 3, -1, 0, 0, 0};
    MenuitemDrawfunc i1 = { NetMultiPlayerDrawFunc };
    NetMultiButtonStorage[0].d.pulldown = i0;
    NetMultiButtonStorage[1].d.drawfunc = i1;
}

/**
**	Multi player custom game menu (server side).
*/
local Menuitem NetMultiSetupMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, MultiGameSetupInit, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640/2+12, 8, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36, MenuButtonDisabled, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36+36, 0, LargeFont, NULL, NULL, {{NULL,0}} },

    // 8+7 player slots (content here is overwritten!)
#define SERVER_PLAYER_STATE	5
    { MI_TYPE_PULLDOWN, 40, 32+22*0, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*1, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*2, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*3, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*4, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*5, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*6, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*7, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*0, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*1, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*2, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*3, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*4, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*5, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*6, 0, GameFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 40, 10+240-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+240, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+240-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+240, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640-224-16, 10+240-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 640-224-16, 10+240, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+300-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+300, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+300-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+300, 0, GameFont, NULL, NULL, {{NULL,0}} },

    // 7+7 player ready buttons
#define SERVER_PLAYER_READY	30
    { MI_TYPE_GEM, 10, 32+22*1, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*2, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*3, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*4, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*6, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*7, 0, LargeFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 330, 32+22*0, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*1, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*2, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*3, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*4, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*6, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    // FIXME: Slot 15 is reserved for neutral computer
    //{ MI_TYPE_GEM, 330, 32+22*7, 0, LargeFont, NULL, NULL,
    //	{ gem:{ MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL,0} } },

    // 7+7 player lag buttons
#define SERVER_PLAYER_LAG	44
    { MI_TYPE_GEM, 218, 32+22*1, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*2, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*3, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*4, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*6, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*7, 0, LargeFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 538, 32+22*0, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*1, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*2, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*3, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*4, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*6, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    // FIXME: Slot 15 is reserved for neutral computer
    //{ MI_TYPE_GEM, 538, 32+22*7, 0, LargeFont, NULL, NULL,
    //	{ gem:{ MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL,0} } },
};
local void InitNetMultiSetupMenuItems() {
    MenuitemDrawfunc i0  = { MultiGameDrawFunc };
    MenuitemText     i1  = { "~<Multi Player Setup~>", MI_TFLAGS_CENTERED};
    MenuitemButton   i2  = { "S~!elect Scenario", 224, 27, MBUTTON_GM_FULL, MultiScenSelectMenu, 'e'};
    MenuitemButton   i3  = { "~!Start Game", 224, 27, MBUTTON_GM_FULL, MultiGameStart, 's'};
    MenuitemButton   i4  = { "~!Cancel Game", 224, 27, MBUTTON_GM_FULL, MultiGameCancel, 'c'};

    MenuitemPulldown i5  = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i6  = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i7  = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i8  = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i9  = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i10 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i11 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i12 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i13 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i14 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i15 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i16 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i17 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i18 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i19 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};

    MenuitemText     i20 = { "~<Your Race:~>", 0};
    MenuitemPulldown i21 = { rcsoptions, 152, 20, MBUTTON_PULLDOWN, GameRCSAction, 3, 2, 2, 0, 0};
    MenuitemText     i22 = { "~<Resources:~>", 0};
    MenuitemPulldown i23 = { resoptions, 152, 20, MBUTTON_PULLDOWN, GameRESAction, 4, 0, 0, 0, 0};
    MenuitemText     i24 = { "~<Units:~>", 0};
    MenuitemPulldown i25 = { unsoptions, 190, 20, MBUTTON_PULLDOWN, GameUNSAction, 2, 0, 0, 0, 0};
    MenuitemText     i26 = { "~<Fog of War:~>", 0};
    MenuitemPulldown i27 = { mgfwsoptions, 152, 20, MBUTTON_PULLDOWN, MultiGameFWSAction, 2, 0, 0, 0, 0};
    MenuitemText     i28 = { "~<Map Tileset:~>", 0};
    MenuitemPulldown i29 = { tssoptions, 152, 20, MBUTTON_PULLDOWN, GameTSSAction, 5, 0, 0, 0, 0};

    MenuitemGem      i30 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i31 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i32 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i33 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i34 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i35 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i36 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};

    MenuitemGem      i37 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i38 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i39 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i40 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i41 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i42 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i43 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};

    MenuitemGem      i44 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i45 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i46 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i47 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i48 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i49 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i50 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};

    MenuitemGem      i51 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i52 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i53 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i54 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i55 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i56 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i57 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};

    NetMultiSetupMenuItems[0].d.drawfunc  = i0;
    NetMultiSetupMenuItems[1].d.text      = i1;
    NetMultiSetupMenuItems[2].d.button    = i2;
    NetMultiSetupMenuItems[3].d.button    = i3;
    NetMultiSetupMenuItems[4].d.button    = i4;
    NetMultiSetupMenuItems[5].d.pulldown  = i5;
    NetMultiSetupMenuItems[6].d.pulldown  = i6;
    NetMultiSetupMenuItems[7].d.pulldown  = i7;
    NetMultiSetupMenuItems[8].d.pulldown  = i8;
    NetMultiSetupMenuItems[9].d.pulldown  = i9;
    NetMultiSetupMenuItems[10].d.pulldown = i10;
    NetMultiSetupMenuItems[11].d.pulldown = i11;
    NetMultiSetupMenuItems[12].d.pulldown = i12;
    NetMultiSetupMenuItems[13].d.pulldown = i13;
    NetMultiSetupMenuItems[14].d.pulldown = i14;
    NetMultiSetupMenuItems[15].d.pulldown = i15;
    NetMultiSetupMenuItems[16].d.pulldown = i16;
    NetMultiSetupMenuItems[17].d.pulldown = i17;
    NetMultiSetupMenuItems[18].d.pulldown = i18;
    NetMultiSetupMenuItems[19].d.pulldown = i19;
    NetMultiSetupMenuItems[20].d.text     = i20;
    NetMultiSetupMenuItems[21].d.pulldown = i21;
    NetMultiSetupMenuItems[22].d.text     = i22;
    NetMultiSetupMenuItems[23].d.pulldown = i23;
    NetMultiSetupMenuItems[24].d.text     = i24;
    NetMultiSetupMenuItems[25].d.pulldown = i25;
    NetMultiSetupMenuItems[26].d.text     = i26;
    NetMultiSetupMenuItems[27].d.pulldown = i27;
    NetMultiSetupMenuItems[28].d.text     = i28;
    NetMultiSetupMenuItems[29].d.pulldown = i29;
    NetMultiSetupMenuItems[30].d.gem      = i30;
    NetMultiSetupMenuItems[31].d.gem      = i31;
    NetMultiSetupMenuItems[32].d.gem      = i32;
    NetMultiSetupMenuItems[33].d.gem      = i33;
    NetMultiSetupMenuItems[34].d.gem      = i34;
    NetMultiSetupMenuItems[35].d.gem      = i35;
    NetMultiSetupMenuItems[36].d.gem      = i36;
    NetMultiSetupMenuItems[37].d.gem      = i37;
    NetMultiSetupMenuItems[38].d.gem      = i38;
    NetMultiSetupMenuItems[39].d.gem      = i39;
    NetMultiSetupMenuItems[40].d.gem      = i40;
    NetMultiSetupMenuItems[41].d.gem      = i41;
    NetMultiSetupMenuItems[42].d.gem      = i42;
    NetMultiSetupMenuItems[43].d.gem      = i43;
    NetMultiSetupMenuItems[44].d.gem      = i44;
    NetMultiSetupMenuItems[45].d.gem      = i45;
    NetMultiSetupMenuItems[46].d.gem      = i46;
    NetMultiSetupMenuItems[47].d.gem      = i47;
    NetMultiSetupMenuItems[48].d.gem      = i48;
    NetMultiSetupMenuItems[49].d.gem      = i49;
    NetMultiSetupMenuItems[50].d.gem      = i50;
    NetMultiSetupMenuItems[51].d.gem      = i51;
    NetMultiSetupMenuItems[52].d.gem      = i52;
    NetMultiSetupMenuItems[53].d.gem      = i53;
    NetMultiSetupMenuItems[54].d.gem      = i54;
    NetMultiSetupMenuItems[55].d.gem      = i55;
    NetMultiSetupMenuItems[56].d.gem      = i56;
    NetMultiSetupMenuItems[57].d.gem      = i57;
}

/**
**	Multi player client game menu.
*/
local Menuitem NetMultiClientMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, MultiGameClientInit, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 640/2+12, 8, 0, LargeFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 640-224-16, 360, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36, 0, LargeFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 640-224-16, 360+36+36, 0, LargeFont, NULL, NULL, {{NULL,0}} },

    // 8+7 player slots
#define CLIENT_PLAYER_STATE	5
    { MI_TYPE_PULLDOWN, 40, 32, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*2, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*3, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*4, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*5, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*6, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*7, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*2, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*3, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*4, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*5, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*6, 0, GameFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 40, 10+240-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
#define CLIENT_RACE	21
    { MI_TYPE_PULLDOWN, 40, 10+240, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+240-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
#define CLIENT_RESOURCE	23
    { MI_TYPE_PULLDOWN, 220, 10+240, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640-224-16, 10+240-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
#define CLIENT_UNITS	25
    { MI_TYPE_PULLDOWN, 640-224-16, 10+240, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+300-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
#define CLIENT_FOG_OF_WAR	27
    { MI_TYPE_PULLDOWN, 40, 10+300, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+300-20, 0, GameFont, NULL, NULL, {{NULL,0}} },
#define CLIENT_TILESET	29
    { MI_TYPE_PULLDOWN, 220, 10+300, 0, GameFont, NULL, NULL, {{NULL,0}} },

    // 7+7 player state buttons
#define CLIENT_PLAYER_READY	30
    { MI_TYPE_GEM, 10, 32+22*1, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*2, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*3, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*4, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*6, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*7, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*0, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*1, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*2, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*3, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*4, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*6, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    //{ MI_TYPE_GEM, 330, 32+22*7, 0, LargeFont, NULL, NULL,
    //	{ gem:{ 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction} } },
};
local void InitNetMultiClientMenuItems() {
    MenuitemDrawfunc i0 = { MultiGameClientDrawFunc };

    MenuitemText     i1 = { "~<Multi Player Game~>", MI_TFLAGS_CENTERED};

    MenuitemButton   i2 = { "~!Ready", 224, 27, MBUTTON_GM_FULL, MultiClientReady, 'r'};
    MenuitemButton   i3 = { "~!Not Ready", 224, 27, MBUTTON_GM_FULL, MultiClientNotReady, 'n'};

    MenuitemButton   i4 = { "~!Cancel Game", 224, 27, MBUTTON_GM_FULL, MultiClientCancel, 'c'};

    MenuitemPulldown i5 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i6 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i7 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i8 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i9 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i10 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i11 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i12 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i13 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i14 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i15 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i16 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i17 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i18 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i19 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};

    MenuitemText     i20 = { "~<Your Race:~>", 0};
    MenuitemPulldown i21 = { rcsoptions, 152, 20, MBUTTON_PULLDOWN, MultiClientRCSAction, 3, 2, 2, 0, 0};
    MenuitemText     i22 = { "~<Resources:~>", 0};
    MenuitemPulldown i23 = { resoptions, 152, 20, MBUTTON_PULLDOWN, GameRESAction, 4, 0, 0, 0, MI_PSTATE_PASSIVE};
    MenuitemText     i24 = { "~<Units:~>", 0};
    MenuitemPulldown i25 = { unsoptions, 190, 20, MBUTTON_PULLDOWN, GameUNSAction, 2, 0, 0, 0, MI_PSTATE_PASSIVE};
    MenuitemText     i26 = { "~<Fog of War:~>", 0};
    MenuitemPulldown i27 = { mgfwsoptions, 152, 20, MBUTTON_PULLDOWN, MultiGameFWSAction, 2, 0, 0, 0, MI_PSTATE_PASSIVE};
    MenuitemText     i28 = { "~<Map Tileset:~>", 0};
    MenuitemPulldown i29 = { tssoptions, 152, 20, MBUTTON_PULLDOWN, GameTSSAction, 5, 0, 0, 0, MI_PSTATE_PASSIVE};

    MenuitemGem      i30 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i31 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i32 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i33 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i34 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i35 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i36 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i37 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i38 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i39 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i40 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i41 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i42 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i43 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};

    NetMultiClientMenuItems[0].d.drawfunc  = i0;
    NetMultiClientMenuItems[1].d.text      = i1;
    NetMultiClientMenuItems[2].d.button    = i2;
    NetMultiClientMenuItems[3].d.button    = i3;
    NetMultiClientMenuItems[4].d.button    = i4;
    NetMultiClientMenuItems[5].d.pulldown  = i5;
    NetMultiClientMenuItems[6].d.pulldown  = i6;
    NetMultiClientMenuItems[7].d.pulldown  = i7;
    NetMultiClientMenuItems[8].d.pulldown  = i8;
    NetMultiClientMenuItems[9].d.pulldown  = i9;
    NetMultiClientMenuItems[10].d.pulldown = i10;
    NetMultiClientMenuItems[11].d.pulldown = i11;
    NetMultiClientMenuItems[12].d.pulldown = i12;
    NetMultiClientMenuItems[13].d.pulldown = i13;
    NetMultiClientMenuItems[14].d.pulldown = i14;
    NetMultiClientMenuItems[15].d.pulldown = i15;
    NetMultiClientMenuItems[16].d.pulldown = i16;
    NetMultiClientMenuItems[17].d.pulldown = i17;
    NetMultiClientMenuItems[18].d.pulldown = i18;
    NetMultiClientMenuItems[19].d.pulldown = i19;
    NetMultiClientMenuItems[20].d.text     = i20;
    NetMultiClientMenuItems[21].d.pulldown = i21;
    NetMultiClientMenuItems[22].d.text     = i22;
    NetMultiClientMenuItems[23].d.pulldown = i23;
    NetMultiClientMenuItems[24].d.text     = i24;
    NetMultiClientMenuItems[25].d.pulldown = i25;
    NetMultiClientMenuItems[26].d.text     = i26;
    NetMultiClientMenuItems[27].d.pulldown = i27;
    NetMultiClientMenuItems[28].d.text     = i28;
    NetMultiClientMenuItems[29].d.pulldown = i29;
    NetMultiClientMenuItems[30].d.gem      = i30;
    NetMultiClientMenuItems[31].d.gem      = i31;
    NetMultiClientMenuItems[32].d.gem      = i32;
    NetMultiClientMenuItems[33].d.gem      = i33;
    NetMultiClientMenuItems[34].d.gem      = i34;
    NetMultiClientMenuItems[35].d.gem      = i35;
    NetMultiClientMenuItems[36].d.gem      = i36;
    NetMultiClientMenuItems[37].d.gem      = i37;
    NetMultiClientMenuItems[38].d.gem      = i38;
    NetMultiClientMenuItems[39].d.gem      = i39;
    NetMultiClientMenuItems[40].d.gem      = i40;
    NetMultiClientMenuItems[41].d.gem      = i41;
    NetMultiClientMenuItems[42].d.gem      = i42;
    NetMultiClientMenuItems[43].d.gem      = i43;
}

local Menuitem NetErrorMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 38, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 92, 80, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitNetErrorMenuItems() {
    MenuitemText   i0 = { "Error:", MI_TFLAGS_CENTERED};
    MenuitemText   i1 = { NULL, MI_TFLAGS_CENTERED};
    MenuitemButton i2 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};
    NetErrorMenuItems[0].d.text   = i0;
    NetErrorMenuItems[1].d.text   = i1;
    NetErrorMenuItems[2].d.button = i2;
}

/**
**	Items for the Connecting Network Menu
*/
local Menuitem ConnectingMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 53, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 90, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitConnectingMenuItems() {
    MenuitemText   i0 = { "Connecting to server", MI_TFLAGS_CENTERED};
    MenuitemText   i1 = { NetServerText, MI_TFLAGS_CENTERED};
    MenuitemText   i2 = { NetTriesText , MI_TFLAGS_CENTERED};
    MenuitemButton i3 = { "~!Cancel", 224, 27, MBUTTON_GM_FULL, NetConnectingCancel, 'c'};
    ConnectingMenuItems[0].d.text   = i0;
    ConnectingMenuItems[1].d.text   = i1;
    ConnectingMenuItems[2].d.text   = i2;
    ConnectingMenuItems[3].d.button = i3;
}

/**
**	Items for the Campaign Select Menu
*/
local Menuitem CampaignSelectMenuItems[] = {
    { MI_TYPE_BUTTON, 208, 212 + 36 * 0, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 1, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 2, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 3, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 4, MenuButtonDisabled, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitCampaignSelectMenuItems() {
    MenuitemButton i0 = { NULL, 224, 27, MBUTTON_GM_FULL, CampaignMenu1, 'a'};
    MenuitemButton i1 = { NULL, 224, 27, MBUTTON_GM_FULL, CampaignMenu2, 'm'};
    MenuitemButton i2 = { NULL, 224, 27, MBUTTON_GM_FULL, CampaignMenu3, 'l'};
    MenuitemButton i3 = { NULL, 224, 27, MBUTTON_GM_FULL, CampaignMenu4, 'y'};
    MenuitemButton i4 = { "~!Select Campaign", 224, 27, MBUTTON_GM_FULL, SelectCampaignMenu, 's'};
    MenuitemButton i5 = { "~!Previous Menu", 224, 27, MBUTTON_GM_FULL, EndMenu, 'p'};
    CampaignSelectMenuItems[0].d.button = i0;
    CampaignSelectMenuItems[1].d.button = i1;
    CampaignSelectMenuItems[2].d.button = i2;
    CampaignSelectMenuItems[3].d.button = i3;
    CampaignSelectMenuItems[4].d.button = i4;
    CampaignSelectMenuItems[5].d.button = i5;
}

/**
**	Items for the Campaign Continue Menu
*/
local Menuitem CampaignContMenuItems[] = {
    { MI_TYPE_BUTTON, 508, 320 + 36 + 36 + 36, 0, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitCampaignContMenuItems() {
    MenuitemButton i0 = { "~!Continue", 106, 27, MBUTTON_GM_HALF, EndMenu, 'c'};
    CampaignContMenuItems[0].d.button = i0;
}

local Menuitem SoundOptionsMenuItems[] = {
    { MI_TYPE_TEXT, 176, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 16, 36*1, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*1.5, 0, 0, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 44, 36*2 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 218, 36*2 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 240, 36*1.5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 266, 36*1.5 + 2, 0, GameFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 16, 36*3, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*3.5, 0, 0, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 44, 36*4 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 218, 36*4 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 240, 36*3.5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 266, 36*3.5 + 2, 0, GameFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 16, 36*5, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*5.5, 0, 0, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 44, 36*6 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 218, 36*6 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 240, 36*5.5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 266, 36*5.5 + 2, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 32, 36*6.5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 58, 36*6.5 + 2, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 154, 36*6.5, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 180, 36*6.5 + 2, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 176 - (106 / 2), 352 - 11 - 27, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitSoundOptionsMenuItems() {
    MenuitemText    i0  = { "Sound Options", MI_TFLAGS_CENTERED};

    MenuitemText    i1  = { "Master Volume", MI_TFLAGS_LALIGN};
    MenuitemHslider i2  = { 0, 11*18, 18, MasterVolumeHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i3  = { "min", MI_TFLAGS_CENTERED};
    MenuitemText    i4  = { "max", MI_TFLAGS_CENTERED};
    MenuitemGem     i5  = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetMasterPower};
    MenuitemText    i6  = { "Enabled", MI_TFLAGS_LALIGN};

    MenuitemText    i7  = { "Music Volume", MI_TFLAGS_LALIGN};
    MenuitemHslider i8  = { 0, 11*18, 18, MusicVolumeHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i9  = { "min", MI_TFLAGS_CENTERED};
    MenuitemText    i10 = { "max", MI_TFLAGS_CENTERED};
    MenuitemGem     i11 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetMusicPower};
    MenuitemText    i12 = { "Enabled", MI_TFLAGS_LALIGN};

    MenuitemText    i13 = { "CD Volume", MI_TFLAGS_LALIGN};
    MenuitemHslider i14 = { 0, 11*18, 18, CdVolumeHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i15 = { "min", MI_TFLAGS_CENTERED};
    MenuitemText    i16 = { "max", MI_TFLAGS_CENTERED};
    MenuitemGem     i17 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetCdPower};
    MenuitemText    i18 = { "Enabled", MI_TFLAGS_LALIGN};
    MenuitemGem     i19 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_ROUND, SetCdModeAll};
    MenuitemText    i20 = { "All Tracks", MI_TFLAGS_LALIGN};
    MenuitemGem     i21 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_ROUND, SetCdModeRandom};
    MenuitemText    i22 = { "Random Tracks", MI_TFLAGS_LALIGN};
    MenuitemButton  i23 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};

    SoundOptionsMenuItems[0].d.text     = i0;
    SoundOptionsMenuItems[1].d.text     = i1;
    SoundOptionsMenuItems[2].d.hslider  = i2;
    SoundOptionsMenuItems[3].d.text     = i3;
    SoundOptionsMenuItems[4].d.text     = i4;
    SoundOptionsMenuItems[5].d.gem      = i5;
    SoundOptionsMenuItems[6].d.text     = i6;
    SoundOptionsMenuItems[7].d.text     = i7;
    SoundOptionsMenuItems[8].d.hslider  = i8;
    SoundOptionsMenuItems[9].d.text     = i9;
    SoundOptionsMenuItems[10].d.text    = i10;
    SoundOptionsMenuItems[11].d.gem     = i11;
    SoundOptionsMenuItems[12].d.text    = i12;
    SoundOptionsMenuItems[13].d.text    = i13;
    SoundOptionsMenuItems[14].d.hslider = i14;
    SoundOptionsMenuItems[15].d.text    = i15;
    SoundOptionsMenuItems[16].d.text    = i16;
    SoundOptionsMenuItems[17].d.gem     = i17;
    SoundOptionsMenuItems[18].d.text    = i18;
    SoundOptionsMenuItems[19].d.gem     = i19;
    SoundOptionsMenuItems[20].d.text    = i20;
    SoundOptionsMenuItems[21].d.gem     = i21;
    SoundOptionsMenuItems[22].d.text    = i22;
    SoundOptionsMenuItems[23].d.button  = i23;
}

local Menuitem PreferencesMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 16, 36*1, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 46, 36*1 + 2, 0, GameFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 16, 36*2, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 46, 36*2 + 2, 0, GameFont, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 128 - (106 / 2), 245, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitPreferencesMenuItems() {
    MenuitemText   i0 = { "Preferences", MI_TFLAGS_CENTERED};
    MenuitemGem    i1 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetFogOfWar};
    MenuitemText   i2 = { "Fog of War Enabled", MI_TFLAGS_LALIGN};
    MenuitemGem    i3 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetCommandKey};
    MenuitemText   i4 = { "Show command key", MI_TFLAGS_LALIGN};
    MenuitemButton i5 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};
    PreferencesMenuItems[0].d.text   = i0;
    PreferencesMenuItems[1].d.gem    = i1;
    PreferencesMenuItems[2].d.text   = i2;
    PreferencesMenuItems[3].d.gem    = i3;
    PreferencesMenuItems[4].d.text   = i4;
    PreferencesMenuItems[5].d.button = i5;
}

local Menuitem SpeedSettingsMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 16, 36*1, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*1.5, 0, 0, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 34, 36*2 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 230, 36*2 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 16, 36*3, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*3.5, 0, 0, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 34, 36*4 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 230, 36*4 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 16, 36*5, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*5.5, 0, 0, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 34, 36*6 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 230, 36*6 + 6, 0, SmallFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 128 - (106 / 2), 245, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitSpeedSettingsMenuItems() {
    MenuitemText    i0  = { "Speed Settings", MI_TFLAGS_CENTERED};
    MenuitemText    i1  = { "Game Speed", MI_TFLAGS_LALIGN};
    MenuitemHslider i2  = { 0, 11*18, 18, GameSpeedHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i3  = { "slow", MI_TFLAGS_LALIGN};
    MenuitemText    i4  = { "fast", MI_TFLAGS_RALIGN};
    MenuitemText    i5  = { "Mouse Scroll", MI_TFLAGS_LALIGN};
    MenuitemHslider i6  = { 0, 11*18, 18, MouseScrollHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i7  = { "off", MI_TFLAGS_LALIGN};
    MenuitemText    i8  = { "fast", MI_TFLAGS_RALIGN};
    MenuitemText    i9  = { "Keyboard Scroll", MI_TFLAGS_LALIGN};
    MenuitemHslider i10 = { 0, 11*18, 18, KeyboardScrollHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i11 = { "off", MI_TFLAGS_LALIGN};
    MenuitemText    i12 = { "fast", MI_TFLAGS_RALIGN};
    MenuitemButton  i13 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};
    SpeedSettingsMenuItems[0].d.text     = i0;
    SpeedSettingsMenuItems[1].d.text     = i1;
    SpeedSettingsMenuItems[2].d.hslider  = i2;
    SpeedSettingsMenuItems[3].d.text     = i3;
    SpeedSettingsMenuItems[4].d.text     = i4;
    SpeedSettingsMenuItems[5].d.text     = i5;
    SpeedSettingsMenuItems[6].d.hslider  = i6;
    SpeedSettingsMenuItems[7].d.text     = i7;
    SpeedSettingsMenuItems[8].d.text     = i8;
    SpeedSettingsMenuItems[9].d.text     = i9;
    SpeedSettingsMenuItems[10].d.hslider = i10;
    SpeedSettingsMenuItems[11].d.text    = i11;
    SpeedSettingsMenuItems[12].d.text    = i12;
    SpeedSettingsMenuItems[13].d.button  = i13;
}

local Menuitem GameOptionsMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },
#ifdef WITH_SOUND
    { MI_TYPE_BUTTON, 16, 40 + 36*0, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
#else
    { MI_TYPE_BUTTON, 16, 40 + 36*0, MenuButtonDisabled, LargeFont, NULL, NULL, {{NULL,0}} },
#endif
    { MI_TYPE_BUTTON, 16, 40 + 36*1, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*2, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 128 - (224 / 2), 245, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitGameOptionsMenuItems() {
    MenuitemText   i0 = { "Game Options", MI_TFLAGS_CENTERED};
#ifdef WITH_SOUND
    MenuitemButton i1 = { "Sound (~!F~!7)", 224, 27, MBUTTON_GM_FULL, SoundOptions, KeyCodeF7};
#else
    MenuitemButton i1 = { "Sound (~!F~!7)", 224, 27, MBUTTON_GM_FULL, SoundOptions, KeyCodeF7};
#endif
    MenuitemButton i2 = { "Speeds (~!F~!8)", 224, 27, MBUTTON_GM_FULL, SpeedSettings, KeyCodeF8};
    MenuitemButton i3 = { "Preferences (~!F~!9)", 224, 27, MBUTTON_GM_FULL, Preferences, KeyCodeF9};
    MenuitemButton i4 = { "Previous (~!E~!s~!c)", 224, 27, MBUTTON_GM_FULL, EndMenu, '\033'};
    GameOptionsMenuItems[0].d.text   = i0;
    GameOptionsMenuItems[1].d.button = i1;
    GameOptionsMenuItems[2].d.button = i2;
    GameOptionsMenuItems[3].d.button = i3;
    GameOptionsMenuItems[4].d.button = i4;
}

local Menuitem HelpMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*0, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*1, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 128 - (224 / 2), 288-40, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitHelpMenuItems() {
    MenuitemText   i0 = { "Help Menu", MI_TFLAGS_CENTERED};
    MenuitemButton i1 = { "Keystroke ~!Help", 224, 27, MBUTTON_GM_FULL, KeystrokeHelpMenu, 'h'};
    MenuitemButton i2 = { "Freecraft ~!Tips", 224, 27, MBUTTON_GM_FULL, ShowTipsMenu, 't'};
    MenuitemButton i3 = { "Previous (~!E~!s~!c)", 224, 27, MBUTTON_GM_FULL, EndMenu, '\033'};
    HelpMenuItems[0].d.text   = i0;
    HelpMenuItems[1].d.button = i1;
    HelpMenuItems[2].d.button = i2;
    HelpMenuItems[3].d.button = i3;
}

local Menuitem KeystrokeHelpMenuItems[] = {
    { MI_TYPE_TEXT, 352/2, 11, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 352 - 18 - 16, 40+20, 0, 0, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 352/2 - (224 / 2), 352-40, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_DRAWFUNC, 16, 40+20, 0, GameFont, NULL, NULL, {{NULL,0}} },
};
// FIXME: ccl these...
// FIXME: add newer helps
local unsigned char *keystrokehelptexts[] = {
    "Alt-F  - toggle full screen",
    "Alt-G  - toggle grab mouse",
    "Ctrl-S - mute sound",
    "Ctrl-M - mute music (NOT SUPPORTED)",
    "+      - increase game speed",
    "-      - decrease game speed",
    "Ctrl-P - pause game",
    "PAUSE  - pause game",
    "PRINT  - make screen shot",
    "Alt-H  - help menu",
    "Alt-R  - restart scenario (NOT SUPPORTED)",
    "Alt-Q  - quit to main menu",
    "Alt-X  - quit game",
    "Alt-B  - toggle expand map",
    "Alt-M  - game menu",
    "ENTER  - write a message",
    "SPACE  - goto last event",
    "TAB    - hide/unhide terrain",
    "Alt-I  - find idle peon",
    "Alt-C  - center on selected unit",
    "Alt-V  - next view port",
    "Ctrl-V - previous view port",
    "^      - select nothing",
    "#      - select group",
    "##     - center on group",
    "Ctrl-# - define group",
    "Shift-#- add to group",
    "Alt-#  - add to alternate group",
    "F2-F4  - recall map position",
    "Shift F2-F4 - save map postition",
    "F5     - game options",
    "F7     - sound options",
    "F8     - speed options",
    "F9     - preferences",
    "F10    - game menu",
    "F11    - save game",
    "F12    - load game",
};

local void InitKeystrokeHelpMenuItems() {
    MenuitemText     i0 = { "Keystroke Help Menu", MI_TFLAGS_CENTERED};
    MenuitemVslider  i1 = { 0, 18, 12*18, KeystrokeHelpVSAction, -1, 0, 0, 0, NULL};
    MenuitemButton   i2 = { "Previous (~!E~!s~!c)", 224, 27, MBUTTON_GM_FULL, EndMenu, '\033'};
    MenuitemDrawfunc i3 = { KeystrokeHelpDrawFunc };
    KeystrokeHelpMenuItems[0].d.text     = i0;
    KeystrokeHelpMenuItems[1].d.vslider  = i1;
    KeystrokeHelpMenuItems[2].d.button   = i2;
    KeystrokeHelpMenuItems[3].d.drawfunc = i3;
}

local Menuitem SaveGameMenuItems[] = {
    { MI_TYPE_TEXT, 384/2, 11, 0, LargeFont, CreateSaveDir, NULL, {{NULL, 0}} },
    { MI_TYPE_INPUT, 16, 11+36*1, 0, GameFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_LISTBOX, 16, 11+36*1.5, 0, GameFont, SaveLBInit, SaveLBExit, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 384-16-16, 11+36*1.5, 0, 0, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 2*384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 3*384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitSaveGameMenuItems() {
    MenuitemText    i0 = { "Save Game", MI_TFLAGS_CENTERED};
    MenuitemInput   i1 = { NULL, 384-16-16, 16, MBUTTON_PULLDOWN, EnterSaveGameAction, 0, 0};
    MenuitemListbox i2 = { NULL, 384-16-16-16, 7*18, MBUTTON_PULLDOWN, SaveLBAction, 0, 0, 0, 0, 7, 0,
			   (void *)SaveLBRetrieve, ScenSelectOk};
    MenuitemVslider i3 = { 0, 18, 7*18, SaveVSAction, -1, 0, 0, 0, SaveOk};
    MenuitemButton  i4 = { "~!Save", 106, 27, MBUTTON_GM_HALF, SaveAction, 's'};
    MenuitemButton  i5 = { "~!Delete", 106, 27, MBUTTON_GM_HALF, FcDeleteMenu, 'd'};
    MenuitemButton  i6 = { "~!Cancel", 106, 27, MBUTTON_GM_HALF, EndMenu, 'c'};
    SaveGameMenuItems[0].d.text    = i0;
    SaveGameMenuItems[1].d.input   = i1;
    SaveGameMenuItems[2].d.listbox = i2;
    SaveGameMenuItems[3].d.vslider = i3;
    SaveGameMenuItems[4].d.button  = i4;
    SaveGameMenuItems[5].d.button  = i5;
    SaveGameMenuItems[6].d.button  = i6;
}

local Menuitem LoadGameMenuItems[] = {
    { MI_TYPE_TEXT, 384/2, 11, 0, LargeFont, CreateSaveDir, NULL, {{NULL, 0}} },
    { MI_TYPE_LISTBOX, 16, 11+36*1.5, 0, GameFont, LoadLBInit, LoadLBExit, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 384-16-16, 11+36*1.5, 0, 0, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 3*384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitLoadGameMenuItems() {
    MenuitemText    i0 = { "Load Game", MI_TFLAGS_CENTERED};
    MenuitemListbox i1 = { NULL, 384-16-16-16, 7*18, MBUTTON_PULLDOWN, LoadLBAction, 0, 0, 0, 0, 7, 0,
			   (void *)LoadLBRetrieve, ScenSelectOk};
    MenuitemVslider i2 = { 0, 18, 7*18, LoadVSAction, -1, 0, 0, 0, LoadOk};
    MenuitemButton  i3 = { "~!Load", 108, 27, MBUTTON_GM_HALF, LoadAction, 'l'};
    MenuitemButton  i4 = { "~!Cancel", 108, 27, MBUTTON_GM_HALF, EndMenu, 'c'};
    LoadGameMenuItems[0].d.text    = i0;
    LoadGameMenuItems[1].d.listbox = i1;
    LoadGameMenuItems[2].d.vslider = i2;
    LoadGameMenuItems[3].d.button  = i3;
    LoadGameMenuItems[4].d.button  = i4;
}

local Menuitem ConfirmSaveMenuItems[] = {
    { MI_TYPE_TEXT, 288/2, 11, 0, LargeFont, ConfirmSaveInit, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*1.5, 0, GameFont, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*2.5, 0, GameFont, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_BUTTON, 16, 128-27*1.5, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 288-16-106, 128-27*1.5, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitConfirmSaveMenuItems() {
    MenuitemText    i0 = { "Overwrite File", MI_TFLAGS_CENTERED};
    MenuitemText    i1 = { "Are you sure you want to overwrite", MI_TFLAGS_LALIGN};
    MenuitemText    i2 = { NULL, MI_TFLAGS_LALIGN};
    MenuitemButton  i3 = { "~!Ok", 106, 27, MBUTTON_GM_HALF, ConfirmSaveFile, 'o'};
    MenuitemButton  i4 = { "~!Cancel", 106, 27, MBUTTON_GM_HALF, EndMenu, 'c'};
    ConfirmSaveMenuItems[0].d.text = i0;
    ConfirmSaveMenuItems[1].d.text = i1;
    ConfirmSaveMenuItems[2].d.text = i2;
    ConfirmSaveMenuItems[3].d.button = i3;
    ConfirmSaveMenuItems[4].d.button = i4;
}

local Menuitem ConfirmDeleteMenuItems[] = {
    { MI_TYPE_TEXT, 288/2, 11, 0, LargeFont, FcDeleteInit, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*1.5, 0, GameFont, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*2.5, 0, GameFont, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_BUTTON, 16, 128-27*1.5, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 288-16-106, 128-27*1.5, MenuButtonSelected, LargeFont, NULL, NULL, {{NULL,0}} },
};
local void InitConfirmDeleteMenuItems() {
    MenuitemText    i0 = { "Delete File", MI_TFLAGS_CENTERED};
    MenuitemText    i1 = { "Are you sure you want to delete", MI_TFLAGS_LALIGN};
    MenuitemText    i2 = { NULL, MI_TFLAGS_LALIGN};
    MenuitemButton  i3 = { "~!Ok", 106, 27, MBUTTON_GM_HALF, FcDeleteFile, 'o'};
    MenuitemButton  i4 = { "~!Cancel", 106, 27, MBUTTON_GM_HALF, EndMenu, 'c'};
    ConfirmDeleteMenuItems[0].d.text = i0;
    ConfirmDeleteMenuItems[1].d.text = i1;
    ConfirmDeleteMenuItems[2].d.text = i2;
    ConfirmDeleteMenuItems[3].d.button = i3;
    ConfirmDeleteMenuItems[4].d.button = i4;
}

/**
**	FIXME: Ari please look, this is now in TheUI.
*/
enum {
    ImageNone,
    ImagePanel1,	// 256 x 288
    ImagePanel2,	// 288 x 256
    ImagePanel3,	// 384 x 256
    ImagePanel4,	// 288 x 128
    ImagePanel5,	// 352 x 352
};

/**
**	Menus
*/
global Menu Menus[] = {
    {
	// Game Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	7, 8,
	GameMenuItems,
	NULL,
    },
    {
	// Victory Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-128)/2,
	288, 128,
	ImagePanel4,
	2, 4,
	VictoryMenuItems,
	NULL,
    },
    {
	// Lost Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-128)/2,
	288, 128,
	ImagePanel4,
	2, 3,
	LostMenuItems,
	NULL,
    },
    {
	// SelectScen Menu
	(640-352)/2,
	(480-352)/2,
	352, 352,
	ImagePanel5,
	4, 10,
	ScenSelectMenuItems,
	NULL,
    },
    {
	// PrgStart Menu
	0,
	0,
	640, 480,
	ImageNone,
	1, 9,
	PrgStartMenuItems,
	NULL,
    },
    {
	// CustomGame Menu
	0,
	0,
	640, 480,
	ImageNone,
	3, 17,
	CustomGameMenuItems,
	NULL,
    },
    {
	// Enter Name Menu
	(640-288)/2,
	260,
	288, 128,
	ImagePanel4,
	2, 4,
	EnterNameMenuItems,
	NULL,
    },
    {
	// Net Create Join Menu
	0,
	0,
	640, 480,
	ImageNone,
	2, 3,
	NetCreateJoinMenuItems,
	NULL,
    },
    {
	// Multi Player Setup Menu
	0,
	0,
	640, 480,
	ImageNone,
	3, 58,
	NetMultiSetupMenuItems,
	NULL,
    },
    {
	// Enter Server Menu
	(640-288)/2,
	260,
	288, 128,
	ImagePanel4,
	3, 4,
	EnterServerIPMenuItems,
	NULL,
    },
    {
	// Multi Player Client Menu
	0,
	0,
	640, 480,
	ImageNone,
	4, 44,
	NetMultiClientMenuItems,
	TerminateNetConnect,
    },
    {
	// Connecting Menu
	(640-288)/2,
	260,
	288, 128,
	ImagePanel4,
	2, 4,
	ConnectingMenuItems,
	TerminateNetConnect,
    },
    {
	// Campaign Select Menu
	0,
	0,
	640, 480,
	ImageNone,
	0, 6,
	CampaignSelectMenuItems,
	NULL,
    },
    {
	// Campaign Continue Menu
	0,
	0,
	640, 480,
	ImageNone,
	0, 1,
	CampaignContMenuItems,
	NULL,
    },
    {
	// Objectives Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	10, 11,
	ObjectivesMenuItems,
	NULL,
    },
    {
	// End Scenario Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	5, 6,
	EndScenarioMenuItems,
	NULL,
    },
    {
	// Sound Options Menu
	176+(14*TileSizeX-352)/2,
	16+(14*TileSizeY-352)/2,
	352, 352,
	ImagePanel5,
	24, 24,
	SoundOptionsMenuItems,
	NULL,
    },
    {
	// Preferences Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	6, 6,
	PreferencesMenuItems,
	NULL,
    },
    {
	// Speed Settings Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	14, 14,
	SpeedSettingsMenuItems,
	NULL,
    },
    {
	// Game Options Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	4, 5,
	GameOptionsMenuItems,
	NULL,
    },
    {
	// Net Errors Menu
	(640-288)/2,
	260,
	288, 128,
	ImagePanel4,
	2, 3,
	NetErrorMenuItems,
	NULL,
    },
    {
	// Tips Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-256)/2,
	288, 256,
	ImagePanel2,
	4, 13,
	TipsMenuItems,
	NULL,
    },
    {
	// Help Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	3, 4,
	HelpMenuItems,
	NULL,
    },
    {
	// Keystroke Help Menu
	176+(14*TileSizeX-352)/2,
	16+(14*TileSizeY-352)/2,
	352, 352,
	ImagePanel5,
	2, 4,
	KeystrokeHelpMenuItems,
	NULL,
    },
    {
	// Save Menu
	176+(14*TileSizeX-384)/2,
	16+(14*TileSizeY-256)/2,
	384, 256,
	ImagePanel3,
	6, 7,
	SaveGameMenuItems,
	NULL,
    },
    {
	// Load Menu
	176+(14*TileSizeX-384)/2,
	16+(14*TileSizeY-256)/2,
	384, 256,
	ImagePanel3,
	4, 5,
	LoadGameMenuItems,
	NULL,
    },
    {
	// Confirm Save Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-256)/2,
	288, 128,
	ImagePanel4,
	1, 5,
	ConfirmSaveMenuItems,
	NULL,
    },
    {
	// Confirm Delete Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-256)/2,
	288, 128,
	ImagePanel4,
	1, 5,
	ConfirmDeleteMenuItems,
	NULL,
    },
};

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Draw menu button 'button' on x,y
**
**	@param button	Button identifier
**	@param flags	State of Button (clicked, mouse over...)
**	@param w	Button width (for border)
**	@param h	Button height (for border)
**	@param x	X display position
**	@param y	Y display position
**	@param font	font number for text
**	@param text	text to print on button
*/
global void DrawMenuButton(MenuButtonId button,unsigned flags,unsigned w,unsigned h,unsigned x,unsigned y,
	const int font,const unsigned char *text)
{
    MenuButtonId rb;
    int s, nc, rc;

    GetDefaultTextColors(&nc, &rc);
    if (flags&MenuButtonDisabled) {
	rb = button - 1;
	s = 0;
	SetDefaultTextColors(FontGrey,FontGrey);
    } else if (flags&MenuButtonClicked) {
	rb = button + 1;
	s = 2;
	SetDefaultTextColors(rc,rc);
    } else {
	rb = button;
	s = 0;
	if (flags&MenuButtonActive) {
	    SetDefaultTextColors(rc,rc);
	}
    }
    if (rb < MenuButtonGfx.Sprite->NumFrames) {
	VideoDraw(MenuButtonGfx.Sprite, rb, x, y);
    } else {
	if (rb < button) {
	    VideoDrawRectangleClip(ColorGray,x+1,y+1,w-2,h-2);
	    VideoDrawRectangleClip(ColorGray,x+2,y+2,w-4,h-4);
	} else {
	    // FIXME: Temp-workaround for missing folder button in non-expansion gfx
	    VideoDrawRectangleClip(ColorYellow,x+1,y+1,w-2,h-2);
	    VideoDrawRectangleClip(ColorYellow,x+2,y+2,w-4,h-4);
	}
    }
    if (text) {
	if (button != MBUTTON_FOLDER) {
	    VideoDrawTextCentered(s+x+w/2,s+y+(font == GameFont ? 4 : 7),font,text);
	} else {
	    SetDefaultTextColors(nc,rc);
	    VideoDrawText(x+44,y+6,font,text);
	}
    }
    if (flags&MenuButtonSelected) {
	if (flags&MenuButtonDisabled) {
	    VideoDrawRectangleClip(ColorGray,x,y,w-1,h);
	} else {
	    VideoDrawRectangleClip(ColorYellow,x,y,w-1,h);
	}
    }
    SetDefaultTextColors(nc,rc);
}

/**
**	Draw pulldown 'button' on menu mx, my
**
**	@param mi	menuitem pointer
**	@param mx	menu X display position (offset)
**	@param my	menu Y display position (offset)
*/
local void DrawPulldown(Menuitem *mi, unsigned mx, unsigned my)
{
    int i, nc, rc;
    char *text;
    unsigned flags = mi->flags;
    MenuButtonId rb = mi->d.pulldown.button;
    unsigned w, h, x, y, oh;
    x = mx+mi->xofs;
    y = my+mi->yofs;
    w = mi->d.pulldown.xsize;
    oh = h = mi->d.pulldown.ysize - 2;

    GetDefaultTextColors(&nc, &rc);
    if (flags&MenuButtonClicked) {
	y -= mi->d.pulldown.curopt * h;
	i = mi->d.pulldown.noptions;
	h *= i;
	while (i--) {
	    PushClipping();
	    SetClipping(0,0,x+w,VideoHeight-1);
	    VideoDrawClip(MenuButtonGfx.Sprite, rb, x-1, y-1 + oh*i);
	    PopClipping();
	    text = mi->d.pulldown.options[i];
	    if (text) {
		if (i == mi->d.pulldown.cursel)
		    SetDefaultTextColors(rc,rc);
		else
		    SetDefaultTextColors(nc,rc);
		VideoDrawText(x+2,y+2 + oh*i ,mi->font,text);
	    }
	}
	w += 2;
    } else {
	h = mi->d.pulldown.ysize;
	y = my+mi->yofs;
	if (flags&MenuButtonDisabled) {
	    rb--;
	    SetDefaultTextColors(FontGrey,FontGrey);
	} else {
	    if (flags&MenuButtonActive) {
		SetDefaultTextColors(rc,rc);
	    }
	}

	PushClipping();
	if (!(mi->d.pulldown.state & MI_PSTATE_PASSIVE)) {
	    SetClipping(0,0,x+w-20,VideoHeight-1);
	} else {
	    SetClipping(0,0,x+w-1,VideoHeight-1);
	}
	VideoDrawClip(MenuButtonGfx.Sprite, rb, x-1, y-1);
	PopClipping();
	if (!(mi->d.pulldown.state & MI_PSTATE_PASSIVE)) {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW + rb - MBUTTON_PULLDOWN, x-1 + w-20, y-2);
	}
	text = mi->d.pulldown.options[mi->d.pulldown.curopt];
	if (text) {
	    VideoDrawText(x+2,y+2,mi->font,text);
	}
    }
    if (flags&MenuButtonSelected) {
	if (flags&MenuButtonDisabled) {
	    VideoDrawRectangleClip(ColorGray,x-2,y-2,w,h);
	} else {
	    VideoDrawRectangleClip(ColorYellow,x-2,y-2,w,h);
	}
    }
    SetDefaultTextColors(nc,rc);
}

/**
**	Draw listbox 'button' on menu mx, my
**
**	@param mi	menuitem pointer
**	@param mx	menu X display position (offset)
**	@param my	menu Y display position (offset)
*/
local void DrawListbox(Menuitem *mi, unsigned mx, unsigned my)
{
    int i, s, nc, rc;
    char *text;
    MenuButtonId rb = mi->d.listbox.button;
    unsigned flags = mi->flags;
    unsigned w, h, x, y;
    w = mi->d.listbox.xsize;
    h = mi->d.listbox.ysize;
    x = mx+mi->xofs;
    y = my+mi->yofs;

    GetDefaultTextColors(&nc, &rc);

    if (flags&MenuButtonDisabled) {
	rb--;
    }
    i = mi->d.listbox.nlines;
    s = mi->d.listbox.startline;
    while (i--) {
	PushClipping();
	SetClipping(0,0,x+w,VideoHeight-1);
	VideoDrawClip(MenuButtonGfx.Sprite, rb, x-1, y-1 + 18*i);
	PopClipping();
	if (!(flags&MenuButtonDisabled)) {
	    if (i < mi->d.listbox.noptions) {
		SetDefaultTextColors(nc,rc);
		text = (*mi->d.listbox.retrieveopt)(mi, i + s);
		if (text) {
		    if (i == mi->d.listbox.curopt)
			SetDefaultTextColors(rc,rc);
		    else
			SetDefaultTextColors(nc,rc);
		    VideoDrawText(x+2,y+2 + 18*i ,mi->font,text);
		}
	    }
	}
    }

    if (flags&MenuButtonSelected) {
	if (flags&MenuButtonDisabled) {
	    VideoDrawRectangleClip(ColorGray,x-2,y-2,w+1,h+2);
	} else {
	    VideoDrawRectangleClip(ColorYellow,x-2,y-2,w+1,h+2);
	}
    }
    SetDefaultTextColors(nc,rc);
}

/**
**	Draw vslider 'button' on menu mx, my
**
**	@param mi	menuitem pointer
**	@param mx	menu X display position (offset)
**	@param my	menu Y display position (offset)
*/
local void DrawVSlider(Menuitem *mi, unsigned mx, unsigned my)
{
    int p;
    unsigned flags = mi->flags;
    unsigned w, h, x, y;
    w = mi->d.vslider.xsize;
    h = mi->d.vslider.ysize;
    x = mx+mi->xofs;
    y = my+mi->yofs;

    if (flags&MenuButtonDisabled) {
	PushClipping();
	SetClipping(0,0,VideoWidth-1,y + h - 20);
	VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_VCONT - 1, x, y - 2);
	VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_VCONT - 1, x, y + h/2);
	PopClipping();
	VideoDraw(MenuButtonGfx.Sprite, MBUTTON_UP_ARROW - 1, x, y - 2);
	VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW - 1, x, y + h - 20);
    } else {
	PushClipping();
	SetClipping(0,0,VideoWidth-1,y + h - 20);
	VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_VCONT, x, y - 2);
	VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_VCONT, x, y + h/2);
	PopClipping();
	if (mi->d.vslider.cflags&MI_CFLAGS_UP) {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_UP_ARROW + 1, x, y - 2);
	} else {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_UP_ARROW, x, y - 2);
	}
	if (mi->d.vslider.cflags&MI_CFLAGS_DOWN) {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW + 1, x, y + h - 20);
	} else {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW, x, y + h - 20);
	}
	p = (mi->d.vslider.percent * (h - 54)) / 100;
	VideoDraw(MenuButtonGfx.Sprite, MBUTTON_S_KNOB, x + 1, y + 18 + p);
    }

    if (flags&MenuButtonSelected) {
	if (flags&MenuButtonDisabled) {
	    VideoDrawRectangleClip(ColorGray,x,y-2,w,h+2);
	} else {
	    VideoDrawRectangleClip(ColorYellow,x,y-2,w,h+2);
	}
    }

}

/**
**	Draw hslider 'button' on menu mx, my
**
**	@param mi	menuitem pointer
**	@param mx	menu X display position (offset)
**	@param my	menu Y display position (offset)
*/
local void DrawHSlider(Menuitem *mi, unsigned mx, unsigned my)
{
    int p;
    unsigned flags = mi->flags;
    unsigned w, h, x, y;
    w = mi->d.hslider.xsize;
    h = mi->d.hslider.ysize;
    x = mx+mi->xofs;
    y = my+mi->yofs;

    if (flags&MenuButtonDisabled) {
	PushClipping();
	SetClipping(0,0,x + w - 20,VideoHeight-1);
	VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_HCONT - 1, x - 2, y);
	VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_HCONT - 1, x + w/2, y);
	PopClipping();
	VideoDraw(MenuButtonGfx.Sprite, MBUTTON_LEFT_ARROW - 1, x - 2, y);
	VideoDraw(MenuButtonGfx.Sprite, MBUTTON_RIGHT_ARROW - 1, x + w - 20, y);
    } else {
	PushClipping();
	SetClipping(0,0,x + w - 20,VideoHeight-1);
	VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_HCONT, x - 2, y);
	VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_HCONT, x + w/2, y);
	PopClipping();
	if (mi->d.hslider.cflags&MI_CFLAGS_LEFT) {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_LEFT_ARROW + 1, x - 2, y);
	} else {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_LEFT_ARROW, x - 2, y);
	}
	if (mi->d.hslider.cflags&MI_CFLAGS_RIGHT) {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_RIGHT_ARROW + 1, x + w - 20, y);
	} else {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_RIGHT_ARROW, x + w - 20, y);
	}
	p = (mi->d.hslider.percent * (w - 54)) / 100;
	VideoDraw(MenuButtonGfx.Sprite, MBUTTON_S_KNOB, x + 18 + p, y + 1);
    }

    if (flags&MenuButtonSelected) {
	if (flags&MenuButtonDisabled) {
	    VideoDrawRectangleClip(ColorGray,x-2,y,w+2,h);
	} else {
	    VideoDrawRectangleClip(ColorYellow,x-2,y,w+2,h);
	}
    }

}

/**
**	Draw gem 'button' on menu mx, my
**
**	@param mi	menuitem pointer
**	@param mx	menu X display position (offset)
**	@param my	menu Y display position (offset)
*/
local void DrawGem(Menuitem *mi, unsigned mx, unsigned my)
{
    unsigned flags = mi->flags;
    MenuButtonId rb = mi->d.gem.button;
    unsigned x, y;

    x = mx+mi->xofs;
    y = my+mi->yofs;
    if ((mi->d.gem.state & MI_GSTATE_INVISIBLE)) {
	return;
    }
    if (flags&MenuButtonDisabled) {
	rb--;
    }
    else {
	if (flags&MenuButtonClicked) {
	    rb++;
	}
	if ((mi->d.gem.state & MI_GSTATE_CHECKED)) {
	    rb += 2;
	}
    }
    VideoDraw(MenuButtonGfx.Sprite, rb, x, y);
}

/**
**	Draw input 'button' on menu mx, my
**
**	@param mi	menuitem pointer
**	@param mx	menu X display position (offset)
**	@param my	menu Y display position (offset)
*/
local void DrawInput(Menuitem *mi, unsigned mx, unsigned my)
{
    int nc, rc;
    char *text;
    unsigned flags = mi->flags;
    MenuButtonId rb = mi->d.input.button;
    unsigned w, h, x, y;

    x = mx+mi->xofs;
    y = my+mi->yofs;
    w = mi->d.input.xsize;
    h = mi->d.input.ysize;

    GetDefaultTextColors(&nc, &rc);
    if (flags&MenuButtonDisabled) {
	rb--;
	SetDefaultTextColors(FontGrey,FontGrey);
    }

    PushClipping();
    SetClipping(0,0,x+w,VideoHeight-1);
    VideoDrawClip(MenuButtonGfx.Sprite, rb, x-1, y-1);
    PopClipping();
    text = mi->d.input.buffer;
    if (text) {
	VideoDrawText(x+2,y+2,mi->font,text);
    }
    if (flags&MenuButtonSelected) {
	if (flags&MenuButtonDisabled) {
	    VideoDrawRectangleClip(ColorGray,x-2,y-2,w,h);
	} else {
	    VideoDrawRectangleClip(ColorYellow,x-2,y-2,w,h);
	}
    }
    SetDefaultTextColors(nc,rc);
}


/**
**	Draw a menu.
**
**	@param menu_id	The menu number to display
*/
global void DrawMenu(int menu_id)
{
    int i, n, l;
    Menu *menu;
    Menuitem *mi, *mip;

    if (menu_id == -1) {
	MustRedraw &= ~RedrawMenu;
	return;
    }

    menu = Menus + menu_id;
    switch( menu->image ) {
	case ImagePanel1:
	    VideoDrawSub(TheUI.GameMenuePanel.Graphic,0,0,
		    VideoGraphicWidth(TheUI.GameMenuePanel.Graphic),
		    VideoGraphicHeight(TheUI.GameMenuePanel.Graphic),
		    menu->x,menu->y);
	    break;
	case ImagePanel2:
	    VideoDrawSub(TheUI.Menue1Panel.Graphic,0,0,
		    VideoGraphicWidth(TheUI.Menue1Panel.Graphic),
		    VideoGraphicHeight(TheUI.Menue1Panel.Graphic),
		    menu->x,menu->y);
	    break;
	case ImagePanel3:
	    VideoDrawSub(TheUI.Menue2Panel.Graphic,0,0,
		   VideoGraphicWidth(TheUI.Menue2Panel.Graphic),
		   VideoGraphicHeight(TheUI.Menue2Panel.Graphic),
		   menu->x,menu->y);
	    break;
	case ImagePanel4:
	    VideoDrawSub(TheUI.VictoryPanel.Graphic,0,0,
		   VideoGraphicWidth(TheUI.VictoryPanel.Graphic),
		   VideoGraphicHeight(TheUI.VictoryPanel.Graphic),
		   menu->x,menu->y);
	    break;
	case ImagePanel5:
	    VideoDrawSub(TheUI.ScenarioPanel.Graphic,0,0,
		   VideoGraphicWidth(TheUI.ScenarioPanel.Graphic),
		   VideoGraphicHeight(TheUI.ScenarioPanel.Graphic),
		   menu->x,menu->y);
	default:
	    break;
    }
    n = menu->nitems;
    mi = menu->items;
    mip = NULL;
    for (i = 0; i < n; i++) {
	switch (mi->mitype) {
	    case MI_TYPE_TEXT:
		if (!mi->d.text.text)
		    break;
		if (mi->d.text.tflags&MI_TFLAGS_CENTERED)
		    VideoDrawTextCentered(menu->x+mi->xofs,menu->y+mi->yofs,
			    mi->font,mi->d.text.text);
		else if (mi->d.text.tflags&MI_TFLAGS_RALIGN) {
		    l = VideoTextLength(mi->font,mi->d.text.text);
		    VideoDrawText(menu->x+mi->xofs-l,menu->y+mi->yofs,
			    mi->font,mi->d.text.text);
		} else
		    VideoDrawText(menu->x+mi->xofs,menu->y+mi->yofs,
			    mi->font,mi->d.text.text);
		break;
	    case MI_TYPE_BUTTON:
		DrawMenuButton(mi->d.button.button,mi->flags,
			mi->d.button.xsize,mi->d.button.ysize,
			menu->x+mi->xofs,menu->y+mi->yofs,
			mi->font,mi->d.button.text);
		break;
	    case MI_TYPE_PULLDOWN:
		if (mi->flags&MenuButtonClicked) {
		    mip = mi;	// Delay, due to possible overlaying!
		} else {
		    DrawPulldown(mi,menu->x,menu->y);
		}
		break;
	    case MI_TYPE_LISTBOX:
		DrawListbox(mi,menu->x,menu->y);
		break;
	    case MI_TYPE_VSLIDER:
		DrawVSlider(mi,menu->x,menu->y);
		break;
	    case MI_TYPE_HSLIDER:
		DrawHSlider(mi,menu->x,menu->y);
		break;
	    case MI_TYPE_DRAWFUNC:
		if (mi->d.drawfunc.draw) {
		    (*mi->d.drawfunc.draw)(mi);
		}
		break;
	    case MI_TYPE_INPUT:
		DrawInput(mi,menu->x,menu->y);
		break;
	    case MI_TYPE_GEM:
		DrawGem(mi,menu->x,menu->y);
		break;
	    default:
		break;
	}
	mi++;
    }
    if (mip) {
	DrawPulldown(mip,menu->x,menu->y);
    }

    MenuRedrawX=menu->x;
    MenuRedrawY=menu->y;
    MenuRedrawW=menu->xsize;
    MenuRedrawH=menu->ysize;
}

/*----------------------------------------------------------------------------
--	Button action handler and Init/Exit functions
----------------------------------------------------------------------------*/

/**
**	Set start menu backgound and draw it.
*/
local void StartMenusSetBackground(Menuitem *mi __attribute__((unused)))
{
    DestroyCursorBackground();
    if (!Menusbgnd) {
	Menusbgnd = LoadGraphic(MenuBackground);
	VideoSetPalette(Menusbgnd->Pixels);
    }

    // VideoLockScreen();

    // FIXME: bigger window ?
    VideoDrawSubClip(Menusbgnd,0,0,
	Menusbgnd->Width,Menusbgnd->Height,
	(VideoWidth-Menusbgnd->Width)/2,(VideoHeight-Menusbgnd->Height)/2);

    // VideoUnlockScreen();
}

/**
**	Draw the version and copyright at bottom of the screen.
**	Also include now the license.
*/
local void NameLineDrawFunc(Menuitem *mi)
{
    int nc, rc;

    GetDefaultTextColors(&nc, &rc);
    StartMenusSetBackground(mi);
    SetDefaultTextColors(rc, rc);

#ifdef WITH_SOUND
    if (SoundFildes == -1) {
	VideoDrawText(16, 16, LargeFont, "Sound disabled, please check!");
    }
#endif

    VideoDrawTextCentered(VideoWidth/2, OffsetY + 440, GameFont, NameLine);
    VideoDrawTextCentered(VideoWidth/2, OffsetY + 456, GameFont,
	"Engine distributed under the terms of the GNU General Public License.");
    SetDefaultTextColors(nc, rc);
}

/**
**	Start menu master init.
**
**	@param mi	The menu.
*/
local void PrgStartInit(Menuitem *mi)
{
    if (NetworkNumInterfaces == 0) {
	mi[2].flags = MenuButtonDisabled;
    } else {
	mi[2].flags = 0;
    }
}

local void GameMenuReturn(void)
{
    EndMenu();
    MustRedraw &= ~RedrawMenu;
    InterfaceState = IfaceStateNormal;
    ClearStatusLine();
    MarkDrawEntireMap();	//FIXME: some tiles could be left as they were?
    MustRedraw=RedrawEverything;
    GamePaused = 0;
}

local char *SaveDir;
local int TypedFileName;

/**
**	FIXME: docu.
*/
local void EnterSaveGameAction(Menuitem *mi, int key)
{
    if (mi->d.input.nch == 0) {
	mi[3].flags = MenuButtonDisabled;	/* mi[i]: Save button! */
    } else {
	mi[3].flags &= ~MenuButtonDisabled;
	if (key == 10 || key == 13) {
	    SaveAction();
	    return;
	}
    }
    TypedFileName = 1;
}

local void SaveAction(void)
{
    char filename[PATH_MAX];
    char *name = SaveGameMenuItems[1].d.input.buffer;
    size_t nameLength;

    nameLength = strlen(name);
    if (TypedFileName)
	nameLength -= 3;

    strcpy(filename, SaveDir);
    strcat(filename, "/");
    strncat(filename, name, nameLength);
    strcat(filename, ".sav");

    if (access(filename,F_OK)) {
        SaveGame(filename);
	SetMessage("Saved game to: %s", filename);
    } else {
	ProcessMenu(MENU_CONFIRM_SAVE, 1);
    }

    EndMenu();
}

local void CreateSaveDir(Menuitem *mi __attribute__((unused)))
{
#ifdef USE_WIN32
    SaveDir="save";
    mkdir(SaveDir);
#else
    char path[PATH_MAX];

    strcpy(path,getenv("HOME"));
    strcat(path,"/");
    strcat(path,FREECRAFT_HOME_PATH);
    mkdir(path,0777);
    strcat(path,"/save");
    mkdir(path,0777);
    if (SaveDir)
	free(SaveDir);
    SaveDir = strdup(path);
#endif
}

global void GameMenuSave(void)
{
    char savegame_buffer[32];

    strcpy(savegame_buffer, "~!_");
    SaveGameMenuItems[1].d.input.buffer = savegame_buffer;
    SaveGameMenuItems[1].d.input.nch = 0; /* strlen(savegame_buffer) - 3; */
    SaveGameMenuItems[1].d.input.maxch = 24;
    SaveGameMenuItems[4].flags = MenuButtonDisabled;	/* Save button! */
    ProcessMenu(MENU_SAVE_GAME, 1);
}

// FIXME: modify function
local void SaveLBExit(Menuitem *mi)
{
    FileList *fl;

    if (mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	free(fl);
	mi->d.listbox.options = NULL;
	mi->d.listbox.noptions = 0;
	mi[1].flags |= MenuButtonDisabled;
    }
}

local void SaveLBInit(Menuitem *mi)
{
    int i;

    SaveLBExit(mi);
    
    i = mi->d.listbox.noptions = ReadDataDirectory(SaveDir, SaveRDFilter,
						     (FileList **)&(mi->d.listbox.options));
    if (i != 0) {
	if (i > 7) {
	    SaveGameMenuItems[3].flags = MenuButtonSelected;
	} else {
	    SaveGameMenuItems[3].flags = MenuButtonDisabled;
	}
    }
    mi->d.listbox.curopt = -1;
}

// FIXME: modify function
local unsigned char *SaveLBRetrieve(Menuitem *mi, int i)
{
    FileList *fl;
    Menu *menu;
    MapInfo *info;
    static char buffer[1024];
    int j, n;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    if (i - mi->d.listbox.startline == mi->d.listbox.curopt) {
		if ((info = fl[i].xdata)) {
		    menu = Menus + MENU_SCEN_SELECT;
		    if (info->Description) {
			VideoDrawText(menu->x+8,menu->y+254,LargeFont,info->Description);
		    }
		    sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
		    VideoDrawText(menu->x+8,menu->y+254+20,LargeFont,buffer);
		    for (n = j = 0; j < PlayerMax; j++) {
			if (info->PlayerType[j] == PlayerPerson) {
			    n++;
			}
		    }
		    if (n == 1) {
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,"1 player");
		    } else {
			sprintf(buffer, "%d players", n);
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,buffer);
		    }
		}
	    }
	    strcpy(buffer, "   ");
	} else {
	    strcpy(buffer, "\260 ");
	}
	strcat(buffer, fl[i].name);
	return buffer;
    }
    return NULL;
}

local void SaveLBAction(Menuitem *mi, int i)
{
    FileList *fl;

    DebugCheck(i<0);
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (mi->d.listbox.noptions > 7) {
	    mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	    mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	}
	if (fl[i].type) {
	    strcpy(SaveGameMenuItems[1].d.input.buffer, fl[i].name);
	    SaveGameMenuItems[1].d.input.nch = strlen(fl[i].name);
	    SaveGameMenuItems[4].flags = MenuButtonSelected;
	} else {
	    SaveGameMenuItems[4].flags = MenuButtonDisabled;
	}
    }
    TypedFileName = 0;
}

local void SaveVSAction(Menuitem *mi, int i)
{
    int op, d1, d2;

    mi--;
    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi->d.listbox.curopt == -1)
	    mi->d.listbox.curopt = 0;

	    if (mi[1].d.vslider.cflags&MI_CFLAGS_DOWN) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
		    mi->d.listbox.curopt++;
		    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
			mi->d.listbox.curopt--;
			mi->d.listbox.startline++;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    } else if (mi[1].d.vslider.cflags&MI_CFLAGS_UP) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
		    mi->d.listbox.curopt--;
		    if (mi->d.listbox.curopt < 0) {
			mi->d.listbox.curopt++;
			mi->d.listbox.startline--;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    }
	    SaveLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
	    if (i == 2) {
		mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
	    }
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline + 1) * 100) /
				 (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.curper - mi[1].d.vslider.percent;
			    d2 = op - mi[1].d.vslider.curper;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt++;
			    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
				mi->d.listbox.curopt--;
				mi->d.listbox.startline++;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 == mi->d.listbox.noptions)
				break;
			}
		    }
		} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
				     (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
			    d2 = mi[1].d.vslider.curper - op;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt--;
			    if (mi->d.listbox.curopt < 0) {
				mi->d.listbox.curopt++;
				mi->d.listbox.startline--;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline == 0)
				break;
			}
		    }
		}

		DebugCheck(mi->d.listbox.startline < 0);
		DebugCheck(mi->d.listbox.startline+mi->d.listbox.curopt >= mi->d.listbox.noptions);

		SaveLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
    TypedFileName = 0;
}

// FIXME: modify function
local void SaveOk(void)
{
    FileList *fl;
    Menuitem *mi = ScenSelectMenuItems + 1;
    int i = mi->d.listbox.curopt + mi->d.listbox.startline;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    strcat(ScenSelectPath, "/");
	    strcat(ScenSelectPath, fl[i].name);
	    if (ScenSelectMenuItems[9].flags&MenuButtonDisabled) {
		ScenSelectMenuItems[9].flags &= ~MenuButtonDisabled;
		ScenSelectMenuItems[9].d.button.text = ScenSelectDisplayPath;
	    } else {
		strcat(ScenSelectDisplayPath, "/");
	    }
	    strcat(ScenSelectDisplayPath, fl[i].name);
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    mi[1].d.hslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	} else {
	    strcpy(ScenSelectFileName, fl[i].name);	// Final map name
	    EndMenu();
	}
    }
}

local int SaveRDFilter(char *pathbuf, FileList *fl)
{
    char *suf, *cp, *fsuffix, *np;

    np = strrchr(pathbuf, '/');
    if (np) {
	np++;
    } else {
	np = pathbuf;
    }
    cp = np;
    cp--;
    fl->type = -1;

    suf = ".sav";

    do {
	fsuffix = cp++;
	cp = strcasestr(cp, suf);
    } while (cp != NULL);
    if (fsuffix >= np) {
	cp = fsuffix + strlen(suf);
#ifdef USE_ZLIB
	if (strcmp(cp, ".gz") == 0) {
	    fl->type='z';
	}
#endif
#ifdef USE_BZ2LIB
	if (strcmp(cp, ".bz2") == 0) {
	    fl->type='b';
	}
#endif
		if (strstr(pathbuf, ".sav")) {
		    if (fl->type == -1)
			fl->type = 'n';
		    fl->name = strdup(np);
		    fl->xdata = NULL;
		    return 1;
	}
    }
    return 0;
}

// FIXME: modify function
local void LoadLBExit(Menuitem *mi)
{
    FileList *fl;

    if (mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	free(fl);
	mi->d.listbox.options = NULL;
	mi->d.listbox.noptions = 0;
	mi[1].flags |= MenuButtonDisabled;
    }
}

local void LoadLBInit(Menuitem *mi)
{
    int i;

    LoadLBExit(mi);
    
    i = mi->d.listbox.noptions = ReadDataDirectory(SaveDir, SaveRDFilter,
						     (FileList **)&(mi->d.listbox.options));
    if (i != 0) {
	if (i > 7) {
	    LoadGameMenuItems[2].flags = MenuButtonSelected;
	} else {
	    LoadGameMenuItems[2].flags = MenuButtonDisabled;
	}
    }
    mi->d.listbox.curopt = -1;
}

// FIXME: modify function
local unsigned char *LoadLBRetrieve(Menuitem *mi, int i)
{
    FileList *fl;
    Menu *menu;
    MapInfo *info;
    static char buffer[1024];
    int j, n;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    if (i - mi->d.listbox.startline == mi->d.listbox.curopt) {
		if ((info = fl[i].xdata)) {
		    menu = Menus + MENU_SCEN_SELECT;
		    if (info->Description) {
			VideoDrawText(menu->x+8,menu->y+254,LargeFont,info->Description);
		    }
		    sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
		    VideoDrawText(menu->x+8,menu->y+254+20,LargeFont,buffer);
		    for (n = j = 0; j < PlayerMax; j++) {
			if (info->PlayerType[j] == PlayerPerson) {
			    n++;
			}
		    }
		    if (n == 1) {
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,"1 player");
		    } else {
			sprintf(buffer, "%d players", n);
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,buffer);
		    }
		}
	    }
	    strcpy(buffer, "   ");
	} else {
	    strcpy(buffer, "\260 ");
	}
	strcat(buffer, fl[i].name);
	return buffer;
    }
    return NULL;
}

local void LoadLBAction(Menuitem *mi, int i)
{
    FileList *fl;

    DebugCheck(i<0);
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (mi->d.listbox.noptions > 7) {
	    mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	    mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	}
	if (fl[i].type) {
	    LoadGameMenuItems[3].flags = MenuButtonSelected;
	} else {
	    LoadGameMenuItems[3].flags = MenuButtonDisabled;
	}
    }
}

local void LoadVSAction(Menuitem *mi, int i)
{
    int op, d1, d2;

    mi--;
    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi->d.listbox.curopt == -1)
	    mi->d.listbox.curopt = 0;

	    if (mi[1].d.vslider.cflags&MI_CFLAGS_DOWN) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
		    mi->d.listbox.curopt++;
		    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
			mi->d.listbox.curopt--;
			mi->d.listbox.startline++;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    } else if (mi[1].d.vslider.cflags&MI_CFLAGS_UP) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
		    mi->d.listbox.curopt--;
		    if (mi->d.listbox.curopt < 0) {
			mi->d.listbox.curopt++;
			mi->d.listbox.startline--;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    }
	    LoadLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
	    if (i == 2) {
		mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
	    }
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline + 1) * 100) /
				 (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.curper - mi[1].d.vslider.percent;
			    d2 = op - mi[1].d.vslider.curper;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt++;
			    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
				mi->d.listbox.curopt--;
				mi->d.listbox.startline++;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 == mi->d.listbox.noptions)
				break;
			}
		    }
		} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
				     (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
			    d2 = mi[1].d.vslider.curper - op;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt--;
			    if (mi->d.listbox.curopt < 0) {
				mi->d.listbox.curopt++;
				mi->d.listbox.startline--;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline == 0)
				break;
			}
		    }
		}

		DebugCheck(mi->d.listbox.startline < 0);
		DebugCheck(mi->d.listbox.startline+mi->d.listbox.curopt >= mi->d.listbox.noptions);

		LoadLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

// FIXME: modify function
local void LoadOk(void)
{
    FileList *fl;
    Menuitem *mi = ScenSelectMenuItems + 1;
    int i = mi->d.listbox.curopt + mi->d.listbox.startline;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    strcat(ScenSelectPath, "/");
	    strcat(ScenSelectPath, fl[i].name);
	    if (ScenSelectMenuItems[9].flags&MenuButtonDisabled) {
		ScenSelectMenuItems[9].flags &= ~MenuButtonDisabled;
		ScenSelectMenuItems[9].d.button.text = ScenSelectDisplayPath;
	    } else {
		strcat(ScenSelectDisplayPath, "/");
	    }
	    strcat(ScenSelectDisplayPath, fl[i].name);
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    mi[1].d.hslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	} else {
	    strcpy(ScenSelectFileName, fl[i].name);	// Final map name
	    EndMenu();
	}
    }
}

#if 0
local void SaveMenu(void)
{
    EndMenu();
    ProcessMenu(MENU_CONFIRM_SAVE, 1);
}
#endif

local void ConfirmSaveInit(Menuitem *mi __attribute__((unused)))
{
    static char name[PATH_MAX];
    strcpy(name, "the file: ");
    strcat(name, SaveGameMenuItems[1].d.input.buffer);
    ConfirmSaveMenuItems[2].d.text.text = name;
}

local void ConfirmSaveFile(void)
{
    char name[PATH_MAX];
    strcpy(name, SaveDir);
    strcat(name, "/");
    strcat(name, SaveGameMenuItems[1].d.input.buffer);
    strcat(name, ".sav");
    SaveGame(name);
    SetMessage("Saved game to: %s", name);
    EndMenu();
}

local void FcDeleteMenu(void)
{
    EndMenu();
    ProcessMenu(MENU_CONFIRM_DELETE, 1);
}

local void FcDeleteInit(Menuitem *mi __attribute__((unused)))
{
    static char name[128];
    strcpy(name, "the file: ");
    strcat(name, SaveGameMenuItems[1].d.input.buffer);
    ConfirmDeleteMenuItems[2].d.text.text = name;
}

local void FcDeleteFile(void)
{
    char name[256];
    strcpy(name, SaveDir);
    strcat(name, "/");
    strcat(name, SaveGameMenuItems[1].d.input.buffer);
    unlink(name);
    EndMenu();
    *SaveGameMenuItems[1].d.input.buffer = '\0';
    SaveGameMenuItems[1].d.input.nch = 0;
    ProcessMenu(MENU_SAVE_GAME, 1);
}

local void LoadAction(void)
{
    char filename[256];
    FileList *fl;
    char *name; 
    size_t nameLength;

    fl = LoadGameMenuItems[1].d.listbox.options;
    
    name = fl[LoadGameMenuItems[1].d.listbox.curopt].name;
    nameLength = strlen(name);

    strcpy(filename, SaveDir);
    strcat(filename, "/");
    strncat(filename, name, nameLength);

    LoadGame(filename);
    SetMessage("Loaded game: %s", filename);
    GameLoaded=1;
    EndMenu();
}

global void GameMenuLoad(void)
{
    GameLoaded=0;
    LoadGameMenuItems[3].flags = MenuButtonDisabled;	/* Load button! */
    ProcessMenu(MENU_LOAD_GAME, 1);
    if( GameLoaded ) {
	GameMenuReturn();
    }
}

local void InitGameMenu(Menuitem *mi)
{
}

global void SoundOptions(void)
{
#ifdef WITH_SOUND
    SoundOptionsMenuItems[2].flags = 0;				// master volume slider
    SoundOptionsMenuItems[5].d.gem.state = MI_GSTATE_CHECKED;	// master power
    SoundOptionsMenuItems[8].flags = 0;				// music volume slider
    SoundOptionsMenuItems[11].flags = 0;			// music power
    SoundOptionsMenuItems[11].d.gem.state = MI_GSTATE_CHECKED;
    SoundOptionsMenuItems[14].flags = 0;			// cd volume slider
    SoundOptionsMenuItems[17].d.gem.state = MI_GSTATE_CHECKED;	// cd power
    SoundOptionsMenuItems[19].flags = 0;			// all tracks button
    SoundOptionsMenuItems[19].d.gem.state = MI_GSTATE_UNCHECKED;
    SoundOptionsMenuItems[21].flags = 0;			// random tracks button
    SoundOptionsMenuItems[21].d.gem.state = MI_GSTATE_UNCHECKED;

    // Set master volume checkbox and slider
    if (SoundFildes == -1) {
	SoundOptionsMenuItems[5].d.gem.state = MI_GSTATE_UNCHECKED;
	SoundOptionsMenuItems[2].flags = -1;
	SoundOptionsMenuItems[11].flags = -1;
    }
    SoundOptionsMenuItems[2].d.hslider.percent = (GlobalVolume * 100) / 255;

    // Set music volume checkbox and slider
    if (PlayingMusic != 1 || SoundFildes == -1) {
	SoundOptionsMenuItems[11].d.gem.state = MI_GSTATE_UNCHECKED;
	SoundOptionsMenuItems[8].flags = -1;
    }
    SoundOptionsMenuItems[8].d.hslider.percent = (MusicVolume * 100) / 255;

#if defined(USE_LIBCDA) || defined(USE_SDLCD)
    if (!strcmp(":off", CDMode) || !strcmp(":stopped", CDMode)) {
	SoundOptionsMenuItems[17].d.gem.state = MI_GSTATE_UNCHECKED;
	SoundOptionsMenuItems[14].flags = -1;
	SoundOptionsMenuItems[19].flags = -1;

	SoundOptionsMenuItems[21].flags = -1;
    } else {
#ifdef USE_LIBCDA
	int i = 17;

	cd_get_volume(&i, &i);
	SoundOptionsMenuItems[14].d.hslider.percent = (i * 100) / 255;
#endif
	if (!strcmp(":all", CDMode)) {
	    SoundOptionsMenuItems[19].d.gem.state = MI_GSTATE_CHECKED;
	}
	if (!strcmp(":random", CDMode)) {
	    SoundOptionsMenuItems[21].d.gem.state = MI_GSTATE_CHECKED;
	}
	SoundOptionsMenuItems[11].flags = -1;
    }
#else
    SoundOptionsMenuItems[14].flags = -1;			// cd volume slider
    SoundOptionsMenuItems[17].flags = -1;			// cd power
    SoundOptionsMenuItems[17].d.gem.state = MI_GSTATE_UNCHECKED;
    SoundOptionsMenuItems[19].flags = -1;			// all tracks button
    SoundOptionsMenuItems[21].flags = -1;			// random tracks button
#endif
    ProcessMenu(MENU_SOUND_OPTIONS, 1);
#endif // with sound
}

local void SetMasterPower(Menuitem *mi __attribute__((unused)))
{
#ifdef WITH_SOUND
#ifdef USE_SDLA
    if (SoundFildes != -1) {
	SDL_CloseAudio();
	SoundFildes=-1;
    } else {
	InitSound();
	MapUnitSounds();
	InitSoundServer();
	InitSoundClient();
	SoundOff=0;
    }
#else
    if (SoundFildes != -1) {
        close(SoundFildes);
        SoundFildes=-1;
    } else {
	InitSound();
	MapUnitSounds();
	InitSoundServer();
	InitSoundClient();
	SoundOff=0;
    }
#endif
#endif // with sound
    CurrentMenu=-1;
    SoundOptions();
}

local void SetMusicPower(Menuitem *mi __attribute__((unused)))
{
#ifdef WITH_SOUND
    SCM cb;

    if (PlayingMusic == 1) {
	StopMusic();
    } else {
	if (CallbackMusic) {
	    cb = gh_symbol2scm("music-stopped");
	    if (!gh_null_p(symbol_boundp(cb, NIL))) {
		SCM value;

		value = symbol_value(cb, NIL);
		if (!gh_null_p(value)) {
		    gh_apply(value, NIL);
		}
	    }
	}
    }
#endif // with sound
    CurrentMenu=-1;
    SoundOptions();
}

local void SetCdPower(Menuitem *mi __attribute__((unused)))
{
#ifdef USE_SDLCD
    // Start Playing CD
    if (!strcmp(":off", CDMode) || !strcmp(":stopped", CDMode)) {
#ifdef USE_WIN32
	SDL_CDResume(CDRom);
#endif
	PlayMusic(":random");
    } else {
    // Stop Playing CD
        SDL_CDPause(CDRom);
	CDMode = ":stopped";
    }
#elif defined(USE_LIBCDA)
    // Start Playing CD
    if (!strcmp(":off", CDMode) || !strcmp(":stopped", CDMode)) {
	PlayMusic(":random");
    } else {
    // Stop Playing CD
        cd_pause();
	CDMode = ":stopped";
    }
#endif
    CurrentMenu=-1;
    SoundOptions();
}

/**
**	Toggle the map of war handling.
**	Used in the preference menu.
**
**	@param mi	Menu item (not used).
*/
local void SetFogOfWar(Menuitem *mi __attribute__((unused)))
{
    if (!TheMap.NoFogOfWar) {
        TheMap.NoFogOfWar = 1;
        UpdateFogOfWarChange();
        MapUpdateVisible();
    } else {
        TheMap.NoFogOfWar = 0;
        UpdateFogOfWarChange();
        MapUpdateVisible();
    }
    MustRedraw &= ~RedrawMinimap;
}

/**
**	Toggle showing the command chars on icons.
**	Used in the preference menu.
**
**	@param mi	Menu item (not used).
*/
local void SetCommandKey(Menuitem *mi __attribute__((unused)))
{
    ShowCommandKey ^= 1;
}

local void SetCdModeAll(Menuitem *mi __attribute__((unused)))
{
#if defined(USE_LIBCDA) || defined(USE_SDLCD)
    CDMode = ":all";
#endif
    CurrentMenu=-1;
    SoundOptions();

}

local void SetCdModeRandom(Menuitem *mi __attribute__((unused)))
{
#if defined(USE_LIBCDA) || defined(USE_SDLCD)
    CDMode = ":random";
#endif
    CurrentMenu=-1;
    SoundOptions();

}

global void SpeedSettings(void)
{
    int i = 2;

    SpeedSettingsMenuItems[i].d.hslider.percent = ((VideoSyncSpeed - MIN_GAME_SPEED) * 100) / (MAX_GAME_SPEED - MIN_GAME_SPEED);
    if (SpeedSettingsMenuItems[i].d.hslider.percent < 0)
	SpeedSettingsMenuItems[i].d.hslider.percent = 0;
    if (SpeedSettingsMenuItems[i].d.hslider.percent > 100)
	SpeedSettingsMenuItems[i].d.hslider.percent = 100;
    SpeedSettingsMenuItems[i + 4].d.hslider.percent = 100 - (SpeedMouseScroll - 1) * 100 / 10;
    if (TheUI.MouseScroll == 0)
	SpeedSettingsMenuItems[i + 4].d.hslider.percent = 0;
    SpeedSettingsMenuItems[i + 8].d.hslider.percent = 100 - (SpeedKeyScroll - 1) * 100 / 10;
    if (TheUI.KeyScroll == 0)
	SpeedSettingsMenuItems[i + 8].d.hslider.percent = 0;
    ProcessMenu(MENU_SPEED_SETTINGS, 1);
}

/**
**	Enter the preferences menu.
**	Setup if the options are enabled / disabled.
**
**	@todo	FIXME: The init should be done by the init callback.
*/
global void Preferences(void)
{
    if (!TheMap.NoFogOfWar) {
	PreferencesMenuItems[1].d.gem.state = MI_GSTATE_CHECKED;
    } else {
	PreferencesMenuItems[1].d.gem.state = MI_GSTATE_UNCHECKED;
    }

    if (NetworkFildes == -1) {		// Not available in net games
	PreferencesMenuItems[1].flags = MI_ENABLED;
    } else {
	PreferencesMenuItems[1].flags = MI_DISABLED;
    }

    if (ShowCommandKey) {
	PreferencesMenuItems[3].d.gem.state = MI_GSTATE_CHECKED;
    } else {
	PreferencesMenuItems[3].d.gem.state = MI_GSTATE_UNCHECKED;
    }

    ProcessMenu(MENU_PREFERENCES, 1);
}

/**
**	Show the game options.
*/
local void GameOptions(void)
{
    ProcessMenu(MENU_GAME_OPTIONS, 1);
}

/**
**	Show the global options.
*/
local void GameGlobalOptionsMenu(void)
{
    // FIXME: write me
}

/**
**	Show the game credits.
*/
local void GameShowCredits(void)
{
    ShowCredits(&GameCredits);
}

/**
**	Show the End Scenario menu
*/
local void GameMenuEndScenario(void)
{
    ProcessMenu(MENU_END_SCENARIO, 1);
    if (!GameRunning)
	EndMenu();
}

/**
**	Restart the scenario
*/
local void EndScenarioRestart(void)
{
    RestartScenario=1;
    InterfaceState=IfaceStateNormal;
    GameRunning=0;
    EndMenu();
}

/**
**	End the game in defeat
*/
local void EndScenarioSurrender(void)
{
    InterfaceState=IfaceStateNormal;
    GameResult=GameDefeat;
    GameRunning=0;
    EndMenu();
}

/**
**	End the game and return to the menu
*/
local void EndScenarioQuitMenu(void)
{
    QuitToMenu=1;
    InterfaceState=IfaceStateNormal;
    GameRunning=0;
    EndMenu();
}

/**
**	End the running game from menu.
*/
local void GameMenuEnd(void)
{
    InterfaceState=IfaceStateNormal;
    GameRunning=0;
    CursorOn=CursorOnUnknown;
    CurrentMenu=-1;
}

local void KeystrokeHelpMenu(void)
{
    KeystrokeHelpMenuItems[1].d.vslider.percent = 0;
    ProcessMenu(MENU_KEYSTROKE_HELP, 1);
}

local void HelpMenu(void)
{
    ProcessMenu(MENU_HELP, 1);
}

local void ShowTipsMenu(void)
{
    if (ShowTips)
	TipsMenuItems[1].d.gem.state = MI_GSTATE_CHECKED;
    else
	TipsMenuItems[1].d.gem.state = MI_GSTATE_UNCHECKED;

    ProcessMenu(MENU_TIPS, 1);
}

/**
**	Free the tips from the menu
*/
local void FreeTips()
{
    int i;

    for( i=5; i<13; i++ ) {
	if( TipsMenuItems[i].d.text.text ) {
	    free(TipsMenuItems[i].d.text.text);
	    TipsMenuItems[i].d.text.text=NULL;
	}
    }
}

/**
**	Initialize the tips menu
*/
local void InitTips(Menuitem *mi __attribute__((unused)))
{
    int i;
    int line;
    char* p;
    char* s;
    char* str;
    int w;
    int font;
    int l;

    if( ShowTips ) {
	TipsMenuItems[1].d.gem.state=MI_GSTATE_CHECKED;
    } else {
	TipsMenuItems[1].d.gem.state=MI_GSTATE_UNCHECKED;
    }

    FreeTips();

    w=Menus[MENU_TIPS].xsize-2*TipsMenuItems[5].xofs;
    font=TipsMenuItems[5].font;
    i=0;
    line=5;

    p=Tips[CurrentTip];
    if( !p ) {
	return;
    }

    l=0;
    s=str=strdup(p);

    while( line<13 ) {
	char* s1;
	char* space;

	space=NULL;
	for( ;; ) {
	    if( VideoTextLength(font,s)<w ) {
		break;
	    }
	    s1=strrchr(s,' ');
	    if( !s1 ) {
		fprintf(stderr, "line too long: \"%s\"\n", s);
		break;
	    }
	    if( space ) {
		*space=' ';
	    }
	    space=s1;
	    *space='\0';
	}
	TipsMenuItems[line++].d.text.text=strdup(s);
	l+=strlen(s);
	if( !p[l] ) {
	    break;
	}
	++l;
	s=str+l;
    }

    free(str);
}

/**
**	Show tips at startup gem callback
*/
local void SetTips(Menuitem* mi __attribute__((unused)))
{
    if( TipsMenuItems[1].d.gem.state==MI_GSTATE_CHECKED ) {
	ShowTips=1;
    } else {
	ShowTips=0;
    }
}

local void NextTip(void)
{
    CurrentTip++;
    if( Tips[CurrentTip]==NULL )
	CurrentTip=0;
}

/**
**	Cycle through the tips
*/
local void ShowNextTip(void)
{
    NextTip();
    InitTips(NULL);
}

/**
**	Exit the tips menu
*/
local void TipsMenuEnd(void)
{
    NextTip();
    FreeTips();

    EndMenu();
}


/**
**	Exit the game from menu.
*/
local void GameMenuExit(void)
{
    Exit(0);
}

/**
**
*/
local void SetMenuObjectives()
{
    int i;
    int line;
    char* p;
    char* s;
    char* str;
    int w;
    int font;
    int l;

    w=Menus[MENU_OBJECTIVES].xsize-2*ObjectivesMenuItems[1].xofs;
    font=ObjectivesMenuItems[1].font;
    i=0;
    line=1;

    for( p=GameIntro.Objectives[i]; p; p=GameIntro.Objectives[++i] ) {
	l=0;
	s=str=strdup(p);

	for( ;; ) {
	    char* s1;
	    char* space;

	    space=NULL;
	    for( ;; ) {
		if( VideoTextLength(font,s)<w ) {
		    break;
		}
		s1=strrchr(s,' ');
		if( !s1 ) {
		    fprintf(stderr, "line too long: \"%s\"\n", s);
		    break;
		}
		if( space )
		    *space=' ';
		space=s1;
		*space='\0';
	    }
	    ObjectivesMenuItems[line++].d.text.text=strdup(s);
	    l+=strlen(s);
	    if( !p[l] ) {
		break;
	    }
	    ++l;
	    s=str+l;

	    if( line==Menus[MENU_OBJECTIVES].nitems-1 ) {
		break;
	    }
	}
	free(str);
    }
}

/**
**
*/
local void FreeMenuObjectives()
{
    int i;

    for( i=1;i<Menus[MENU_OBJECTIVES].nitems-1;i++ ) {
	if( ObjectivesMenuItems[i].d.text.text ) {
	    free(ObjectivesMenuItems[i].d.text.text);
	    ObjectivesMenuItems[i].d.text.text=NULL;
	}
    }
}

/**
**
*/
local void GameMenuObjectives(void)
{
    SetMenuObjectives();
    ProcessMenu(MENU_OBJECTIVES, 1);
    FreeMenuObjectives();
}

/**
**	Get pud info from select path+name
*/
local void GetInfoFromSelectPath(void)
{
    int i;

    FreeMapInfo(ScenSelectPudInfo);
    ScenSelectPudInfo = NULL;

    if (ScenSelectPath[0]) {
	i = strlen(ScenSelectPath);
	strcat(ScenSelectPath, "/");
    } else {
	i = 0;
    }
    strcat(ScenSelectPath, ScenSelectFileName);	// Final map name with path
    if (strcasestr(ScenSelectFileName, ".pud")) {
	ScenSelectPudInfo = GetPudInfo(ScenSelectPath);
	strcpy(ScenSelectFullPath, ScenSelectPath);
    } else {
	// FIXME: GetCmInfo();
    }
    ScenSelectPath[i] = '\0';		// Remove appended part
}

/**
**	Enter select scenario menu.
*/
local void ScenSelectMenu(void)
{
    ProcessMenu(MENU_SCEN_SELECT, 1);

    GetInfoFromSelectPath();
}

/**
**	Enter multiplayer select scenario menu.
*/
local void MultiScenSelectMenu(void)
{
    ScenSelectMenu();
    MultiGamePlayerSelectorsUpdate(1);
}

/**
**	Enter single player menu.
*/
local void SinglePlayerGameMenu(void)
{
    DestroyCursorBackground();
    GuiGameStarted = 0;
    ProcessMenu(MENU_CUSTOM_GAME_SETUP, 1);
    if (GuiGameStarted) {
	GameMenuReturn();
    }
}

/**
**	Show the campaign select menu.
**
**	Look which campaigns are available and how they are called.
*/
local void CampaignGameMenu(void)
{
    int i;

    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();
    Invalidate();

    DebugLevel0Fn("%d campaigns available\n" _C_ NumCampaigns);
    IfDebug(
	for( i=0; i<NumCampaigns; ++i ) {
	    DebugLevel0Fn("Campaign %d: %16.16s: %s\n" _C_ i _C_
		Campaigns[i].Ident _C_
		Campaigns[i].Name);
	}
    );

    //
    //	Setup campaign name.
    //
    for( i=0; i<NumCampaigns && i<4; ++i ) {
	char* s;

	CampaignSelectMenuItems[i].d.button.text=Campaigns[i].Name;
	CampaignSelectMenuItems[i].flags&=~MenuButtonDisabled;

	if( (s=strchr(Campaigns[i].Name,'!')) ) {
	    CampaignSelectMenuItems[i].d.button.hotkey=tolower(s[1]);
	}
    }
    for( ; i<4; ++i ) {
	CampaignSelectMenuItems[i].d.button.text="Not available";
	CampaignSelectMenuItems[i].flags|=MenuButtonDisabled;
    }

    GuiGameStarted = 0;
    ProcessMenu(MENU_CAMPAIGN_SELECT, 1);
    if (GuiGameStarted) {
	GameMenuReturn();
    }

    for( i=0; i<4; ++i ) {
	CampaignSelectMenuItems[i].d.button.text=NULL;
    }
}

/**
**	Start campaign from menu.
**
**	@param number	Number of the compaign.
*/
local void StartCampaignFromMenu(int number)
{
    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();
    Invalidate();

#if 0
    // JOHNS: this is currently not needed:

    // Any Campaign info should be displayed through a DrawFunc() Item
    // int the CAMPAIN_CONT menu processed below...
    ProcessMenu(MENU_CAMPAIGN_CONT, 1);
    // Set GuiGameStarted = 1 to acctually run a game here...
    // See CustomGameStart() for info...
#endif

    PlayCampaign(Campaigns[number].Ident);
    GuiGameStarted = 1;

    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoClearScreen();
    VideoUnlockScreen();
    Invalidate();

    // FIXME: johns otherwise crash in UpdateDisplay -> DrawMinimapCursor
    EndMenu();
}

/**
**	Call back for 1st entry of campaign menu.
**
**	@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu1(void)
{
    StartCampaignFromMenu(0);
}

/**
**	Call back for 2nd entry of campaign menu.
**
**	@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu2(void)
{
    StartCampaignFromMenu(1);
}

/**
**	Call back for 3rd entry of campaign menu.
**
**	@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu3(void)
{
    StartCampaignFromMenu(2);
}

/**
**	Call back for 4th entry of campaign menu.
**
**	@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu4(void)
{
    StartCampaignFromMenu(3);
}

/**
**	Call back for select entry of campaign menu.
*/
local void SelectCampaignMenu(void)
{
    // FIXME: not written
}

/**
**	Cancel button of player name menu pressed.
*/
local void EnterNameCancel(void)
{
    EnterNameMenuItems[1].d.input.nch = 0;
    EndMenu();
}

/**
**	Input field action of player name menu.
*/
local void EnterNameAction(Menuitem *mi, int key)
{
    if (mi->d.input.nch == 0) {
	mi[1].flags = MenuButtonDisabled;
    } else {
	mi[1].flags &= ~MenuButtonDisabled;
	if (key == 10 || key == 13) {
	    EndMenu();
	}
    }
}

/**
**	Cancel button of enter server ip/name menu pressed.
*/
local void EnterServerIPCancel(void)
{
    EnterServerIPMenuItems[1].d.input.nch = 0;
    EndMenu();
}

/**
**	Input field action of server ip/name.
*/
local void EnterServerIPAction(Menuitem *mi, int key)
{
    if (mi->d.input.nch == 0) {
	mi[1].flags = MenuButtonDisabled;
    } else {
	mi[1].flags &= ~MenuButtonDisabled;
	if (key == 10 || key == 13) {
	    EndMenu();
	}
    }
}

/**
**	Start processing join a network game menu.
*/
local void JoinNetGameMenu(void)
{
    char server_host_buffer[32];

    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();
    Invalidate();

    //
    //  Prepare enter ip/hostname menu
    //
    if (NetworkArg) {
	strcpy(server_host_buffer, NetworkArg);
    } else {
	server_host_buffer[0] = '\0';
    }
    strcat(server_host_buffer, "~!_");
    EnterServerIPMenuItems[1].d.input.buffer = server_host_buffer;
    EnterServerIPMenuItems[1].d.input.nch = strlen(server_host_buffer) - 3;
    EnterServerIPMenuItems[1].d.input.maxch = 24;
    if (EnterServerIPMenuItems[1].d.input.nch) {
	EnterServerIPMenuItems[2].flags &= ~MenuButtonDisabled;
    } else {
	EnterServerIPMenuItems[2].flags |= MenuButtonDisabled;
    }

    ProcessMenu(MENU_NET_ENTER_SERVER_IP, 1);

    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();

    if (EnterServerIPMenuItems[1].d.input.nch == 0) {
	return;
    }
    // Now finally here is the address
    server_host_buffer[EnterServerIPMenuItems[1].d.input.nch] = 0;
    if (NetworkSetupServerAddress(server_host_buffer)) {
	NetErrorMenuItems[1].d.text.text = "Unable to lookup host.";
	ProcessMenu(MENU_NET_ERROR, 1);
	VideoLockScreen();
	StartMenusSetBackground(NULL);
	VideoUnlockScreen();
	return;
    }
    NetworkInitClientConnect();
    if (NetworkArg) {
	free(NetworkArg);
    }
    NetworkArg = strdup(server_host_buffer);

    // Here we really go...
    ProcessMenu(MENU_NET_CONNECTING, 1);

    if (GuiGameStarted) {
	VideoLockScreen();
	StartMenusSetBackground(NULL);
	VideoUnlockScreen();
	Invalidate();
	EndMenu();
    }
}

/**
**	Cancel button of network connect menu pressed.
*/
local void NetConnectingCancel(void)
{
    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();
    NetworkExitClientConnect();
    // Trigger TerminateNetConnect() to call us again and end the menu
    NetLocalState = ccs_usercanceled;
    EndMenu();
}

/**
**	Call back from menu loop, if network state has changed.
*/
local void TerminateNetConnect(void)
{
    switch (NetLocalState) {
	case ccs_unreachable:
	    NetErrorMenuItems[1].d.text.text = "Cannot reach server.";
	    ProcessMenu(MENU_NET_ERROR, 1);

	    NetConnectingCancel();
	    return;
	case ccs_nofreeslots:
	    NetErrorMenuItems[1].d.text.text = "Server is full.";
	    ProcessMenu(MENU_NET_ERROR, 1);

	    NetConnectingCancel();
	    return;
	case ccs_serverquits:
	    NetErrorMenuItems[1].d.text.text = "Server gone.";
	    ProcessMenu(MENU_NET_ERROR, 1);

	    NetConnectingCancel();
	    return;
	case ccs_incompatibleengine:
	    NetErrorMenuItems[1].d.text.text = "Incompatible engine version.";
	    ProcessMenu(MENU_NET_ERROR, 1);

	    NetConnectingCancel();
	    return;
	case ccs_incompatiblenetwork:
	    NetErrorMenuItems[1].d.text.text = "Incompatible network version.";
	    ProcessMenu(MENU_NET_ERROR, 1);

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

    DebugLevel1Fn("NetLocalState %d\n" _C_ NetLocalState);
    NetConnectRunning = 2;
    DestroyCursorBackground();
    GuiGameStarted = 0;
    ProcessMenu(MENU_NET_MULTI_CLIENT, 1);
    if (GuiGameStarted) {
	GameMenuReturn();
    } else {
	NetConnectingCancel();
    }
}

/**
**	Start processing network game setup menu (server).
*/
local void CreateNetGameMenu(void)
{
    DestroyCursorBackground();
    GuiGameStarted = 0;
    ProcessMenu(MENU_NET_MULTI_SETUP, 1);
    if (GuiGameStarted) {
	GameMenuReturn();
    }
}

/**
**	Multiplayer game start game button pressed.
*/
local void MultiGameStart(void)
{
    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();
    Invalidate();

    NetworkServerStartGame();
    NetworkGamePrepareGameSettings();

    CustomGameStart();
}

/**
**	Enter multiplayer game menu.
*/
local void MultiPlayerGameMenu(void)
{
    char NameBuf[32];

    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();
    Invalidate();

    EnterNameMenuItems[1].d.input.buffer = NameBuf;
    strcpy(NameBuf, NetworkName);
    strcat(NameBuf, "~!_");
    EnterNameMenuItems[1].d.input.nch = strlen(NameBuf) - 3;
    EnterNameMenuItems[1].d.input.maxch = 15;
    EnterNameMenuItems[2].flags &= ~MenuButtonDisabled;

    ProcessMenu(MENU_ENTER_NAME, 1);

    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();

    if (EnterNameMenuItems[1].d.input.nch == 0) {
	return;
    }

    NameBuf[EnterNameMenuItems[1].d.input.nch] = 0;	// Now finally here is the name
    memset(NetworkName, 0, 16);
    strcpy(NetworkName, NameBuf);

    GuiGameStarted = 0;
    // Here we really go...
    ProcessMenu(MENU_NET_CREATE_JOIN, 1);

    DebugLevel0Fn("GuiGameStarted: %d\n" _C_ GuiGameStarted);
    if (GuiGameStarted) {
	GameMenuReturn();
    }
}

local void FreeMapInfos(FileList *fl, int n)
{
    int i;

    for (i = 0; i < n; i++) {
	if (fl[i].type && fl[i].xdata) {
	    FreeMapInfo(fl[i].xdata);
	    fl[i].xdata = NULL;
	}
    }
}

/**
**	Initialize the scenario selector menu.
*/
local void ScenSelectInit(Menuitem * mi __attribute__ ((unused)))
{
    DebugCheck(!*ScenSelectPath);
    ScenSelectMenuItems[9].flags =
	*ScenSelectDisplayPath ? 0 : MenuButtonDisabled;
    ScenSelectMenuItems[9].d.button.text = ScenSelectDisplayPath;
    DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);
}

local void ScenSelectLBAction(Menuitem *mi, int i)
{
    FileList *fl;

    DebugCheck(i<0);
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    ScenSelectMenuItems[3].d.button.text = "OK";
	} else {
	    ScenSelectMenuItems[3].d.button.text = "Open";
	}
	if (mi->d.listbox.noptions > 5) {
	    mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	    mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	}
    }
}

local void ScenSelectLBExit(Menuitem *mi)
{
    FileList *fl;

    if (mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	FreeMapInfos(fl, mi->d.listbox.noptions);
	free(fl);
	mi->d.listbox.options = NULL;
	mi->d.listbox.noptions = 0;
	mi[1].flags |= MenuButtonDisabled;
    }
}

local int ScenSelectRDFilter(char *pathbuf, FileList *fl)
{
    MapInfo *info;
    char *suf, *cp, *lcp, *np;
    int p, sz;
    static int szl[] = { -1, 32, 64, 96, 128, 256, 512, 1024 };
#ifdef USE_ZZIPLIB
    ZZIP_FILE *zzf;
#endif

#if 0
    if (ScenSelectMenuItems[6].d.pulldown.curopt == 0) {
	suf = NULL;
	p = -1;
    } else
#endif
    if (ScenSelectMenuItems[6].d.pulldown.curopt == 0) {
	suf = ".cm";
	p = 0;
    } else {
	suf = ".pud";
	p = 1;
    }
    np = strrchr(pathbuf, '/');
    if (np) {
	np++;
    } else {
	np = pathbuf;
    }
    cp = np;
    cp--;
    fl->type = -1;
#ifdef USE_ZZIPLIB
    if ((zzf = zzip_open(pathbuf, O_RDONLY|O_BINARY))) {
	sz = zzip_file_real(zzf);
	zzip_close(zzf);
	if (!sz) {
	    goto usezzf;
	}
    }
#endif
    do {
	lcp = cp++;
	cp = strcasestr(cp, suf);
#if 0
	if( suf ) {
	} else if( !suf && (cp = strcasestr(cp, ".cm")) ) {
	    suf = ".cm";
	    p = 0;
	} else if( !suf && (cp = strcasestr(cp, ".pud")) ) {
	    suf = ".pud";
	    p = 1;
	} else {
	    cp=NULL;
	}
#endif
    } while (cp != NULL);
    if (lcp >= np) {
	cp = lcp + strlen(suf);
#ifdef USE_ZLIB
	if (strcmp(cp, ".gz") == 0) {
	    *cp = 0;
	}
#endif
#ifdef USE_BZ2LIB
	if (strcmp(cp, ".bz2") == 0) {
	    *cp = 0;
	}
#endif
	if (*cp == 0) {
#ifdef USE_ZZIPLIB
usezzf:
#endif
#if 0
	    if( p==-1 ) {
		printf("What now ?\n");
		if (strcasestr(pathbuf, ".pud")) {
		    p=1;
		} else {
		    p=0;
		}
	    }
#endif
	    if (p) {
		if (strcasestr(pathbuf, ".pud")) {
		    info = GetPudInfo(pathbuf);
		    if (info) {
			DebugLevel3Fn("GetPudInfo(%s) : %p\n" _C_ pathbuf _C_ info);
			sz = szl[ScenSelectMenuItems[8].d.pulldown.curopt];
			if (sz < 0 || (info->MapWidth == sz && info->MapHeight == sz)) {
			    fl->type = 1;
			    fl->name = strdup(np);
			    fl->xdata = info;
			    return 1;
			} else {
			    FreeMapInfo(info);
			}
		    }
		}
	    } else {
		if (strstr(pathbuf, ".cm")) {
		    // info = GetCmInfo(pathbuf);
		    info = NULL;
		    DebugLevel3Fn("GetCmInfo(%s) : %p\n" _C_ pathbuf _C_ info);
		    fl->type = 1;
		    fl->name = strdup(np);
		    fl->xdata = info;
		    return 1;
		}
	    }
	}
    }
    return 0;
}

local void ScenSelectLBInit(Menuitem *mi)
{
    int i;

    ScenSelectLBExit(mi);
    if (ScenSelectMenuItems[6].d.pulldown.curopt == 0) {
	ScenSelectMenuItems[8].flags |= MenuButtonDisabled;
    } else {
	ScenSelectMenuItems[8].flags &= ~MenuButtonDisabled;
    }
    i = mi->d.listbox.noptions = ReadDataDirectory(ScenSelectPath, ScenSelectRDFilter,
						     (FileList **)&(mi->d.listbox.options));
    if (i == 0) {
	ScenSelectMenuItems[3].d.button.text = "OK";
	ScenSelectMenuItems[3].flags |= MenuButtonDisabled;
    } else {
	ScenSelectLBAction(mi, 0);
	ScenSelectMenuItems[3].flags &= ~MenuButtonDisabled;
	if (i > 5) {
	    mi[1].flags &= ~MenuButtonDisabled;
	}
    }
}

local unsigned char *ScenSelectLBRetrieve(Menuitem *mi, int i)
{
    FileList *fl;
    Menu *menu;
    MapInfo *info;
    static char buffer[1024];
    int j, n;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    if (i - mi->d.listbox.startline == mi->d.listbox.curopt) {
		if ((info = fl[i].xdata)) {
		    menu = Menus + MENU_SCEN_SELECT;
		    if (info->Description) {
			VideoDrawText(menu->x+8,menu->y+254,LargeFont,info->Description);
		    }
		    sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
		    VideoDrawText(menu->x+8,menu->y+254+20,LargeFont,buffer);
		    for (n = j = 0; j < PlayerMax; j++) {
			if (info->PlayerType[j] == PlayerPerson) {
			    n++;
			}
		    }
		    if (n == 1) {
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,"1 player");
		    } else {
			sprintf(buffer, "%d players", n);
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,buffer);
		    }
		}
	    }
	    strcpy(buffer, "   ");
	} else {
	    strcpy(buffer, "\260 ");
	}
	strcat(buffer, fl[i].name);
	return buffer;
    }
    return NULL;
}

local void ScenSelectTPMSAction(Menuitem *mi, int i __attribute__((unused)))
{
    mi = ScenSelectMenuItems + 1;
    ScenSelectLBInit(mi);
    mi->d.listbox.cursel = -1;
    mi->d.listbox.startline = 0;
    mi->d.listbox.curopt = 0;
    mi[1].d.vslider.percent = 0;
    mi[1].d.hslider.percent = 0;
    MustRedraw |= RedrawMenu;
}

local void ScenSelectVSAction(Menuitem *mi, int i)
{
    int op, d1, d2;

    mi--;
    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_DOWN) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
		    mi->d.listbox.curopt++;
		    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
			mi->d.listbox.curopt--;
			mi->d.listbox.startline++;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    } else if (mi[1].d.vslider.cflags&MI_CFLAGS_UP) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
		    mi->d.listbox.curopt--;
		    if (mi->d.listbox.curopt < 0) {
			mi->d.listbox.curopt++;
			mi->d.listbox.startline--;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    }
	    ScenSelectLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
	    if (i == 2) {
		mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
	    }
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline + 1) * 100) /
				 (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.curper - mi[1].d.vslider.percent;
			    d2 = op - mi[1].d.vslider.curper;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt++;
			    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
				mi->d.listbox.curopt--;
				mi->d.listbox.startline++;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 == mi->d.listbox.noptions)
				break;
			}
		    }
		} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
				     (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
			    d2 = mi[1].d.vslider.curper - op;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt--;
			    if (mi->d.listbox.curopt < 0) {
				mi->d.listbox.curopt++;
				mi->d.listbox.startline--;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline == 0)
				break;
			}
		    }
		}

		DebugCheck(mi->d.listbox.startline < 0);
		DebugCheck(mi->d.listbox.startline+mi->d.listbox.curopt >= mi->d.listbox.noptions);

		ScenSelectLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void KeystrokeHelpVSAction(Menuitem *mi, int i)
{
    int j, nitems = sizeof(keystrokehelptexts) / sizeof(unsigned char *);

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    j = ((mi->d.vslider.percent + 1) * (nitems - 11)) / 100;
	    if (mi->d.vslider.cflags&MI_CFLAGS_DOWN && j < nitems - 11) {
		    j++;
		    MustRedraw |= RedrawMenu;
	    } else if (mi->d.vslider.cflags&MI_CFLAGS_UP && j > 0) {
		    j--;
		    MustRedraw |= RedrawMenu;
	    }
	    if (i == 2) {
		mi->d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
	    }
	    mi->d.vslider.percent = j * 100 / (nitems - 11);
	    break;
	case 1:		// mouse - move
	    if ((mi->d.vslider.cflags&MI_CFLAGS_KNOB) && (mi->flags&MenuButtonClicked)) {
		j = ((mi->d.vslider.curper + 1) * (nitems - 11)) / 100;
		mi->d.vslider.percent = mi->d.vslider.curper;
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

/**
**	Draw the Keystroke Help texts
*/
local void KeystrokeHelpDrawFunc(Menuitem *mi)
{
    int i, j, nitems = sizeof(keystrokehelptexts) / sizeof(unsigned char *);
    Menu *menu = Menus + MENU_KEYSTROKE_HELP;

    j = ((mi[-2].d.vslider.percent + 1) * (nitems - 11)) / 100;
    for (i = 0; i < 11; i++) {
	VideoDrawText(menu->x+mi->xofs,menu->y+mi->yofs+(i*20),
			    mi->font,keystrokehelptexts[j]);
	j++;
    }
}

local void GameSpeedHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing game speed by 10%s\n" _C_ "%");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing game speed by 10%s\n" _C_ "%");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
		VideoSyncSpeed = (mi[1].d.hslider.percent * (MAX_GAME_SPEED - MIN_GAME_SPEED)) / 100 + MIN_GAME_SPEED;
		SetVideoSync();
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		VideoSyncSpeed = (mi[1].d.hslider.percent * (MAX_GAME_SPEED - MIN_GAME_SPEED)) / 100 + MIN_GAME_SPEED;
		SetVideoSync();
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void MouseScrollHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing mouse speed\n");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing mouse speed\n");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
	    TheUI.MouseScroll = 1;
	    SpeedMouseScroll = 10 - (mi[1].d.hslider.percent * 9) / 100;
	    if (mi[1].d.hslider.percent == 0)
		TheUI.MouseScroll = 0;
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = (mi[1].d.hslider.curper + 6) / 10 * 10;
		TheUI.MouseScroll = 1;
		SpeedMouseScroll = 10 - (mi[1].d.hslider.percent * 9) / 100;
		if (mi[1].d.hslider.percent == 0)
		    TheUI.MouseScroll = 0;
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void KeyboardScrollHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing keyboard speed\n");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing keyboard speed\n");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
	    TheUI.KeyScroll = 1;
	    SpeedKeyScroll = 10 - (mi[1].d.hslider.percent * 9) / 100;
	    if (mi[1].d.hslider.percent == 0)
		TheUI.KeyScroll = 0;
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = (mi[1].d.hslider.curper + 6) / 10 * 10;
		TheUI.KeyScroll = 1;
		SpeedKeyScroll = 10 - (mi[1].d.hslider.percent * 9) / 100;
		if (mi[1].d.hslider.percent == 0)
		    TheUI.KeyScroll = 0;
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void MasterVolumeHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing master volume\n");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing master volume\n");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
	    SetGlobalVolume((mi[1].d.hslider.percent * 255) / 100);
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = (mi[1].d.hslider.curper + 6) / 10 * 10;
		SetGlobalVolume((mi[1].d.hslider.percent * 255) / 100);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void MusicVolumeHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing music volume\n");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing music volume\n");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
	    SetMusicVolume((mi[1].d.hslider.percent * 255) / 100);
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = (mi[1].d.hslider.curper + 6) / 10 * 10;
		SetMusicVolume((mi[1].d.hslider.percent * 255) / 100);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

#ifdef USE_LIBCDA
local void CdVolumeHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing cd volume\n");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing cd volume\n");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
	    cd_set_volume((mi[1].d.hslider.percent * 255) / 100,(mi[1].d.hslider.percent * 255) / 100);
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = (mi[1].d.hslider.curper + 6) / 10 * 10;
		cd_set_volume((mi[1].d.hslider.percent * 255) / 100,(mi[1].d.hslider.percent * 255) / 100);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}
#else
local void CdVolumeHSAction(Menuitem *mi __attribute__((unused)),
	int i __attribute__((unused)))
{
}
#endif

local void ScenSelectFolder(void)
{
    char *cp;
    Menuitem *mi = ScenSelectMenuItems + 1;

    if (ScenSelectDisplayPath[0]) {
	cp = strrchr(ScenSelectDisplayPath, '/');
	if (cp) {
	    *cp = 0;
	} else {
	    ScenSelectDisplayPath[0] = 0;
	    ScenSelectMenuItems[9].flags |= MenuButtonDisabled;
	    ScenSelectMenuItems[9].d.button.text = NULL;
	}
	cp = strrchr(ScenSelectPath, '/');
	if (cp) {
	    *cp = 0;
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    mi[1].d.hslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	}
    }
}

local void ScenSelectOk(void)
{
    FileList *fl;
    Menuitem *mi = ScenSelectMenuItems + 1;
    int i = mi->d.listbox.curopt + mi->d.listbox.startline;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    strcat(ScenSelectPath, "/");
	    strcat(ScenSelectPath, fl[i].name);
	    if (ScenSelectMenuItems[9].flags&MenuButtonDisabled) {
		ScenSelectMenuItems[9].flags &= ~MenuButtonDisabled;
		ScenSelectMenuItems[9].d.button.text = ScenSelectDisplayPath;
	    } else {
		strcat(ScenSelectDisplayPath, "/");
	    }
	    strcat(ScenSelectDisplayPath, fl[i].name);
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    mi[1].d.hslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	} else {
	    strcpy(ScenSelectFileName, fl[i].name);	// Final map name
	    EndMenu();
	}
    }
}

/**
**	Scenario select cancel button.
*/
local void ScenSelectCancel(void)
{
    char* s;

    //
    //  Use last selected map.
    //
    DebugLevel0Fn("Map   path: %s\n" _C_ CurrentMapPath);
    strcpy(ScenSelectPath, FreeCraftLibPath);
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

    DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);

    EndMenu();
}

local void GameCancel(void)
{
    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();
    FreeMapInfo(ScenSelectPudInfo);
    ScenSelectPudInfo = NULL;
    EndMenu();
}

/**
**	Custom game start game button pressed.
*/
local void CustomGameStart(void)
{
    int i;
    char *p;

    FreeMapInfo(ScenSelectPudInfo);
    ScenSelectPudInfo = NULL;

    if (ScenSelectPath[0]) {
	strcat(ScenSelectPath, "/");
	strcat(ScenSelectPath, ScenSelectFileName);	// Final map name with path
	p = ScenSelectPath + strlen(FreeCraftLibPath) + 1;
	strcpy(CurrentMapPath, p);
    } else {
	strcpy(CurrentMapPath, ScenSelectFileName);
	strcat(ScenSelectPath, ScenSelectFileName);	// Final map name with path
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
**	Single player custom game menu entered.
*/
local void GameSetupInit(Menuitem * mi __attribute__ ((unused)))
{
    char* s;

    //
    //  No old path, setup the default.
    //
    if (!*CurrentMapPath || *CurrentMapPath == '.' || *CurrentMapPath == '/') {
#if 0					// FIXME: as soon as .cm is supported..
	strcpy(CurrentMapPath, "default.cm");
#endif
	strcpy(CurrentMapPath, "default.pud");

    }

    DebugLevel0Fn("Map   path: %s\n" _C_ CurrentMapPath);
    strcpy(ScenSelectPath, FreeCraftLibPath);
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
    DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);

    GetInfoFromSelectPath();
}

local void GameDrawFunc(Menuitem *mi)
{
    int nc, rc, l;
    char buffer[32];

    GetDefaultTextColors(&nc, &rc);
    StartMenusSetBackground(mi);
    SetDefaultTextColors(rc, rc);
    l = VideoTextLength(GameFont, "Scenario:");
    VideoDrawText(OffsetX + 16, OffsetY + 360, GameFont, "Scenario:");
    VideoDrawText(OffsetX + 16, OffsetY + 360+24 , GameFont, ScenSelectFileName);
    if (ScenSelectPudInfo) {
	if (ScenSelectPudInfo->Description) {
	    VideoDrawText(OffsetX + 16 + l + 8, OffsetY + 360, GameFont, ScenSelectPudInfo->Description);
	}
	sprintf(buffer, " (%d x %d)", ScenSelectPudInfo->MapWidth, ScenSelectPudInfo->MapHeight);
	VideoDrawText(OffsetX + 16+l+8+VideoTextLength(GameFont, ScenSelectFileName), OffsetY + 360+24, GameFont, buffer);
    }
#if 0
    for (n = j = 0; j < PlayerMax; j++) {
	if (info->PlayerType[j] == PlayerPerson) {
	    n++;
	}
    }
    if (n == 1) {
	VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,"1 player");
    } else {
	sprintf(buffer, "%d players", n);
	VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,buffer);
    }
#endif
    SetDefaultTextColors(nc, rc);
}

/**
**	Menu setup race pulldown action.
**
**	@note FIXME: Todo support more and other races.
*/
local void GameRCSAction(Menuitem *mi, int i)
{
    int v[] = { PlayerRaceHuman, PlayerRaceOrc, SettingsPresetMapDefault };

    if (mi->d.pulldown.curopt == i) {
	GameSettings.Presets[0].Race = v[i];
	ServerSetupState.Race[0] = 2 - i;
	NetworkServerResyncClients();
    }
}

local void GameRESAction(Menuitem *mi, int i)
{
    int v[] = { SettingsResourcesMapDefault, SettingsResourcesLow,
		SettingsResourcesMedium, SettingsResourcesHigh };

    if (!mi || mi->d.pulldown.curopt == i) {
	GameSettings.Resources = v[i];
	ServerSetupState.ResOpt = i;
	if (mi) {
	    NetworkServerResyncClients();
	}
    }
}

local void GameUNSAction(Menuitem *mi, int i)
{
    if (!mi || mi->d.pulldown.curopt == i) {
	GameSettings.NumUnits = i ? SettingsNumUnits1 : SettingsNumUnitsMapDefault;
	ServerSetupState.UnsOpt = i;
	if (mi) {
	    NetworkServerResyncClients();
	}
    }
}

local void GameTSSAction(Menuitem *mi, int i)
{
    // FIXME: TilesetSummer, ... shouldn't be used, they will be removed.
    int v[] = { SettingsPresetMapDefault, TilesetSummer, TilesetWinter, TilesetWasteland, TilesetSwamp };

    if (!mi || mi->d.pulldown.curopt == i) {
	GameSettings.Terrain = v[i];
	ServerSetupState.TssOpt = i;
	if (mi) {
	    NetworkServerResyncClients();
	}
    }
}

/**
**	Called if the pulldown menu of the game type is changed.
*/
local void GameGATAction(Menuitem *mi, int i)
{
    if (!mi || mi->d.pulldown.curopt == i) {
	// FIXME: not supported
	// GameSettings.GameType = i-1;
	// ServerSetupState.GaTOpt = i;
	if (mi) {
	    NetworkServerResyncClients();
	}
    }
}

local void CustomGameOPSAction(Menuitem *mi __attribute__((unused)), int i)
{
    GameSettings.Opponents = i ? i : SettingsPresetMapDefault;
}

/**
**	Menu setup fog-of-war pulldown action.
**
**	@note FIXME: Want to support more features. fe. reveal map on start.
*/
local void MultiGameFWSAction(Menuitem *mi, int i)
{
    if (!mi || mi->d.pulldown.curopt == i) {
	FlagRevealMap = i;
	ServerSetupState.FwsOpt = i;
	if (mi) {
	    NetworkServerResyncClients();
	}
    }
}

/**
**	Multiplayer menu player server states.
*/
local void MultiGamePTSAction(Menuitem *mi, int o)
{
    int i;

    i = mi - NetMultiSetupMenuItems - SERVER_PLAYER_STATE;
    // JOHNS: Must this be always true?
    // ARI: NO! think of client menus!
    // DebugCheck( i<0 || i>PlayerMax-1 );

    if (i > 0 && i < PlayerMax-1) {
	if (mi->d.pulldown.curopt == o) {
	    ServerSetupState.CompOpt[i] = o;
	    MultiGamePlayerSelectorsUpdate(3);	// Recalc buttons on server
	    NetworkServerResyncClients();
	}
    }
}

local void MultiGameDrawFunc(Menuitem *mi)
{
    GameDrawFunc(mi);
}

local void MultiGameClientDrawFunc(Menuitem *mi)
{
    // FIXME: do something better
    GameDrawFunc(mi);
}

/**
**	Multiplayer network game final race an player type setup.
*/
local void NetworkGamePrepareGameSettings(void)
{
    int h, i;
    int num[PlayerMax];

    DebugCheck(!ScenSelectPudInfo);

    DebugLevel0Fn("NetPlayers = %d\n" _C_ NetPlayers);

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
    for (h = i = 0; i < PlayerMax-1; i++) {
	if (ScenSelectPudInfo->PlayerType[i] == PlayerPerson) {
	    DebugLevel3Fn("Player slot %i is available for a person\n" _C_ i);
	    num[h++] = i;
	}
    }
    for (i = 0; i < h; i++) {
	switch(ServerSetupState.CompOpt[num[i]]) {
	    case 0:
		GameSettings.Presets[num[i]].Type = PlayerPerson;
		DebugLevel3Fn("Settings[%d].Type == Person\n" _C_ num[i]);
		switch (ServerSetupState.Race[num[i]]) {
		    case 1:
			GameSettings.Presets[num[i]].Race = PlayerRaceOrc;
			break;
		    case 2:
			GameSettings.Presets[num[i]].Race = PlayerRaceHuman;
		    default:
			break;
		}
		break;
	    case 1:
		GameSettings.Presets[num[i]].Type = PlayerComputer;
		DebugLevel3Fn("Settings[%d].Type == Computer\n" _C_ num[i]);
		break;
	    case 2:
		GameSettings.Presets[num[i]].Type = PlayerNobody;
		DebugLevel3Fn("Settings[%d].Type == Closed\n" _C_ num[i]);
	    default:
		break;
	}
    }

#ifdef DEBUG
    for (i = 0; i < NetPlayers; i++) {
	DebugCheck(GameSettings.Presets[Hosts[i].PlyNr].Type != PlayerPerson);
	;
    }
#endif
}

/**
**	Player selectors have changed.
**	Caution: Called by map change (inital = 1)!
**	Caution: Called by network events from clients (inital = 2)!
**	Caution: Called by button action on server (inital = 3)!
*/
local void MultiGamePlayerSelectorsUpdate(int initial)
{
    int i, h, c;
    int avail, ready;

    //	FIXME: What this has to do:
    //	Use lag gem as KICK button
    //  Notify clients about MAP change: (initial = 1...)

    DebugLevel3Fn("initial = %d\n" _C_ initial);

    //	Calculate available slots from pudinfo
    for (c = h = i = 0; i < PlayerMax; i++) {
	if (ScenSelectPudInfo->PlayerType[i] == PlayerPerson) {
	    h++;	// available interactive player slots
	}
	if (ScenSelectPudInfo->PlayerType[i] == PlayerComputer) {
	    c++;	// available computer player slots
	}
    }

    avail = h;
    //	Setup the player menu
    for (ready = i = 1; i < PlayerMax-1; i++) {
	if (initial == 1) {
	    NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i].flags = 0;
	    NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;

	    NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i].flags = 0;
	    NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;

	    // FIXME: don't forget to throw out additional players
	    //	  without available slots here!

	}
	if (Hosts[i].PlyNr) {
	    NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i] = NetMultiButtonStorage[1];

	    NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i].flags = 0;
	    NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;
	    if (ServerSetupState.Ready[i]) {
		NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i].d.gem.state |= MI_GSTATE_CHECKED;
		++ready;
	    }

	    NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i].flags = 0;
	    NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;
	} else {
	    // don't allow network and button events to intercept server player's action on pulldown buttons!
	    if (!(NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i].flags&MenuButtonClicked)) {
		if (initial == 1 ||
		    (initial == 2 && NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i].mitype != MI_TYPE_PULLDOWN)) {
		    NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i] = NetMultiButtonStorage[0];
		    NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i].d.pulldown.state = 0;
		    NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i].d.pulldown.curopt = ServerSetupState.CompOpt[i];
		}
	    }
	    if (i < h && ServerSetupState.CompOpt[i] != 0) {
		avail--;
	    }

	    NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i].flags = MenuButtonDisabled;
	    NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;

	    NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i].flags = MenuButtonDisabled;
	    NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
	}

	NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i].yofs = 32 + (i&7) * 22;
	if (i > 7) {
	    NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i].xofs = 320 + 40;
	}


	if (i >= h) {
	    NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i].d.pulldown.state = MI_PSTATE_PASSIVE;

	    NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i].flags = MenuButtonDisabled;
	    NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;

	    NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i].flags = MenuButtonDisabled;
	    NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
	}

	if (i >= h + c) {
	    NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i].d.pulldown.defopt = 2;
	    NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i].d.pulldown.curopt = 2;
	    NetMultiSetupMenuItems[SERVER_PLAYER_STATE + i].flags = MenuButtonDisabled;
	}
    }

    //	Tell connect state machines how many interactive players we can have
    NetPlayers = avail;
    //	Check if all players are ready.
    DebugLevel0Fn("READY to START: AVAIL = %d, READY = %d\n" _C_ avail _C_ ready);
    if (ready == avail) {
	if (NetMultiSetupMenuItems[3].flags == MenuButtonDisabled) {
	    // enable start game button
	    NetMultiSetupMenuItems[3].flags = 0;
	}
    } else {
	// disable start game button
	NetMultiSetupMenuItems[3].flags = MenuButtonDisabled;
    }
}

/**
**	Update client network menu.
*/
local void MultiClientUpdate(int initial)
{
    int i, h, c;

    //	Calculate available slots from pudinfo
    for (c = h = i = 0; i < PlayerMax; i++) {
	if (ScenSelectPudInfo->PlayerType[i] == PlayerPerson) {
	    h++;	// available interactive player slots
	}
	if (ScenSelectPudInfo->PlayerType[i] == PlayerComputer) {
	    c++;	// available computer player slots
	}
    }

    if (initial) {
	NetMultiClientMenuItems[CLIENT_PLAYER_STATE] = NetMultiButtonStorage[1];
	NetMultiClientMenuItems[CLIENT_PLAYER_STATE].yofs = 32;
	memset(&ServerSetupState, 0, sizeof(ServerSetup));
	memset(&LocalSetupState, 0, sizeof(ServerSetup));
    }
    for (i = 1; i < PlayerMax - 1; i++) {
	if (Hosts[i].PlyNr) {
	    NetMultiClientMenuItems[CLIENT_PLAYER_STATE + i] =
		NetMultiButtonStorage[1];
	    if (i == NetLocalHostsSlot) {
		NetMultiClientMenuItems[CLIENT_PLAYER_READY - 1 + i].d.gem.state = 0;
	    } else {
		NetMultiClientMenuItems[CLIENT_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;
	    }
	} else {
	    NetMultiClientMenuItems[CLIENT_PLAYER_STATE + i] = NetMultiButtonStorage[0];
	    NetMultiClientMenuItems[CLIENT_PLAYER_STATE + i].d.pulldown.state = MI_PSTATE_PASSIVE;
	    NetMultiClientMenuItems[CLIENT_PLAYER_STATE + i].d.pulldown.curopt = ServerSetupState.CompOpt[i];
	    NetMultiClientMenuItems[CLIENT_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
	}
	NetMultiClientMenuItems[CLIENT_PLAYER_STATE + i].yofs = 32 + (i&7) * 22;
	if (i > 7) {
	    NetMultiClientMenuItems[CLIENT_PLAYER_STATE + i].xofs = 320 + 40;
	}

	NetMultiClientMenuItems[CLIENT_PLAYER_READY - 1 + i].flags = 0;
	if (ServerSetupState.Ready[i]) {
	    NetMultiClientMenuItems[CLIENT_PLAYER_READY - 1 + i].d.gem.state |= MI_GSTATE_CHECKED;
	} else {
	    NetMultiClientMenuItems[CLIENT_PLAYER_READY - 1 + i].d.gem.state &= ~MI_GSTATE_CHECKED;
	}

	if (i >= h) {
	    NetMultiClientMenuItems[CLIENT_PLAYER_STATE + i].d.pulldown.curopt = ServerSetupState.CompOpt[i];
	}
	if (i >= h + c) {
	    NetMultiClientMenuItems[CLIENT_PLAYER_READY - 1 + i].flags = MenuButtonDisabled;
	    NetMultiClientMenuItems[CLIENT_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
	    NetMultiClientMenuItems[CLIENT_PLAYER_STATE + i].d.pulldown.defopt =
			 NetMultiClientMenuItems[CLIENT_PLAYER_STATE + i].d.pulldown.curopt = 2;
	    NetMultiClientMenuItems[CLIENT_PLAYER_STATE + i].flags = MenuButtonDisabled;
	}
    }
}

local void MultiGameSetupInit(Menuitem *mi)
{
    int i, h;

    GameSetupInit(mi);
    NetworkInitServerConnect();
    NetMultiSetupMenuItems[SERVER_PLAYER_STATE] = NetMultiButtonStorage[1];
    NetMultiSetupMenuItems[SERVER_PLAYER_STATE].yofs = 32;

    memset(&ServerSetupState, 0, sizeof(ServerSetup));
    //	Calculate available slots from pudinfo
    for (h = i = 0; i < PlayerMax; i++) {
	if (ScenSelectPudInfo->PlayerType[i] == PlayerPerson) {
	    h++;	// available interactive player slots
	}
    }
    for (i = h; i < PlayerMax - 1; i++) {
	ServerSetupState.CompOpt[i] = 1;
    }
    MultiGamePlayerSelectorsUpdate(1);
    DebugLevel3Fn("h = %d, NetPlayers = %d\n" _C_ h _C_ NetPlayers);
}

local void MultiGameSetupExit(Menuitem *mi __attribute__((unused)))
{
    NetworkExitServerConnect();
}

/**
**	Cancel button of server multi player menu pressed.
*/
local void MultiGameCancel(void)
{
    MultiGameSetupExit(NULL);
    NetPlayers = 0;		// Make single player menus work again!
    GameCancel();
}

/**
**	Draw the multi player setup menu.
**
**	@note FIXME: the other buttons aren't updated if the player leaves.
*/
local void NetMultiPlayerDrawFunc(Menuitem *mi)
{
    int i, nc, rc;

    i = mi - NetMultiSetupMenuItems - SERVER_PLAYER_STATE;
    if (i >= 0 && i < PlayerMax - 1) {		// Ugly test to detect server
	if (i > 0) {
	    NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i].flags &=
		~MenuButtonDisabled;
	    // Note: re-disabled in MultiGamePlayerSelectorsUpdate()
	    //		for kicked out clients!!
	    if (ServerSetupState.Ready[i]) {
		NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE|MI_GSTATE_CHECKED;
	    } else {
		NetMultiSetupMenuItems[SERVER_PLAYER_READY - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE;
	    }
	    if (ServerSetupState.LastFrame[i]+30>FrameCounter) {
		NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i].flags &=
		    ~MenuButtonDisabled;
		NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE|MI_GSTATE_CHECKED;
	    } else {
		NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i].flags |=
		    MenuButtonDisabled;
		NetMultiSetupMenuItems[SERVER_PLAYER_LAG - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE;
	    }

	}
    } else {
	i = mi - NetMultiClientMenuItems - CLIENT_PLAYER_STATE;
	if (i > 0) {
	    NetMultiClientMenuItems[CLIENT_PLAYER_READY - 1 + i].flags &=
		~MenuButtonDisabled;
	    if (i == NetLocalHostsSlot) {
		NetMultiClientMenuItems[CLIENT_PLAYER_READY - 1 + i].
			d.gem.state &= ~MI_GSTATE_PASSIVE;
	    } else {
		NetMultiClientMenuItems[CLIENT_PLAYER_READY - 1 + i].
			d.gem.state |= MI_GSTATE_PASSIVE;
	    }
	}
    }
    /* FIXME:
	 further changes to implement client menu...
    */

    GetDefaultTextColors(&nc, &rc);
    SetDefaultTextColors(rc, rc);
    DebugLevel3Fn("Hosts[%d].PlyName = %s\n" _C_ i _C_ Hosts[i].PlyName);
    VideoDrawText(OffsetX+mi->xofs, OffsetY+mi->yofs, GameFont, Hosts[i].PlyName);

    SetDefaultTextColors(nc, rc);
}

/**
**	Cancel button of multiplayer client menu pressed.
*/
local void MultiClientCancel(void)
{
    NetworkDetachFromServer();
    // GameCancel();
}

local void MultiGameClientInit(Menuitem *mi __attribute__((unused)))
{
    // GameSetupInit(mi);
    MultiClientUpdate(1);
    if (LocalSetupState.Ready[NetLocalHostsSlot]) {
	NetMultiClientMenuItems[2].flags = MenuButtonDisabled;
	NetMultiClientMenuItems[3].flags = 0;
    } else {
	NetMultiClientMenuItems[3].flags = MenuButtonDisabled;
	NetMultiClientMenuItems[2].flags = 0;
    }
}

/**
**	Multiplayer client gem action. Toggles ready flag.
*/
local void MultiClientGemAction(Menuitem *mi)
{
    int i;

    i = mi - NetMultiClientMenuItems - CLIENT_PLAYER_READY + 1;
    DebugLevel3Fn("i = %d, NetLocalHostsSlot = %d\n" _C_ i _C_ NetLocalHostsSlot);
    if (i == NetLocalHostsSlot) {
	LocalSetupState.Ready[i] = !LocalSetupState.Ready[i];
	if (LocalSetupState.Ready[i]) {
	    NetMultiClientMenuItems[2].flags = MenuButtonDisabled;
	    NetMultiClientMenuItems[3].flags = 0;
	} else {
	    NetMultiClientMenuItems[3].flags = MenuButtonDisabled;
	    NetMultiClientMenuItems[2].flags = 0;
	}
	MultiClientUpdate(0);
    }
}

local void MultiClientRCSAction(Menuitem *mi, int i)
{
    if (mi->d.pulldown.curopt == i) {
	LocalSetupState.Race[NetLocalHostsSlot] = 2 - i;
	MultiClientUpdate(0);
    }
}

local void MultiClientReady(void)
{
    NetMultiClientMenuItems[2].flags = MenuButtonDisabled;
    NetMultiClientMenuItems[3].flags = 0;
    LocalSetupState.Ready[NetLocalHostsSlot] = 1;
    MultiClientUpdate(0);
}

local void MultiClientNotReady(void)
{
    NetMultiClientMenuItems[3].flags = MenuButtonDisabled;
    NetMultiClientMenuItems[2].flags = 0;
    LocalSetupState.Ready[NetLocalHostsSlot] = 0;
    MultiClientUpdate(0);
}

/*
 * Callback from netconnect loop in Client-Sync state:
 * Compare local state with server's information
 * and force update when changes have occured.
 */
global void NetClientCheckLocalState(void)
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

global int NetClientSelectScenario(void)
{
    char *cp;

    FreeMapInfo(ScenSelectPudInfo);
    ScenSelectPudInfo = NULL;

    cp = strrchr(ScenSelectFullPath, '/');
    if (cp) {
	strcpy(ScenSelectFileName, cp + 1);
	*cp = 0;
	strcpy(ScenSelectPath, ScenSelectFullPath);
	*cp = '/';
    } else {
	strcpy(ScenSelectFileName, ScenSelectFullPath);
	ScenSelectPath[0] = 0;
    }

    if (strcasestr(ScenSelectFileName, ".pud")) {
	ScenSelectPudInfo = GetPudInfo(ScenSelectFullPath);
    } else {
	// FIXME: GetCmInfo();
    }
    return ScenSelectPudInfo == NULL;
}

global void NetConnectForceDisplayUpdate(void)
{
    MultiGamePlayerSelectorsUpdate(2);
    MustRedraw |= RedrawMenu;
}

/**
**	Update client menu to follow server menu.
*/
global void NetClientUpdateState(void)
{
    GameRESAction(NULL, ServerSetupState.ResOpt);
    NetMultiClientMenuItems[CLIENT_RESOURCE].d.pulldown.curopt =
	ServerSetupState.ResOpt;

    GameUNSAction(NULL, ServerSetupState.UnsOpt);
    NetMultiClientMenuItems[CLIENT_UNITS].d.pulldown.curopt =
	ServerSetupState.UnsOpt;

    MultiGameFWSAction(NULL, ServerSetupState.FwsOpt);
    NetMultiClientMenuItems[CLIENT_FOG_OF_WAR].d.pulldown.curopt =
	ServerSetupState.FwsOpt;

    GameTSSAction(NULL, ServerSetupState.TssOpt);
    NetMultiClientMenuItems[CLIENT_TILESET].d.pulldown.curopt =
	ServerSetupState.TssOpt;

    MultiClientUpdate(0);
    DebugLevel1Fn("MultiClientMenuRedraw\n");

    MustRedraw |= RedrawMenu;
}

/**
**	Start editor.
*/
local void StartEditor(void)
{
}

/*----------------------------------------------------------------------------
--	Menu operation functions
----------------------------------------------------------------------------*/

/**
**	Handle keys in menu mode.
**
**	@param key	Key scancode.
**	@param keychar	ASCII character code of key.
**
**	@todo FIXME: Should be MenuKeyDown(), and act on _new_ MenuKeyUp() !!!
**      to implement button animation (depress before action)
*/
local void MenuHandleKeyDown(unsigned key,unsigned keychar)
{
    int i, n;
    Menuitem *mi;
    Menu *menu;

    HandleKeyModifiersDown(key,keychar);

    if (CurrentMenu < 0) {
	return;
    }
    menu = Menus + CurrentMenu;

    if (MenuButtonCurSel != -1 && menu->items[MenuButtonCurSel].mitype == MI_TYPE_INPUT) {
	mi = menu->items + MenuButtonCurSel;
	if (!(mi->flags & MenuButtonDisabled)) {
inkey:
#if 0
	    // Key is unsigned
	    if (key < 0) {
		key &= 0xFF;
	    }
#endif
	    if (key >= 0x80 && key < 0x100) {
		// FIXME ARI: ISO->WC2 Translation here!
		key = 0;
	    }
	    switch(key) {
		case '\b': case '\177':
		    if (mi->d.input.nch > 0) {
			strcpy(mi->d.input.buffer + (--mi->d.input.nch), "~!_");
			MustRedraw |= RedrawMenu;
		    }
		    break;
		case 9:
		    goto normkey;
		case 'x':
		case 'X':
		    if( (KeyModifiers&ModifierAlt) ) {
			goto normkey;
		    }
		    /* FALL THROUGH */
		default:
		    if (key >= 32 && key < 0x100) {
			if (mi->d.input.nch < mi->d.input.maxch) {
			    mi->d.input.buffer[mi->d.input.nch++] = keychar;
			    strcpy(mi->d.input.buffer + mi->d.input.nch, "~!_");
			    MustRedraw |= RedrawMenu;
			}
		    }
		    break;
	    }
	    if (mi->d.input.action) {
		(*mi->d.input.action)(mi, key);
	    }
	    return;
	}
    }

normkey:
    if( !(KeyModifiers&ModifierAlt) ) {
	mi = menu->items;
	i = menu->nitems;
	while (i--) {
	    switch (mi->mitype) {
		case MI_TYPE_BUTTON:
		    if (key == mi->d.button.hotkey) {
			if (mi->d.button.handler) {
			    (*mi->d.button.handler)();
			}
			return;
		    }
		default:
		    break;
	    }
	    mi++;
	}
    }
    switch (key) {
	case 10: case 13:			// RETURN
	    if (MenuButtonCurSel != -1) {
		mi = menu->items + MenuButtonCurSel;
		switch (mi->mitype) {
		    case MI_TYPE_BUTTON:
			if (mi->d.button.handler) {
			    (*mi->d.button.handler)();
			}
			return;
		    case MI_TYPE_LISTBOX:
			if (mi->d.listbox.handler) {
			    (*mi->d.listbox.handler)();
			}
			return;
		    case MI_TYPE_VSLIDER:
			if (mi->d.vslider.handler) {
			    (*mi->d.vslider.handler)();
			}
			return;
		    case MI_TYPE_HSLIDER:
			if (mi->d.hslider.handler) {
			    (*mi->d.hslider.handler)();
			}
			return;
		    default:
			break;
		}
	    }
	    break;
	case KeyCodeUp: case KeyCodeDown:
	    if (MenuButtonCurSel != -1) {
		mi = menu->items + MenuButtonCurSel;
		if (!(mi->flags&MenuButtonClicked)) {
		    switch (mi->mitype) {
			case MI_TYPE_PULLDOWN:
			    if (key == KeyCodeDown) {
				if (mi->d.pulldown.curopt + 1 < mi->d.pulldown.noptions)
				    mi->d.pulldown.curopt++;
				else
				    break;
			    } else {
				if (mi->d.pulldown.curopt > 0)
				    mi->d.pulldown.curopt--;
				else
				    break;
			    }
			    MustRedraw |= RedrawMenu;
			    if (mi->d.pulldown.action) {
				(*mi->d.pulldown.action)(mi, mi->d.pulldown.curopt);
			    }
			    break;
			case MI_TYPE_LISTBOX:
			    if (key == KeyCodeDown) {
				if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.pulldown.noptions) {
				    mi->d.listbox.curopt++;
				    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
					mi->d.listbox.curopt--;
					mi->d.listbox.startline++;
				    }
				} else {
				    break;
				}
			    } else {
				if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
				    mi->d.listbox.curopt--;
				    if (mi->d.listbox.curopt < 0) {
					mi->d.listbox.curopt++;
					mi->d.listbox.startline--;
				    }
				} else {
				    break;
				}
			    }
			    if (mi->d.listbox.action) {
				(*mi->d.listbox.action)(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			    }
			    MustRedraw |= RedrawMenu;
			    break;
			case MI_TYPE_VSLIDER:
			    if (key == KeyCodeDown) {
				mi->d.vslider.cflags |= MI_CFLAGS_DOWN;
			    } else {
				mi->d.vslider.cflags |= MI_CFLAGS_UP;
			    }
			    if (mi->d.vslider.action) {
				(*mi->d.vslider.action)(mi, 2);		// 0 indicates key down
			    }
			    MustRedraw |= RedrawMenu;
			    break;
			default:
			    break;
		    }
		}
	    }
	    break;
	case 9:				// TAB			// FIXME: Add Shift-TAB
	    if (MenuButtonCurSel != -1 && !(menu->items[MenuButtonCurSel].flags&MenuButtonClicked)) {
		n = menu->nitems;
		for (i = 0; i < n; ++i) {
		    mi = menu->items + ((MenuButtonCurSel + i + 1) % n);
		    switch (mi->mitype) {
			case MI_TYPE_PULLDOWN:
			    if ((mi->d.pulldown.state & MI_PSTATE_PASSIVE)) {
				continue;
			    }
			    /* FALL THROUGH */
			case MI_TYPE_BUTTON:
			case MI_TYPE_LISTBOX:
			case MI_TYPE_VSLIDER:
			case MI_TYPE_HSLIDER:
			case MI_TYPE_INPUT:
			    if (mi->flags & MenuButtonDisabled) {
				break;
			    }
			    mi->flags |= MenuButtonSelected;
			    menu->items[MenuButtonCurSel].flags &= ~MenuButtonSelected;
			    MenuButtonCurSel = mi - menu->items;
			    MustRedraw |= RedrawMenu;
			    return;
			default:
			    break;
		    }
		}
	    }
	    break;
	case 'x':
	case 'X':
	    if( (KeyModifiers&ModifierAlt) ) {
		Exit(0);
	    }
	default:
	    mi = menu->items;
	    i = menu->nitems;
	    while (i--) {
		switch (mi->mitype) {
		    case MI_TYPE_INPUT:
			if (!(mi->flags & MenuButtonDisabled)) {
			    menu->items[MenuButtonCurSel].flags &= ~MenuButtonSelected;
			    mi->flags |= MenuButtonSelected;
			    MenuButtonCurSel = mi - menu->items;
			    MustRedraw |= RedrawMenu;
			    goto inkey;
			}
		    default:
			break;
		}
		mi++;
	    }
	    DebugLevel3("Key %d\n" _C_ key);
	    return;
    }
    return;
}

/**
**	Handle keys in menu mode.
**
**	@param key	Key scancode.
**	@param keychar	ASCII character code of key.
*/
local void MenuHandleKeyUp(unsigned key,unsigned keychar)
{
    HandleKeyModifiersUp(key,keychar);
}

/**
**	Handle keys repeated in menu mode.
**
**	@param key	Key scancode.
**	@param keychar	ASCII character code of key.
*/
local void MenuHandleKeyRepeat(unsigned key,unsigned keychar)
{
    Menu *menu;

    HandleKeyModifiersDown(key,keychar);

    if (CurrentMenu < 0) {
	return;
    }
    menu = Menus + CurrentMenu;

    if (MenuButtonCurSel != -1 && menu->items[MenuButtonCurSel].mitype == MI_TYPE_INPUT) {
	MenuHandleKeyDown(key,keychar);
    }
}

/**
**	Handle movement of the cursor.
**
**	@param x	Screen X position.
**	@param y	Screen Y position.
*/
local void MenuHandleMouseMove(int x,int y)
{
    int h, w, i, j, n, xs, ys;
    Menuitem *mi;
    Menu *menu = Menus + CurrentMenu;
    int RedrawFlag = 0;

    HandleCursorMove(&x,&y);

    if (CurrentMenu == -1)
	return;

    n = menu->nitems;
    MenuButtonUnderCursor = -1;

    /* check active (popped-up) pulldown first, as it may overlay other menus! */
    mi = menu->items;
    for (i = 0; i < n; ++i) {
	if (!(mi->flags&MenuButtonDisabled)) {
	    if (mi->mitype == MI_TYPE_PULLDOWN && (mi->flags&MenuButtonClicked)) {
		xs = menu->x + mi->xofs;
		ys = menu->y + mi->yofs;
		h = mi->d.pulldown.ysize - 2;
		ys -= mi->d.pulldown.curopt * h;
		if (!(x<xs || x>xs + mi->d.pulldown.xsize || y<ys || y>ys + h*mi->d.pulldown.noptions)) {
		    j = (y - ys) / h;
		    if (j >= 0 && j < mi->d.pulldown.noptions && j != mi->d.pulldown.cursel) {
			mi->d.pulldown.cursel = j;
			RedrawFlag = 1;
			if (mi->d.pulldown.action) {
			    (*mi->d.pulldown.action)(mi, mi->d.pulldown.cursel);
			}
		    }
		}
		MenuButtonUnderCursor = i;
		break;
	    }
	}
	mi++;
    }
    if (MenuButtonUnderCursor == -1) {
	for (i = 0; i < n; ++i) {
	    mi = menu->items + i;
	    if (!(mi->flags&MenuButtonDisabled)) {
		switch (mi->mitype) {
		    case MI_TYPE_GEM:
			xs = menu->x + mi->xofs;
			ys = menu->y + mi->yofs;
			if (x < xs || x > xs + mi->d.gem.xsize || y < ys || y > ys + mi->d.gem.ysize) {
			    if (!(mi->flags&MenuButtonClicked)) {
				if (mi->flags&MenuButtonActive) {
				    RedrawFlag = 1;
				    mi->flags &= ~MenuButtonActive;
				}
			    }
			    continue;
			}
			break;
		    case MI_TYPE_BUTTON:
			xs = menu->x + mi->xofs;
			ys = menu->y + mi->yofs;
			if (x < xs || x > xs + mi->d.button.xsize || y < ys || y > ys + mi->d.button.ysize) {
			    if (!(mi->flags&MenuButtonClicked)) {
				if (mi->flags&MenuButtonActive) {
				    RedrawFlag = 1;
				    mi->flags &= ~MenuButtonActive;
				}
			    }
			    continue;
			}
			break;
		    case MI_TYPE_INPUT:
			xs = menu->x + mi->xofs;
			ys = menu->y + mi->yofs;
			if (x<xs || x>xs + mi->d.input.xsize || y<ys || y>ys + mi->d.input.ysize) {
			    if (!(mi->flags&MenuButtonClicked)) {
				if (mi->flags&MenuButtonActive) {
				    RedrawFlag = 1;
				    mi->flags &= ~MenuButtonActive;
				}
			    }
			    continue;
			}
			break;
		    case MI_TYPE_PULLDOWN:
			if ((mi->d.pulldown.state & MI_PSTATE_PASSIVE)) {
			    continue;
			}
			// Clicked-state already checked above - there can only be one!
			xs = menu->x + mi->xofs;
			ys = menu->y + mi->yofs;
			if (x<xs || x>xs + mi->d.pulldown.xsize || y<ys || y>ys + mi->d.pulldown.ysize) {
			    if (!(mi->flags&MenuButtonClicked)) {
				if (mi->flags&MenuButtonActive) {
				    RedrawFlag = 1;
				    mi->flags &= ~MenuButtonActive;
				}
			    }
			    continue;
			}
			break;
		    case MI_TYPE_LISTBOX:
			xs = menu->x + mi->xofs;
			ys = menu->y + mi->yofs;
			if (x < xs || x > xs + mi->d.listbox.xsize || y < ys || y > ys + mi->d.listbox.ysize) {
			    if (!(mi->flags&MenuButtonClicked)) {
				if (mi->flags&MenuButtonActive) {
				    RedrawFlag = 1;
				    mi->flags &= ~MenuButtonActive;
				}
			    }
			    continue;
			}
			j = (y - ys) / 18;
			if (j != mi->d.listbox.cursel) {
			    mi->d.listbox.cursel = j;	// just store for click
			}
			break;
		    case MI_TYPE_VSLIDER:
			xs = menu->x + mi->xofs;
			ys = menu->y + mi->yofs;
			if (x < xs || x > xs + mi->d.vslider.xsize || y < ys || y > ys + mi->d.vslider.ysize) {
			    if (!(mi->flags&MenuButtonClicked)) {
				if (mi->flags&MenuButtonActive) {
				    RedrawFlag = 1;
				    mi->flags &= ~MenuButtonActive;
				}
			    }
			    mi->d.vslider.cursel = 0;
			    continue;
			}
			j = y - ys;
			mi->d.vslider.cursel = 0;

			if (j < 20) {
			    mi->d.vslider.cursel |= MI_CFLAGS_UP;
			} else if (j > mi->d.vslider.ysize - 20) {
			    mi->d.vslider.cursel |= MI_CFLAGS_DOWN;
			} else {
			    mi->d.vslider.cursel &= ~(MI_CFLAGS_UP|MI_CFLAGS_DOWN);
			    h = (mi->d.vslider.percent * (mi->d.vslider.ysize - 54)) / 100 + 18;
			    if (j > h && j < h + 18) {
				mi->d.vslider.cursel |= MI_CFLAGS_KNOB;
			    } else {
				mi->d.vslider.cursel |= MI_CFLAGS_CONT;
				if (j <= h) {
				    mi->d.vslider.cursel |= MI_CFLAGS_UP;
				} else {
				    mi->d.vslider.cursel |= MI_CFLAGS_DOWN;
				}
			    }
			    j -= 8;
			    if (j < 20) j=20;

			    mi->d.vslider.curper = ((j - 20) * 100) / (mi->d.vslider.ysize - 54);
			    if (mi->d.vslider.curper > 100) {
				mi->d.vslider.curper = 100;
			    }
			}
			if (mi->d.vslider.action) {
			    (*mi->d.vslider.action)(mi, 1);		// 1 indicates move
			}
			break;
		    case MI_TYPE_HSLIDER:
			xs = menu->x + mi->xofs;
			ys = menu->y + mi->yofs;
			if (x < xs || x > xs + mi->d.hslider.xsize || y < ys || y > ys + mi->d.hslider.ysize) {
			    if (!(mi->flags&MenuButtonClicked)) {
				if (mi->flags&MenuButtonActive) {
				    RedrawFlag = 1;
				    mi->flags &= ~MenuButtonActive;
				}
			    }
			    mi->d.hslider.cursel = 0;
			    continue;
			}
			j = x - xs;
			j -= 6;
			if (j < 20) {
			    mi->d.hslider.cursel |= MI_CFLAGS_LEFT;
			} else if (j > mi->d.hslider.xsize - 20) {
			    mi->d.hslider.cursel |= MI_CFLAGS_RIGHT;
			} else {
			    mi->d.hslider.cursel &= ~(MI_CFLAGS_LEFT|MI_CFLAGS_RIGHT);
			    w = (mi->d.hslider.percent * (mi->d.hslider.xsize - 54)) / 100 + 18;
			    mi->d.hslider.curper = ((j - 20) * 100) / (mi->d.hslider.xsize - 54);
			    if (mi->d.hslider.curper > 100) {
				mi->d.hslider.curper = 100;
			    }
			    if (j > w && j < w + 18) {
				mi->d.hslider.cursel |= MI_CFLAGS_KNOB;
			    } else {
				mi->d.hslider.cursel |= MI_CFLAGS_CONT;
				if (j <= w) {
				    mi->d.hslider.cursel |= MI_CFLAGS_LEFT;
				} else {
				    mi->d.hslider.cursel |= MI_CFLAGS_RIGHT;
				}
			    }
			}
			if (mi->d.hslider.action) {
			    (*mi->d.hslider.action)(mi, 1);		// 1 indicates move
			}
			break;
		    default:
			continue;
			// break;
		}
		switch (mi->mitype) {
		    case MI_TYPE_GEM:
			if ((mi->d.gem.state & MI_GSTATE_PASSIVE)) {
			    break;
			}
			/* FALL THROUGH */
		    case MI_TYPE_BUTTON:
		    case MI_TYPE_PULLDOWN:
		    case MI_TYPE_LISTBOX:
		    case MI_TYPE_VSLIDER:
		    case MI_TYPE_HSLIDER:
		    case MI_TYPE_INPUT:
			if (!(mi->flags&MenuButtonActive)) {
			    RedrawFlag = 1;
			    mi->flags |= MenuButtonActive;
			}
			MenuButtonUnderCursor = i;
		    default:
			break;
		}
	    }
	}
    }
    if (RedrawFlag) {
	MustRedraw |= RedrawMenu;
    }
}

/**
**	Called if mouse button pressed down.
**
**	@param b	button code
*/
local void MenuHandleButtonDown(unsigned b __attribute__((unused)))
{
    Menuitem *mi;
    Menu *menu = Menus + CurrentMenu;

    if (CurrentMenu == -1)
	return;

    if (MouseButtons&(LeftButton<<MouseHoldShift))
	return;

    if (MouseButtons&LeftButton) {
	if (MenuButtonUnderCursor != -1) {
	    mi = menu->items + MenuButtonUnderCursor;
	    if (!(mi->flags&MenuButtonClicked)) {
		switch (mi->mitype) {
		    case MI_TYPE_GEM:
		    case MI_TYPE_BUTTON:
		    case MI_TYPE_PULLDOWN:
		    case MI_TYPE_LISTBOX:
		    case MI_TYPE_VSLIDER:
		    case MI_TYPE_HSLIDER:
		    case MI_TYPE_INPUT:
			if (MenuButtonCurSel != -1) {
			    menu->items[MenuButtonCurSel].flags &= ~MenuButtonSelected;
			}
			MenuButtonCurSel = MenuButtonUnderCursor;
			mi->flags |= MenuButtonClicked|MenuButtonSelected;
			MustRedraw |= RedrawMenu;
		    default:
			break;
		}
	    }
	    PlayGameSound(GameSounds.Click.Sound,MaxSampleVolume);
	    switch (mi->mitype) {
		case MI_TYPE_VSLIDER:
		    mi->d.vslider.cflags = mi->d.vslider.cursel;
		    if (mi->d.vslider.action) {
			(*mi->d.vslider.action)(mi, 0);		// 0 indicates down
		    }
		    break;
		case MI_TYPE_HSLIDER:
		    mi->d.hslider.cflags = mi->d.hslider.cursel;
		    if (mi->d.hslider.action) {
			(*mi->d.hslider.action)(mi, 0);		// 0 indicates down
		    }
		    break;
		case MI_TYPE_PULLDOWN:
		    if (mi->d.pulldown.curopt >= 0 &&
					    mi->d.pulldown.curopt < mi->d.pulldown.noptions) {
			mi->d.pulldown.cursel = mi->d.pulldown.curopt;
		    }
		    break;
		case MI_TYPE_LISTBOX:
		    if (mi->d.listbox.cursel != mi->d.listbox.curopt) {
			mi->d.listbox.curopt = mi->d.listbox.cursel;
			if (mi->d.listbox.action) {
			    (*mi->d.listbox.action)(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			}
		    } else if (mi->d.listbox.handler) {
			// double click support - maybe limit time!
			(*mi->d.listbox.handler)();
		    }
		    break;
		default:
		    break;
	    }
	}
    }
}

/**
**	Called if mouse button released.
**
**	@param b	button code
*/
local void MenuHandleButtonUp(unsigned b)
{
    int i, n;
    Menuitem *mi;
    Menu *menu = Menus + CurrentMenu;
    int RedrawFlag = 0;

    if (CurrentMenu == -1)
	return;

    if ((1<<b) == LeftButton) {
	n = menu->nitems;
	for (i = 0; i < n; ++i) {
	    mi = menu->items + i;
	    switch (mi->mitype) {
		case MI_TYPE_GEM:
		    if (mi->flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->flags &= ~MenuButtonClicked;
			if (MenuButtonUnderCursor == i) {
			    MenuButtonUnderCursor = -1;
			    if ((mi->d.gem.state & MI_GSTATE_CHECKED)) {
				mi->d.gem.state &= ~MI_GSTATE_CHECKED;
			    } else {
				mi->d.gem.state |= MI_GSTATE_CHECKED;
			    }
			    if (mi->d.gem.action) {
				(*mi->d.gem.action)(mi);
			    }
			}
		    }
		    break;
		case MI_TYPE_BUTTON:
		    if (mi->flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->flags &= ~MenuButtonClicked;
			if (MenuButtonUnderCursor == i) {
			    MenuButtonUnderCursor = -1;
			    if (mi->d.button.handler) {
				(*mi->d.button.handler)();
			    }
			}
		    }
		    break;
		case MI_TYPE_PULLDOWN:
		    if (mi->flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->flags &= ~MenuButtonClicked;
			if (MenuButtonUnderCursor == i) {
			    MenuButtonUnderCursor = -1;
			    if (mi->d.pulldown.cursel != mi->d.pulldown.curopt &&
					    mi->d.pulldown.cursel >= 0 &&
					    mi->d.pulldown.cursel < mi->d.pulldown.noptions) {
				mi->d.pulldown.curopt = mi->d.pulldown.cursel;
				if (mi->d.pulldown.action) {
				    (*mi->d.pulldown.action)(mi, mi->d.pulldown.curopt);
				}
			    }
			}
			mi->d.pulldown.cursel = 0;
		    }
		    break;
		case MI_TYPE_LISTBOX:
		case MI_TYPE_INPUT:
		    if (mi->flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->flags &= ~MenuButtonClicked;
			// MAYBE ADD HERE
		    }
		    break;
		case MI_TYPE_VSLIDER:
		    if (mi->flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->flags &= ~MenuButtonClicked;
			mi->d.vslider.cflags = 0;
		    }
		    break;
		case MI_TYPE_HSLIDER:
		    if (mi->flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->flags &= ~MenuButtonClicked;
			mi->d.hslider.cflags = 0;
		    }
		    break;
		default:
		    break;
	    }
	}
    }
    if (RedrawFlag) {
	MustRedraw |= RedrawMenu;

	MenuHandleMouseMove(CursorX,CursorY);
    }
}

/**
**	End process menu
**
*/
local void EndMenu(void)
{
    CursorOn = CursorOnUnknown;
    CurrentMenu = -1;

    MustRedraw = RedrawEverything;
    InterfaceState = IfaceStateNormal;
    UpdateDisplay();
    InterfaceState = IfaceStateMenu;
    MustRedraw = RedrawMenu;
}

/**
**	Process a menu.
**
**	@param menu_id	The menu number to process
**	@param loop	Indicates to setup handlers and really 'Process'
**
**	@todo FIXME: This function is called from the event handler!!
*/
global void ProcessMenu(int menu_id, int loop)
{
    int i, oldncr;
    Menu *menu;
    Menuitem *mi;
    int CurrentMenuSave = -1;
    int MenuButtonUnderCursorSave = -1;
    int MenuButtonCurSelSave = -1;

    CancelBuildingMode();

    // Recursion protection:
    if (loop) {
	CurrentMenuSave = CurrentMenu;
	MenuButtonUnderCursorSave = MenuButtonUnderCursor;
	MenuButtonCurSelSave = MenuButtonCurSel;
    }

    InterfaceState = IfaceStateMenu;
    VideoLockScreen();
    HideAnyCursor();
    VideoUnlockScreen();
    DestroyCursorBackground();
    MustRedraw |= RedrawCursor;
    CursorState = CursorStatePoint;
    GameCursor = TheUI.Point.Cursor;
    CurrentMenu = menu_id;
    menu = Menus + CurrentMenu;
    MenuButtonCurSel = -1;
    for (i = 0; i < menu->nitems; ++i) {
	mi = menu->items + i;
	switch (mi->mitype) {
	    case MI_TYPE_BUTTON:
	    case MI_TYPE_PULLDOWN:
	    case MI_TYPE_LISTBOX:
	    case MI_TYPE_VSLIDER:
	    case MI_TYPE_HSLIDER:
	    case MI_TYPE_INPUT:
		mi->flags &= ~(MenuButtonClicked|MenuButtonActive
				|MenuButtonSelected);
		if (i == menu->defsel) {
		    mi->flags |= MenuButtonSelected;
		    MenuButtonCurSel = i;
		}
		break;
	}
	switch (mi->mitype) {
	    case MI_TYPE_PULLDOWN:
		mi->d.pulldown.cursel = 0;
		if (mi->d.pulldown.defopt != -1)
		    mi->d.pulldown.curopt = mi->d.pulldown.defopt;
		break;
	    case MI_TYPE_LISTBOX:
		mi->d.listbox.cursel = -1;
		mi->d.listbox.startline = 0;
		if (mi->d.listbox.defopt != -1)
		    mi->d.listbox.curopt = mi->d.listbox.defopt;
		break;
	    case MI_TYPE_VSLIDER:
		mi->d.vslider.cflags = 0;
		if (mi->d.vslider.defper != -1)
		    mi->d.vslider.percent = mi->d.vslider.defper;
		break;
	    case MI_TYPE_HSLIDER:
		mi->d.hslider.cflags = 0;
		if (mi->d.hslider.defper != -1)
		    mi->d.hslider.percent = mi->d.hslider.defper;
		break;
	    default:
		break;
	}
	if (mi->initfunc) {
	    (*mi->initfunc)(mi);
	}
    }
    MenuButtonUnderCursor = -1;
    if (loop) {
	SetVideoSync();
	MustRedraw = 0;
	MenuHandleMouseMove(CursorX,CursorY);	// This activates buttons as appropriate!
	MustRedraw |= RedrawCursor;
    }


    VideoLockScreen();
    DrawMenu(CurrentMenu);
    InvalidateAreaAndCheckCursor(MenuRedrawX,MenuRedrawY,MenuRedrawW,MenuRedrawH);
    MustRedraw&=~RedrawMenu;
    VideoUnlockScreen();

    if (loop) {
	while (CurrentMenu != -1) {
	    DebugLevel3("MustRedraw: 0x%08x\n" _C_ MustRedraw);
	    if (MustRedraw) {
		UpdateDisplay();
	    }
	    RealizeVideoMemory();
	    oldncr = NetConnectRunning;
	    WaitEventsOneFrame(&callbacks);
	    if (NetConnectRunning == 2) {
		NetworkProcessClientRequest();
		MustRedraw |= RedrawMenu;
	    }
	    // stopped by network activity?
	    if (oldncr == 2 && NetConnectRunning == 0) {
		if (menu->netaction) {
		    (*menu->netaction)();
		}
	    }
	}
    }

    for (i = 0; i < menu->nitems; ++i) {
	mi = menu->items + i;
	if (mi->exitfunc) {
	    (*mi->exitfunc)(mi);		// action/destructor
	}
    }

    if (loop) {
	CurrentMenu = CurrentMenuSave;
	MenuButtonUnderCursor = MenuButtonUnderCursorSave;
	MenuButtonCurSel = MenuButtonCurSelSave;
    }

    // FIXME: Johns good point?
    if (Menusbgnd) {
	VideoFree(Menusbgnd);
	Menusbgnd = NULL;
    }
}

/**
**	Move buttons so they're centered on different resolutions
*/
local void MoveButtons()
{
    int menus = sizeof(Menus) / sizeof(Menu);
    int i;

    OffsetX = (VideoWidth - 640) / 2;
    OffsetY = (VideoHeight - 480) / 2;

    for (i=0; i<menus; i++) {
	Menus[i].x += OffsetX;
	Menus[i].y += OffsetY;
    }
}

/**
**	Init Menus for a specific race
**
**	@param race	The Race to set-up for
*/
global void InitMenus(unsigned int race)
{
    static int last_race = -1;
    const char *file;
    char *buf;

    if (race == last_race) {	// same race? already loaded!
	return;
    }

    if (last_race == -1) {
	// There go all my Gnuish compile time inits -
	// Why is ANSI C so dumb that it cannot
	// even initialize unions sanely?
	InitGameMenuItems();
	InitVictoryMenuItems();
	InitLostMenuItems();
	InitTipsMenuItems();
	InitObjectivesMenuItems();
	InitEndScenarioMenuItems();
	InitScenSelectMenuItems();
	InitPrgStartMenuItems();
	InitCustomGameMenuItems();
	InitEnterNameMenuItems();
	InitEnterServerIPMenuItems();
	InitNetCreateJoinMenuItems();
	InitNetMultiButtonStorage();
	InitNetMultiSetupMenuItems();
	InitNetMultiClientMenuItems();
	InitNetErrorMenuItems();
	InitConnectingMenuItems();
	InitCampaignSelectMenuItems();
	InitCampaignContMenuItems();
	InitSoundOptionsMenuItems();
	InitPreferencesMenuItems();
	InitSpeedSettingsMenuItems();
	InitGameOptionsMenuItems();
	InitHelpMenuItems();
	InitKeystrokeHelpMenuItems();
	InitSaveGameMenuItems();
	InitLoadGameMenuItems();
	InitConfirmSaveMenuItems();
	InitConfirmDeleteMenuItems();

	if (VideoWidth != 640) {
	    MoveButtons();
	}

	callbacks.ButtonPressed = &MenuHandleButtonDown;
	callbacks.ButtonReleased = &MenuHandleButtonUp;
	callbacks.MouseMoved = &MenuHandleMouseMove;
	callbacks.MouseExit = &HandleMouseExit;
	callbacks.KeyPressed = &MenuHandleKeyDown;
	callbacks.KeyReleased = &MenuHandleKeyUp;
	callbacks.KeyRepeated = &MenuHandleKeyRepeat;
	callbacks.NetworkEvent = NetworkEvent;
	callbacks.SoundReady = WriteSound;

    } else {
    	// free previous sprites for different race
	VideoFree(MenuButtonGfx.Sprite);
    }
    last_race = race;
    file = MenuButtonGfx.File[race];
    buf = alloca(strlen(file) + 9 + 1);
    file = strcat(strcpy(buf, "graphics/"), file);
    MenuButtonGfx.Sprite = LoadSprite(file, 300, 144);	// 50/53 images!

    //
    //	Autodetect the swamp tileset
    //
#ifdef HAVE_EXPANSION
    strcpy(ScenSelectFullPath, FreeCraftLibPath);
    if (ScenSelectFullPath[0]) {
	strcat(ScenSelectFullPath, "/graphics/tilesets/");
    }
    strcat(ScenSelectFullPath, "swamp");
    if (access(ScenSelectFullPath, F_OK) != 0) {
	// ARI FIXME: Hack to disable Expansion Gfx..
	// also shows how to add new tilesets....
	// - FIXME2:
	// With new dynamic tileset configuration this
	// should read what siod-config gave us and
	// build the menu from that..
	CustomGameMenuItems[14].d.pulldown.noptions = 4;
    }
#else
    CustomGameMenuItems[14].d.pulldown.noptions = 4;
#endif
}

/**
**	Exit Menus code (freeing data)
**
**	// FIXME: NOT CALLED YET.....!!!!!
*/
global void ExitMenus(void)
{
    if (Menusbgnd) {
	VideoFree(Menusbgnd);
	Menusbgnd = NULL;
    }
}

//@}
