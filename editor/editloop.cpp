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
#include "minimap.h"
#include "settings.h"
#include "network.h"
#include "sound_server.h"
#include "ui.h"
#include "interface.h"
#include "font.h"
#include "editor.h"
#include "campaign.h"
#include "menus.h"

#include "ccl.h"

extern void PreMenuSetup(void);		/// FIXME: not here!
extern void DoScrollArea(enum _scroll_state_ state, int fast);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local char EditorRunning;		/// True editor is running

local enum _editor_state_ {
    EditorSelecting,			/// Select
    EditorEditTile,			/// Edit tiles
} EditorState;				/// Current editor state

// FIXME: support for bigger tools 2x2, 3x3, 4x4.
local int TileCursor;			/// TileCursor 

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Draw tile icons.
**
**	@todo for the start the solid tiles are hardcoded
**	If we have more solid tiles, than they fit into the panel, we need
**	some new ideas.
*/
local void DrawTileIcons(void)
{
    unsigned char** tiles;
    int x;
    int y;
    int i;

    tiles=TheMap.Tiles;

    y=TheUI.ButtonPanelY+4;
    i=0;

    while( y<TheUI.ButtonPanelY+100 ) {
	x=TheUI.ButtonPanelX+4;
	while( x<TheUI.ButtonPanelX+144 ) {
	    VideoDrawTile(tiles[TheMap.Tileset->Table[0x10+i*16]],x,y);
	    VideoDrawRectangle(ColorGray,x,y,32,32);
	    if( TileCursor == i ) {
		VideoDrawRectangle(ColorGreen,x+1,y+1,30,30);

	    }
	    if( CursorOn == CursorOnButton && ButtonUnderCursor == i + 100 ) {
		VideoDrawRectangle(ColorWhite,x-1,y-1,34,34);
	    }
	    x+=34;
	    i++;
	}
	y+=34;
    }
}

/**
**	Draw special cursor on map.
**
**	@todo support for bigger cursors (2x2, 3x3 ...)
*/
local void DrawMapCursor(void)
{
    int v;
    int x;
    int y;

    //
    //  Draw map cursor
    //

    v = TheUI.LastClickedVP;
    if (v != -1) {
	x = Viewport2MapX(v, CursorX);
	y = Viewport2MapY(v, CursorY);
	x = Map2ViewportX(v, x);
	y = Map2ViewportY(v, y);
	if (EditorState == EditorEditTile) {
	    VideoDrawTile(TheMap.Tiles[TheMap.Tileset->Table[0x10 +
			TileCursor * 16]], x, y);
	}
	VideoDrawRectangle(ColorWhite, x, y, 32, 32);
    }
}

/**
**	Update editor display.
*/
local void EditorUpdateDisplay(void)
{
    int i;

    VideoLockScreen();			// { prepare video write

    HideAnyCursor();			// remove cursor (when available)

    DrawMapArea();			// draw the map area
    DrawMapCursor();			// cursor on map

    //
    //  Menu button
    //
    if (TheUI.MenuButton.Graphic) {
	VideoDrawSub(TheUI.MenuButton.Graphic, 0, 0,
	    TheUI.MenuButton.Graphic->Width, TheUI.MenuButton.Graphic->Height,
	    TheUI.MenuButtonX, TheUI.MenuButtonY);
    }
    DrawMenuButton(MBUTTON_MAIN,
	    (ButtonUnderCursor == 0 ? MenuButtonActive : 0)|
	    (GameMenuButtonClicked ? MenuButtonClicked : 0),
	    128, 19,
	    TheUI.MenuButtonX+24,TheUI.MenuButtonY+2,
	    GameFont,"Menu (~<F10~>)");

    //
    //  Minimap border
    //
    if (TheUI.Minimap.Graphic) {
	VideoDrawSub(TheUI.Minimap.Graphic, 0, 0, TheUI.Minimap.Graphic->Width,
	    TheUI.Minimap.Graphic->Height, TheUI.MinimapX, TheUI.MinimapY);
    }
    //
    //  Minimap
    //
    i = TheUI.LastClickedVP;
    if (i >= 0) {
	DrawMinimap(TheUI.VP[i].MapX, TheUI.VP[i].MapY);
	DrawMinimapCursor(TheUI.VP[i].MapX, TheUI.VP[i].MapY);
    }
    //
    //  Info panel
    //
    if (TheUI.InfoPanel.Graphic) {
	VideoDrawSub(TheUI.InfoPanel.Graphic, 0, 0,
	    TheUI.InfoPanel.Graphic->Width, TheUI.InfoPanel.Graphic->Height/4,
	    TheUI.InfoPanelX, TheUI.InfoPanelY);
    }
    //
    //  Button panel
    //
    if (TheUI.ButtonPanel.Graphic) {
	VideoDrawSub(TheUI.ButtonPanel.Graphic, 0, 0,
	    TheUI.ButtonPanel.Graphic->Width,
	    TheUI.ButtonPanel.Graphic->Height, TheUI.ButtonPanelX,
	    TheUI.ButtonPanelY);
    }
    DrawTileIcons();
    //
    //  Resource
    //
    if (TheUI.Resource.Graphic) {
	VideoDrawSub(TheUI.Resource.Graphic, 0, 0,
	    TheUI.Resource.Graphic->Width, TheUI.Resource.Graphic->Height,
	    TheUI.ResourceX, TheUI.ResourceY);
    }
    //
    //  Filler
    //
    if (TheUI.Filler1.Graphic) {
	VideoDrawSub(TheUI.Filler1.Graphic, 0, 0, TheUI.Filler1.Graphic->Width,
	    TheUI.Filler1.Graphic->Height, TheUI.Filler1X, TheUI.Filler1Y);
    }
    //
    //  Status line
    //
    if (TheUI.StatusLine.Graphic) {
	VideoDrawSub(TheUI.StatusLine.Graphic, 0, 0,
	    TheUI.StatusLine.Graphic->Width, TheUI.StatusLine.Graphic->Height,
	    TheUI.StatusLineX, TheUI.StatusLineY);
    }

    DrawAnyCursor();

    VideoUnlockScreen();		// } end write access

    // FIXME: For now update everything each frame

    // refresh entire screen, so no further invalidate needed
    InvalidateAreaAndCheckCursor(0, 0, VideoWidth, VideoHeight);
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
}

/**
**	Called if mouse button pressed down.
**
**	@param button	Mouse button number (0 left, 1 middle, 2 right)
*/
global void EditorCallbackButtonDown(unsigned button __attribute__((unused)))
{
    DebugLevel3Fn("%x %x\n",button,MouseButtons);

    //
    //	Click on menu button just exit.
    //
    if( CursorOn == CursorOnButton && ButtonUnderCursor == 0 ) {
	EditorRunning=0;
	return;
    }

    //
    //	Click on tile area
    //
    if( CursorOn == CursorOnButton && ButtonUnderCursor >= 100 ) {
	EditorState = EditorEditTile;
	TileCursor = ButtonUnderCursor - 100;
	return;
    }
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
    int i;
    int bx;
    int by;

    DebugLevel3Fn("Moved %d,%d\n" _C_ x _C_ y);

    HandleCursorMove(&x, &y);		// Reduce to screen

    MouseScrollState = ScrollNone;
    GameCursor = TheUI.Point.Cursor;
    CursorOn = -1;

    //
    //  Handle tile area
    //
    i = 0;
    by = TheUI.ButtonPanelY + 4;
    while (by < TheUI.ButtonPanelY + 100) {
	bx = TheUI.ButtonPanelX + 4;
	while (bx < TheUI.ButtonPanelX + 144) {
	    if (bx < x && x < bx + 32 && by < y && y < by + 32) {
		DebugLevel3Fn("Button %d\n" _C_ i);
		ButtonUnderCursor = i + 100;
		CursorOn = CursorOnButton;
		return;
	    }
	    bx += 34;
	    i++;
	}
	by += 34;
    }

    //
    //  Handle buttons
    //  FIXME: just a copy from the engine.
    //
    for (i = 0; i < sizeof(TheUI.Buttons) / sizeof(*TheUI.Buttons); ++i) {
	if (x < TheUI.Buttons[i].X
		|| x > TheUI.Buttons[i].X + TheUI.Buttons[i].Width
		|| y < TheUI.Buttons[i].Y
		|| y > TheUI.Buttons[i].Y + TheUI.Buttons[i].Height) {
	    continue;
	}
	DebugLevel3("On button %d\n" _C_ i);
	ButtonUnderCursor = i;
	CursorOn = CursorOnButton;
	return;
    }

    //
    //  Scrolling Region Handling
    //
    if (x < SCROLL_LEFT) {
	CursorOn = CursorOnScrollLeft;
	MouseScrollState = ScrollLeft;
	GameCursor = TheUI.ArrowW.Cursor;
	if (y < SCROLL_UP) {
	    CursorOn = CursorOnScrollLeftUp;
	    MouseScrollState = ScrollLeftUp;
	    GameCursor = TheUI.ArrowNW.Cursor;
	}
	if (y > SCROLL_DOWN) {
	    CursorOn = CursorOnScrollLeftDown;
	    MouseScrollState = ScrollLeftDown;
	    GameCursor = TheUI.ArrowSW.Cursor;
	}
	return;
    }
    if (x > SCROLL_RIGHT) {
	CursorOn = CursorOnScrollRight;
	MouseScrollState = ScrollRight;
	GameCursor = TheUI.ArrowE.Cursor;
	if (y < SCROLL_UP) {
	    CursorOn = CursorOnScrollRightUp;
	    MouseScrollState = ScrollRightUp;
	    GameCursor = TheUI.ArrowNE.Cursor;
	}
	if (y > SCROLL_DOWN) {
	    CursorOn = CursorOnScrollRightDown;
	    MouseScrollState = ScrollRightDown;
	    GameCursor = TheUI.ArrowSE.Cursor;
	}
	return;
    }
    if (y < SCROLL_UP) {
	CursorOn = CursorOnScrollUp;
	MouseScrollState = ScrollUp;
	GameCursor = TheUI.ArrowN.Cursor;
	return;
    }
    if (y > SCROLL_DOWN) {
	CursorOn = CursorOnScrollDown;
	MouseScrollState = ScrollDown;
	GameCursor = TheUI.ArrowS.Cursor;
	return;
    }
    //  Not reached if cursor is inside the scroll area
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

    callbacks.ButtonPressed = EditorCallbackButtonDown;
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
    EditorState = EditorSelecting;
    TheUI.LastClickedVP = 0;

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
