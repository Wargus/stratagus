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
/**@name menubuttons.c	-	The menu buttons. */
/*
**	(c) Copyright 1999,2000 by Andreas Arens
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "freecraft.h"
#include "video.h"
#include "player.h"
#include "image.h"
#include "font.h"
#include "tileset.h"
#include "interface.h"
#include "menus.h"
#include "cursor.h"
#include "new_video.h"

/*----------------------------------------------------------------------------
--	Prototypes for action handlers
----------------------------------------------------------------------------*/

local void GameMenuSave(void);
local void GameMenuEnd(void);
local void GameMenuReturn(void);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /// private struct which specifies the buttons gfx
local struct {
	/// resource filename one for each race
    const char*	File[PlayerMaxRaces];
    int		Width, Height;
	/// sprite : FILLED
    RleSprite*	RleSprite;
} MenuButtonGfx = {
    { "interface/buttons 1.png" ,"interface/buttons 2.png" },
    300, 7632
};

/**
**	The currently processed menu
*/
global int CurrentMenu = -1;

local int MenuButtonUnderCursor = -1;

global Menu Menus[] = {
    {
	/// Game Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	ImagePanel1,
	RedrawMapOverlay,
	8,
	{
	    { MI_TYPE_TEXT, { text:{ 128, 11, MI_FLAGS_CENTERED, LargeFont, "Game Menu"} } },
	    { MI_TYPE_BUTTON, { button:{ 16, 40, MenuButtonDisabled, LargeFont,
		 "Save (~<F11~>)", 106, 27, MBUTTON_GM_HALF, KeyCodeF11, GameMenuSave} } },
	    { MI_TYPE_BUTTON, { button:{ 16 + 12 + 106, 40, MenuButtonDisabled, LargeFont,
		 "Load (~<F12~>)", 106, 27, MBUTTON_GM_HALF, KeyCodeF12, NULL} } },
	    { MI_TYPE_BUTTON, { button:{ 16, 40 + 36, MenuButtonDisabled, LargeFont,
		 "Options (~<F5~>)", 224, 27, MBUTTON_GM_FULL, KeyCodeF5, NULL} } },
	    { MI_TYPE_BUTTON, { button:{ 16, 40 + 36 + 36, MenuButtonDisabled, LargeFont,
		 "Help (~<F1~>)", 224, 27, MBUTTON_GM_FULL, KeyCodeF1, NULL} } },
	    { MI_TYPE_BUTTON, { button:{ 16, 40 + 36 + 36 + 36, MenuButtonDisabled, LargeFont,
		 "Scenario ~!Objectives", 224, 27, MBUTTON_GM_FULL, 'o', NULL} } },
	    { MI_TYPE_BUTTON, { button:{ 16, 40 + 36 + 36 + 36 + 36, 0, LargeFont,
		 "~!End Scenario", 224, 27, MBUTTON_GM_FULL, 'e', GameMenuEnd} } },
	    { MI_TYPE_BUTTON, { button:{ 16, 288-40, MenuButtonSelected, LargeFont,
		 "Return to Game (~<Esc~>)", 224, 27, MBUTTON_GM_FULL, '\033', GameMenuReturn} } },
	},
    },
};

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Draw menu button 'button' on x,y
**
**	@param button	Button identifier
**	@param flags	State of Button (clicked, mouse over...)
**	@param w	Button width (for border)
**	@param h	Button height (for border)
**	@param x	X display position
**	@param y	Y display position
**	@param font	font number for text
**	@param text	text to print on button
*/
global void DrawMenuButton(MenuButtonId button,unsigned flags,unsigned w,unsigned h,unsigned x,unsigned y,
	const int font,const unsigned char *text)
{
    MenuButtonId rb;
    int s, nc, rc;

    GetDefaultTextColors(&nc, &rc);
    if (flags&MenuButtonDisabled) {
	rb = button - 1;
	s = 0;
	SetDefaultTextColors(FontGrey,FontGrey);
    } else if (flags&MenuButtonClicked) {
	rb = button + 1;
	s = 2;
	SetDefaultTextColors(rc,rc);
    } else {
	rb = button;
	s = 0;
	if (flags&MenuButtonActive) {
	    SetDefaultTextColors(rc,rc);
	}
    }
    DrawRleSprite(MenuButtonGfx.RleSprite, rb, x, y);
    if (text) {
	DrawTextCentered(s+x+w/2,s+y+(font == GameFont ? 4 : 7),font,text);
    }
    if (flags&MenuButtonSelected) {
	/// FIXME: use ColorGrey if selected button is disabled!
	VideoDrawRectangle(ColorYellow,x,y,w,h);
    }
    SetDefaultTextColors(nc,rc);
}


/**
**	Draw menu  'menu'
**
**	@param Menu	The menu number to display
*/
global void DrawMenu(int MenuId)
{
    int i;
    Menu *menu;
    Menuitem *mi;

    if (MenuId == -1) {
	return;
    }
    menu = Menus + MenuId;
    if (menu->image != ImageNone) {
	DrawImage(menu->image,0,0,menu->x,menu->y);
    }
    for (i = 0; i < menu->nitems; i++) {
	mi = &menu->items[i];
	switch (mi->mitype) {
	    case MI_TYPE_TEXT:
		if (mi->d.text.flags&MI_FLAGS_CENTERED)
		    DrawTextCentered(menu->x+mi->d.text.xofs,menu->y+mi->d.text.yofs,
			    mi->d.text.font,mi->d.text.text);
		else
		    DrawText(menu->x+mi->d.text.xofs,menu->y+mi->d.text.yofs,
			    mi->d.text.font,mi->d.text.text);
		break;
	    case MI_TYPE_BUTTON:
		    DrawMenuButton(mi->d.button.button,mi->d.button.flags,
			    mi->d.button.xsize,mi->d.button.ysize,
			    menu->x+mi->d.button.xofs,menu->y+mi->d.button.yofs,
			    mi->d.button.font,mi->d.button.text);
		break;
	    default:
		break;
	}
    }
    MustRedraw |= menu->area;	// for Invalidate()
}

/*----------------------------------------------------------------------------
--	Button action handler functions
----------------------------------------------------------------------------*/

local void GameMenuReturn(void)
{
    ClearStatusLine();
    InterfaceState=IfaceStateNormal;
    MustRedraw&=~RedrawMapOverlay;
    MustRedraw|=RedrawMap;
    GamePaused=0;
    CursorOn=CursorOnUnknown;
    CurrentMenu=-1;
    /// FIXME: restore mouse pointer to sane state (call fake mouse move?)
}

local void GameMenuSave(void)
{
    SaveAll();	/// FIXME: Sample code
}

local void GameMenuEnd(void)
{
    Exit(0);
}

/**
**	Handle keys in menu mode.
**
**	@param key	Key scancode.
**	@return		True, if key is handled; otherwise false.
*/
global int MenuKey(int key)
{
    int i;
    Menuitem *mi;
    Menu *menu = Menus + CurrentMenu;

    i = menu->nitems;
    mi = menu->items;
    while (i--) {
	switch (mi->mitype) {
	    case MI_TYPE_BUTTON:
		if (key == mi->d.button.hotkey) {
		    if (mi->d.button.handler) {
			(*mi->d.button.handler)();
		    }
		    return 1;
		}
	    default:
		break;
	}
	mi++;
    }
    /// FIXME: ADD <RETURN-KEY> HANDLER HERE!
    switch (key) {
	case 'q':
	    Exit(0);
	default:
	    DebugLevel3("Key %d\n",key);
	    return 0;
    }
    return 1;
}


/**
**	Handle movement of the cursor.
**
**	@param x	Screen X position.
**	@param y	Screen Y position.
*/
global void MenuHandleMouseMove(int x,int y)
{
    int i, n, xs, ys;
    Menuitem *mi;
    Menu *menu = Menus + CurrentMenu;
    int RedrawFlag = 0;

    n = menu->nitems;
    MenuButtonUnderCursor = -1;
    for (i = 0; i < n; ++i) {
	mi = menu->items + i;
	switch (mi->mitype) {
	    case MI_TYPE_BUTTON:
		if (!(mi->d.button.flags&MenuButtonDisabled)) {
		    xs = menu->x + mi->d.button.xofs;
		    ys = menu->y + mi->d.button.yofs;
		    if (x < xs || x > xs + mi->d.button.xsize || y < ys || y > ys + mi->d.button.ysize) {
			if (!(mi->d.button.flags&MenuButtonClicked)) {
			    if (mi->d.button.flags&MenuButtonActive) {
				RedrawFlag = 1;
				mi->d.button.flags &= ~MenuButtonActive;
			    }
			}
			continue;
		    }
		    if (!(mi->d.button.flags&MenuButtonActive)) {
			RedrawFlag = 1;
			mi->d.button.flags |= MenuButtonActive;
		    }
		    DebugLevel3("On menu button %d\n", i);
		    MenuButtonUnderCursor = i;
		}
		break;
	    default:
		break;
	}
    }
    if (RedrawFlag) {
	MustRedraw |= RedrawMenu;
    }
}

/**
**	Called if mouse button pressed down.
**
**	@param b	button code
*/
global void MenuHandleButtonDown(int b)
{
    Menuitem *mi;

    if (MouseButtons&LeftButton) {
	if (MenuButtonUnderCursor != -1) {
	    mi = Menus[CurrentMenu].items + MenuButtonUnderCursor;
	    if (!(mi->d.button.flags&MenuButtonClicked)) {
		mi->d.button.flags |= MenuButtonClicked; /// FIXME: | MenuButtonSelected (like original!)
		MustRedraw |= RedrawMenu;
	    }
	}
    }
}

/**
**	Called if mouse button released.
**
**	@param b	button code
*/
global void MenuHandleButtonUp(int b)
{
    int i, n;
    Menuitem *mi;
    Menu *menu = Menus + CurrentMenu;
    int RedrawFlag = 0;

    if ((1<<b) == LeftButton) {
	n = menu->nitems;
	for (i = 0; i < n; ++i) {
	    mi = menu->items + i;
	    switch (mi->mitype) {
		case MI_TYPE_BUTTON:
		    if (mi->d.button.flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->d.button.flags &= ~MenuButtonClicked;
			if (MenuButtonUnderCursor == i) {
			    MenuButtonUnderCursor = -1;
			    if (mi->d.button.handler) {
				(*mi->d.button.handler)();
			    }
			}
		    }
		    break;
		default:
		    break;
	    }
	}
    }
    if (RedrawFlag) {
	MustRedraw |= RedrawMenu;
    }
}


/**
**	Process menu  'menu'
**
**	@param Menu	The menu number to process
**	@param Loop	Indicates to setup handlers and really 'Process'
*/
global void ProcessMenu(int MenuId, int Loop)
{
    int i;
    Menu *menu;
    Menuitem *mi;

    InterfaceState = IfaceStateMenu;
    HideCursor();
    CursorState = CursorStatePoint;
    GameCursor = &Cursors[CursorTypePoint];
    CurrentMenu = MenuId;
    menu = Menus + CurrentMenu;
    for (i = 0; i < menu->nitems; ++i) {
	mi = menu->items + i;
	switch (mi->mitype) {
	    case MI_TYPE_BUTTON:
		mi->d.button.flags &= ~(MenuButtonClicked|MenuButtonActive);
		// FIXME: Activate if mouse is over it!!!
		break;
	    default:
		break;
	}
    }
    MenuButtonUnderCursor = -1;
    if (Loop) {
	InitVideoSync();
	MustRedraw = 0;
    }
    DrawMenu(CurrentMenu);

    if (Loop) {
	Invalidate();
	for( ; CurrentMenu != -1 ; ) {
	    DebugLevel3("MustRedraw: 0x%08x\n",MustRedraw);
	    UpdateDisplay();
	    if (MustRedraw & Menus[CurrentMenu].area) {    
		Invalidate();
	    }
	    VideoInterrupts=0;
	    RealizeVideoMemory();
	    WaitEventsAndKeepSync();
	}
    }
}


/**
**	Init Menus for a specific race
**
**	@param race	The Race to set-up for
*/
global void InitMenus(unsigned int race)
{
    static int last_race = -1;
    const char* file;
    char *buf;

    if (race == last_race)	// same race? already loaded!
	return;
    if (last_race != -1) {	// free previous sprites for different race
	FreeRleSprite(MenuButtonGfx.RleSprite);
    }
    last_race = race;
    file = MenuButtonGfx.File[race];
    buf = alloca(strlen(file) + 9 + 1);
    file = strcat(strcpy(buf, "graphic/"), file);
    MenuButtonGfx.RleSprite = LoadRleSprite(file, 0, 144);
}

//@}
