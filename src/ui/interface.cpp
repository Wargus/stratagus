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

#include "stratagus.h"

#include "interface.h"

#include "ai.h"
#include "commands.h"
#include "cursor.h"
#include "font.h"
#include "iolib.h"
#include "network.h"
#include "player.h"
#include "replay.h"
#include "sound.h"
#include "sound_server.h"
#include "translate.h"
#include "ui.h"
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"
#include "video.h"
#include "widgets.h"

#include <array>
#include <optional>

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static int ScrollMarginTop = 15;
static int ScrollMarginBottom = 16;
static int ScrollMarginLeft = 15;
static int ScrollMarginRight = 16;

static const char Cursor[] = "~!_";  /// Input cursor
static Vec2i SavedMapPosition[3];    /// Saved map position
static std::array<char, 80> Input{}; /// line input for messages/long commands
static int InputIndex;               /// current index into input
static std::array<decltype(Input), 16> InputHistory = {}; /// History of inputs
static int InputHistoryIdx = 0;      /// Next history idx
static int InputHistoryPos = 0;      /// Current position in history
static int InputHistorySize = 0;     /// History fill size
const char DefaultGroupKeys[] = "0123456789`";/// Default group keys
std::string UiGroupKeys = DefaultGroupKeys;/// Up to 11 keys, last unselect. Default for qwerty
bool GameRunning;                    /// Current running state
bool GamePaused;                     /// Current pause state
bool GameObserve;                    /// Observe mode
bool GameEstablishing;               /// Game establishing mode
double SkipGameCycle;                /// Skip the next n game cycles
bool BigMapMode;                     /// Show only the map
IfaceState InterfaceState;           /// Current interface state
bool GodMode;                        /// Invincibility cheat
EKeyState KeyState;                  /// current key state
static CUnit *LastIdleWorker;        /// Last called idle worker

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static void moveInputContent(int targetPos, int srcPos)
{
	if (targetPos < srcPos) {
		std::move(Input.begin() + srcPos, Input.end(), Input.begin() + targetPos);
	} else {
		std::move_backward(Input.begin() + srcPos, Input.end() + (srcPos - targetPos) - 1, Input.end() - 1);
	}
}

// Replace ~ with ~~
static std::string ReplaceTildeBy2Tilde(std::string_view s)
{
	std::string res;
	for (char c : s) {
		if (c == '~') {
			res += '~';
		}
		res += c;
	}
	return res;
}

static std::string prepareInputToDisplay(std::string_view input, std::size_t index)
{
	std::string res;
	index = std::min(index, input.size());

	for (std::size_t i = 0; i != index; ++i) {
		if (input[i] == '~') {
			res.push_back('~');
		}
		res.push_back(input[i]);
	}
	res += Cursor;
	for (std::size_t i = index; i != input.size(); ++i) {
		if (input[i] == '~') {
			res.push_back('~');
		}
		res.push_back(input[i]);
	}
	return res;
}

/**
**  Show input.
*/
static void ShowInput()
{
	const std::string message = Translate("MESSAGE:%s");
	const std::string escaped_input = prepareInputToDisplay(Input.data(), InputIndex);
	std::string InputStatusLine(escaped_input.size() + message.size(), '\0');

	const auto len = snprintf(InputStatusLine.data(), InputStatusLine.size(), message.c_str(), escaped_input.c_str());
	std::string_view sv(InputStatusLine.c_str(), len);
	// FIXME: This is slow!
	while (UI.StatusLine.Font->Width(sv) > UI.StatusLine.Width) {
		if (sv[0] == '~') sv = sv.substr(1);
		sv = sv.substr(UTF8GetNext(sv, 0));
	}
	KeyState = EKeyState::Command;
	UI.StatusLine.Clear();
	UI.StatusLine.Set(std::string(sv));
	KeyState = EKeyState::Input;
}

/**
**  Begin input.
*/
static void UiBeginInput()
{
	KeyState = EKeyState::Input;
	ranges::fill(Input, '\0');
	InputIndex = 0;
	UI.StatusLine.ClearCosts();
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
	NetworkSendSelection((CUnit **)nullptr, 0);
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
static void
UiCenterOnGroup(unsigned group,
                EGroupSelectionMode mode = EGroupSelectionMode::SelectableByRectangleOnly)
{
	const std::vector<CUnit *> &units = GetUnitsOfGroup(group);
	PixelPos pos(-1, -1);

	// FIXME: what should we do with the removed units? ignore?
	for (CUnit *unit : units) {
		if (unit->Type && unit->Type->CanSelect(mode)) {
			if (pos.x != -1) {
				pos += (unit->GetMapPixelPosCenter() - pos) / 2;
			} else {
				pos = unit->GetMapPixelPosCenter();
			}
		}
	}
	if (pos.x != -1) {
		UI.SelectedViewport->Center(pos);
	}
}

/**
**  Select group.
**
**  @param group  Group number to select.
*/
static void UiSelectGroup(unsigned group, EGroupSelectionMode mode = EGroupSelectionMode::SelectAll)
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
	const std::vector<CUnit *> &units = GetUnitsOfGroup(group);

	if (units.empty()) {
		return;
	}

	//  Don't allow to mix units and buildings
	if (!Selected.empty() && Selected[0]->Type->Building) {
		return;
	}

	for (CUnit *unit : units) {
		if (!(unit->Removed || unit->Type->Building)) {
			SelectUnit(*unit);
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
	for (CUnit *unit : Selected) {
		if (unit->Player == ThisPlayer && unit->GroupId) {
			RemoveUnitFromGroups(*unit);
		}
	}
	SetGroup(&Selected[0], Selected.size(), group);
}

/**
**  Add to group. The current selected units are added to the group.
**
**  @param group  Group number to be expanded.
*/
static void UiAddToGroup(unsigned group)
{
	AddToGroup(&Selected[0], Selected.size(), group);
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
			CallbackMusicTrigger();
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
	CyclesPerSecond++;
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
	if (CyclesPerSecond > 1) {
		--CyclesPerSecond;
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
	CyclesPerSecond = CYCLES_PER_SECOND;
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
	if (Selected.empty()) {
		return;
	}

	PixelPos pos;

	for (const CUnit *unit : Selected) {
		pos += unit->GetMapPixelPosCenter();
	}
	pos /= Selected.size();
	UI.SelectedViewport->Center(pos);
}

/**
**  Save current map position.
**
**  @param position  Map position slot.
*/
static void UiSaveMapPosition(unsigned position)
{
	SavedMapPosition[position] = UI.SelectedViewport->MapPos;
}

/**
**  Recall map position.
**
**  @param position  Map position slot.
*/
static void UiRecallMapPosition(unsigned position)
{
	UI.SelectedViewport->Set(SavedMapPosition[position], PixelTileSize / 2);
}

/**
**  Toggle terrain display on/off.
*/
void UiToggleTerrain()
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
void UiFindIdleWorker()
{
	const auto &freeWorkers = ThisPlayer->GetFreeWorkers();
	if (freeWorkers.empty()) {
		return;
	}
	CUnit *unit = freeWorkers[0];
	if (LastIdleWorker) {
		auto it = ranges::find(freeWorkers, LastIdleWorker);
		if (it != freeWorkers.end() && std::next(it) != freeWorkers.end()) {
			unit = *std::next(it);
		}
	}

	if (unit != nullptr) {
		LastIdleWorker = unit;
		SelectSingleUnit(*unit);
		UI.StatusLine.Clear();
		UI.StatusLine.ClearCosts();
		CurrentButtonLevel = 0;
		PlayUnitSound(*Selected[0], EUnitVoice::Selected);
		SelectionChanged();
		UI.SelectedViewport->Center(unit->GetMapPixelPosCenter());
	}
}

/**
**  Toggle grab mouse on/off.
*/
static void UiToggleGrabMouse()
{
	DebugPrint("%x\n", KeyModifiers);
	ToggleGrabMouse(0);
	UI.StatusLine.Set(_("Grab mouse toggled."));
}

/**
**  Track unit, the viewport follows the unit.
*/
void UiTrackUnit()
{
	//Check if player has selected at least 1 unit
	if (Selected.empty()) {
		UI.SelectedViewport->Unit = nullptr;
		return;
	}
	if (UI.SelectedViewport->Unit == Selected[0]) {
		UI.SelectedViewport->Unit = nullptr;
	} else {
		UI.SelectedViewport->Unit = Selected[0];
	}
}

/**
**  Call the lua function HandleCommandKey
*/
bool HandleCommandKey(int key)
{
	int base = lua_gettop(Lua);

	lua_getglobal(Lua, "HandleCommandKey");
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
		bool ret = LuaToBoolean(Lua, base + 1);
		lua_pop(Lua, 1);
		return ret;
	} else {
		LuaError(Lua, "HandleCommandKey must return a boolean");
		return false;
	}
}

#ifdef DEBUG
extern void ToggleShowBuilListMessages();
#endif

static void CommandKey_Group(int group)
{
	if (KeyModifiers & ModifierShift) {
		if (KeyModifiers & (ModifierAlt | ModifierDoublePress)) {
			if (KeyModifiers & ModifierDoublePress) {
				UiCenterOnGroup(group, EGroupSelectionMode::SelectAll);
			} else {
				UiSelectGroup(group, EGroupSelectionMode::SelectAll);
			}
		} else if (KeyModifiers & ModifierControl) {
			UiAddToGroup(group);
		} else {
			UiAddGroupToSelection(group);
		}
	} else {
		if (KeyModifiers & (ModifierAlt | ModifierDoublePress)) {
			if (KeyModifiers & ModifierAlt) {
				if (KeyModifiers & ModifierDoublePress) {
					UiCenterOnGroup(group, EGroupSelectionMode::NonSelectableByRectangleOnly);
				} else {
					UiSelectGroup(group, EGroupSelectionMode::NonSelectableByRectangleOnly);
				}
			} else {
				UiCenterOnGroup(group);
			}
		} else if (KeyModifiers & ModifierControl) {
			UiDefineGroup(group);
		} else {
			UiSelectGroup(group);
		}
	}
}

static void CommandKey_MapPosition(int index)
{
	if (KeyModifiers & ModifierShift) {
		UiSaveMapPosition(index);
	} else {
		UiRecallMapPosition(index);
	}
}

/**
**  Handle keys in command mode.
**
**  @param key  Key scancode.
**
**  @return     True, if key is handled; otherwise false.
*/
static bool CommandKey(int key)
{
	const char *ptr = strchr(UiGroupKeys.c_str(), key);

	// FIXME: don't handle unicode well. Should work on all latin keyboard.
	if (ptr) {
		key = ((int)'0') + ptr - UiGroupKeys.c_str();
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
			// UiUnselectAll();
			break;

		// Group selection
		case '0': case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
		case '9':
			CommandKey_Group(key - '0');
			break;

		case SDLK_F2:
		case SDLK_F3:
		case SDLK_F4: // Set/Goto place
			CommandKey_MapPosition(key - SDLK_F2);
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
			[[fallthrough]];
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
			[[fallthrough]]; // (CTRL+P, ALT+P)
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
			else if (KeyModifiers & ModifierAlt) {
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
		case SDLK_KP_8:
			KeyScrollState |= ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP_2:
			KeyScrollState |= ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP_4:
			KeyScrollState |= ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP_6:
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

/**
**  Handle cheats
**
**  @return  true if a cheat was handled, false otherwise
*/
bool HandleCheats(const std::string &input)
{
#if defined(DEBUG) || defined(PROF)
	if (input == "ai me") {
		if (ThisPlayer->AiEnabled) {
			// FIXME: UnitGoesUnderFog and UnitGoesOutOfFog change unit refs
			// for human players.  We can't switch back to a human player or
			// we'll be using the wrong ref counts.
#if 0
			ThisPlayer->AiEnabled = false;
			ThisPlayer->Type = PlayerPerson;
			SetMessage("AI is off, Normal Player");
#else
			SetMessage("Cannot disable 'ai me' cheat");
#endif
		} else {
			ThisPlayer->AiEnabled = true;
			ThisPlayer->Type = PlayerTypes::PlayerComputer;
			if (!ThisPlayer->Ai) {
				AiInit(*ThisPlayer);
			}
			SetMessage("I'm the BORG, resistance is futile!");
		}
		return true;
	}
#endif
	int base = lua_gettop(Lua);
	lua_getglobal(Lua, "HandleCheats");
	if (!lua_isfunction(Lua, -1)) {
		DebugPrint("No HandleCheats function in lua.\n");
		return false;
	}
	lua_pushstring(Lua, input.c_str());
	LuaCall(1, 0, false);
	if (lua_gettop(Lua) - base == 1) {
		bool ret = LuaToBoolean(Lua, -1);
		lua_pop(Lua, 1);
		return ret;
	} else {
		DebugPrint("HandleCheats must return a boolean");
		return false;
	}
}

/**
**  Handle keys in input mode.
**
**  @param key  Key scancode.
*/
static void InputKey(int key)
{
	switch (key) {
		case SDLK_RETURN:
		case SDLK_KP_ENTER: { // RETURN
			// save to history
			InputHistory[InputHistoryIdx] = Input;
			if (InputHistorySize < InputHistory.size() - 1) {
				InputHistorySize++;
				InputHistoryIdx = InputHistorySize;
			} else {
				InputHistoryIdx = (InputHistoryIdx + 1 + InputHistorySize) % InputHistory.size();
			}
			InputHistoryPos = InputHistoryIdx;

			// Replace ~~ with ~
#ifdef DEBUG
			if (Input[0] == '-') {
				if (!GameObserve && !GamePaused && !GameEstablishing) {
					CommandLog("input", nullptr, EFlushMode::On, -1, -1, nullptr, Input.data(), -1);
					CclCommand(Input.data() + 1, false);
				}
			} else
#endif
				if (!IsNetworkGame()) {
					if (!GameObserve && !GameEstablishing) {
						if (HandleCheats(Input.data())) {
							CommandLog("input", nullptr, EFlushMode::On, -1, -1, nullptr, Input.data(), -1);
						}
					}
				}

			// Check for Replay and ffw x
#ifdef DEBUG
			if (starts_with(Input.data(), "ffw ")) {
#else
			if (starts_with(Input.data(), "ffw ") && ReplayGameType != EReplayType::NoReplay) {
#endif
				FastForwardCycle = atoi(&Input[4]);
			}

			if (Input[0]) {
				auto escapedInput = ReplaceTildeBy2Tilde(Input.data());
				std::string chatMessage = "~" + PlayerColorNames[ThisPlayer->Index] + "~<"
				                        + ThisPlayer->Name + ">~> " + escapedInput;
				NetworkSendChatMessage(chatMessage);
			}
		}
			[[fallthrough]];
		case SDLK_ESCAPE:
			KeyState = EKeyState::Command;
			UI.StatusLine.Clear();
			break;

#ifdef USE_MAC
		case SDLK_DELETE:
#endif
		case SDLK_BACKSPACE: {
			if (InputIndex) {
				InputHistoryPos = InputHistoryIdx;
				int prevIndex = UTF8GetPrev(Input.data(), InputIndex);
				if (prevIndex >= 0) {
					moveInputContent(prevIndex, InputIndex);
					InputIndex = prevIndex;
				}
				ShowInput();
			}
			break;
		}
		case SDLK_UP:
			InputHistory[InputHistoryPos] = Input;
			if (InputHistorySize == 0) {
				break;
			}
			InputHistoryPos = ((InputHistoryPos - 1) % InputHistorySize + InputHistorySize) % InputHistorySize;
			Input = InputHistory[InputHistoryPos];
			InputIndex = strlen(Input.data());
			ShowInput();
			break;

		case SDLK_DOWN:
			InputHistory[InputHistoryPos] = Input;
			if (InputHistorySize == 0) {
				break;
			}
			InputHistoryPos = ((InputHistoryPos + 1) % InputHistorySize + InputHistorySize) % InputHistorySize;
			Input = InputHistory[InputHistoryPos];
			InputIndex = strlen(Input.data());
			ShowInput();
			break;

		case SDLK_LEFT:
			if (InputIndex) {
				InputIndex = UTF8GetPrev(Input.data(), InputIndex);
				ShowInput();
			}
			break;

		case SDLK_RIGHT:
			InputIndex = std::min<int>(strlen(Input.data()), UTF8GetNext(Input.data(), InputIndex));
			ShowInput();
			break;

		case SDLK_TAB: {
			InputHistoryPos = InputHistoryIdx;
			char *namestart = strrchr(Input.data(), ' ');
			if (namestart) {
				++namestart;
			} else {
				namestart = Input.data();
			}
			if (strlen(namestart)) {
				for (int i = 0; i < PlayerMax; ++i) {
					if (Players[i].Type != PlayerTypes::PlayerPerson) {
						continue;
					}
					if (!strncasecmp(namestart, Players[i].Name.c_str(), strlen(namestart))) {
						InputIndex += Players[i].Name.size() - strlen(namestart);
						strcpy_s(namestart, sizeof(Input) - (namestart - Input.data()), Players[i].Name.c_str());
						if (namestart == Input.data()) {
							InputIndex += 2;
							strcat_s(namestart, Input.size() - (namestart - Input.data()), ": ");
						}
					}
				}
			}
			ShowInput();
			break;
		}
		default:
			if (key >= ' ') {
				InputHistoryPos = InputHistoryIdx;
				std::string kstr = to_utf8(key);
				if (InputIndex < (int)(Input.size() - kstr.size())) {
					moveInputContent(InputIndex + kstr.size(), InputIndex);
					for (char c : kstr) {
						Input[InputIndex++] = c;
					}
				}
				ShowInput();
			}
			break;
	}
}

/**
**  Save a screenshot.
*/
static void Screenshot()
{
	CFile fd;
	char filename[30];

	for (int i = 1; i <= 99; ++i) {
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
**  @return         true if modifier found, false otherwise
*/
bool HandleKeyModifiersDown(unsigned key, unsigned)
{
	switch (key) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			KeyModifiers |= ModifierShift;
			return true;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			KeyModifiers |= ModifierControl;
			return true;
		case SDLK_LALT:
		case SDLK_RALT:
			KeyModifiers |= ModifierAlt;
			// maxy: disabled
			if (InterfaceState == IfaceState::Normal) {
				SelectedUnitChanged(); // VLADI: to allow alt-buttons
			}
			return true;
		case SDLK_LGUI:
		case SDLK_RGUI:
			KeyModifiers |= ModifierSuper;
			return true;
		case SDLK_SYSREQ:
		case SDLK_PRINTSCREEN:
			Screenshot();
			if (GameRunning) {
				SetMessage("%s", _("Screenshot made."));
			}
			return true;
		default:
			break;
	}
	return false;
}

/**
**  Update KeyModifiers if a key is released.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
**
**  @return         true if modifier found, false otherwise
*/
bool HandleKeyModifiersUp(unsigned key, unsigned)
{
	switch (key) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			KeyModifiers &= ~ModifierShift;
			return true;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			KeyModifiers &= ~ModifierControl;
			return true;
		case SDLK_LALT:
		case SDLK_RALT:
			KeyModifiers &= ~ModifierAlt;
			// maxy: disabled
			if (InterfaceState == IfaceState::Normal) {
				SelectedUnitChanged(); // VLADI: to allow alt-buttons
			}
			return true;
		case SDLK_LGUI:
		case SDLK_RGUI:
			KeyModifiers &= ~ModifierSuper;
			return true;
	}
	return false;
}

/**
**  Get ascii from keypad key
*/
static std::optional<unsigned> getKeyPad(unsigned key)
{
	switch (key)
	{
		case SDLK_RIGHT: return key;
		case SDLK_LEFT: return key;
		case SDLK_DOWN: return key;
		case SDLK_UP: return key;
		case SDLK_KP_DIVIDE: return SDLK_SLASH;
		case SDLK_KP_MULTIPLY: return SDLK_ASTERISK;
		case SDLK_KP_MINUS: return SDLK_MINUS;
		case SDLK_KP_PLUS: return SDLK_PLUS;
		case SDLK_KP_ENTER: return SDLK_RETURN;
		case SDLK_KP_1: return SDLK_1;
		case SDLK_KP_2: return SDLK_2;
		case SDLK_KP_3: return SDLK_3;
		case SDLK_KP_4: return SDLK_4;
		case SDLK_KP_5: return SDLK_5;
		case SDLK_KP_6: return SDLK_6;
		case SDLK_KP_7: return SDLK_7;
		case SDLK_KP_8: return SDLK_8;
		case SDLK_KP_9: return SDLK_9;
		case SDLK_KP_0: return SDLK_0;
		case SDLK_KP_PERIOD: return SDLK_PERIOD;
		case SDLK_KP_EQUALS: return SDLK_EQUALS;
		default: return std::nullopt;
	}
}

/**
**  Handle key down.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
*/
void HandleKeyDown(unsigned key, unsigned keychar)
{
	if (IsDemoMode()) {
		// If we are in "demo mode", exit no matter which key we hit.
		void ActionDraw();
		ActionDraw();
		return;
	}

	if (HandleKeyModifiersDown(key, keychar)) {
		return;
	}

	// Handle All other keys

	// Command line input: for message or cheat
	if (KeyState == EKeyState::Input) {
		if (auto kp = getKeyPad(key).value_or(keychar)) {
			InputKey(kp);
			return;
		}
	}
	// If no modifier look if button bound
	if (!(KeyModifiers & (ModifierControl | ModifierAlt | ModifierSuper))) {
		if (!GameObserve && !GamePaused && !GameEstablishing) {
			if (UI.ButtonPanel.DoKey(key)) {
				return;
			}
		}
	}
	CommandKey(key);
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
		case SDLK_KP_8:
			KeyScrollState &= ~ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP_2:
			KeyScrollState &= ~ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP_4:
			KeyScrollState &= ~ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP_6:
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
	if (KeyState == EKeyState::Input && keychar) {
		InputKey(keychar);
	}
}

/**
**  Handle the mouse in scroll area
**
**  @param mousePos  Screen position.
**
**  @return   true if the mouse is in the scroll area, false otherwise
*/
bool HandleMouseScrollArea(const PixelPos &mousePos)
{
	if (mousePos.x < ScrollMarginLeft) {
		if (mousePos.y < ScrollMarginTop) {
			CursorOn = ECursorOn::ScrollLeftUp;
			MouseScrollState = ScrollLeftUp;
			GameCursor = UI.ArrowNW.Cursor;
		} else if (mousePos.y > (Video.Height - ScrollMarginBottom)) {
			CursorOn = ECursorOn::ScrollLeftDown;
			MouseScrollState = ScrollLeftDown;
			GameCursor = UI.ArrowSW.Cursor;
		} else {
			CursorOn = ECursorOn::ScrollLeft;
			MouseScrollState = ScrollLeft;
			GameCursor = UI.ArrowW.Cursor;
		}
	} else if (mousePos.x > (Video.Width - ScrollMarginRight)) {
		if (mousePos.y < ScrollMarginTop) {
			CursorOn = ECursorOn::ScrollRightUp;
			MouseScrollState = ScrollRightUp;
			GameCursor = UI.ArrowNE.Cursor;
		} else if (mousePos.y > (Video.Height - ScrollMarginBottom)) {
			CursorOn = ECursorOn::ScrollRightDown;
			MouseScrollState = ScrollRightDown;
			GameCursor = UI.ArrowSE.Cursor;
		} else {
			CursorOn = ECursorOn::ScrollRight;
			MouseScrollState = ScrollRight;
			GameCursor = UI.ArrowE.Cursor;
		}
	} else {
		if (mousePos.y < ScrollMarginTop) {
			CursorOn = ECursorOn::ScrollUp;
			MouseScrollState = ScrollUp;
			GameCursor = UI.ArrowN.Cursor;
		} else if (mousePos.y > (Video.Height - ScrollMarginBottom)) {
			CursorOn = ECursorOn::ScrollDown;
			MouseScrollState = ScrollDown;
			GameCursor = UI.ArrowS.Cursor;
		} else {
			return false;
		}
	}
	return true;
}

/**
**  Keep coordinates in window and update cursor position
**
**  @param x  screen pixel X position.
**  @param y  screen pixel Y position.
*/
void HandleCursorMove(int *x, int *y)
{
	//  Reduce coordinates to window-size.
	clamp(x, 0, Video.Width - 1);
	clamp(y, 0, Video.Height - 1);
	CursorScreenPos.x = *x;
	CursorScreenPos.y = *y;
}

/**
**  Handle movement of the cursor.
**
**  @param screePos  screen pixel position.
*/
void HandleMouseMove(const PixelPos &screenPos)
{
	PixelPos pos(screenPos);
	HandleCursorMove(&pos.x, &pos.y);
	UIHandleMouseMove(pos);
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

int DoubleClickDelay = 300;       /// Time to detect double clicks.
int HoldClickDelay = 1000;        /// Time to detect hold clicks.

static enum {
	InitialMouseState,            /// start state
	ClickedMouseState             /// button is clicked
} MouseState;                     /// Current state of mouse

static PixelPos LastMousePos;     /// Last mouse position
static unsigned LastMouseButton;  /// last mouse button handled
static unsigned StartMouseTicks;  /// Ticks of first click
static unsigned LastMouseTicks;   /// Ticks of last mouse event

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
void InputMouseButtonPress(const EventCallback &callbacks,
						   unsigned ticks, unsigned button)
{
	//  Button new pressed.
	if (!(MouseButtons & (1 << button))) {
		MouseButtons |= (1 << button);
		//  Detect double click
		if (MouseState == ClickedMouseState && button == LastMouseButton
			&& ticks < StartMouseTicks + DoubleClickDelay) {
			MouseButtons |= (1 << button) << MouseDoubleShift;
			button |= button << MouseDoubleShift;
		} else {
			MouseState = InitialMouseState;
			StartMouseTicks = ticks;
			LastMouseButton = button;
		}
	}
	LastMouseTicks = ticks;

	if (callbacks.ButtonPressed) {
		callbacks.ButtonPressed(button);
	}
}

/**
**  Called if any mouse button is released up
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param button     Mouse button released.
*/
void InputMouseButtonRelease(const EventCallback &callbacks,
							 unsigned ticks, unsigned button)
{
	//  Same button before pressed.
	if (button == LastMouseButton && MouseState == InitialMouseState) {
		MouseState = ClickedMouseState;
	} else {
		LastMouseButton = 0;
		MouseState = InitialMouseState;
	}
	LastMouseTicks = ticks;

	unsigned mask = 0;
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

	if (callbacks.ButtonReleased) {
		callbacks.ButtonReleased(button | mask);
	}
}

/**
**  Called if the mouse is moved
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
**  @param x          X movement
**  @param y          Y movement
*/
void InputMouseMove(const EventCallback &callbacks,
					unsigned ticks, int x, int y)
{
	PixelPos mousePos(x, y);
	// Don't reset the mouse state unless we really moved
	if (LastMousePos != mousePos) {
		MouseState = InitialMouseState;
		LastMouseTicks = ticks;
		LastMousePos = mousePos;
	}
	if (callbacks.MouseMoved) {
		callbacks.MouseMoved(mousePos);
	}
}

/**
**  Called if the mouse exits the game window (when supported by videomode)
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
*/
void InputMouseExit(const EventCallback &callbacks, unsigned /* ticks */)
{
	// FIXME: should we do anything here with ticks? don't know, but conform others
	// JOHNS: called by callback HandleMouseExit();
	if (callbacks.MouseExit) {
		callbacks.MouseExit();
	}
}

/**
**  Called each frame to handle mouse timeouts.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
*/
void InputMouseTimeout(const EventCallback &callbacks, unsigned ticks)
{
	if (MouseButtons & (1 << LastMouseButton)) {
		if (ticks > StartMouseTicks + DoubleClickDelay) {
			MouseState = InitialMouseState;
		}
		if (ticks > LastMouseTicks + HoldClickDelay) {
			LastMouseTicks = ticks;
			MouseButtons |= (1 << LastMouseButton) << MouseHoldShift;
			if (callbacks.ButtonPressed) {
				callbacks.ButtonPressed(LastMouseButton | (LastMouseButton << MouseHoldShift));
			}
		}
	}
}


static const int HoldKeyDelay = 250;          /// Time to detect hold key
static const int HoldKeyAdditionalDelay = 50; /// Time to detect additional hold key

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
void InputKeyButtonPress(const EventCallback &callbacks,
						 unsigned ticks, unsigned ikey, unsigned ikeychar)
{
	if (!LastIKey && DoubleKey == ikey && ticks < LastKeyTicks + DoubleClickDelay) {
		KeyModifiers |= ModifierDoublePress;
	}
	DoubleKey = ikey;
	LastIKey = ikey;
	LastIKeyChar = ikeychar;
	LastKeyTicks = ticks;
	if (callbacks.KeyPressed) {
		callbacks.KeyPressed(ikey, ikeychar);
	}
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
void InputKeyButtonRelease(const EventCallback &callbacks,
						   unsigned ticks, unsigned ikey, unsigned ikeychar)
{
	if (ikey == LastIKey) {
		LastIKey = 0;
	}
	LastKeyTicks = ticks;
	if (callbacks.KeyReleased) {
		callbacks.KeyReleased(ikey, ikeychar);
	}
}

/**
**  Called each frame to handle keyboard timeouts.
**
**  @param callbacks  Callback structure for events.
**  @param ticks      Denotes time-stamp of video-system
*/
void InputKeyTimeout(const EventCallback &callbacks, unsigned ticks)
{
	if (LastIKey && ticks > LastKeyTicks + HoldKeyDelay) {
		LastKeyTicks = ticks - (HoldKeyDelay - HoldKeyAdditionalDelay);
		if (callbacks.KeyRepeated) {
			callbacks.KeyRepeated(LastIKey, LastIKeyChar);
		}
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

/**
 * Configure margins (in pixels) on screen where map scrolling starts.
 */
void SetScrollMargins(unsigned int top, unsigned int right, unsigned int bottom, unsigned int left)
{
	ScrollMarginTop = top;
	ScrollMarginBottom = bottom + 1;
	ScrollMarginLeft = left;
	ScrollMarginRight = right + 1;
}

//@}
