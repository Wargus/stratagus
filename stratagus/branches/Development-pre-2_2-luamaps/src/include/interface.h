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
//      (c) Copyright 1998-2004 by Lutz Sammer
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

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#include "icons.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct _unit_;
struct _event_callback_;

/*----------------------------------------------------------------------------
--  Definitons
----------------------------------------------------------------------------*/

	/// Button Commands that need target selection
enum _button_cmd_ {
	ButtonMove,           ///< order move
	ButtonAttack,         ///< order attack
	ButtonRepair,         ///< order repair
	ButtonHarvest,        ///< order harvest
	ButtonBuild,          ///< order build
	ButtonPatrol,         ///< order patrol
	ButtonAttackGround,   ///< order attack ground
	ButtonSpellCast,      ///< order cast spell
	ButtonUnload,         ///< order unload unit
	ButtonStop,           ///< order stop
	ButtonButton,         ///< choose other button set
	ButtonTrain,          ///< order train
	ButtonStandGround,    ///< order stand ground
	ButtonReturn,         ///< order return goods
	ButtonResearch,       ///< order reseach
	ButtonUpgradeTo,      ///< order upgrade
	ButtonCancel,         ///< cancel
	ButtonCancelUpgrade,  ///< cancel upgrade
	ButtonCancelTrain,    ///< cancel training
	ButtonCancelBuild,    ///< cancel building
};

	/// typedef for action of button
typedef struct _button_action_ ButtonAction;
typedef int (*ButtonCheckFunc)(const struct _unit_*, const ButtonAction*);

	/// Action of button
struct _button_action_ {
	int Pos;    ///< button position in the grid
	int Level;  ///< requires button level
	enum _button_cmd_ Action; ///< command on button press
	int Value;  ///< extra value for command
	char* ValueStr;  ///< keep original value string

	ButtonCheckFunc Allowed; ///< Check if this button is allowed
	char*      AllowStr;  ///< argument for allowed
	char*      UnitMask;  ///< for which units is it available
	IconConfig Icon;      ///< icon to display
	int        Key;       ///< alternative on keyboard
	char*      Hint;      ///< tip text
};

	/// Button area under cursor
enum _button_area_ {
	ButtonAreaSelected,      ///< Selected button
	ButtonAreaTraining,      ///< Training button
	ButtonAreaUpgrading,     ///< Upgrading button
	ButtonAreaResearching,   ///< Researching button
	ButtonAreaTransporting,  ///< Transporting button
	ButtonAreaButton,        ///< Button panel button
	ButtonAreaMenu,          ///< Menu button
};

	/// Menu button under cursor
enum _menu_button_under_ {
	ButtonUnderMenu,              ///< Menu button
	ButtonUnderNetworkMenu,       ///< Network menu button
	ButtonUnderNetworkDiplomacy,  ///< Diplomacy button
};

	/// current interface state
enum _iface_state_ {
	IfaceStateNormal,  ///< Normal Game state
	IfaceStateMenu,    ///< Menu active
};

	/// current key state
enum _key_state_ {
	KeyStateCommand = 0,  ///< keys -> commands
	KeyStateInput         ///< keys -> line editor
};                        ///< current keyboard state

	/// additional keycodes
enum _key_codes_ {
	KeyCodeUp = 0x101,  ///< internal keycode: cursor up key
	KeyCodeDown,      ///< internal keycode: cursor down key
	KeyCodeLeft,      ///< internal keycode: cursor left key
	KeyCodeRight,     ///< internal keycode: cursor right key
	KeyCodePause,     ///< internal keycode: game pause key

	KeyCodeF1,        ///< internal keycode: F1 function keys
	KeyCodeF2,        ///< internal keycode: F2 function keys
	KeyCodeF3,        ///< internal keycode: F3 function keys
	KeyCodeF4,        ///< internal keycode: F4 function keys
	KeyCodeF5,        ///< internal keycode: F5 function keys
	KeyCodeF6,        ///< internal keycode: F6 function keys
	KeyCodeF7,        ///< internal keycode: F7 function keys
	KeyCodeF8,        ///< internal keycode: F8 function keys
	KeyCodeF9,        ///< internal keycode: F9 function keys
	KeyCodeF10,       ///< internal keycode: F10 function keys
	KeyCodeF11,       ///< internal keycode: F11 function keys
	KeyCodeF12,       ///< internal keycode: F12 function keys

	KeyCodeKP0,       ///< internal keycode: keypad 0
	KeyCodeKP1,       ///< internal keycode: keypad 1
	KeyCodeKP2,       ///< internal keycode: keypad 2
	KeyCodeKP3,       ///< internal keycode: keypad 3
	KeyCodeKP4,       ///< internal keycode: keypad 4
	KeyCodeKP5,       ///< internal keycode: keypad 5
	KeyCodeKP6,       ///< internal keycode: keypad 6
	KeyCodeKP7,       ///< internal keycode: keypad 7
	KeyCodeKP8,       ///< internal keycode: keypad 8
	KeyCodeKP9,       ///< internal keycode: keypad 9
	KeyCodeKPPlus,    ///< internal keycode: keypad +
	KeyCodeKPMinus,   ///< internal keycode: keypad -
	KeyCodeKPPeriod,  ///< internal keycode: keypad .

	KeyCodeShift,     ///< internal keycode: shift modifier
	KeyCodeControl,   ///< internal keycode: ctrl modifier
	KeyCodeAlt,       ///< internal keycode: alt modifier
	KeyCodeSuper,     ///< internal keycode: super modifier
	KeyCodeHyper,     ///< internal keycode: hyper modifier

	KeyCodePrint,     ///< internal keycode: print screen
	KeyCodeDelete,    ///< internal keycode: delete
};

	/// Key modifier
enum _key_modifiers_ {
	ModifierShift = 1,    ///< any shift key pressed
	ModifierControl = 2,  ///< any controll key pressed
	ModifierAlt = 4,      ///< any alt key pressed
	ModifierSuper = 8,    ///< super key (reserved for WM)
	ModifierHyper = 16,   ///< any hyper key pressed
};

#define MouseDoubleShift 8   ///< shift for double click button
#define MouseDragShift   16  ///< shift for drag button
#define MouseHoldShift   24  ///< shift for hold button

	/// pressed mouse button flags
enum _mouse_buttons_ {
	NoButton = 0,      ///< No button
	LeftButton = 2,    ///< Left button on mouse
	MiddleButton = 4,  ///< Middle button on mouse
	RightButton = 8,   ///< Right button on mouse

	UpButton = 16,    ///< Scroll up button on mouse
	DownButton = 32,  ///< Scroll down button on mouse

	LeftAndMiddleButton = LeftButton | MiddleButton,  ///< Left + Middle button on mouse
	LeftAndRightButton = LeftButton | RightButton,    ///< Left + Right button on mouse
	MiddleAndRightButton= MiddleButton | RightButton, ///< Middle + Right button on mouse
};

	/// Where is our cursor ?
enum _cursor_on_ {
	CursorOnUnknown = -1,     ///< not known
	CursorOnMinimap,          ///< minimap area
	CursorOnButton,           ///< button area see: ButtonUnderCursor
	CursorOnMap,              ///< over map area
	CursorOnScrollUp,         ///< in scroll up area
	CursorOnScrollDown,       ///< in scroll down area
	CursorOnScrollLeft,       ///< in scroll left area
	CursorOnScrollRight,      ///< in scroll right area
	CursorOnScrollLeftUp,     ///< in scroll left+up area
	CursorOnScrollLeftDown,   ///< in scroll left+down area
	CursorOnScrollRightUp,    ///< in scroll right+up area
	CursorOnScrollRightDown,  ///< in scroll right+down area
};

	/// Are We Scrolling With the Keyboard ?
enum _scroll_state_ {
	ScrollNone = 0,        ///< not scrolling
	ScrollUp = 1,          ///< scroll up only
	ScrollDown = 2,        ///< scroll down only
	ScrollLeft = 4,        ///< scroll left only
	ScrollRight = 8,       ///< scroll right only
	ScrollLeftUp = 5,      ///< scroll left + up
	ScrollLeftDown = 6,    ///< scroll left + down
	ScrollRightUp = 9,     ///< scroll right + up
	ScrollRightDown = 10,  ///< scroll right + down
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

	/// Flag telling if the game is running
extern char GameRunning;
	/// Flag telling if the game is paused
extern char GamePaused;
	/// Flag telling if the game is in observe mode
extern char GameObserve;
	/// Flag telling not to advance to the next game cycle
extern char SkipGameCycle;
	/// Invincibility cheat
extern int GodMode;
	/// Whether the map is the only thing displayed or not
extern char BigMapMode;

	/// pressed mouse buttons (normal,double,dragged,long)
extern enum _mouse_buttons_ MouseButtons;
	/// current active modifiers
extern enum _key_modifiers_ KeyModifiers;
	/// current interface state
extern enum _iface_state_ InterfaceState;
	/// current scroll state of keyboard
extern enum _scroll_state_ KeyScrollState;
	/// current scroll state of mouse
extern enum _scroll_state_ MouseScrollState;
	/// current key state
extern enum _key_state_ KeyState;
	/// pointer to unit under the cursor
extern struct _unit_* UnitUnderCursor;
	/// button area under the cursor
extern int ButtonAreaUnderCursor;
	/// button number under the cursor
extern int ButtonUnderCursor;
	/// menu button was clicked down
extern char GameMenuButtonClicked;
	/// diplomacy button was clicked down
extern char GameDiplomacyButtonClicked;
	/// Mouse leaves windows stops scroll
extern char LeaveStops;
	/// current CursorOn field
extern enum _cursor_on_ CursorOn;

	/// vladi: used for unit buttons sub-menus etc
extern int CurrentButtonLevel;

	/// Display the command key in the buttons
extern char ShowCommandKey;

	/// Time to detect double clicks
extern int DoubleClickDelay;
	/// Time to detect hold clicks
extern int HoldClickDelay;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

//
// in botpanel.c
//
	/// Generate all buttons
extern void InitButtons(void);
	/// Free memory for buttons
extern void CleanButtons(void);
	/// Make a new button
extern int AddButton(int pos, int level, const char* IconIdent,
	enum _button_cmd_ action, const char* value, const ButtonCheckFunc func,
	const void* arg, int key, const char* hint, const char* umask);

//
// in mouse.c
//
	/// Called if any mouse button is pressed down
extern void HandleButtonDown(unsigned button);
	/// Called if any mouse button is released up
extern void HandleButtonUp(unsigned button);
	/// Keep coordinates in window and update cursor position
extern void HandleCursorMove(int* x, int* y);
	/// Called if the mouse is moved
extern void HandleMouseMove(int x, int y);
	/// Called if the mouse exits the game window (only for some videomodes)
extern void HandleMouseExit(void);

	/// Update KeyModifiers if a key is pressed
extern int HandleKeyModifiersDown(unsigned keycode, unsigned keychar);
	/// Update KeyModifiers if a key is released
extern int HandleKeyModifiersUp(unsigned keycode, unsigned keychar);

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
extern void InputMouseButtonPress(const struct _event_callback_* callbacks,
	unsigned ticks, unsigned button);
	/// Called if any mouse button is released up
extern void InputMouseButtonRelease(const struct _event_callback_* callbacks,
	unsigned ticks, unsigned button);
	/// Called if the mouse is moved
extern void InputMouseMove(const struct _event_callback_* callbacks,
	unsigned ticks, int x, int y);
	/// Called if the mouse exits the game window (when supported by videomode)
extern void InputMouseExit(const struct _event_callback_* callbacks,
	unsigned ticks);
	/// Called to look for mouse timeouts
extern void InputMouseTimeout(const struct _event_callback_* callbacks,
	unsigned ticks);

	/// Called if any key button is pressed down
extern void InputKeyButtonPress(const struct _event_callback_* callbacks,
	unsigned ticks, unsigned ikey, unsigned ikeychar);
	/// Called if any key button is released up
extern void InputKeyButtonRelease(const struct _event_callback_* callbacks,
	unsigned ticks, unsigned ikey, unsigned ikeychar);
	/// Called to look for key timeouts
extern void InputKeyTimeout(const struct _event_callback_* callbacks,
	unsigned ticks);

	/// Toggle pause mode
extern void UiTogglePause(void);
	/// Handle cheats
extern int HandleCheats(const char* input);

//
// Chaos pur.
//
	/// Called if right mouse button is pressed
extern void DoRightButton(int tx, int ty);
	/// Cancel the building input mode
extern void CancelBuildingMode(void);

	/// Draw menu button area
extern void DrawMenuButtonArea(void);
	/// Update messages
extern void UpdateMessages(void);
	/// Draw messages as overlay over of the map
extern void DrawMessages(void);
	/// Draw the player resource in resource line
extern void DrawResources(void);
	/// Set message to display
extern void SetMessage(const char* fmt, ...);
	/// Set message to display with event point
extern void SetMessageEvent(int x, int y, const char* fmt, ...);
	/// Center view-point on last event message
extern void CenterOnMessage(void);
	/// Cleanup all messages
extern void CleanMessages(void);

	/// Set status line to show this information
extern void SetStatusLine(char* status);
	/// Clear the content of the message line
extern void ClearStatusLine(void);
	/// Draw status line
extern void DrawStatusLine(void);
	/// Draw costs in status line
extern void DrawCosts(void);
	/// Set costs to be displayed in status line
extern void SetCosts(int mana, int food, const int* costs);
	/// Clear the costs displayed in status line (undisplay!)
extern void ClearCosts(void);

	/// Draw the unit info panel
extern void DrawInfoPanel(void);
	/// Draw the timer
extern void DrawTimer(void);
	/// Update the timer
extern void UpdateTimer(void);
	/// Draw the unit button panel
extern void DrawButtonPanel(void);
	/// Update the status line with hints from the button
extern void UpdateStatusLineForButton(const ButtonAction* button);
	/// Draw the Pie Menu
extern void DrawPieMenu(void);
	/// Update the content of the unit button panel
extern void UpdateButtonPanel(void);
	/// Handle button click in button panel area
extern void DoButtonButtonClicked(int button);
	/// Lookup key for bottom panel buttons
extern int DoButtonPanelKey(int key);

	/// Handle the mouse in scroll area
extern int HandleMouseScrollArea(int x, int y);

//
// in button_checks.c
//
	/// Check is always true
extern int ButtonCheckTrue(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check is always false
extern int ButtonCheckFalse(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check if allowed upgrade is ready
extern int ButtonCheckUpgrade(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check if allowed units exists
extern int ButtonCheckUnitsOr(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check if allowed units exists
extern int ButtonCheckUnitsAnd(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check if have network play
extern int ButtonCheckNetwork(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check if don't have network play
extern int ButtonCheckNoNetwork(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check if unit isn't working (train,upgrade,research)
extern int ButtonCheckNoWork(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check if unit isn't researching or upgrading
extern int ButtonCheckNoResearch(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check if all requirements for an attack to are meet
extern int ButtonCheckAttack(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check if all requirements for an upgrade to are meet
extern int ButtonCheckUpgradeTo(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check if all requirements for a research are meet
extern int ButtonCheckResearch(const struct _unit_* unit,
	const ButtonAction* button);
	/// Check if all requirements for a single research are meet
extern int ButtonCheckSingleResearch(const struct _unit_* unit,
	const ButtonAction* button);

//
// in ccl_ui.c
//
	/// Called whenever the units selection is altered
extern void SelectionChanged(void);
	/// Called whenever the selected unit was updated
extern void SelectedUnitChanged(void);

//@}

#endif // !__INTERFACE_H__
