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
/**@name interface.cpp - The interface. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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
#include "sound.h"
#include "sound_server.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "interface.h"
#include "cursor.h"
#include "ui.h"
#include "menus.h"
#include "script.h"
#include "tileset.h"
#include "minimap.h"
#include "network.h"
#include "font.h"
#include "campaign.h"
#include "video.h"
#include "iolib.h"
#include "commands.h"
#include "ai.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static int SavedMapPositionX[3];     /// Saved map position X
static int SavedMapPositionY[3];     /// Saved map position Y
static char Input[80];               /// line input for messages/long commands
static int InputIndex;               /// current index into input
static char InputStatusLine[99];     /// Last input status line
char DefaultGroupKeys[] = "0123456789`";/// Default group keys
char *UiGroupKeys = DefaultGroupKeys;/// Up to 11 keys, last unselect. Default for qwerty
char GameRunning;                    /// Current running state
char GamePaused;                     /// Current pause state
char GameObserve;                    /// Observe mode
char SkipGameCycle;                  /// Skip the next game cycle
char BigMapMode;                     /// Show only the map
enum _iface_state_ InterfaceState;   /// Current interface state
bool GodMode;                        /// Invincibility cheat
enum _key_state_ KeyState;           /// current key state

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Show input.
*/
static void ShowInput(void)
{
	char *input;

	sprintf(InputStatusLine, _("MESSAGE:%s~!_"), Input);
	input = InputStatusLine;
	// FIXME: This is slow!
	while (UI.StatusLine.Font->Width(input) > UI.StatusLine.Width) {
		++input;
	}
	KeyState = KeyStateCommand;
	UI.StatusLine.Clear();
	UI.StatusLine.Set(input);
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
	CUnit **units;
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
		UI.SelectedViewport->Center(x, y, TileSizeX / 2, TileSizeY / 2);
	}
}

/**
**  Select group.
**
**  @param group  Group number to select.
*/
static void UiSelectGroup(unsigned group)
{
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
	CUnit **units;
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
	if (SoundEnabled()) {
		if (IsEffectsEnabled()) {
			SetEffectsEnabled(false);
			SetMusicEnabled(false);
		} else {
			SetEffectsEnabled(true);
			SetMusicEnabled(true);
		}
	}

	if (IsEffectsEnabled()) {
		UI.StatusLine.Set(_("Sound is on."));
	} else {
		UI.StatusLine.Set(_("Sound is off."));
	}
}

/**
**  Toggle music on / off.
*/
static void UiToggleMusic(void)
{
	static int vol;
	if (GetMusicVolume()) {
		vol = GetMusicVolume();
		SetMusicVolume(0);
		UI.StatusLine.Set(_("Music is off."));
	} else {
		SetMusicVolume(vol);
		UI.StatusLine.Set(_("Music is on."));
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
			UI.StatusLine.Set(_("Game Paused"));
		} else {
			UI.StatusLine.Set(_("Game Resumed"));
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
		UI.StatusLine.Set(_("Game Paused"));
	}
#if 0
	ProcessMenu("menu-game", 0);
#else
	if (UI.MenuButton.Callback) {
		UI.MenuButton.Callback->action("");
	}
#endif
}

/**
**  Enter help menu
*/
static void UiEnterHelpMenu(void)
{
	if (!IsNetworkGame()) {
		GamePaused = 1;
		UI.StatusLine.Set(_("Game Paused"));
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
		UI.StatusLine.Set(_("Game Paused"));
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
		UI.StatusLine.Set(_("Game Paused"));
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
		UI.StatusLine.Set(_("Game Paused"));
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
		UI.StatusLine.Set(_("Game Paused"));
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
		UI.StatusLine.Set(_("Game Paused"));
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
		UI.StatusLine.Set(_("Game Paused"));
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
		UI.StatusLine.Set(_("Game Paused"));
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
		UI.StatusLine.Set(_("Game Paused"));
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
		UI.StatusLine.Set(_("Game Paused"));
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
		mapx = UI.MapArea.X;
		mapy = UI.MapArea.Y;
		mapex = UI.MapArea.EndX;
		mapey = UI.MapArea.EndY;

		UI.MapArea.X = 0;
		UI.MapArea.Y = 0;
		UI.MapArea.EndX = Video.Width - 1;
		UI.MapArea.EndY = Video.Height - 1;

		SetViewportMode(UI.ViewportMode);

		UI.StatusLine.Set(_("Big map enabled"));
	} else {
		UI.MapArea.X = mapx;
		UI.MapArea.Y = mapy;
		UI.MapArea.EndX = mapex;
		UI.MapArea.EndY = mapey;

		SetViewportMode(UI.ViewportMode);

		UI.StatusLine.Set(_("Returning to old map"));
	}
}

/**
**  Increase game speed.
*/
static void UiIncreaseGameSpeed(void)
{
	VideoSyncSpeed += 10;
	SetVideoSync();
	UI.StatusLine.Set(_("Faster"));
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
	UI.StatusLine.Set(_("Slower"));
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
		UI.SelectedViewport->Center(x, y, TileSizeX / 2, TileSizeY / 2);
	}
}

/**
**  Save current map position.
**
**  @param position  Map position slot.
*/
static void UiSaveMapPosition(unsigned position)
{
	SavedMapPositionX[position] = UI.SelectedViewport->MapX;
	SavedMapPositionY[position] = UI.SelectedViewport->MapY;
}

/**
**  Recall map position.
**
**  @param position  Map position slot.
*/
static void UiRecallMapPosition(unsigned position)
{
	UI.SelectedViewport->Set(
		SavedMapPositionX[position], SavedMapPositionY[position], TileSizeX / 2, TileSizeY / 2);
}

/**
**  Toggle terrain display on/off.
*/
static void UiToggleTerrain(void)
{
	UI.Minimap.WithTerrain ^= 1;
	if (UI.Minimap.WithTerrain) {
		UI.StatusLine.Set(_("Terrain displayed."));
	} else {
		UI.StatusLine.Set(_("Terrain hidden."));
	}
}

/**
**  Find the next idle worker, select it, and center on it
*/
static void UiFindIdleWorker(void)
{
	CUnit *unit;
	// FIXME: static variable, is not needed.
	static CUnit *LastIdleWorker = NoUnitP;

	unit = FindIdleWorker(ThisPlayer, LastIdleWorker);
	if (unit != NoUnitP) {
		LastIdleWorker = unit;
		SelectSingleUnit(unit);
		UI.StatusLine.Clear();
		ClearCosts();
		CurrentButtonLevel = 0;
		PlayUnitSound(Selected[0], VoiceSelected);
		SelectionChanged();
		UI.SelectedViewport->Center(unit->X, unit->Y, TileSizeX / 2, TileSizeY / 2);
	}
}

/**
**  Toggle grab mouse on/off.
*/
static void UiToggleGrabMouse(void)
{
	DebugPrint("%x\n" _C_ KeyModifiers);
	ToggleGrabMouse(0);
	UI.StatusLine.Set(_("Grab mouse toggled."));
}

/**
**  Track unit, the viewport follows the unit.
*/
static void UiTrackUnit(void)
{
	if (UI.SelectedViewport->Unit == Selected[0]) {
		UI.SelectedViewport->Unit = NULL;
	} else {
		UI.SelectedViewport->Unit = Selected[0];
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
	char *ptr;

	// FIXME: don't handle unicode well. Should work on all latin keyboard.
	if ((ptr = strchr(UiGroupKeys, key))) {
		key = '0' + ptr - UiGroupKeys;
		if (key > '9') {
			key = SDLK_BACKQUOTE;
		}
	}

	switch (key) {
		// Return enters chat/input mode.
		case SDLK_RETURN:
		case SDLK_KP_ENTER: // RETURN
			UiBeginInput();
			return 1;

		// Unselect everything
		case SDLK_CARET:
		case SDLK_BACKQUOTE:
			UiUnselectAll();
			break;

		// Group selection
		case '0': case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
		case '9':
			if (KeyModifiers & ModifierShift) {
				if (KeyModifiers & (ModifierAlt | ModifierDoublePress)) {
					UiCenterOnGroup(key - '0');
				} else if (KeyModifiers & ModifierControl) {
					UiAddToGroup(key - '0');
				} else {
					UiAddGroupToSelection(key - '0');
				}
			} else {
				if (KeyModifiers & (ModifierAlt | ModifierDoublePress)) {
					UiCenterOnGroup(key - '0');
				} else if (KeyModifiers & ModifierControl) {
					UiDefineGroup(key - '0');
				} else {
					UiSelectGroup(key - '0');
				}
			}
			break;

		case 'p': // CTRL+P, ALT+P Toggle pause
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			// FALL THROUGH (CTRL+P, ALT+P)
		case SDLK_PAUSE:
			UiTogglePause();
			break;

		case SDLK_F1:
			UiEnterHelpMenu();
			break;

		case SDLK_F2:
		case SDLK_F3:
		case SDLK_F4: // Set/Goto place
			if (KeyModifiers & ModifierShift) {
				UiSaveMapPosition(key - SDLK_F2);
			} else {
				UiRecallMapPosition(key - SDLK_F2);
			}
			break;

		case 'm': // CTRL+M Turn music on / off
		          // ALT+M, F10 Game menu
			if (KeyModifiers & ModifierControl) {
				UiToggleMusic();
				SavePreferences();
				break;
			}
			if (!(KeyModifiers & ModifierAlt)) {
				break;
			}
			// FALL THROUGH (ALT+M)
		case SDLK_F10: // Game Options menu
			if (KeyState != KeyStateInput) {
				UiEnterMenu();
			}
			break;

		case SDLK_F5: // Options menu
			if (KeyState != KeyStateInput) {
				UiEnterOptionsMenu();
			}
			break;

		case SDLK_F7: // Sound Options menu
			if (KeyState != KeyStateInput) {
				UiEnterSoundOptionsMenu();
			}
			break;

		case SDLK_F8: // Speed Options menu
			if (KeyState != KeyStateInput) {
				UiEnterSpeedOptionsMenu();
			}
			break;

		case SDLK_F9: // Preferences menu
			if (KeyState != KeyStateInput) {
				UiEnterPreferencesOptionsMenu();
			}
			break;

		case SDLK_EQUALS: // plus is shift-equals.
		case SDLK_KP_PLUS:
			UiIncreaseGameSpeed();
			break;

		case SDLK_MINUS: // - Slower
		case SDLK_KP_MINUS:
			UiDecreaseGameSpeed();
			break;

		case 'l': // ALT+L, F12 load game menu
			if (!(KeyModifiers & ModifierAlt)) {
				break;
			}
			// FALL THROUGH (ALT+L)
		case SDLK_F12:
			UiEnterLoadGameMenu();
			break;

		case 's': // ALT+S, F11 save game menu
		          // CTRL+S - Turn sound on / off
			if (KeyModifiers & ModifierControl) {
				UiToggleSound();
				SavePreferences();
				break;
			}
			if (!(KeyModifiers & ModifierAlt)) {
				break;
			}
			// FALL THROUGH (ALT+S)
		case SDLK_F11:
			UiEnterSaveGameMenu();
			break;

		case 't': // ALT+T, CTRL+T Track unit
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiTrackUnit();
			break;

		case 'b': // ALT+B, CTRL+B Toggle big map
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiToggleBigMap();
			break;

		case 'c': // ALT+C, CTRL+C C center on units
			UiCenterOnSelected();
			break;

		case 'f': // ALT+F, CTRL+F toggle fullscreen
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			ToggleFullScreen();
			SavePreferences();
			break;

		case 'g': // ALT+G, CTRL+G grab mouse pointer
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiToggleGrabMouse();
			break;

		case 'h': // ALT+H, CTRL+H Help menu
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiEnterHelpMenu();
			break;

		case SDLK_SPACE: // center on last action
			CenterOnMessage();
			break;

		case SDLK_TAB: // TAB toggles minimap.
					// FIXME: more...
					// FIXME: shift+TAB
			if (KeyModifiers & ModifierAlt) {
				break;
			}
			UiToggleTerrain();
			break;

		case 'x': // ALT+X, CTRL+X: Exit game
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiExitConfirmMenu();
			break;

		case 'q': // ALT+Q, CTRL+Q: Quit level
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiQuitToMenuConfirmMenu();
			break;

		case 'r': // ALT+R, CTRL+R: Restart scenario
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiRestartConfirmMenu();
			break;

		case 'i':
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			// FALL THROUGH
		case SDLK_PERIOD: // ., ALT+I, CTRL+I: Find idle worker
			UiFindIdleWorker();
			break;

		case 'v': // ALT+V, CTRL+V: Viewport
			if (KeyModifiers & ModifierControl) {
				CycleViewportMode(-1);
			} else {
				CycleViewportMode(1);
			}
			break;

		case SDLK_UP:
		case SDLK_KP8:
			KeyScrollState |= ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP2:
			KeyScrollState |= ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP4:
			KeyScrollState |= ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP6:
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
int HandleCheats(const char *input)
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
	char *namestart;
	char *p;
	char *q;

	switch (key) {
		case SDLK_RETURN:
		case SDLK_KP_ENTER: // RETURN
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
			if (strncmp(Input, "ffw ", 4) == 0) {
#else
			if (strncmp(Input, "ffw ", 4) == 0 && ReplayGameType != ReplayNone) {
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
					PlayerColorNames[ThisPlayer->Index],
					ThisPlayer->Name, Input);
				// FIXME: only to selected players ...
				NetworkChatMessage(ChatMessage);
			}
			// FALL THROUGH
		case SDLK_ESCAPE:
			KeyState = KeyStateCommand;
			UI.StatusLine.Clear();
			return 1;

		case SDLK_BACKSPACE:
			if (InputIndex) {
				if (Input[InputIndex - 1] == '~') {
					Input[--InputIndex] = '\0';
				}
				Input[--InputIndex] = '\0';
				ShowInput();
			}
			return 1;

		case SDLK_TAB:
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
	CFile fd;
	char filename[30];
	int i;

	for (i = 1; i <= 99; ++i) {
		// FIXME: what if we can't write to this directory?
		sprintf(filename, "screen%02d.png", i);
		if (fd.open(filename, CL_OPEN_READ) == -1) {
			break;
		}
		fd.close();
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
int HandleKeyModifiersDown(unsigned key, unsigned keychar)
{
	switch (key) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			KeyModifiers |= ModifierShift;
			return 1;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			KeyModifiers |= ModifierControl;
			return 1;
		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_LMETA:
		case SDLK_RMETA:
			KeyModifiers |= ModifierAlt;
			// maxy: disabled
			if (InterfaceState == IfaceStateNormal) {
				SelectedUnitChanged(); // VLADI: to allow alt-buttons
			}
			return 1;
		case SDLK_LSUPER:
		case SDLK_RSUPER:
			KeyModifiers |= ModifierSuper;
			return 1;
		case SDLK_SYSREQ:
		case SDLK_PRINT:
			Screenshot();
			if (GameRunning) {
				SetMessage(_("Screenshot made."));
			}
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
int HandleKeyModifiersUp(unsigned key, unsigned keychar)
{
	switch (key) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			KeyModifiers &= ~ModifierShift;
			return 1;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			KeyModifiers &= ~ModifierControl;
			return 1;
		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_LMETA:
		case SDLK_RMETA:
			KeyModifiers &= ~ModifierAlt;
			// maxy: disabled
			if (InterfaceState == IfaceStateNormal) {
				SelectedUnitChanged(); // VLADI: to allow alt-buttons
			}
			return 1;
		case SDLK_LSUPER:
		case SDLK_RSUPER:
			KeyModifiers &= ~ModifierSuper;
			return 1;
	}
	return 0;
}

/**
**  Check if a key is from the keypad and convert to ascii
*/
static bool IsKeyPad(unsigned key, unsigned *kp)
{
	if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		*kp = SDLK_0 + (key - SDLK_KP0);
	} else if (key == SDLK_KP_PERIOD) {
		*kp = SDLK_PERIOD;
	} else if (key == SDLK_KP_DIVIDE) {
		*kp = SDLK_SLASH;
	} else if (key == SDLK_KP_MULTIPLY) {
		*kp = SDLK_ASTERISK;
	} else if (key == SDLK_KP_MINUS) {
		*kp = SDLK_MINUS;
	} else if (key == SDLK_KP_PLUS) {
		*kp = SDLK_PLUS;
	} else if (key == SDLK_KP_ENTER) {
		*kp = SDLK_RETURN;
	} else if (key == SDLK_KP_EQUALS) {
		*kp = SDLK_EQUALS;
	} else  {
		*kp = SDLK_UNKNOWN;
		return false;
	}
	return true;
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
	unsigned kp = 0;
	if (KeyState == KeyStateInput && (keychar || IsKeyPad(key, &kp))) {
		InputKey(kp ? kp : keychar);
	} else {
		// If no modifier look if button bound
		if (!(KeyModifiers & (ModifierControl | ModifierAlt |
				ModifierSuper))) {
			if (!GameObserve && !GamePaused) {
				if (UI.ButtonPanel.DoKey(key)) {
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
		case SDLK_UP:
		case SDLK_KP8:
			KeyScrollState &= ~ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP2:
			KeyScrollState &= ~ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP4:
			KeyScrollState &= ~ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP6:
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
void HandleKeyRepeat(unsigned key, unsigned keychar)
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
			GameCursor = UI.ArrowNW.Cursor;
		} else if (y > SCROLL_DOWN) {
			CursorOn = CursorOnScrollLeftDown;
			MouseScrollState = ScrollLeftDown;
			GameCursor = UI.ArrowSW.Cursor;
		} else {
			CursorOn = CursorOnScrollLeft;
			MouseScrollState = ScrollLeft;
			GameCursor = UI.ArrowW.Cursor;
		}
	} else if (x > SCROLL_RIGHT) {
		if (y < SCROLL_UP) {
			CursorOn = CursorOnScrollRightUp;
			MouseScrollState = ScrollRightUp;
			GameCursor = UI.ArrowNE.Cursor;
		} else if (y > SCROLL_DOWN) {
			CursorOn = CursorOnScrollRightDown;
			MouseScrollState = ScrollRightDown;
			GameCursor = UI.ArrowSE.Cursor;
		} else {
			CursorOn = CursorOnScrollRight;
			MouseScrollState = ScrollRight;
			GameCursor = UI.ArrowE.Cursor;
		}
	} else if (y < SCROLL_UP) {
		CursorOn = CursorOnScrollUp;
		MouseScrollState = ScrollUp;
		GameCursor = UI.ArrowN.Cursor;
	} else if (y > SCROLL_DOWN) {
		CursorOn = CursorOnScrollDown;
		MouseScrollState = ScrollDown;
		GameCursor = UI.ArrowS.Cursor;
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
void HandleCursorMove(int *x, int *y)
{
	//
	//  Reduce coordinates to window-size.
	//
	if (*x < 0) {
		*x = 0;
	} else if (*x >= Video.Width) {
		*x = Video.Width - 1;
	}
	if (*y < 0) {
		*y = 0;
	} else if (*y >= Video.Height) {
		*y = Video.Height - 1;
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
**  @todo FIXME: the mouse handling should be complete rewritten
**  @todo FIXME: this is needed for double click, long click,...
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

int DoubleClickDelay = 300;             /// Time to detect double clicks.
int HoldClickDelay = 1000;              /// Time to detect hold clicks.

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
void InputMouseButtonPress(const EventCallback *callbacks,
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
void InputMouseButtonRelease(const EventCallback *callbacks,
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
void InputMouseMove(const EventCallback *callbacks,
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
void InputMouseExit(const EventCallback *callbacks, unsigned ticks)
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
void InputMouseTimeout(const EventCallback *callbacks, unsigned ticks)
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
static unsigned DoubleKey;                   /// last key pressed

/**
**  Handle keyboard key press.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param ikey       Key scancode.
**  @param ikeychar   Character code.
*/
void InputKeyButtonPress(const EventCallback *callbacks,
	unsigned ticks, unsigned ikey, unsigned ikeychar)
{
	if (!LastIKey && DoubleKey == ikey &&
			ticks < LastKeyTicks + DoubleClickDelay) {
		KeyModifiers |= ModifierDoublePress;
	}
	DoubleKey = ikey;
	LastIKey = ikey;
	LastIKeyChar = ikeychar;
	LastKeyTicks = ticks;
	callbacks->KeyPressed(ikey, ikeychar);
	KeyModifiers &= ~ModifierDoublePress;
}

/**
**  Handle keyboard key release.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param ikey       Key scancode.
**  @param ikeychar   Character code.
*/
void InputKeyButtonRelease(const EventCallback *callbacks,
	unsigned ticks, unsigned ikey, unsigned ikeychar)
{
	if (ikey == LastIKey) {
		LastIKey = 0;
	}
	LastKeyTicks = ticks;
	callbacks->KeyReleased(ikey, ikeychar);
}

/**
**  Called each frame to handle keyboard timeouts.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
*/
void InputKeyTimeout(const EventCallback *callbacks, unsigned ticks)
{
	if (LastIKey && ticks > LastKeyTicks + HoldKeyDelay) {
		LastKeyTicks = ticks - (HoldKeyDelay - HoldKeyAdditionalDelay);
		callbacks->KeyRepeated(LastIKey, LastIKeyChar);
	}
}

/**
**  Get double click delay
*/
int GetDoubleClickDelay(void)
{
	return DoubleClickDelay;
}

/**
**  Set double click delay
**
**  @param delay  Double click delay
*/
void SetDoubleClickDelay(int delay)
{
	DoubleClickDelay = delay;
}

/**
**  Get hold click delay
*/
int GetHoldClickDelay(void)
{
	return HoldClickDelay;
}

/**
**  Set hold click delay
**
**  @param delay  Hold click delay
*/
void SetHoldClickDelay(int delay)
{
	HoldClickDelay = delay;
}

//@}
