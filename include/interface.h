//   ___________		     _________		      _____  __
//   \_	  _____/______	 ____	____ \_	  ___ \____________ _/ ____\/  |_
//    |	   __) \_  __ \_/ __ \_/ __ \/	  \  \/\_  __ \__  \\	__\\   __\ 
//    |	    \	|  | \/\  ___/\	 ___/\	   \____|  | \// __ \|	|   |  |
//    \___  /	|__|	\___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________			     ______________________
//			  T H E	  W A R	  B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name interface.h	-	The user interface header file. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "player.h"
#include "unit.h"
#include "icons.h"

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

    /// Button Commands
enum _button_cmd_ {
    ButtonMove,				/// order move
    ButtonStop,				/// order stop
    ButtonAttack,			/// order attack
    ButtonRepair,			/// order repair
    ButtonHarvest,			/// order harvest
    ButtonButton,			/// choose other button set
    ButtonBuild,			/// order build
    ButtonTrain,			/// order train
    ButtonPatrol,			/// order patrol
    ButtonStandGround,			/// order stand ground
    ButtonAttackGround,			/// order attack ground
    ButtonReturn,			/// order return goods
    ButtonDemolish,			/// order demolish/explode
    ButtonSpellCast,			/// order cast spell
    ButtonResearch,			/// order reseach
    ButtonUpgradeTo,			/// order upgrade
    ButtonUnload,			/// order unload unit
    ButtonCancel,			/// cancel
    ButtonCancelUpgrade,		/// cancel upgrade
    ButtonCancelTrain,			/// cancel training
    ButtonCancelBuild,			/// cancel building
};

    /// typedef for action of button
typedef struct _button_action_ ButtonAction;

    /// Action of button
struct _button_action_ {
    int		Pos;			/// button position in the grid
    int		Level;			/// requires button level
    IconConfig	Icon;			/// icon to display
    enum _button_cmd_ Action;		/// command on button press
    int		Value;			/// extra value for command
    char*	ValueStr;		/// keep original value string
	/// Check if this button is allowed
    int		(*Allowed)(const Unit* unit,const ButtonAction* button);
    char*	AllowStr;		/// argument for allowed
    int		Key;			/// alternative on keyboard
    char*	Hint;			/// tip text
    char*	UnitMask;		/// for which units is it available
};

    /// current interface state
enum _iface_state_ {
    IfaceStateNormal,			/// Normal Game state
    IfaceStateMenu,			/// Menu active
};

    /// additional keycodes
enum _key_codes_ {
    KeyCodeUp=0x101,			/// internal keycode: cursor up key
    KeyCodeDown,			/// internal keycode: cursor down key
    KeyCodeLeft,			/// internal keycode: cursor left key
    KeyCodeRight,			/// internal keycode: cursor right key
    KeyCodePause,			/// internal keycode: game pause key

    KeyCodeF1,				/// internal keycode: F1 function keys
    KeyCodeF2,				/// internal keycode: F2 function keys
    KeyCodeF3,				/// internal keycode: F3 function keys
    KeyCodeF4,				/// internal keycode: F4 function keys
    KeyCodeF5,				/// internal keycode: F5 function keys
    KeyCodeF6,				/// internal keycode: F6 function keys
    KeyCodeF7,				/// internal keycode: F7 function keys
    KeyCodeF8,				/// internal keycode: F8 function keys
    KeyCodeF9,				/// internal keycode: F9 function keys
    KeyCodeF10,				/// internal keycode: F10 function keys
    KeyCodeF11,				/// internal keycode: F11 function keys
    KeyCodeF12,				/// internal keycode: F12 function keys

    KeyCodeKP0,				/// internal keycode: keypad 0
    KeyCodeKP1,				/// internal keycode: keypad 1
    KeyCodeKP2,				/// internal keycode: keypad 2
    KeyCodeKP3,				/// internal keycode: keypad 3
    KeyCodeKP4,				/// internal keycode: keypad 4
    KeyCodeKP5,				/// internal keycode: keypad 5
    KeyCodeKP6,				/// internal keycode: keypad 6
    KeyCodeKP7,				/// internal keycode: keypad 7
    KeyCodeKP8,				/// internal keycode: keypad 8
    KeyCodeKP9,				/// internal keycode: keypad 9
    KeyCodeKPPlus,			/// internal keycode: keypad +
    KeyCodeKPMinus,			/// internal keycode: keypad -

    KeyCodeShift,			/// internal keycode: shift modifier
    KeyCodeControl,			/// internal keycode: ctrl modifier
    KeyCodeAlt,				/// internal keycode: alt modifier
    KeyCodeSuper,			/// internal keycode: super modifier
    KeyCodeHyper,			/// internal keycode: hyper modifier
};

    /// Key modifier
enum _key_modifiers_ {
    ModifierShift	= 1,		/// any shift key pressed
    ModifierControl	= 2,		/// any controll key pressed
    ModifierAlt		= 4,		/// any alt key pressed
    ModifierSuper	= 8,		/// super key (reserved for WM)
    ModifierHyper	= 16,		/// any hyper key pressed
};

#define MouseDoubleShift	8	/// shift for double click button
#define MouseDragShift		16	/// shift for drag button
#define MouseHoldShift		24	/// shift for hold button

    /// pressed mouse button flags
enum _mouse_buttons_ {
    LeftButton		= 2,		/// Left button on mouse
    MiddleButton	= 4,		/// Middle button on mouse
    RightButton		= 8,		/// Right button on mouse

    UpButton		= 16,		/// Scroll up button on mouse
    DownButton		= 32,		/// Scroll down button on mouse

	/// Left+Middle button on mouse
    LeftAndMiddleButton = LeftButton|MiddleButton,
	/// Left+Right button on mouse
    LeftAndRightButton	= LeftButton|RightButton,
	/// Middle+Right button on mouse
    MiddleAndRightButton= MiddleButton|RightButton,
};

    /// Where is our cursor ?
enum _cursor_on_ {
    CursorOnUnknown = -1,		/// not known
    CursorOnMinimap,			/// minimap area
    CursorOnButton,			/// button area see: ButtonUnderCursor
    CursorOnMap,			/// over map area
    CursorOnScrollUp,			/// in scroll up area
    CursorOnScrollDown,			/// in scroll down area
    CursorOnScrollLeft,			/// in scroll left area
    CursorOnScrollRight,		/// in scroll right area
    CursorOnScrollLeftUp,		/// in scroll left+up area
    CursorOnScrollLeftDown,		/// in scroll left+down area
    CursorOnScrollRightUp,		/// in scroll right+up area
    CursorOnScrollRightDown,		/// in scroll right+down area
};

    /// Are We Scrolling With the Keyboard ?
enum _scroll_state_ {
    ScrollNone = 0,			/// not scrolling
    ScrollUp = 1,			/// scroll up only
    ScrollDown = 2,			/// scroll down only
    ScrollLeft = 4,			/// scroll left only
    ScrollRight = 8,			/// scroll right only
    ScrollLeftUp = 5,			/// scroll left+up
    ScrollLeftDown = 6,			/// scroll left+down
    ScrollRightUp = 9,			/// scroll right+up
    ScrollRightDown = 10,		/// scroll right+down
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /// Flag telling if the game is paused
extern char GamePaused;

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

    /// pointer to unit under the cursor
extern Unit* UnitUnderCursor;
    /// button number under the cursor
extern int ButtonUnderCursor;
    /// button 0 (Game Menu) was clicked down
extern int GameMenuButtonClicked;
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
--	Functions
----------------------------------------------------------------------------*/

//
//	in console.c
//
    /// Console clear
extern void ConsoleClear(void);
    /// Console printf
extern void ConsolePrintf(const char*,...);
    /// Redraw the console
extern void DrawConsole(void);

//
//	in botpanel.c
//
    /// Generate all buttons
extern void InitButtons(void);
    /// Free memory for buttons
extern void CleanButtons(void);
    /// Make a new button
extern int AddButton(int pos,int level,const char* IconIdent,
	enum _button_cmd_ action,const char* value,
	const void* func,const void* arg,
	int key,const char* hint,const char* umask);

    /// Save all buttons
extern void SaveButtons(FILE* file);

//
//	in interface.c
//
    /// Called if any mouse button is pressed down
extern void HandleButtonDown(unsigned button);
    /// Called if any mouse button is released up
extern void HandleButtonUp(unsigned button);
    /// Called if the mouse is moved
extern void HandleMouseMove(int x,int y);
    /// Called if a key is pressed
extern void HandleKeyDown(unsigned keycode,unsigned keychar);
    /// Called when a key is released
extern void HandleKeyUp(unsigned keycode,unsigned keychar);

    /// Called if any mouse button is pressed down
extern void InputMouseButtonPress(const EventCallback*,unsigned,unsigned);
    /// Called if any mouse button is released up
extern void InputMouseButtonRelease(const EventCallback*,unsigned,unsigned);
    /// Called if the mouse is moved
extern void InputMouseMove(const EventCallback*,unsigned,int,int);
    /// Called to look for mouse timeout's
extern void InputMouseTimeout(const EventCallback*,unsigned);

//
//	Chaos pur.
//
    /// Called if right mouse button is pressed
extern void DoRightButton(int tx,int ty);
    /// Cancel the building input mode
extern void CancelBuildingMode(void);

    /// Draw messages as overlay over of the map
extern void DrawMessage(void);
    /// Draw the player resource in resource line
extern void DrawResources(void);
    /// Set message to display
extern void SetMessage( const char* fmt, ... );
    /// Set message to display with event point
extern void SetMessage2( int x, int y, const char* fmt, ... );
    /// Set message to display, saving the message
extern void SetMessageDup(const char* message);
    /// Center view-point on last message
extern void CenterOnMessage();

    /// Set status line to show this information
extern void SetStatusLine(char* status);
    /// Clear the content of the message line
extern void ClearStatusLine(void);
    /// Draw status line
extern void DrawStatusLine(void);
    /// Draw costs in status line
extern void DrawCosts(void);
    /// Set costs to be displayed in status line
extern void SetCosts(int,const int* costs);
    /// Clear the costs displayed in status line (undisplay!)
extern void ClearCosts(void);

    /// Draw the unit info panel
extern void DrawInfoPanel(void);
    /// Draw the unit button panel
extern void DrawButtonPanel(void);
    /// Update the content of the unit button panel
extern void UpdateButtonPanel(void);
    /// Handle button click in button panel area
extern void DoButtonButtonClicked(int button);
    /// Lookup key for bottom panel buttons
extern int DoButtonPanelKey(int key);

//
//	in button_table.c
//
    /// Check is always true
extern int ButtonCheckTrue(const Unit* unit,const ButtonAction* button);
    /// Check is always false
extern int ButtonCheckFalse(const Unit* unit,const ButtonAction* button);
    /// Check if allowed upgrade is ready
extern int ButtonCheckUpgrade(const Unit* unit,const ButtonAction* button);
    /// Check if allowed unit exists
extern int ButtonCheckUnit(const Unit* unit,const ButtonAction* button);
    /// Check if allowed units exists
extern int ButtonCheckUnits(const Unit* unit,const ButtonAction* button);
    /// Check if have network play
extern int ButtonCheckNetwork(const Unit* unit,const ButtonAction* button);
    /// Check if unit isn't working (train,upgrade,research)
extern int ButtonCheckNoWork(const Unit* unit,const ButtonAction* button);
    /// Check if unit isn't researching or upgrading
extern int ButtonCheckNoResearch(const Unit* unit,const ButtonAction* button);

    /// Check if all requirements for an attack to are meet
extern int ButtonCheckAttack(const Unit* unit,const ButtonAction* button);
    /// Check if all requirements for an upgrade to are meet
extern int ButtonCheckUpgradeTo(const Unit* unit,const ButtonAction* button);
    /// Check if all requirements for a research are meet
extern int ButtonCheckResearch(const Unit* unit,const ButtonAction* button);

//@}

#endif	// !__INTERFACE_H__
