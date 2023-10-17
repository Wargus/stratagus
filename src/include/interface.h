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
/**@name interface.h - The user interface header file. */
//
//      (c) Copyright 1998-2015 by Lutz Sammer, Jimmy Salmon and Andrettin
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

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/
#ifndef __ICONS_H__
#include "icons.h"
#endif

#include "unitsound.h"
#include "vec2i.h"
#include <optional>
#include <vector>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUIButton;
class CUnit;
struct EventCallback;

/*----------------------------------------------------------------------------
--  Definitons
----------------------------------------------------------------------------*/

/// Button Commands that need target selection
enum class ButtonCmd {
	Move,           /// order move
	Attack,         /// order attack
	Repair,         /// order repair
	Harvest,        /// order harvest
	Build,          /// order build
	Patrol,         /// order patrol
	Explore,        /// order explore
	AttackGround,   /// order attack ground
	SpellCast,      /// order cast spell
	Unload,         /// order unload unit
	Stop,           /// order stop
	Button,         /// choose other button set
	Train,          /// order train
	StandGround,    /// order stand ground
	Return,         /// order return goods
	Research,       /// order reseach
	UpgradeTo,      /// order upgrade
	Cancel,         /// cancel
	CancelUpgrade,  /// cancel upgrade
	CancelTrain,    /// cancel training
	CancelBuild,    /// cancel building
	CallbackAction
};

class ButtonAction;
using ButtonCheckFunc = bool (*)(const CUnit &, const ButtonAction &);

/// Action of button
class ButtonAction
{
public:
	ButtonAction() = default;

	int Pos = 0;          /// button position in the grid
	int Level = 0;        /// requires button level
	bool AlwaysShow = false;  /// button is always shown but drawn grayscale if not available
	ButtonCmd Action = ButtonCmd::Move; /// command on button press
	int Value = 0;        /// extra value for command
	void* Payload = nullptr;
	std::string ValueStr;    /// keep original value string

	ButtonCheckFunc Allowed = nullptr;    /// Check if this button is allowed
	std::string AllowStr;       /// argument for allowed
	std::string UnitMask;       /// for which units is it available
	IconConfig Icon;      /// icon to display
	int Key = 0;                    /// alternative on keyboard
	std::string Hint;           /// tip texts
	std::string Description;    /// description shown on status bar (optional)
	SoundConfig CommentSound;   /// Sound comment used when you press the button
	std::string ButtonCursor;   /// Custom cursor for button action (for example, to set spell target)
	std::string Popup;          /// Popup screen used for button
};

/// Button area under cursor
enum class ButtonArea
{
	Selected,      /// Selected button
	Training,      /// Training button
	Upgrading,     /// Upgrading button
	Researching,   /// Researching button
	Transporting,  /// Transporting button
	Button,        /// Button panel button
	Menu,          /// Menu button
	User           /// User buttons
};

/// Menu button under cursor
enum _menu_button_under_ {
	ButtonUnderMenu,              /// Menu button
	ButtonUnderNetworkMenu,       /// Network menu button
	ButtonUnderNetworkDiplomacy,  /// Diplomacy button
	ButtonUnderFreeWorkers        /// Free workers icon
};

/// current interface state
enum class IfaceState {
	Normal,  /// Normal Game state
	Menu     /// Menu active
};

/// current keyboard state
enum class EKeyState {
	Command = 0,  /// keys -> commands
	Input         /// keys -> line editor
};

/// Key modifier
constexpr unsigned int ModifierShift = 1;        /// any shift key pressed
constexpr unsigned int ModifierControl = 2;      /// any control key pressed
constexpr unsigned int ModifierAlt = 4;          /// any alt key pressed
constexpr unsigned int ModifierSuper = 8;        /// super key (reserved for WM)
constexpr unsigned int ModifierDoublePress = 16; /// key double pressed

constexpr unsigned int MouseDoubleShift = 8 ; /// shift for double click button
constexpr unsigned int MouseDragShift = 16;   /// shift for drag button
constexpr unsigned int MouseHoldShift = 24;   /// shift for hold button

/// pressed mouse button flags
constexpr unsigned int NoButton = 0;      /// No button
constexpr unsigned int LeftButton = 2;    /// Left button on mouse
constexpr unsigned int MiddleButton = 4;  /// Middle button on mouse
constexpr unsigned int RightButton = 8; /// Right button on mouse

constexpr unsigned int UpButton = 16; /// Scroll up button on mouse
constexpr unsigned int DownButton = 32; /// Scroll down button on mouse

constexpr unsigned int LeftAndMiddleButton =
	(LeftButton | MiddleButton); /// Left + Middle button on mouse
constexpr unsigned int LeftAndRightButton =
	(LeftButton | RightButton); /// Left + Right button on mouse
constexpr unsigned int MiddleAndRightButton =
	(MiddleButton | RightButton); /// Middle + Right button on mouse

/// Where is our cursor ?
enum class ECursorOn {
	Unknown = -1,     /// not known
	Minimap,          /// minimap area
	Button,           /// button area see: ButtonUnderCursor
	Map,              /// over map area
	ScrollUp,         /// in scroll up area
	ScrollDown,       /// in scroll down area
	ScrollLeft,       /// in scroll left area
	ScrollRight,      /// in scroll right area
	ScrollLeftUp,     /// in scroll left+up area
	ScrollLeftDown,   /// in scroll left+down area
	ScrollRightUp,    /// in scroll right+up area
	ScrollRightDown   /// in scroll right+down area
};

/// Are We Scrolling With the Keyboard ?
constexpr unsigned int ScrollNone = 0;        /// not scrolling
constexpr unsigned int ScrollUp = 1;          /// scroll up only
constexpr unsigned int ScrollDown = 2;        /// scroll down only
constexpr unsigned int ScrollLeft = 4;        /// scroll left only
constexpr unsigned int ScrollRight = 8;       /// scroll right only
constexpr unsigned int ScrollLeftUp = ScrollUp | ScrollLeft;       /// scroll left + up
constexpr unsigned int ScrollLeftDown = ScrollDown | ScrollLeft;   /// scroll left + down
constexpr unsigned int ScrollRightUp = ScrollUp | ScrollRight;     /// scroll right + up
constexpr unsigned int ScrollRightDown = ScrollRight | ScrollLeft; /// scroll right + down

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/
extern std::vector<std::unique_ptr<ButtonAction>> UnitButtonTable;

/// Flag telling if the game is running
extern bool GameRunning;
/// Flag telling if the game is paused
extern bool GamePaused;
/// Flag telling if the game is in observe mode
extern bool GameObserve;
/// Flag telling if the game is in establishing mode
extern bool GameEstablishing;
/// Counter of how many game cycles to skip for each rendered frame
extern double SkipGameCycle;
/// Invincibility cheat
extern bool GodMode;
/// Whether the map is the only thing displayed or not
extern char BigMapMode;
/// Flag telling if the SDL window is visible
extern bool IsSDLWindowVisible;

/// pressed mouse buttons (normal,double,dragged,long)
extern int MouseButtons;
/// current active modifiers
extern int KeyModifiers;
/// current interface state
extern IfaceState InterfaceState;
/// current scroll state of keyboard
extern int KeyScrollState;
/// current scroll state of mouse
extern int MouseScrollState;
/// current key state
extern EKeyState KeyState;
/// shared pointer to unit under the cursor
extern CUnit *UnitUnderCursor;
/// button area under the cursor
extern std::optional<ButtonArea> ButtonAreaUnderCursor;
/// button number under the cursor
extern int ButtonUnderCursor;
/// oldbutton number under the cursor
extern int OldButtonUnderCursor;
/// menu button was clicked down
extern bool GameMenuButtonClicked;
/// diplomacy button was clicked down
extern bool GameDiplomacyButtonClicked;
/// Mouse leaves windows stops scroll
extern bool LeaveStops;
/// current CursorOn field
extern ECursorOn CursorOn;

/// vladi: used for unit buttons sub-menus etc
extern int CurrentButtonLevel;
/// Last drawn popup : used to speed up drawing
extern ButtonAction *LastDrawnButtonPopup;

/// Time to detect double clicks
extern int DoubleClickDelay;
/// Time to detect hold clicks
extern int HoldClickDelay;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern CUnit *GetUnitUnderCursor();

//
// in botpanel.cpp
//
/// Generate all buttons
extern void InitButtons();
/// Free memory for buttons
extern void CleanButtons();
/// Make a new button
extern void AddButton(int pos, int level, const std::string &IconIdent,
					  ButtonCmd action, const std::string &value, void* payload, const ButtonCheckFunc func,
					  const std::string &arg, const int key, const std::string &hint, const std::string &descr,
					  const std::string &sound, const std::string &cursor, const std::string &umask,
					  const std::string &popup, bool alwaysShow);
// Check if the button is allowed for the unit.
extern bool IsButtonAllowed(const CUnit &unit, const ButtonAction &buttonaction);

//
// in mouse.cpp
//
/// Called if any mouse button is pressed down
extern void HandleButtonDown(unsigned button);
/// Called if any mouse button is released up
extern void HandleButtonUp(unsigned button);
/// Keep coordinates in window and update cursor position
extern void HandleCursorMove(int *x, int *y);
/// Called if the mouse is moved
extern void HandleMouseMove(const PixelPos &pos);
/// Called if the mouse exits the game window (only for some videomodes)
extern void HandleMouseExit();

/// Update KeyModifiers if a key is pressed
extern bool HandleKeyModifiersDown(unsigned keycode, unsigned keychar);
/// Update KeyModifiers if a key is released
extern bool HandleKeyModifiersUp(unsigned keycode, unsigned keychar);

/// Called if a key is pressed
extern void HandleKeyDown(unsigned keycode, unsigned keychar);
/// Called when a key is released
extern void HandleKeyUp(unsigned keycode, unsigned keychar);
/// Called when a key is repeated
extern void HandleKeyRepeat(unsigned keycode, unsigned keychar);

//
// in interface.c (for link between video and mouse.c)
//
/// Called if any mouse button is pressed down
extern void InputMouseButtonPress(const EventCallback &callbacks, unsigned ticks, unsigned button);
/// Called if any mouse button is released up
extern void InputMouseButtonRelease(const EventCallback &callbacks, unsigned ticks, unsigned button);
/// Called if the mouse is moved
extern void InputMouseMove(const EventCallback &callbacks, unsigned ticks, int x, int y);
/// Called if the mouse exits the game window (when supported by videomode)
extern void InputMouseExit(const EventCallback &callbacks, unsigned ticks);
/// Called to look for mouse timeouts
extern void InputMouseTimeout(const EventCallback &callbacks, unsigned ticks);

/// Called if any key button is pressed down
extern void InputKeyButtonPress(const EventCallback &callbacks, unsigned ticks, unsigned ikey, unsigned ikeychar);
/// Called if any key button is released up
extern void InputKeyButtonRelease(const EventCallback &callbacks, unsigned ticks, unsigned ikey, unsigned ikeychar);
/// Called to look for key timeouts
extern void InputKeyTimeout(const EventCallback &callbacks, unsigned ticks);

/// Get double click delay
extern int GetDoubleClickDelay();
/// Set double click delay
extern void SetDoubleClickDelay(int delay);
/// Get hold click delay
extern int GetHoldClickDelay();
/// Set hold click delay
extern void SetHoldClickDelay(int delay);

/// Toggle pause mode
extern void UiTogglePause();
/// Toggle big map
extern void UiToggleBigMap();
/// Toggle terrain display on/off.
extern void UiToggleTerrain();
/// Find the next idle worker
extern void UiFindIdleWorker();
/// Track unit, the viewport follows the unit.
extern void UiTrackUnit();

/// Scroll margins on screen
extern void SetScrollMargins(unsigned int top, unsigned int right, unsigned int bottom, unsigned int left);

/// Handle cheats
extern bool HandleCheats(const std::string &input);

/// Call the lua function HandleCommandKey
bool HandleCommandKey(int key);

//
// Chaos pur.
//
/// Cancel the building input mode
extern void CancelBuildingMode();

/// Draw menu button area
extern void DrawMenuButtonArea();
/// Draw user defined buttons
extern void DrawUserDefinedButtons();
/// Update messages
extern void UpdateMessages();
/// Draw messages as overlay over of the map
extern void DrawMessages();
/// Draw the player resource in resource line
extern void DrawResources();
/// Set message to display
extern void SetMessage(const char *fmt, ...) PRINTF_VAARG_ATTRIBUTE(1, 2);
/// Set message to display with event point
extern void SetMessageEvent(const Vec2i &pos, const char *fmt, ...) PRINTF_VAARG_ATTRIBUTE(2, 3);
/// Center view-point on last event message
extern void CenterOnMessage();
/// Cleanup all messages
extern void CleanMessages();
/// show/hide messages
extern void ToggleShowMessages();
/// max message count
extern void SetMaxMessageCount(int newMax);

/// Draw the timer
extern void DrawTimer();
/// Update the timer
extern void UpdateTimer();
/// Update the status line with hints from the button
extern void UpdateStatusLineForButton(const ButtonAction &button);
/// Draw the Pie Menu
extern void DrawPieMenu();
/// Draw the button popup
extern void DrawPopup(const ButtonAction &button, const CUIButton &uibutton, int x = 0, int y = 0);

/// Handle the mouse in scroll area
extern bool HandleMouseScrollArea(const PixelPos &mousePos);

//
// in button_checks.cpp
//
/// Check is always true
extern bool ButtonCheckTrue(const CUnit &unit, const ButtonAction &button);
/// Check is always false
extern bool ButtonCheckFalse(const CUnit &unit, const ButtonAction &button);
/// Check if allowed upgrade is ready
extern bool ButtonCheckUpgrade(const CUnit &unit, const ButtonAction &button);
/// Check if unit has an individual upgrade
extern bool ButtonCheckIndividualUpgrade(const CUnit &unit, const ButtonAction &button);
/// Check if unit's variables pass the condition check
extern bool ButtonCheckUnitVariable(const CUnit &unit, const ButtonAction &button);
/// Check if allowed units exists
extern bool ButtonCheckUnitsOr(const CUnit &unit, const ButtonAction &button);
/// Check if allowed units exists
extern bool ButtonCheckUnitsAnd(const CUnit &unit, const ButtonAction &button);
/// Check if not one unit exist
extern bool ButtonCheckUnitsNot(const CUnit &unit, const ButtonAction &button);
/// Check if none of the units exist
extern bool ButtonCheckUnitsNor(const CUnit &unit, const ButtonAction &button);
/// Check if have network play
extern bool ButtonCheckNetwork(const CUnit &unit, const ButtonAction &button);
/// Check if don't have network play
extern bool ButtonCheckNoNetwork(const CUnit &unit, const ButtonAction &button);
/// Check if unit isn't working (train,upgrade,research)
extern bool ButtonCheckNoWork(const CUnit &unit, const ButtonAction &button);
/// Check if unit isn't researching or upgrading
extern bool ButtonCheckNoResearch(const CUnit &unit, const ButtonAction &button);
/// Check if all requirements for an attack to are meet
extern bool ButtonCheckAttack(const CUnit &unit, const ButtonAction &button);
/// Check if all requirements for an upgrade to are meet
extern bool ButtonCheckUpgradeTo(const CUnit &unit, const ButtonAction &button);
/// Check if all requirements for a research are meet
extern bool ButtonCheckResearch(const CUnit &unit, const ButtonAction &button);
/// Check if all requirements for a single research are meet
extern bool ButtonCheckSingleResearch(const CUnit &unit, const ButtonAction &button);
/// Check for button enabled, if requested condition passes check. Used for debug purposes
extern bool ButtonCheckDebug(const CUnit &unit, const ButtonAction &button);

//
// in ccl_ui.c
//
/// Called whenever the units selection is altered
extern void SelectionChanged();
/// Called whenever the selected unit was updated
extern void SelectedUnitChanged();

//
// in game.cpp
//
/// Set the game paused or unpaused
extern void SetGamePaused(bool paused);
/// Get the game paused or unpaused
extern bool GetGamePaused();
/// Set the game speed
extern void SetGameSpeed(int speed);
/// Get the game speed
extern int GetGameSpeed();

//@}

#endif // !__INTERFACE_H__
