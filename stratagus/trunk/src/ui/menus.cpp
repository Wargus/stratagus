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
/**@name menus.c	-	The menu function code. */
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
#include <ctype.h>
#include <limits.h>

#include "iocompat.h"

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
#include "editor.h"

#if defined(USE_SDLCD) || defined(USE_SDLA)
#include "SDL.h"
#endif

#ifdef USE_LIBCDA
#include "libcda.h"
#endif

//#define OLD_MENU		/// CCL menus not used
//#define SAVE_MENU_CCL		/// SAVE (REWRITE!) the menus.ccl file

/*----------------------------------------------------------------------------
--	Prototypes for local functions
----------------------------------------------------------------------------*/

local void EditorEndMenu(void);

/*----------------------------------------------------------------------------
--	Prototypes for action handlers and helper functions
----------------------------------------------------------------------------*/

local void GameMenuEnd(void);
local void GameMenuExit(void);
local void GameMenuReturn(void);
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

local void GlobalOptions(Menuitem *mi);
local void InitGlobalOptions(Menuitem *mi);
local void SetRes640(Menuitem *mi);
local void SetRes800(Menuitem *mi);
local void SetRes1024(Menuitem *mi);
local void SetRes1280(Menuitem *mi);
local void SetRes1600(Menuitem *mi);
local void SetFullscreen(Menuitem *mi);
local void SetShadowFogAlpha(Menuitem *mi);
local void SetShadowFogGray(Menuitem *mi);

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

local void InitSaveGameMenu(Menuitem *mi);
local void EnterSaveGameAction(Menuitem *mi, int key);
local void SaveAction(void);
local void CreateSaveDir(void);

local void SaveLBExit(Menuitem *mi);
local void SaveLBInit(Menuitem *mi);
local unsigned char *SaveLBRetrieve(Menuitem *mi, int i);
local void SaveLBAction(Menuitem *mi, int i);
local void SaveVSAction(Menuitem *mi, int i);
local void SaveOk(void);

local void InitLoadGameMenu(Menuitem *mi);
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
local void EditorNewMap(void);
local void EditorLoadMap(void);
local void EditorLoadInit(Menuitem *mi);
local void EditorLoadLBInit(Menuitem *mi);
local void EditorLoadLBExit(Menuitem *mi);
local void EditorLoadFolder(void);
local int EditorLoadRDFilter(char *pathbuf, FileList *fl);
local void EditorLoadLBAction(Menuitem *mi, int i);
local unsigned char *EditorLoadLBRetrieve(Menuitem *mi, int i);
local void EditorLoadOk(void);
local void EditorLoadCancel(void);
local void EditorLoadVSAction(Menuitem *mi, int i);
local void EditorMapProperties(void);
local void EditorPlayerProperties(void);
local void EditorEnterMapDescriptionAction(Menuitem *mi, int key);
local void EditorMapPropertiesOk(void);
local void EditorQuitMenu(void);

global void SaveMenus(FILE* file);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifdef SAVE_MENU_CCL
typedef char char30[30];
global hashtable(char30,MENUS_MAXFUNC) MenuFuncHash2;
#endif
global _MenuHash MenuHash;
global _MenuFuncHash MenuFuncHash;

    /// Name, Version, Copyright FIXME: move to headerfile
extern char NameLine[];

local int GameLoaded;
local int EditorLoadCancelled;

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
**	Items for the Game Menu
*/
local Menuitem GameMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, InitGameMenu, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16 + 12 + 106, 40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36 + 36 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

/**
**	Items for the Victory Menu
*/
local Menuitem VictoryMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 90, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 56, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};

#ifdef OLD_MENU
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
#endif

/**
**	Items for the Lost Menu
*/
local Menuitem LostMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 90, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitLostMenuItems() {
    MenuitemText   i0 = { "You failed to", MI_TFLAGS_CENTERED};
    MenuitemText   i1 = { "achieve victory!", MI_TFLAGS_CENTERED};
    MenuitemButton i2 = { "~!OK", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'o'};
    LostMenuItems[0].d.text   = i0;
    LostMenuItems[1].d.text   = i1;
    LostMenuItems[2].d.button = i2;
}
#endif

local Menuitem TipsMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, InitTips, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 14, 256-75, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14+22, 256-75+4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 14, 256-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 168, 256-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*0, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*6, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*7, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

local Menuitem ObjectivesMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*7, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*8, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

local Menuitem EndScenarioMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

/**
**	Items for the SelectScen Menu
*/
local unsigned char *ssmtoptions[] = {
    // "All scenarios (cm+pud)",
    "Freecraft scenario (cm)",
    "Foreign scenario (pud)"
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
*/
local Menuitem ScenSelectMenuItems[] = {
    { MI_TYPE_TEXT, 176, 8, 0, LargeFont, ScenSelectInit, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_LISTBOX, 24, 140, 0, GameFont, ScenSelectLBInit, ScenSelectLBExit, NULL, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 312, 140, 0, 0, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 48, 318, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 198, 318, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 132, 40, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 140, 40, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 132, 80, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 140, 80, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 22, 112, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitScenSelectMenuItems() {
    MenuitemText    i0 = { "Select scenario", MI_TFLAGS_CENTERED};

    MenuitemListbox  i1 = { NULL, 288, 6*18, MBUTTON_PULLDOWN, ScenSelectLBAction, 0, 0, 0, 0, 6, 0, 0,
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
#endif

/**
**	Items for the Program Start Menu
*/
local Menuitem PrgStartMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, PrgStartInit, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 7, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitPrgStartMenuItems() {
    MenuitemDrawfunc i0 = { NameLineDrawFunc };
    MenuitemButton   i1 = { "~!Single Player Game", 224, 27, MBUTTON_GM_FULL, SinglePlayerGameMenu, 's'};
    MenuitemButton   i2 = { "~!Multi Player Game", 224, 27, MBUTTON_GM_FULL, MultiPlayerGameMenu, 'm'};
    MenuitemButton   i3 = { "~!Campaign Game", 224, 27, MBUTTON_GM_FULL, CampaignGameMenu, 'c'};
    MenuitemButton   i4 = { "~!Load Game", 224, 27, MBUTTON_GM_FULL, GameMenuLoad, 'l'};
    MenuitemButton   i5 = { "~!Options", 224, 27, MBUTTON_GM_FULL, GlobalOptions, 'o'};
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
#endif

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
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, GameSetupInit, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640/2+12, 192, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36+36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 220, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 640-224-16, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 640-224-16, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640-224-16, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 640-224-16, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

/**
**	Items for the Enter Name Menu
*/
local Menuitem EnterNameMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_INPUT, 40, 38, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 24, 80, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 154, 80, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

/**
**	Items for the Enter Server Menu
*/
local Menuitem EnterServerIPMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_INPUT, 40, 38, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 24, 80, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 154, 80, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

/**
**	Items for the Net Create Join Menu
*/
local Menuitem NetCreateJoinMenuItems[] = {
    { MI_TYPE_BUTTON, 208, 320, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 320 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 320 + 36 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitNetCreateJoinMenuItems() {
    MenuitemButton i0 = { "~!Join Game", 224, 27, MBUTTON_GM_FULL, JoinNetGameMenu, 'j'};
    MenuitemButton i1 = { "~!Create Game", 224, 27, MBUTTON_GM_FULL, CreateNetGameMenu, 'c'};
    MenuitemButton i2 = { "~!Previous Menu", 224, 27, MBUTTON_GM_FULL, EndMenu, 'p'};
    NetCreateJoinMenuItems[0].d.button = i0;
    NetCreateJoinMenuItems[1].d.button = i1;
    NetCreateJoinMenuItems[2].d.button = i2;
}
#endif


/**
**	Items for the Net Multiplayer Setup Menu
*/
local Menuitem NetMultiButtonStorage[] = {
    { MI_TYPE_PULLDOWN, 40, 32, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_DRAWFUNC, 40, 32, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
};
//#ifdef OLD_MENU
local void InitNetMultiButtonStorage() {
    MenuitemPulldown i0 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, MultiGamePTSAction, 3, -1, 0, 0, 0};
    MenuitemDrawfunc i1 = { NetMultiPlayerDrawFunc };
    NetMultiButtonStorage[0].d.pulldown = i0;
    NetMultiButtonStorage[1].d.drawfunc = i1;
}
//#endif

/**
**	Multi player custom game menu (server side).
*/
local Menuitem NetMultiSetupMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, MultiGameSetupInit, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640/2+12, 8, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36+36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    // 8+7 player slots (content here is overwritten!)
#define SERVER_PLAYER_STATE	5
    { MI_TYPE_PULLDOWN, 40, 32+22*0, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*6, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*7, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*0, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*6, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 40, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640-224-16, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 640-224-16, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    // 7+7 player ready buttons
#define SERVER_PLAYER_READY	30
    { MI_TYPE_GEM, 10, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*7, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 330, 32+22*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    // FIXME: Slot 15 is reserved for neutral computer
    //{ MI_TYPE_GEM, 330, 32+22*7, 0, LargeFont, NULL, NULL, NULL,
    //	{ gem:{ MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL,0} } },

    // 7+7 player lag buttons
#define SERVER_PLAYER_LAG	44
    { MI_TYPE_GEM, 218, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*7, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 538, 32+22*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    // FIXME: Slot 15 is reserved for neutral computer
    //{ MI_TYPE_GEM, 538, 32+22*7, 0, LargeFont, NULL, NULL, NULL,
    //	{ gem:{ MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL,0} } },
};
#ifdef OLD_MENU
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
#endif

/**
**	Multi player client game menu.
*/
local Menuitem NetMultiClientMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, MultiGameClientInit, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 640/2+12, 8, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 640-224-16, 360, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 640-224-16, 360+36+36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    // 8+7 player slots
#define CLIENT_PLAYER_STATE	5
    { MI_TYPE_PULLDOWN, 40, 32, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*6, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*7, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*6, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 40, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
#define CLIENT_RACE	21
    { MI_TYPE_PULLDOWN, 40, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
#define CLIENT_RESOURCE	23
    { MI_TYPE_PULLDOWN, 220, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640-224-16, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
#define CLIENT_UNITS	25
    { MI_TYPE_PULLDOWN, 640-224-16, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
#define CLIENT_FOG_OF_WAR	27
    { MI_TYPE_PULLDOWN, 40, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
#define CLIENT_TILESET	29
    { MI_TYPE_PULLDOWN, 220, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    // 7+7 player state buttons
#define CLIENT_PLAYER_READY	30
    { MI_TYPE_GEM, 10, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*7, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    //{ MI_TYPE_GEM, 330, 32+22*7, 0, LargeFont, NULL, NULL, NULL,
    //	{ gem:{ 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction} } },
};
#ifdef OLD_MENU
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
#endif

local Menuitem NetErrorMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 38, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 92, 80, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitNetErrorMenuItems() {
    MenuitemText   i0 = { "Error:", MI_TFLAGS_CENTERED};
    MenuitemText   i1 = { NULL, MI_TFLAGS_CENTERED};
    MenuitemButton i2 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};
    NetErrorMenuItems[0].d.text   = i0;
    NetErrorMenuItems[1].d.text   = i1;
    NetErrorMenuItems[2].d.button = i2;
}
#endif

/**
**	Items for the Connecting Network Menu
*/
local Menuitem ConnectingMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 53, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 90, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

/**
**	Items for the Campaign Select Menu
*/
local Menuitem CampaignSelectMenuItems[] = {
    { MI_TYPE_BUTTON, 208, 212 + 36 * 0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 4, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

/**
**	Items for the Campaign Continue Menu
*/
local Menuitem CampaignContMenuItems[] = {
    { MI_TYPE_BUTTON, 508, 320 + 36 + 36 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitCampaignContMenuItems() {
    MenuitemButton i0 = { "~!Continue", 106, 27, MBUTTON_GM_HALF, EndMenu, 'c'};
    CampaignContMenuItems[0].d.button = i0;
}
#endif

local Menuitem SoundOptionsMenuItems[] = {
    { MI_TYPE_TEXT, 176, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 16, 36*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*1.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 44, 36*2 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 218, 36*2 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 240, 36*1.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 266, 36*1.5 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 16, 36*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*3.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 44, 36*4 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 218, 36*4 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 240, 36*3.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 266, 36*3.5 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 16, 36*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*5.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 44, 36*6 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 218, 36*6 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 240, 36*5.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 266, 36*5.5 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 32, 36*6.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 58, 36*6.5 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 154, 36*6.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 180, 36*6.5 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 176 - (106 / 2), 352 - 11 - 27, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

local Menuitem PreferencesMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 16, 36*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 46, 36*1 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 16, 36*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 46, 36*2 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 128 - (106 / 2), 245, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

local Menuitem SpeedSettingsMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 16, 36*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*1.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 34, 36*2 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 230, 36*2 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 16, 36*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*3.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 34, 36*4 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 230, 36*4 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 16, 36*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*5.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 34, 36*6 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 230, 36*6 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 128 - (106 / 2), 245, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

local Menuitem GameOptionsMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#ifdef WITH_SOUND
    { MI_TYPE_BUTTON, 16, 40 + 36*0, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#else
    { MI_TYPE_BUTTON, 16, 40 + 36*0, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#endif
    { MI_TYPE_BUTTON, 16, 40 + 36*1, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*2, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 128 - (224 / 2), 245, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitGameOptionsMenuItems() {
    MenuitemText   i0 = { "Game Options", MI_TFLAGS_CENTERED};
    MenuitemButton i1 = { "Sound (~!F~!7)", 224, 27, MBUTTON_GM_FULL, SoundOptions, KeyCodeF7};
    MenuitemButton i2 = { "Speeds (~!F~!8)", 224, 27, MBUTTON_GM_FULL, SpeedSettings, KeyCodeF8};
    MenuitemButton i3 = { "Preferences (~!F~!9)", 224, 27, MBUTTON_GM_FULL, Preferences, KeyCodeF9};
    MenuitemButton i4 = { "Previous (~!E~!s~!c)", 224, 27, MBUTTON_GM_FULL, EndMenu, '\033'};
    GameOptionsMenuItems[0].d.text   = i0;
    GameOptionsMenuItems[1].d.button = i1;
    GameOptionsMenuItems[2].d.button = i2;
    GameOptionsMenuItems[3].d.button = i3;
    GameOptionsMenuItems[4].d.button = i4;
}
#endif

local Menuitem HelpMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*0, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*1, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 128 - (224 / 2), 288-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

local Menuitem KeystrokeHelpMenuItems[] = {
    { MI_TYPE_TEXT, 352/2, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 352 - 18 - 16, 40+20, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 352/2 - (224 / 2), 352-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_DRAWFUNC, 16, 40+20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
};
// FIXME: ccl these...
// FIXME: add newer helps
local unsigned char *keystrokekeystexts[] = {
    "Alt-F",
    "Alt-G",
    "Ctrl-S",
    "Ctrl-M",
    "+",
    "-",
    "Ctrl-P",
    "PAUSE",
    "PRINT",
    "Alt-H",
    "Alt-R",
    "Alt-Q",
    "Alt-X",
    "Alt-B",
    "Alt-M",
    "ENTER",
    "SPACE",
    "TAB",
    "Alt-I",
    "Alt-C",
    "Alt-V",
    "Ctrl-V",
    "^",
    "#",
    "##",
    "Ctrl-#",
    "Shift-#",
    "Alt-#",
    "F2-F4",
    "Shift F2-F4",
    "F5",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
};

local unsigned char *keystrokehintstexts[] = {
    "- toggle full screen",
    "- toggle grab mouse",
    "- mute sound",
    "- mute music (NOT SUPPORTED)",
    "- increase game speed",
    "- decrease game speed",
    "- pause game",
    "- pause game",
    "- make screen shot",
    "- help menu",
    "- restart scenario (NOT SUPPORTED)",
    "- quit to main menu",
    "- quit game",
    "- toggle expand map",
    "- game menu",
    "- write a message",
    "- goto last event",
    "- hide/unhide terrain",
    "- find idle peon",
    "- center on selected unit",
    "- next view port",
    "- previous view port",
    "- select nothing",
    "- select group",
    "- center on group",
    "- define group",
    "- add to group",
    "- add to alternate group",
    "- recall map position",
    "- save map postition",
    "- game options",
    "- sound options",
    "- speed options",
    "- preferences",
    "- game menu",
    "- save game",
    "- load game",

};

#ifdef OLD_MENU
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
#endif

local Menuitem SaveGameMenuItems[] = {
    { MI_TYPE_TEXT, 384/2, 11, 0, LargeFont, InitSaveGameMenu, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_INPUT, 16, 11+36*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_LISTBOX, 16, 11+36*1.5, 0, GameFont, SaveLBInit, SaveLBExit, NULL, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 384-16-16, 11+36*1.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 2*384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 3*384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitSaveGameMenuItems() {
    MenuitemText    i0 = { "Save Game", MI_TFLAGS_CENTERED};
    MenuitemInput   i1 = { NULL, 384-16-16, 16, MBUTTON_PULLDOWN, EnterSaveGameAction, 0, 0};
    MenuitemListbox i2 = { NULL, 384-16-16-16, 7*18, MBUTTON_PULLDOWN, SaveLBAction, 0, 0, 0, 0, 7, 0, 0,
			   (void *)SaveLBRetrieve, ScenSelectOk};
    MenuitemVslider i3 = { 0, 18, 7*18, SaveVSAction, -1, 0, 0, 0, SaveOk};
    MenuitemButton  i4 = { "Save (~<F11~>)", 106, 27, MBUTTON_GM_HALF, SaveAction, KeyCodeF11};
    MenuitemButton  i5 = { "Delete (~<F5~>)", 106, 27, MBUTTON_GM_HALF, FcDeleteMenu, KeyCodeF5};
    MenuitemButton  i6 = { "Cancel (~<Esc~>)", 106, 27, MBUTTON_GM_HALF, EndMenu, '\033'};
    SaveGameMenuItems[0].d.text    = i0;
    SaveGameMenuItems[1].d.input   = i1;
    SaveGameMenuItems[2].d.listbox = i2;
    SaveGameMenuItems[3].d.vslider = i3;
    SaveGameMenuItems[4].d.button  = i4;
    SaveGameMenuItems[5].d.button  = i5;
    SaveGameMenuItems[6].d.button  = i6;
}
#endif

local Menuitem LoadGameMenuItems[] = {
    { MI_TYPE_TEXT, 384/2, 11, 0, LargeFont, InitLoadGameMenu, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_LISTBOX, 16, 11+36*1.5, 0, GameFont, LoadLBInit, LoadLBExit, NULL, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 384-16-16, 11+36*1.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 3*384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitLoadGameMenuItems() {
    MenuitemText    i0 = { "Load Game", MI_TFLAGS_CENTERED};
    MenuitemListbox i1 = { NULL, 384-16-16-16, 7*18, MBUTTON_PULLDOWN, LoadLBAction, 0, 0, 0, 0, 7, 0, 0,
			   (void *)LoadLBRetrieve, ScenSelectOk};
    MenuitemVslider i2 = { 0, 18, 7*18, LoadVSAction, -1, 0, 0, 0, LoadOk};
    MenuitemButton  i3 = { "Load (~<F12~>)", 106, 27, MBUTTON_GM_HALF, LoadAction, KeyCodeF12};
    MenuitemButton  i4 = { "~!Cancel (~<Esc~>)", 106, 27, MBUTTON_GM_HALF, EndMenu, '\033'};
    LoadGameMenuItems[0].d.text    = i0;
    LoadGameMenuItems[1].d.listbox = i1;
    LoadGameMenuItems[2].d.vslider = i2;
    LoadGameMenuItems[3].d.button  = i3;
    LoadGameMenuItems[4].d.button  = i4;
}
#endif

local Menuitem ConfirmSaveMenuItems[] = {
    { MI_TYPE_TEXT, 288/2, 11, 0, LargeFont, ConfirmSaveInit, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*1.5, 0, GameFont, NULL, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*2.5, 0, GameFont, NULL, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_BUTTON, 16, 128-27*1.5, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 288-16-106, 128-27*1.5, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

local Menuitem ConfirmDeleteMenuItems[] = {
    { MI_TYPE_TEXT, 288/2, 11, 0, LargeFont, FcDeleteInit, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*1.5, 0, GameFont, NULL, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*2.5, 0, GameFont, NULL, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_BUTTON, 16, 128-27*1.5, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 288-16-106, 128-27*1.5, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
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
#endif

local Menuitem EditorSelectMenuItems[] = {
    { MI_TYPE_BUTTON, 208, 320 + 36 * 0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 320 + 36 * 1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 320 + 36 * 2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitEditorSelectMenuItems() {
    MenuitemButton i0 = { "~!New Map", 224, 27, MBUTTON_GM_FULL, EditorNewMap, 'n'};
    MenuitemButton i1 = { "~!Load Map", 224, 27, MBUTTON_GM_FULL, EditorLoadMap, 'l'};
    MenuitemButton i2 = { "~!Cancel", 224, 27, MBUTTON_GM_FULL, EndMenu, 'c'};
    EditorSelectMenuItems[0].d.button = i0;
    EditorSelectMenuItems[1].d.button = i1;
    EditorSelectMenuItems[2].d.button = i2;
}
#endif

local Menuitem EditorLoadMapMenuItems[] = {
    { MI_TYPE_TEXT, 352/2, 11, 0, LargeFont, EditorLoadInit, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_LISTBOX, (352-18-288)/2, 11+98, 0, GameFont, EditorLoadLBInit, EditorLoadLBExit, NULL, {{NULL,0}} },
    { MI_TYPE_VSLIDER, (352-18-288)/2+288, 11+98, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 48, 308, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 198, 308, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, (352-18-288)/2-2, 11+98-28, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

};
#ifdef OLD_MENU
local void InitEditorLoadMapMenuItems() {
    MenuitemText    i0 = { "Select map", MI_TFLAGS_CENTERED};

    MenuitemListbox  i1 = { NULL, 288, 6*18, MBUTTON_PULLDOWN, EditorLoadLBAction, 0, 0, 0, 0, 6, 0, 0,
			    (void *)EditorLoadLBRetrieve, EditorLoadOk};
    MenuitemVslider  i2 = { 0, 18, 6*18, EditorLoadVSAction, -1, 0, 0, 0, EditorLoadOk};

    MenuitemButton   i3 = { "OK", 106, 27, MBUTTON_GM_HALF, EditorLoadOk, 0};
    MenuitemButton   i4 = { "Cancel", 106, 27, MBUTTON_GM_HALF, EditorLoadCancel, 0};
    MenuitemButton   i5 = { NULL, 36, 24, MBUTTON_FOLDER, EditorLoadFolder, 0};
    EditorLoadMapMenuItems[0].d.text     = i0;
    EditorLoadMapMenuItems[1].d.listbox  = i1;
    EditorLoadMapMenuItems[2].d.vslider  = i2;
    EditorLoadMapMenuItems[3].d.button   = i3;
    EditorLoadMapMenuItems[4].d.button   = i4;
    EditorLoadMapMenuItems[5].d.button   = i5;
}
#endif

/**
**	Items for the main Editor Menu
*/
local Menuitem EditorMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, InitGameMenu, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16 + 12 + 106, 40, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36 + 36, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40-36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitEditorMenuItems() {
    MenuitemText   i0 = { "Editor Menu", MI_TFLAGS_CENTERED};
    MenuitemButton i1 = { "Save (~<F11~>)", 106, 27, MBUTTON_GM_HALF, GameMenuSave, KeyCodeF11};
    MenuitemButton i2 = { "Load (~<F12~>)", 106, 27, MBUTTON_GM_HALF, GameMenuLoad, KeyCodeF12};
    MenuitemButton i3 = { "Map Properties (~<F5~>)", 224, 27, MBUTTON_GM_FULL, EditorMapProperties, KeyCodeF5};
    MenuitemButton i4 = { "Player Properties (~<F6~>)", 224, 27, MBUTTON_GM_FULL, EditorPlayerProperties, KeyCodeF6};
    MenuitemButton i5 = { "", 224, 27, MBUTTON_GM_FULL, GameMenuObjectives, 'o'};
    MenuitemButton i6 = { "E~!xit to Menu", 224, 27, MBUTTON_GM_FULL, EditorQuitMenu, 'x'};
    MenuitemButton i7 = { "Return to Editor (~<Esc~>)", 224, 27, MBUTTON_GM_FULL, GameMenuReturn, '\033'};
    EditorMenuItems[0].d.text   = i0;
    EditorMenuItems[1].d.button = i1;
    EditorMenuItems[2].d.button = i2;
    EditorMenuItems[3].d.button = i3;
    EditorMenuItems[4].d.button = i4;
    EditorMenuItems[5].d.button = i5;
    EditorMenuItems[6].d.button = i6;
    EditorMenuItems[7].d.button = i7;
}
#endif

local unsigned char *mptssoptions[] = {
    "Forest",
    "Winter",
    "Wasteland",
    "Orc Swamp",
};

local unsigned char *veroptions[] = {
    "Original",
    "Expansion",
    "FreeCraft",
};

local Menuitem EditorMapPropertiesMenuItems[] = {
    { MI_TYPE_TEXT, 288/2, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, (288-260)/2, 11+36, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_INPUT, (288-260)/2, 11+36+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, (288-260)/2, 11+36*2+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 288-(288-260)/2-152, 11+36*2+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, (288-260)/2, 11+36*3+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 288-(288-260)/2-152, 11+36*3+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, (288-260)/2, 11+36*4+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 288-(288-260)/2-152, 11+36*4+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, (288-106*2)/4, 256 - 11 - 27, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 288-(288-106*2)/4 - 106, 256 - 11 - 27, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitEditorMapPropertiesMenuItems() {
    MenuitemText     i0  = { "Map Properties", MI_TFLAGS_CENTERED};
    MenuitemText     i1  = { "Map Description:", MI_TFLAGS_LALIGN};
    MenuitemInput    i2  = { NULL, 260, 16, MBUTTON_PULLDOWN, EditorEnterMapDescriptionAction, 0, 0};
    MenuitemText     i3  = { "Size:", MI_TFLAGS_LALIGN};
    MenuitemText     i4  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText     i5  = { "Tileset:", MI_TFLAGS_LALIGN};
    MenuitemPulldown i6  = { mptssoptions, 152, 20, MBUTTON_PULLDOWN, NULL, 4, 0, 0, 0, 0};
    MenuitemText     i7  = { "Version:", MI_TFLAGS_LALIGN};
    MenuitemPulldown i8  = { veroptions, 152, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemButton   i9  = { "OK", 106, 27, MBUTTON_GM_HALF, EditorMapPropertiesOk, 0};
    MenuitemButton   i10 = { "Cancel (~<Esc~>)", 106, 27, MBUTTON_GM_HALF, EditorEndMenu, '\033'};
    EditorMapPropertiesMenuItems[0].d.text     = i0;
    EditorMapPropertiesMenuItems[1].d.text     = i1;
    EditorMapPropertiesMenuItems[2].d.input    = i2;
    EditorMapPropertiesMenuItems[3].d.text     = i3;
    EditorMapPropertiesMenuItems[4].d.text     = i4;
    EditorMapPropertiesMenuItems[5].d.text     = i5;
    EditorMapPropertiesMenuItems[6].d.pulldown = i6;
    EditorMapPropertiesMenuItems[7].d.text     = i7;
    EditorMapPropertiesMenuItems[8].d.pulldown = i8;
    EditorMapPropertiesMenuItems[9].d.button   = i9;
    EditorMapPropertiesMenuItems[10].d.button  = i10;
}
#endif

local Menuitem EditorPlayerPropertiesMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
#ifdef OLD_MENU
local void InitEditorPlayerPropertiesMenuItems() {
    MenuitemText   i0 = { "Player Properties", MI_TFLAGS_CENTERED};
    EditorPlayerPropertiesMenuItems[0].d.text   = i0;
}
#endif


#ifdef OLD_MENU
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
    {
	// Editor Select Menu
	0,
	0,
	640, 480,
	ImageNone,
	0, 3,
	EditorSelectMenuItems,
	NULL,
    },
    {
	// Editor Load Map Menu
	(640-352)/2,
	(480-352)/2,
	352, 352,
	ImagePanel5,
	4, 6,
	EditorLoadMapMenuItems,
	NULL,
    },
    {
	// Editor Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	7, 8,
	EditorMenuItems,
	NULL,
    },
    {
	// Editor Map Properties Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-256)/2,
	288, 256,
	ImagePanel2,
	10, 11,
	EditorMapPropertiesMenuItems,
	NULL,
    },
    {
	// Editor Player Properties Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	0, 1,
	EditorPlayerPropertiesMenuItems,
	NULL,
    },
};
#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

#ifdef SAVE_MENU_CCL
#define HASHADD(x,y) { \
    *(void **)hash_add(MenuFuncHash,(y)) = (void *)(x); \
    sprintf(buf,"%p",(x)); \
    strcpy((char*)hash_add(MenuFuncHash2,buf), (y)); \
}
#else
#define HASHADD(x,y) { \
    *(void **)hash_add(MenuFuncHash,(y)) = (void *)(x); \
}
#endif

global void InitMenuFuncHash(void) {
#ifdef SAVE_MENU_CCL
    char buf[10];
#endif

    HASHADD(NULL,"null");

// Game menu
    HASHADD(InitGameMenu,"init-game-menu");
    HASHADD(GameMenuSave,"game-menu-save");
    HASHADD(GameMenuLoad,"game-menu-load");
    HASHADD(GameOptions,"game-options");
    HASHADD(HelpMenu,"menu-help");
    HASHADD(GameMenuObjectives,"game-menu-objectives");
    HASHADD(GameMenuEndScenario,"game-menu-end-scenario");
    HASHADD(GameMenuReturn,"game-menu-return");
    HASHADD(EndMenu,"end-menu");

// Victory, lost
    HASHADD(GameMenuEnd,"game-menu-end");

// Scene select
    HASHADD(ScenSelectMenu,"scen-select-menu");
    HASHADD(ScenSelectLBExit,"scen-select-lb-exit");
    HASHADD(ScenSelectLBInit,"scen-select-lb-init");
    HASHADD(ScenSelectLBRetrieve,"scen-select-lb-retrieve");
    HASHADD(ScenSelectLBAction,"scen-select-lb-action");
    HASHADD(ScenSelectTPMSAction,"scen-select-tpms-action");
    HASHADD(ScenSelectVSAction,"scen-select-vs-action");
    HASHADD(ScenSelectFolder,"scen-select-folder");
    HASHADD(ScenSelectInit,"scen-select-init");
    HASHADD(ScenSelectOk,"scen-select-ok");
    HASHADD(ScenSelectCancel,"scen-select-cancel");
    HASHADD(ScenSelectRDFilter,"scen-select-rd-filter");

// Program start
    HASHADD(PrgStartInit,"program-start");
    HASHADD(NameLineDrawFunc,"name-line-draw");
    HASHADD(SinglePlayerGameMenu,"single-player-game-menu");
    HASHADD(MultiPlayerGameMenu,"multi-player-game-menu");
    HASHADD(CampaignGameMenu,"campaign-game-menu");
    HASHADD(StartEditor,"game-start-editor");
    HASHADD(GameShowCredits,"game-show-credits");
    HASHADD(GameMenuExit,"game-menu-exit");

// Global Options
    HASHADD(GlobalOptions,"game-global-options");
    HASHADD(InitGlobalOptions,"init-global options");
    HASHADD(SetRes640,"set-res-640");
    HASHADD(SetRes800,"set-res-800");
    HASHADD(SetRes1024,"set-res-1024");
    HASHADD(SetRes1280,"set-res-1280");
    HASHADD(SetRes1600,"set-res-1600");
    HASHADD(SetFullscreen,"set-fullscreen");
    HASHADD(SetShadowFogAlpha,"set-shadow-fog-alpha");
    HASHADD(SetShadowFogGray,"set-shadow-fog-gray");

// Tips
    HASHADD(InitTips,"init-tips");
    HASHADD(SetTips,"set-tips");
    HASHADD(ShowNextTip,"show-next-tip");
    HASHADD(TipsMenuEnd,"tips-menu-end");

// Custom game setup
    HASHADD(GameSetupInit,"game-setup-init");
    HASHADD(GameDrawFunc,"game-draw-func");
    HASHADD(GameRCSAction,"game-rcs-action");
    HASHADD(GameRESAction,"game-res-action");
    HASHADD(GameUNSAction,"game-uns-action");
    HASHADD(GameTSSAction,"game-tss-action");
    HASHADD(GameGATAction,"game-gat-action");
    HASHADD(GameCancel,"game-cancel");
    HASHADD(CustomGameStart,"custom-game-start");
    HASHADD(CustomGameOPSAction,"custom-game-ops-action");

// Enter name
    HASHADD(EnterNameAction,"enter-name-action");
    HASHADD(EnterNameCancel,"enter-name-cancel");

// Net create join
    HASHADD(JoinNetGameMenu,"net-join-game");
    HASHADD(CreateNetGameMenu,"net-create-game");

// Net multi setup
    HASHADD(MultiGameSetupInit,"multi-game-setup-init");
    HASHADD(MultiGameDrawFunc,"multi-game-draw-func");
    HASHADD(MultiScenSelectMenu,"multi-scen-select");
    HASHADD(MultiGameStart,"multi-game-start");
    HASHADD(MultiGameCancel,"multi-game-cancel");
    HASHADD(MultiGameFWSAction,"multi-game-fws-action");

// Enter server ip
    HASHADD(EnterServerIPAction,"enter-server-ip-action");
    HASHADD(EnterServerIPCancel,"enter-server-ip-cancel");

// Net multi client
    HASHADD(TerminateNetConnect,"terminate-net-connect");
    HASHADD(MultiGameClientInit,"multi-game-client-init");
    HASHADD(MultiGameClientDrawFunc,"multi-client-draw-func");
    HASHADD(MultiClientReady,"multi-client-ready");
    HASHADD(MultiClientNotReady,"multi-client-not-ready");
    HASHADD(MultiClientCancel,"multi-client-cancel");
    HASHADD(MultiClientRCSAction,"multi-client-rcs-action");
    HASHADD(MultiClientGemAction,"multi-client-gem-action");

// Net connecting
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
    HASHADD(EndScenarioQuitMenu,"end-scenario-quit-menu");

// Sound options
    HASHADD(MasterVolumeHSAction,"master-volume-hs-action");
    HASHADD(SetMasterPower,"set-master-power");
    HASHADD(MusicVolumeHSAction,"music-volume-hs-action");
    HASHADD(SetMusicPower,"set-music-power");
    HASHADD(CdVolumeHSAction,"cd-volume-hs-action");
    HASHADD(SetCdPower,"set-cd-power");
    HASHADD(SetCdModeAll,"set-cd-mode-all");
    HASHADD(SetCdModeRandom,"set-cd-mode-random");

// Preferences
    HASHADD(SetFogOfWar,"set-fog-of-war");
    HASHADD(SetCommandKey,"set-command-key");

// Speed settings
    HASHADD(GameSpeedHSAction,"game-speed-hs-action");
    HASHADD(MouseScrollHSAction,"mouse-scroll-hs-action");
    HASHADD(KeyboardScrollHSAction,"keyboard-scroll-hs-action");

// Game options
    HASHADD(SoundOptions,"sound-options");
    HASHADD(SpeedSettings,"speed-settings");
    HASHADD(Preferences,"preferences");

// Help
    HASHADD(KeystrokeHelpMenu,"keystroke-help");
    HASHADD(ShowTipsMenu,"show-tips");

// Keystroke help
    HASHADD(KeystrokeHelpVSAction,"keystroke-help-vs-action");
    HASHADD(KeystrokeHelpDrawFunc,"keystroke-help-draw-func");

// Save
    HASHADD(InitSaveGameMenu,"init-save-game-menu");
    HASHADD(SaveLBInit,"save-lb-init");
    HASHADD(SaveLBExit,"save-lb-exit");
    HASHADD(EnterSaveGameAction,"enter-save-game-action");
    HASHADD(SaveLBAction,"save-lb-action");
    HASHADD(SaveLBRetrieve,"save-lb-retrieve");
    HASHADD(SaveVSAction,"save-vs-action");
    HASHADD(SaveOk,"save-ok");
    HASHADD(SaveAction,"save-action");
    HASHADD(FcDeleteMenu,"fc-delete-menu");

// Load
    HASHADD(InitLoadGameMenu,"init-load-game-menu");
    HASHADD(LoadLBInit,"load-lb-init");
    HASHADD(LoadLBExit,"load-lb-exit");
    HASHADD(LoadLBAction,"load-lb-action");
    HASHADD(LoadLBRetrieve,"load-lb-retrieve");
    HASHADD(LoadVSAction,"load-vs-action");
    HASHADD(LoadOk,"load-ok");
    HASHADD(LoadAction,"load-action");

// Confirm save
    HASHADD(ConfirmSaveInit,"confirm-save-init");
    HASHADD(ConfirmSaveFile,"confirm-save-file");

// Confirm delete
    HASHADD(FcDeleteInit,"fc-delete-init");
    HASHADD(FcDeleteFile,"fc-delete-file");

// Editor select
    HASHADD(EditorNewMap,"editor-new-map");
    HASHADD(EditorLoadMap,"editor-load-map");

// Editor load map
    HASHADD(EditorLoadInit,"editor-load-init");
    HASHADD(EditorLoadLBInit,"editor-load-lb-init");
    HASHADD(EditorLoadLBExit,"editor-load-lb-exit");
    HASHADD(EditorLoadLBAction,"editor-load-lb-action");
    HASHADD(EditorLoadLBRetrieve,"editor-load-lb-retrieve");
    HASHADD(EditorLoadVSAction,"editor-load-vs-action");
    HASHADD(EditorLoadOk,"editor-load-ok");
    HASHADD(EditorLoadCancel,"editor-load-cancel");
    HASHADD(EditorLoadFolder,"editor-load-folder");
    HASHADD(EditorMapProperties,"menu-editor-map-properties");
    HASHADD(EditorEnterMapDescriptionAction,"editor-enter-map-description-action");
    HASHADD(EditorPlayerProperties,"menu-editor-player-properties");

// Editor menu
    HASHADD(EditorQuitMenu,"editor-quit-menu");

// Editor map properties
    HASHADD(EditorEnterMapDescriptionAction,"editor-map-description-action");
    HASHADD(EditorMapPropertiesOk,"editor-map-properties-ok");
    HASHADD(EditorEndMenu,"editor-end-menu");
}

/*----------------------------------------------------------------------------
--	Button action handler and Init/Exit functions
----------------------------------------------------------------------------*/

/**
**	Set start menu backgound and draw it.
*/
local void StartMenusSetBackground(Menuitem *mi __attribute__((unused)))
{
    MenusSetBackground();
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

local void InitSaveGameMenu(Menuitem *mi)
{
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-save-game");
#else
    menu = mi->menu;
#endif
    menu->items[4].flags = MenuButtonDisabled;    
    menu->items[5].flags = MenuButtonDisabled;
    CreateSaveDir();
}

/**
**	FIXME: docu.
*/
local void EnterSaveGameAction(Menuitem *mi, int key)
{
#ifdef OLD_MENU
    Menu *menu;
#endif

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
#ifdef OLD_MENU
    menu = FindMenu("menu-save-game");
    menu->items[5].flags = MenuButtonDisabled;
#else
    mi->menu->items[5].flags = MenuButtonDisabled;
#endif
}

local void SaveAction(void)
{
    char filename[PATH_MAX];
    char *name;
    Menu *menu;
    size_t nameLength;
  
    menu = FindMenu("menu-save-game");
    name = menu->items[1].d.input.buffer;

    nameLength = strlen(name);
    if (TypedFileName) {
	nameLength -= 3;
    }

    strcpy(filename, SaveDir);
    strcat(filename, "/");
    strncat(filename, name, nameLength);
    if (strstr(filename, ".sav") == NULL)
	strcat(filename, ".sav");

    if (access(filename,F_OK)) {
        SaveGame(filename);
	SetMessage("Saved game to: %s", filename);
    } else {
	ProcessMenu("menu-save-confirm", 1);
    }

    EndMenu();
}

local void CreateSaveDir(void)
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
    Menu *menu;

    menu = FindMenu("menu-save-game");
    strcpy(savegame_buffer, "~!_");
    menu->items[1].d.input.buffer = savegame_buffer;
    menu->items[1].d.input.nch = 0; /* strlen(savegame_buffer) - 3; */
    menu->items[1].d.input.maxch = 24;
    menu->items[4].flags = MenuButtonDisabled;	/* Save button! */
    ProcessMenu("menu-save-game", 1);
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
#ifdef OLD_MENU
    Menu *menu;

    menu = FindMenu("menu-save-game");
#endif
    SaveLBExit(mi);

    i = mi->d.listbox.noptions = ReadDataDirectory(SaveDir,
	    NULL, (FileList **) & (mi->d.listbox.options));
    if (i != 0) {
#ifdef OLD_MENU
	if (i > 7) {
	    menu->items[3].flags = MI_ENABLED;
	} else {
	    menu->items[3].flags = MI_DISABLED;
	}
#else
	if (i > 7) {
	    mi->menu->items[3].flags = MI_ENABLED;
	} else {
	    mi->menu->items[3].flags = MI_DISABLED;
	}
#endif
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
#ifdef OLD_MENU
		    menu = FindMenu("menu-select-scenario");
#else
		    menu = mi->menu;
#endif
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
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-save-game");
#else
    menu = mi->menu;
#endif
    DebugCheck(i<0);
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (mi->d.listbox.noptions > 7) {
	    mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	    mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	}
	if (fl[i].type) {
	    strcpy(menu->items[1].d.input.buffer, fl[i].name);
	    menu->items[1].d.input.nch = strlen(fl[i].name);
	    menu->items[4].flags = MI_ENABLED;
	    menu->items[5].flags = MI_ENABLED;
	} else {
	    menu->items[4].flags = MenuButtonDisabled;
	    menu->items[5].flags = MenuButtonDisabled;
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
    Menuitem *mi;
    Menu *menu;
    int i;

    menu = FindMenu("menu-select-scenario");
    mi = &menu->items[1];
    i = mi->d.listbox.curopt + mi->d.listbox.startline;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    strcat(ScenSelectPath, "/");
	    strcat(ScenSelectPath, fl[i].name);
	    if (menu->items[9].flags&MenuButtonDisabled) {
		menu->items[9].flags &= ~MenuButtonDisabled;
		menu->items[9].d.button.text = ScenSelectDisplayPath;
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

local void InitLoadGameMenu(Menuitem *mi)
{
#ifdef OLD_MENU
    Menu *menu;

    menu = FindMenu("menu-load-game");
    menu->items[3].flags = MI_DISABLED;
#else
    mi->menu->items[3].flags = MI_DISABLED;
#endif
    CreateSaveDir();
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
    Menu *menu;
    int i;

#ifdef OLD_MENU
    menu = FindMenu("menu-load-game");
#else
    menu = mi->menu;
#endif
    LoadLBExit(mi);

    i = mi->d.listbox.noptions = ReadDataDirectory(SaveDir, SaveRDFilter,
						     (FileList **)&(mi->d.listbox.options));
    if (i != 0) {
	if (i > 7) {
	    menu->items[2].flags = MenuButtonSelected;
	} else {
	    menu->items[2].flags = MenuButtonDisabled;
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
#ifdef OLD_MENU
		    menu = FindMenu("menu-select-scenario");
#else
		    menu = mi->menu;
#endif
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
#ifdef OLD_MENU
    Menu *menu;
#endif
    FileList *fl;

#ifdef OLD_MENU
    menu = FindMenu("menu-load-game");
#endif
    DebugCheck(i<0);
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (mi->d.listbox.noptions > 7) {
	    mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	    mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	}
#ifdef OLD_MENU
	if (fl[i].type) {
	    menu->items[3].flags = MI_ENABLED;
	} else {
	    menu->items[3].flags = MI_DISABLED;
	}
#else
	if (fl[i].type) {
	    mi->menu->items[3].flags = MI_ENABLED;
	} else {
	    mi->menu->items[3].flags = MI_DISABLED;
	}
#endif
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
    Menuitem *mi;
    Menu *menu;
    int i;

    menu = FindMenu("menu-select-scenario");
    mi = &menu->items[1];
    i = mi->d.listbox.curopt + mi->d.listbox.startline;
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    strcat(ScenSelectPath, "/");
	    strcat(ScenSelectPath, fl[i].name);
	    if (menu->items[9].flags&MenuButtonDisabled) {
		menu->items[9].flags &= ~MenuButtonDisabled;
		menu->items[9].d.button.text = ScenSelectDisplayPath;
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
    ProcessMenu("menu-save-confirm", 1);
}
#endif

local void ConfirmSaveInit(Menuitem * mi __attribute__ ((unused)))
{
    static char name[PATH_MAX];		// FIXME: much memory wasted
    int fileLength;
    Menu *menu;

    menu = FindMenu("menu-save-game");
    fileLength = strlen(menu->items[1].d.input.buffer);
    if (TypedFileName) {
	fileLength -= 3;
    }

    strcpy(name, "the file: ");
    strncat(name, menu->items[1].d.input.buffer, fileLength);
    if (strstr(name, ".sav") == NULL) {
	strcat(name, ".sav");
    }
    menu = FindMenu("menu-save-confirm");
    menu->items[2].d.text.text = name;
}

local void ConfirmSaveFile(void)
{
    char name[PATH_MAX];
    int fileLength;
    Menu *menu;

    menu = FindMenu("menu-save-game");
    fileLength = strlen(menu->items[1].d.input.buffer);
    if (TypedFileName) {
	fileLength -= 3;
    }

    strcpy(name, SaveDir);
    strcat(name, "/");
    strncat(name, menu->items[1].d.input.buffer, fileLength);
    if (strstr(name, ".sav") == NULL) {
	strcat(name, ".sav");
    }
    SaveGame(name);
    SetMessage("Saved game to: %s", name);
    EndMenu();
}

local void FcDeleteMenu(void)
{
    EndMenu();
    ProcessMenu("menu-delete-confirm", 1);
}

local void FcDeleteInit(Menuitem *mi __attribute__((unused)))
{
    static char name[PATH_MAX];		// FIXME: much memory wasted
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-save-game");
#else
    menu = mi->menu;
#endif
    strcpy(name, "the file: ");
    strcat(name, menu->items[1].d.input.buffer);
    menu = FindMenu("menu-delete-confirm");
    menu->items[2].d.text.text = name;
}
  
local void FcDeleteFile(void)
{
    Menu *menu;
    char name[PATH_MAX];

    menu = FindMenu("menu-save-game");
    strcpy(name, SaveDir);
    strcat(name, "/");
    strcat(name, menu->items[1].d.input.buffer);
    unlink(name);
    EndMenu();
    *menu->items[1].d.input.buffer = '\0';
    menu->items[1].d.input.nch = 0;
    ProcessMenu("menu-save-game", 1);
}

local void LoadAction(void)
{
    Menu *menu;
    char filename[256];
    FileList *fl;
    char *name;
    size_t nameLength;

    menu = FindMenu("menu-load-game");
    fl = menu->items[1].d.listbox.options;

    name = fl[menu->items[1].d.listbox.curopt].name;
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
    Menu *menu;

    menu = FindMenu("menu-load-game");
    menu->items[3].flags = MI_DISABLED;		// Load button!
    GameLoaded=0;
    ProcessMenu("menu-load-game", 1);
    if( GameLoaded ) {
	GameMenuReturn();
    }
}

local void InitGameMenu(Menuitem *mi __attribute__((unused)))
{
    // FIXME: populate...
}

global void SoundOptions(void)
{
    Menu *menu;

    menu = FindMenu("menu-sound-options");
#ifdef WITH_SOUND
    menu->items[2].flags = 0;				// master volume slider
    menu->items[5].d.gem.state = MI_GSTATE_CHECKED;	// master power
    menu->items[8].flags = 0;				// music volume slider
    menu->items[11].flags = 0;				// music power
    menu->items[11].d.gem.state = MI_GSTATE_CHECKED;
    menu->items[14].flags = 0;				// cd volume slider
    menu->items[17].d.gem.state = MI_GSTATE_CHECKED;	// cd power
    menu->items[19].flags = 0;				// all tracks button
    menu->items[19].d.gem.state = MI_GSTATE_UNCHECKED;
    menu->items[21].flags = 0;				// random tracks button
    menu->items[21].d.gem.state = MI_GSTATE_UNCHECKED;

    // Set master volume checkbox and slider
    if (SoundFildes == -1) {
	menu->items[5].d.gem.state = MI_GSTATE_UNCHECKED;
	menu->items[2].flags = -1;
	menu->items[11].flags = -1;
    }
    menu->items[2].d.hslider.percent = (GlobalVolume * 100) / 255;

    // Set music volume checkbox and slider
    if (PlayingMusic != 1 || SoundFildes == -1) {
	menu->items[11].d.gem.state = MI_GSTATE_UNCHECKED;
	menu->items[8].flags = -1;
    }
    menu->items[8].d.hslider.percent = (MusicVolume * 100) / 255;

#if defined(USE_LIBCDA) || defined(USE_SDLCD)
    if (!strcmp(":off", CDMode) || !strcmp(":stopped", CDMode)) {
	menu->items[17].d.gem.state = MI_GSTATE_UNCHECKED;
	menu->items[14].flags = -1;
	menu->items[19].flags = -1;

	menu->items[21].flags = -1;
    } else {
#ifdef USE_LIBCDA
	int i = 17;

	cd_get_volume(&i, &i);
	menu->items[14].d.hslider.percent = (i * 100) / 255;
#endif
	if (!strcmp(":all", CDMode)) {
	    menu->items[19].d.gem.state = MI_GSTATE_CHECKED;
	}
	if (!strcmp(":random", CDMode)) {
	    menu->items[21].d.gem.state = MI_GSTATE_CHECKED;
	}
	menu->items[11].flags = -1;
    }
#else
    menu->items[14].flags = -1;			// cd volume slider
    menu->items[17].flags = -1;			// cd power
    menu->items[17].d.gem.state = MI_GSTATE_UNCHECKED;
    menu->items[19].flags = -1;			// all tracks button
    menu->items[21].flags = -1;			// random tracks button
#endif
    if (InterfaceState == IfaceStateMenu)
	ProcessMenu("menu-sound-options", 0);
    else
	ProcessMenu("menu-sound-options", 1);
#endif // with sound
}

local void GlobalOptions(Menuitem *mi)
{
}

local void InitGlobalOptions(Menuitem *mi)
{
}

local void SetRes640(Menuitem *mi)
{
}

local void SetRes800(Menuitem *mi)
{
}

local void SetRes1024(Menuitem *mi)
{
}

local void SetRes1280(Menuitem *mi)
{
}

local void SetRes1600(Menuitem *mi)
{
}

local void SetFullscreen(Menuitem *mi)
{
}

local void SetShadowFogAlpha(Menuitem *mi)
{
}

local void SetShadowFogGray(Menuitem *mi)
{
}

local void SetMasterPower(Menuitem *mi __attribute__((unused)))
{
#ifdef WITH_SOUND
    if (SoundFildes != -1) {
#ifdef USE_SDLA
	SDL_CloseAudio();
#else
        close(SoundFildes);
#endif
	SoundFildes=-1;
	SoundOff=1;
    } else {
	SoundOff=0;
	if( InitSound() ) {
	    SoundOff=1;
	    SoundFildes=-1;
	}
	MapUnitSounds();
	InitSoundServer();
	InitSoundClient();
    }
#endif
    CurrentMenu=NULL;
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
    CurrentMenu=NULL;
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
    CurrentMenu=NULL;
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
    CurrentMenu=NULL;
    SoundOptions();

}

local void SetCdModeRandom(Menuitem *mi __attribute__((unused)))
{
#if defined(USE_LIBCDA) || defined(USE_SDLCD)
    CDMode = ":random";
#endif
    CurrentMenu=NULL;
    SoundOptions();

}

global void SpeedSettings(void)
{
    Menu *menu;
    int i = 2;

    menu = FindMenu("menu-settings-speed");
    menu->items[i].d.hslider.percent = ((VideoSyncSpeed - MIN_GAME_SPEED) * 100) / (MAX_GAME_SPEED - MIN_GAME_SPEED);
    if (menu->items[i].d.hslider.percent < 0)
	menu->items[i].d.hslider.percent = 0;
    if (menu->items[i].d.hslider.percent > 100)
	menu->items[i].d.hslider.percent = 100;
    menu->items[i + 4].d.hslider.percent = 100 - (SpeedMouseScroll - 1) * 100 / 10;
    if (TheUI.MouseScroll == 0)
	menu->items[i + 4].d.hslider.percent = 0;
    menu->items[i + 8].d.hslider.percent = 100 - (SpeedKeyScroll - 1) * 100 / 10;
    if (TheUI.KeyScroll == 0)
	menu->items[i + 8].d.hslider.percent = 0;
    ProcessMenu("menu-settings-speed", 1);
}

/**
**	Enter the preferences menu.
**	Setup if the options are enabled / disabled.
**
**	@todo	FIXME: The init should be done by the init callback.
*/
global void Preferences(void)
{
    Menu *menu;

    menu = FindMenu("menu-preferences");
    if (!TheMap.NoFogOfWar) {
	menu->items[1].d.gem.state = MI_GSTATE_CHECKED;
    } else {
	menu->items[1].d.gem.state = MI_GSTATE_UNCHECKED;
    }

    if (NetworkFildes == -1) {		// Not available in net games
	menu->items[1].flags = MI_ENABLED;
    } else {
	menu->items[1].flags = MI_DISABLED;
    }

    if (ShowCommandKey) {
	menu->items[3].d.gem.state = MI_GSTATE_CHECKED;
    } else {
	menu->items[3].d.gem.state = MI_GSTATE_UNCHECKED;
    }

    ProcessMenu("menu-preferences", 1);
}

/**
**	Show the game options.
*/
local void GameOptions(void)
{
    ProcessMenu("menu-game-options", 1);
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
    ProcessMenu("menu-end-scenario", 1);
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
    CurrentMenu=NULL;
}

local void KeystrokeHelpMenu(void)
{
    Menu *menu;

    menu = FindMenu("menu-keystroke-help");
    menu->items[1].d.vslider.percent = 0;
    ProcessMenu("menu-keystroke-help", 1);
}

local void HelpMenu(void)
{
    ProcessMenu("menu-help", 1);
}

local void ShowTipsMenu(void)
{
    Menu *menu;

    menu = FindMenu("menu-tips");
    if (ShowTips) {
	menu->items[1].d.gem.state = MI_GSTATE_CHECKED;
    } else {
	menu->items[1].d.gem.state = MI_GSTATE_UNCHECKED;
    }

    ProcessMenu("menu-tips", 1);
}

/**
**	Free the tips from the menu
*/
local void FreeTips()
{
    int i;
    Menu *menu;

    menu = FindMenu("menu-tips");
    for( i=5; i<13; i++ ) {
	if( menu->items[i].d.text.text ) {
	    free(menu->items[i].d.text.text);
	    menu->items[i].d.text.text=NULL;
	}
    }
}

/**
**	Initialize the tips menu
*/
#ifdef OLD_MENU
local void InitTips(Menuitem *mi __attribute__((unused)))
#else
local void InitTips(Menuitem *mi)
#endif
{
    int i;
    int line;
    char* p;
    char* s;
    char* str;
    int w;
    int font;
    int l;
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-tips");
#else
    menu = mi->menu;
#endif
    if( ShowTips ) {
	menu->items[1].d.gem.state=MI_GSTATE_CHECKED;
    } else {
	menu->items[1].d.gem.state=MI_GSTATE_UNCHECKED;
    }

    FreeTips();

    w=menu->xsize-2*menu->items[5].xofs;
    font=menu->items[5].font;
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
	menu->items[line++].d.text.text=strdup(s);
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
#ifdef OLD_MENU
local void SetTips(Menuitem *mi __attribute__((unused)))
#else
local void SetTips(Menuitem *mi)
#endif
{
#ifdef OLD_MENU
    Menu *menu;

    menu = FindMenu("menu-tips");
    if (menu->items[1].d.gem.state == MI_GSTATE_CHECKED)
#else
    if (mi->menu->items[1].d.gem.state == MI_GSTATE_CHECKED)
#endif
	 {
	ShowTips = 1;
    } else {
	ShowTips = 0;
    }
}

local void NextTip(void)
{
    CurrentTip++;
    if (Tips[CurrentTip] == NULL)
	CurrentTip = 0;
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
local void SetMenuObjectives(void)
{
    int i;
    int line;
    char* p;
    char* s;
    char* str;
    int w;
    int font;
    int l;
    Menu *menu;

    menu = FindMenu("menu-objectives");
    w=menu->xsize-2*menu->items[1].xofs;
    font=menu->items[1].font;
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
	    menu->items[line++].d.text.text=strdup(s);
	    l+=strlen(s);
	    if( !p[l] ) {
		break;
	    }
	    ++l;
	    s=str+l;

	    if( line==menu->nitems-1 ) {
		break;
	    }
	}
	free(str);
    }
}

/**
**
*/
local void FreeMenuObjectives(void)
{
    int i;
    Menu *menu;

    menu = FindMenu("menu-objectives");
    for( i=1;i<menu->nitems-1;i++ ) {
	if( menu->items[i].d.text.text ) {
	    free(menu->items[i].d.text.text);
	    menu->items[i].d.text.text=NULL;
	}
    }
}

/**
**
*/
local void GameMenuObjectives(void)
{
    SetMenuObjectives();
    ProcessMenu("menu-objectives", 1);
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
    ProcessMenu("menu-select-scenario", 1);

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
    ProcessMenu("menu-custom-game", 1);
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
    Menu *menu;

    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();
    Invalidate();

    menu = FindMenu("menu-campaign-select");
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

	menu->items[i].d.button.text=Campaigns[i].Name;
	menu->items[i].flags&=~MenuButtonDisabled;

	if( (s=strchr(Campaigns[i].Name,'!')) ) {
	    menu->items[i].d.button.hotkey=tolower(s[1]);
	}
    }
    for( ; i<4; ++i ) {
	menu->items[i].d.button.text="Not available";
	menu->items[i].flags|=MenuButtonDisabled;
    }

    GuiGameStarted = 0;
    ProcessMenu("menu-campaign-select", 1);
    if (GuiGameStarted) {
	GameMenuReturn();
    }

    for( i=0; i<4; ++i ) {
	menu->items[i].d.button.text=NULL;
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
    ProcessMenu("menu-campaign-continue", 1);
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
    Menu *menu;
    menu = FindMenu("menu-enter-name");
    menu->items[1].d.input.nch = 0;
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
    Menu *menu;
    menu = FindMenu("menu-enter-server");
    menu->items[1].d.input.nch = 0;
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
    Menu *menu;

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
    menu = FindMenu("menu-enter-server");
    strcat(server_host_buffer, "~!_");
    menu->items[1].d.input.buffer = server_host_buffer;
    menu->items[1].d.input.nch = strlen(server_host_buffer) - 3;
    menu->items[1].d.input.maxch = 24;
    if (menu->items[1].d.input.nch) {
	menu->items[2].flags &= ~MenuButtonDisabled;
    } else {
	menu->items[2].flags |= MenuButtonDisabled;
    }

    ProcessMenu("menu-enter-server", 1);

    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();

    if (menu->items[1].d.input.nch == 0) {
	return;
    }
    // Now finally here is the address
    server_host_buffer[menu->items[1].d.input.nch] = 0;
    if (NetworkSetupServerAddress(server_host_buffer)) {
	menu->items[1].d.text.text = "Unable to lookup host.";
	ProcessMenu("menu-net-error", 1);
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
    ProcessMenu("menu-net-connecting", 1);

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
    Menu *menu;

    menu = FindMenu("menu-net-error");
    switch (NetLocalState) {
	case ccs_unreachable:
	    menu->items[1].d.text.text = "Cannot reach server.";
	    ProcessMenu("menu-net-error", 1);

	    NetConnectingCancel();
	    return;
	case ccs_nofreeslots:
	    menu->items[1].d.text.text = "Server is full.";
	    ProcessMenu("menu-net-error", 1);

	    NetConnectingCancel();
	    return;
	case ccs_serverquits:
	    menu->items[1].d.text.text = "Server gone.";
	    ProcessMenu("menu-net-error", 1);

	    NetConnectingCancel();
	    return;
	case ccs_incompatibleengine:
	    menu->items[1].d.text.text = "Incompatible engine version.";
	    ProcessMenu("menu-net-error", 1);

	    NetConnectingCancel();
	    return;
	case ccs_incompatiblenetwork:
	    menu->items[1].d.text.text = "Incompatible network version.";
	    ProcessMenu("menu-net-error", 1);

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
    ProcessMenu("menu-net-multi-client", 1);
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
    ProcessMenu("menu-multi-setup", 1);
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
    Menu *menu;

    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();
    Invalidate();

    menu = FindMenu("menu-enter-name");
    menu->items[1].d.input.buffer = NameBuf;
    strcpy(NameBuf, NetworkName);
    strcat(NameBuf, "~!_");
    menu->items[1].d.input.nch = strlen(NameBuf) - 3;
    menu->items[1].d.input.maxch = 15;
    menu->items[2].flags &= ~MenuButtonDisabled;

    ProcessMenu("menu-enter-name", 1);

    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();

    if (menu->items[1].d.input.nch == 0) {
	return;
    }

    NameBuf[menu->items[1].d.input.nch] = 0;	// Now finally here is the name
    memset(NetworkName, 0, 16);
    strcpy(NetworkName, NameBuf);

    GuiGameStarted = 0;
    // Here we really go...
    ProcessMenu("menu-create-join-menu", 1);

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
#ifdef OLD_MENU
local void ScenSelectInit(Menuitem *mi __attribute__ ((unused)))
#else
local void ScenSelectInit(Menuitem *mi)
#endif
{
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-select-scenario");
#else
    menu = mi->menu;
#endif
    DebugCheck(!*ScenSelectPath);
    menu->items[9].flags =
	*ScenSelectDisplayPath ? 0 : MenuButtonDisabled;
    menu->items[9].d.button.text = ScenSelectDisplayPath;
    DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);
}

local void ScenSelectLBAction(Menuitem *mi, int i)
{
    FileList *fl;
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-select-scenario");
#else
    menu = mi->menu;
#endif
    DebugCheck(i<0);
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    menu->items[3].d.button.text = "OK";
	} else {
	    menu->items[3].d.button.text = "Open";
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
    Menu *menu;
#ifdef USE_ZZIPLIB
    ZZIP_FILE *zzf;
#endif

    menu = FindMenu("menu-select-scenario");

#if 0
    if (menu->items[6].d.pulldown.curopt == 0) {
	suf = NULL;
	p = -1;
    } else
#endif
    if (menu->items[6].d.pulldown.curopt == 0) {
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
			sz = szl[menu->items[8].d.pulldown.curopt];
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
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-select-scenario");
#else
    menu = mi->menu;
#endif
    ScenSelectLBExit(mi);
    if (menu->items[6].d.pulldown.curopt == 0) {
	menu->items[8].flags |= MenuButtonDisabled;
    } else {
	menu->items[8].flags &= ~MenuButtonDisabled;
    }
    i = mi->d.listbox.noptions = ReadDataDirectory(ScenSelectPath, ScenSelectRDFilter,
						     (FileList **)&(mi->d.listbox.options));
    if (i == 0) {
	menu->items[3].d.button.text = "OK";
	menu->items[3].flags |= MenuButtonDisabled;
    } else {
	ScenSelectLBAction(mi, 0);
	menu->items[3].flags &= ~MenuButtonDisabled;
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
#ifdef OLD_MENU
		    menu = FindMenu("menu-select-scenario");
#else
		    menu = mi->menu;
#endif
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
#ifdef OLD_MENU
    Menu *menu;
    menu = FindMenu("menu-select-scenario");
    mi = menu->items + 1;
#else
    mi = mi->menu->items + 1;
#endif
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
    int j, nitems = sizeof(keystrokekeystexts) / sizeof(unsigned char *);

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
    int i, j, nitems = sizeof(keystrokekeystexts) / sizeof(unsigned char *);
    int i2, j2, nitems2 = sizeof(keystrokehintstexts) / sizeof(unsigned char *);
#ifdef OLD_MENU
    Menu *menu = FindMenu("menu-keystroke-help");
#else
    Menu *menu = mi->menu;
#endif

    j = ((mi[-2].d.vslider.percent + 1) * (nitems - 11)) / 100;
    for (i = 0; i < 11; i++) {
	VideoDrawText(menu->x+mi->xofs,menu->y+mi->yofs+(i*20),
			    mi->font,keystrokekeystexts[j]);
	j++;
    }

    j2 = ((mi[-2].d.vslider.percent + 1) * (nitems2 - 11)) / 100;
    for (i2 = 0; i2 < 11; i2++) {
	VideoDrawText(menu->x+mi->xofs+80,menu->y+mi->yofs+(i2*20),
			    mi->font,keystrokehintstexts[j2]);
	j2++;
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
    Menu *menu;
    Menuitem *mi;

    menu = FindMenu("menu-select-scenario");
    mi = menu->items + 1;
    if (ScenSelectDisplayPath[0]) {
	cp = strrchr(ScenSelectDisplayPath, '/');
	if (cp) {
	    *cp = 0;
	} else {
	    ScenSelectDisplayPath[0] = 0;
	    menu->items[9].flags |= MenuButtonDisabled;
	    menu->items[9].d.button.text = NULL;
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
    Menu *menu;
    Menuitem *mi;
    int i;

    menu = FindMenu("menu-select-scenario");
    mi = &menu->items[1];
    i = mi->d.listbox.curopt + mi->d.listbox.startline;
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    strcat(ScenSelectPath, "/");
	    strcat(ScenSelectPath, fl[i].name);
	    if (menu->items[9].flags&MenuButtonDisabled) {
		menu->items[9].flags &= ~MenuButtonDisabled;
		menu->items[9].d.button.text = ScenSelectDisplayPath;
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
    char *s;

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
local void GameSetupInit(Menuitem *mi __attribute__ ((unused)))
{
    char *s;

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
    Menu *menu;
    int i;

    menu = FindMenu("menu-multi-setup");
    i = mi - menu->items - SERVER_PLAYER_STATE;
    // JOHNS: Must this be always true?
    // ARI: NO! think of client menus!
    // DebugCheck( i<0 || i>PlayerMax-1 );

    if (i > 0 && i < PlayerMax-1) {
	if (mi->d.pulldown.curopt == o) {
	    if (mi->d.pulldown.noptions == 2) {	// computer slot
		ServerSetupState.CompOpt[i] = o + 1;
	    } else {
		ServerSetupState.CompOpt[i] = o;
	    }
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
    int c, h, i;
    int num[PlayerMax], comp[PlayerMax];

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
    for (c = h = i = 0; i < PlayerMax; i++) {
	if (ScenSelectPudInfo->PlayerType[i] == PlayerPerson) {
	    DebugLevel3Fn("Player slot %i is available for a person\n" _C_ i);
	    num[h++] = i;
	}
	if (ScenSelectPudInfo->PlayerType[i] == PlayerComputer) {
	    comp[c++] = i;	// available computer player slots
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
    for (i = 0; i < c; i++) {
	if (ServerSetupState.CompOpt[comp[i]] == 2) {	// closed..
	    GameSettings.Presets[comp[i]].Type = PlayerNobody;
	    DebugLevel0Fn("Settings[%d].Type == Closed\n" _C_ comp[i]);
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
**	Caution: Called by map change (initial = 1)!
**	Caution: Called by network events from clients (initial = 2)!
**	Caution: Called by button action on server (initial = 3)!
*/
local void MultiGamePlayerSelectorsUpdate(int initial)
{
    Menu *menu;
    int i, h, c;
    int avail, ready;

    menu = FindMenu("menu-multi-setup");

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
	    if (i < h) {
		ServerSetupState.CompOpt[i] = 0;
	    }
	    menu->items[SERVER_PLAYER_READY - 1 + i].flags = 0;
	    menu->items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;

	    menu->items[SERVER_PLAYER_LAG - 1 + i].flags = 0;
	    menu->items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;

	    // FIXME: don't forget to throw out additional players
	    //	  without available slots here!

	}
	if (Hosts[i].PlyNr) {
	    menu->items[SERVER_PLAYER_STATE + i] = NetMultiButtonStorage[1];

	    menu->items[SERVER_PLAYER_READY - 1 + i].flags = 0;
	    menu->items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;
	    if (ServerSetupState.Ready[i]) {
		menu->items[SERVER_PLAYER_READY - 1 + i].d.gem.state |= MI_GSTATE_CHECKED;
		++ready;
	    }

	    menu->items[SERVER_PLAYER_LAG - 1 + i].flags = 0;
	    menu->items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;
	} else {
	    // don't allow network and button events to intercept server player's action on pulldown buttons!
	    if (!(menu->items[SERVER_PLAYER_STATE + i].flags&MenuButtonClicked)) {
		if (initial == 1 ||
		    (initial == 2 && menu->items[SERVER_PLAYER_STATE + i].mitype != MI_TYPE_PULLDOWN)) {
		    menu->items[SERVER_PLAYER_STATE + i] = NetMultiButtonStorage[0];
		    menu->items[SERVER_PLAYER_STATE + i].d.pulldown.state = 0;
		    menu->items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = ServerSetupState.CompOpt[i];
		}
	    }
	    if (i < h && ServerSetupState.CompOpt[i] != 0) {
		avail--;
	    }

	    menu->items[SERVER_PLAYER_READY - 1 + i].flags = MenuButtonDisabled;
	    menu->items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;

	    menu->items[SERVER_PLAYER_LAG - 1 + i].flags = MenuButtonDisabled;
	    menu->items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
	}

	menu->items[SERVER_PLAYER_STATE + i].yofs = 32 + (i&7) * 22;
	if (i > 7) {
	    menu->items[SERVER_PLAYER_STATE + i].xofs = 320 + 40;
	}


	if (i >= h) {
	    // Allow to switch off (close) preset ai-computer slots
	    // FIXME: evaluate game type...
	    if (initial == 1 && i < h + c) {
		menu->items[SERVER_PLAYER_STATE + i].d.pulldown.state = 0;
		menu->items[SERVER_PLAYER_STATE + i].d.pulldown.noptions = 2;
		menu->items[SERVER_PLAYER_STATE + i].d.pulldown.options = mgptsoptions + 1;
		menu->items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = 0;
		ServerSetupState.CompOpt[i] = 1;
		menu->items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = ServerSetupState.CompOpt[i] - 1;
	    }

	    menu->items[SERVER_PLAYER_READY - 1 + i].flags = MenuButtonDisabled;
	    menu->items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;

	    menu->items[SERVER_PLAYER_LAG - 1 + i].flags = MenuButtonDisabled;
	    menu->items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
	}

	if (i >= h + c) {
	    menu->items[SERVER_PLAYER_STATE + i].d.pulldown.state = MI_PSTATE_PASSIVE;
	    menu->items[SERVER_PLAYER_STATE + i].d.pulldown.defopt = 2;
	    menu->items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = 2;
	    menu->items[SERVER_PLAYER_STATE + i].flags = MenuButtonDisabled;
	}
    }

    //	Tell connect state machines how many interactive players we can have
    NetPlayers = avail;
    //	Check if all players are ready.
    DebugLevel0Fn("READY to START: AVAIL = %d, READY = %d\n" _C_ avail _C_ ready);
    if (ready == avail) {
	if (menu->items[3].flags == MenuButtonDisabled) {
	    // enable start game button
	    menu->items[3].flags = 0;
	}
    } else {
	// disable start game button
	menu->items[3].flags = MenuButtonDisabled;
    }
}

/**
**	Update client network menu.
*/
local void MultiClientUpdate(int initial)
{
    Menu *menu;
    int i, h, c;

    menu = FindMenu("menu-net-multi-client");

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
	menu->items[CLIENT_PLAYER_STATE] = NetMultiButtonStorage[1];
	menu->items[CLIENT_PLAYER_STATE].yofs = 32;
	memset(&ServerSetupState, 0, sizeof(ServerSetup));
	memset(&LocalSetupState, 0, sizeof(ServerSetup));
    }
    for (i = 1; i < PlayerMax - 1; i++) {
	if (Hosts[i].PlyNr) {
	    menu->items[CLIENT_PLAYER_STATE + i] =
		NetMultiButtonStorage[1];
	    if (i == NetLocalHostsSlot) {
		menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state = 0;
	    } else {
		menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;
	    }
	} else {
	    menu->items[CLIENT_PLAYER_STATE + i] = NetMultiButtonStorage[0];
	    menu->items[CLIENT_PLAYER_STATE + i].d.pulldown.state = MI_PSTATE_PASSIVE;
	    menu->items[CLIENT_PLAYER_STATE + i].d.pulldown.curopt = ServerSetupState.CompOpt[i];
	    menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
	}
	menu->items[CLIENT_PLAYER_STATE + i].yofs = 32 + (i&7) * 22;
	if (i > 7) {
	    menu->items[CLIENT_PLAYER_STATE + i].xofs = 320 + 40;
	}

	menu->items[CLIENT_PLAYER_READY - 1 + i].flags = 0;
	if (ServerSetupState.Ready[i]) {
	    menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state |= MI_GSTATE_CHECKED;
	} else {
	    menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state &= ~MI_GSTATE_CHECKED;
	}

	if (i >= h) {
	    menu->items[CLIENT_PLAYER_STATE + i].d.pulldown.curopt = ServerSetupState.CompOpt[i];
	}
	if (i >= h + c) {
	    menu->items[CLIENT_PLAYER_READY - 1 + i].flags = MenuButtonDisabled;
	    menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
	    menu->items[CLIENT_PLAYER_STATE + i].d.pulldown.defopt =
			 menu->items[CLIENT_PLAYER_STATE + i].d.pulldown.curopt = 2;
	    menu->items[CLIENT_PLAYER_STATE + i].flags = MenuButtonDisabled;
	}
    }
}

local void MultiGameSetupInit(Menuitem *mi)
{
    Menu *menu;
    int i, h;

#ifdef OLD_MENU
    menu = FindMenu("menu-multi-setup");
#else
    menu = mi->menu;
#endif

    GameSetupInit(mi);
    NetworkInitServerConnect();
    menu->items[SERVER_PLAYER_STATE] = NetMultiButtonStorage[1];
    menu->items[SERVER_PLAYER_STATE].yofs = 32;

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
*/
local void NetMultiPlayerDrawFunc(Menuitem *mi)
{
    Menu *menu;
    int i, nc, rc;

    menu = FindMenu("menu-multi-setup");
    i = mi - menu->items - SERVER_PLAYER_STATE;
    if (i >= 0 && i < PlayerMax - 1) {		// Ugly test to detect server
	if (i > 0) {
	    menu->items[SERVER_PLAYER_READY - 1 + i].flags &=
		~MenuButtonDisabled;
	    // Note: re-disabled in MultiGamePlayerSelectorsUpdate()
	    //		for kicked out clients!!
	    if (ServerSetupState.Ready[i]) {
		menu->items[SERVER_PLAYER_READY - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE|MI_GSTATE_CHECKED;
	    } else {
		menu->items[SERVER_PLAYER_READY - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE;
	    }
	    if (ServerSetupState.LastFrame[i] + 30 > FrameCounter) {
		menu->items[SERVER_PLAYER_LAG - 1 + i].flags &=
		    ~MenuButtonDisabled;
		menu->items[SERVER_PLAYER_LAG - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE|MI_GSTATE_CHECKED;
	    } else {
		menu->items[SERVER_PLAYER_LAG - 1 + i].flags |=
		    MenuButtonDisabled;
		menu->items[SERVER_PLAYER_LAG - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE;
	    }

	}
    } else {
	menu = FindMenu("menu-net-multi-client");
	i = mi - menu->items - CLIENT_PLAYER_STATE;
	if (i > 0) {
	    menu->items[CLIENT_PLAYER_READY - 1 + i].flags &=
		~MenuButtonDisabled;
	    if (i == NetLocalHostsSlot) {
		menu->items[CLIENT_PLAYER_READY - 1 + i].
			d.gem.state &= ~MI_GSTATE_PASSIVE;
	    } else {
		menu->items[CLIENT_PLAYER_READY - 1 + i].
			d.gem.state |= MI_GSTATE_PASSIVE;
	    }
	}
    }

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

#ifdef OLD_MENU
local void MultiGameClientInit(Menuitem *mi __attribute__((unused)))
#else
local void MultiGameClientInit(Menuitem *mi)
#endif
{
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-net-multi-client");
#else
    menu = mi->menu;
#endif

    // GameSetupInit(mi);
    MultiClientUpdate(1);
    if (LocalSetupState.Ready[NetLocalHostsSlot]) {
	menu->items[2].flags = MenuButtonDisabled;
	menu->items[3].flags = 0;
    } else {
	menu->items[3].flags = MenuButtonDisabled;
	menu->items[2].flags = 0;
    }
}

/**
**	Multiplayer client gem action. Toggles ready flag.
*/
local void MultiClientGemAction(Menuitem *mi)
{
    Menu *menu;
    int i;

#ifdef OLD_MENU
    menu = FindMenu("menu-net-multi-client");
#else
    menu = mi->menu;
#endif
    i = mi - menu->items - CLIENT_PLAYER_READY + 1;
    DebugLevel3Fn("i = %d, NetLocalHostsSlot = %d\n" _C_ i _C_ NetLocalHostsSlot);
    if (i == NetLocalHostsSlot) {
	LocalSetupState.Ready[i] = !LocalSetupState.Ready[i];
	if (LocalSetupState.Ready[i]) {
	    menu->items[2].flags = MenuButtonDisabled;
	    menu->items[3].flags = 0;
	} else {
	    menu->items[3].flags = MenuButtonDisabled;
	    menu->items[2].flags = 0;
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
    Menu *menu;

    menu = FindMenu("menu-net-multi-client");
    menu->items[2].flags = MenuButtonDisabled;
    menu->items[3].flags = 0;
    LocalSetupState.Ready[NetLocalHostsSlot] = 1;
    MultiClientUpdate(0);
}

local void MultiClientNotReady(void)
{
    Menu *menu;

    menu = FindMenu("menu-net-multi-client");
    menu->items[3].flags = MenuButtonDisabled;
    menu->items[2].flags = 0;
    LocalSetupState.Ready[NetLocalHostsSlot] = 0;
    MultiClientUpdate(0);
}

/**
**	Callback from netconnect loop in Client-Sync state:
**	Compare local state with server's information
**	and force update when changes have occured.
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
    Menu *menu;

    menu = FindMenu("menu-net-multi-client");

    GameRESAction(NULL, ServerSetupState.ResOpt);
    menu->items[CLIENT_RESOURCE].d.pulldown.curopt =
	ServerSetupState.ResOpt;

    GameUNSAction(NULL, ServerSetupState.UnsOpt);
    menu->items[CLIENT_UNITS].d.pulldown.curopt =
	ServerSetupState.UnsOpt;

    MultiGameFWSAction(NULL, ServerSetupState.FwsOpt);
    menu->items[CLIENT_FOG_OF_WAR].d.pulldown.curopt =
	ServerSetupState.FwsOpt;

    GameTSSAction(NULL, ServerSetupState.TssOpt);
    menu->items[CLIENT_TILESET].d.pulldown.curopt =
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
    char* s;

    VideoLockScreen();
    StartMenusSetBackground(NULL);
    VideoUnlockScreen();
    Invalidate();

    if (!*CurrentMapPath || *CurrentMapPath == '.' || *CurrentMapPath == '/') {
	strcpy(CurrentMapPath, "default.pud");
    }

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

    GetInfoFromSelectPath();

    ProcessMenu("menu-editor-select", 1);

}

local void EditorNewMap(void)
{
    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    // FIXME: currently just loads default.pud
    strcpy(CurrentMapPath, "default.pud");

    EditorMainLoop();
    EndMenu();
}

local void EditorLoadMap(void)
{
    char *p;

    EditorLoadCancelled=0;
    ProcessMenu("menu-editor-load-map", 1);
    GetInfoFromSelectPath();

    if (EditorLoadCancelled) {
	VideoLockScreen();
	StartMenusSetBackground(NULL);
	VideoUnlockScreen();
	return;
    }

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    if (ScenSelectPath[0]) {
	strcat(ScenSelectPath, "/");
	strcat(ScenSelectPath, ScenSelectFileName);	// Final map name with path
	p = ScenSelectPath + strlen(FreeCraftLibPath) + 1;
	strcpy(CurrentMapPath, p);
    } else {
	strcpy(CurrentMapPath, ScenSelectFileName);
	strcat(ScenSelectPath, ScenSelectFileName);	// Final map name with path
    }

    EditorMainLoop();
    EndMenu();
}

#ifdef OLD_MENU
local void EditorLoadInit(Menuitem *mi __attribute__((unused)))
#else
local void EditorLoadInit(Menuitem *mi)
#endif
{
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-editor-load-map");
#else
    menu = mi->menu;
#endif
    DebugCheck(!*ScenSelectPath);
    menu->items[5].flags =
	*ScenSelectDisplayPath ? 0 : MenuButtonDisabled;
    menu->items[5].d.button.text = ScenSelectDisplayPath;
    DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);
}

local void EditorLoadLBInit(Menuitem *mi)
{
    Menu *menu;
    int i;

#ifdef OLD_MENU
    menu = FindMenu("menu-editor-load-map");
#else
    menu = mi->menu;
#endif
    EditorLoadLBExit(mi);
    i = mi->d.listbox.noptions = ReadDataDirectory(ScenSelectPath, EditorLoadRDFilter,
	(FileList **)&(mi->d.listbox.options));

    if (i == 0) {
	menu->items[3].d.button.text = "OK";
	menu->items[3].flags |= MenuButtonDisabled;
    } else {
	EditorLoadLBAction(mi, 0);
	menu->items[3].flags &= ~MenuButtonDisabled;
	if (i > 5) {
	    mi[1].flags &= ~MenuButtonDisabled;
	}
    }
}

local void EditorLoadLBExit(Menuitem *mi)
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

local int EditorLoadRDFilter(char *pathbuf, FileList *fl)
{
    MapInfo *info;
    char *suf;
    char *np, *cp, *lcp;
#ifdef USE_ZZIPLIB
    int sz;
    ZZIP_FILE *zzf;
#endif

    suf = ".pud";
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
	    if (strcasestr(pathbuf, ".pud")) {
		info = GetPudInfo(pathbuf);
		if (info) {
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

local void EditorLoadFolder(void)
{
    Menu *menu;
    Menuitem *mi;
    char *cp;

    menu = FindMenu("menu-editor-load-map");
    mi = &menu->items[1];

    if (ScenSelectDisplayPath[0]) {
	cp = strrchr(ScenSelectDisplayPath, '/');
	if (cp) {
	    *cp = 0;
	} else {
	    ScenSelectDisplayPath[0] = 0;
	    menu->items[5].flags |= MenuButtonDisabled;
	    menu->items[5].d.button.text = NULL;
	}
	cp = strrchr(ScenSelectPath, '/');
	if (cp) {
	    *cp = 0;
	    EditorLoadLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    mi[1].d.hslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	}
    }
}

local void EditorLoadOk(void)
{
    Menu *menu;
    Menuitem *mi;
    FileList *fl;
    int i;

    menu = FindMenu("menu-editor-load-map");
    mi = &menu->items[1];
    i = mi->d.listbox.curopt + mi->d.listbox.startline;
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    strcat(ScenSelectPath, "/");
	    strcat(ScenSelectPath, fl[i].name);
	    if (menu->items[5].flags&MenuButtonDisabled) {
		menu->items[5].flags &= ~MenuButtonDisabled;
		menu->items[5].d.button.text = ScenSelectDisplayPath;
	    } else {
		strcat(ScenSelectDisplayPath, "/");
	    }
	    strcat(ScenSelectDisplayPath, fl[i].name);
	    EditorLoadLBInit(mi);
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

local void EditorLoadCancel(void)
{
    char* s;

    EditorLoadCancelled=1;

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

local unsigned char *EditorLoadLBRetrieve(Menuitem *mi, int i)
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
#ifdef OLD_MENU
		    menu = FindMenu("menu-editor-load-map");
#else
		    menu = mi->menu;
#endif
		    if (info->Description) {
			VideoDrawText(menu->x+8,menu->y+234,LargeFont,info->Description);
		    }
		    sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
		    VideoDrawText(menu->x+8,menu->y+234+20,LargeFont,buffer);
		    for (n = j = 0; j < PlayerMax; j++) {
			if (info->PlayerType[j] == PlayerPerson) {
			    n++;
			}
		    }
		    if (n == 1) {
			VideoDrawText(menu->x+8,menu->y+234+40,LargeFont,"1 player");
		    } else {
			sprintf(buffer, "%d players", n);
			VideoDrawText(menu->x+8,menu->y+234+40,LargeFont,buffer);
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

local void EditorLoadLBAction(Menuitem *mi, int i)
{
    Menu *menu;
    FileList *fl;

#ifdef OLD_MENU
    menu = FindMenu("menu-editor-load-map");
#else
    menu = mi->menu;
#endif
    DebugCheck(i<0);
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    menu->items[3].d.button.text = "OK";
	} else {
	    menu->items[3].d.button.text = "Open";
	}
	if (mi->d.listbox.noptions > 5) {
	    mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	    mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	}
    }
}

local void EditorLoadVSAction(Menuitem *mi, int i)
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
	    EditorLoadLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
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

		EditorLoadLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void EditorMapProperties(void)
{
    Menu *menu;
    char MapDescription[36];
    char MapSize[30];

    menu = FindMenu("menu-editor-map-properties");

    menu->items[2].d.input.buffer = MapDescription;
    strcpy(MapDescription, TheMap.Info->Description);
    strcat(MapDescription, "~!_");
    menu->items[2].d.input.nch = strlen(MapDescription)-3;
    menu->items[2].d.input.maxch = 31;

    sprintf(MapSize, "%d x %d", TheMap.Width, TheMap.Height);
    menu->items[4].d.text.text = MapSize;

    menu->items[6].d.pulldown.defopt = TheMap.Terrain;

    // FIXME: Set the correct pud version
    menu->items[8].d.pulldown.defopt = 1;
    menu->items[8].flags = -1;

    ProcessMenu("menu-editor-map-properties", 1);
}

local void EditorEnterMapDescriptionAction(Menuitem *mi, int key)
{
    if (key == 10 || key == 13) {
	EditorEndMenu();
    }
}

local void EditorMapPropertiesOk(void)
{
    Menu *menu;
    char *MapDescription;
    // FIXME: TilesetSummer, ... shouldn't be used, they will be removed.
    int v[] = { TilesetSummer, TilesetWinter, TilesetWasteland, TilesetSwamp };

    menu = FindMenu("menu-editor-map-properties");

    MapDescription = menu->items[2].d.input.buffer;
    MapDescription[strlen(MapDescription)-3] = '\0';
    strcpy(TheMap.Info->Description, MapDescription);

    // FIXME: Need to actually change the terrain
    TheMap.Terrain = v[menu->items[6].d.pulldown.curopt];

    // FIXME: Save the pud version somewhere

    EditorEndMenu();
}

local void EditorMapPropertiesCancel(void)
{
    EditorEndMenu();
}

local void EditorPlayerProperties(void)
{
    ProcessMenu("menu-editor-player-properties", 1);
}

local void EditorQuitMenu(void)
{
    EditorRunning=0;
    GameMenuReturn();
}

local void EditorEndMenu(void)
{
    CursorOn = CursorOnUnknown;
    CurrentMenu = NULL;

    MustRedraw = RedrawEverything;
    InterfaceState = IfaceStateNormal;
    EditorUpdateDisplay();
    InterfaceState = IfaceStateMenu;
    MustRedraw = RedrawMenu;
}

/*----------------------------------------------------------------------------
--	Init functions
----------------------------------------------------------------------------*/

/**
**	Move buttons so they're centered on different resolutions
*/
#ifdef OLD_MENU
local void MoveButtons(void)
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
#else
local void MoveButtons(void)
{
    OffsetX = (VideoWidth - 640) / 2;
    OffsetY = (VideoHeight - 480) / 2;
}
#endif

/**
**	Initialize the loaded menu data
*/
global void InitMenuData(void)
{
#ifdef OLD_MENU
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
    InitEditorSelectMenuItems();
    InitEditorLoadMapMenuItems();
    InitEditorMenuItems();
    InitEditorMapPropertiesMenuItems();
    InitEditorPlayerPropertiesMenuItems();
#endif
    InitNetMultiButtonStorage();

    if (VideoWidth != 640) {
	MoveButtons();
    }

#ifdef OLD_MENU
    *(Menu **)hash_add(MenuHash,"menu-game") = Menus + 0;
    *(Menu **)hash_add(MenuHash,"menu-victory") = Menus + 1;
    *(Menu **)hash_add(MenuHash,"menu-defeated") = Menus + 2;
    *(Menu **)hash_add(MenuHash,"menu-select-scenario") = Menus + 3;
    *(Menu **)hash_add(MenuHash,"menu-program-start") = Menus + 4;
    *(Menu **)hash_add(MenuHash,"menu-custom-game") = Menus + 5;
    *(Menu **)hash_add(MenuHash,"menu-enter-name") = Menus + 6;
    *(Menu **)hash_add(MenuHash,"menu-create-join-menu") = Menus + 7;
    *(Menu **)hash_add(MenuHash,"menu-multi-setup") = Menus + 8;
    *(Menu **)hash_add(MenuHash,"menu-enter-server") = Menus + 9;
    *(Menu **)hash_add(MenuHash,"menu-net-multi-client") = Menus + 10;
    *(Menu **)hash_add(MenuHash,"menu-net-connecting") = Menus + 11;
    *(Menu **)hash_add(MenuHash,"menu-campaign-select") = Menus + 12;
    *(Menu **)hash_add(MenuHash,"menu-campaign-continue") = Menus + 13;
    *(Menu **)hash_add(MenuHash,"menu-objectives") = Menus + 14;
    *(Menu **)hash_add(MenuHash,"menu-end-scenario") = Menus + 15;
    *(Menu **)hash_add(MenuHash,"menu-sound-options") = Menus + 16;
    *(Menu **)hash_add(MenuHash,"menu-preferences") = Menus + 17;
    *(Menu **)hash_add(MenuHash,"menu-settings-speed") = Menus + 18;
    *(Menu **)hash_add(MenuHash,"menu-game-options") = Menus + 19;
    *(Menu **)hash_add(MenuHash,"menu-net-error") = Menus + 20;
    *(Menu **)hash_add(MenuHash,"menu-tips") = Menus + 21;
    *(Menu **)hash_add(MenuHash,"menu-help") = Menus + 22;
    *(Menu **)hash_add(MenuHash,"menu-keystroke-help") = Menus + 23;
    *(Menu **)hash_add(MenuHash,"menu-save-game") = Menus + 24;
    *(Menu **)hash_add(MenuHash,"menu-load-game") = Menus + 25;
    *(Menu **)hash_add(MenuHash,"menu-save-confirm") = Menus + 26;
    *(Menu **)hash_add(MenuHash,"menu-delete-confirm") = Menus + 27;
    *(Menu **)hash_add(MenuHash,"menu-editor-select") = Menus + 28;
    *(Menu **)hash_add(MenuHash,"menu-editor-load-map") = Menus + 29;
    *(Menu **)hash_add(MenuHash,"menu-editor") = Menus + 30;
    *(Menu **)hash_add(MenuHash,"menu-editor-map-properties") = Menus + 31;
    *(Menu **)hash_add(MenuHash,"menu-editor-player-properties") = Menus + 32;
#endif
}

/**
**	Post-Initialize the loaded menu functions
*/
global void InitMenuFunctions(void)
{
    Menu *menu;

    //
    //	Autodetect the swamp tileset
    //
#ifdef HAVE_EXPANSION
    strcpy(ScenSelectFullPath, FreeCraftLibPath);
    if (ScenSelectFullPath[0]) {
	strcat(ScenSelectFullPath, "/graphics/tilesets/");
    }
    strcat(ScenSelectFullPath, "swamp");
    menu = FindMenu("menu-custom-game");
    if (access(ScenSelectFullPath, F_OK) != 0) {
	// ARI FIXME: Hack to disable Expansion Gfx..
	// also shows how to add new tilesets....
	// - FIXME2:
	// With new dynamic tileset configuration this
	// should read what siod-config gave us and
	// build the menu from that..
	menu->items[14].d.pulldown.noptions = 4;
    }
#else
    //
    // HACK_MOUSE
    //
    //menu->items[14].d.pulldown.noptions = 4;
    //
#endif

#ifdef OLD_MENU
#ifdef SAVE_MENU_CCL
    {
	FILE *fd=fopen("menus.ccl","w");
	SaveMenus(fd);
	fclose(fd);
    }
#endif
#endif
}

#ifdef OLD_MENU
#ifdef SAVE_MENU_CCL
char *menu_names[] = {
    "menu-game",
    "menu-victory",
    "menu-defeated",
    "menu-select-scenario",
    "menu-program-start",
    "menu-custom-game",
    "menu-enter-name",
    "menu-create-join-menu",
    "menu-multi-setup",
    "menu-enter-server",
    "menu-net-multi-client",
    "menu-net-connecting",
    "menu-campaign-select",
    "menu-campaign-continue",
    "menu-objectives",
    "menu-end-scenario",
    "menu-sound-options",
    "menu-preferences",
    "menu-settings-speed",
    "menu-game-options",
    "menu-net-error",
    "menu-tips",
    "menu-help",
    "menu-keystroke-help",
    "menu-save-game",
    "menu-load-game",
    "menu-save-confirm",
    "menu-delete-confirm",
    "menu-editor-select",
    "menu-editor-load-map",
    "menu-editor",
    "menu-editor-map-properties",
    "menu-editor-player-properties",
};

char *menu_flags[] = {
    "",
    "'flags '(active)",
    "'flags '(clicked)",
    "",
    "'flags '(selected)",
    "",
    "",
    "",
    "'flags '(disabled)",
};

char *images[] = {
    "none",
    "panel1",
    "panel2",
    "panel3",
    "panel4",
    "panel5",
};

char *font_names[] = {
    "small",
    "game",
    "large",
    "smallTitle",
    "largeTitle",
};

char *text_flags[] = {
    "none",
    "center",
    "right",
    "",
    "left",
};

char *gem_state[] = {
    "unchecked",
    "passive",
    "invisible",
    "",
    "checked",
};

local char *hotkey2str(int key, char *buf)
{
    if (!key) {
	buf[0] = '\0';
    } else if (('a' <= key && key <= 'z') || ('A' <= key && key <= 'Z')) {
	buf[0] = key;
	buf[1] = '\0';
    } else if (KeyCodeF1 <= key && key <= KeyCodeF12) {
	buf[0] = 'f';
	if (key >= KeyCodeF10) {
	    buf[1] = '1';
	    buf[2] = key-10-KeyCodeF1+1 + '0';
	    buf[3] = '\0';
	} else {
	    buf[1] = key-KeyCodeF1+1 + '0';
	    buf[2] = '\0';
	}
    } else if (key == '\033') {
	strcpy(buf,"esc");
    }
    return buf;
}

local char *button_style(int style)
{
    switch (style) {
	case MBUTTON_MAIN:
	    return "main";
	case MBUTTON_GM_HALF:
	    return "gm-half";
	case MBUTTON_132:
	    return "132";
	case MBUTTON_GM_FULL:
	    return "gm-full";
	case MBUTTON_GEM_ROUND:
	    return "gem-round";
	case MBUTTON_GEM_SQUARE:
	    return "gem-square";
	case MBUTTON_UP_ARROW:
	    return "up-arrow";
	case MBUTTON_DOWN_ARROW:
	    return "down-arrow";
	case MBUTTON_LEFT_ARROW:
	    return "left-arrow";
	case MBUTTON_RIGHT_ARROW:
	    return "right-arrow";
	case MBUTTON_S_KNOB:
	    return "s-knob";
	case MBUTTON_S_VCONT:
	    return "s-vcont";
	case MBUTTON_S_HCONT:
	    return "s-hcont";
	case MBUTTON_PULLDOWN:
	    return "pulldown";
	case MBUTTON_VTHIN:
	    return "vthin";
	case MBUTTON_FOLDER:
	    return "folder";
    }
    fprintf(stderr,"button_style not found: %d\n", style);
    return "";
}

global void SaveMenus(FILE* file)
{
    Menu *menu;
    int i, j, n;
    int OffsetX, OffsetY;
    char hotkey[10];
    char func[10];
    char func2[10];
    char func3[10];
    char initfunc[40];
    char exitfunc[40];
    char netaction[40];

    OffsetX = (VideoWidth - 640) / 2;
    OffsetY = (VideoHeight - 480) / 2;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; $Id$\n\n");

    for (i=0; i<sizeof(menu_names)/sizeof(*menu_names); ++i) {
	menu = FindMenu(menu_names[i]);
	if (!menu) {
	    abort();
	}
	fprintf(file,";;\n;; %s\n;;\n", menu_names[i]);
	if (menu->netaction) {
	    sprintf(func,"%p",menu->netaction);
	    sprintf(netaction," 'netaction '%s",(char*)hash_find(MenuFuncHash2,func));
	} else {
	    netaction[0] = '\0';
	}
	fprintf(file,"(define-menu 'name \"%s\" 'geometry '(%d %d %d %d)\n"
		     "    'image '%s 'default '%d%s)\n",
		     menu_names[i],
		     menu->x - OffsetX, menu->y - OffsetY,
		     menu->xsize, menu->ysize,
		     images[menu->image],
		     menu->defsel,
		     netaction);
	for (j=0; j<menu->nitems; ++j) {
	    if (menu->items[j].initfunc) {
		sprintf(func,"%p",menu->items[j].initfunc);
		sprintf(initfunc," 'init '%s",(char*)hash_find(MenuFuncHash2,func));
	    } else {
		initfunc[0] = '\0';
	    }
	    if (menu->items[j].exitfunc) {
		sprintf(func,"%p",menu->items[j].exitfunc);
		sprintf(exitfunc," 'exit '%s",(char*)hash_find(MenuFuncHash2,func));
	    } else {
		exitfunc[0] = '\0';
	    }

	    fprintf(file,"(define-menu-item 'pos '(%d %d) 'font '%s %s%s%s\n",
			 menu->items[j].xofs, menu->items[j].yofs,
			 font_names[menu->items[j].font],
			 menu_flags[menu->items[j].flags],
			 initfunc,
			 exitfunc);
	    switch (menu->items[j].mitype) {
		case MI_TYPE_TEXT:
		    fprintf(file,"    'text '(\"%s\" %s)\n",
			    menu->items[j].d.text.text ? (char*)menu->items[j].d.text.text : "null",
			    text_flags[menu->items[j].d.text.tflags]);
		    break;
		case MI_TYPE_BUTTON:
		    sprintf(func,"%p",menu->items[j].d.button.handler);
		    fprintf(file,"    'button '(size (%d %d)\n"
			         "            caption \"%s\"\n"
				 "            hotkey \"%s\"\n"
				 "            func %s\n"
				 "            style %s)\n",
				 menu->items[j].d.button.xsize,
				 menu->items[j].d.button.ysize,
				 menu->items[j].d.button.text,
				 hotkey2str(menu->items[j].d.button.hotkey,hotkey),
				 (char*)hash_find(MenuFuncHash2,func),
				 button_style(menu->items[j].d.button.button));
		    break;
		case MI_TYPE_PULLDOWN:
		    sprintf(func,"%p",menu->items[j].d.pulldown.action);
		    fprintf(file,"    'pulldown '(size (%d %d)\n"
			         "              style %s\n"
				 "              func %s\n",
				 menu->items[j].d.pulldown.xsize,
				 menu->items[j].d.pulldown.ysize,
				 button_style(menu->items[j].d.pulldown.button),
				 (char*)hash_find(MenuFuncHash2,func));
		    fprintf(file,"              options (");
		    for (n=0; n<menu->items[j].d.pulldown.noptions; ++n) {
			fprintf(file,"\"%s\" ", menu->items[j].d.pulldown.options[n]);
		    }

		    fprintf(file,")\n"
				 "              default %d\n"
				 "              current %d)\n",
				 menu->items[j].d.pulldown.defopt,
				 menu->items[j].d.pulldown.curopt);
		    break;
		case MI_TYPE_LISTBOX:
		    sprintf(func,"%p",menu->items[j].d.listbox.action);
		    sprintf(func2,"%p",menu->items[j].d.listbox.retrieveopt);
		    sprintf(func3,"%p",menu->items[j].d.listbox.handler);
		    fprintf(file,"    'listbox '(size (%d %d)\n"
				 "             style %s\n"
				 "             func %s\n"
				 "             retopt %s\n"
				 "             handler %s\n"
				 "             nlines %d)\n",
				 menu->items[j].d.listbox.xsize,
				 menu->items[j].d.listbox.ysize,
				 button_style(menu->items[j].d.listbox.button),
				 (char*)hash_find(MenuFuncHash2,func),
				 (char*)hash_find(MenuFuncHash2,func2),
				 (char*)hash_find(MenuFuncHash2,func3),
				 menu->items[j].d.listbox.nlines);
		    break;
		case MI_TYPE_VSLIDER:
		    sprintf(func,"%p",menu->items[j].d.vslider.action);
		    sprintf(func2,"%p",menu->items[j].d.vslider.handler);
		    fprintf(file,"    'vslider '(size (%d %d)\n"
			         "             func %s\n"
				 "             handler %s)\n",
				 menu->items[j].d.vslider.xsize,
				 menu->items[j].d.vslider.ysize,
				 (char*)hash_find(MenuFuncHash2,func),
				 (char*)hash_find(MenuFuncHash2,func2));
		    break;
		case MI_TYPE_DRAWFUNC:
		    sprintf(func,"%p",menu->items[j].d.drawfunc.draw);
		    fprintf(file,"    'drawfunc '%s\n",
			         (char*)hash_find(MenuFuncHash2,func));
		    break;
		case MI_TYPE_INPUT:
		    sprintf(func,"%p",menu->items[j].d.input.action);
		    fprintf(file,"    'input '(size (%d %d)\n"
			         "           func %s\n"
				 "           style %s)\n",
				 menu->items[j].d.input.xsize,
				 menu->items[j].d.input.ysize,
				 (char*)hash_find(MenuFuncHash2,func),
				 button_style(menu->items[j].d.input.button));
		    break;
		case MI_TYPE_GEM:
		    sprintf(func,"%p",menu->items[j].d.gem.action);
		    fprintf(file,"    'gem '(size (%d %d)\n"
			         "         state %s\n"
				 "         func %s\n"
				 "         style %s)\n",
				 menu->items[j].d.gem.xsize,
				 menu->items[j].d.gem.ysize,
				 gem_state[menu->items[j].d.gem.state],
				 (char*)hash_find(MenuFuncHash2,func),
				 button_style(menu->items[j].d.gem.button));
		    break;
		case MI_TYPE_HSLIDER:
		    sprintf(func,"%p",menu->items[j].d.hslider.action);
		    sprintf(func2,"%p",menu->items[j].d.hslider.handler);
		    fprintf(file,"    'hslider '(size (%d %d)\n"
			         "             func %s\n"
				 "             handler %s)\n",
				 menu->items[j].d.hslider.xsize,
				 menu->items[j].d.hslider.ysize,
				 (char*)hash_find(MenuFuncHash2,func),
				 (char*)hash_find(MenuFuncHash2,func2));
		    break;
		default:
		    abort();
	    }
	    fprintf(file,"    'menu \"%s\")\n",menu_names[i]);
	}
	fprintf(file,"\n\n");
    }
}
#endif /// SAVE_MENU_CCL
#endif

//@}
