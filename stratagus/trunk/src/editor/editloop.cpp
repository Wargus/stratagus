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
/**@name editloop.c	-	The editor main loop. */
//
//	(c) Copyright 2002 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"
#include "video.h"
#include "map.h"
#include "settings.h"
#include "network.h"
#include "sound_server.h"
#include "ui.h"
#include "interface.h"
#include "font.h"
#include "editor.h"
#include "campaign.h"

#include "ccl.h"

extern void PreMenuSetup(void);		/// FIXME: not here!
extern void DoScrollArea(enum _scroll_state_ state, int fast);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local char EditorRunning;		/// True editor is running

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Update editor display.
*/
local void EditorUpdateDisplay(void)
{
    VideoLockScreen();			// { prepare video write

    HideAnyCursor();			// remove cursor (when available)

    DrawMapArea();			// draw the map area

    DrawAnyCursor();

    VideoUnlockScreen();		// } end write access

    // FIXME: For now update everything each frame

    // refresh entire screen, so no further invalidate needed
    InvalidateAreaAndCheckCursor(0,0,VideoWidth,VideoHeight);
}

/*----------------------------------------------------------------------------
--	Input / Keyboard / Mouse
----------------------------------------------------------------------------*/

/**
**	Callback for input.
*/
local void EditorCallbackKey(unsigned dummy __attribute__((unused)))
{
    DebugLevel3Fn("Pressed %8x %8x\n" _C_ MouseButtons _C_ dummy);
    EditorRunning=0;
}

/**
**	Handle key down.
**
**	@param key	Key scancode.
**	@param keychar	Character code.
*/
global void EditorCallbackKeyDown(unsigned key, unsigned keychar)
{
    if (HandleKeyModifiersDown(key, keychar)) {
	return;
    }

    switch (key) {
	case KeyCodeUp:		// Keyboard scrolling
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
	    DebugLevel3("Key %d\n" _C_ key);
	    return;
    }
    return;
}

/**
**	Handle key up.
**
**	@param key	Key scancode.
**	@param keychar	Character code.
*/
global void EditorCallbackKeyUp(unsigned key, unsigned keychar)
{
    if (HandleKeyModifiersUp(key, keychar)) {
	return;
    }

    switch (key) {
	case KeyCodeUp:			// Keyboard scrolling
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
**	Callback for input.
*/
local void EditorCallbackKey2(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
    DebugLevel3Fn("Pressed %8x %8x %8x\n" _C_ MouseButtons _C_ dummy1 _C_
	    dummy2);
}

/**
**	Callback for input.
*/
local void EditorCallbackKey3(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
    DebugLevel3Fn("Repeated %8x %8x %8x\n" _C_ MouseButtons _C_ dummy1 _C_
	    dummy2);
}

/**
**	Callback for input movement of the cursor.
**
**	@param x	Screen X position.
**	@param y	Screen Y position.
*/
local void EditorCallbackMouse(int x, int y)
{
    DebugLevel2Fn("Moved %d,%d\n" _C_ x _C_ y);

    HandleCursorMove(&x,&y);
}

/**
**	Callback for exit.
*/
local void EditorCallbackExit(void)
{
    DebugLevel3Fn("Exit\n");
}

/**
**	Create editor.
*/
local void CreateEditor(void)
{
    FlagRevealMap=1;
    TheMap.NoFogOfWar=1;
    CreateGame(CurrentMapPath,&TheMap);
    FlagRevealMap=0;
}

/**
**	Editor main event loop.
*/
global void EditorMainLoop(void)
{
    EventCallback callbacks;

    CreateEditor();

    SetVideoSync();

    callbacks.ButtonPressed = EditorCallbackKey;
    callbacks.ButtonReleased = EditorCallbackKey;
    callbacks.MouseMoved = EditorCallbackMouse;
    callbacks.MouseExit = EditorCallbackExit;
    callbacks.KeyPressed = EditorCallbackKeyDown;
    callbacks.KeyReleased = EditorCallbackKeyUp;
    callbacks.KeyRepeated = EditorCallbackKey3;

    callbacks.NetworkEvent = NetworkEvent;
    callbacks.SoundReady = WriteSound;

    GameCursor = TheUI.Point.Cursor;
    InterfaceState = IfaceStateNormal;

    EditorRunning = 1;
    while (EditorRunning) {
	EditorUpdateDisplay();

	//
	//	Map scrolling
	//
	if( TheUI.MouseScroll && !(FrameCounter%SpeedMouseScroll) ) {
	    DoScrollArea(MouseScrollState, 0);
	}
	if( TheUI.KeyScroll && !(FrameCounter%SpeedKeyScroll) ) {
	    DoScrollArea(KeyScrollState, KeyModifiers&ModifierControl);
	}

	if( !(FrameCounter%COLOR_CYCLE_SPEED) ) {
	    ColorCycle();
	}

	WaitEventsOneFrame(&callbacks);
    }

    //
    //	Restore all for menu
    //
    CleanModules();
    CleanFonts();

    LoadCcl();			// Reload the main config file

    PreMenuSetup();

    InterfaceState = IfaceStateMenu;
    GameCursor = TheUI.Point.Cursor;

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();
}

//@}
