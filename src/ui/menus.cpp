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
//      (c) Copyright 1999-2004 by Andreas Arens, Jimmy Salmon, Nehal Mistry
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
--		Includes
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


//#define SAVE_MENU_CCL				/// SAVE (REWRITE!) the menus.ccl file

/*----------------------------------------------------------------------------
--		Prototypes for local functions
----------------------------------------------------------------------------*/

local void EditorEndMenu(void);

/*----------------------------------------------------------------------------
--		Prototypes for action handlers and helper functions
----------------------------------------------------------------------------*/

// Game menu
local void GameMenuInit(Menuitem *mi);
local void GameOptionsMenu(void);
local void HelpMenu(void);
local void ObjectivesMenu(void);
local void EndScenarioMenu(void);

// Objectives
local void ObjectivesInit(Menuitem *mi);
local void ObjectivesExit(Menuitem *mi);

// Victory, lost
local void GameMenuEnd(void);
local void VictoryInit(Menuitem *mi);
local void DefeatedInit(Menuitem *mi);
local void SaveReplay(void);
local void SaveReplayEnterAction(Menuitem *mi, int key);
local void SaveReplayOk(void);

// Scenario select
local void ScenSelectLBExit(Menuitem *mi);
local void ScenSelectLBInit(Menuitem *mi);
local unsigned char *ScenSelectLBRetrieve(Menuitem *mi, int i);
local void ScenSelectLBAction(Menuitem *mi, int i);
local void ScenSelectTPMSAction(Menuitem *mi, int i);
local void ScenSelectVSAction(Menuitem *mi, int i);
local void ScenSelectFolder(void);
local void ScenSelectInit(Menuitem *mi);
local void ScenSelectOk(void);
local void ScenSelectCancel(void);
local int ScenSelectRDFilter(char *pathbuf, FileList *fl);

// Program start
local void PrgStartInit(Menuitem *mi);
local void NameLineDrawFunc(Menuitem *mi);
local void SinglePlayerGameMenu(void);
local void MultiPlayerGameMenu(void);
local void CampaignGameMenu(void);
local void ReplayGameMenu(void);
local void GlobalOptionsMenu(void);
local void StartEditor(void);
local void GameShowCredits(void);
local void GameMenuExit(void);

// Confirm menus
local void SurrenderConfirmMenu(void);

// Global Options
local void GlobalOptionsInit(Menuitem *mi);
local void GlobalOptionsExit(Menuitem *mi);
local void GlobalOptionsResolutionGem(Menuitem *mi);
local void GlobalOptionsFullscreenGem(Menuitem *mi);

// Tips
local void TipsInit(Menuitem *mi);
local void TipsExit(Menuitem *mi);
local void TipsShowTipsGem(Menuitem *mi);
local void TipsShowTipsText(Menuitem *mi);
local void TipsNextTip(void);
local void TipsPreviousTip(void);

// Custom game setup
local void GameSetupInit(Menuitem *mi);
local void ScenSelectMenu(void);
local void CustomGameStart(void);
local void GameCancel(void);
local void GameDrawFunc(Menuitem *mi);
local void GameRCSAction(Menuitem *mi, int i);
local void GameRESAction(Menuitem *mi, int i);
local void GameUNSAction(Menuitem *mi, int i);
local void GameTSSAction(Menuitem *mi, int i);
local void GameGATAction(Menuitem *mi, int i);
local void CustomGameOPSAction(Menuitem *mi, int i);

// Enter name
local void EnterNameAction(Menuitem *mi, int key);
local void EnterNameCancel(void);

// Net create join
local void JoinNetGameMenu(void);
local void CreateNetGameMenu(void);
local void CreateInternetGameMenu(void);

// Multi net type
local void MultiPlayerLANGame(void);
local void MultiPlayerInternetGame(void);

// Net multi setup
local void MultiGameSetupInit(Menuitem *mi);
local void MultiGameSetupExit(Menuitem *mi);
local void MultiGameDrawFunc(Menuitem *mi);
local void MultiScenSelectMenu(void);
local void MultiGameStart(void);
local void MultiGameCancel(void);
local void MultiGameFWSAction(Menuitem *mi, int i);

// Enter server ip
local void EnterServerIPAction(Menuitem *mi, int key);
local void EnterServerIPCancel(void);

// Net multi client
local void TerminateNetConnect(void);
local void MultiGameClientInit(Menuitem *mi);
local void MultiGameClientExit(Menuitem *mi);
local void MultiGameClientDrawFunc(Menuitem *mi);
local void MultiClientReady(void);
local void MultiClientNotReady(void);
local void MultiClientCancel(void);
local void MultiClientRCSAction(Menuitem *mi, int i);
local void MultiClientGemAction(Menuitem *mi);
local void MultiClientUpdate(int initial);

// Net connecting
local void NetConnectingInit(Menuitem *mi);
local void NetConnectingExit(Menuitem *mi);
local void NetConnectingCancel(void);

// Campaign select
local void CampaignMenu1(void);
local void CampaignMenu2(void);
local void CampaignMenu3(void);
local void CampaignMenu4(void);
local void SelectCampaignMenu(void);

// End scenario
local void EndScenarioRestart(void);
local void EndScenarioSurrender(void);
local void EndScenarioQuitMenu(void);

// Sound options
local void SoundOptionsInit(Menuitem *mi);
local void SoundOptionsExit(Menuitem *mi);
local void MasterVolumeHSAction(Menuitem *mi);
local void SetMasterPower(Menuitem *mi);
local void MusicVolumeHSAction(Menuitem *mi);
local void SetMusicPower(Menuitem *mi);
local void CdVolumeHSAction(Menuitem *mi);
local void SetCdPower(Menuitem *mi);
local void SetCdModeDefined(Menuitem *mi);
local void SetCdModeRandom(Menuitem *mi);

// Preferences
local void PreferencesInit(Menuitem *mi);
local void PreferencesExit(Menuitem *mi);
local void SetFogOfWar(Menuitem *mi);
local void SetCommandKey(Menuitem *mi);

// Speed options
local void SpeedOptionsInit(Menuitem *mi);
local void SpeedOptionsExit(Menuitem *mi);
local void GameSpeedHSAction(Menuitem *mi);
local void MouseScrollHSAction(Menuitem *mi);
local void KeyboardScrollHSAction(Menuitem *mi);

// Game options

// Diplomacy options
local void DiplomacyInit(Menuitem *mi);
local void DiplomacyExit(Menuitem *mi);
local void DiplomacyWait(Menuitem *mi);
local void DiplomacyOk(void);

// Help
local void KeystrokeHelpMenu(void);
local void TipsMenu(void);

// Keystroke help
local void KeystrokeHelpVSAction(Menuitem *mi, int i);
local void KeystrokeHelpDrawFunc(Menuitem *mi);

// Save
local void SaveGameInit(Menuitem *mi);
local void SaveGameExit(Menuitem *mi);
local void SaveGameLBInit(Menuitem *mi);
local void SaveGameLBExit(Menuitem *mi);
local void SaveGameEnterAction(Menuitem *mi, int key);
local void SaveGameLBAction(Menuitem *mi, int i);
local unsigned char *SaveGameLBRetrieve(Menuitem *mi, int i);
local void SaveGameVSAction(Menuitem *mi, int i);
local void SaveGameOk(void);
local void DeleteConfirmMenu(void);
local int SaveGameRDFilter(char *pathbuf, FileList *fl);
local void CreateSaveDir(void);

// Load
local void LoadGameInit(Menuitem *mi);
local void LoadGameExit(Menuitem *mi);
local void LoadGameLBInit(Menuitem *mi);
local void LoadGameLBExit(Menuitem *mi);
local void LoadGameLBAction(Menuitem *mi, int i);
local unsigned char *LoadGameLBRetrieve(Menuitem *mi, int i);
local void LoadGameVSAction(Menuitem *mi, int i);
local void LoadGameOk(void);

// Confirm save
local void SaveConfirmInit(Menuitem *mi);
local void SaveConfirmExit(Menuitem *mi);
local void SaveConfirmOk(void);
local void SaveConfirmCancel(void);

// Confirm delete
local void DeleteConfirmInit(Menuitem *mi);
local void DeleteConfirmExit(Menuitem *mi);
local void DeleteConfirmOk(void);
local void DeleteConfirmCancel(void);

// Save replay
local void SaveReplayInit(Menuitem *mi);
local void SaveReplayExit(Menuitem *mi);

// Editor select
local void EditorNewMap(void);
local void EditorMainLoadMap(void);
local void EditorSelectCancel(void);

// Editor new
local void EditorNewDrawFunc(Menuitem *mi);
local void EditorNewMapDescriptionEnterAction(Menuitem *mi, int key);
local void EditorNewMapSizeEnterAction(Menuitem *mi, int key);
local void EditorNewOk(void);
local void EditorNewCancel(void);

// Editor main load map
local void EditorMainLoadInit(Menuitem *mi);
local void EditorMainLoadLBInit(Menuitem *mi);
local void EditorMainLoadLBExit(Menuitem *mi);
local void EditorMainLoadLBAction(Menuitem *mi, int i);
local unsigned char *EditorMainLoadLBRetrieve(Menuitem *mi, int i);
local void EditorMainLoadVSAction(Menuitem *mi, int i);
local void EditorMainLoadOk(void);
local void EditorMainLoadCancel(void);
local void EditorMainLoadFolder(void);
local int EditorMainLoadRDFilter(char *pathbuf, FileList *fl);

// Editor load map
local void EditorLoadOk(void);
local void EditorLoadCancel(void);

// Editor menu
local void EditorMapPropertiesMenu(void);
local void EditorPlayerPropertiesMenu(void);
local void EditorQuitToMenu(void);

// Editor map properties
local void EditorMapPropertiesEnterAction(Menuitem *mi, int key);
local void EditorMapPropertiesOk(void);

// Editor player properties
local void EditorPlayerPropertiesDrawFunc(Menuitem *mi);
local void EditorPlayerPropertiesEnterAction(Menuitem *mi, int key);

// Editor edit resource
local void EditorEditResourceEnterAction(Menuitem *mi,int key);
local void EditorEditResourceOk(void);
local void EditorEditResourceCancel(void);

// Editor edit ai properties
local void EditorEditAiPropertiesGem(Menuitem *mi);
local void EditorEditAiPropertiesOk(void);
local void EditorEditAiPropertiesCancel(void);

// Editor save
local void EditorSaveLBInit(Menuitem *mi);
local void EditorSaveLBExit(Menuitem *mi);
local void EditorSaveFolder(void);
local void EditorSaveLBAction(Menuitem *mi, int i);
local unsigned char *EditorSaveLBRetrieve(Menuitem *mi, int i);
local void EditorSaveVSAction(Menuitem *mi, int i);
local void EditorSaveEnterAction(Menuitem *mi, int key);
local void EditorSaveOk(void);
local void EditorSaveCancel(void);
local int EditorSaveRDFilter(char *pathbuf, FileList *fl);

// Editor save confirm
local void EditorSaveConfirmInit(Menuitem *mi);
local void EditorSaveConfirmOk(void);
local void EditorSaveConfirmCancel(void);

// Replay game
local void ReplayGameInit(Menuitem *mi);
local void ReplayGameLBInit(Menuitem *mi);
local void ReplayGameLBExit(Menuitem *mi);
local void ReplayGameLBAction(Menuitem *mi, int i);
local unsigned char *ReplayGameLBRetrieve(Menuitem *mi, int i);
local void ReplayGameVSAction(Menuitem *mi, int i);
local void ReplayGameFolder(void);
local void ReplayGameDisableFog(Menuitem *mi);
local void ReplayGameOk(void);
local void ReplayGameCancel(void);
local int ReplayGameRDFilter(char *pathbuf, FileList *fl);

// Metaserver
local void MultiGameMasterReport(void);
local void EnterMasterAction(Menuitem *mi, int key);
local void ShowMetaServerList(void); // Addition for Magnant
local void MultiMetaServerGameSetupInit(Menuitem *mi); // init callback
local void MultiMetaServerGameSetupExit(Menuitem *mi); // exit callback
local void SelectGameServer(Menuitem *mi); // Game Server selection so that client joins the game
local void AddGameServer(void); //Add Game Server on Meta server
local void ChangeGameServer(void); //Change Game Parameters on Meta server
local int MetaServerConnectError(void); // Display error message
local void MultiMetaServerClose(void); //Close Master Server connection
//others
local void GameMenuReturn(void);
local void NetErrorMenu(char *error);
local void NetworkGamePrepareGameSettings(void);
local void MultiGamePTSAction(Menuitem *mi, int o);
local void NetMultiPlayerDrawFunc(Menuitem *mi);
local void MultiGamePlayerSelectorsUpdate(int initial);


/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

	/// Hash table of all the menus
global _MenuHash MenuHash;
	/// Hash table of all the menu functions
global _MenuFuncHash MenuFuncHash;

#define HASHADD(x,y) { \
	*(void **)hash_add(MenuFuncHash,(y)) = (void *)(x); \
}

	/// Game started
global int GuiGameStarted;
	/// Editor cancel button pressed
local int EditorCancelled;

/**
**		Other client and server selection state for Multiplayer clients
*/
global ServerSetup ServerSetupState;
global ServerSetup LocalSetupState;

local char ScenSelectPath[1024];				/// Scenario selector path
local char ScenSelectDisplayPath[1024];				/// Displayed selector path
local char ScenSelectFileName[128];				/// Scenario selector name

global MapInfo *MenuMapInfo;						/// Selected map info
global char MenuMapFullPath[1024];				/// Selected map path+name

local char *SaveDir;								/// Save game directory
local char TempPathBuf[PATH_MAX];				/// Temporary buffer for paths

global int nKeyStrokeHelps;						/// Number of keystroke help lines
global char **KeyStrokeHelps;						/// Array of keystroke help lines

/// FIXME: -> ccl...
local unsigned char *mgptsoptions[] = {
	"Available",
	"Computer",
	"Closed",
};

/**
**		Help-items for the Net Multiplayer Setup and Client Menus
*/
local Menuitem NetMultiButtonStorage[] = {
		{ MI_TYPE_PULLDOWN, 40, 32, 0, GameFont, 0, NULL, NULL, NULL, {{NULL,0}} },
		{ MI_TYPE_DRAWFUNC, 40, 32, 0, GameFont, 0, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitNetMultiButtonStorage(void) {
	MenuitemPulldown i0 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, MultiGamePTSAction, 3, -1, 0, 0, 0};
	MenuitemDrawfunc i1 = { NetMultiPlayerDrawFunc };
	NetMultiButtonStorage[0].d.pulldown = i0;
	NetMultiButtonStorage[1].d.drawfunc = i1;
}

#include "menu_defs.inc"

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Initialize the hash table of menu functions
*/
global void InitMenuFuncHash(void)
{
	HASHADD(NULL,"null");

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

// Objectives
	HASHADD(ObjectivesInit,"objectives-init");
	HASHADD(ObjectivesExit,"objectives-exit");

// Victory, lost
	HASHADD(GameMenuEnd,"game-menu-end");
	HASHADD(VictoryInit,"victory-init");
	HASHADD(DefeatedInit,"defeated-init");
	HASHADD(SaveReplay,"save-replay");
	HASHADD(SaveReplayEnterAction,"save-replay-enter-action");
	HASHADD(SaveReplayOk,"save-replay-ok");

// Scenario select
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
	HASHADD(GlobalOptionsResolutionGem,"global-options-resolution-gem");
	HASHADD(GlobalOptionsFullscreenGem,"global-options-fullscreen-gem");

// Tips
	HASHADD(TipsInit,"tips-init");
	HASHADD(TipsExit,"tips-exit");
	HASHADD(TipsShowTipsGem,"tips-show-tips-gem");
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
	HASHADD(MultiClientGemAction,"multi-client-gem-action");

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
	HASHADD(SaveGameLBInit,"save-game-lb-init");
	HASHADD(SaveGameLBExit,"save-game-lb-exit");
	HASHADD(SaveGameEnterAction,"save-game-enter-action");
	HASHADD(SaveGameLBAction,"save-game-lb-action");
	HASHADD(SaveGameLBRetrieve,"save-game-lb-retrieve");
	HASHADD(SaveGameVSAction,"save-game-vs-action");
	HASHADD(SaveGameOk,"save-game-ok");
	HASHADD(DeleteConfirmMenu,"delete-confirm-menu");

// Load
	HASHADD(LoadGameInit,"load-game-init");
	HASHADD(LoadGameExit,"load-game-exit");
	HASHADD(LoadGameLBInit,"load-game-lb-init");
	HASHADD(LoadGameLBExit,"load-game-lb-exit");
	HASHADD(LoadGameLBAction,"load-game-lb-action");
	HASHADD(LoadGameLBRetrieve,"load-game-lb-retrieve");
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
	HASHADD(EditorMainLoadLBInit,"editor-main-load-lb-init");
	HASHADD(EditorMainLoadLBExit,"editor-main-load-lb-exit");
	HASHADD(EditorMainLoadLBAction,"editor-main-load-lb-action");
	HASHADD(EditorMainLoadLBRetrieve,"editor-main-load-lb-retrieve");
	HASHADD(EditorMainLoadVSAction,"editor-main-load-vs-action");
	HASHADD(EditorMainLoadOk,"editor-main-load-ok");
	HASHADD(EditorMainLoadCancel,"editor-main-load-cancel");
	HASHADD(EditorMainLoadFolder,"editor-main-load-folder");

// Editor load map
	HASHADD(EditorMainLoadInit,"editor-load-init");
	HASHADD(EditorMainLoadLBInit,"editor-load-lb-init");
	HASHADD(EditorMainLoadLBExit,"editor-load-lb-exit");
	HASHADD(EditorMainLoadLBAction,"editor-load-lb-action");
	HASHADD(EditorMainLoadLBRetrieve,"editor-load-lb-retrieve");
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
	HASHADD(EditorEditAiPropertiesGem,"editor-edit-ai-properties-gem");
	HASHADD(EditorEditAiPropertiesOk,"editor-edit-ai-properties-ok");
	HASHADD(EditorEditAiPropertiesCancel,"editor-edit-ai-properties-cancel");

// Editor save
	HASHADD(EditorSaveLBInit,"editor-save-lb-init");
	HASHADD(EditorSaveLBExit,"editor-save-lb-exit");
	HASHADD(EditorSaveFolder,"editor-save-folder");
	HASHADD(EditorSaveLBAction,"editor-save-lb-action");
	HASHADD(EditorSaveLBRetrieve,"editor-save-lb-retrieve");
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
	HASHADD(ReplayGameLBInit,"replay-game-lb-init");
	HASHADD(ReplayGameLBExit,"replay-game-lb-exit");
	HASHADD(ReplayGameLBAction,"replay-game-lb-action");
	HASHADD(ReplayGameLBRetrieve,"replay-game-lb-retrieve");
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
--		Button action handler and Init/Exit functions
----------------------------------------------------------------------------*/

/**
**		Draw the version and copyright at bottom of the screen.
**		Also include now the license.
*/
local void NameLineDrawFunc(Menuitem * mi __attribute__ ((unused)))
{
	char* nc;
	char* rc;

	GetDefaultTextColors(&nc, &rc);
	MenusSetBackground();
	SetDefaultTextColors(rc, rc);

	if (SoundFildes == -1 && !SoundOff) {
		VideoDrawText(16, 16, LargeFont, "Sound disabled, please check!");
	}

	VideoDrawTextCentered(VideoWidth/2, TheUI.Offset480Y + 440, GameFont, NameLine);
	VideoDrawTextCentered(VideoWidth/2, TheUI.Offset480Y + 456, GameFont,
		"Engine distributed under the terms of the GNU General Public License.");
	SetDefaultTextColors(nc, rc);
}

/**
**		Start menu master init.
**
**		@param mi		The menu.
*/
local void PrgStartInit(Menuitem *mi)
{
	if (NetworkNumInterfaces == 0) {
		mi[2].flags = MenuButtonDisabled;
	} else {
		mi[2].flags = 0;
	}
}

/**
**		Leave menus and return to game mode
*/
local void GameMenuReturn(void)
{
	while (CurrentMenu) {
		EndMenu();
	}
	InterfaceState = IfaceStateNormal;
	ClearStatusLine();
	GamePaused = 0;
}

/**
**		Init callback for save game menu
*/
local void SaveGameInit(Menuitem *mi __attribute__ ((unused)))
{
	Menu *menu;
	char *buf;

	menu = CurrentMenu;

	buf = malloc(64);
	strcpy(buf, "~!_");
	menu->Items[1].d.input.buffer = buf;
	menu->Items[1].d.input.nch = 0;
	menu->Items[1].d.input.maxch = 60;

	menu->Items[4].flags = MenuButtonDisabled;
	menu->Items[5].flags = MenuButtonDisabled;
	CreateSaveDir();
}

/**
**		Exit callback for save game menu
*/
local void SaveGameExit(Menuitem *mi)
{
	free(mi->menu->Items[1].d.input.buffer);
	mi->menu->Items[1].d.input.buffer = NULL;
}

/**
**		Save game input box callback
*/
local void SaveGameEnterAction(Menuitem *mi, int key)
{
	if (mi->d.input.nch == 0) {
		mi[3].flags = MenuButtonDisabled;		/* mi[i]: Save button! */
	} else {
		mi[3].flags &= ~MenuButtonDisabled;
		if (key == 10 || key == 13) {
			SaveGameOk();
			return;
		}
	}
	mi->menu->Items[5].flags = MenuButtonDisabled;
}

/**
**		Save game
*/
local void SaveGameOk(void)
{
	char *name;
	Menu *menu;
	size_t nameLength;

	menu = CurrentMenu;
	name = menu->Items[1].d.input.buffer;

	nameLength = strlen(name) - 3;
	if (nameLength != 0) {
		strcpy(TempPathBuf, SaveDir);
		strcat(TempPathBuf, "/");
		strncat(TempPathBuf, name, nameLength);

		// Strip .gz extension.
		if (!strcmp(TempPathBuf + strlen(TempPathBuf)-3, ".gz")) {
			TempPathBuf[strlen(TempPathBuf) - 3]=0;
		}
		// Strip .bz2 extension.
		if (!strcmp(TempPathBuf + strlen(TempPathBuf) - 4,".bz2")) {
			TempPathBuf[strlen(TempPathBuf) - 4]=0;
		}
		// Add .sav if not already there.
		if (strcmp(TempPathBuf + strlen(TempPathBuf) - 4,".sav")) {
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
**		Create the save directory
*/
local void CreateSaveDir(void)
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
**		Save game menu
*/
global void SaveGameMenu(void)
{
	ProcessMenu("menu-save-game", 0);
}

/**
**		Exit callback for listbox in save game menu
*/
local void SaveGameLBExit(Menuitem *mi)
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

/**
**		Init callback for listbox in save game menu
*/
local void SaveGameLBInit(Menuitem *mi)
{
	int i;

	SaveGameLBExit(mi);

	i = mi->d.listbox.noptions = ReadDataDirectory(SaveDir,
			NULL, (FileList **) & (mi->d.listbox.options));
	if (i != 0) {
		if (i > mi->d.listbox.nlines) {
			mi->menu->Items[3].flags = MI_ENABLED;
		} else {
			mi->menu->Items[3].flags = MI_DISABLED;
		}
	}
	mi->d.listbox.curopt = -1;
}

/**
**		Save game listbox retrieve callback
*/
local unsigned char *SaveGameLBRetrieve(Menuitem *mi, int i)
{
	FileList *fl;
	static char buffer[1024];

	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (fl[i].type) {
			strcpy(buffer, "   ");
		} else {
			strcpy(buffer, "\260 ");
		}
		strcat(buffer, fl[i].name);
		return buffer;
	}
	return NULL;
}

/**
**		Save game listbox action callback
*/
local void SaveGameLBAction(Menuitem *mi, int i)
{
	FileList *fl;

	DebugCheck(i<0);
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (mi->d.listbox.noptions > mi->d.listbox.nlines) {
			mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
			mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
		}
		if (fl[i].type) {
			sprintf(mi->menu->Items[1].d.input.buffer, "%s~!_", fl[i].name);
			mi->menu->Items[1].d.input.nch = strlen(mi->menu->Items[1].d.input.buffer) - 3;
			mi->menu->Items[4].flags = MI_ENABLED;
			mi->menu->Items[5].flags = MI_ENABLED;
		} else {
			mi->menu->Items[4].flags = MenuButtonDisabled;
			mi->menu->Items[5].flags = MenuButtonDisabled;
		}
	}
}

/**
**		Save game vertical slider callback
*/
local void SaveGameVSAction(Menuitem *mi, int i)
{
	int op;
	int d1;
	int d2;

	mi--;
	switch (i) {
		case 0:				// click - down
		case 2:				// key - down
			if (mi->d.listbox.curopt == -1)
			mi->d.listbox.curopt = 0;

			if (mi[1].d.vslider.cflags&MI_CFLAGS_DOWN) {
				if (mi->d.listbox.curopt+mi->d.listbox.startline + 1 < mi->d.listbox.noptions) {
					mi->d.listbox.curopt++;
					if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
						mi->d.listbox.curopt--;
						mi->d.listbox.startline++;
					}
				}
			} else if (mi[1].d.vslider.cflags&MI_CFLAGS_UP) {
				if (mi->d.listbox.curopt + mi->d.listbox.startline > 0) {
					mi->d.listbox.curopt--;
					if (mi->d.listbox.curopt < 0) {
						mi->d.listbox.curopt++;
						mi->d.listbox.startline--;
					}
				}
			}
			SaveGameLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			if (i == 2) {
				mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
			}
			break;
		case 1:				// mouse - move
			if (mi[1].d.vslider.cflags & MI_CFLAGS_KNOB && (mi[1].flags & MenuButtonClicked)) {
				if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt+mi->d.listbox.startline + 1 < mi->d.listbox.noptions) {
						for (;;) {
							op = ((mi->d.listbox.curopt + mi->d.listbox.startline + 1) * 100) /
								 (mi->d.listbox.noptions - 1);
							d1 = mi[1].d.vslider.curper - mi[1].d.vslider.percent;
							d2 = op - mi[1].d.vslider.curper;
							if (d2 >= d1) {
								break;
							}
							mi->d.listbox.curopt++;
							if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
								mi->d.listbox.curopt--;
								mi->d.listbox.startline++;
							}
							if (mi->d.listbox.curopt+mi->d.listbox.startline + 1 == mi->d.listbox.noptions) {
								break;
							}
						}
					}
				} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
						for (;;) {
							op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
									 (mi->d.listbox.noptions - 1);
							d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
							d2 = mi[1].d.vslider.curper - op;
							if (d2 >= d1) {
								break;
							}
							mi->d.listbox.curopt--;
							if (mi->d.listbox.curopt < 0) {
								mi->d.listbox.curopt++;
								mi->d.listbox.startline--;
							}
							if (mi->d.listbox.curopt+mi->d.listbox.startline == 0) {
								break;
							}
						}
					}
				}

				DebugCheck(mi->d.listbox.startline < 0);
				DebugCheck(mi->d.listbox.noptions > 0 &&
					mi->d.listbox.startline+mi->d.listbox.curopt >= mi->d.listbox.noptions);

				SaveGameLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			}
			break;
		default:
			break;
	}
}

/**
**		Save game read directory filter
*/
local int SaveGameRDFilter(char *pathbuf, FileList *fl)
{
	char *suf;
	char *cp;
	char *fsuffix;
	char *np;

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
			fl->type = 'z';
		}
#endif
#ifdef USE_BZ2LIB
		if (strcmp(cp, ".bz2") == 0) {
			fl->type = 'b';
		}
#endif
		if (strcasestr(pathbuf, ".sav")) {
			if (fl->type == -1) {
				fl->type = 'n';
			}
			fl->name = strdup(np);
			fl->xdata = NULL;
			return 1;
		}
	}
	return 0;
}

/**
**		Load game init callback
*/
local void LoadGameInit(Menuitem *mi)
{
	mi->menu->Items[3].flags = MI_DISABLED;
	CreateSaveDir();
}

/**
**		Load game exit callback
*/
local void LoadGameExit(Menuitem *mi __attribute__ ((unused)))
{
}

/**
**		Exit callback for listbox in load game menu
*/
local void LoadGameLBExit(Menuitem *mi)
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

/**
**		Init callback for listbox in load game menu
*/
local void LoadGameLBInit(Menuitem *mi)
{
	int i;

	LoadGameLBExit(mi);

	i = mi->d.listbox.noptions = ReadDataDirectory(SaveDir, SaveGameRDFilter,
													 (FileList **)&(mi->d.listbox.options));
	if (i != 0) {
		if (i > mi->d.listbox.nlines) {
			mi->menu->Items[2].flags = MenuButtonSelected;
		} else {
			mi->menu->Items[2].flags = MenuButtonDisabled;
		}
	}
	mi->d.listbox.curopt = -1;
}

/**
**		Load game listbox retrieve callback
*/
local unsigned char *LoadGameLBRetrieve(Menuitem *mi, int i)
{
	FileList *fl;
	static char buffer[1024];

	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (fl[i].type) {
			strcpy(buffer, "   ");
		} else {
			strcpy(buffer, "\260 ");
		}
		strcat(buffer, fl[i].name);
		return buffer;
	}
	return NULL;
}

/**
**		Load game listbox action callback
*/
local void LoadGameLBAction(Menuitem *mi, int i)
{
	FileList *fl;

	DebugCheck(i<0);
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (mi->d.listbox.noptions > mi->d.listbox.nlines) {
			mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
			mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
		}
		if (fl[i].type) {
			mi->menu->Items[3].flags = MI_ENABLED;
		} else {
			mi->menu->Items[3].flags = MI_DISABLED;
		}
	} else {
		mi->menu->Items[3].flags = MI_DISABLED;
	}
}

/**
**		Load game vertical slider callback
*/
local void LoadGameVSAction(Menuitem *mi, int i)
{
	int op;
	int d1;
	int d2;

	mi--;
	switch (i) {
		case 0:				// click - down
		case 2:				// key - down
			if (mi->d.listbox.curopt == -1)
			mi->d.listbox.curopt = 0;

			if (mi[1].d.vslider.cflags & MI_CFLAGS_DOWN) {
				if (mi->d.listbox.curopt + mi->d.listbox.startline + 1 < mi->d.listbox.noptions) {
					mi->d.listbox.curopt++;
					if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
						mi->d.listbox.curopt--;
						mi->d.listbox.startline++;
					}
				}
			} else if (mi[1].d.vslider.cflags & MI_CFLAGS_UP) {
				if (mi->d.listbox.curopt + mi->d.listbox.startline > 0) {
					mi->d.listbox.curopt--;
					if (mi->d.listbox.curopt < 0) {
						mi->d.listbox.curopt++;
						mi->d.listbox.startline--;
					}
				}
			}
			LoadGameLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			if (i == 2) {
				mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
			}
			break;
		case 1:				// mouse - move
			if (mi[1].d.vslider.cflags & MI_CFLAGS_KNOB && (mi[1].flags & MenuButtonClicked)) {
				if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt  +mi->d.listbox.startline + 1 < mi->d.listbox.noptions) {
						for (;;) {
							op = ((mi->d.listbox.curopt + mi->d.listbox.startline + 1) * 100) /
								 (mi->d.listbox.noptions - 1);
							d1 = mi[1].d.vslider.curper - mi[1].d.vslider.percent;
							d2 = op - mi[1].d.vslider.curper;
							if (d2 >= d1) {
								break;
							}
							mi->d.listbox.curopt++;
							if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
								mi->d.listbox.curopt--;
								mi->d.listbox.startline++;
							}
							if (mi->d.listbox.curopt+mi->d.listbox.startline+1 == mi->d.listbox.noptions) {
								break;
							}
						}
					}
				} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt + mi->d.listbox.startline > 0) {
						for (;;) {
							op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
									 (mi->d.listbox.noptions - 1);
							d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
							d2 = mi[1].d.vslider.curper - op;
							if (d2 >= d1) {
								break;
							}
							mi->d.listbox.curopt--;
							if (mi->d.listbox.curopt < 0) {
								mi->d.listbox.curopt++;
								mi->d.listbox.startline--;
							}
							if (mi->d.listbox.curopt + mi->d.listbox.startline == 0) {
								break;
							}
						}
					}
				}

				DebugCheck(mi->d.listbox.startline < 0);
				DebugCheck(mi->d.listbox.noptions > 0 &&
					mi->d.listbox.startline+mi->d.listbox.curopt >= mi->d.listbox.noptions);

				LoadGameLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			}
			break;
		default:
			break;
	}
}

/**
**		Load game ok button callback
*/
local void LoadGameOk(void)
{
	FileList *fl;
	Menuitem *mi;
	Menu *menu;
	int i;

	menu = CurrentMenu;
	mi = &menu->Items[1];
	i = mi->d.listbox.curopt + mi->d.listbox.startline;
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		sprintf(TempPathBuf, "%s/%s", SaveDir, fl[i].name);
		SaveGameLoading = 1;
		LoadGame(TempPathBuf);
		Callbacks = &GameCallbacks;
		SetMessage("Loaded game: %s", TempPathBuf);
		GuiGameStarted = 1;
		GameMenuReturn();
	}
}

/**
**		Save confirm init callback
*/
local void SaveConfirmInit(Menuitem *mi)
{
	static char name[PATH_MAX];				// FIXME: much memory wasted
	int fileLength;
	Menu *menu;

	menu = FindMenu("menu-save-game");
	fileLength = strlen(menu->Items[1].d.input.buffer) - 3;

	strcpy(name, "the file: ");
	strncat(name, menu->Items[1].d.input.buffer, fileLength);
	if (strstr(name, ".sav") == NULL) {
		strcat(name, ".sav");
	}
	mi->menu->Items[2].d.text.text = name;
}

/**
**		Save confirm exit callback
*/
local void SaveConfirmExit(Menuitem *mi)
{
	mi->menu->Items[2].d.text.text = NULL;
}

/**
**		Save confirm ok button
*/
local void SaveConfirmOk(void)
{
	int fileLength;
	Menu *menu;

	menu = FindMenu("menu-save-game");
	fileLength = strlen(menu->Items[1].d.input.buffer) - 3;

	strcpy(TempPathBuf, SaveDir);
	strcat(TempPathBuf, "/");
	strncat(TempPathBuf, menu->Items[1].d.input.buffer, fileLength);
	if (strstr(TempPathBuf, ".sav") == NULL) {
		strcat(TempPathBuf, ".sav");
	}
	SaveGame(TempPathBuf);
	SetMessage("Saved game to: %s", TempPathBuf);
	GameMenuReturn();
}

/**
**		Save confirm cancel button
*/
local void SaveConfirmCancel(void)
{
	EndMenu();
}

/**
**		Delete menu
*/
local void DeleteConfirmMenu(void)
{
	ProcessMenu("menu-delete-confirm", 0);
}

/**
**		Init callback for delete confirm menu
*/
local void DeleteConfirmInit(Menuitem *mi)
{
	Menu *menu;
	static char name[PATH_MAX];				// FIXME: much memory wasted

	menu = FindMenu("menu-save-game");
	strcpy(name, "the file: ");
	strcat(name, menu->Items[1].d.input.buffer);
	name[strlen(name) - 3] = '\0';
	mi->menu->Items[2].d.text.text = name;
}

/**
**		Exit callback for delete confirm menu
*/
local void DeleteConfirmExit(Menuitem *mi)
{
	mi->menu->Items[2].d.text.text = NULL;
}

/**
**		Delete confirm ok button
*/
local void DeleteConfirmOk(void)
{
	Menu *menu;

	menu = FindMenu("menu-save-game");
	strcpy(TempPathBuf, SaveDir);
	strcat(TempPathBuf, "/");
	strcat(TempPathBuf, menu->Items[1].d.input.buffer);
	TempPathBuf[strlen(TempPathBuf) - 3] = '\0';
	unlink(TempPathBuf);
	EndMenu();

	// Update list of files and clear input
	SaveGameLBInit(&CurrentMenu->Items[2]);
	strcpy(CurrentMenu->Items[1].d.input.buffer, "~!_");
	CurrentMenu->Items[1].d.input.nch = 0;
}

/**
**		Delete confirm cancel button
*/
local void DeleteConfirmCancel(void)
{
	EndMenu();
}

/**
**		Load game menu
*/
global void LoadGameMenu(void)
{
	ProcessMenu("menu-load-game", 0);
}

/**
**		Init callback for game menu
*/
local void GameMenuInit(Menuitem *mi __attribute__((unused)))
{
	// Disable save menu in multiplayer and replays
	if (IsNetworkGame() || ReplayGameType != ReplayNone) {
		mi->menu->Items[1].flags |= MenuButtonDisabled;
	} else {
		mi->menu->Items[1].flags &= ~MenuButtonDisabled;
	}

	// Disable load menu in multiplayer
	if (IsNetworkGame()) {
		mi->menu->Items[2].flags |= MenuButtonDisabled;
	} else {
		mi->menu->Items[2].flags &= ~MenuButtonDisabled;
	}
}

/**
**		Sound options menu
*/
global void SoundOptionsMenu(void)
{
	ProcessMenu("menu-sound-options", 0);
}

/**
**		Init callback for sound options menu
*/
local void SoundOptionsInit(Menuitem *mi __attribute__((unused)))
{
	Menu *menu;

	menu = FindMenu("menu-sound-options");

	// master volume slider
	if (SoundFildes == -1) {
		menu->Items[2].flags = MenuButtonDisabled;
	} else {
		menu->Items[2].flags = 0;
		menu->Items[2].d.hslider.percent = (GlobalVolume * 100) / 255;
	}

	// master power
	if (SoundFildes == -1) {
		menu->Items[5].d.gem.state = MI_GSTATE_UNCHECKED;
	} else {
		menu->Items[5].d.gem.state = MI_GSTATE_CHECKED;
	}

	// music volume slider
	if (PlayingMusic != 1 || SoundFildes == -1) {
		menu->Items[7].flags = MenuButtonDisabled;
	} else {
		menu->Items[7].flags = 0;
		menu->Items[7].d.hslider.percent = (MusicVolume * 100) / 255;
	}

	// music power
	if (SoundFildes == -1) {
		menu->Items[10].flags = MenuButtonDisabled;
	} else {
		menu->Items[10].flags = 0;
	}
#ifdef USE_CDAUDIO
	if (CDMode != CDModeStopped && CDMode != CDModeOff) {
		menu->Items[7].flags = MenuButtonDisabled;
		menu->Items[10].flags = MenuButtonDisabled;
	}
#endif
	if (PlayingMusic != 1 || SoundFildes == -1) {
		menu->Items[10].d.gem.state = MI_GSTATE_UNCHECKED;
	} else {
		menu->Items[10].d.gem.state = MI_GSTATE_CHECKED;
	}

	menu->Items[12].flags = MenuButtonDisabled;				// cd volume slider
	menu->Items[15].flags = MenuButtonDisabled;				// cd power
	menu->Items[15].d.gem.state = MI_GSTATE_UNCHECKED;
	menu->Items[16].flags = MenuButtonDisabled;				// all tracks button
	menu->Items[17].flags = MenuButtonDisabled;				// random tracks button
#ifdef USE_CDAUDIO
	menu->Items[15].flags = 0;						// cd power
	if (CDMode != CDModeStopped && CDMode != CDModeOff) {
#if (!defined(USE_WIN32) && defined(USE_LIBCDA)) || defined(USE_CDDA)
		int i = 0;
		i = GetCDVolume();

		menu->Items[12].flags = 0;
		menu->Items[12].d.hslider.percent = (i * 100) / 255;
#endif
		menu->Items[15].d.gem.state = MI_GSTATE_CHECKED;
		menu->Items[16].flags = 0;
		menu->Items[17].flags = 0;

		if (CDMode == CDModeDefined) {
			menu->Items[16].d.gem.state = MI_GSTATE_CHECKED;
			menu->Items[17].d.gem.state = MI_GSTATE_UNCHECKED;
		} else if (CDMode == CDModeRandom) {
			menu->Items[16].d.gem.state = MI_GSTATE_UNCHECKED;
			menu->Items[17].d.gem.state = MI_GSTATE_CHECKED;
		}
	}
#endif // cd
}

/**
**		Exit callback for sound options menu
*/
local void SoundOptionsExit(Menuitem *mi __attribute__((unused)))
{
	// FIXME: Only save if something changed
	SavePreferences();
}

/**
**		Global options menu
*/
local void GlobalOptionsMenu(void)
{
	ProcessMenu("menu-global-options", 1);
}

/**
**		Init callback for global options menu
*/
local void GlobalOptionsInit(Menuitem *mi __attribute__((unused)))
{
	Menu *menu;

	menu = CurrentMenu;

	menu->Items[2].d.gem.state = MI_GSTATE_UNCHECKED;
	menu->Items[3].d.gem.state = MI_GSTATE_UNCHECKED;
	menu->Items[4].d.gem.state = MI_GSTATE_UNCHECKED;
	menu->Items[5].d.gem.state = MI_GSTATE_UNCHECKED;
	menu->Items[6].d.gem.state = MI_GSTATE_UNCHECKED;
	menu->Items[7].d.gem.state = MI_GSTATE_UNCHECKED;

	if (VideoWidth == 640) {
		menu->Items[2].d.gem.state = MI_GSTATE_CHECKED;
	} else if (VideoWidth == 800) {
		menu->Items[3].d.gem.state = MI_GSTATE_CHECKED;
	} else if (VideoWidth == 1024) {
		menu->Items[4].d.gem.state = MI_GSTATE_CHECKED;
	} else if (VideoWidth == 1280) {
		menu->Items[5].d.gem.state = MI_GSTATE_CHECKED;
	} else if (VideoWidth == 1600) {
		menu->Items[6].d.gem.state = MI_GSTATE_CHECKED;
	}

	if (VideoFullScreen) {
		menu->Items[7].d.gem.state = MI_GSTATE_CHECKED;
	}
}

/**
**		Exit callback for global options menu
*/
local void GlobalOptionsExit(Menuitem *mi __attribute__((unused)))
{
	// FIXME: Only save if something changed
	SavePreferences();
}

/**
**		Global options resolution gem callback
*/
local void GlobalOptionsResolutionGem(Menuitem *mi)
{
	int res;

	res = VideoWidth;
	switch (mi - mi->menu->Items) {
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
		Menu *menu;
		int i;

		VideoWidth = res;
		VideoHeight = res * 3 / 4;
		SavePreferences();
		ExitMenus();
		InitVideo();
		// Force Update Background Size
		SetClipping(0,0,VideoWidth - 1,VideoHeight - 1);
		MenusSetBackground();
		Invalidate();
		CleanModules();
		LoadCcl();
		PreMenuSetup();
		GameCursor = TheUI.Point.Cursor;
		menu = FindMenu("menu-program-start");
		for (i=0; i<menu->NumItems; ++i) {
			if (menu->Items[i].initfunc) {
				(*menu->Items[i].initfunc)(menu->Items + i);
			}
		}
		DrawMenu(menu);
	}
	GlobalOptionsInit(NULL);
}

/**
**		Global options fullscreen gem callback
*/
local void GlobalOptionsFullscreenGem(Menuitem *mi __attribute__((unused)))
{
	ToggleFullScreen();
	GlobalOptionsInit(NULL);
}

/**
**		Master volume gem callback
*/
local void SetMasterPower(Menuitem *mi __attribute__((unused)))
{
	if (SoundFildes != -1) {
		QuitSound();
		SoundOff = 1;
	} else {
		SoundOff = 0;
		if(InitSound()) {
			SoundOff = 1;
			SoundFildes = -1;
		}
		MapUnitSounds();
		if (InitSoundServer()) {
			SoundOff = 1;
		} else {
			InitSoundClient();
		}
	}

	SoundOptionsInit(NULL);
}

/**
**		Music volume gem callback
*/
local void SetMusicPower(Menuitem *mi __attribute__((unused)))
{
	if (PlayingMusic) {
		MusicOff = 1;
		StopMusic();
	} else {
		MusicOff = 0;
		if (CallbackMusic) {
			lua_pushstring(Lua, "MusicStopped");
			lua_gettable(Lua, LUA_GLOBALSINDEX);
			LuaCall(0, 1);
		}
	}

	SoundOptionsInit(NULL);
}

/**
**		CD volume gem callback
*/
local void SetCdPower(Menuitem *mi __attribute__((unused)))
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
**		Toggle the fog of war handling.
**		Used in the preference menu.
**
**		@param mi		Menu item (not used).
*/
local void SetFogOfWar(Menuitem *mi __attribute__((unused)))
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
	MustRedraw &= ~RedrawMinimap;
}

/**
**		Toggle showing the command chars on icons.
**		Used in the preference menu.
**
**		@param mi		Menu item (not used).
*/
local void SetCommandKey(Menuitem *mi __attribute__((unused)))
{
	ShowCommandKey ^= 1;
}

/**
**		CD play all tracks gem callback
*/
local void SetCdModeDefined(Menuitem *mi __attribute__((unused)))
{
#ifdef USE_CDAUDIO
	CDMode = CDModeDefined;
#endif
	SoundOptionsInit(NULL);
}

/**
**		CD play random tracks gem callback
*/
local void SetCdModeRandom(Menuitem *mi __attribute__((unused)))
{
#ifdef USE_CDAUDIO
	CDMode = CDModeRandom;
#endif
	SoundOptionsInit(NULL);
}

/**
**		Speed settings menu
*/
global void SpeedOptionsMenu(void)
{
	ProcessMenu("menu-speed-options", 0);
}

/**
**		Init callback for speed settings menu
*/
global void SpeedOptionsInit(Menuitem *mi __attribute__((unused)))
{
	Menu *menu;
	int i;

	i = 2;
	menu = CurrentMenu;

	menu->Items[i].d.hslider.percent = ((VideoSyncSpeed - MIN_GAME_SPEED) * 100) / (MAX_GAME_SPEED - MIN_GAME_SPEED);
	if (menu->Items[i].d.hslider.percent < 0) {
		menu->Items[i].d.hslider.percent = 0;
	}
	if (menu->Items[i].d.hslider.percent > 100) {
		menu->Items[i].d.hslider.percent = 100;
	}

	menu->Items[i + 4].d.hslider.percent = 100 - (SpeedMouseScroll - 1) * 100 / 10;
	if (TheUI.MouseScroll == 0) {
		menu->Items[i + 4].d.hslider.percent = 0;
	}
	menu->Items[i + 8].d.hslider.percent = 100 - (SpeedKeyScroll - 1) * 100 / 10;
	if (TheUI.KeyScroll == 0) {
		menu->Items[i + 8].d.hslider.percent = 0;
	}
}

/**
**		Exit callback for speed settings menu
*/
global void SpeedOptionsExit(Menuitem *mi __attribute__((unused)))
{
	// FIXME: Only save if something changed
	SavePreferences();
}

/**
**		Diplomacy options menu
*/
global void DiplomacyMenu(void)
{
	ProcessMenu("menu-diplomacy", 0);
}

/**
**		Diplomacy init callback
*/
local void DiplomacyInit(Menuitem *mi __attribute__ ((unused)))
{
	Menu *menu;
	int i;
	int j;

	menu = CurrentMenu;
	j = 0;

	for (i=0; i<=PlayerMax-2; ++i) {
		if (Players[i].Type != PlayerNobody && &Players[i] != ThisPlayer) {
			menu->Items[4*j+4].d.text.text = Players[i].Name;
			if (ThisPlayer->Allied&(1<<Players[i].Player)) {
				menu->Items[4 * j + 5].d.gem.state = MI_GSTATE_CHECKED;
			} else {
				menu->Items[4 * j + 5].d.gem.state = MI_GSTATE_UNCHECKED;
			}
			if (ThisPlayer->Enemy&(1<<Players[i].Player)) {
				menu->Items[4 * j + 6].d.gem.state = MI_GSTATE_CHECKED;
			} else {
				menu->Items[4 * j + 6].d.gem.state = MI_GSTATE_UNCHECKED;
			}
			if (ThisPlayer->SharedVision&(1<<Players[i].Player)) {
				menu->Items[4 * j + 7].d.gem.state = MI_GSTATE_CHECKED;
			} else {
				menu->Items[4 * j + 7].d.gem.state = MI_GSTATE_UNCHECKED;
			}

			if (ReplayGameType != ReplayNone || ThisPlayer->Team == Players[i].Team) {
				menu->Items[4 * j + 5].d.gem.state |= MI_GSTATE_PASSIVE;
				menu->Items[4 * j + 6].d.gem.state |= MI_GSTATE_PASSIVE;
				menu->Items[4 * j + 7].d.gem.state |= MI_GSTATE_PASSIVE;
			} else {
				menu->Items[4 * j + 5].d.gem.state &= ~MI_GSTATE_PASSIVE;
				menu->Items[4 * j + 6].d.gem.state &= ~MI_GSTATE_PASSIVE;
				menu->Items[4 * j + 7].d.gem.state &= ~MI_GSTATE_PASSIVE;
			}

			++j;
		}
	}
	for (; j<=PlayerMax - 3; ++j ) {
		menu->Items[4 * j + 4].d.text.text = NULL;
		menu->Items[4 * j + 5].d.gem.state = MI_GSTATE_INVISIBLE;
		menu->Items[4 * j + 6].d.gem.state = MI_GSTATE_INVISIBLE;
		menu->Items[4 * j + 7].d.gem.state = MI_GSTATE_INVISIBLE;
	}
}

/**
**		Diplomacy exit callback
*/
local void DiplomacyExit(Menuitem *mi __attribute__ ((unused)))
{
	Menu* menu;
	int i;

	menu = CurrentMenu;

	for (i=0; i<=PlayerMax - 3; ++i) {
		menu->Items[4 * i + 4].d.text.text = NULL;
	}
}

/**
**		Diplomacy gem callback
*/
local void DiplomacyWait(Menuitem *mi)
{
	int player;
	int item;

	item = mi - mi->menu->Items;
	player = (item - 4) / 4;

	// Don't allow allies and enemies at the same time
	if (item == 4 * player + 5) {
		mi->menu->Items[4 * player + 5].d.gem.state |= MI_GSTATE_CHECKED;
		mi->menu->Items[4 * player + 6].d.gem.state &= ~MI_GSTATE_CHECKED;
	} else if (item == 4 * player + 6) {
		mi->menu->Items[4 * player + 5].d.gem.state &= ~MI_GSTATE_CHECKED;
		mi->menu->Items[4 * player + 6].d.gem.state |= MI_GSTATE_CHECKED;
	}

	// Don't set diplomacy until clicking ok
}

/**
**		Diplomacy Ok button callback
*/
local void DiplomacyOk(void)
{
	Menu *menu;
	int i;
	int j;

	menu = CurrentMenu;
	j = 0;

	for (i=0; i<=PlayerMax - 2; ++i) {
		if (Players[i].Type != PlayerNobody && &Players[i] != ThisPlayer) {
			// Menu says to ally
			if (menu->Items[4 * j + 5].d.gem.state == MI_GSTATE_CHECKED &&
				menu->Items[4 * j + 6].d.gem.state == MI_GSTATE_UNCHECKED) {
				// Are they allied?
				if (!(ThisPlayer->Allied&(1<<Players[i].Player) &&
					!(ThisPlayer->Enemy&(1<<Players[i].Player)))) {
					SendCommandDiplomacy(ThisPlayer->Player,DiplomacyAllied,
						Players[i].Player);
				}
			}
			// Menu says to be enemies
			if (menu->Items[4 * j + 5].d.gem.state == MI_GSTATE_UNCHECKED &&
				menu->Items[4 * j + 6].d.gem.state == MI_GSTATE_CHECKED) {
				// Are they enemies?
				if( !(!(ThisPlayer->Allied&(1<<Players[i].Player)) &&
					ThisPlayer->Enemy&(1<<Players[i].Player))) {
					SendCommandDiplomacy(ThisPlayer->Player,DiplomacyEnemy,
						Players[i].Player);
				}
			}
			// Menu says to be neutral
			if (menu->Items[4 * j + 5].d.gem.state == MI_GSTATE_UNCHECKED &&
				menu->Items[4 * j + 6].d.gem.state == MI_GSTATE_UNCHECKED) {
				// Are they neutral?
				if (!(!(ThisPlayer->Allied&(1<<Players[i].Player)) &&
					!(ThisPlayer->Enemy&(1<<Players[i].Player)))) {
					SendCommandDiplomacy(ThisPlayer->Player,DiplomacyNeutral,
						Players[i].Player);
				}
			}
			// Menu says to be crazy
			if (menu->Items[4 * j + 5].d.gem.state == MI_GSTATE_CHECKED &&
				menu->Items[4 * j + 6].d.gem.state == MI_GSTATE_CHECKED) {
				// Are they crazy?
				if (!(ThisPlayer->Allied&(1<<Players[i].Player) &&
					ThisPlayer->Enemy&(1<<Players[i].Player))) {
					SendCommandDiplomacy(ThisPlayer->Player,DiplomacyCrazy,
						Players[i].Player);
				}
			}
			// Shared vision
			if (menu->Items[4 * j + 7].d.gem.state == MI_GSTATE_CHECKED) {
				if (!(ThisPlayer->SharedVision&(1<<Players[i].Player))) {
					SendCommandSharedVision(ThisPlayer->Player,1,
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
**		Enter the preferences menu.
**		Setup if the options are enabled / disabled.
*/
global void PreferencesMenu(void)
{
	ProcessMenu("menu-preferences", 0);
}

/**
**  Preferences menu init callback
*/
local void PreferencesInit(Menuitem* mi __attribute__((unused)))
{
	Menu* menu;

	menu = CurrentMenu;
	if (!TheMap.NoFogOfWar) {
		menu->Items[1].d.gem.state = MI_GSTATE_CHECKED;
	} else {
		menu->Items[1].d.gem.state = MI_GSTATE_UNCHECKED;
	}

	// Not available in net games or replays
	if (!IsNetworkGame() && ReplayGameType == ReplayNone) {
		menu->Items[1].flags = MI_ENABLED;
	} else {
		menu->Items[1].flags = MI_DISABLED;
	}

	if (ShowCommandKey) {
		menu->Items[2].d.gem.state = MI_GSTATE_CHECKED;
	} else {
		menu->Items[2].d.gem.state = MI_GSTATE_UNCHECKED;
	}
}

/**
**  Preferences menu init callback
*/
local void PreferencesExit(Menuitem* mi __attribute__((unused)))
{
	// FIXME: Only save if something changed
	SavePreferences();
}

/**
**  Show the game options.
*/
local void GameOptionsMenu(void)
{
	ProcessMenu("menu-game-options", 0);
}

/**
**  Show the game credits.
*/
local void GameShowCredits(void)
{
	ShowCredits(&GameCredits);
}

/**
**  Show the Restart Confirm menu
*/
global void RestartConfirmMenu(void)
{
	ProcessMenu("menu-restart-confirm", 0);
}

/**
**  Show the Surrender Confirm menu
*/
local void SurrenderConfirmMenu(void)
{
	ProcessMenu("menu-surrender-confirm", 0);
}

/**
**  Show the Quit To Menu Confirm menu
*/
global void QuitToMenuConfirmMenu(void)
{
	ProcessMenu("menu-quit-to-menu-confirm", 0);
}

/**
**  Show the Exit Confirm menu
*/
global void ExitConfirmMenu(void)
{
	ProcessMenu("menu-exit-confirm", 0);
}

/**
**  Show the End Scenario menu
*/
local void EndScenarioMenu(void)
{
	Menu* menu;

	menu = FindMenu("menu-end-scenario");
	if (!IsNetworkGame()) {
		menu->Items[1].flags = MI_ENABLED;
	} else {
		menu->Items[1].flags = MI_DISABLED;
	}

	ProcessMenu("menu-end-scenario", 0);
}

/**
**  Restart the scenario
*/
local void EndScenarioRestart(void)
{
	RestartScenario = 1;
	GameRunning = 0;
	GameMenuReturn();
}

/**
**  End the game in defeat
*/
local void EndScenarioSurrender(void)
{
	GameResult = GameDefeat;
	GameRunning = 0;
	GameMenuReturn();
}

/**
**  End the game and return to the menu
*/
local void EndScenarioQuitMenu(void)
{
	QuitToMenu = 1;
	GameRunning = 0;
	GameMenuReturn();
}

/**
**  End the running game from menu.
*/
local void GameMenuEnd(void)
{
	InterfaceState = IfaceStateNormal;
	GameRunning = 0;
	CursorOn = CursorOnUnknown;
	CurrentMenu = NULL;
}

/**
**  Victory menu init callback
*/
local void VictoryInit(Menuitem* mi __attribute__((unused)))
{
	Menu* menu;

	menu = CurrentMenu;
	if (CommandLogDisabled) {
		menu->Items[3].flags = MI_DISABLED;
	}
}

/**
**  Defeated menu init callback
*/
local void DefeatedInit(Menuitem* mi __attribute__((unused)))
{
	Menu* menu;

	menu = CurrentMenu;
	if (CommandLogDisabled) {
		menu->Items[3].flags = MI_DISABLED;
	}
}

/**
**  Save replay of completed game.
*/
local void SaveReplay(void)
{
	ProcessMenu("menu-save-replay", 0);
}

/**
**  Save replay menu init callback
*/
local void SaveReplayInit(Menuitem* mi __attribute__((unused)))
{
	Menu* menu;
	char* buf;

	menu = CurrentMenu;

	buf = malloc(32);
	strcpy(buf, "~!_");
	menu->Items[1].d.input.buffer = buf;
	menu->Items[1].d.input.nch = 0;
	menu->Items[1].d.input.maxch = 28;
}

/**
**  Save replay menu exit callback
*/
local void SaveReplayExit(Menuitem* mi)
{
	free(mi->menu->Items[1].d.input.buffer);
	mi->menu->Items[1].d.input.buffer = NULL;
}

/**
**  Input field action of save replay menu.
*/
local void SaveReplayEnterAction(Menuitem* mi, int key)
{
	if (mi->d.input.nch == 0) {
		mi[1].flags = MenuButtonDisabled;
	} else {
		mi[1].flags &= ~MenuButtonDisabled;
		if (key == 10 || key == 13) {
			SaveReplayOk();
		}
	}
}

/**
**		Save replay Ok button.
*/
local void SaveReplayOk(void)
{
	FILE *fd;
	Menu *menu;
	char *buf;
	struct stat s;
	char *ptr;

	menu = FindMenu("menu-save-replay");

	if (strchr(menu->Items[1].d.input.buffer, '/')) {
		ErrorMenu("Name cannot contain '/'");
		return;
	}
	if (strchr(menu->Items[1].d.input.buffer, '\\')) {
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
	buf = (char*)malloc(s.st_size);
	fd = fopen(TempPathBuf, "rb");
	fread(buf, s.st_size, 1, fd);
	fclose(fd);

	strncpy(ptr, menu->Items[1].d.input.buffer, menu->Items[1].d.input.nch);
	ptr[menu->Items[1].d.input.nch] = '\0';
	if (!strcasestr(menu->Items[1].d.input.buffer, ".log\0")) {
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
}

/**
**		Keystroke help menu
*/
local void KeystrokeHelpMenu(void)
{
	ProcessMenu("menu-keystroke-help", 0);
}

/**
**		Help menu
*/
local void HelpMenu(void)
{
	ProcessMenu("menu-help", 0);
}

/**
**		Tips menu
*/
local void TipsMenu(void)
{
	ProcessMenu("menu-tips", 0);
}

/**
**		Free the tips from the menu
*/
local void TipsFreeTips(void)
{
	int i;
	Menu *menu;

	menu = FindMenu("menu-tips");
	for (i = 4; i < 12; i++) {
		if (menu->Items[i].d.text.text) {
			free(menu->Items[i].d.text.text);
			menu->Items[i].d.text.text = NULL;
		}
	}
}

/**
**		Initialize the tips menu
*/
local void TipsInit(Menuitem *mi __attribute__((unused)))
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

	menu = CurrentMenu;

	if(ShowTips) {
		menu->Items[1].d.gem.state = MI_GSTATE_CHECKED;
	} else {
		menu->Items[1].d.gem.state = MI_GSTATE_UNCHECKED;
	}

	TipsFreeTips();

	w = menu->Width-2 * menu->Items[5].xofs;
	font = menu->Items[5].font;
	i = 0;
	line = 5;

	p = Tips[CurrentTip];
	if(!p) {
		return;
	}

	l = 0;
	s = str = strdup(p);

	while (line<13) {
		char* s1;
		char* space;

		space = NULL;
		for(;;) {
			if(VideoTextLength(font, s) < w) {
				break;
			}
			s1 = strrchr(s,' ');
			if(!s1) {
				fprintf(stderr, "line too long: \"%s\"\n", s);
				break;
			}
			if(space) {
				*space=' ';
			}
			space = s1;
			*space = '\0';
		}
		menu->Items[line++].d.text.text = strdup(s);
		l += strlen(s);
		if(!p[l]) {
			break;
		}
		++l;
		s = str+l;
	}

	free(str);
}

/**
**		Cycle to the next tip
*/
local void TipsCycleNextTip(void)
{
	++CurrentTip;
	if (Tips[CurrentTip] == NULL) {
		CurrentTip = 0;
	}
}

/**
**		Cycle to the previous tip
*/
local void TipsCyclePreviousTip(void)
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
**		Tips menu exit callback
*/
local void TipsExit(Menuitem *mi __attribute__((unused)))
{
	TipsCycleNextTip();
	TipsFreeTips();
	SavePreferences();
}

/**
**		Show tips at startup gem callback
*/
local void TipsShowTipsGem(Menuitem *mi)
{
	if (mi->menu->Items[1].d.gem.state == MI_GSTATE_CHECKED) {
		ShowTips = 1;
	} else {
		ShowTips = 0;
	}
}

/**
**		Show tips at startup text callback
*/
local void TipsShowTipsText(Menuitem *mi)
{
	if (mi->menu->Items[1].d.gem.state == MI_GSTATE_UNCHECKED) {
		ShowTips = 1;
		mi->menu->Items[1].d.gem.state = MI_GSTATE_CHECKED;
	} else {
		ShowTips = 0;
		mi->menu->Items[1].d.gem.state = MI_GSTATE_UNCHECKED;
	}
}

/**
**		Tips menu next tip button callback
*/
local void TipsNextTip(void)
{
	TipsCycleNextTip();
	TipsInit(NULL);
}

/**
**		Tips menu previous tip button callback
*/
local void TipsPreviousTip(void)
{
	TipsCyclePreviousTip();
	TipsInit(NULL);
}

/**
**		Exit the game from menu.
*/
local void GameMenuExit(void)
{
	ExitMenus();
	Exit(0);
}

/**
**		Initialize the game objectives menu
*/
local void ObjectivesInit(Menuitem *mi __attribute__((unused)))
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
	w = menu->Width-2 * menu->Items[1].xofs;
	font = menu->Items[1].font;
	i = 0;
	line = 1;

	for(p = GameIntro.Objectives[i]; p; p = GameIntro.Objectives[++i]) {
		l = 0;
		s = str = strdup(p);

		for(;;) {
			char* s1;
			char* space;

			space = NULL;
			for(;;) {
				if(VideoTextLength(font, s) < w) {
					break;
				}
				s1=strrchr(s, ' ');
				if(!s1) {
					fprintf(stderr, "line too long: \"%s\"\n", s);
					break;
				}
				if( space ) {
					*space=' ';
				}
				space = s1;
				*space = '\0';
			}
			menu->Items[line++].d.text.text = strdup(s);
			l += strlen(s);
			if(!p[l]) {
				break;
			}
			++l;
			s = str + l;

			if(line==menu->NumItems-1) {
				break;
			}
		}
		free(str);
	}
}

/**
**		Free the game objectives menu memory
*/
local void ObjectivesExit(Menuitem *mi)
{
	int i;
	Menu *menu;

	menu = mi->menu;
	for(i = 1; i < menu->NumItems - 1;i++) {
		if( menu->Items[i].d.text.text ) {
			free(menu->Items[i].d.text.text);
			menu->Items[i].d.text.text = NULL;
		}
	}
}

/**
**		Game objectives menu
*/
local void ObjectivesMenu(void)
{
	ProcessMenu("menu-objectives", 0);
}

/**
**		Get map info from select path+name
*/
local void GetInfoFromSelectPath(void)
{
	int i;

	FreeMapInfo(MenuMapInfo);
	MenuMapInfo = NULL;

	if (ScenSelectPath[0]) {
		i = strlen(ScenSelectPath);
		strcat(ScenSelectPath, "/");
	} else {
		i = 0;
	}
	strcat(ScenSelectPath, ScenSelectFileName);		// Final map name with path
	if (strcasestr(ScenSelectFileName, ".pud")) {
		MenuMapInfo = GetPudInfo(ScenSelectPath);
		strcpy(MenuMapFullPath, ScenSelectPath);
	} else {
		// FIXME: GetCmInfo();
	}
	ScenSelectPath[i] = '\0';				// Remove appended part
}

/**
**		Enter select scenario menu.
*/
local void ScenSelectMenu(void)
{
	Menu *menu;
	int n;
	int j;
	int t;

	ProcessMenu("menu-select-scenario", 1);

	GetInfoFromSelectPath();

	menu = FindMenu("menu-custom-game");
	// FIXME: This check is only needed until GetCmInfo works
	if (!MenuMapInfo) {
		menu->Items[12].d.pulldown.noptions = PlayerMax-1;
		menu->Items[12].d.pulldown.curopt = 0;
	} else {
		for (n = j = 0; j < PlayerMax; ++j) {
			t = MenuMapInfo->PlayerType[j];
			if (t == PlayerPerson || t == PlayerComputer) {
				n++;
			}
		}
		menu->Items[12].d.pulldown.noptions = n;
		if (menu->Items[12].d.pulldown.curopt >= n) {
			menu->Items[12].d.pulldown.curopt = 0;
		}
	}
}

/**
**		Enter multiplayer select scenario menu.
*/
local void MultiScenSelectMenu(void)
{
	Menu *menu;
	unsigned flags;

	// FIXME: remove when cm works with multiplayer
	menu = FindMenu("menu-select-scenario");
	flags = menu->Items[6].flags;
	menu->Items[6].flags = MenuButtonDisabled;

	ScenSelectMenu();
	MultiGamePlayerSelectorsUpdate(1);

	menu->Items[6].flags = MI_DISABLED;
}

/**
**		Enter single player menu.
*/
local void SinglePlayerGameMenu(void)
{
	GuiGameStarted = 0;
	ProcessMenu("menu-custom-game", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

/**
**		Show the campaign select menu.
**
**		Look which campaigns are available and how they are called.
*/
local void CampaignGameMenu(void)
{
	int i;
	Menu* menu;

	MenusSetBackground();
	Invalidate();

	menu = FindMenu("menu-campaign-select");
	DebugLevel0Fn("%d campaigns available\n" _C_ NumCampaigns);
#ifdef DEBUG
	for (i = 0; i < NumCampaigns; ++i) {
		DebugLevel0Fn("Campaign %d: %16.16s: %s\n" _C_ i _C_
			Campaigns[i].Ident _C_
			Campaigns[i].Name);
	}
#endif

	//
	//		Setup campaign name.
	//
	for (i = 0; i < NumCampaigns && i < 4; ++i) {
		char* s;

		menu->Items[i].d.button.text = Campaigns[i].Name;
		menu->Items[i].flags &= ~MenuButtonDisabled;

		if ((s = strchr(Campaigns[i].Name, '!'))) {
			menu->Items[i].d.button.hotkey = tolower(s[1]);
		}
	}
	for (; i < 4; ++i) {
		menu->Items[i].d.button.text = "Not available";
		menu->Items[i].flags |= MenuButtonDisabled;
	}

	GuiGameStarted = 0;
	ProcessMenu("menu-campaign-select", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}

	for (i = 0; i < 4; ++i) {
		menu->Items[i].d.button.text = NULL;
	}
}

/**
**		Start campaign from menu.
**
**		@param number		Number of the compaign.
*/
local void StartCampaignFromMenu(int number)
{
	MenusSetBackground();
	Invalidate();

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

	MenusSetBackground();
	VideoClearScreen();
	Invalidate();

	// FIXME: johns otherwise crash in UpdateDisplay -> DrawMinimapCursor
	EndMenu();
}

/**
**		Call back for 1st entry of campaign menu.
**
**		@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu1(void)
{
	StartCampaignFromMenu(0);
}

/**
**		Call back for 2nd entry of campaign menu.
**
**		@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu2(void)
{
	StartCampaignFromMenu(1);
}

/**
**		Call back for 3rd entry of campaign menu.
**
**		@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu3(void)
{
	StartCampaignFromMenu(2);
}

/**
**		Call back for 4th entry of campaign menu.
**
**		@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu4(void)
{
	StartCampaignFromMenu(3);
}

/**
**		Call back for select entry of campaign menu.
*/
local void SelectCampaignMenu(void)
{
	// FIXME: not written
}

/**
**		Cancel button of player name menu pressed.
*/
local void EnterNameCancel(void)
{
	Menu *menu;

	menu = CurrentMenu;
	menu->Items[1].d.input.nch = 0;
	EndMenu();
}

/**
**		Input field action of player name menu.
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
**		Cancel button of enter server ip/name menu pressed.
*/
local void EnterServerIPCancel(void)
{
	CurrentMenu->Items[1].d.input.nch = 0;
	EndMenu();
}

/**
**		Input field action of server ip/name.
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
**		Start processing join a network game menu.
*/
local void JoinNetGameMenu(void)
{
	char server_host_buffer[64];
	char *port;
	Menu *menu;

	MenusSetBackground();
	Invalidate();

	//
	//  Prepare enter ip/hostname menu
	//
	if (NetworkArg) {
		strncpy(server_host_buffer, NetworkArg, 24);
		server_host_buffer[24] = 0;
		if (NetworkPort != NetworkDefaultPort) {
			strcat(server_host_buffer, ":");
			port = (char*)malloc(10);
			sprintf(port, "%d", NetworkPort);
			strcat(server_host_buffer, port);
			free(port);
		}
	} else {
		server_host_buffer[0] = '\0';
	}
	menu = FindMenu("menu-enter-server");
	strcat(server_host_buffer, "~!_");
	menu->Items[1].d.input.buffer = server_host_buffer;
	menu->Items[1].d.input.nch = strlen(server_host_buffer) - 3;
	menu->Items[1].d.input.maxch = 60;
	if (menu->Items[1].d.input.nch) {
		menu->Items[2].flags &= ~MenuButtonDisabled;
	} else {
		menu->Items[2].flags |= MenuButtonDisabled;
	}

	ProcessMenu("menu-enter-server", 1);

	MenusSetBackground();

	if (menu->Items[1].d.input.nch == 0) {
		return;
	}

	if ( (port = strchr(server_host_buffer, ':')) != NULL) {
		NetworkPort = atoi(port + 1);
		port[0] = 0;
	}

	// Now finally here is the address
	server_host_buffer[menu->Items[1].d.input.nch] = 0;
	if (NetworkSetupServerAddress(server_host_buffer)) {
		NetErrorMenu("Unable to lookup host.");
		MenusSetBackground();
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
		MenusSetBackground();
		Invalidate();
		EndMenu();
	}
}

/**
**		Network connect menu init.
*/
local void NetConnectingInit(Menuitem *mi)
{
	mi->menu->Items[1].d.text.text = NetServerText;
	mi->menu->Items[2].d.text.text = NetTriesText;
}

/**
**		Network connect menu exit.
*/
local void NetConnectingExit(Menuitem *mi)
{
	mi->menu->Items[1].d.text.text = NULL;
	mi->menu->Items[2].d.text.text = NULL;
}

/**
**		Cancel button of network connect menu pressed.
*/
local void NetConnectingCancel(void)
{
	MenusSetBackground();
	NetworkExitClientConnect();
	// Trigger TerminateNetConnect() to call us again and end the menu
	NetLocalState = ccs_usercanceled;
	EndMenu();
}

/**
**		Call back from menu loop, if network state has changed.
*/
local void TerminateNetConnect(void)
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

	DebugLevel1Fn("NetLocalState %d\n" _C_ NetLocalState);
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
**		Start processing network game setup menu (server).
*/
local void CreateNetGameMenu(void)
{
	GuiGameStarted = 0;
	ProcessMenu("menu-multi-setup", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

/**
**		Start process network game setup menu (server).
**		Internet game, register with meta server
*/
local void CreateInternetGameMenu(void)
{
	GuiGameStarted = 0;
	AddGameServer();
	ProcessMenu("menu-multi-setup", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}

}

/**
**		Multiplayer game start game button pressed.
*/
local void MultiGameStart(void)
{
	MenusSetBackground();
	Invalidate();

	if (MetaServerInUse) {
		SendMetaCommand("StartGame","");
	}

	GameSettings.Presets[0].Race = SettingsPresetMapDefault;

	NetworkServerStartGame();
	NetworkGamePrepareGameSettings();

	CustomGameStart();
}

/**
**		Enter multiplayer game menu.
*/
local void MultiPlayerGameMenu(void)
{
	char NameBuf[32];
	Menu *menu;

	MenusSetBackground();
	Invalidate();

	menu = FindMenu("menu-enter-name");
	menu->Items[1].d.input.buffer = NameBuf;
	strcpy(NameBuf, LocalPlayerName);
	strcat(NameBuf, "~!_");
	menu->Items[1].d.input.nch = strlen(NameBuf) - 3;
	menu->Items[1].d.input.maxch = 15;
	menu->Items[2].flags &= ~MenuButtonDisabled;

	ProcessMenu("menu-enter-name", 1);

	MenusSetBackground();

	if (menu->Items[1].d.input.nch == 0) {
		return;
	}

	NameBuf[menu->Items[1].d.input.nch] = 0;		// Now finally here is the name
	memset(LocalPlayerName, 0, 16);
	strcpy(LocalPlayerName, NameBuf);

	// FIXME: Only save if player name changed
	SavePreferences();

	GuiGameStarted = 0;
	// Here we really go...
	// ProcessMenu("menu-create-join-menu", 1);

	ProcessMenu("menu-multi-net-type-menu", 1);


	DebugLevel0Fn("GuiGameStarted: %d\n" _C_ GuiGameStarted);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

/**
**		Process LAN/P2P game menu
*/
local void MultiPlayerLANGame(void)
{
	ProcessMenu("menu-create-join-menu", 1);
	MetaServerInUse = 0;
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

/**
**		Process Internet game menu
*/
local void MultiPlayerInternetGame(void)
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
**		Free map info data
*/
local void FreeMapInfos(FileList *fl, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (fl[i].type && fl[i].xdata) {
			FreeMapInfo(fl[i].xdata);
			fl[i].xdata = NULL;
		}
		free(fl[i].name);
	}
}

/**
**		Initialize the scenario selector menu.
*/
local void ScenSelectInit(Menuitem *mi)
{
	DebugCheck(!*ScenSelectPath);
	mi->menu->Items[9].flags =
		*ScenSelectDisplayPath ? 0 : MenuButtonDisabled;
	mi->menu->Items[9].d.button.text = ScenSelectDisplayPath;
	DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);
}

/**
**		Scenario select listbox action callback
*/
local void ScenSelectLBAction(Menuitem *mi, int i)
{
	FileList *fl;

	DebugCheck( i < 0);
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		free(mi->menu->Items[3].d.button.text);
		if (fl[i].type) {
			mi->menu->Items[3].d.button.text = strdup("OK");
		} else {
			mi->menu->Items[3].d.button.text = strdup("Open");
		}
		if (mi->d.listbox.noptions > mi->d.listbox.nlines) {
			mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
			mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
		}
	}
}

/**
**		Scenario select listbox exit callback
*/
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

/**
**		Scenario select read directory filter
*/
local int ScenSelectRDFilter(char *pathbuf, FileList *fl)
{
	MapInfo *info;
	char *suf[3];
	char *cp;
	char *lcp;
	char *np;
	int curopt;
	unsigned u;
	int sz;
	static int szl[] = { -1, 32, 64, 96, 128, 256, 512, 1024 };
	Menu *menu;

	menu = FindMenu("menu-select-scenario");

	curopt = menu->Items[6].d.pulldown.curopt;
	if (curopt == 0) {
		suf[0] = ".cm";
		suf[1] = NULL;
	} else if (curopt == 1) {
		suf[0] = ".pud";
		suf[1] = NULL;
	} else {
		suf[0] = ".scm";
		suf[1] = ".chk";
		suf[2] = NULL;
	}
	np = strrchr(pathbuf, '/');
	if (np) {
		np++;
	} else {
		np = pathbuf;
	}
	fl->type = -1;
	u = 0;
	lcp = 0;
	while (suf[u]) {
		cp = np;
		--cp;
		do {
			lcp = cp++;
			cp = strcasestr(cp, suf[u]);
		} while (cp != NULL);
		if (lcp >= np) {
			break;
		}
		++u;
	}
	if (!suf[u]) {
		return 0;
	}

	if (lcp >= np) {
		cp = lcp + strlen(suf[u]);
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
			if (curopt == 0) {
				// info = GetCmInfo(pathbuf);
				info = NULL;
				DebugLevel3Fn("GetCmInfo(%s) : %p\n" _C_ pathbuf _C_ info);
				fl->type = 1;
				fl->name = strdup(np);
				fl->xdata = info;
				return 1;
			} else if (curopt == 1) {
				info = GetPudInfo(pathbuf);
				if (info) {
					DebugLevel3Fn("GetPudInfo(%s) : %p\n" _C_ pathbuf _C_ info);
					sz = szl[menu->Items[8].d.pulldown.curopt];
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
		}
	}
	return 0;
}

/**
**		Scenario select listbox init callback
*/
local void ScenSelectLBInit(Menuitem *mi)
{
	int i;

	ScenSelectLBExit(mi);
	if (mi->menu->Items[6].d.pulldown.curopt == 0) {
		mi->menu->Items[8].flags |= MenuButtonDisabled;
	} else {
		mi->menu->Items[8].flags &= ~MenuButtonDisabled;
	}
	i = mi->d.listbox.noptions = ReadDataDirectory(ScenSelectPath, ScenSelectRDFilter,
													 (FileList **)&(mi->d.listbox.options));
	if (i == 0) {
		free(mi->menu->Items[3].d.button.text);
		mi->menu->Items[3].d.button.text = strdup("OK");
		mi->menu->Items[3].flags |= MenuButtonDisabled;
	} else {
		ScenSelectLBAction(mi, 0);
		mi->menu->Items[3].flags &= ~MenuButtonDisabled;
		if (i > mi->d.listbox.nlines) {
			mi[1].flags &= ~MenuButtonDisabled;
		}
	}
}

/**
**		Scenario select listbox retrieve callback
*/
local unsigned char *ScenSelectLBRetrieve(Menuitem *mi, int i)
{
	FileList *fl;
	MapInfo *info;
	static char buffer[1024];
	int j;
	int n;

	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (fl[i].type) {
			if (i - mi->d.listbox.startline == mi->d.listbox.curopt) {
				if ((info = fl[i].xdata)) {
					if (info->Description) {
						VideoDrawText(mi->menu->X + 8, mi->menu->Y + 254, LargeFont, info->Description);
					}
					sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
					VideoDrawText(mi->menu->X + 8,mi->menu->Y + 254 + 20, LargeFont, buffer);
					for (n = j = 0; j < PlayerMax; j++) {
						if (info->PlayerType[j] == PlayerPerson) {
							n++;
						}
					}
					if (n == 1) {
						VideoDrawText(mi->menu->X + 8, mi->menu->Y + 254 + 40, LargeFont, "1 player");
					} else {
						sprintf(buffer, "%d players", n);
						VideoDrawText(mi->menu->X + 8, mi->menu->Y + 254 + 40, LargeFont, buffer);
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

/**
**		Scenario select map type action callback
*/
local void ScenSelectTPMSAction(Menuitem *mi, int i __attribute__((unused)))
{
	mi = mi->menu->Items + 1;
	ScenSelectLBInit(mi);
	mi->d.listbox.cursel = -1;
	mi->d.listbox.startline = 0;
	mi->d.listbox.curopt = 0;
	mi[1].d.vslider.percent = 0;
	mi[1].d.hslider.percent = 0;
}

/**
**		Scenario select vertical slider action callback
*/
local void ScenSelectVSAction(Menuitem *mi, int i)
{
	int op;
	int d1;
	int d2;

	mi--;
	switch (i) {
		case 0:				// click - down
		case 2:				// key - down
			if (mi[1].d.vslider.cflags&MI_CFLAGS_DOWN) {
				if (mi->d.listbox.curopt + mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
					mi->d.listbox.curopt++;
					if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
						mi->d.listbox.curopt--;
						mi->d.listbox.startline++;
					}
				}
			} else if (mi[1].d.vslider.cflags & MI_CFLAGS_UP) {
				if (mi->d.listbox.curopt + mi->d.listbox.startline > 0) {
					mi->d.listbox.curopt--;
					if (mi->d.listbox.curopt < 0) {
						mi->d.listbox.curopt++;
						mi->d.listbox.startline--;
					}
				}
			}
			ScenSelectLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			if (i == 2) {
				mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
			}
			break;
		case 1:				// mouse - move
			if (mi[1].d.vslider.cflags & MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
				if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt + mi->d.listbox.startline + 1 < mi->d.listbox.noptions) {
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
							if (mi->d.listbox.curopt + mi->d.listbox.startline + 1 == mi->d.listbox.noptions)
								break;
						}
					}
				} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt + mi->d.listbox.startline > 0) {
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
							if (mi->d.listbox.curopt + mi->d.listbox.startline == 0)
								break;
						}
					}
				}

				DebugCheck(mi->d.listbox.startline < 0);
				DebugCheck(mi->d.listbox.noptions > 0 &&
					mi->d.listbox.startline + mi->d.listbox.curopt >= mi->d.listbox.noptions);

				ScenSelectLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			}
			break;
		default:
			break;
	}
}

/**
**		Keystroke help vertical slider action callback
*/
local void KeystrokeHelpVSAction(Menuitem *mi, int i)
{
	int j;

	switch (i) {
		case 0:				// click - down
		case 2:				// key - down
			j = ((mi->d.vslider.percent + 1) * (nKeyStrokeHelps - 11)) / 100;
			if (mi->d.vslider.cflags&MI_CFLAGS_DOWN && j < nKeyStrokeHelps - 11) {
				j++;
			} else if (mi->d.vslider.cflags  & MI_CFLAGS_UP && j > 0) {
				j--;
			}
			if (i == 2) {
				mi->d.vslider.cflags &= ~(MI_CFLAGS_DOWN | MI_CFLAGS_UP);
			}
			mi->d.vslider.percent = j * 100 / (nKeyStrokeHelps - 11);
			break;
		case 1:				// mouse - move
			if ((mi->d.vslider.cflags&MI_CFLAGS_KNOB) && (mi->flags&MenuButtonClicked)) {
				j = ((mi->d.vslider.curper + 1) * (nKeyStrokeHelps - 11)) / 100;
				mi->d.vslider.percent = mi->d.vslider.curper;
			}
			break;
		default:
			break;
	}
}

/**
**		Draw the Keystroke Help texts
*/
local void KeystrokeHelpDrawFunc(Menuitem *mi)
{
	int i;
	int j;

	j = ((mi[-2].d.vslider.percent + 1) * (nKeyStrokeHelps - 11)) / 100;
	for (i = 0; i < 11; i++) {
		VideoDrawText(mi->menu->X+mi->xofs,mi->menu->Y+mi->yofs+(i*20),
							mi->font,KeyStrokeHelps[j * 2]);
		VideoDrawText(mi->menu->X + mi->xofs + 80,mi->menu->Y + mi->yofs+(i * 20),
							mi->font,KeyStrokeHelps[j * 2 + 1]);
		j++;
	}
}

/**
**		Game speed horizontal slider action callback
*/
local void GameSpeedHSAction(Menuitem *mi)
{
	VideoSyncSpeed = (mi->d.hslider.percent * (MAX_GAME_SPEED - MIN_GAME_SPEED)) / 100 + MIN_GAME_SPEED;
	SetVideoSync();
}

/**
**		Mouse scroll horizontal slider action callback
*/
local void MouseScrollHSAction(Menuitem *mi)
{
	TheUI.MouseScroll = 1;
	SpeedMouseScroll = 10 - (mi->d.hslider.percent * 9) / 100;
	if (mi->d.hslider.percent == 0) {
		TheUI.MouseScroll = 0;
	}
}

/**
**		Keyboard scroll horizontal slider action callback
*/
local void KeyboardScrollHSAction(Menuitem *mi)
{
	TheUI.KeyScroll = 1;
	SpeedKeyScroll = 10 - (mi->d.hslider.percent * 9) / 100;
	if (mi->d.hslider.percent == 0) {
		TheUI.KeyScroll = 0;
	}
}

/**
**		Master volume horizontal slider action callback
*/
local void MasterVolumeHSAction(Menuitem *mi)
{
	SetGlobalVolume((mi->d.hslider.percent * 255) / 100);
}

/**
**		Music volume horizontal slider action callback
*/
local void MusicVolumeHSAction(Menuitem *mi)
{
	SetMusicVolume((mi->d.hslider.percent * 255) / 100);
}

#ifdef USE_CDAUDIO
/**
**		CD volume horizontal slider action callback
*/
local void CdVolumeHSAction(Menuitem *mi)
{
	SetCDVolume((mi->d.hslider.percent * 255) / 100);
}
#else
/**
**		CD volume horizontal slider action callback
*/
local void CdVolumeHSAction(Menuitem *mi __attribute__((unused)))
{
}
#endif

/**
**		Scenario select folder button
*/
local void ScenSelectFolder(void)
{
	char *cp;
	Menu *menu;
	Menuitem *mi;

	menu = CurrentMenu;
	mi = menu->Items + 1;
	if (ScenSelectDisplayPath[0]) {
		cp = strrchr(ScenSelectDisplayPath, '/');
		if (cp) {
			*cp = 0;
		} else {
			ScenSelectDisplayPath[0] = 0;
			menu->Items[9].flags |= MenuButtonDisabled;
			menu->Items[9].d.button.text = NULL;
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
		}
	}
}

/**
**		Scenario select ok button
*/
local void ScenSelectOk(void)
{
	FileList *fl;
	Menu *menu;
	Menuitem *mi;
	int i;

	menu = CurrentMenu;
	mi = &menu->Items[1];
	i = mi->d.listbox.curopt + mi->d.listbox.startline;
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (fl[i].type == 0) {
			strcat(ScenSelectPath, "/");
			strcat(ScenSelectPath, fl[i].name);
			if (menu->Items[9].flags & MenuButtonDisabled) {
				menu->Items[9].flags &= ~MenuButtonDisabled;
				menu->Items[9].d.button.text = ScenSelectDisplayPath;
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
		} else {
			strcpy(ScenSelectFileName, fl[i].name);		// Final map name
			EndMenu();
			menu->Items[9].d.button.text = NULL;
		}
	}
}

/**
**		Scenario select cancel button.
*/
local void ScenSelectCancel(void)
{
	// FIXME: why is all this below needed?
/*
	char *s;

	//
	//  Use last selected map.
	//
	DebugLevel0Fn("Map   path: %s\n" _C_ CurrentMapPath);
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

	DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);
*/
	CurrentMenu->Items[9].d.button.text = NULL;

	EndMenu();
}

/**
**		Custom game cancel button callback
*/
local void GameCancel(void)
{
	MenusSetBackground();
	FreeMapInfo(MenuMapInfo);
	MenuMapInfo = NULL;
	EndMenu();
}

/**
**		Custom game start game button pressed.
*/
local void CustomGameStart(void)
{
	int i;
	char *p;

	FreeMapInfo(MenuMapInfo);
	MenuMapInfo = NULL;

	if (ScenSelectPath[0]) {
		strcat(ScenSelectPath, "/");
		strcat(ScenSelectPath, ScenSelectFileName);		// Final map name with path
		p = ScenSelectPath + strlen(StratagusLibPath) + 1;
		strcpy(CurrentMapPath, p);
	} else {
		strcpy(CurrentMapPath, ScenSelectFileName);
		strcat(ScenSelectPath, ScenSelectFileName);		// Final map name with path
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
**		Single player custom game menu entered.
*/
local void GameSetupInit(Menuitem *mi __attribute__ ((unused)))
{
	Menu *menu;
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

	DebugLevel0Fn("Map   path: %s\n" _C_ CurrentMapPath);
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
	DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);

	GetInfoFromSelectPath();

	menu = FindMenu("menu-custom-game");
	// FIXME: This check is only needed until GetCmInfo works
	if (!MenuMapInfo) {
		menu->Items[12].d.pulldown.noptions = PlayerMax-1;
		menu->Items[12].d.pulldown.curopt = 0;
	} else {
		for (n = j = 0; j < PlayerMax; ++j) {
			t = MenuMapInfo->PlayerType[j];
			if (t == PlayerPerson || t == PlayerComputer) {
				n++;
			}
		}
		menu->Items[12].d.pulldown.noptions = n;
		if (menu->Items[12].d.pulldown.curopt >= n) {
			menu->Items[12].d.pulldown.curopt = 0;
		}
	}
}

/**
**		FIXME: docu
*/
local void GameDrawFunc(Menuitem *mi __attribute__((unused)))
{
	char* nc;
	char* rc;
	int l;
	char buffer[32];

	GetDefaultTextColors(&nc, &rc);
	MenusSetBackground();
	SetDefaultTextColors(rc, rc);
	l = VideoTextLength(GameFont, "Scenario:");
	VideoDrawText(TheUI.Offset640X + 16, TheUI.Offset480Y + 360, GameFont, "Scenario:");
	VideoDrawText(TheUI.Offset640X + 16, TheUI.Offset480Y + 360 + 24 , GameFont, ScenSelectFileName);
	if (MenuMapInfo) {
		if (MenuMapInfo->Description) {
			VideoDrawText(TheUI.Offset640X + 16 + l + 8, TheUI.Offset480Y + 360, GameFont, MenuMapInfo->Description);
		}
		sprintf(buffer, " (%d x %d)", MenuMapInfo->MapWidth, MenuMapInfo->MapHeight);
		VideoDrawText(TheUI.Offset640X + 16 + l + 8 + VideoTextLength(GameFont, ScenSelectFileName), TheUI.Offset480Y + 360 + 24, GameFont, buffer);
	}
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
**		Menu setup race pulldown action.
**
**		@note FIXME: Todo support more and other races.
*/
local void GameRCSAction(Menuitem *mi, int i)
{
	int n;
	int x;

	if (mi->d.pulldown.curopt == i) {
		for (n = 0, x = 0; n < PlayerRaces.Count; ++n) {
			if (PlayerRaces.Visible[n]) {
				if (x == i) {
					break;
				}
				++x;
			}
		}
		if (n != PlayerRaces.Count) {
			GameSettings.Presets[0].Race = PlayerRaces.Race[x];
		} else {
			GameSettings.Presets[0].Race = SettingsPresetMapDefault;
		}
		ServerSetupState.Race[0] = mi->d.pulldown.noptions - 1 - i;
		NetworkServerResyncClients();
	}
}

/**
**		Game resources action callback
*/
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

/**
**		Game units action callback
*/
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

/**
**		Game tilesets action callback
*/
local void GameTSSAction(Menuitem *mi, int i)
{
	if (!mi || mi->d.pulldown.curopt == i) {
		// Subtract 1 for default option.
		GameSettings.Terrain = i - 1;
		ServerSetupState.TssOpt = i;
		if (mi) {
			NetworkServerResyncClients();
		}
	}
}

/**
**		Called if the pulldown menu of the game type is changed.
*/
local void GameGATAction(Menuitem *mi, int i)
{
	if (!mi || mi->d.pulldown.curopt == i) {
		GameSettings.GameType = i ? SettingsGameTypeMelee + i - 1 : SettingsGameTypeMapDefault;
		ServerSetupState.GaTOpt = i;
		if (mi) {
			NetworkServerResyncClients();
		}
	}
}

/**
**		Game opponents action callback
*/
local void CustomGameOPSAction(Menuitem *mi __attribute__((unused)), int i)
{
	GameSettings.Opponents = i ? i : SettingsPresetMapDefault;
}

/**
**		Menu setup fog-of-war pulldown action.
*/
local void MultiGameFWSAction(Menuitem *mi, int i)
{
	if (!mi || mi->d.pulldown.curopt == i) {
		DebugLevel0Fn("Update fow %d\n" _C_ i);
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
**		Multiplayer menu player server states.
*/
local void MultiGamePTSAction(Menuitem *mi, int o)
{
	Menu *menu;
	int i;

	menu = FindMenu("menu-multi-setup");
	i = mi - menu->Items - SERVER_PLAYER_STATE;
	// JOHNS: Must this be always true?
	// ARI: NO! think of client menus!
	// DebugCheck( i<0 || i>PlayerMax-1 );

	if (i > 0 && i < PlayerMax-1) {
		if (mi->d.pulldown.curopt == o) {
			if (mi->d.pulldown.noptions == 2) {		// computer slot
				ServerSetupState.CompOpt[i] = o + 1;
			} else {
				ServerSetupState.CompOpt[i] = o;
			}
			MultiGamePlayerSelectorsUpdate(3);		// Recalc buttons on server
			NetworkServerResyncClients();
		}
	}

}

/**
**		Multiplayer server draw func
*/
local void MultiGameDrawFunc(Menuitem *mi)
{
	GameDrawFunc(mi);
}

/**
**		Multiplayer client draw func
*/
local void MultiGameClientDrawFunc(Menuitem *mi)
{
	// FIXME: do something better
	GameDrawFunc(mi);
}

/**
**		Multiplayer network game final race an player type setup.
*/
local void NetworkGamePrepareGameSettings(void)
{
	int c;
	int h;
	int i;
	int num[PlayerMax];
	int comp[PlayerMax];
	int n;
	int x;
	int v;

	DebugCheck(!MenuMapInfo);

	DebugLevel0Fn("NetPlayers = %d\n" _C_ NetPlayers);

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
		if (MenuMapInfo->PlayerType[i] == PlayerPerson) {
			DebugLevel3Fn("Player slot %i is available for a person\n" _C_ i);
			num[h++] = i;
		}
		if (MenuMapInfo->PlayerType[i] == PlayerComputer) {
			comp[c++] = i;		// available computer player slots
		}
	}
	for (i = 0; i < h; i++) {
		switch(ServerSetupState.CompOpt[num[i]]) {
			case 0:
				GameSettings.Presets[num[i]].Type = PlayerPerson;
				DebugLevel3Fn("Settings[%d].Type == Person\n" _C_ num[i]);
				for (n = 0, v = 0; n < PlayerRaces.Count; ++n) {
					if (PlayerRaces.Visible[n]) {
						++v;
					}
				}
				v -= ServerSetupState.Race[num[i]];
				for (n = 0, x = 0; n < PlayerRaces.Count; ++n) {
					if (PlayerRaces.Visible[n]) {
						if (x == v) {
							break;
						}
						++x;
					}
				}
				if (n != PlayerRaces.Count) {
					GameSettings.Presets[num[i]].Race = PlayerRaces.Race[x];
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
		if (ServerSetupState.CompOpt[comp[i]] == 2) {		// closed..
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
**		Player selectors have changed.
**		Caution: Called by map change (initial = 1)!
**		Caution: Called by network events from clients (initial = 2)!
**		Caution: Called by button action on server (initial = 3)!
*/
local void MultiGamePlayerSelectorsUpdate(int initial)
{
	Menu *menu;
	int i;
	int h;
	int c;
	int avail;
	int ready;
	int plyrs;

	menu = FindMenu("menu-multi-setup");

	//		FIXME: What this has to do:
	//		Use lag gem as KICK button
	//  Notify clients about MAP change: (initial = 1...)

	DebugLevel3Fn("initial = %d\n" _C_ initial);

	//		Calculate available slots from pudinfo
	for (c = h = i = 0; i < PlayerMax; i++) {
		if (MenuMapInfo->PlayerType[i] == PlayerPerson) {
			h++;		// available interactive player slots
		}
		if (MenuMapInfo->PlayerType[i] == PlayerComputer) {
			c++;		// available computer player slots
		}
	}

	avail = h;
	plyrs = 0;
	//		Setup the player menu
	for (ready = i = 1; i < PlayerMax-1; i++) {
		if (initial == 1) {
			if (i < h) {
				ServerSetupState.CompOpt[i] = 0;
			}
			menu->Items[SERVER_PLAYER_READY - 1 + i].flags = 0;
			menu->Items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;

			menu->Items[SERVER_PLAYER_LAG - 1 + i].flags = 0;
			menu->Items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;

			// FIXME: don't forget to throw out additional players
			//		  without available slots here!

		}
		if (Hosts[i].PlyNr) {
			menu->Items[SERVER_PLAYER_STATE + i] = NetMultiButtonStorage[1];

			menu->Items[SERVER_PLAYER_READY - 1 + i].flags = 0;
			menu->Items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;
			++plyrs;
			if (ServerSetupState.Ready[i]) {
				menu->Items[SERVER_PLAYER_READY - 1 + i].d.gem.state |= MI_GSTATE_CHECKED;
				++ready;
			}

			menu->Items[SERVER_PLAYER_LAG - 1 + i].flags = 0;
			menu->Items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;
		} else {
			// don't allow network and button events to intercept server player's action on pulldown buttons!
			if (!(menu->Items[SERVER_PLAYER_STATE + i].flags & MenuButtonClicked)) {
				if (initial == 1 ||
					(initial == 2 && menu->Items[SERVER_PLAYER_STATE + i].mitype != MI_TYPE_PULLDOWN)) {
					menu->Items[SERVER_PLAYER_STATE + i] = NetMultiButtonStorage[0];
					menu->Items[SERVER_PLAYER_STATE + i].d.pulldown.state = 0;
					menu->Items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = ServerSetupState.CompOpt[i];
				}
			}
			if (i < h && ServerSetupState.CompOpt[i] != 0) {
				avail--;
			}

			menu->Items[SERVER_PLAYER_READY - 1 + i].flags = MenuButtonDisabled;
			menu->Items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;

			menu->Items[SERVER_PLAYER_LAG - 1 + i].flags = MenuButtonDisabled;
			menu->Items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
		}

		menu->Items[SERVER_PLAYER_STATE + i].yofs = 32 + (i & 7) * 22;
		if (i > 7) {
			menu->Items[SERVER_PLAYER_STATE + i].xofs = 320 + 40;
		}


		if (i >= h) {
			// Allow to switch off (close) preset ai-computer slots
			// FIXME: evaluate game type...
			if (initial == 1 && i < h + c) {
				menu->Items[SERVER_PLAYER_STATE + i].d.pulldown.state = 0;
				menu->Items[SERVER_PLAYER_STATE + i].d.pulldown.noptions = 2;
				menu->Items[SERVER_PLAYER_STATE + i].d.pulldown.options = mgptsoptions + 1;
				menu->Items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = 0;
				ServerSetupState.CompOpt[i] = 1;
				menu->Items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = ServerSetupState.CompOpt[i] - 1;
			}

			menu->Items[SERVER_PLAYER_READY - 1 + i].flags = MenuButtonDisabled;
			menu->Items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;

			menu->Items[SERVER_PLAYER_LAG - 1 + i].flags = MenuButtonDisabled;
			menu->Items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
		}

		if (i >= h + c) {
			menu->Items[SERVER_PLAYER_STATE + i].d.pulldown.state = MI_PSTATE_PASSIVE;
			menu->Items[SERVER_PLAYER_STATE + i].d.pulldown.defopt = 2;
			menu->Items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = 2;
			menu->Items[SERVER_PLAYER_STATE + i].flags = MenuButtonDisabled;
		}
	}


	//		Tell connect state machines how many interactive players we can have
	NetPlayers = avail;
	//		Check if all players are ready.
	DebugLevel0Fn("READY to START: AVAIL = %d, READY = %d\n" _C_ avail
			_C_ ready);

	// Disable the select scenario after players have joined.
	if (plyrs) {
		// disable Select Scenario button
		menu->Items[2].flags = MenuButtonDisabled;
	} else {
		// enable Select Scenario button
		menu->Items[2].flags = 0;
	}
	if (ready == avail) {
		if (menu->Items[3].flags == MenuButtonDisabled) {
			// enable start game button
			menu->Items[3].flags = 0;
		}
	} else {
		// disable start game button
		menu->Items[3].flags = MenuButtonDisabled;
	}

	if (MetaServerInUse) {
		ChangeGameServer();
	}
}

/**
**		Update client network menu.
*/
local void MultiClientUpdate(int initial)
{
	Menu *menu;
	int i;
	int h;
	int c;

	menu = FindMenu("menu-net-multi-client");

	//  Calculate available slots from pudinfo
	for (c = h = i = 0; i < PlayerMax; i++) {
		if (MenuMapInfo->PlayerType[i] == PlayerPerson) {
			h++;						// available interactive player slots
		}
		if (MenuMapInfo->PlayerType[i] == PlayerComputer) {
			c++;						// available computer player slots
		}
	}

	//
	//		Setup defaults, reset values.
	//
	if (initial) {
		menu->Items[CLIENT_PLAYER_STATE] = NetMultiButtonStorage[1];
		menu->Items[CLIENT_PLAYER_STATE].yofs = 32;
		memset(&ServerSetupState, 0, sizeof(ServerSetup));
		memset(&LocalSetupState, 0, sizeof(ServerSetup));
	}
	for (i = 1; i < PlayerMax - 1; i++) {
		DebugLevel3Fn("%d: %d %d\n" _C_ i _C_ Hosts[i].PlyNr
				_C_ NetLocalHostsSlot);
		//
		//		Johns: This works only if initial. Hosts[i].PlyNr is later lost.
		//
		if (Hosts[i].PlyNr || i == NetLocalHostsSlot) {
			menu->Items[CLIENT_PLAYER_STATE + i] = NetMultiButtonStorage[1];
			if (i == NetLocalHostsSlot) {
				menu->Items[CLIENT_PLAYER_READY - 1 + i].d.gem.state = 0;
			} else {
				menu->Items[CLIENT_PLAYER_READY - 1 + i].d.gem.state =
					MI_GSTATE_PASSIVE;
			}
		} else {
			menu->Items[CLIENT_PLAYER_STATE + i] = NetMultiButtonStorage[0];
			menu->Items[CLIENT_PLAYER_STATE + i].d.pulldown.state =
				MI_PSTATE_PASSIVE;
			menu->Items[CLIENT_PLAYER_STATE + i].d.pulldown.curopt =
				ServerSetupState.CompOpt[i];
			menu->Items[CLIENT_PLAYER_READY - 1 + i].d.gem.state =
				MI_GSTATE_INVISIBLE;
		}
		menu->Items[CLIENT_PLAYER_STATE + i].yofs = 32 + (i & 7) * 22;
		if (i > 7) {
			menu->Items[CLIENT_PLAYER_STATE + i].xofs = 320 + 40;
		}
		menu->Items[CLIENT_PLAYER_READY - 1 + i].flags = 0;

		if (ServerSetupState.Ready[i]) {
			menu->Items[CLIENT_PLAYER_READY - 1 + i].d.gem.state |=
				MI_GSTATE_CHECKED;
		} else {
			menu->Items[CLIENT_PLAYER_READY - 1 + i].d.gem.state &=
				~MI_GSTATE_CHECKED;
		}

#if 0
		if (i != NetLocalHostsSlot) {
		//if (i >= h) {
			menu->Items[CLIENT_PLAYER_STATE + i].d.pulldown.curopt =
				ServerSetupState.CompOpt[i];
		}
#endif

		// Unused slots are always disabled.
		if (i >= h + c) {
			menu->Items[CLIENT_PLAYER_READY - 1 + i].flags =
				MenuButtonDisabled;
			menu->Items[CLIENT_PLAYER_READY - 1 + i].d.gem.state =
				MI_GSTATE_INVISIBLE;
			menu->Items[CLIENT_PLAYER_STATE + i].d.pulldown.defopt =
				menu->Items[CLIENT_PLAYER_STATE + i].d.pulldown.curopt = 2;
			menu->Items[CLIENT_PLAYER_STATE + i].flags = MenuButtonDisabled;
		}
	}
}

/**
**		Multiplayer server menu init callback
*/
local void MultiGameSetupInit(Menuitem *mi)
{
	int i;
	int h;

	// FIXME: Remove this when .cm is supported
	if (*CurrentMapPath && strstr(CurrentMapPath, ".cm\0")) {
		*CurrentMapPath = '\0';
	}

	GameSetupInit(mi);
	NetworkInitServerConnect();
	mi->menu->Items[SERVER_PLAYER_STATE] = NetMultiButtonStorage[1];
	mi->menu->Items[SERVER_PLAYER_STATE].yofs = 32;
	MultiGameFWSAction(NULL, mi->menu->Items[27].d.pulldown.defopt);

	memset(&ServerSetupState, 0, sizeof(ServerSetup));
	//		Calculate available slots from pudinfo
	for (h = i = 0; i < PlayerMax; i++) {
		if (MenuMapInfo->PlayerType[i] == PlayerPerson) {
			h++;		// available interactive player slots
		}
	}
	for (i = h; i < PlayerMax - 1; i++) {
		ServerSetupState.CompOpt[i] = 1;
	}
	MultiGamePlayerSelectorsUpdate(1);
	DebugLevel3Fn("h = %d, NetPlayers = %d\n" _C_ h _C_ NetPlayers);

	if (MetaServerInUse) {
		ChangeGameServer();
	}

}

/**
**		Multiplayer server menu exit callback
*/
local void MultiGameSetupExit(Menuitem *mi)
{
	int i;

	// ugly hack to prevent NetMultiButtonStorage[0].d.pulldown.options
	// from being freed
	for (i=0; i<PlayerMax-1; ++i) {
		mi->menu->Items[SERVER_PLAYER_STATE + i] = NetMultiButtonStorage[1];
	}
}

/**
**		Cancel button of server multi player menu pressed.
*/
local void MultiGameCancel(void)
{
	NetworkExitServerConnect();

	if (MetaServerInUse) {
		SendMetaCommand("AbandonGame", "");
	}

	NetPlayers = 0;				// Make single player menus work again!
	GameCancel();
}

/**
**		Draw the multi player setup menu.
*/
local void NetMultiPlayerDrawFunc(Menuitem *mi)
{
	Menu *menu;
	int i;
	char* nc;
	char* rc;

	menu = FindMenu("menu-multi-setup");
	i = mi - menu->Items - SERVER_PLAYER_STATE;
	if (i >= 0 && i < PlayerMax - 1) {				// Ugly test to detect server
		if (i > 0) {
			menu->Items[SERVER_PLAYER_READY - 1 + i].flags &=
				~MenuButtonDisabled;
			// Note: re-disabled in MultiGamePlayerSelectorsUpdate()
			//				for kicked out clients!!
			if (ServerSetupState.Ready[i]) {
				menu->Items[SERVER_PLAYER_READY - 1 + i]
						.d.gem.state = MI_GSTATE_PASSIVE|MI_GSTATE_CHECKED;
			} else {
				menu->Items[SERVER_PLAYER_READY - 1 + i]
						.d.gem.state = MI_GSTATE_PASSIVE;
			}
			if (ServerSetupState.LastFrame[i] + 30 > FrameCounter) {
				menu->Items[SERVER_PLAYER_LAG - 1 + i].flags &=
					~MenuButtonDisabled;
				menu->Items[SERVER_PLAYER_LAG - 1 + i]
						.d.gem.state = MI_GSTATE_PASSIVE|MI_GSTATE_CHECKED;
			} else {
				menu->Items[SERVER_PLAYER_LAG - 1 + i].flags |=
					MenuButtonDisabled;
				menu->Items[SERVER_PLAYER_LAG - 1 + i]
						.d.gem.state = MI_GSTATE_PASSIVE;
			}

		}
	} else {
		menu = FindMenu("menu-net-multi-client");
		i = mi - menu->Items - CLIENT_PLAYER_STATE;
		if (i > 0) {
			menu->Items[CLIENT_PLAYER_READY - 1 + i].flags &=
				~MenuButtonDisabled;
			if (i == NetLocalHostsSlot) {
				menu->Items[CLIENT_PLAYER_READY - 1 + i].
						d.gem.state &= ~MI_GSTATE_PASSIVE;
			} else {
				menu->Items[CLIENT_PLAYER_READY - 1 + i].
						d.gem.state |= MI_GSTATE_PASSIVE;
			}
		}
	}

	GetDefaultTextColors(&nc, &rc);
	SetDefaultTextColors(rc, rc);
	DebugLevel3Fn("Hosts[%d].PlyName = %s\n" _C_ i _C_ Hosts[i].PlyName);
	VideoDrawText(TheUI.Offset640X + mi->xofs, TheUI.Offset480Y + mi->yofs, GameFont, Hosts[i].PlyName);

	SetDefaultTextColors(nc, rc);
}

/**
**		Cancel button of multiplayer client menu pressed.
*/
local void MultiClientCancel(void)
{
	NetworkDetachFromServer();
	// GameCancel();
}

/**
**		Multiplayer client menu init callback
*/
local void MultiGameClientInit(Menuitem *mi)
{
	// GameSetupInit(mi);
	MultiClientUpdate(1);
	if (LocalSetupState.Ready[NetLocalHostsSlot]) {
		mi->menu->Items[2].flags = MenuButtonDisabled;
		mi->menu->Items[3].flags = 0;
		mi->menu->Items[CLIENT_RACE].flags = MenuButtonDisabled;
	} else {
		mi->menu->Items[3].flags = MenuButtonDisabled;
		mi->menu->Items[2].flags = 0;
		mi->menu->Items[CLIENT_RACE].flags = 0;
	}
}

/**
**		Multiplayer client menu exit callback
*/
local void MultiGameClientExit(Menuitem *mi)
{
	int i;

	// ugly hack to prevent NetMultiButtonStorage[0].d.pulldown.options
	// from being freed
	for (i = 0; i < PlayerMax-1; ++i) {
		mi->menu->Items[CLIENT_PLAYER_STATE + i] = NetMultiButtonStorage[1];
	}
}

/**
**		Multiplayer client gem action. Toggles ready flag.
*/
local void MultiClientGemAction(Menuitem *mi)
{
	int i;

	i = mi - mi->menu->Items - CLIENT_PLAYER_READY + 1;
	DebugLevel3Fn("i = %d, NetLocalHostsSlot = %d\n" _C_ i _C_ NetLocalHostsSlot);
	if (i == NetLocalHostsSlot) {
		LocalSetupState.Ready[i] = !LocalSetupState.Ready[i];
		if (LocalSetupState.Ready[i]) {
			mi->menu->Items[2].flags = MenuButtonDisabled;
			mi->menu->Items[3].flags = 0;
			mi->menu->Items[CLIENT_RACE].flags = MenuButtonDisabled;
		} else {
			mi->menu->Items[3].flags = MenuButtonDisabled;
			mi->menu->Items[2].flags = 0;
			mi->menu->Items[CLIENT_RACE].flags = 0;
		}
		MultiClientUpdate(0);
	}
}

/**
**		Multiplayer client races action callback
*/
local void MultiClientRCSAction(Menuitem *mi, int i)
{
	if (mi->d.pulldown.curopt == i) {
		LocalSetupState.Race[NetLocalHostsSlot] = mi->d.pulldown.noptions - 1 - i;
		MultiClientUpdate(0);
	}
}

/**
**		Multiplayer client ready button
*/
local void MultiClientReady(void)
{
	Menu *menu;

	menu = FindMenu("menu-net-multi-client");
	menu->Items[2].flags = MenuButtonDisabled;
	menu->Items[3].flags = 0;
	menu->Items[CLIENT_RACE].flags = MenuButtonDisabled;
	LocalSetupState.Ready[NetLocalHostsSlot] = 1;
	MultiClientUpdate(0);
}

/**
**		Multiplayer client not ready button
*/
local void MultiClientNotReady(void)
{
	Menu *menu;

	menu = FindMenu("menu-net-multi-client");
	menu->Items[3].flags = MenuButtonDisabled;
	menu->Items[2].flags = 0;
	menu->Items[CLIENT_RACE].flags = 0;
	LocalSetupState.Ready[NetLocalHostsSlot] = 0;
	MultiClientUpdate(0);
}

/**
**		Callback from netconnect loop in Client-Sync state:
**		Compare local state with server's information
**		and force update when changes have occured.
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

/**
**		FIXME: docu
*/
global int NetClientSelectScenario(void)
{
	char *cp;

	FreeMapInfo(MenuMapInfo);
	MenuMapInfo = NULL;

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

	if (strcasestr(ScenSelectFileName, ".pud")) {
		MenuMapInfo = GetPudInfo(MenuMapFullPath);
	} else {
		// FIXME: GetCmInfo();
	}
	return MenuMapInfo == NULL;
}

/**
**		FIXME: docu
*/
global void NetConnectForceDisplayUpdate(void)
{
	MultiGamePlayerSelectorsUpdate(2);
}

/**
**		Update client menu to follow server menu.
*/
global void NetClientUpdateState(void)
{
	Menu *menu;

	menu = FindMenu("menu-net-multi-client");

	GameRESAction(NULL, ServerSetupState.ResOpt);
	menu->Items[CLIENT_RESOURCE].d.pulldown.curopt =
		ServerSetupState.ResOpt;

	GameUNSAction(NULL, ServerSetupState.UnsOpt);
	menu->Items[CLIENT_UNITS].d.pulldown.curopt =
		ServerSetupState.UnsOpt;

	MultiGameFWSAction(NULL, ServerSetupState.FwsOpt);
	menu->Items[CLIENT_FOG_OF_WAR].d.pulldown.curopt =
		ServerSetupState.FwsOpt;

	GameTSSAction(NULL, ServerSetupState.TssOpt);
	menu->Items[CLIENT_TILESET].d.pulldown.curopt =
		ServerSetupState.TssOpt;

	GameGATAction(NULL, ServerSetupState.GaTOpt);
	menu->Items[CLIENT_GAMETYPE].d.pulldown.curopt =
		ServerSetupState.GaTOpt;

	MultiClientUpdate(0);
	DebugLevel1Fn("MultiClientMenuRedraw\n");
}

/**
**		Start editor.
*/
local void StartEditor(void)
{
	MenusSetBackground();
	Invalidate();

	SetupEditor();

	EditorRunning = 1;
	EndMenu();
}

/**
**		Setup Editor Paths
*/
global void SetupEditor(void)
{
	char *s;
	//
	//  Create a default path + map.
	//
	if (!*CurrentMapPath || *CurrentMapPath == '.' || *CurrentMapPath == '/') {
		strcpy(CurrentMapPath, DefaultMap);
	}

	//
	//		Use the last path.
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
**		Editor select cancel button callback
*/
local void EditorSelectCancel(void)
{
	QuitToMenu = 1;
	EditorRunning = 0;
	EndMenu();
}

/**
**		Called from menu, for new editor map.
*/
local void EditorNewMap(void)
{
	Menu *menu;
	char width[10];
	char height[10];
	char description[36];

	MenusSetBackground();
	Invalidate();

	EditorCancelled = 0;

	menu = FindMenu("menu-editor-new");
	menu->Items[2].d.input.buffer = description;
	strcpy(description, "~!_");
	menu->Items[2].d.input.nch = strlen(description) - 3;
	menu->Items[2].d.input.maxch = 31;
	menu->Items[4].d.input.buffer = width;
	strcpy(width, "128~!_");
	menu->Items[4].d.input.nch = strlen(width) - 3;
	menu->Items[4].d.input.maxch = 4;
	menu->Items[5].d.input.buffer = height;
	strcpy(height, "128~!_");
	menu->Items[5].d.input.nch = strlen(width) - 3;
	menu->Items[5].d.input.maxch = 4;
	menu->Items[7].d.pulldown.noptions = NumTilesets;
	ProcessMenu("menu-editor-new", 1);

	if (EditorCancelled) {
		MenusSetBackground();
		return;
	}

	TheMap.Info = calloc(1, sizeof(MapInfo));
	description[strlen(description) - 3] = '\0';
	TheMap.Info->Description = strdup(description);
	TheMap.Info->MapTerrain = menu->Items[7].d.pulldown.curopt;
	TheMap.Info->MapWidth = atoi(width);
	TheMap.Info->MapHeight = atoi(height);

	VideoClearScreen();

	*CurrentMapPath = '\0';

	GuiGameStarted = 1;
	EndMenu();
}

/**
**		Editor new map draw func
*/
local void EditorNewDrawFunc(Menuitem *mi __attribute__((unused)))
{
	MenusSetBackground();
}

/**
**		Editor new map, map description input box callback
*/
local void EditorNewMapDescriptionEnterAction(
		Menuitem *mi __attribute__((unused)), int key __attribute__((unused)))
{
}

/**
**		Editor new map, size input box callback
*/
local void EditorNewMapSizeEnterAction(Menuitem * mi,
		int key __attribute__((unused)))
{
	if (mi->d.input.nch > 0
			&& !isdigit(mi->d.input.buffer[mi->d.input.nch - 1])) {
		strcpy(mi->d.input.buffer + (--mi->d.input.nch), "~!_");
	}
}

/**
**		Editor new map ok button
*/
local void EditorNewOk(void)
{
	Menu *menu;
	unsigned value1;
	unsigned value2;

	menu = CurrentMenu;
	value1 = atoi(menu->Items[4].d.input.buffer);
	value2 = atoi(menu->Items[5].d.input.buffer);

	if (value1 < 32 || value2 < 32) {
		if (value1 < 32) {
			sprintf(menu->Items[4].d.input.buffer, "32~!_");
			menu->Items[4].d.input.nch = strlen(menu->Items[4].d.text.text) - 3;
		}
		if (value2 < 32) {
			sprintf(menu->Items[5].d.input.buffer, "32~!_");
			menu->Items[5].d.input.nch = strlen(menu->Items[5].d.text.text) - 3;
		}
		ErrorMenu("Size smaller than 32");
	} else if (value1 > 1024 || value2 > 1024) {
		if (value1 == 0) {
			sprintf(menu->Items[4].d.input.buffer, "1024~!_");
			menu->Items[4].d.input.nch = strlen(menu->Items[4].d.text.text) - 3;
		}
		if (value2 == 0) {
			sprintf(menu->Items[5].d.input.buffer, "1024~!_");
			menu->Items[5].d.input.nch = strlen(menu->Items[5].d.text.text) - 3;
		}
		ErrorMenu("Size larger than 1024");
	} else if (value1 / 32 * 32 != value1 || value2/32*32 != value2) {
		if (value1 / 32 * 32 != value1) {
			sprintf(menu->Items[4].d.input.buffer, "%d~!_", (value1 + 16) / 32 * 32);
			menu->Items[4].d.input.nch = strlen(menu->Items[4].d.text.text) - 3;
		}
		if (value2 / 32 * 32 != value2) {
			sprintf(menu->Items[5].d.input.buffer, "%d~!_", (value2 + 16) / 32 * 32);
			menu->Items[5].d.input.nch = strlen(menu->Items[5].d.text.text) - 3;
		}
		ErrorMenu("Size must be a multiple of 32");
	}
	else {
		EndMenu();
	}
}

/**
**		Editor new map cancel button
*/
local void EditorNewCancel(void)
{
	EditorCancelled = 1;
	EndMenu();
}

/**
**		Editor main load map menu
*/
local void EditorMainLoadMap(void)
{
	char *p;
	char *s;

	EditorCancelled = 0;
	ProcessMenu("menu-editor-main-load-map", 1);
	GetInfoFromSelectPath();

	if (EditorCancelled) {
		MenusSetBackground();
		return;
	}

	VideoClearScreen();

	if (ScenSelectPath[0]) {
		s = ScenSelectPath + strlen(ScenSelectPath);
		*s = '/';
		strcpy(s+1, ScenSelectFileName);		// Final map name with path
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
**		Editor main load init callback
*/
local void EditorMainLoadInit(Menuitem *mi)
{
	DebugCheck(!*ScenSelectPath);
	mi->menu->Items[5].flags =
		*ScenSelectDisplayPath ? 0 : MenuButtonDisabled;
	mi->menu->Items[5].d.button.text = ScenSelectDisplayPath;
	DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);
}

/**
**		Editor main load listbox init callback
*/
local void EditorMainLoadLBInit(Menuitem *mi)
{
	int i;

	EditorMainLoadLBExit(mi);
	i = mi->d.listbox.noptions = ReadDataDirectory(ScenSelectPath, EditorMainLoadRDFilter,
		(FileList **) & (mi->d.listbox.options));

	if (i == 0) {
		free(mi->menu->Items[3].d.button.text);
		mi->menu->Items[3].d.button.text = strdup("OK");
		mi->menu->Items[3].flags |= MenuButtonDisabled;
	} else {
		EditorMainLoadLBAction(mi, 0);
		mi->menu->Items[3].flags &= ~MenuButtonDisabled;
		if (i > mi->d.listbox.nlines) {
			mi[1].flags &= ~MenuButtonDisabled;
		}
	}
}

/**
**		Editor main load listbox exit callback
*/
local void EditorMainLoadLBExit(Menuitem *mi)
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

/**
**		Editor main load read directory filter
*/
local int EditorMainLoadRDFilter(char *pathbuf, FileList *fl)
{
	MapInfo *info;
	char *suf[4];
	char *np;
	char *cp;
	char *lcp;
	unsigned u;

	np = strrchr(pathbuf, '/');
	if (np) {
		np++;
	} else {
		np = pathbuf;
	}
	fl->type = -1;
	suf[0] = ".pud";
	suf[1] = ".scm";
	suf[2] = ".chk";
	suf[3] = NULL;
	u = 0;
	lcp = 0;
	while (suf[u]) {
		cp = np;
		--cp;
		do {
			lcp = cp++;
			cp = strcasestr(cp, suf[u]);
		} while (cp != NULL);
		if (lcp >= np) {
			break;
		}
		++u;
	}
	if (!suf[u]) {
		return 0;
	}

	if (lcp >= np) {
		cp = lcp + strlen(suf[u]);
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
			if (strcasestr(np, ".pud")) {
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

/**
**		Editor main load folder button
*/
local void EditorMainLoadFolder(void)
{
	Menu *menu;
	Menuitem *mi;
	char *cp;

	menu = CurrentMenu;
	mi = &menu->Items[1];

	if (ScenSelectDisplayPath[0]) {
		cp = strrchr(ScenSelectDisplayPath, '/');
		if (cp) {
			*cp = 0;
		} else {
			ScenSelectDisplayPath[0] = 0;
			menu->Items[5].flags |= MenuButtonDisabled;
			menu->Items[5].d.button.text = NULL;
		}
		cp = strrchr(ScenSelectPath, '/');
		if (cp) {
			*cp = 0;
			EditorMainLoadLBInit(mi);
			mi->d.listbox.cursel = -1;
			mi->d.listbox.startline = 0;
			mi->d.listbox.curopt = 0;
			mi[1].d.vslider.percent = 0;
			mi[1].d.hslider.percent = 0;
		}
	}
}

/**
**		Editor main load ok button
*/
local void EditorMainLoadOk(void)
{
	Menu *menu;
	Menuitem *mi;
	FileList *fl;
	int i;

	menu = CurrentMenu;
	mi = &menu->Items[1];
	i = mi->d.listbox.curopt + mi->d.listbox.startline;
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (fl[i].type == 0) {
			strcat(ScenSelectPath, "/");
			strcat(ScenSelectPath, fl[i].name);
			if (menu->Items[5].flags & MenuButtonDisabled) {
				menu->Items[5].flags &= ~MenuButtonDisabled;
				menu->Items[5].d.button.text = ScenSelectDisplayPath;
			} else {
				strcat(ScenSelectDisplayPath, "/");
			}
			strcat(ScenSelectDisplayPath, fl[i].name);
			EditorMainLoadLBInit(mi);
			mi->d.listbox.cursel = -1;
			mi->d.listbox.startline = 0;
			mi->d.listbox.curopt = 0;
			mi[1].d.vslider.percent = 0;
			mi[1].d.hslider.percent = 0;
		} else {
			strcpy(ScenSelectFileName, fl[i].name);		// Final map name
			EndMenu();
			menu->Items[5].d.button.text = NULL;
		}
	}
}

/**
**		Editor main load cancel button
*/
local void EditorMainLoadCancel(void)
{
	char *s;

	EditorCancelled=1;

	//
	//  Use last selected map.
	//
	DebugLevel0Fn("Map   path: %s\n" _C_ CurrentMapPath);
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

	DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);

	CurrentMenu->Items[5].d.button.text = NULL;

	EndMenu();
}

/**
**		Editor main load listbox retrieve callback
*/
local unsigned char *EditorMainLoadLBRetrieve(Menuitem *mi, int i)
{
	FileList *fl;
	MapInfo *info;
	static char buffer[1024];
	int j;
	int n;

	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (fl[i].type) {
			if (i - mi->d.listbox.startline == mi->d.listbox.curopt) {
				if ((info = fl[i].xdata)) {
					if (info->Description) {
						VideoDrawText(mi->menu->X + 8, mi->menu->Y + 234, LargeFont, info->Description);
					}
					sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
					VideoDrawText(mi->menu->X + 8, mi->menu->Y + 234 + 20, LargeFont, buffer);
					for (n = j = 0; j < PlayerMax; j++) {
						if (info->PlayerType[j] == PlayerPerson) {
							n++;
						}
					}
					if (n == 1) {
						VideoDrawText(mi->menu->X + 8, mi->menu->Y + 234 + 40, LargeFont, "1 player");
					} else {
						sprintf(buffer, "%d players", n);
						VideoDrawText(mi->menu->X + 8, mi->menu->Y + 234 + 40, LargeFont, buffer);
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

/**
**		Editor main load listbox action callback
*/
local void EditorMainLoadLBAction(Menuitem *mi, int i)
{
	FileList *fl;

	DebugCheck(i<0);
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		free(mi->menu->Items[3].d.button.text);
		if (fl[i].type) {
			mi->menu->Items[3].d.button.text = strdup("OK");
		} else {
			mi->menu->Items[3].d.button.text = strdup("Open");
		}
		if (mi->d.listbox.noptions > mi->d.listbox.nlines) {
			mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
			mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
		}
	}
}

/**
**		Editor main load vertical slider action callback
*/
local void EditorMainLoadVSAction(Menuitem *mi, int i)
{
	int op;
	int d1;
	int d2;

	mi--;
	switch (i) {
		case 0:				// click - down
		case 2:				// key - down
			if (mi[1].d.vslider.cflags & MI_CFLAGS_DOWN) {
				if (mi->d.listbox.curopt + mi->d.listbox.startline + 1 < mi->d.listbox.noptions) {
					mi->d.listbox.curopt++;
					if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
						mi->d.listbox.curopt--;
						mi->d.listbox.startline++;
					}
				}
			} else if (mi[1].d.vslider.cflags & MI_CFLAGS_UP) {
				if (mi->d.listbox.curopt + mi->d.listbox.startline > 0) {
					mi->d.listbox.curopt--;
					if (mi->d.listbox.curopt < 0) {
						mi->d.listbox.curopt++;
						mi->d.listbox.startline--;
					}
				}
			}
			EditorMainLoadLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			if (i == 2) {
				mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
			}
			break;
		case 1:				// mouse - move
			if (mi[1].d.vslider.cflags & MI_CFLAGS_KNOB && (mi[1].flags & MenuButtonClicked)) {
				if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt + mi->d.listbox.startline + 1 < mi->d.listbox.noptions) {
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
							if (mi->d.listbox.curopt + mi->d.listbox.startline + 1 == mi->d.listbox.noptions) {
								break;
							}
						}
					}
				} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt + mi->d.listbox.startline > 0) {
						for (;;) {
							op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
									 (mi->d.listbox.noptions - 1);
							d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
							d2 = mi[1].d.vslider.curper - op;
							if (d2 >= d1) {
								break;
							}
							mi->d.listbox.curopt--;
							if (mi->d.listbox.curopt < 0) {
								mi->d.listbox.curopt++;
								mi->d.listbox.startline--;
							}
							if (mi->d.listbox.curopt+mi->d.listbox.startline == 0) {
								break;
							}
						}
					}
				}

				DebugCheck(mi->d.listbox.startline < 0);
				DebugCheck(mi->d.listbox.noptions > 0 &&
					mi->d.listbox.startline+mi->d.listbox.curopt >= mi->d.listbox.noptions);

				EditorMainLoadLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			}
			break;
		default:
			break;
	}
}

/**
**		Editor load map menu
*/
global void EditorLoadMenu(void)
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
		strcpy(s + 1, ScenSelectFileName);		// Final map name with path
		p = ScenSelectPath + strlen(StratagusLibPath) + 1;
		strcpy(CurrentMapPath, p);
		*s = '\0';
	} else {
		strcpy(CurrentMapPath, ScenSelectFileName);
	}

	EditorMapLoaded = 1;
	EditorRunning = 0;
	EndMenu();
}

/**
**		Editor main load ok button
*/
local void EditorLoadOk(void)
{
	Menu *menu;
	Menuitem *mi;
	FileList *fl;
	int i;

	menu = CurrentMenu;
	mi = &menu->Items[1];
	i = mi->d.listbox.curopt + mi->d.listbox.startline;
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (fl[i].type == 0) {
			strcat(ScenSelectPath, "/");
			strcat(ScenSelectPath, fl[i].name);
			if (menu->Items[5].flags & MenuButtonDisabled) {
				menu->Items[5].flags &= ~MenuButtonDisabled;
				menu->Items[5].d.button.text = ScenSelectDisplayPath;
			} else {
				strcat(ScenSelectDisplayPath, "/");
			}
			strcat(ScenSelectDisplayPath, fl[i].name);
			EditorMainLoadLBInit(mi);
			mi->d.listbox.cursel = -1;
			mi->d.listbox.startline = 0;
			mi->d.listbox.curopt = 0;
			mi[1].d.vslider.percent = 0;
			mi[1].d.hslider.percent = 0;
		} else {
			strcpy(ScenSelectFileName, fl[i].name);		// Final map name
			EndMenu();
			menu->Items[5].d.button.text = NULL;
		}
	}
}

/**
**		Editor main load cancel button
*/
local void EditorLoadCancel(void)
{
	char *s;

	EditorCancelled = 1;

	//
	//  Use last selected map.
	//
	DebugLevel0Fn("Map   path: %s\n" _C_ CurrentMapPath);
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

	DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);

	CurrentMenu->Items[5].d.button.text = NULL;

	EditorEndMenu();
}

/**
**		Editor map properties menu
*/
local void EditorMapPropertiesMenu(void)
{
	Menu *menu;
	char description[36];
	char size[30];

	menu = FindMenu("menu-editor-map-properties");

	menu->Items[2].d.input.buffer = description;
	strcpy(description, TheMap.Info->Description);
	strcat(description, "~!_");
	menu->Items[2].d.input.nch = strlen(description) - 3;
	menu->Items[2].d.input.maxch = 31;

	sprintf(size, "%d x %d", TheMap.Width, TheMap.Height);
	menu->Items[4].d.text.text = size;

	menu->Items[6].d.pulldown.defopt = TheMap.Terrain;

	// FIXME: Set the correct pud version
	menu->Items[8].d.pulldown.defopt = 1;
	menu->Items[8].flags = -1;

	ProcessMenu("menu-editor-map-properties", 1);

	menu->Items[4].d.text.text = NULL;
}

/**
**		Editor map properties input box callback
*/
local void EditorMapPropertiesEnterAction(
		Menuitem *mi __attribute__((unused)), int key __attribute__((unused)))
{
}

/**
**		Editor map properties ok button
*/
local void EditorMapPropertiesOk(void)
{
	Menu *menu;
	char *description;
	int old;

	menu = CurrentMenu;

	description = menu->Items[2].d.input.buffer;
	description[strlen(description)-3] = '\0';
	free(TheMap.Info->Description);
	TheMap.Info->Description = strdup(description);

	// Change the terrain
	old = TheMap.Info->MapTerrain;
	if (old != menu->Items[6].d.pulldown.curopt) {
		TheMap.Info->MapTerrain = menu->Items[6].d.pulldown.curopt;
		free(TheMap.Info->MapTerrainName);
		TheMap.Info->MapTerrainName = strdup(TilesetWcNames[TheMap.Info->MapTerrain]);
		TheMap.Terrain = TheMap.Info->MapTerrain;
		free(TheMap.TerrainName);
		TheMap.TerrainName = strdup(TilesetWcNames[TheMap.Info->MapTerrain]);
		TheMap.Tileset = Tilesets[TheMap.Info->MapTerrain];

		LoadTileset();
		ChangeTilesetPud(old, &TheMap);
		SetPlayersPalette();
		PreprocessMap();
		LoadConstructions();
		LoadUnitTypes();
		LoadIcons();
		UpdateMinimapTerrain();
	}

	// FIXME: Save the pud version somewhere

	EditorEndMenu();
}

/**
**		Editor player properties draw func
*/
local void EditorPlayerPropertiesDrawFunc(Menuitem *mi __attribute__((unused)))
{
	MenusSetBackground();
}

/**
**		Editor player properties input box callback
*/
local void EditorPlayerPropertiesEnterAction(Menuitem *mi,
		int key __attribute__((unused)))
{
	if (mi->d.input.nch > 0 && !isdigit(mi->d.input.buffer[mi->d.input.nch - 1])) {
		strcpy(mi->d.input.buffer + (--mi->d.input.nch), "~!_");
	}
}

	/// Player type conversion from internal fc to menu number
local int PlayerTypesFcToMenu[] = {
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
local int PlayerTypesMenuToFc[] = {
	PlayerPerson,
	PlayerComputer,
	PlayerRescuePassive,
	PlayerRescueActive,
	PlayerNeutral,
	PlayerNobody,
};

/**
**		Convert player ai from internal fc number to menu number
**
**		@param num		Ai number
*/
local int PlayerAiFcToMenu(int num)
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
	DebugLevel0Fn("Invalid Ai number: %d\n" _C_ num);
	return -1;
}

/**
**		Convert player ai from menu number to internal fc number
**
**		@param num		Ai number
*/
local int PlayerAiMenuToFc(int num)
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
	DebugLevel0Fn("Invalid Ai number: %d\n" _C_ num);
	return -1;
}

/**
**		Edit player properties menu
*/
local void EditorPlayerPropertiesMenu(void)
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
		menu->Items[RACE_POSITION + i].d.pulldown.defopt = TheMap.Info->PlayerSide[i];
		menu->Items[TYPE_POSITION + i].d.pulldown.defopt = PlayerTypesFcToMenu[TheMap.Info->PlayerType[i]];
		menu->Items[AI_POSITION + i].d.pulldown.defopt = PlayerAiFcToMenu(TheMap.Info->PlayerAi[i]);
		sprintf(gold[i], "%d~!_", TheMap.Info->PlayerResources[i][GoldCost]);
		sprintf(lumber[i], "%d~!_", TheMap.Info->PlayerResources[i][WoodCost]);
		sprintf(oil[i], "%d~!_", TheMap.Info->PlayerResources[i][OilCost]);
		menu->Items[GOLD_POSITION + i].d.input.buffer = gold[i];
		menu->Items[GOLD_POSITION + i].d.input.nch = strlen(gold[i]) - 3;
		menu->Items[GOLD_POSITION + i].d.input.maxch = 7;
		menu->Items[LUMBER_POSITION + i].d.input.buffer = lumber[i];
		menu->Items[LUMBER_POSITION + i].d.input.nch = strlen(lumber[i]) - 3;
		menu->Items[LUMBER_POSITION + i].d.input.maxch = 7;
		menu->Items[OIL_POSITION + i].d.input.buffer = oil[i];
		menu->Items[OIL_POSITION + i].d.input.nch = strlen(oil[i]) - 3;
		menu->Items[OIL_POSITION + i].d.input.maxch = 7;
	}

	ProcessMenu("menu-editor-player-properties", 1);

	for (i = 0; i < PlayerMax; ++i) {
		TheMap.Info->PlayerSide[i] = menu->Items[RACE_POSITION + i].d.pulldown.curopt;
		TheMap.Info->PlayerType[i] = PlayerTypesMenuToFc[menu->Items[TYPE_POSITION + i].d.pulldown.curopt];
		TheMap.Info->PlayerAi[i] = PlayerAiMenuToFc(menu->Items[AI_POSITION + i].d.pulldown.curopt);
		TheMap.Info->PlayerResources[i][GoldCost] = atoi(gold[i]);
		TheMap.Info->PlayerResources[i][WoodCost] = atoi(lumber[i]);
		TheMap.Info->PlayerResources[i][OilCost] = atoi(oil[i]);
	}
}

/**
**		Edit resource properties
*/
global void EditorEditResource(void)
{
	Menu *menu;
	char buf[13];
	char buf2[32];

	menu = FindMenu("menu-editor-edit-resource");

	sprintf(buf2,"Amount of %s:",DefaultResourceNames[UnitUnderCursor->Type->GivesResource]);
	menu->Items[0].d.text.text = buf2;
	sprintf(buf, "%d~!_", UnitUnderCursor->Value);
	menu->Items[1].d.input.buffer = buf;
	menu->Items[1].d.input.nch = strlen(buf) - 3;
	menu->Items[1].d.input.maxch = 6;
	ProcessMenu("menu-editor-edit-resource", 1);
	menu->Items[0].d.text.text = NULL;
}

/**
**		Key pressed in menu-editor-edit-resource
*/
local void EditorEditResourceEnterAction(Menuitem *mi,int key)
{
	if (mi->d.input.nch > 0 && !isdigit(mi->d.input.buffer[mi->d.input.nch - 1])) {
		strcpy(mi->d.input.buffer + (--mi->d.input.nch), "~!_");
	} else if (key==10 || key==13) {
		EditorEditResourceOk();
	}
}

/**
**		Ok button from menu-editor-edit-resource
*/
local void EditorEditResourceOk(void)
{
	Menu *menu;
	unsigned value;

	menu = CurrentMenu;
	value = atoi(menu->Items[1].d.input.buffer);
	if (value < 2500) {
		strcpy(menu->Items[1].d.text.text, "2500~!_");
		menu->Items[1].d.input.nch = strlen(menu->Items[1].d.text.text) - 3;
		menu = FindMenu("menu-editor-error");
		menu->Items[1].d.text.text = "Must be greater than 2500";
		ProcessMenu("menu-editor-error", 1);
		menu->Items[1].d.text.text = NULL;
	} else if (value > 655000) {
		strcpy(menu->Items[1].d.text.text, "655000~!_");
		menu->Items[1].d.input.nch = strlen(menu->Items[1].d.text.text) - 3;
		menu = FindMenu("menu-editor-error");
		menu->Items[1].d.text.text = "Must be smaller than 655000";
		ProcessMenu("menu-editor-error", 1);
		menu->Items[1].d.text.text = NULL;
	} else if (value / 2500 * 2500 != value) {
		value = (value + 1250)/ 2500 * 2500;
		sprintf(menu->Items[1].d.text.text, "%d~!_", value);
		menu->Items[1].d.input.nch = strlen(menu->Items[1].d.text.text) - 3;
		menu = FindMenu("menu-editor-error");
		menu->Items[1].d.text.text = "Must be a multiple of 2500";
		ProcessMenu("menu-editor-error", 1);
		menu->Items[1].d.text.text = NULL;
	} else {
		UnitUnderCursor->Value = value;
		GameMenuReturn();
	}
}

/**
**		Cancel button from menu-editor-edit-resource
*/
local void EditorEditResourceCancel(void)
{
	GameMenuReturn();
}

/**
**		Edit ai properties
*/
global void EditorEditAiProperties(void)
{
	Menu *menu;

	menu = FindMenu("menu-editor-edit-ai-properties");
	if (UnitUnderCursor->Active) {
		menu->Items[1].d.gem.state = MI_GSTATE_CHECKED;
		menu->Items[3].d.gem.state = MI_GSTATE_UNCHECKED;
	} else {
		menu->Items[1].d.gem.state = MI_GSTATE_UNCHECKED;
		menu->Items[3].d.gem.state = MI_GSTATE_CHECKED;
	}

	ProcessMenu("menu-editor-edit-ai-properties", 1);
}

/**
**		Active or Passive gem clicked in menu-editor-edit-ai-properties
*/
local void EditorEditAiPropertiesGem(Menuitem *mi)
{
	if (&mi->menu->Items[1] == mi) {
		mi->d.gem.state = MI_GSTATE_CHECKED;
		mi->menu->Items[3].d.gem.state = MI_GSTATE_UNCHECKED;
	} else {
		mi->d.gem.state = MI_GSTATE_CHECKED;
		mi->menu->Items[1].d.gem.state = MI_GSTATE_UNCHECKED;
	}
}

/**
**		Ok button from menu-editor-edit-ai-properties
*/
local void EditorEditAiPropertiesOk(void)
{
	Menu *menu;

	menu = CurrentMenu;
	if (menu->Items[1].d.gem.state == MI_GSTATE_CHECKED) {
		UnitUnderCursor->Active = 1;
	} else {
		UnitUnderCursor->Active = 0;
	}
	GameMenuReturn();
}

/**
**		Cancel button from menu-editor-edit-ai-properties
*/
local void EditorEditAiPropertiesCancel(void)
{
	GameMenuReturn();
}

/**
**		Save map from the editor
**
**		@return				0 for success, -1 for error
*/
global int EditorSaveMenu(void)
{
	Menu *menu;
	char path[PATH_MAX];
	char *s;
	char *p;
	int ret;

	ret = 0;
	menu = FindMenu("menu-editor-save");

	EditorCancelled = 0;

	menu->Items[3].d.input.buffer = path;
	menu->Items[3].d.input.maxch = PATH_MAX - 4;

	DebugCheck(!*ScenSelectPath);
	menu->Items[6].flags =
		*ScenSelectDisplayPath ? 0 : MenuButtonDisabled;
	menu->Items[6].d.button.text = ScenSelectDisplayPath;

	ProcessMenu("menu-editor-save", 1);

	if (!EditorCancelled) {
		sprintf(path, "%s/%s.gz", ScenSelectPath, ScenSelectFileName);
		if (EditorSavePud(path) == -1) {
			ret = -1;
		}
		s = ScenSelectPath + strlen(ScenSelectPath);
		*s = '/';
		strcpy(s + 1, ScenSelectFileName);		// Final map name with path
		p = ScenSelectPath + strlen(StratagusLibPath) + 1;
		strcpy(CurrentMapPath, p);
		*s = '\0';
	}
	return ret;
}

/**
**		Editor save listbox init callback
*/
local void EditorSaveLBInit(Menuitem *mi)
{
	int i;

	EditorSaveLBExit(mi);
	i = mi->d.listbox.noptions = ReadDataDirectory(ScenSelectPath, EditorSaveRDFilter,
		(FileList **) & (mi->d.listbox.options));

	if (i == 0) {
		free(mi->menu->Items[4].d.button.text);
		mi->menu->Items[4].d.button.text = strdup("Save");
		mi->menu->Items[4].flags |= MenuButtonDisabled;
	} else {
		EditorSaveLBAction(mi, 0);
		sprintf(mi->menu->Items[3].d.input.buffer, "%s~!_", ScenSelectFileName);
		mi->menu->Items[3].d.input.nch = strlen(mi->menu->Items[3].d.input.buffer) - 3;
		mi->menu->Items[4].flags &= ~MenuButtonDisabled;
		if (i > mi->d.listbox.nlines) {
			mi[1].flags &= ~MenuButtonDisabled;
		}
	}
}

/**
**		Editor save listbox exit callback
*/
local void EditorSaveLBExit(Menuitem *mi)
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

/**
**		Editor save read directory filter
*/
local int EditorSaveRDFilter(char *pathbuf, FileList *fl)
{
	char *suf;
	char *np;
	char *cp;
	char *lcp;

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
			if (strcasestr(pathbuf, ".pud")) {
				fl->type = 1;
				fl->name = strdup(np);
				return 1;
			}
		}
	}
	return 0;
}

/**
**		Editor save folder button
*/
local void EditorSaveFolder(void)
{
	Menu *menu;
	Menuitem *mi;
	char *cp;

	menu = CurrentMenu;
	mi = &menu->Items[1];

	if (ScenSelectDisplayPath[0]) {
		cp = strrchr(ScenSelectDisplayPath, '/');
		if (cp) {
			*cp = 0;
		} else {
			ScenSelectDisplayPath[0] = 0;
			menu->Items[6].flags |= MenuButtonDisabled;
			menu->Items[6].d.button.text = NULL;
		}
		cp = strrchr(ScenSelectPath, '/');
		if (cp) {
			*cp = 0;
			EditorSaveLBInit(mi);
			mi->d.listbox.cursel = -1;
			mi->d.listbox.startline = 0;
			mi->d.listbox.curopt = 0;
			mi[1].d.vslider.percent = 0;
		}
	}
}

/**
**		Editor save ok button
*/
local void EditorSaveOk(void)
{
	Menu *menu;
	Menuitem *mi;
	FileList *fl;
	int i;

	menu = CurrentMenu;
	mi = &menu->Items[1];
	i = mi->d.listbox.curopt + mi->d.listbox.startline;
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (mi->menu->Items[3].d.input.nch == 0 && fl[i].type == 0) {
			strcat(ScenSelectPath, "/");
			strcat(ScenSelectPath, fl[i].name);
			if (menu->Items[6].flags & MenuButtonDisabled) {
				menu->Items[6].flags &= ~MenuButtonDisabled;
				menu->Items[6].d.button.text = ScenSelectDisplayPath;
			} else {
				strcat(ScenSelectDisplayPath, "/");
			}
			strcat(ScenSelectDisplayPath, fl[i].name);
			EditorSaveLBInit(mi);
			mi->d.listbox.cursel = -1;
			mi->d.listbox.startline = 0;
			mi->d.listbox.curopt = 0;
			mi[1].d.vslider.percent = 0;
		} else {
			strcpy(ScenSelectFileName, menu->Items[3].d.input.buffer);		// Final map name
			ScenSelectFileName[strlen(ScenSelectFileName) - 3] = '\0';
			if (!strcasestr(ScenSelectFileName, ".pud\0")) {
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
			menu->Items[6].d.button.text = NULL;
		}
	}
}

/**
**		Editor save cancel button
*/
local void EditorSaveCancel(void)
{
	Menu *menu;

	EditorCancelled = 1;
	EditorEndMenu();

	menu = FindMenu("menu-editor-save");
	menu->Items[6].d.button.text = NULL;
}

/**
**		Editor save listbox retrieve callback
*/
local unsigned char *EditorSaveLBRetrieve(Menuitem *mi, int i)
{
	FileList *fl;
	static char buffer[1024];

	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (fl[i].type) {
			strcpy(buffer, "   ");
		} else {
			strcpy(buffer, "\260 ");
		}
		strcat(buffer, fl[i].name);
		return buffer;
	}
	return NULL;
}

/**
**		Editor save listbox action callback
*/
local void EditorSaveLBAction(Menuitem *mi, int i)
{
	FileList *fl;

	DebugCheck(i<0);
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		free(mi->menu->Items[4].d.button.text);
		if (fl[i].type) {
			sprintf(mi->menu->Items[3].d.input.buffer, "%s~!_", fl[i].name);
			mi->menu->Items[3].d.input.nch = strlen(mi->menu->Items[3].d.input.buffer) - 3;
			mi->menu->Items[4].d.button.text = strdup("Save");
		} else {
			strcpy(mi->menu->Items[3].d.input.buffer, "~!_");
			mi->menu->Items[3].d.input.nch = strlen(mi->menu->Items[3].d.input.buffer) - 3;
			mi->menu->Items[4].d.button.text = strdup("Open");
		}
		mi->menu->Items[4].flags &= ~MenuButtonDisabled;
		if (mi->d.listbox.noptions > mi->d.listbox.nlines) {
			mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
		}
	}
}

/**
**		Editor save vertical slider action callback
*/
local void EditorSaveVSAction(Menuitem *mi, int i)
{
	int op;
	int d1;
	int d2;

	mi--;
	switch (i) {
		case 0:				// click - down
		case 2:				// key - down
			if (mi[1].d.vslider.cflags & MI_CFLAGS_DOWN) {
				if (mi->d.listbox.curopt + mi->d.listbox.startline + 1 < mi->d.listbox.noptions) {
					mi->d.listbox.curopt++;
					if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
						mi->d.listbox.curopt--;
						mi->d.listbox.startline++;
					}
				}
			} else if (mi[1].d.vslider.cflags & MI_CFLAGS_UP) {
				if (mi->d.listbox.curopt + mi->d.listbox.startline > 0) {
					mi->d.listbox.curopt--;
					if (mi->d.listbox.curopt < 0) {
						mi->d.listbox.curopt++;
						mi->d.listbox.startline--;
					}
				}
			}
			EditorSaveLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			if (i == 2) {
				mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN | MI_CFLAGS_UP);
			}
			break;
		case 1:				// mouse - move
			if (mi[1].d.vslider.cflags & MI_CFLAGS_KNOB && (mi[1].flags & MenuButtonClicked)) {
				if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt + mi->d.listbox.startline + 1 < mi->d.listbox.noptions) {
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
							if (mi->d.listbox.curopt + mi->d.listbox.startline + 1 == mi->d.listbox.noptions)
								break;
						}
					}
				} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt + mi->d.listbox.startline > 0) {
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
							if (mi->d.listbox.curopt + mi->d.listbox.startline == 0)
								break;
						}
					}
				}

				DebugCheck(mi->d.listbox.startline < 0);
				DebugCheck(mi->d.listbox.noptions > 0 &&
					mi->d.listbox.startline + mi->d.listbox.curopt >= mi->d.listbox.noptions);

				EditorSaveLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			}
			break;
		default:
			break;
	}
}

/**
**		Editor save input callback
*/
local void EditorSaveEnterAction(Menuitem *mi, int key)
{
	if (mi->d.input.nch == 0) {
		mi->menu->Items[4].flags = MenuButtonDisabled;
	} else {
		if (mi->d.input.nch == 1 && key != '\b' && key != '\177') {
			free(mi->menu->Items[4].d.button.text);
			mi->menu->Items[4].d.button.text = strdup("Save");
		}
		mi->menu->Items[4].flags &= ~MenuButtonDisabled;
		if (key == 10 || key == 13) {
			EditorSaveOk();
		}
	}
}

/**
**		Editor save confirm init callback
*/
local void EditorSaveConfirmInit(Menuitem *mi)
{
	mi->menu->Items[2].d.text.text = ScenSelectFileName;
}

/**
**		Editor save confirm ok button
*/
local void EditorSaveConfirmOk(void)
{
	CurrentMenu->Items[2].d.text.text = NULL;
	EditorEndMenu();
}

/**
**		Editor save confirm cancel button
*/
local void EditorSaveConfirmCancel(void)
{
	CurrentMenu->Items[2].d.text.text = NULL;
	EditorCancelled = 1;
	EditorEndMenu();
}

/**
**		Called from menu, to quit editor to menu.
**
**		@todo Should check if modified file should be saved.
*/
local void EditorQuitToMenu(void)
{
	QuitToMenu = 1;
	EditorRunning = 0;
	EndMenu();
}

/**
**		End menus state of the editor.
*/
local void EditorEndMenu(void)
{
	CursorOn = CursorOnUnknown;
	CurrentMenu = NULL;

	InterfaceState = IfaceStateNormal;
	EditorUpdateDisplay();
	InterfaceState = IfaceStateMenu;
}

/**
**		Replay game menu
*/
local void ReplayGameMenu(void)
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

	VideoClearScreen();
	MenusSetBackground();
	Invalidate();

	GuiGameStarted = 0;
	ProcessMenu("menu-replay-game", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}
}

/**
**		Replay game menu init callback
*/
local void ReplayGameInit(Menuitem *mi)
{
	DebugCheck(!*ScenSelectPath);
	mi->menu->Items[5].flags =
		*ScenSelectDisplayPath ? 0 : MenuButtonDisabled;
	mi->menu->Items[5].d.button.text = ScenSelectDisplayPath;
	mi->menu->Items[6].d.gem.state = MI_GSTATE_UNCHECKED;
	DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);
}

/**
**		Replay game listbox init callback
*/
local void ReplayGameLBInit(Menuitem *mi)
{
	int i;

	ReplayGameLBExit(mi);
	i = mi->d.listbox.noptions = ReadDataDirectory(ScenSelectPath, ReplayGameRDFilter,
		(FileList **) & (mi->d.listbox.options));

	if (i == 0) {
		free(mi->menu->Items[3].d.button.text);
		mi->menu->Items[3].d.button.text = strdup("OK");
		mi->menu->Items[3].flags |= MenuButtonDisabled;
	} else {
		ReplayGameLBAction(mi, 0);
		mi->menu->Items[3].flags &= ~MenuButtonDisabled;
		if (i > mi->d.listbox.nlines) {
			mi[1].flags &= ~MenuButtonDisabled;
		} else {
			mi[1].flags |= MenuButtonDisabled;
		}
	}
}

/**
**		Replay game listbox exit callback
*/
local void ReplayGameLBExit(Menuitem *mi)
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

/**
**		Replay game read directory filter
*/
local int ReplayGameRDFilter(char *pathbuf, FileList *fl)
{
	char *suf;
	char *np;
	char *cp;
	char *lcp;

	suf = ".log";
	np = strrchr(pathbuf, '/');
	if (np) {
		np++;
	} else {
		np = pathbuf;
	}
	cp = np;
	cp--;
	fl->type = -1;
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
			if (strcasestr(pathbuf, ".log")) {
				fl->type = 1;
				fl->name = strdup(np);
				return 1;
			}
		}
	}
	return 0;
}

/**
**		Replay game listbox action
*/
local void ReplayGameLBAction(Menuitem *mi, int i)
{
	FileList *fl;

	DebugCheck(i<0);
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		free(mi->menu->Items[3].d.button.text);
		if (fl[i].type) {
			mi->menu->Items[3].d.button.text = strdup("OK");
		} else {
			mi->menu->Items[3].d.button.text = strdup("Open");
		}
		if (mi->d.listbox.noptions > mi->d.listbox.nlines) {
			mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
			mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
		}
	}
}

/**
**		Replay game listbox retrieve
*/
local unsigned char *ReplayGameLBRetrieve(Menuitem *mi, int i)
{
	FileList *fl;
	static char buffer[1024];

	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (fl[i].type) {
			strcpy(buffer, "   ");
		} else {
			strcpy(buffer, "\260 ");
		}
		strcat(buffer, fl[i].name);
		return buffer;
	}
	return NULL;
}

/**
**		Replay game vertical slider action
*/
local void ReplayGameVSAction(Menuitem *mi, int i)
{
	int op;
	int d1;
	int d2;

	mi--;
	switch (i) {
		case 0:				// click - down
		case 2:				// key - down
			if (mi[1].d.vslider.cflags & MI_CFLAGS_DOWN) {
				if (mi->d.listbox.curopt + mi->d.listbox.startline + 1 < mi->d.listbox.noptions) {
					mi->d.listbox.curopt++;
					if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
						mi->d.listbox.curopt--;
						mi->d.listbox.startline++;
					}
				}
			} else if (mi[1].d.vslider.cflags & MI_CFLAGS_UP) {
				if (mi->d.listbox.curopt + mi->d.listbox.startline > 0) {
					mi->d.listbox.curopt--;
					if (mi->d.listbox.curopt < 0) {
						mi->d.listbox.curopt++;
						mi->d.listbox.startline--;
					}
				}
			}
			ReplayGameLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			if (i == 2) {
				mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN | MI_CFLAGS_UP);
			}
			break;
		case 1:				// mouse - move
			if (mi[1].d.vslider.cflags & MI_CFLAGS_KNOB && (mi[1].flags & MenuButtonClicked)) {
				if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt + mi->d.listbox.startline + 1 < mi->d.listbox.noptions) {
						for (;;) {
							op = ((mi->d.listbox.curopt + mi->d.listbox.startline + 1) * 100) /
								 (mi->d.listbox.noptions - 1);
							d1 = mi[1].d.vslider.curper - mi[1].d.vslider.percent;
							d2 = op - mi[1].d.vslider.curper;
							if (d2 >= d1) {
								break;
							}
							mi->d.listbox.curopt++;
							if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
								mi->d.listbox.curopt--;
								mi->d.listbox.startline++;
							}
							if (mi->d.listbox.curopt + mi->d.listbox.startline+1 == mi->d.listbox.noptions) {
								break;
							}
						}
					}
				} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
					if (mi->d.listbox.curopt + mi->d.listbox.startline > 0) {
						for (;;) {
							op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
									 (mi->d.listbox.noptions - 1);
							d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
							d2 = mi[1].d.vslider.curper - op;
							if (d2 >= d1) {
								break;
							}
							mi->d.listbox.curopt--;
							if (mi->d.listbox.curopt < 0) {
								mi->d.listbox.curopt++;
								mi->d.listbox.startline--;
							}
							if (mi->d.listbox.curopt + mi->d.listbox.startline == 0) {
								break;
							}
						}
					}
				}

				DebugCheck(mi->d.listbox.startline < 0);
				DebugCheck(mi->d.listbox.noptions > 0 &&
					mi->d.listbox.startline + mi->d.listbox.curopt >= mi->d.listbox.noptions);

				ReplayGameLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			}
			break;
		default:
			break;
	}
}

/**
**		Replay game folder button callback
*/
local void ReplayGameFolder(void)
{
	Menu *menu;
	Menuitem *mi;
	char *cp;

	menu = CurrentMenu;
	mi = &menu->Items[1];

	if (ScenSelectDisplayPath[0]) {
		cp = strrchr(ScenSelectDisplayPath, '/');
		if (cp) {
			*cp = 0;
		} else {
			ScenSelectDisplayPath[0] = 0;
			menu->Items[5].flags |= MenuButtonDisabled;
			menu->Items[5].d.button.text = NULL;
		}
		cp = strrchr(ScenSelectPath, '/');
		if (cp) {
			*cp = 0;
			ReplayGameLBInit(mi);
			mi->d.listbox.cursel = -1;
			mi->d.listbox.startline = 0;
			mi->d.listbox.curopt = 0;
			mi[1].d.vslider.percent = 0;
			mi[1].d.hslider.percent = 0;
		}
	}
}

/**
**		Replay game disable fog gem callback
*/
local void ReplayGameDisableFog(Menuitem *mi __attribute__((unused)))
{
}

/**
**		Replay game ok button callback
*/
local void ReplayGameOk(void)
{
	Menu *menu;
	Menuitem *mi;
	FileList *fl;
	int i;

	menu = CurrentMenu;
	mi = &menu->Items[1];
	i = mi->d.listbox.curopt + mi->d.listbox.startline;
	if (i < mi->d.listbox.noptions) {
		fl = mi->d.listbox.options;
		if (fl[i].type == 0) {
			strcat(ScenSelectPath, "/");
			strcat(ScenSelectPath, fl[i].name);
			if (menu->Items[5].flags & MenuButtonDisabled) {
				menu->Items[5].flags &= ~MenuButtonDisabled;
				menu->Items[5].d.button.text = ScenSelectDisplayPath;
			} else {
				strcat(ScenSelectDisplayPath, "/");
			}
			strcat(ScenSelectDisplayPath, fl[i].name);
			ReplayGameLBInit(mi);
			mi->d.listbox.cursel = -1;
			mi->d.listbox.startline = 0;
			mi->d.listbox.curopt = 0;
			mi[1].d.vslider.percent = 0;
			mi[1].d.hslider.percent = 0;
		} else {
			strcpy(ScenSelectFileName, fl[i].name);		// Final map name

			if (ScenSelectPath[0]) {
				strcat(ScenSelectPath, "/");
				strcat(ScenSelectPath, ScenSelectFileName);
			} else {
				strcpy(ScenSelectPath, ScenSelectFileName);
			}

			LoadReplay(ScenSelectPath);

			for (i = 0; i < MAX_OBJECTIVES; i++) {
				if (GameIntro.Objectives[i]) {
					free(GameIntro.Objectives[i]);
					GameIntro.Objectives[i] = NULL;
				}
			}
			GameIntro.Objectives[0] = strdup(DefaultObjective);

			GuiGameStarted = 1;
			EndMenu();
			menu->Items[5].d.button.text = NULL;

			if (menu->Items[6].d.gem.state == MI_GSTATE_CHECKED) {
				ReplayRevealMap = 1;
			} else {
				ReplayRevealMap = 0;
			}
		}
	}
}

/**
**		Replay game cancel button callback
*/
local void ReplayGameCancel(void)
{
	char *s;

	//
	//  Use last selected map.
	//
	DebugLevel0Fn("Map   path: %s\n" _C_ CurrentMapPath);
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

	DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);

	CurrentMenu->Items[5].d.button.text = NULL;

	EndMenu();
}

/**
**		Net error menu
**
**		@param error			Error message
*/
local void NetErrorMenu(char *error)
{
	Menu *menu;

	menu = FindMenu("menu-net-error");
	menu->Items[1].d.text.text = error;
	ProcessMenu("menu-net-error", 1);
	menu->Items[1].d.text.text = NULL;
}

/**
**		Error menu
**
**		@param error			Error message
*/
global void ErrorMenu(char *error)
{
	Menu *menu;
	int oldx;
	int oldy;

	menu = FindMenu("menu-net-error");
	oldx = menu->X;
	oldy = menu->Y;
	menu->X = (VideoWidth - menu->Width) / 2;
	menu->Y = (VideoHeight - menu->Height) / 2;
	menu->Items[1].d.text.text = error;
	ProcessMenu("menu-net-error", 1);
	menu->Items[1].d.text.text = NULL;
	menu->X = oldx;
	menu->Y = oldy;
}

/*----------------------------------------------------------------------------
--  Init functions
----------------------------------------------------------------------------*/

/**
**  Initialize player races for a menu item
*/
local void InitPlayerRaces(Menuitem* mi)
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
	if (mi->d.pulldown.options) {
		free(mi->d.pulldown.options);
	}
	mi->d.pulldown.options = (unsigned char**)malloc(n * sizeof(unsigned char*));
	for (i = 0, n = 0; i < PlayerRaces.Count; ++i) {
		if (PlayerRaces.Visible[i]) {
			mi->d.pulldown.options[n++] = strdup(PlayerRaces.Display[i]);
		}
	}
	mi->d.pulldown.options[n++] = strdup("Map Default");
	mi->d.pulldown.noptions = n;
	mi->d.pulldown.defopt = n - 1;
}

/**
**  Initialize tilesets for a menu item
*/
local void InitTilesets(Menuitem* mi, int mapdefault)
{
	int i;
	int n;

	// Reallocate pulldown options.
	if (mi->d.pulldown.options) {
		free(mi->d.pulldown.options);
	}
	n = NumTilesets + (mapdefault ? 1 : 0);
	mi->d.pulldown.options = (unsigned char**)malloc(n * sizeof(unsigned char*));
	n = 0;
	if (mapdefault) {
		mi->d.pulldown.options[n++] = strdup("Map Default");
	}
	for (i = 0; i < NumTilesets; ++i) {
		mi->d.pulldown.options[n++] = strdup(Tilesets[i]->Name);
	}
	mi->d.pulldown.noptions = n;
	mi->d.pulldown.defopt = 0;
}

/**
**  Initialize the loaded menu data
*/
global void InitMenuData(void)
{
	Menu *menu;

	InitNetMultiButtonStorage();

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

/**
**  Post-Initialize the loaded menu functions
*/
global void InitMenuFunctions(void)
{
#ifdef SAVE_MENU_CCL
	{
		FILE* fd;
		fd = fopen("menus.ccl", "wb");
		SaveMenus(fd);
		fclose(fd);
	}
#endif
}


/**
**  FIXME: docu
*/
local void MultiGameMasterReport(void)
{
//	EndMenu();
	MenusSetBackground();
	Invalidate();

	ProcessMenu("metaserver-list", 1);
	if (GuiGameStarted) {
		GameMenuReturn();
	}

}

/**
**  Menu for Mater Server Game list.
*/
local void ShowMetaServerList(void)
{
	EndMenu();
	Invalidate();
	MenusSetBackground();

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
**  TODO: well, redo this :)
*/
local void MultiMetaServerGameSetupInit(Menuitem* mi)
{
	int i;
	int j;
	int k;
	int numparams;
	int nummenus;
	char* parameter;
	char* reply;
	Menu* menu;
	char* port;

	SendMetaCommand("NumberOfGames", "");
	menu = FindMenu("metaserver-list");

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
			menu->Items[j].d.text.text = NULL;
			menu->Items[j + 1].d.text.text = NULL;
			menu->Items[j + 2].d.text.text = NULL;
			menu->Items[j + 3].d.text.text = NULL;
			menu->Items[j + 4].d.text.text = NULL;
			menu->Items[j + 5].d.gem.state = MI_GSTATE_INVISIBLE;
		} else {
			GetMetaParameter(reply, 0, &parameter);       // Player Name
			menu->Items[j].d.text.text = parameter;
			GetMetaParameter(reply, 3, &parameter);       // IP
			GetMetaParameter(reply, 4, &port);            // port
			sprintf(parameter, "%s:%s", parameter, port); // IP:Port
			menu->Items[j + 1].d.text.text = parameter;
			GetMetaParameter(reply, 6, &parameter);
			menu->Items[j + 2].d.text.text = parameter;
			GetMetaParameter(reply, 7, &parameter);
			menu->Items[j + 3].d.text.text = parameter;
			GetMetaParameter(reply, 8, &parameter);
			menu->Items[j + 4].d.text.text = parameter;
			menu->Items[j + 5].d.gem.state = MI_GSTATE_UNCHECKED;
		}
		++k;
	}

	// Don't display slots not in use
	// FIXME: HardCoded Number of Items in list
	// 5 is the hardcoded value
	for (; j <= numparams * 5; j += numparams + 1) {
		// fill the menus with the right info.
		menu->Items[j].d.text.text = NULL;
		menu->Items[j + 1].d.text.text = NULL;
		menu->Items[j + 2].d.text.text = NULL;
		menu->Items[j + 3].d.text.text = NULL;
		menu->Items[j + 4].d.text.text = NULL;
		menu->Items[j + 5].d.gem.state = MI_GSTATE_INVISIBLE;
	}
}

/**
**  Multiplayer server menu exit callback
*/
local void MultiMetaServerGameSetupExit(Menuitem *mi)
{
	// TODO: how to free stuff?
//	EndMenu();
	MenusSetBackground();
	Invalidate();
//	EndMenu();
}

/**
**  Action taken when a player select an online game
*/
local void SelectGameServer(Menuitem *mi)
{
	char server_host_buffer[64];
	char *port;
	int j;

	j = mi - mi->menu->Items;
	mi->menu->Items[j].d.gem.state = MI_GSTATE_UNCHECKED;
	MenusSetBackground();
	Invalidate();
	EndMenu();

	strcpy(server_host_buffer, mi->menu->Items[j - 4].d.text.text);

	// Launch join directly
	if ((port = strchr(server_host_buffer, ':')) != NULL) {
		NetworkPort = atoi(port + 1);
		port[0] = 0;
	}

	// Now finally here is the address
//	server_host_buffer[menu->Items[1].d.input.nch] = 0;
	if (NetworkSetupServerAddress(server_host_buffer)) {
		NetErrorMenu("Unable to lookup host.");
		MenusSetBackground();
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
		MenusSetBackground();
		Invalidate();
		EndMenu();
	}
}

/**
**  Action to add a game server on the meta-server.
*/
local void AddGameServer(void)
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
local void ChangeGameServer(void)
{
	int i;
	int freespots;
	int	players;

	// send message to meta server. meta server will detect IP address.
	// Meta-server will return "ERR" if the list of online games is busy.

	freespots = 0;
	players = 0;
	for (i = 0; i < PlayerMax - 1; ++i) {
		if (MenuMapInfo->PlayerType[i] == PlayerPerson) {
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
local int MetaServerConnectError(void)
{
	Invalidate();
	NetErrorMenu("Cannot Connect to Meta-Server");
	MenusSetBackground();
	return 0;
}

/**
**  Close MetaServer connection
*/
local void MultiMetaServerClose(void)
{
	MetaClose();
	MetaServerInUse = 0;
	EndMenu();
}
//@}
