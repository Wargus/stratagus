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
/**@name interface.c - The interface. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer and Jimmy Salmon
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "sound_id.h"
#include "sound.h"
#include "sound_server.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "interface.h"
#include "cursor.h"
#include "ui.h"
#include "map.h"
#include "menus.h"
#include "script.h"
#include "tileset.h"
#include "minimap.h"
#include "network.h"
#include "font.h"
#include "campaign.h"
#include "video.h"
#include "iolib.h"
#include "pud.h"
#include "commands.h"
#include "ai.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static int SavedMapPositionX[4];     /// Saved map position X
static int SavedMapPositionY[4];     /// Saved map position Y
static char Input[80];               /// line input for messages/long commands
static int InputIndex;               /// current index into input
static char InputStatusLine[99];     /// Last input status line
char* UiGroupKeys = "0123456789`"; /// Up to 11 keys, last unselect. Default for qwerty
char GameRunning;            /// Current running state
char GamePaused;             /// Current pause state
char GameObserve;            /// Observe mode
char SkipGameCycle;          /// Skip the next game cycle
char BigMapMode;             /// Show only the map
enum _iface_state_ InterfaceState; /// Current interface state
int GodMode;                 /// Invincibility cheat
enum _key_state_ KeyState;   /// current key state

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Show input.
*/
static void ShowInput(void)
{
	char* input;

	sprintf(InputStatusLine, "MESSAGE:%s~!_", Input);
	input = InputStatusLine;
	// FIXME: This is slow!
	while (VideoTextLength(TheUI.StatusLineFont, input) >
			TheUI.StatusLine.Graphic->Width) {
		++input;
	}
	KeyState = KeyStateCommand;
	ClearStatusLine();
	SetStatusLine(input);
	KeyState = KeyStateInput;
}

/**
**  Begin input.
*/
static void UiBeginInput(void)
{
	KeyState = KeyStateInput;
	Input[0] = '\0';
	InputIndex = 0;
	ClearCosts();
	ShowInput();
}

//-----------------------------------------------------------------------------
//  User interface group commands
//-----------------------------------------------------------------------------

/**
**  Unselect all currently selected units.
*/
static void UiUnselectAll(void)
{
	UnSelectAll();
	NetworkSendSelection(NULL, 0);
	SelectionChanged();
}

/**
**  Center on group.
**
**  @param group  Group number to center on.
**
**  @todo Improve this function, try to show all selected units
**        or the most possible units.
*/
static void UiCenterOnGroup(unsigned group)
{
	Unit** units;
	int n;
	int x;
	int y;

	n = GetNumberUnitsOfGroup(group);
	if (n--) {
		units = GetUnitsOfGroup(group);
		// FIXME: what should we do with the removed units? ignore?

		x = units[n]->X;
		y = units[n]->Y;
		while (n--) {
			x += (units[n]->X - x) / 2;
			y += (units[n]->Y - y) / 2;
		}
		ViewportCenterViewpoint(TheUI.SelectedViewport, x, y, TileSizeX / 2, TileSizeY / 2);
	}
}

/**
**  Select group. If already selected center on the group.
**
**  @param group  Group number to select.
*/
static void UiSelectGroup(unsigned group)
{
	Unit** units;
	int n;

	//
	//  Check if group is already selected.
	//
	n = GetNumberUnitsOfGroup(group);
	if (n && NumSelected == n) {
		units = GetUnitsOfGroup(group);

		// FIXME: what should we do with the removed units? ignore?
		while (n-- && units[n] == Selected[n]) {
		}
		if (n == -1) {
			UiCenterOnGroup(group);
			return;
		}
	}

	SelectGroup(group);
	SelectionChanged();
}

/**
**  Add group to current selection.
**
**  @param group  Group number to add.
*/
static void UiAddGroupToSelection(unsigned group)
{
	Unit** units;
	int n;

	if (!(n = GetNumberUnitsOfGroup(group))) {
		return;
	}

	//
	//  Don't allow to mix units and buildings
	//
	if (NumUnits && Selected[0]->Type->Building) {
		return;
	}

	units = GetUnitsOfGroup(group);
	if (units[0]->Type->Building) {
		return;
	}

	while (n--) {
		if (!units[n]->Removed) {
			SelectUnit(units[n]);
		}
	}

	SelectionChanged();
}

/**
**  Define a group. The current selected units become a new group.
**
**  @param group  Group number to create.
*/
static void UiDefineGroup(unsigned group)
{
	SetGroup(Selected, NumSelected, group);
}

/**
**  Add to group. The current selected units are added to the group.
**
**  @param group  Group number to be expanded.
*/
static void UiAddToGroup(unsigned group)
{
	AddToGroup(Selected, NumSelected, group);
}

/**
**  Toggle sound on / off.
*/
static void UiToggleSound(void)
{
	if (SoundFildes != -1) {
		SoundOff ^= 1;
	}
	if (SoundOff) {
		SetStatusLine("Sound is off.");
	} else {
		SetStatusLine("Sound is on.");
	}
}

/**
**  Toggle music on / off.
*/
static void UiToggleMusic(void)
{
	static int vol;
	if (MusicVolume) {
		vol = MusicVolume;
		MusicVolume = 0;
		SetStatusLine("Music is off.");
	} else {
		MusicVolume = vol;
		SetStatusLine("Music is on.");
	}
}

/**
**  Toggle pause on / off.
*/
void UiTogglePause(void)
{
	if (!IsNetworkGame()) {
		GamePaused ^= 1;
		if (GamePaused) {
			SetStatusLine("Game Paused");
		} else {
			SetStatusLine("Game Resumed");
		}
	}
}

/**
**  Enter menu mode.
*/
static void UiEnterMenu(void)
{
	if (!IsNetworkGame()) {
		GamePaused = 1;
		SetStatusLine("Game Paused");
	}
	ProcessMenu("menu-game", 0);
}

/**
**  Enter help menu
*/
static void UiEnterHelpMenu(void)
{
	if (!IsNetworkGame()) {
		GamePaused = 1;
		SetStatusLine("Game Paused");
	}
	ProcessMenu("menu-help", 0);
}

/**
**  Enter options menu
*/
static void UiEnterOptionsMenu(void)
{
	if (!IsNetworkGame()) {
		GamePaused = 1;
		SetStatusLine("Game Paused");
	}
	ProcessMenu("menu-game-options", 0);
}

/**
**  Enter Sound Options menu
*/
static void UiEnterSoundOptionsMenu(void)
{
	if (!IsNetworkGame()) {
		GamePaused = 1;
		SetStatusLine("Game Paused");
	}
	SoundOptionsMenu();
}

/**
**  Enter Speed Options menu
*/
static void UiEnterSpeedOptionsMenu(void)
{
	if (!IsNetworkGame()) {
		GamePaused = 1;
		SetStatusLine("Game Paused");
	}
	SpeedOptionsMenu();
}

/**
**  Enter Preferences Options menu
*/
static void UiEnterPreferencesOptionsMenu(void)
{
	if (!IsNetworkGame()) {
		GamePaused = 1;
		SetStatusLine("Game Paused");
	}
	PreferencesMenu();
}

/**
**  Enter Save Game Menu
*/
static void UiEnterSaveGameMenu(void)
{
	// Disable save menu in multiplayer and replays
	if (IsNetworkGame() || ReplayGameType != ReplayNone) {
		return;
	}

	if (!IsNetworkGame()) {
		GamePaused = 1;
		SetStatusLine("Game Paused");
	}
	SaveGameMenu();
}

/**
**  Enter Load Game Menu
*/
static void UiEnterLoadGameMenu(void)
{
	// Disable load menu in multiplayer
	if (IsNetworkGame()) {
		return;
	}

	if (!IsNetworkGame()) {
		GamePaused = 1;
		SetStatusLine("Game Paused");
	}
	LoadGameMenu();
}

/**
**  Enter Exit Confirm menu
*/
static void UiExitConfirmMenu(void)
{
	if (!IsNetworkGame()) {
		GamePaused = 1;
		SetStatusLine("Game Paused");
	}
	ExitConfirmMenu();
}

/**
**  Enter Quit To Menu Confirm menu
*/
static void UiQuitToMenuConfirmMenu(void)
{
	if (!IsNetworkGame()) {
		GamePaused = 1;
		SetStatusLine("Game Paused");
	}
	QuitToMenuConfirmMenu();
}

/**
**  Enter Restart Confirm menu
*/
static void UiRestartConfirmMenu(void)
{
	if (!IsNetworkGame()) {
		GamePaused = 1;
		SetStatusLine("Game Paused");
	}
	RestartConfirmMenu();
}

/**
**  Toggle big map mode.
**
**  @todo FIXME: We should try to keep the same view, if possible
*/
static void UiToggleBigMap(void)
{
	static int mapx;
	static int mapy;
	static int mapex;
	static int mapey;

	BigMapMode ^= 1;
	if (BigMapMode) {
		mapx = TheUI.MapArea.X;
		mapy = TheUI.MapArea.Y;
		mapex = TheUI.MapArea.EndX;
		mapey = TheUI.MapArea.EndY;

		TheUI.MapArea.X = 0;
		TheUI.MapArea.Y = 0;
		TheUI.MapArea.EndX = VideoWidth - 1;
		TheUI.MapArea.EndY = VideoHeight - 1;

		SetViewportMode(TheUI.ViewportMode);

		EnableRedraw = RedrawMap | RedrawCursor | RedrawMessage | RedrawMenu |
			RedrawTimer | RedrawAll;
		SetStatusLine("Big map enabled");
	} else {
		TheUI.MapArea.X = mapx;
		TheUI.MapArea.Y = mapy;
		TheUI.MapArea.EndX = mapex;
		TheUI.MapArea.EndY = mapey;

		SetViewportMode(TheUI.ViewportMode);

		EnableRedraw = RedrawEverything;
		SetStatusLine("Returning to old map");
	}
}

/**
**  Increase game speed.
*/
static void UiIncreaseGameSpeed(void)
{
	VideoSyncSpeed += 10;
	SetVideoSync();
	SetStatusLine("Faster");
}

/**
**  Decrease game speed.
*/
static void UiDecreaseGameSpeed(void)
{
	if (VideoSyncSpeed <= 10) {
		if (VideoSyncSpeed > 1) {
			--VideoSyncSpeed;
		}
	} else {
		VideoSyncSpeed -= 10;
	}
	SetVideoSync();
	SetStatusLine("Slower");
}

/**
**  Center on the selected units.
**
**  @todo Improve this function, try to show all selected units
**        or the most possible units.
*/
static void UiCenterOnSelected(void)
{
	int x;
	int y;
	int n;

	if ((n = NumSelected)) {
		x = Selected[--n]->X;
		y = Selected[n]->Y;
		while (n--) {
			x += (Selected[n]->X - x) / 2;
			y += (Selected[n]->Y - y) / 2;
		}
		ViewportCenterViewpoint(TheUI.SelectedViewport, x, y, TileSizeX / 2, TileSizeY / 2);
	}
}

/**
**  Save current map position.
**
**  @param position  Map position slot.
*/
static void UiSaveMapPosition(unsigned position)
{
	SavedMapPositionX[position] = TheUI.SelectedViewport->MapX;
	SavedMapPositionY[position] = TheUI.SelectedViewport->MapY;
}

/**
**  Recall map position.
**
**  @param position  Map position slot.
*/
static void UiRecallMapPosition(unsigned position)
{
	ViewportSetViewpoint(TheUI.SelectedViewport,
		SavedMapPositionX[position], SavedMapPositionY[position], TileSizeX / 2, TileSizeY / 2);
}

/**
**  Toggle terrain display on/off.
*/
static void UiToggleTerrain(void)
{
	MinimapWithTerrain ^= 1;
	if (MinimapWithTerrain) {
		SetStatusLine("Terrain displayed.");
	} else {
		SetStatusLine("Terrain hidden.");
	}
	MustRedraw |= RedrawMinimap;
}

/**
**  Find the next idle worker, select it, and center on it
*/
static void UiFindIdleWorker(void)
{
	Unit* unit;
	// FIXME: static variable, is not needed.
	static Unit* LastIdleWorker = NoUnitP;

	unit = FindIdleWorker(ThisPlayer, LastIdleWorker);
	if (unit != NoUnitP) {
		LastIdleWorker = unit;
		SelectSingleUnit(unit);
		ClearStatusLine();
		ClearCosts();
		CurrentButtonLevel = 0;
		PlayUnitSound(Selected[0], VoiceSelected);
		SelectionChanged();
		ViewportCenterViewpoint(TheUI.SelectedViewport, unit->X, unit->Y, TileSizeX / 2, TileSizeY / 2);
	}
}

/**
**  Toggle grab mouse on/off.
*/
static void UiToggleGrabMouse(void)
{
	DebugPrint("%x\n" _C_ KeyModifiers);
	ToggleGrabMouse(0);
	SetStatusLine("Grab mouse toggled.");
}

/**
**  Track unit, the viewport follows the unit.
*/
static void UiTrackUnit(void)
{
	if (TheUI.SelectedViewport->Unit == Selected[0]) {
		TheUI.SelectedViewport->Unit = NULL;
	} else {
		TheUI.SelectedViewport->Unit = Selected[0];
	}
}

/**
**  Handle keys in command mode.
**
**  @param key  Key scancode.
**
**  @return     True, if key is handled; otherwise false.
*/
static int CommandKey(int key)
{
	char* ptr;

	// FIXME: don't handle unicode well. Should work on all latin keyboard.
	if ((ptr = strchr(UiGroupKeys, key))) {
		key = '0' + ptr - UiGroupKeys;
		if (key > '9') {
			key = '`';
		}
	}

	switch (key) {
		// Return enters chat/input mode.
		case '\r':
			UiBeginInput();
			return 1;

		// Unselect everything
		case '^':
		case '`':
			UiUnselectAll();
			break;

		// Group selection
		case '0': case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
		case '9':
			if (KeyModifiers & ModifierShift) {
				if (KeyModifiers & ModifierAlt) {
					UiCenterOnGroup(key - '0');
				} else if (KeyModifiers & ModifierControl) {
					UiAddToGroup(key - '0');
				} else {
					UiAddGroupToSelection(key - '0');
				}
			} else {
				if (KeyModifiers & ModifierAlt) {
					UiCenterOnGroup(key - '0');
				} else if (KeyModifiers & ModifierControl) {
					UiDefineGroup(key - '0');
				} else {
					UiSelectGroup(key - '0');
				}
			}
			break;

		case 'p' & 0x1F:
		case 'p':						// If pause-key didn't work
		case 'P':						// CTRL-P, ALT-P Toggle pause
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
		case KeyCodePause:
			UiTogglePause();
			break;

		case KeyCodeF1:
			UiEnterHelpMenu();
			break;

		case KeyCodeF2:
		case KeyCodeF3:
		case KeyCodeF4:						// Set/Goto place
			if (KeyModifiers & ModifierShift) {
				UiSaveMapPosition(key - KeyCodeF1);
			} else {
				UiRecallMapPosition(key - KeyCodeF1);
			}
			break;

		case 'm':
		case 'M':						// ALT+M, F10 Game menu
			if (KeyModifiers & ModifierControl) {
				UiToggleMusic();
				SavePreferences();
				break;
			}
			if (!(KeyModifiers & ModifierAlt)) {
				break;
			}

		case KeyCodeF5:						// Options menu
			if (KeyState != KeyStateInput) {
				UiEnterOptionsMenu();
			}
			break;

		case KeyCodeF7:						// Sound Options menu
			if (KeyState != KeyStateInput) {
				UiEnterSoundOptionsMenu();
			}
			break;

		case KeyCodeF8:						// Speed Options menu
			if (KeyState != KeyStateInput) {
				UiEnterSpeedOptionsMenu();
			}
			break;

		case KeyCodeF9:						// Preferences menu
			if (KeyState != KeyStateInput) {
				UiEnterPreferencesOptionsMenu();
			}
			break;

		case KeyCodeF10:				// Game Options menu
			if (KeyState != KeyStateInput) {
				UiEnterMenu();
			}
			break;

		case '+':						// + Faster
		case '=': // plus is shift-equals.
		case KeyCodeKPPlus:
			UiIncreaseGameSpeed();
			break;

		case '-':						// - Slower
		case KeyCodeKPMinus:
			UiDecreaseGameSpeed();
			break;

		case 'l' & 0x1F:
		case 'l':						// ALT l F12 load game menu
		case 'L':
#ifdef DEBUG
			if (KeyModifiers & ModifierControl) {// Ctrl + L - load - all debug
				LoadAll();
				SetMessage("All loaded");
				break;
			}
#endif
			if (!(KeyModifiers & ModifierAlt)) {
				break;
			}
		case KeyCodeF12:
			UiEnterLoadGameMenu();
			break;

		case 's' & 0x1F:						// Ctrl + S - Turn sound on / off
			UiToggleSound();
			SavePreferences();
			break;

		case 's':						// ALT s F11 save game menu
		case 'S':
			if (KeyModifiers & ModifierControl) {
				UiToggleSound();
				break;
			}
			if (!(KeyModifiers & ModifierAlt)) {
				break;
			}
			// FALL THROUGH (ALT+S)
		case KeyCodeF11:
			UiEnterSaveGameMenu();
			break;

		case 't' & 0x1F:
		case 't':
		case 'T':						// ALT-T, CTRL-T Track unit
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiTrackUnit();
			break;

		case 'b' & 0x1F:
		case 'b':
		case 'B':						// ALT+B, CTRL+B Toggle big map
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiToggleBigMap();
			break;

		case 'c':						// CTRL+C,ALT+C, C center on units
		case 'C':
			UiCenterOnSelected();
			break;

		case 'f' & 0x1F:
		case 'f':
		case 'F':						// ALT+F, CTRL+F toggle fullscreen
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			ToggleFullScreen();
			SavePreferences();
			break;

		case 'g' & 0x1F:
		case 'g':
		case 'G':						// ALT+G, CTRL+G grab mouse pointer
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiToggleGrabMouse();
			break;

		case 'h' & 0x1F:
		case 'h':
		case 'H':						// ALT+H, CTRL+H Help menu
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiEnterHelpMenu();
			break;

		case ' ':						// center on last action
			CenterOnMessage();
			break;

		case '\t':						// TAB toggles minimap.
										// FIXME: more...
										// FIXME: shift+TAB
			if (KeyModifiers & ModifierAlt) {
				break;
			}
			UiToggleTerrain();
			break;

		case 'x' & 0x1F:
		case 'x':
		case 'X':						// ALT+X, CTRL+X: Exit game
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiExitConfirmMenu();
			break;

		case 'q' & 0x1F:
		case 'q':
		case 'Q':						// ALT+Q, CTRL+Q: Quit level
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiQuitToMenuConfirmMenu();
			break;

		case 'r' & 0x1F:
		case 'r':
		case 'R':						// ALT+R, CTRL+R: Restart scenario
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiRestartConfirmMenu();
			break;

		case 'i':
		case 'I':
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			// FALL THROUGH
		case '.':						// ., ALT+I, CTRL+I: Find idle worker
			UiFindIdleWorker();
			break;

		case 'v':						// ALT+v CTRL+V: Viewport
			if (KeyModifiers & ModifierControl) {
				CycleViewportMode(-1);
			} else {
				CycleViewportMode(1);
			}
			break;

		case KeyCodeUp:
		case KeyCodeKP8:
			KeyScrollState |= ScrollUp;
			break;
		case KeyCodeDown:
		case KeyCodeKP2:
			KeyScrollState |= ScrollDown;
			break;
		case KeyCodeLeft:
		case KeyCodeKP4:
			KeyScrollState |= ScrollLeft;
			break;
		case KeyCodeRight:
		case KeyCodeKP6:
			KeyScrollState |= ScrollRight;
			break;

		default:
			return 0;
	}
	return 1;
}

/**
**  Handle cheats
**
**  @return  1 if a cheat was handled, 0 otherwise
*/
int HandleCheats(const char* input)
{
	int ret;

#ifdef DEBUG
	if (!strcmp(input, "ai me")) {
		if (ThisPlayer->AiEnabled) {
			ThisPlayer->AiEnabled = 0;
			ThisPlayer->Type = PlayerPerson;
			SetMessage("AI is off, Normal Player");
		} else {
			ThisPlayer->AiEnabled = 1;
			ThisPlayer->Type = PlayerComputer;
			if (!ThisPlayer->Ai) {
				AiInit(ThisPlayer);
			}
			SetMessage("I'm the BORG, resistance is futile!");
		}
		return 1;
	}
#endif
	lua_pushstring(Lua, "HandleCheats");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	if (!lua_isfunction(Lua, -1)) {
		DebugPrint("No HandleCheats function in lua.\n");
		return 0;
	}
	lua_pushstring(Lua, input);
	LuaCall(1, 0);
	ret = lua_gettop(Lua);
	if (lua_gettop(Lua) == 1) {
		ret = LuaToBoolean(Lua, 1);
		lua_pop(Lua, 1);
	} else {
		LuaError(Lua, "HandleCheats must return a boolean");
		ret = 0;
	}
	return ret;
}

/**
**  Handle keys in input mode.
**
**  @param key  Key scancode.
**  @return     True input finished.
*/
static int InputKey(int key)
{
	char ChatMessage[sizeof(Input) + 40];
	int i;
	char* namestart;
	char* p;
	char* q;

	switch (key) {
		case '\r':
			// Replace ~~ with ~
			for (p = q = Input; *p;) {
				if (*p == '~') {
					++p;
				}
				*q++ = *p++;
			}
			*q = '\0';
#ifdef DEBUG
			if (Input[0] == '-') {
				if (!GameObserve && !GamePaused) {
					CommandLog("input", NoUnitP, FlushCommands, -1, -1, NoUnitP, Input, -1);
					CclCommand(Input + 1);
				}
			} else
#endif
			if (!IsNetworkGame()) {
				if (!GameObserve && !GamePaused) {
					if (HandleCheats(Input)) {
						CommandLog("input", NoUnitP, FlushCommands, -1, -1, NoUnitP, Input, -1);
					}
				}
			}

			// Check for Replay and ffw x
#ifdef DEBUG
			if (strncmp(Input,"ffw ",4) == 0) {
#else
			if (strncmp(Input,"ffw ",4) == 0 && ReplayGameType != ReplayNone) {
#endif
				FastForwardCycle = atoi(&Input[4]);
			}

			if (Input[0]) {
				// Replace ~ with ~~
				for (p = Input; *p; ++p) {
					if (*p == '~') {
						q = p + strlen(p);
						q[1] = '\0';
						while (q > p) {
							*q = *(q - 1);
							--q;
						}
						++p;
					}
				}
				sprintf(ChatMessage, "~%s~<%s>~> %s",
					PlayerColorNames[ThisPlayer->Player],
					ThisPlayer->Name, Input);
				// FIXME: only to selected players ...
				NetworkChatMessage(ChatMessage);
			}
			// FALL THROUGH
		case '\033':
			KeyState = KeyStateCommand;
			ClearStatusLine();
			return 1;
		case '\b':
			if (InputIndex) {
				if (Input[InputIndex - 1] == '~') {
					Input[--InputIndex] = '\0';
				}
				Input[--InputIndex] = '\0';
				ShowInput();
			}
			return 1;
		case '\t':
			namestart = strrchr(Input, ' ');
			if (namestart) {
				++namestart;
			} else {
				namestart = Input;
			}
			if (!strlen(namestart)) {
				return 1;
			}
			for (i = 0; i < PlayerMax; ++i) {
				if (Players[i].Type != PlayerPerson) {
					continue;
				}
				if (!strncasecmp(namestart, Players[i].Name, strlen(namestart))) {
					InputIndex += strlen(Players[i].Name) - strlen(namestart);
					strcpy(namestart, Players[i].Name);
					if (namestart == Input) {
						InputIndex += 2;
						strcat(namestart, ": ");
					}
					ShowInput();
				}
			}
			return 1;
		default:
			if (key >= ' ' && key <= 256) {
				if ((key == '~' && InputIndex < (int)sizeof(Input) - 2) ||
						InputIndex < (int)sizeof(Input) - 1) {
					Input[InputIndex++] = key;
					Input[InputIndex] = '\0';
					if (key == '~') {
						Input[InputIndex++] = key;
						Input[InputIndex] = '\0';
					}
					ShowInput();
				}
				return 1;
			}
			break;
	}
	return 0;
}

/**
**  Save a screenshot.
*/
static void Screenshot(void)
{
	CLFile* fd;
	char filename[30];
	int i;

	for (i = 1; i <= 99; ++i) {
		// FIXME: what if we can't write to this directory?
		sprintf(filename, "screen%02d.png", i);
		if (!(fd = CLopen(filename, CL_OPEN_READ))) {
			break;
		}
		CLclose(fd);
	}
	SaveScreenshotPNG(filename);
}

/**
**  Update KeyModifiers if a key is pressed.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
**
**  @return         1 if modifier found, 0 otherwise
*/
int HandleKeyModifiersDown(unsigned key, unsigned keychar
	__attribute__ ((unused)))
{
	switch (key) {
		case KeyCodeShift:
			KeyModifiers |= ModifierShift;
			return 1;
		case KeyCodeControl:
			KeyModifiers |= ModifierControl;
			return 1;
		case KeyCodeAlt:
			KeyModifiers |= ModifierAlt;
			// maxy: disabled
			if (InterfaceState == IfaceStateNormal) {
				SelectedUnitChanged();		// VLADI: to allow alt-buttons
			}
			return 1;
		case KeyCodeSuper:
			KeyModifiers |= ModifierSuper;
			return 1;
		case KeyCodeHyper:
			KeyModifiers |= ModifierHyper;
			return 1;
		case KeyCodePrint:
			Screenshot();
			SetMessage("Screenshot made.");
			return 1;
		default:
			break;
	}
	return 0;
}

/**
**  Update KeyModifiers if a key is released.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
**
**  @return         1 if modifier found, 0 otherwise
*/
int HandleKeyModifiersUp(unsigned key,
	unsigned keychar __attribute__((unused)))
{
	switch (key) {
		case KeyCodeShift:
			KeyModifiers &= ~ModifierShift;
			return 1;
		case KeyCodeControl:
			KeyModifiers &= ~ModifierControl;
			return 1;
		case KeyCodeAlt:
			KeyModifiers &= ~ModifierAlt;
			// maxy: disabled
			if (InterfaceState == IfaceStateNormal) {
				SelectedUnitChanged(); // VLADI: to allow alt-buttons
			}
			return 1;
		case KeyCodeSuper:
			KeyModifiers &= ~ModifierSuper;
			return 1;
		case KeyCodeHyper:
			KeyModifiers &= ~ModifierHyper;
			return 1;
	}
	return 0;
}

/**
**  Handle key down.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
*/
void HandleKeyDown(unsigned key, unsigned keychar)
{
	if (HandleKeyModifiersDown(key, keychar)) {
		return;
	}

	// Handle All other keys

	// Command line input: for message or cheat
	if (KeyState == KeyStateInput && keychar) {
		InputKey(keychar);
	} else {
		// If no modifier look if button bound
		if (!(KeyModifiers & (ModifierControl | ModifierAlt |
				ModifierSuper | ModifierHyper))) {
			if (!GameObserve && !GamePaused) {
				if (DoButtonPanelKey(key)) {
					return;
				}
			}
		}
		CommandKey(key);
	}
}

/**
**  Handle key up.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
*/
void HandleKeyUp(unsigned key, unsigned keychar)
{
	if (HandleKeyModifiersUp(key, keychar)) {
		return;
	}

	switch (key) {
		case KeyCodeUp:
		case KeyCodeKP8:
			KeyScrollState &= ~ScrollUp;
			break;
		case KeyCodeDown:
		case KeyCodeKP2:
			KeyScrollState &= ~ScrollDown;
			break;
		case KeyCodeLeft:
		case KeyCodeKP4:
			KeyScrollState &= ~ScrollLeft;
			break;
		case KeyCodeRight:
		case KeyCodeKP6:
			KeyScrollState &= ~ScrollRight;
			break;
		default:
			break;
	}
}

/**
**  Handle key up.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
*/
void HandleKeyRepeat(unsigned key __attribute__((unused)),
	unsigned keychar)
{
	if (KeyState == KeyStateInput && keychar) {
		InputKey(keychar);
	}
}

/**
**  Handle the mouse in scroll area
**
**  @param x  Screen X position.
**  @param y  Screen Y position.
**
**  @return   1 if the mouse is in the scroll area, 0 otherwise
*/
int HandleMouseScrollArea(int x, int y)
{
	if (x < SCROLL_LEFT) {
		if (y < SCROLL_UP) {
			CursorOn = CursorOnScrollLeftUp;
			MouseScrollState = ScrollLeftUp;
			GameCursor = TheUI.ArrowNW.Cursor;
		} else if (y > SCROLL_DOWN) {
			CursorOn = CursorOnScrollLeftDown;
			MouseScrollState = ScrollLeftDown;
			GameCursor = TheUI.ArrowSW.Cursor;
		} else {
			CursorOn = CursorOnScrollLeft;
			MouseScrollState = ScrollLeft;
			GameCursor = TheUI.ArrowW.Cursor;
		}
	} else if (x > SCROLL_RIGHT) {
		if (y < SCROLL_UP) {
			CursorOn = CursorOnScrollRightUp;
			MouseScrollState = ScrollRightUp;
			GameCursor = TheUI.ArrowNE.Cursor;
		} else if (y > SCROLL_DOWN) {
			CursorOn = CursorOnScrollRightDown;
			MouseScrollState = ScrollRightDown;
			GameCursor = TheUI.ArrowSE.Cursor;
		} else {
			CursorOn = CursorOnScrollRight;
			MouseScrollState = ScrollRight;
			GameCursor = TheUI.ArrowE.Cursor;
		}
	} else if (y < SCROLL_UP) {
		CursorOn = CursorOnScrollUp;
		MouseScrollState = ScrollUp;
		GameCursor = TheUI.ArrowN.Cursor;
	} else if (y > SCROLL_DOWN) {
		CursorOn = CursorOnScrollDown;
		MouseScrollState = ScrollDown;
		GameCursor = TheUI.ArrowS.Cursor;
	} else {
		return 0;
	}
	return 1;
}

/**
**  Keep coordinates in window and update cursor position
**
**  @param x  screen pixel X position.
**  @param y  screen pixel Y position.
*/
void HandleCursorMove(int* x, int* y)
{
	//
	//  Reduce coordinates to window-size.
	//
	if (*x < 0) {
		*x = 0;
	} else if (*x >= VideoWidth) {
		*x = VideoWidth - 1;
	}
	if (*y < 0) {
		*y = 0;
	} else if (*y >= VideoHeight) {
		*y = VideoHeight - 1;
	}

	CursorX = *x;
	CursorY = *y;
}

/**
**  Handle movement of the cursor.
**
**  @param x  screen pixel X position.
**  @param y  screen pixel Y position.
*/
void HandleMouseMove(int x, int y)
{
	HandleCursorMove(&x, &y);
	UIHandleMouseMove(x, y);
}

/**
**  Called if mouse button pressed down.
**
**  @param button  Mouse button number (0 left, 1 middle, 2 right)
*/
void HandleButtonDown(unsigned button)
{
	UIHandleButtonDown(button);
}

/**
**  Called if mouse button released.
**
**  FIXME: the mouse handling should be complete rewritten
**  FIXME: this is needed for double click, long click,...
**
**  @param button  Mouse button number (0 left, 1 middle, 2 right)
*/
void HandleButtonUp(unsigned button)
{
	UIHandleButtonUp(button);
}

/*----------------------------------------------------------------------------
--  Lowlevel input functions
----------------------------------------------------------------------------*/

int DoubleClickDelay = 300;      /// Time to detect double clicks.
int HoldClickDelay = 1000;       /// Time to detect hold clicks.

static enum {
	InitialMouseState,                  /// start state
	ClickedMouseState,                  /// button is clicked
} MouseState;                           /// Current state of mouse

static int MouseX;                       /// Last mouse X position
static int MouseY;                       /// Last mouse Y position
static unsigned LastMouseButton;         /// last mouse button handled
static unsigned StartMouseTicks;         /// Ticks of first click
static unsigned LastMouseTicks;          /// Ticks of last mouse event

/**
**  Called if any mouse button is pressed down
**
**  Handles event conversion to double click, dragging, hold.
**
**  FIXME: dragging is not supported.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param button     Mouse button pressed.
*/
void InputMouseButtonPress(const EventCallback* callbacks,
	unsigned ticks, unsigned button)
{
	//
	//  Button new pressed.
	//
	if (!(MouseButtons & (1 << button))) {
		MouseButtons |= (1 << button);
		//
		//  Detect double click
		//
		if (MouseState == ClickedMouseState && button == LastMouseButton &&
				ticks < StartMouseTicks + DoubleClickDelay) {
			MouseButtons |= (1 << button) << MouseDoubleShift;
			button |= button << MouseDoubleShift;
		} else {
			MouseState = InitialMouseState;
			StartMouseTicks = ticks;
			LastMouseButton = button;
		}
	}
	LastMouseTicks = ticks;

	callbacks->ButtonPressed(button);
}

/**
**  Called if any mouse button is released up
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param button     Mouse button released.
*/
void InputMouseButtonRelease(const EventCallback* callbacks,
	unsigned ticks, unsigned button)
{
	unsigned mask;

	//
	//  Same button before pressed.
	//
	if (button == LastMouseButton && MouseState == InitialMouseState) {
		MouseState = ClickedMouseState;
	} else {
		LastMouseButton = 0;
		MouseState = InitialMouseState;
	}
	LastMouseTicks = ticks;

	mask = 0;
	if (MouseButtons & ((1 << button) << MouseDoubleShift)) {
		mask |= button << MouseDoubleShift;
	}
	if (MouseButtons & ((1 << button) << MouseDragShift)) {
		mask |= button << MouseDragShift;
	}
	if (MouseButtons & ((1 << button) << MouseHoldShift)) {
		mask |= button << MouseHoldShift;
	}
	MouseButtons &= ~(0x01010101 << button);

	callbacks->ButtonReleased(button | mask);
}

/**
**  Called if the mouse is moved
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param x          X movement
**  @param y          Y movement
*/
void InputMouseMove(const EventCallback* callbacks,
	unsigned ticks, int x, int y)
{
	// Don't reset the mouse state unless we really moved
	if (MouseX != x || MouseY != y) {
		MouseState = InitialMouseState;
		LastMouseTicks = ticks;
		MouseX = x;
		MouseY = y;
	}
	callbacks->MouseMoved(x, y);
}

/**
**  Called if the mouse exits the game window (when supported by videomode)
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**
*/
void InputMouseExit(const EventCallback* callbacks,
	unsigned ticks __attribute__((unused)))
{
	// FIXME: should we do anything here with ticks? don't know, but conform others
	// JOHNS: called by callback HandleMouseExit();
	callbacks->MouseExit();
}

/**
**  Called each frame to handle mouse timeouts.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
*/
void InputMouseTimeout(const EventCallback* callbacks, unsigned ticks)
{
	if (MouseButtons & (1 << LastMouseButton)) {
		if (ticks > StartMouseTicks + DoubleClickDelay) {
			MouseState = InitialMouseState;
		}
		if (ticks > LastMouseTicks + HoldClickDelay) {
			LastMouseTicks = ticks;
			MouseButtons |= (1 << LastMouseButton) << MouseHoldShift;
			callbacks->ButtonPressed(LastMouseButton |
				(LastMouseButton << MouseHoldShift));
		}
	}
}


static int HoldKeyDelay = 250;               /// Time to detect hold key
static int HoldKeyAdditionalDelay = 50;      /// Time to detect additional hold key

static unsigned LastIKey;                    /// last key handled
static unsigned LastIKeyChar;                /// last keychar handled
static unsigned LastKeyTicks;                /// Ticks of last key

/**
**  Handle keyboard key press.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param ikey       Key scancode.
**  @param ikeychar   Character code.
*/
void InputKeyButtonPress(const EventCallback* callbacks,
	unsigned ticks, unsigned ikey, unsigned ikeychar)
{
	LastIKey = ikey;
	LastIKeyChar = ikeychar;
	LastKeyTicks = ticks;
	callbacks->KeyPressed(ikey, ikeychar);
}

/**
**  Handle keyboard key release.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param ikey       Key scancode.
**  @param ikeychar   Character code.
*/
void InputKeyButtonRelease(const EventCallback* callbacks,
	unsigned ticks __attribute__((unused)), unsigned ikey,
	unsigned ikeychar)
{
	if (ikey == LastIKey) {
		LastIKey = 0;
	}
	callbacks->KeyReleased(ikey, ikeychar);
}

/**
**  Called each frame to handle keyboard timeouts.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
*/
void InputKeyTimeout(const EventCallback* callbacks, unsigned ticks)
{
	if (LastIKey && ticks > LastKeyTicks + HoldKeyDelay) {
		LastKeyTicks = ticks - (HoldKeyDelay - HoldKeyAdditionalDelay);
		callbacks->KeyRepeated(LastIKey, LastIKeyChar);
	}
}

//@}
