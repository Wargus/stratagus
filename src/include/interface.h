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

    /// Cursor state
enum _cursor_state_ {
    CursorStatePoint,			/// normal cursor
    CursorStateSelect,			/// select position
    CursorStateRectangle,		/// rectangle selecting
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

    /// Flag telling if the game is paused.
extern char GamePaused;

    /// pressed mouse buttons (normal,double,dragged,long)
extern enum _mouse_buttons_ MouseButtons;
    /// current active modifiers
extern enum _key_modifiers_ KeyModifiers;
    /// current cursor state
extern enum _cursor_state_ CursorState;
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

    /// vladi: used for unit buttons sub-menus etc.
extern int CurrentButtonLevel;

    /// Display the command key in the buttons.
extern char ShowCommandKey;

    /// All buttons in game
extern ButtonAction AllButtons[];

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
extern void DoneButtons(void);
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
extern void HandleKeyDown(unsigned key);
    /// Called when a key is released
extern void HandleKeyUp(unsigned key);

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
extern void DoRightButton(int x,int y);
    /// cancel the building input mode
extern void CancelBuildingMode(void);

    /// FIXME: more docu
extern void DrawMessage(void);
    /// FIXME: more docu
extern void DrawResources(void);
    /// FIXME: more docu
extern void DrawMessage(void);
    /// FIXME: more docu
extern void SetMessage( char* fmt, ... );
    /// FIXME: more docu
extern void DrawStatusLine(void);
    /// FIXME: more docu
extern void DrawCosts(void);
    /// FIXME: more docu
extern void SetCosts(int,const int* costs);	/// set costs to be displayed
    /// FIXME: more docu
extern void ClearCosts(void);
    /// FIXME: more docu
extern void DrawInfoPanel(void);
    /// FIXME: more docu
extern void DrawButtonPanel(void);
    /// FIXME: more docu
extern void UpdateButtonPanel(void);
    /// FIXME: more docu
extern void DoButtonButtonClicked(int button);
    /// Lookup key for bottom panel buttons.
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
