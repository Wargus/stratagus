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
#include "map.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--	Prototypes for action handlers
----------------------------------------------------------------------------*/

local void GameMenuSave(void);
local void GameMenuEnd(void);
local void GameMenuReturn(void);

local void ScenSelectLBExit(Menuitem *mi);
local void ScenSelectLBInit(Menuitem *mi);
local unsigned char *ScenSelectLBRetrieve(Menuitem *mi, int i);
local void ScenSelectLBAction(Menuitem *mi, int i);
local void ScenSelectTPAction(Menuitem *mi, int i);
local void ScenSelectFolder(void);
local void ScenSelectInit(Menuitem *mi);	// master init
local void ScenSelectOk(void);
local void ScenSelectCancel(void);

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
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL,
	{ text:{ "Game Menu", MI_FLAGS_CENTERED} } },
    { MI_TYPE_BUTTON, 16, 40, MenuButtonDisabled, LargeFont, NULL, NULL,
	{ button:{ "Save (~<F11~>)", 106, 27, MBUTTON_GM_HALF, GameMenuSave, KeyCodeF11} } },
    { MI_TYPE_BUTTON, 16 + 12 + 106, 40, MenuButtonDisabled, LargeFont, NULL, NULL,
	{ button:{ "Load (~<F12~>)", 106, 27, MBUTTON_GM_HALF, NULL, KeyCodeF12} } },
    { MI_TYPE_BUTTON, 16, 40 + 36, MenuButtonDisabled, LargeFont, NULL, NULL,
	{ button:{ "Options (~<F5~>)", 224, 27, MBUTTON_GM_FULL, NULL, KeyCodeF5} } },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36, MenuButtonDisabled, LargeFont, NULL, NULL,
	{ button:{ "Help (~<F1~>)", 224, 27, MBUTTON_GM_FULL, NULL, KeyCodeF1} } },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36 + 36, MenuButtonDisabled, LargeFont, NULL, NULL,
	{ button:{ "Scenario ~!Objectives", 224, 27, MBUTTON_GM_FULL, NULL, 'o'} } },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36 + 36 + 36, 0, LargeFont, NULL, NULL,
	{ button:{ "~!End Scenario", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'e'} } },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL,
	{ button:{ "Return to Game (~<Esc~>)", 224, 27, MBUTTON_GM_FULL, GameMenuReturn, '\033'} } },
};

/**
**	Items for the Victory Menu
*/
local Menuitem VictoryMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL,
	{ text:{ "Congratulations!", MI_FLAGS_CENTERED} } },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL,
	{ text:{ "You are victorious!", MI_FLAGS_CENTERED} } },
    { MI_TYPE_BUTTON, 32, 90, MenuButtonSelected, LargeFont, NULL, NULL,
	{ button:{ "~!Victory", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'v'} } },
    { MI_TYPE_BUTTON, 32, 56, MenuButtonDisabled, LargeFont, NULL, NULL,
	{ button:{ "Save Game (~<F11~>)", 224, 27, MBUTTON_GM_FULL, NULL, KeyCodeF11} } },
};

/**
**	Items for the Lost Menu
*/
local Menuitem LostMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL,
	{ text:{ "You failed to", MI_FLAGS_CENTERED} } },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL,
	{ text:{ "achieve victory!", MI_FLAGS_CENTERED} } },
    { MI_TYPE_BUTTON, 32, 90, MenuButtonSelected, LargeFont, NULL, NULL,
	{ button:{ "~!OK", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'o'} } },
};

/**
**	Items for the SelectScen Menu
*/
local unsigned char *ssmtoptions[] = {
    "Freecraft scenario (cm)",
    "Foreign scenario (pud)"
};

local unsigned char *ssmsoptions[] = {
    "Any size",
    "32 x 32",
    "64 x 64",
    "96 x 96",
    "128 x 128",
};

local char ScenSelectPath[1024];
local char ScenSelectDisplayPath[1024];

local Menuitem ScenSelectMenuItems[] = {
    { MI_TYPE_TEXT, 176, 8, 0, LargeFont, ScenSelectInit, NULL,
	{ text:{ "Select scenario", MI_FLAGS_CENTERED} } },

    { MI_TYPE_LISTBOX, 24, 140, 0, GameFont, ScenSelectLBInit, ScenSelectLBExit,
	{ listbox:{ NULL, 288, 6*18, MBUTTON_PULLDOWN, ScenSelectLBAction, 0, 0, 0, 0, 6, 0,
		    (void *)ScenSelectLBRetrieve} } },

    { MI_TYPE_BUTTON, 48, 318, MenuButtonSelected, LargeFont, NULL, NULL,
	{ button:{ "OK", 106, 27, MBUTTON_GM_HALF, ScenSelectOk, 0} } },
    { MI_TYPE_BUTTON, 198, 318, 0, LargeFont, NULL, NULL,
	{ button:{ "Cancel", 106, 27, MBUTTON_GM_HALF, ScenSelectCancel, 0} } },

    { MI_TYPE_TEXT, 132, 40, 0, LargeFont, NULL, NULL,
	{ text:{ "Type:", MI_FLAGS_RALIGN} } },
    { MI_TYPE_PULLDOWN, 140, 40, 0, GameFont, NULL, NULL,
	{ pulldown:{ ssmtoptions, 192, 20, MBUTTON_PULLDOWN, ScenSelectTPAction, 2, 1, 1, 0} } },
    { MI_TYPE_TEXT, 132, 80, 0, LargeFont, NULL, NULL,
	{ text:{ "Map size:", MI_FLAGS_RALIGN} } },
    { MI_TYPE_PULLDOWN, 140, 80, MenuButtonDisabled, GameFont, NULL, NULL,
	{ pulldown:{ ssmsoptions, 192, 20, MBUTTON_PULLDOWN, NULL, 5, 0, 0, 0} } },
    { MI_TYPE_BUTTON, 22, 112, 0, GameFont, NULL, NULL,
	{ button:{ NULL, 36, 24, MBUTTON_FOLDER, ScenSelectFolder, 0} } },
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
	3, 9,
	ScenSelectMenuItems
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
	if (button != MBUTTON_FOLDER) {
	    DrawTextCentered(s+x+w/2,s+y+(font == GameFont ? 4 : 7),font,text);
	} else {
	    SetDefaultTextColors(nc,rc);
	    DrawText(x+44,y+6,font,text);
	}
    }
    if (flags&MenuButtonSelected) {
	if (flags&MenuButtonDisabled) {
	    VideoDrawRectangle(ColorGray,x,y,w,h);
	} else {
	    VideoDrawRectangle(ColorYellow,x,y,w,h);
	}
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
    unsigned flags = mi->flags;
    MenuButtonId rb = mi->d.pulldown.button;
    unsigned w, h, x, y, oh;
    x = mx+mi->xofs;
    y = my+mi->yofs;
    w = mi->d.pulldown.xsize;
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
		DrawText(x+2,y+2 + oh*i ,mi->font,text);
	    }
	}
	w += 2;
    } else {
	h = mi->d.pulldown.ysize;
	y = my+mi->yofs;
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
	    DrawText(x+2,y+2,mi->font,text);
	}
    }
    if (flags&MenuButtonSelected) {
	if (flags&MenuButtonDisabled) {
	    VideoDrawRectangle(ColorGray,x-2,y-2,w,h);
	} else {
	    VideoDrawRectangle(ColorYellow,x-2,y-2,w,h);
	}
    }
    SetDefaultTextColors(nc,rc);
}

/**
**	Draw listbox 'button' on menu mx, my
**
**	@param mi	menuitem pointer
**	@param mx	menu X display position (offset)
**	@param my	menu Y display position (offset)
*/
local void DrawListbox(Menuitem *mi, unsigned mx, unsigned my)
{
    int i, s, nc, rc;
    char *text;
    MenuButtonId rb = mi->d.listbox.button;
    unsigned flags = mi->flags;
    unsigned w, h, x, y;
    w = mi->d.listbox.xsize;
    h = mi->d.listbox.ysize;
    x = mx+mi->xofs;
    y = my+mi->yofs;

    GetDefaultTextColors(&nc, &rc);

    if (flags&MenuButtonDisabled) {
	rb--;
    } 
    i = mi->d.listbox.nlines;
    s = mi->d.listbox.startline;
    while (i--) {
	PushClipping();
	SetClipping(0,0,x+w,VideoHeight);
	VideoDrawClip(MenuButtonGfx.Sprite, rb, x-1, y-1 + 18*i);
	PopClipping();
	if (!(flags&MenuButtonDisabled)) {
	    if (i < mi->d.listbox.noptions) {
		text = (*mi->d.listbox.retrieveopt)(mi, i + s);
		if (text) {
		    if (i == mi->d.listbox.curopt)
			SetDefaultTextColors(rc,rc);
		    else
			SetDefaultTextColors(nc,rc);
		    DrawText(x+2,y+2 + 18*i ,mi->font,text);
		}
	    }
	}
    }

    if (flags&MenuButtonSelected) {
	if (flags&MenuButtonDisabled) {
	    VideoDrawRectangle(ColorGray,x-2,y-2,w+2,h+2);
	} else {
	    VideoDrawRectangle(ColorYellow,x-2,y-2,w+2,h+2);
	}
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
		if (mi->d.text.tflags&MI_FLAGS_CENTERED)
		    DrawTextCentered(menu->x+mi->xofs,menu->y+mi->yofs,
			    mi->font,mi->d.text.text);
		else if (mi->d.text.tflags&MI_FLAGS_RALIGN) {
		    l = TextLength(mi->font,mi->d.text.text);
		    DrawText(menu->x+mi->xofs-l,menu->y+mi->yofs,
			    mi->font,mi->d.text.text);
		} else
		    DrawText(menu->x+mi->xofs,menu->y+mi->yofs,
			    mi->font,mi->d.text.text);
		break;
	    case MI_TYPE_BUTTON:
		    DrawMenuButton(mi->d.button.button,mi->flags,
			    mi->d.button.xsize,mi->d.button.ysize,
			    menu->x+mi->xofs,menu->y+mi->yofs,
			    mi->font,mi->d.button.text);
		break;
	    case MI_TYPE_PULLDOWN:
		    DrawPulldown(mi,menu->x,menu->y);
		break;
	    case MI_TYPE_LISTBOX:
		    DrawListbox(mi,menu->x,menu->y);
		break;
	    default:
		break;
	}
	mi++;
    }
    InvalidateArea(menu->x,menu->y,menu->xsize,menu->ysize);
}

/*----------------------------------------------------------------------------
--	Button action handler and Init/Exit functions
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

local void ScenSelectInit(Menuitem *mi __attribute__((unused)) )
{
    strcpy(ScenSelectPath, FreeCraftLibPath);
    ScenSelectDisplayPath[0] = 0;
    ScenSelectMenuItems[8].flags = MenuButtonDisabled;
    ScenSelectMenuItems[8].d.button.text = NULL;
}

local void ScenSelectLBAction(Menuitem *mi, int i)
{
    FileList *fl;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    ScenSelectMenuItems[2].d.button.text = "OK";
	} else {
	    ScenSelectMenuItems[2].d.button.text = "Open";
	}
    }
}

local void ScenSelectLBExit(Menuitem *mi)
{
    if (mi->d.listbox.noptions) {
	free(mi->d.listbox.options);
	mi->d.listbox.options = NULL;
	mi->d.listbox.noptions = 0;
    }
}

local void ScenSelectLBInit(Menuitem *mi)
{
    char *suf;

    ScenSelectLBExit(mi);
    if (ScenSelectMenuItems[5].d.pulldown.curopt == 0)
	suf = ".cm";
    else
	suf = ".pud";
    mi->d.listbox.noptions = ReadDataDirectory(ScenSelectPath, suf, (FileList **)&(mi->d.listbox.options));
    // FIXME: Fill xdata here
    if (mi->d.listbox.noptions == 0) {
	ScenSelectMenuItems[2].d.button.text = "OK";
	ScenSelectMenuItems[2].flags |= MenuButtonDisabled;
    } else {
	ScenSelectLBAction(mi, 0);
	ScenSelectMenuItems[2].flags &= ~MenuButtonDisabled;
    }
}

local unsigned char *ScenSelectLBRetrieve(Menuitem *mi, int i)
{
    FileList *fl;
    static char buffer[1024];

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    strcpy(buffer, "   ");
	} else {
	    strcpy(buffer, "\260 ");
	}
	strcat(buffer, fl[i].name);
	return buffer;
    }
    return NULL;
}

local void ScenSelectTPAction(Menuitem *mi, int i __attribute__((unused)) )
{
    mi = ScenSelectMenuItems + 1;
    ScenSelectLBInit(mi);
    mi->d.listbox.cursel = -1;
    mi->d.listbox.startline = 0;
    mi->d.listbox.curopt = 0;
    MustRedraw |= RedrawMenu;
}

local void ScenSelectFolder(void)
{
    char *cp;
    Menuitem *mi = ScenSelectMenuItems + 1;

    if (ScenSelectDisplayPath[0]) {
	cp = strrchr(ScenSelectDisplayPath, '/');
	if (cp) {
	    *cp = 0;
	} else {
	    ScenSelectDisplayPath[0] = 0;
	    ScenSelectMenuItems[8].flags |= MenuButtonDisabled;
	    ScenSelectMenuItems[8].d.button.text = NULL;
	}
	cp = strrchr(ScenSelectPath, '/');
	if (cp) {
	    *cp = 0;
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    MustRedraw |= RedrawMenu;
	}
    }
}

local void ScenSelectOk(void)
{
    FileList *fl;
    char *cp;
    Menuitem *mi = ScenSelectMenuItems + 1;
    int i = mi->d.listbox.curopt + mi->d.listbox.startline;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    cp = strrchr(ScenSelectPath, '/');
	    if (!cp || cp[1]) {
		strcat(ScenSelectPath, "/");
	    }
	    strcat(ScenSelectPath, fl[i].name);
	    if (ScenSelectMenuItems[8].flags&MenuButtonDisabled) {
		ScenSelectMenuItems[8].flags &= ~MenuButtonDisabled;
		ScenSelectMenuItems[8].d.button.text = ScenSelectDisplayPath;
	    } else {
		strcat(ScenSelectDisplayPath, "/");
	    }
	    strcat(ScenSelectDisplayPath, fl[i].name);
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    MustRedraw |= RedrawMenu;
	} else {
	    if (ScenSelectPath[0]) {
		strcat(ScenSelectPath, "/");
	    }
	    strcat(ScenSelectPath, fl[i].name);		// Final map name with path
	    LoadMap(ScenSelectPath, &TheMap);
	    ScenSelectCancel();				// Not really, just end menu
	}
    }
}

local void ScenSelectCancel(void)
{
    InterfaceState=IfaceStateNormal;
    MustRedraw&=~RedrawMenu;
    CursorOn=CursorOnUnknown;
    CurrentMenu=-1;
    /// FIXME: restore mouse pointer to sane state (call fake mouse move?)
}

/*----------------------------------------------------------------------------
--	Menu operation functions
----------------------------------------------------------------------------*/


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
		if (!(mi->flags&MenuButtonClicked)) {
		    switch (mi->mitype) {
			case MI_TYPE_PULLDOWN:
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
			    if (mi->d.pulldown.action) {
				(*mi->d.pulldown.action)(mi, mi->d.pulldown.curopt);
			    }
			    break;
			case MI_TYPE_LISTBOX:
			    if (key == KeyCodeDown) {
				if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.pulldown.noptions) {
				    mi->d.listbox.curopt++;
				    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
					mi->d.listbox.curopt--;
					mi->d.listbox.startline++;
				    }
				} else {
				    break;
				}
			    } else {
				if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
				    mi->d.listbox.curopt--;
				    if (mi->d.listbox.curopt < 0) {
					mi->d.listbox.curopt++;
					mi->d.listbox.startline--;
				    }
				} else {
				    break;
				}
			    }
			    MustRedraw |= RedrawMenu;
			    if (mi->d.listbox.action) {
				(*mi->d.listbox.action)(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			    }
			    break;
			default:
			    break;
		    }
		}
	    }
	    break;
	case 9: 				/// TAB			// FIXME: Add Shift-TAB
	    if (MenuButtonCurSel != -1 && !(menu->items[MenuButtonCurSel].flags&MenuButtonClicked)) {
		n = menu->nitems;
		for (i = 0; i < n; ++i) {
		    mi = menu->items + ((MenuButtonCurSel + i + 1) % n);
		    switch (mi->mitype) {
			case MI_TYPE_TEXT:
			    break;
			case MI_TYPE_LISTBOX:
			case MI_TYPE_PULLDOWN:
			case MI_TYPE_BUTTON:
			    if (mi->flags & MenuButtonDisabled) {
				break;
			    }
			    mi->flags |= MenuButtonSelected;
			    menu->items[MenuButtonCurSel].flags &= ~MenuButtonSelected;
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
	if (!(mi->flags&MenuButtonDisabled)) {
	    switch (mi->mitype) {
		case MI_TYPE_BUTTON:
		    xs = menu->x + mi->xofs;
		    ys = menu->y + mi->yofs;
		    if (x < xs || x > xs + mi->d.button.xsize || y < ys || y > ys + mi->d.button.ysize) {
			if (!(mi->flags&MenuButtonClicked)) {
			    if (mi->flags&MenuButtonActive) {
				RedrawFlag = 1;
				mi->flags &= ~MenuButtonActive;
			    }
			}
			continue;
		    }
		    break;
		case MI_TYPE_PULLDOWN:
		    xs = menu->x + mi->xofs;
		    if (mi->flags&MenuButtonClicked) {
			ys = menu->y + mi->yofs;
			h = mi->d.pulldown.ysize - 2;
			ys -= mi->d.pulldown.curopt * h;
			if (x<xs || x>xs + mi->d.pulldown.xsize || y<ys || y>ys + h*mi->d.pulldown.noptions) {
			    continue;
			}
			j = (y - ys) / h;
			if (j != mi->d.pulldown.cursel) {
			    mi->d.pulldown.cursel = j;
			    RedrawFlag = 1;
			    if (mi->d.pulldown.action) {
				(*mi->d.pulldown.action)(mi, mi->d.pulldown.cursel);
			    }
			}
		    } else {
			ys = menu->y + mi->yofs;
			if (x<xs || x>xs + mi->d.pulldown.xsize || y<ys || y>ys + mi->d.pulldown.ysize) {
			    if (!(mi->flags&MenuButtonClicked)) {
				if (mi->flags&MenuButtonActive) {
				    RedrawFlag = 1;
				    mi->flags &= ~MenuButtonActive;
				}
			    }
			    continue;
			}
		    }
		    break;
		case MI_TYPE_LISTBOX:
		    xs = menu->x + mi->xofs;
		    ys = menu->y + mi->yofs;
		    if (x < xs || x > xs + mi->d.listbox.xsize || y < ys || y > ys + mi->d.listbox.ysize) {
			if (!(mi->flags&MenuButtonClicked)) {
			    if (mi->flags&MenuButtonActive) {
				RedrawFlag = 1;
				mi->flags &= ~MenuButtonActive;
			    }
			}
			continue;
		    }
		    j = (y - ys) / 18;
		    if (j != mi->d.listbox.cursel) {
			mi->d.listbox.cursel = j;	// just store for click
		    }
		    break;
		default:
		    break;
	    }
	    switch (mi->mitype) {
		case MI_TYPE_BUTTON:
		case MI_TYPE_PULLDOWN:
		case MI_TYPE_LISTBOX:
		    if (!(mi->flags&MenuButtonActive)) {
			RedrawFlag = 1;
			mi->flags |= MenuButtonActive;
		    }
		    DebugLevel3("On menu item %d\n", i);
		    MenuButtonUnderCursor = i;
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
	    if (!(mi->flags&MenuButtonClicked)) {
		switch (mi->mitype) {
		    case MI_TYPE_BUTTON:
		    case MI_TYPE_PULLDOWN:
		    case MI_TYPE_LISTBOX:
			if (MenuButtonCurSel != -1) {
			    menu->items[MenuButtonCurSel].flags &= ~MenuButtonSelected;
			}
			MenuButtonCurSel = MenuButtonUnderCursor;
			mi->flags |= MenuButtonClicked|MenuButtonSelected;
			MustRedraw |= RedrawMenu;
		    default:
			break;
		}
	    }
	    switch (mi->mitype) {
		case MI_TYPE_PULLDOWN:
		    mi->d.pulldown.cursel = mi->d.pulldown.curopt;
		    break;
		case MI_TYPE_LISTBOX:
		    if (mi->d.listbox.cursel != mi->d.listbox.curopt) {
			mi->d.listbox.curopt = mi->d.listbox.cursel;
			if (mi->d.listbox.action) {
			    (*mi->d.listbox.action)(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			}
		    }
		    break;
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
		    if (mi->flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->flags &= ~MenuButtonClicked;
			if (MenuButtonUnderCursor == i) {
			    MenuButtonUnderCursor = -1;
			    if (mi->d.button.handler) {
				(*mi->d.button.handler)();
			    }
			}
		    }
		    break;
		case MI_TYPE_PULLDOWN:
		    if (mi->flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->flags &= ~MenuButtonClicked;
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
		case MI_TYPE_LISTBOX:
		    if (mi->flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->flags &= ~MenuButtonClicked;
			// MAYBE ADD HERE
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
	    case MI_TYPE_PULLDOWN:
	    case MI_TYPE_LISTBOX:
		mi->flags &= ~(MenuButtonClicked|MenuButtonActive|MenuButtonSelected);
		// FIXME: Maybe activate if mouse-pointer is over it right now?
		if (i == menu->defsel) {
		    mi->flags |= MenuButtonSelected;
		    MenuButtonCurSel = i;
		}
		break;
	}
	switch (mi->mitype) {
	    case MI_TYPE_PULLDOWN:
		mi->d.pulldown.cursel = 0;
		if (mi->d.pulldown.defopt != -1)
		    mi->d.pulldown.curopt = mi->d.pulldown.defopt;
		break;
	    case MI_TYPE_LISTBOX:
		mi->d.listbox.cursel = -1;
		mi->d.listbox.startline = 0;
		if (mi->d.listbox.defopt != -1)
		    mi->d.listbox.curopt = mi->d.listbox.defopt;
		break;
	    default:
		break;
	}
	if (mi->initfunc) {
	    (*mi->initfunc)(mi);
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

    for (i = 0; i < menu->nitems; ++i) {
	mi = menu->items + i;
	if (mi->exitfunc) {
	    (*mi->exitfunc)(mi);	// action/destructor
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
