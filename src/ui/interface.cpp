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
//      (c) Copyright 1998-2011 by Lutz Sammer, Jimmy Salmon and Pali Rohár
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
#include "results.h"
#include "video.h"
#include "iolib.h"
#include "commands.h"
#include "ai.h"
#include "widgets.h"
#include "replay.h"
#include "actions.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

	/// Scrolling area (<= 15 y)
#define SCROLL_UP     15
	/// Scrolling area (>= VideoHeight - 16 y)
#define SCROLL_DOWN   (Video.Height - 16)
	/// Scrolling area (<= 15 y)
#define SCROLL_LEFT   15
	/// Scrolling area (>= VideoWidth - 16 x)
#define SCROLL_RIGHT  (Video.Width - 16)

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static int SavedMapPositionX[3];     /// Saved map position X
static int SavedMapPositionY[3];     /// Saved map position Y
static char Input[80];               /// line input for messages/long commands
static int InputIndex;               /// current index into input
static char InputStatusLine[99];     /// Last input status line
const char DefaultGroupKeys[] = "0123456789`";/// Default group keys
const char *UiGroupKeys = DefaultGroupKeys;/// Up to 11 keys, last unselect. Default for qwerty
bool GameRunning;                    /// Current running state
bool GamePaused;                     /// Current pause state
bool GameObserve;                    /// Observe mode
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
static void ShowInput()
{
	char *input;

	snprintf(InputStatusLine, sizeof(InputStatusLine), _("MESSAGE:%s~!_"), Input);
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
static void UiBeginInput()
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
static void UiUnselectAll()
{
	UnSelectAll();
	NetworkSendSelection((CUnit **)NULL, 0);
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
static void UiCenterOnGroup(unsigned group, GroupSelectionMode mode = SELECTABLE_BY_RECTANGLE_ONLY)
{
	CUnit **units;
	int n;
	Vec2i pos = {-1, -1};

	n = GetNumberUnitsOfGroup(group, SELECT_ALL);
	if (n--) {
		units = GetUnitsOfGroup(group);
		// FIXME: what should we do with the removed units? ignore?
		if (units[n]->Type && units[n]->Type->CanSelect(mode)) {
			pos = units[n]->tilePos;
		}

		while (n--) {
			if (units[n]->Type && units[n]->Type->CanSelect(mode)) {
				if (pos.x != -1) {
					pos += (units[n]->tilePos - pos) / 2;
				} else {
					pos = units[n]->tilePos;
				}
			}
		}
		if (pos.x != -1) {
			UI.SelectedViewport->Center(pos, PixelTileSize / 2);
		}
	}
}

/**
**  Select group.
**
**  @param group  Group number to select.
*/
static void UiSelectGroup(unsigned group, GroupSelectionMode mode = SELECTABLE_BY_RECTANGLE_ONLY)
{
	SelectGroup(group, mode);
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

	if (!(n = GetNumberUnitsOfGroup(group, SELECT_ALL))) {
		return;
	}

	//
	//  Don't allow to mix units and buildings
	//
	if (NumUnits && Selected[0]->Type->Building) {
		return;
	}

	units = GetUnitsOfGroup(group);

	while (n--) {
		if (!(units[n]->Removed || units[n]->Type->Building)) {
			SelectUnit(*units[n]);
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
	for (int i= 0; i < NumSelected; ++i) {
		if (Selected[i]->GroupId) {
			RemoveUnitFromGroups(*Selected[i]);
		}
	}
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
static void UiToggleSound()
{
	if (SoundEnabled()) {
		if (IsEffectsEnabled()) {
			SetEffectsEnabled(false);
			SetMusicEnabled(false);
		} else {
			SetEffectsEnabled(true);
			SetMusicEnabled(true);
			CheckMusicFinished(true);
		}
	}

	if (SoundEnabled()) {
		if (IsEffectsEnabled()) {
			UI.StatusLine.Set(_("Sound is on."));
		} else {
			UI.StatusLine.Set(_("Sound is off."));
		}
	}
}

/**
**  Toggle music on / off.
*/
static void UiToggleMusic()
{
	static int vol;
	if (SoundEnabled()) {
		if (GetMusicVolume()) {
			vol = GetMusicVolume();
			SetMusicVolume(0);
			UI.StatusLine.Set(_("Music is off."));
		} else {
			SetMusicVolume(vol);
			UI.StatusLine.Set(_("Music is on."));
		}
	}
}

/**
**  Toggle pause on / off.
*/
void UiTogglePause()
{
	if (!IsNetworkGame()) {
		GamePaused = !GamePaused;
		if (GamePaused) {
			UI.StatusLine.Set(_("Game Paused"));
		} else {
			UI.StatusLine.Set(_("Game Resumed"));
		}
	}
}

/**
**  Toggle big map mode.
**
**  @todo FIXME: We should try to keep the same view, if possible
*/
void UiToggleBigMap()
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
static void UiIncreaseGameSpeed()
{
	if (FastForwardCycle >= GameCycle) {
		return;
	}
	VideoSyncSpeed += 10;
	SetVideoSync();
	UI.StatusLine.Set(_("Faster"));
}

/**
**  Decrease game speed.
*/
static void UiDecreaseGameSpeed()
{
	if (FastForwardCycle >= GameCycle) {
		return;
	}
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
**  Set default game speed.
*/
static void UiSetDefaultGameSpeed()
{
	if (FastForwardCycle >= GameCycle) {
		return;
	}
	VideoSyncSpeed = 100;
	SetVideoSync();
	UI.StatusLine.Set(_("Set default game speed"));
}

/**
**  Center on the selected units.
**
**  @todo Improve this function, try to show all selected units
**        or the most possible units.
*/
static void UiCenterOnSelected()
{
	int n = NumSelected;

	if (n) {
		Vec2i pos = Selected[--n]->tilePos;

		while (n--) {
			pos += (Selected[n]->tilePos - pos) / 2;
		}
		UI.SelectedViewport->Center(pos, PixelTileSize / 2);
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
	const Vec2i savedTilePos = {SavedMapPositionX[position], SavedMapPositionY[position]};

	UI.SelectedViewport->Set(savedTilePos, PixelTileSize / 2);
}

/**
**  Toggle terrain display on/off.
*/
static void UiToggleTerrain()
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
static void UiFindIdleWorker()
{
	// FIXME: static variable, is not needed.
	static CUnit *LastIdleWorker = NoUnitP;

	CUnit *unit = FindIdleWorker(*ThisPlayer, LastIdleWorker);

	if (unit != NoUnitP) {
		LastIdleWorker = unit;
		SelectSingleUnit(*unit);
		UI.StatusLine.Clear();
		ClearCosts();
		CurrentButtonLevel = 0;
		PlayUnitSound(*Selected[0], VoiceSelected);
		SelectionChanged();
		UI.SelectedViewport->Center(unit->tilePos, PixelTileSize / 2);
	}
}

/**
**  Toggle grab mouse on/off.
*/
static void UiToggleGrabMouse()
{
	DebugPrint("%x\n" _C_ KeyModifiers);
	ToggleGrabMouse(0);
	UI.StatusLine.Set(_("Grab mouse toggled."));
}

/**
**  Track unit, the viewport follows the unit.
*/
static void UiTrackUnit()
{
	//Check if player has selected at least 1 unit
	if (!Selected[0])
	{
		UI.SelectedViewport->Unit = NULL;
		return;
	}
	if (UI.SelectedViewport->Unit == Selected[0]) {
		UI.SelectedViewport->Unit = NULL;
	} else {
		UI.SelectedViewport->Unit = Selected[0];
	}
}

/**
**  Call the lua function HandleCommandKey
*/
bool HandleCommandKey(int key)
{
	bool ret;
	int base = lua_gettop(Lua);

	lua_pushstring(Lua, "HandleCommandKey");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	if (!lua_isfunction(Lua, -1)) {
		DebugPrint("No HandleCommandKey function in lua.\n");
		return false;
	}
	lua_pushstring(Lua, SdlKey2Str(key));
	lua_pushboolean(Lua, (KeyModifiers & ModifierControl));
	lua_pushboolean(Lua, (KeyModifiers & ModifierAlt));
	lua_pushboolean(Lua, (KeyModifiers & ModifierShift));
	LuaCall(4, 0);
	if (lua_gettop(Lua) - base == 1) {
		ret = LuaToBoolean(Lua, base + 1);
		lua_pop(Lua, 1);
	} else {
		LuaError(Lua, "HandleCommandKey must return a boolean");
		ret = false;
	}
	return ret;
}

#ifdef DEBUG
extern void ToggleShowBuilListMessages();
#endif

/**
**  Handle keys in command mode.
**
**  @param key  Key scancode.
**
**  @return     True, if key is handled; otherwise false.
*/
static bool CommandKey(int key)
{
	const char *ptr;

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
			return true;

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
					if (KeyModifiers & ModifierDoublePress) {
						UiCenterOnGroup(key - '0', SELECT_ALL);
					} else {
						UiSelectGroup(key - '0', SELECT_ALL);
					}
				} else if (KeyModifiers & ModifierControl) {
					UiAddToGroup(key - '0');
				} else {
					UiAddGroupToSelection(key - '0');
				}
			} else {
				if (KeyModifiers & (ModifierAlt | ModifierDoublePress)) {
					if (KeyModifiers & ModifierAlt) {
						if (KeyModifiers & ModifierDoublePress) {
							UiCenterOnGroup(key - '0', NON_SELECTABLE_BY_RECTANGLE_ONLY);
						} else {
							UiSelectGroup(key - '0', NON_SELECTABLE_BY_RECTANGLE_ONLY);
						}
					} else {
						UiCenterOnGroup(key - '0');
					}
				} else if (KeyModifiers & ModifierControl) {
					UiDefineGroup(key - '0');
				} else {
					UiSelectGroup(key - '0');
				}
			}
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

		case SDLK_SPACE: // center on last action
			CenterOnMessage();
			break;

		case SDLK_EQUALS: // plus is shift-equals.
		case SDLK_KP_PLUS:
			UiIncreaseGameSpeed();
			break;

		case SDLK_MINUS: // - Slower
		case SDLK_KP_MINUS:
			UiDecreaseGameSpeed();
			break;
		case SDLK_KP_MULTIPLY:
			UiSetDefaultGameSpeed();
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

		case 'i':
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			// FALL THROUGH
		case SDLK_PERIOD: // ., ALT+I, CTRL+I: Find idle worker
			UiFindIdleWorker();
			break;

		case 'm': // CTRL+M Turn music on / off
			if (KeyModifiers & ModifierControl) {
				UiToggleMusic();
				SavePreferences();
				break;
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

		case 's': // CTRL+S - Turn sound on / off
			if (KeyModifiers & ModifierControl) {
				UiToggleSound();
				SavePreferences();
				break;
			}
			break;

		case 't': // ALT+T, CTRL+T Track unit
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			UiTrackUnit();
			break;

		case 'v': // ALT+V, CTRL+V: Viewport
			if (KeyModifiers & ModifierControl) {
				CycleViewportMode(-1);
			} else if (KeyModifiers & ModifierAlt) {
				CycleViewportMode(1);
			}
			break;

		case 'e': // CTRL+E Turn messages on / off
			if (KeyModifiers & ModifierControl) {
				ToggleShowMessages();
			}
#ifdef DEBUG
			else
			if (KeyModifiers & ModifierAlt) {
				ToggleShowBuilListMessages();
			}
#endif
			break;

		case SDLK_TAB: // TAB toggles minimap.
					// FIXME: more...
					// FIXME: shift+TAB
			if (KeyModifiers & ModifierAlt) {
				break;
			}
			UiToggleTerrain();
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
			if (HandleCommandKey(key)) {
				break;
			}
			return false;
	}
	return true;
}

#if defined(DEBUG) || defined(PROF)
extern void MapUnmarkUnitGuard(CUnit &unit);
#endif

/**
**  Handle cheats
**
**  @return  1 if a cheat was handled, 0 otherwise
*/
int HandleCheats(const std::string &input)
{
	int ret;

#if defined(DEBUG) || defined(PROF)
	if (input == "ai me") {
		if (ThisPlayer->AiEnabled) {
			// FIXME: UnitGoesUnderFog and UnitGoesOutOfFog change unit refs
			// for human players.  We can't switch back to a human player or
			// we'll be using the wrong ref counts.
#if 0
			ThisPlayer->AiEnabled = 0;
			ThisPlayer->Type = PlayerPerson;
			SetMessage("AI is off, Normal Player");
#else
			SetMessage("Cannot disable 'ai me' cheat");
#endif
		} else {

			for (int j = 0; j < ThisPlayer->TotalNumUnits; ++j) {
				CUnit *guard = ThisPlayer->Units[j];
				bool stand_ground =
					guard->CurrentAction() == UnitActionStandGround;
				if ((stand_ground || guard->IsIdle()) &&
							 !guard->IsUnusable()) {
					MapUnmarkUnitGuard(*guard);
					guard->SubAction = 0;
				}
			}
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
	int base = lua_gettop(Lua);
	lua_pushstring(Lua, "HandleCheats");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	if (!lua_isfunction(Lua, -1)) {
		DebugPrint("No HandleCheats function in lua.\n");
		return 0;
	}
	lua_pushstring(Lua, input.c_str());
	LuaCall(1, 0, false);
	if (lua_gettop(Lua) - base == 1) {
		ret = LuaToBoolean(Lua, -1);
		lua_pop(Lua, 1);
	} else {
		DebugPrint("HandleCheats must return a boolean");
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
					CclCommand(Input + 1, false);
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
				snprintf(ChatMessage, sizeof(ChatMessage), "~%s~<%s>~> %s",
					PlayerColorNames[ThisPlayer->Index].c_str(),
					ThisPlayer->Name.c_str(), Input);
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
				InputIndex = UTF8GetPrev(Input, InputIndex);
				if (InputIndex >= 0) {
					Input[InputIndex] = '\0';
					ShowInput();
				}
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
				if (!strncasecmp(namestart, Players[i].Name.c_str(), strlen(namestart))) {
					InputIndex += strlen(Players[i].Name.c_str()) - strlen(namestart);
					strcpy_s(namestart, sizeof(Input) - (namestart - Input), Players[i].Name.c_str());
					if (namestart == Input) {
						InputIndex += 2;
						strcat_s(namestart, sizeof(Input) - (namestart - Input), ": ");
					}
					ShowInput();
				}
			}
			return 1;

		default:
			if (key >= ' ') {
				gcn::Key k(key);
				std::string kstr = k.toString();
				if (key == '~') {
					if (InputIndex < (int)sizeof(Input) - 2) {
						Input[InputIndex++] = key;
						Input[InputIndex++] = key;
						Input[InputIndex] = '\0';
						ShowInput();
					}
				} else if (InputIndex < (int)(sizeof(Input) - kstr.size())) {
					for (size_t i = 0; i < kstr.size(); ++i) {
						Input[InputIndex++] = kstr[i];
					}
					Input[InputIndex] = '\0';
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
static void Screenshot()
{
	CFile fd;
	char filename[30];
	int i;

	for (i = 1; i <= 99; ++i) {
		// FIXME: what if we can't write to this directory?
		snprintf(filename, sizeof(filename), "screen%02d.png", i);
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
int HandleKeyModifiersDown(unsigned key, unsigned)
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
int HandleKeyModifiersUp(unsigned key, unsigned)
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
void HandleKeyRepeat(unsigned, unsigned keychar)
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

#ifdef USE_TOUCHSCREEN
int DoubleClickDelay = 1000;             /// Time to detect double clicks.
int HoldClickDelay = 2000;              /// Time to detect hold clicks.
#else
int DoubleClickDelay = 300;             /// Time to detect double clicks.
int HoldClickDelay = 1000;              /// Time to detect hold clicks.
#endif

static enum {
	InitialMouseState,                  /// start state
	ClickedMouseState                   /// button is clicked
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
#ifdef USE_TOUCHSCREEN
#define buff 32
	if (((x - buff) <= MouseX && MouseX <= (x + buff)) == 0 ||
			((y - buff) <= MouseY && MouseY <= (y + buff)) == 0) {
		MouseState = InitialMouseState;
		LastMouseTicks = ticks;
	}
	if (MouseX != x || MouseY != y) {
		MouseX = x;
		MouseY = y;
	}
#undef buff
#else
	if (MouseX != x || MouseY != y) {
		MouseState = InitialMouseState;
		LastMouseTicks = ticks;
		MouseX = x;
		MouseY = y;
	}
#endif
	callbacks->MouseMoved(x, y);
}

/**
**  Called if the mouse exits the game window (when supported by videomode)
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**
*/
void InputMouseExit(const EventCallback *callbacks, unsigned)
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
int GetDoubleClickDelay()
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
int GetHoldClickDelay()
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
