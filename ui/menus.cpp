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

#ifndef NEW_VIDEO
#error ONLY WORKS WITH NEW VIDEO!
#endif

/*----------------------------------------------------------------------------
--	Prototypes for action handlers
----------------------------------------------------------------------------*/

local void GameMenuSave(void);
local void GameMenuEnd(void);
local void GameMenuReturn(void);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

// FIXME: Johns: this must be all be configured from ccl some time.

    /// private struct which specifies the buttons gfx
local struct {
	/// resource filename one for each race
    const char*	File[PlayerMaxRaces];
	/// Width of button
    int		Width, Height;
	/// sprite : FILLED
    Graphic*	Sprite;
} MenuButtonGfx = {
    { "interface/buttons 1.png" ,"interface/buttons 2.png" },
    300, 7632
};

/**
**	The currently processed menu
*/
global int CurrentMenu = -1;

local int MenuButtonUnderCursor = -1;
local int MenuButtonCurSel = -1;

/**
**	Items for the Game Menu
*/
local Menuitem GameMenuItems[] = {
    { MI_TYPE_TEXT, { text:{ 128, 11, MI_FLAGS_CENTERED, LargeFont, "Game Menu"} } },
    { MI_TYPE_BUTTON, { button:{ 16, 40, MenuButtonDisabled, LargeFont,
	 "Save (~<F11~>)", 106, 27, MBUTTON_GM_HALF, GameMenuSave, KeyCodeF11} } },
    { MI_TYPE_BUTTON, { button:{ 16 + 12 + 106, 40, MenuButtonDisabled, LargeFont,
	 "Load (~<F12~>)", 106, 27, MBUTTON_GM_HALF, NULL, KeyCodeF12} } },
    { MI_TYPE_BUTTON, { button:{ 16, 40 + 36, MenuButtonDisabled, LargeFont,
	 "Options (~<F5~>)", 224, 27, MBUTTON_GM_FULL, NULL, KeyCodeF5} } },
    { MI_TYPE_BUTTON, { button:{ 16, 40 + 36 + 36, MenuButtonDisabled, LargeFont,
	 "Help (~<F1~>)", 224, 27, MBUTTON_GM_FULL, NULL, KeyCodeF1} } },
    { MI_TYPE_BUTTON, { button:{ 16, 40 + 36 + 36 + 36, MenuButtonDisabled, LargeFont,
	 "Scenario ~!Objectives", 224, 27, MBUTTON_GM_FULL, NULL, 'o'} } },
    { MI_TYPE_BUTTON, { button:{ 16, 40 + 36 + 36 + 36 + 36, 0, LargeFont,
	 "~!End Scenario", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'e'} } },
    { MI_TYPE_BUTTON, { button:{ 16, 288-40, MenuButtonSelected, LargeFont,
	 "Return to Game (~<Esc~>)", 224, 27, MBUTTON_GM_FULL, GameMenuReturn, '\033'} } },
};

/**
**	Items for the Victory Menu
*/
local Menuitem VictoryMenuItems[] = {
    { MI_TYPE_TEXT, { text:{ 144, 11, MI_FLAGS_CENTERED, LargeFont, "Congratulations!"} } },
    { MI_TYPE_TEXT, { text:{ 144, 32, MI_FLAGS_CENTERED, LargeFont, "You are victorious!"} } },
    { MI_TYPE_BUTTON, { button:{ 32, 90, MenuButtonSelected, LargeFont,
	 "~!Victory", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'v'} } },
    { MI_TYPE_BUTTON, { button:{ 32, 56, MenuButtonDisabled, LargeFont,
	 "Save Game (~<F11~>)", 224, 27, MBUTTON_GM_FULL, NULL, KeyCodeF11} } },
};

/**
**	Items for the Lost Menu
*/
local Menuitem LostMenuItems[] = {
    { MI_TYPE_TEXT, { text:{ 144, 11, MI_FLAGS_CENTERED, LargeFont, "You failed to"} } },
    { MI_TYPE_TEXT, { text:{ 144, 32, MI_FLAGS_CENTERED, LargeFont, "achieve victory!"} } },
    { MI_TYPE_BUTTON, { button:{ 32, 90, MenuButtonSelected, LargeFont,
	 "~!OK", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'o'} } },
};

/**
**	Items for the SelectScen Menu
*/
local unsigned char *ssmtoptions[] = {
    "Built-in scenario",
    "Custom scenario"
};

local unsigned char *ssmsoptions[] = {
    "Any size",
    "32 x 32",
    "64 x 64",
    "96 x 96",
    "128 x 128",
};

local Menuitem SelectScenMenuItems[] = {
    { MI_TYPE_TEXT, { text:{ 176, 8, MI_FLAGS_CENTERED, LargeFont, "Select scenario"} } },
    { MI_TYPE_TEXT, { text:{ 132, 40, MI_FLAGS_RALIGN, LargeFont, "Type:"} } },
    { MI_TYPE_PULLDOWN, { pulldown:{ 140, 40, 0, GameFont, ssmtoptions,
	      192, 20, MBUTTON_PULLDOWN, NULL, 2, 1, 1, 0} } },
    { MI_TYPE_TEXT, { text:{ 132, 80, MI_FLAGS_RALIGN, LargeFont, "Map size:"} } },
    { MI_TYPE_PULLDOWN, { pulldown:{ 140, 80, 0, GameFont, ssmsoptions,
	      192, 20, MBUTTON_PULLDOWN, NULL, 5, 0, 0, 0} } },
    { MI_TYPE_BUTTON, { button:{ 48, 318, MenuButtonSelected, LargeFont,
	 "OK", 106, 27, MBUTTON_GM_HALF, NULL, 0} } },
    { MI_TYPE_BUTTON, { button:{ 198, 318, 0, LargeFont,
	 "Cancel", 106, 27, MBUTTON_GM_HALF, NULL, 0} } },
};

/**
**	Menus
*/
global Menu Menus[] = {
    {
	/// Game Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	7, 8,
	GameMenuItems
    },
    {
	/// Victory Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-128)/2,
	288, 128,
	ImagePanel4,
	2, 4,
	VictoryMenuItems
    },
    {
	/// Lost Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-128)/2,
	288, 128,
	ImagePanel4,
	2, 3,
	LostMenuItems
    },
    {
	/// SelectScen Menu
	(640-352)/2,
	(480-352)/2,
	352, 352,
	ImagePanel5,
	5, 7,
	SelectScenMenuItems
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
    VideoDraw(MenuButtonGfx.Sprite, rb, x, y);
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
**	Draw pulldown 'button' on menu mx, my
**
**	@param mi	menuitem pointer
**	@param mx	menu X display position (offset)
**	@param my	menu Y display position (offset)
*/
local void DrawPulldown(Menuitem *mi, unsigned mx, unsigned my)
{
    int i, nc, rc;
    char *text;
    MenuButtonId rb = mi->d.pulldown.button;
    unsigned flags = mi->d.pulldown.flags;
    unsigned w, h, x, y, oh;
    w = mi->d.pulldown.xsize;
    x = mx+mi->d.pulldown.xofs;
    y = my+mi->d.pulldown.yofs;
    oh = h = mi->d.pulldown.ysize - 2;

    GetDefaultTextColors(&nc, &rc);
    if (flags&MenuButtonClicked) {
	y -= mi->d.pulldown.curopt * h;
	i = mi->d.pulldown.noptions;
	h *= i;
	while (i--) {
	    PushClipping();
	    SetClipping(0,0,x+w,VideoHeight);
	    VideoDrawClip(MenuButtonGfx.Sprite, rb, x-1, y-1 + oh*i);
	    PopClipping();
	    text = mi->d.pulldown.options[i];
	    if (text) {
		if (i == mi->d.pulldown.cursel)
		    SetDefaultTextColors(rc,rc);
		else
		    SetDefaultTextColors(nc,rc);
		DrawText(x+2,y+2 + oh*i ,mi->d.pulldown.font,text);
	    }
	}
	w += 2;
    } else {
	h = mi->d.pulldown.ysize;
	y = my+mi->d.pulldown.yofs;
	if (flags&MenuButtonDisabled) {
	    rb--;
	    SetDefaultTextColors(FontGrey,FontGrey);
	} else {
	    if (flags&MenuButtonActive) {
		SetDefaultTextColors(rc,rc);
	    }
	}
	
	PushClipping();
	SetClipping(0,0,x+w-20,VideoHeight);
	VideoDrawClip(MenuButtonGfx.Sprite, rb, x-1, y-1);
	PopClipping();
	VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW + rb - MBUTTON_PULLDOWN, x-1 + w-20, y-2);
	text = mi->d.pulldown.options[mi->d.pulldown.curopt];
	if (text) {
	    DrawText(x+2,y+2,mi->d.pulldown.font,text);
	}
    }
    if (flags&MenuButtonSelected) {
	VideoDrawRectangle(ColorYellow,x-2,y-2,w,h);
	/// FIXME: use ColorGrey if selected button is disabled!
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
    int i, n, l;
    Menu *menu;
    Menuitem *mi;

    if (MenuId == -1) {
	return;
    }
    menu = Menus + MenuId;
    if (menu->image != ImageNone) {
	DrawImage(menu->image,0,0,menu->x,menu->y);
    }
    n = menu->nitems;
    mi = menu->items;
    for (i = 0; i < n; i++) {
	switch (mi->mitype) {
	    case MI_TYPE_TEXT:
		if (mi->d.text.flags&MI_FLAGS_CENTERED)
		    DrawTextCentered(menu->x+mi->d.text.xofs,menu->y+mi->d.text.yofs,
			    mi->d.text.font,mi->d.text.text);
		else if (mi->d.text.flags&MI_FLAGS_RALIGN) {
		    l = TextLength(mi->d.text.font,mi->d.text.text);
		    DrawText(menu->x+mi->d.text.xofs-l,menu->y+mi->d.text.yofs,
			    mi->d.text.font,mi->d.text.text);
		} else
		    DrawText(menu->x+mi->d.text.xofs,menu->y+mi->d.text.yofs,
			    mi->d.text.font,mi->d.text.text);
		break;
	    case MI_TYPE_BUTTON:
		    DrawMenuButton(mi->d.button.button,mi->d.button.flags,
			    mi->d.button.xsize,mi->d.button.ysize,
			    menu->x+mi->d.button.xofs,menu->y+mi->d.button.yofs,
			    mi->d.button.font,mi->d.button.text);
		break;
	    case MI_TYPE_PULLDOWN:
		    DrawPulldown(mi,menu->x,menu->y);
		break;
	    default:
		break;
	}
	mi++;
    }
    InvalidateArea(menu->x,menu->y,menu->xsize,menu->ysize);
}

/*----------------------------------------------------------------------------
--	Button action handler functions
----------------------------------------------------------------------------*/

local void GameMenuReturn(void)
{
    ClearStatusLine();
    InterfaceState=IfaceStateNormal;
    MustRedraw&=~RedrawMenu;
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
global int MenuKey(int key)		// FIXME: Should be MenuKeyDown(), and act on _new_ MenuKeyUp() !!!
{					//        to implement button animation (depress before action)
    int i, n;
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
    switch (key) {
	case 10: case 13:			/// RETURN
	    if (MenuButtonCurSel != -1) {
		mi = menu->items + MenuButtonCurSel;
		switch (mi->mitype) {
		    case MI_TYPE_BUTTON:
			if (mi->d.button.handler) {
			    (*mi->d.button.handler)();
			}
			return 1;
		    default:
			break;
		}
	    }
	    break;
	case KeyCodeUp: case KeyCodeDown:
	    if (MenuButtonCurSel != -1) {
		mi = menu->items + MenuButtonCurSel;
		if (mi->mitype == MI_TYPE_PULLDOWN && !(mi->d.pulldown.flags&MenuButtonClicked)) {
		    if (key == KeyCodeDown) {
			if (mi->d.pulldown.curopt + 1 < mi->d.pulldown.noptions)
			    mi->d.pulldown.curopt++;
			else
			    break;
		    } else {
			if (mi->d.pulldown.curopt > 0)
			    mi->d.pulldown.curopt--;
			else
			    break;
		    }
		    MustRedraw |= RedrawMenu;
		    // FIXME: DISPLAY-ACTION HERE .....
		}
	    }
	    break;
	case 9: 				/// TAB			// FIXME: Add Shift-TAB
	    if (MenuButtonCurSel != -1 && !(menu->items[MenuButtonCurSel].d.button.flags&MenuButtonClicked)) {
		n = menu->nitems;
		for (i = 0; i < n; ++i) {
		    mi = menu->items + ((MenuButtonCurSel + i + 1) % n);
		    switch (mi->mitype) {
			case MI_TYPE_TEXT:
			    break;
			case MI_TYPE_BUTTON:
			    if (mi->d.button.flags & MenuButtonDisabled) {
				break;
			    }
			    mi->d.button.flags |= MenuButtonSelected;
			    menu->items[MenuButtonCurSel].d.button.flags &= ~MenuButtonSelected;
			    MenuButtonCurSel = mi - menu->items;
			    MustRedraw |= RedrawMenu;
			    return 1;
			case MI_TYPE_PULLDOWN:
			    if (mi->d.pulldown.flags & MenuButtonDisabled) {
				break;
			    }
			    mi->d.pulldown.flags |= MenuButtonSelected;
			    menu->items[MenuButtonCurSel].d.button.flags &= ~MenuButtonSelected;
			    // FIXME: maybe different union member, not button!
			    MenuButtonCurSel = mi - menu->items;
			    MustRedraw |= RedrawMenu;
			    return 1;
			default:
			    break;
		    }
		}
	    }
	    break;
	case 'Q':
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
    int h, i, j, n, xs, ys;
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
	    case MI_TYPE_PULLDOWN:
		if (!(mi->d.pulldown.flags&MenuButtonDisabled)) {
		    xs = menu->x + mi->d.pulldown.xofs;
		    if (mi->d.pulldown.flags&MenuButtonClicked) {
			ys = menu->y + mi->d.pulldown.yofs;
			h = mi->d.pulldown.ysize - 2;
			ys -= mi->d.pulldown.curopt * h;
			if (x<xs || x>xs + mi->d.pulldown.xsize || y<ys || y>ys + h*mi->d.pulldown.noptions) {
			    continue;
			}
			j = (y - ys) / h;
			if (j != mi->d.pulldown.cursel) {
			    mi->d.pulldown.cursel = j;
			    RedrawFlag = 1;
			    // FIXME: DISPLAY-ACTION HERE .....
			}
		    } else {
			ys = menu->y + mi->d.pulldown.yofs;
			if (x<xs || x>xs + mi->d.pulldown.xsize || y<ys || y>ys + mi->d.pulldown.ysize) {
			    if (!(mi->d.pulldown.flags&MenuButtonClicked)) {
				if (mi->d.pulldown.flags&MenuButtonActive) {
				    RedrawFlag = 1;
				    mi->d.pulldown.flags &= ~MenuButtonActive;
				}
			    }
			    continue;
			}
		    }
		    if (!(mi->d.pulldown.flags&MenuButtonActive)) {
			RedrawFlag = 1;
			mi->d.pulldown.flags |= MenuButtonActive;
		    }
		    DebugLevel3("On menu pulldown %d\n", i);
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
    Menu *menu = Menus + CurrentMenu;

    if (MouseButtons&LeftButton) {
	if (MenuButtonUnderCursor != -1) {
	    mi = menu->items + MenuButtonUnderCursor;
	    switch (mi->mitype) {
		case MI_TYPE_BUTTON:
		    if (!(mi->d.button.flags&MenuButtonClicked)) {
			if (MenuButtonCurSel != -1) {
			    // FIXME: maybe different union member, not button!
			    menu->items[MenuButtonCurSel].d.button.flags &= ~MenuButtonSelected;
			}
			MenuButtonCurSel = MenuButtonUnderCursor;
			mi->d.button.flags |= MenuButtonClicked|MenuButtonSelected;
			MustRedraw |= RedrawMenu;
		    }
		    break;
		case MI_TYPE_PULLDOWN:
		    if (!(mi->d.pulldown.flags&MenuButtonClicked)) {
			if (MenuButtonCurSel != -1) {
			    // FIXME: maybe different union member, not button!
			    menu->items[MenuButtonCurSel].d.button.flags &= ~MenuButtonSelected;
			}
			MenuButtonCurSel = MenuButtonUnderCursor;
			mi->d.pulldown.flags |= MenuButtonClicked|MenuButtonSelected;
			mi->d.pulldown.cursel = mi->d.pulldown.curopt;
			MustRedraw |= RedrawMenu;
		    }
		default:
		    break;
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
		case MI_TYPE_PULLDOWN:
		    if (mi->d.pulldown.flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->d.pulldown.flags &= ~MenuButtonClicked;
			if (MenuButtonUnderCursor == i) {
			    MenuButtonUnderCursor = -1;
			    if (mi->d.pulldown.cursel != mi->d.pulldown.curopt) {
				mi->d.pulldown.curopt = mi->d.pulldown.cursel;
				if (mi->d.button.handler) {
				    (*mi->d.button.handler)();
				}
			    }
			}
			mi->d.pulldown.cursel = 0;
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
    // FIXME: Recursion: Save: CurrentMenu, MenuButtonCurSel, MenuButtonUnderCursor

    InterfaceState = IfaceStateMenu;
    HideCursor();
    CursorState = CursorStatePoint;
    GameCursor = &Cursors[CursorTypePoint];
    CurrentMenu = MenuId;
    menu = Menus + CurrentMenu;
    MenuButtonCurSel = -1;
    for (i = 0; i < menu->nitems; ++i) {
	mi = menu->items + i;
	switch (mi->mitype) {
	    case MI_TYPE_BUTTON:
		mi->d.button.flags &= ~(MenuButtonClicked|MenuButtonActive|MenuButtonSelected);
		if (i == menu->defsel) {
		    mi->d.button.flags |= MenuButtonSelected;
		    MenuButtonCurSel = i;
		}
		// FIXME: Maybe activate if mouse-pointer is over it right now?
		break;
	    case MI_TYPE_PULLDOWN:
		mi->d.pulldown.flags &= ~(MenuButtonClicked|MenuButtonActive|MenuButtonSelected);
		mi->d.pulldown.cursel = 0;
		if (mi->d.pulldown.defopt != -1)
		    mi->d.pulldown.curopt = mi->d.pulldown.defopt;
		if (i == menu->defsel) {
		    mi->d.pulldown.flags |= MenuButtonSelected;
		    MenuButtonCurSel = i;
		}
		// FIXME: Maybe activate if mouse-pointer is over it right now?
		break;
	    default:
		break;
	}
    }
    MenuButtonUnderCursor = -1;
    if (Loop) {
	SetVideoSync();
	MustRedraw = 0;
    }
    DrawMenu(CurrentMenu);

    if (Loop) {
	for( ; CurrentMenu != -1 ; ) {
	    DebugLevel3("MustRedraw: 0x%08x\n",MustRedraw);
	    UpdateDisplay();
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
	VideoFree(MenuButtonGfx.Sprite);
    }
    last_race = race;
    file = MenuButtonGfx.File[race];
    buf = alloca(strlen(file) + 9 + 1);
    file = strcat(strcpy(buf, "graphic/"), file);
    MenuButtonGfx.Sprite = LoadSprite(file, 0, 144);
}

//@}
