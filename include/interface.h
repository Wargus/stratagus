//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name interface.h	-	The user interface header file. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

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
    B_Move,				/// order move
    B_Stop,				/// order stop
    B_Attack,				/// order attack
    B_Repair,				/// order repair
    B_Harvest,				/// order harvest
    B_Button,				/// choose other button set
    B_Build,				/// order build
    B_Train,				/// order train
    B_Patrol,				/// order patrol
    B_StandGround,			/// order stand ground
    B_AttackGround,			/// order attack ground
    B_Return,				/// order return goods
    B_Demolish,				/// order demolish/explode
    B_SpellCast,			/// order cast spell
    B_Research,				/// order reseach
    B_UpgradeTo,			/// order upgrade
    B_Unload,				/// order unload unit
    B_Cancel,				/// cancel
    B_CancelTrain,			/// cancel training
    B_CancelBuild,			/// cancel building
};

    /// typedef for action of button
typedef struct _button_action_ ButtonAction;

    /// Action of button
struct _button_action_ {
    int         Pos;                    /// button position in the grid
    int         Level;                  /// requires button level
    IconConfig	Icon;			/// icon to display
    enum _button_cmd_ Action;		/// command on button press
    int		Value;			/// extra value for command
    char*	ValueStr;               /// keep original value string
	/// Check if this button is allowed
    int         (*Allowed)(const Unit* unit,const ButtonAction* button);
    char*	AllowStr;		/// argument for allowed
    int		Key;			/// alternative on keyboard
    char*	Hint;			/// tip text
    char*       UMask;                  /// for which units is available
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

    /// pressed mouse button flags
enum _mouse_buttons_ {
    LeftButton		= 2,		/// Left button on mouse
    MiddleButton	= 4,		/// Middle button on mouse
    RightButton		= 8,		/// Right button on mouse

    // FIXME: support wheel and more buttons

	/// Left+Middle button on mouse
    LeftAndMiddleButton	= LeftButton|MiddleButton,
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

    /// pressed mouse buttons
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

    /// Called if any mouse button is pressed down
extern void HandleButtonDown(int b);
    /// Called if any mouse button is released up
extern void HandleButtonUp(int b);
    /// Called if the mouse is moved
extern void HandleMouseMove(int x,int y);
    /// Called if a key is pressed
extern int HandleKeyDown(int key);
    /// Called when a key is released
extern int HandleKeyUp(int key);

    /// Called if right mouse button is pressed
extern void DoRightButton(int x,int y);
    /// cancel the building input mode
extern void CancelBuildingMode(void);

    /// FIXME: more docu
extern void DrawMessage(void);

extern void DrawResources(void);
extern void DrawMessage(void);
extern void SetMessage( char* fmt, ... );
extern void DrawStatusLine(void);
extern void DrawCosts(void);
extern void SetCosts(int,const int* costs);	/// set costs to be displayed
extern void ClearCosts(void);
extern void DrawInfoPanel(void);
extern void DrawButtonPanel(void);
extern void UpdateButtonPanel(void);
extern void DoButtonButtonClicked(int button);
extern void DoButtonPanelKey(int key);

//@}

#endif	// !__INTERFACE_H__
