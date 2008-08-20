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
//ServerSetup ServerSetupState;
//ServerSetup LocalSetupState;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/
#if 0
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
