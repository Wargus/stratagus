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
//
//	(c) Copyright 1999-2001 by Andreas Arens
//
//	$Id$

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
#include "pud.h"
#include "iolib.h"
#include "network.h"
#include "settings.h"

/*----------------------------------------------------------------------------
--	Prototypes for local functions
----------------------------------------------------------------------------*/

local void EndMenu(void);

/*----------------------------------------------------------------------------
--	Prototypes for action handlers and helper functions
----------------------------------------------------------------------------*/

local void GameMenuSave(void);
local void GameMenuEnd(void);
local void GameMenuReturn(void);

local void StartMenusSetBackground(Menuitem *mi);
local void NameLineDrawFunc(Menuitem *mi);
local void EnterNameAction(Menuitem *mi, int key);
local void EnterNameCancel(void);

local void JoinNetGameMenu(void);
local void CreateNetGameMenu(void);

local void SinglePlayerGameMenu(void);
local void MultiPlayerGameMenu(void);
local void ScenSelectMenu(void);

local void ScenSelectLBExit(Menuitem *mi);
local void ScenSelectLBInit(Menuitem *mi);
local unsigned char *ScenSelectLBRetrieve(Menuitem *mi, int i);
local void ScenSelectLBAction(Menuitem *mi, int i);
local void ScenSelectTPMSAction(Menuitem *mi, int i);
local void ScenSelectVSAction(Menuitem *mi, int i);
local void ScenSelectFolder(void);
local void ScenSelectInit(Menuitem *mi);	// master init
local void ScenSelectOk(void);
local void ScenSelectCancel(void);

local void GameSetupInit(Menuitem *mi);	// master init
local void GameDrawFunc(Menuitem *mi);

local void GameRCSAction(Menuitem *mi, int i);
local void GameRESAction(Menuitem *mi, int i);
local void GameUNSAction(Menuitem *mi, int i);
local void GameTSSAction(Menuitem *mi, int i);

local void CustomGameCancel(void);
local void CustomGameStart(void);
local void CustomGameOPSAction(Menuitem *mi, int i);

local void MultiGameSetupInit(Menuitem *mi);	// master init
local void MultiGameDrawFunc(Menuitem *mi);
local void MultiGameFWSAction(Menuitem *mi, int i);
local void NetMultiPlayerDrawFunc(Menuitem *mi);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /// Name, Version, Copyright
extern char NameLine[];

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
**	Offsets from top and left, used for different resolutions
*/
local int OffsetX = 0;
local int OffsetY = 0;

/**
**	Items for the Game Menu
*/
local Menuitem GameMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL,
	{ text:{ "Game Menu", MI_TFLAGS_CENTERED} } },
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
	{ text:{ "Congratulations!", MI_TFLAGS_CENTERED} } },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL,
	{ text:{ "You are victorious!", MI_TFLAGS_CENTERED} } },
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
	{ text:{ "You failed to", MI_TFLAGS_CENTERED} } },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL,
	{ text:{ "achieve victory!", MI_TFLAGS_CENTERED} } },
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
local char ScenSelectFileName[128];
local int CustomGameStarted = 0;
local PudInfo *ScenSelectPudInfo = NULL;

local Menuitem ScenSelectMenuItems[] = {
    { MI_TYPE_TEXT, 176, 8, 0, LargeFont, ScenSelectInit, NULL,
	{ text:{ "Select scenario", MI_TFLAGS_CENTERED} } },

    { MI_TYPE_LISTBOX, 24, 140, 0, GameFont, ScenSelectLBInit, ScenSelectLBExit,
	{ listbox:{ NULL, 288, 6*18, MBUTTON_PULLDOWN, ScenSelectLBAction, 0, 0, 0, 0, 6, 0,
		    (void *)ScenSelectLBRetrieve, ScenSelectOk} } },
    { MI_TYPE_VSLIDER, 312, 140, 0, 0, NULL, NULL,
	{ vslider:{ 0, 18, 6*18, ScenSelectVSAction, -1, 0, 0, 0, ScenSelectOk} } },

    { MI_TYPE_BUTTON, 48, 318, MenuButtonSelected, LargeFont, NULL, NULL,
	{ button:{ "OK", 106, 27, MBUTTON_GM_HALF, ScenSelectOk, 0} } },
    { MI_TYPE_BUTTON, 198, 318, 0, LargeFont, NULL, NULL,
	{ button:{ "Cancel", 106, 27, MBUTTON_GM_HALF, ScenSelectCancel, 0} } },

    { MI_TYPE_TEXT, 132, 40, 0, LargeFont, NULL, NULL,
	{ text:{ "Type:", MI_TFLAGS_RALIGN} } },
    { MI_TYPE_PULLDOWN, 140, 40, 0, GameFont, NULL, NULL,
	{ pulldown:{ ssmtoptions, 192, 20, MBUTTON_PULLDOWN, ScenSelectTPMSAction, 2, 1, 1, 0} } },
    { MI_TYPE_TEXT, 132, 80, 0, LargeFont, NULL, NULL,
	{ text:{ "Map size:", MI_TFLAGS_RALIGN} } },
    { MI_TYPE_PULLDOWN, 140, 80, 0, GameFont, NULL, NULL,
	{ pulldown:{ ssmsoptions, 192, 20, MBUTTON_PULLDOWN, ScenSelectTPMSAction, 5, 0, 0, 0} } },
    { MI_TYPE_BUTTON, 22, 112, 0, GameFont, NULL, NULL,
	{ button:{ NULL, 36, 24, MBUTTON_FOLDER, ScenSelectFolder, 0} } },
};

/**
**	Items for the Prg Start Menu
*/
local Menuitem PrgStartMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, NULL, NULL,
	{ drawfunc:{ NameLineDrawFunc } } },
    { MI_TYPE_BUTTON, 208, 320, 0, LargeFont, StartMenusSetBackground, NULL,
	{ button:{ "~!Single Player Game", 224, 27, MBUTTON_GM_FULL, SinglePlayerGameMenu, 's'} } },
    { MI_TYPE_BUTTON, 208, 320 + 36, MenuButtonDisabled, LargeFont, NULL, NULL,
	{ button:{ "~!Multi Player Game", 224, 27, MBUTTON_GM_FULL, MultiPlayerGameMenu, 'm'} } },
    { MI_TYPE_BUTTON, 208, 320 + 36 + 36, 0, LargeFont, NULL, NULL,
	{ button:{ "E~!xit Program", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'x'} } },
};

/**
**	Items for the Custom Game Setup Menu
*/
local unsigned char *rcsoptions[] = {
    "Human",
    "Orc",
    "Map Default",
};

local unsigned char *resoptions[] = {
    "Map Default",
    "Low",
    "Medium",
    "High",
};

local unsigned char *unsoptions[] = {
    "Map Default",
    "One Peasant Only",
};

local unsigned char *tssoptions[] = {
    "Map Default",
    "Forest",
    "Winter",
    "Wasteland",
    "Orc Swamp",	// hmm. need flag if XPN-GFX is present! - tmp-hack in InitMenus()..
			// Some time later _all_ tilesets should be a dynamic (ccl-defineable) resource..
};

local unsigned char *cgopsoptions[] = {
    "Map Default",
    "1 Opponent",
    "2 Opponents",
    "3 Opponents",
    "4 Opponents",
    "5 Opponents",
    "6 Opponents",
    "7 Opponents",
};

local unsigned char *mgfwsoptions[] = {
    "On",
    "Off",
};

local unsigned char *mgptsoptions[] = {
    "Available",
    "Computer",
    "Closed",
};

local Menuitem CustomGameMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, GameSetupInit, NULL,
	{ drawfunc:{ GameDrawFunc } } },
    { MI_TYPE_TEXT, 640/2+12, 192, 0, LargeFont, NULL, NULL,
	{ text:{ "~<Single Player Game Setup~>", MI_TFLAGS_CENTERED} } },
    { MI_TYPE_BUTTON, 640-224-16, 360, 0, LargeFont, NULL, NULL,
	{ button:{ "S~!elect Scenario", 224, 27, MBUTTON_GM_FULL, ScenSelectMenu, 'e'} } },
    { MI_TYPE_BUTTON, 640-224-16, 360+36, 0, LargeFont, NULL, NULL,
	{ button:{ "~!Start Game", 224, 27, MBUTTON_GM_FULL, CustomGameStart, 's'} } },
    { MI_TYPE_BUTTON, 640-224-16, 360+36+36, 0, LargeFont, NULL, NULL,
	{ button:{ "~!Cancel Game", 224, 27, MBUTTON_GM_FULL, CustomGameCancel, 'c'} } },
    { MI_TYPE_TEXT, 40, 10+240-20, 0, GameFont, NULL, NULL,
	{ text:{ "~<Your Race:~>", 0} } },
    { MI_TYPE_PULLDOWN, 40, 10+240, 0, GameFont, NULL, NULL,
	{ pulldown:{ rcsoptions, 152, 20, MBUTTON_PULLDOWN, GameRCSAction, 3, 2, 2, 0} } },
    { MI_TYPE_TEXT, 220, 10+240-20, 0, GameFont, NULL, NULL,
	{ text:{ "~<Resources:~>", 0} } },
    { MI_TYPE_PULLDOWN, 220, 10+240, 0, GameFont, NULL, NULL,
	{ pulldown:{ resoptions, 152, 20, MBUTTON_PULLDOWN, GameRESAction, 4, 0, 0, 0} } },
    { MI_TYPE_TEXT, 640-224-16, 10+240-20, 0, GameFont, NULL, NULL,
	{ text:{ "~<Units:~>", 0} } },
    { MI_TYPE_PULLDOWN, 640-224-16, 10+240, 0, GameFont, NULL, NULL,
	{ pulldown:{ unsoptions, 190, 20, MBUTTON_PULLDOWN, GameUNSAction, 2, 0, 0, 0} } },
    { MI_TYPE_TEXT, 40, 10+300-20, 0, GameFont, NULL, NULL,
	{ text:{ "~<Opponents:~>", 0} } },
    { MI_TYPE_PULLDOWN, 40, 10+300, 0, GameFont, NULL, NULL,
	{ pulldown:{ cgopsoptions, 152, 20, MBUTTON_PULLDOWN, CustomGameOPSAction, 8, 0, 0, 0} } },
    { MI_TYPE_TEXT, 220, 10+300-20, 0, GameFont, NULL, NULL,
	{ text:{ "~<Map Tileset:~>", 0} } },
    { MI_TYPE_PULLDOWN, 220, 10+300, 0, GameFont, NULL, NULL,
	{ pulldown:{ tssoptions, 152, 20, MBUTTON_PULLDOWN, GameTSSAction, 5, 0, 0, 0} } },
};

/**
**	Items for the Enter Name Menu
*/
local Menuitem EnterNameMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, GameFont, NULL, NULL,
	{ text:{ "Enter your name:", MI_TFLAGS_CENTERED} } },
    { MI_TYPE_INPUT, 40, 38, 0, GameFont, NULL, NULL,
	{ input:{ NULL, 212, 20, MBUTTON_PULLDOWN, EnterNameAction, 0, 0} } },
    { MI_TYPE_BUTTON, 24, 80, MenuButtonSelected, LargeFont, NULL, NULL,
	{ button:{ "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'} } },
    { MI_TYPE_BUTTON, 154, 80, 0, LargeFont, NULL, NULL,
	{ button:{ "~!Cancel", 106, 27, MBUTTON_GM_HALF, EnterNameCancel, 'c'} } },
};

/**
**	Items for the Net Create Join Menu
*/
local Menuitem NetCreateJoinMenuItems[] = {
    { MI_TYPE_BUTTON, 208, 320, 0, LargeFont, StartMenusSetBackground, NULL,
	{ button:{ "~!Join Game", 224, 27, MBUTTON_GM_FULL, JoinNetGameMenu, 'j'} } },
    { MI_TYPE_BUTTON, 208, 320 + 36, 0, LargeFont, NULL, NULL,
	{ button:{ "~!Create Game", 224, 27, MBUTTON_GM_FULL, CreateNetGameMenu, 'c'} } },
    { MI_TYPE_BUTTON, 208, 320 + 36 + 36, 0, LargeFont, NULL, NULL,
	{ button:{ "~!Previous Menu", 224, 27, MBUTTON_GM_FULL, EndMenu, 'p'} } },
};


/**
**	Items for the Net Multiplayer Setup Menu
*/
local Menuitem NetMultiButtonStorage[] = {
    { MI_TYPE_PULLDOWN, 40, 32, 0, GameFont, NULL, NULL,
	{ pulldown:{ mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0} } },
    { MI_TYPE_DRAWFUNC, 40, 32, 0, GameFont, NULL, NULL,
	{ drawfunc:{ NetMultiPlayerDrawFunc } } },
};

local Menuitem NetMultiSetupMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, MultiGameSetupInit, NULL,
	{ drawfunc:{ MultiGameDrawFunc } } },
    { MI_TYPE_TEXT, 640/2+12, 8, 0, LargeFont, NULL, NULL,
	{ text:{ "~<Multi Player Setup~>", MI_TFLAGS_CENTERED} } },
    { MI_TYPE_BUTTON, 640-224-16, 360, 0, LargeFont, NULL, NULL,
	{ button:{ "S~!elect Scenario", 224, 27, MBUTTON_GM_FULL, ScenSelectMenu, 'e'} } },
    { MI_TYPE_BUTTON, 640-224-16, 360+36, MenuButtonDisabled, LargeFont, NULL, NULL,
	{ button:{ "~!Start Game", 224, 27, MBUTTON_GM_FULL, CustomGameStart, 's'} } },
    { MI_TYPE_BUTTON, 640-224-16, 360+36+36, 0, LargeFont, NULL, NULL,
	{ button:{ "~!Cancel Game", 224, 27, MBUTTON_GM_FULL, CustomGameCancel, 'c'} } },

    { MI_TYPE_PULLDOWN, 40, 32, 0, GameFont, NULL, NULL,
	{ pulldown:{ mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0} } },
    { MI_TYPE_PULLDOWN, 40, 32+22, 0, GameFont, NULL, NULL,
	{ pulldown:{ mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0} } },
    { MI_TYPE_PULLDOWN, 40, 32+22*2, 0, GameFont, NULL, NULL,
	{ pulldown:{ mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0} } },
    { MI_TYPE_PULLDOWN, 40, 32+22*3, 0, GameFont, NULL, NULL,
	{ pulldown:{ mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0} } },
    { MI_TYPE_PULLDOWN, 40, 32+22*4, 0, GameFont, NULL, NULL,
	{ pulldown:{ mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0} } },
    { MI_TYPE_PULLDOWN, 40, 32+22*5, 0, GameFont, NULL, NULL,
	{ pulldown:{ mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0} } },
    { MI_TYPE_PULLDOWN, 40, 32+22*6, 0, GameFont, NULL, NULL,
	{ pulldown:{ mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0} } },
    { MI_TYPE_PULLDOWN, 40, 32+22*7, 0, GameFont, NULL, NULL,
	{ pulldown:{ mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0} } },

    { MI_TYPE_TEXT, 40, 10+240-20, 0, GameFont, NULL, NULL,
	{ text:{ "~<Your Race:~>", 0} } },
    { MI_TYPE_PULLDOWN, 40, 10+240, 0, GameFont, NULL, NULL,
	{ pulldown:{ rcsoptions, 152, 20, MBUTTON_PULLDOWN, GameRCSAction, 3, 2, 2, 0} } },
    { MI_TYPE_TEXT, 220, 10+240-20, 0, GameFont, NULL, NULL,
	{ text:{ "~<Resources:~>", 0} } },
    { MI_TYPE_PULLDOWN, 220, 10+240, 0, GameFont, NULL, NULL,
	{ pulldown:{ resoptions, 152, 20, MBUTTON_PULLDOWN, GameRESAction, 4, 0, 0, 0} } },
    { MI_TYPE_TEXT, 640-224-16, 10+240-20, 0, GameFont, NULL, NULL,
	{ text:{ "~<Units:~>", 0} } },
    { MI_TYPE_PULLDOWN, 640-224-16, 10+240, 0, GameFont, NULL, NULL,
	{ pulldown:{ unsoptions, 190, 20, MBUTTON_PULLDOWN, GameUNSAction, 2, 0, 0, 0} } },
    { MI_TYPE_TEXT, 40, 10+300-20, 0, GameFont, NULL, NULL,
	{ text:{ "~<Fog of War:~>", 0} } },
    { MI_TYPE_PULLDOWN, 40, 10+300, 0, GameFont, NULL, NULL,
	{ pulldown:{ mgfwsoptions, 152, 20, MBUTTON_PULLDOWN, MultiGameFWSAction, 2, 0, 0, 0} } },
    { MI_TYPE_TEXT, 220, 10+300-20, 0, GameFont, NULL, NULL,
	{ text:{ "~<Map Tileset:~>", 0} } },
    { MI_TYPE_PULLDOWN, 220, 10+300, 0, GameFont, NULL, NULL,
	{ pulldown:{ tssoptions, 152, 20, MBUTTON_PULLDOWN, GameTSSAction, 5, 0, 0, 0} } },
};


/**
**	Menus
*/
global Menu Menus[] = {
    {
	// Game Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	7, 8,
	GameMenuItems
    },
    {
	// Victory Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-128)/2,
	288, 128,
	ImagePanel4,
	2, 4,
	VictoryMenuItems
    },
    {
	// Lost Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-128)/2,
	288, 128,
	ImagePanel4,
	2, 3,
	LostMenuItems
    },
    {
	// SelectScen Menu
	(640-352)/2,
	(480-352)/2,
	352, 352,
	ImagePanel5,
	4, 10,
	ScenSelectMenuItems
    },
    {
	// PrgStart Menu
	0,
	0,
	640, 480,
	ImageNone,
	1, 4,
	PrgStartMenuItems
    },
    {
	// CustomGame Menu
	0,
	0,
	640, 480,
	ImageNone,
	3, 15,
	CustomGameMenuItems
    },
    {
	// Enter Name Menu
	(640-288)/2,
	260,
	288, 128,
	ImagePanel4,
	2, 4,
	EnterNameMenuItems
    },
    {
	// Net Create Join Menu
	0,
	0,
	640, 480,
	ImageNone,
	2, 3,
	NetCreateJoinMenuItems
    },
    {
	// Multi Player Setup Menu
	0,
	0,
	640, 480,
	ImageNone,
	3, 23,
	NetMultiSetupMenuItems
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
    if (rb < MenuButtonGfx.Sprite->NumFrames) {
	VideoDraw(MenuButtonGfx.Sprite, rb, x, y);
    } else {
	if (rb < button) {
	    VideoDrawRectangleClip(ColorGray,x+1,y+1,w-2,h-2);
	    VideoDrawRectangleClip(ColorGray,x+2,y+2,w-4,h-4);
	} else {
	    // FIXME: Temp-workaround for missing folder button in non-expansion gfx
	    VideoDrawRectangleClip(ColorYellow,x+1,y+1,w-2,h-2);
	    VideoDrawRectangleClip(ColorYellow,x+2,y+2,w-4,h-4);
	}
    }
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
	    VideoDrawRectangleClip(ColorGray,x,y,w-1,h);
	} else {
	    VideoDrawRectangleClip(ColorYellow,x,y,w-1,h);
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
	    SetClipping(0,0,x+w,VideoHeight-1);
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
	SetClipping(0,0,x+w-20,VideoHeight-1);
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
	    VideoDrawRectangleClip(ColorGray,x-2,y-2,w,h);
	} else {
	    VideoDrawRectangleClip(ColorYellow,x-2,y-2,w,h);
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
	SetClipping(0,0,x+w,VideoHeight-1);
	VideoDrawClip(MenuButtonGfx.Sprite, rb, x-1, y-1 + 18*i);
	PopClipping();
	if (!(flags&MenuButtonDisabled)) {
	    if (i < mi->d.listbox.noptions) {
		SetDefaultTextColors(nc,rc);
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
	    VideoDrawRectangleClip(ColorGray,x-2,y-2,w+1,h+2);
	} else {
	    VideoDrawRectangleClip(ColorYellow,x-2,y-2,w+1,h+2);
	}
    }
    SetDefaultTextColors(nc,rc);
}

/**
**	Draw vslider 'button' on menu mx, my
**
**	@param mi	menuitem pointer
**	@param mx	menu X display position (offset)
**	@param my	menu Y display position (offset)
*/
local void DrawVSlider(Menuitem *mi, unsigned mx, unsigned my)
{
    int p;
    unsigned flags = mi->flags;
    unsigned w, h, x, y;
    w = mi->d.vslider.xsize;
    h = mi->d.vslider.ysize;
    x = mx+mi->xofs;
    y = my+mi->yofs;

    if (flags&MenuButtonDisabled) {
	PushClipping();
	SetClipping(0,0,VideoWidth-1,y + h - 20);
	VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_VCONT - 1, x, y - 2);
	PopClipping();
	VideoDraw(MenuButtonGfx.Sprite, MBUTTON_UP_ARROW - 1, x, y - 2);
	VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW - 1, x, y + h - 20);
    } else {
	PushClipping();
	SetClipping(0,0,VideoWidth-1,y + h - 20);
	VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_VCONT, x, y - 2);
	PopClipping();
	if (mi->d.vslider.cflags&MI_CFLAGS_UP) {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_UP_ARROW + 1, x, y - 2);
	} else {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_UP_ARROW, x, y - 2);
	}
	if (mi->d.vslider.cflags&MI_CFLAGS_DOWN) {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW + 1, x, y + h - 20);
	} else {
	    VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW, x, y + h - 20);
	}
	p = (mi->d.vslider.percent * (h - 54)) / 100;
	VideoDraw(MenuButtonGfx.Sprite, MBUTTON_S_KNOB, x + 1, y + 18 + p);
    }

    if (flags&MenuButtonSelected) {
	if (flags&MenuButtonDisabled) {
	    VideoDrawRectangleClip(ColorGray,x,y-2,w,h+2);
	} else {
	    VideoDrawRectangleClip(ColorYellow,x,y-2,w,h+2);
	}
    }

}

/**
**	Draw input 'button' on menu mx, my
**
**	@param mi	menuitem pointer
**	@param mx	menu X display position (offset)
**	@param my	menu Y display position (offset)
*/
local void DrawInput(Menuitem *mi, unsigned mx, unsigned my)
{
    int nc, rc;
    char *text;
    unsigned flags = mi->flags;
    MenuButtonId rb = mi->d.input.button;
    unsigned w, h, x, y;

    x = mx+mi->xofs;
    y = my+mi->yofs;
    w = mi->d.input.xsize;
    h = mi->d.input.ysize;

    GetDefaultTextColors(&nc, &rc);
    if (flags&MenuButtonDisabled) {
	rb--;
	SetDefaultTextColors(FontGrey,FontGrey);
    }

    PushClipping();
    SetClipping(0,0,x+w,VideoHeight-1);
    VideoDrawClip(MenuButtonGfx.Sprite, rb, x-1, y-1);
    PopClipping();
    text = mi->d.input.buffer;
    if (text) {
	DrawText(x+2,y+2,mi->font,text);
    }
    if (flags&MenuButtonSelected) {
	if (flags&MenuButtonDisabled) {
	    VideoDrawRectangleClip(ColorGray,x-2,y-2,w,h);
	} else {
	    VideoDrawRectangleClip(ColorYellow,x-2,y-2,w,h);
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
    Menuitem *mi, *mip;

    if (MenuId == -1) {
	return;
    }
    menu = Menus + MenuId;
    if (menu->image != ImageNone) {
	DrawImage(menu->image,0,0,menu->x,menu->y);
    }
    n = menu->nitems;
    mi = menu->items;
    mip = NULL;
    for (i = 0; i < n; i++) {
	switch (mi->mitype) {
	    case MI_TYPE_TEXT:
		if (mi->d.text.tflags&MI_TFLAGS_CENTERED)
		    DrawTextCentered(menu->x+mi->xofs,menu->y+mi->yofs,
			    mi->font,mi->d.text.text);
		else if (mi->d.text.tflags&MI_TFLAGS_RALIGN) {
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
		if (mi->flags&MenuButtonClicked) {
		    mip = mi;	// Delay, due to possible overlaying!
		} else {
		    DrawPulldown(mi,menu->x,menu->y);
		}
		break;
	    case MI_TYPE_LISTBOX:
		DrawListbox(mi,menu->x,menu->y);
		break;
	    case MI_TYPE_VSLIDER:
		DrawVSlider(mi,menu->x,menu->y);
		break;
	    case MI_TYPE_DRAWFUNC:
		if (mi->d.drawfunc.draw) {
		    (*mi->d.drawfunc.draw)(mi);
		}
		break;
	    case MI_TYPE_INPUT:
		DrawInput(mi,menu->x,menu->y);
		break;
	    default:
		break;
	}
	mi++;
    }
    if (mip) {
	DrawPulldown(mip,menu->x,menu->y);
    }
    InvalidateArea(menu->x,menu->y,menu->xsize,menu->ysize);
}

/*----------------------------------------------------------------------------
--	Button action handler and Init/Exit functions
----------------------------------------------------------------------------*/

local void StartMenusSetBackground(Menuitem *mi __attribute__((unused)))
{
    HideCursor();
    DestroyCursorBackground();
    DisplayPicture("graphic/interface/Menu background without title.png");
}

local void NameLineDrawFunc(Menuitem *mi __attribute__((unused)))
{
    int nc, rc;

    GetDefaultTextColors(&nc, &rc);
    StartMenusSetBackground(mi);
    SetDefaultTextColors(rc, rc);
    DrawTextCentered(VideoWidth/2, OffsetY + 440, GameFont, NameLine);
    SetDefaultTextColors(nc, rc);
}

local void GameMenuReturn(void)
{
    EndMenu();
    MustRedraw &= ~RedrawMenu;
    InterfaceState = IfaceStateNormal;
    ClearStatusLine();
    MarkDrawEntireMap(); //FIXME: some tiles could be left as they were?
    GamePaused = 0;
}

local void GameMenuSave(void)
{
    SaveAll();	// FIXME: Sample code
}

local void GameMenuEnd(void)
{
    Exit(0);
}

local void ScenSelectMenu(void)
{
    int i;

    ProcessMenu(MENU_SCEN_SELECT, 1);
    // StartMenusSetBackground(NULL);
    FreePudInfo(ScenSelectPudInfo);
    ScenSelectPudInfo = NULL;
    if (ScenSelectPath[0]) {
	i = strlen(ScenSelectPath);
	strcat(ScenSelectPath, "/");
    } else {
	i = 0;
    }
    strcat(ScenSelectPath, ScenSelectFileName);		// Final map name with path
    if (strstr(ScenSelectFileName, ".pud")) {
	ScenSelectPudInfo = GetPudInfo(ScenSelectPath);
    } else {
	// FIXME: GetCmInfo();
    }
    ScenSelectPath[i] = 0;
}

local void SinglePlayerGameMenu(void)
{
    DestroyCursorBackground();
    CustomGameStarted = 0;
    ProcessMenu(MENU_CUSTOM_GAME_SETUP, 1);
    if (CustomGameStarted) {
	GameMenuReturn();
    }
}

local void EnterNameCancel(void)
{
    EnterNameMenuItems[1].d.input.nch = 0;
    EndMenu();
}

local void EnterNameAction(Menuitem *mi, int key)
{
    if (mi->d.input.nch == 0) {
	mi[1].flags = MenuButtonDisabled;
    } else {
	mi[1].flags &= ~MenuButtonDisabled;
	if (key == 10 || key == 13) {
	    EndMenu();
	}
    }
}

local void JoinNetGameMenu(void)
{
    DestroyCursorBackground();
    CustomGameStarted = 0;
    ProcessMenu(MENU_CUSTOM_GAME_SETUP, 1);
    if (CustomGameStarted) {
	GameMenuReturn();
    }
}

local void CreateNetGameMenu(void)
{
    DestroyCursorBackground();
    CustomGameStarted = 0;
    ProcessMenu(MENU_NET_MULTI_SETUP, 1);
    if (CustomGameStarted) {
	GameMenuReturn();
    }
}

local void MultiPlayerGameMenu(void)
{
    char NameBuf[32];

    StartMenusSetBackground(NULL);
    Invalidate();
    EnterNameMenuItems[1].d.input.buffer = NameBuf;
    strcpy(NameBuf, "Anonymous~!_");
    EnterNameMenuItems[1].d.input.nch = strlen(NameBuf) - 3;
    EnterNameMenuItems[1].d.input.maxch = 15;
    ProcessMenu(MENU_ENTER_NAME, 1);
    DestroyCursorBackground();
    StartMenusSetBackground(NULL);
    if (EnterNameMenuItems[1].d.input.nch == 0) {
	return;
    }

    NameBuf[EnterNameMenuItems[1].d.input.nch] = 0;	// Now finally here is the name
    strcpy(NetworkName, NameBuf);
    // Here we really go...
    ProcessMenu(MENU_NET_CREATE_JOIN, 1);
}


local void FreePudInfos(FileList *fl, int n)
{
    int i;

    for (i = 0; i < n; i++) {
	if (fl[i].type && fl[i].xdata) {
	    FreePudInfo(fl[i].xdata);
	    fl[i].xdata = NULL;
	}
    }
}

local void ScenSelectInit(Menuitem *mi __attribute__((unused)))
{
    strcpy(ScenSelectPath, FreeCraftLibPath);
    ScenSelectDisplayPath[0] = 0;
    ScenSelectMenuItems[9].flags = MenuButtonDisabled;
    ScenSelectMenuItems[9].d.button.text = NULL;
}

local void ScenSelectLBAction(Menuitem *mi, int i)
{
    FileList *fl;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    ScenSelectMenuItems[3].d.button.text = "OK";
	} else {
	    ScenSelectMenuItems[3].d.button.text = "Open";
	}
	if (mi->d.listbox.noptions > 5) {
	    mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	}
    }
}

local void ScenSelectLBExit(Menuitem *mi)
{
    FileList *fl;

    if (mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	FreePudInfos(fl, mi->d.listbox.noptions);
	free(fl);
	mi->d.listbox.options = NULL;
	mi->d.listbox.noptions = 0;
	mi[1].flags |= MenuButtonDisabled;
    }
}

local int ScenSelectRDFilter(char *pathbuf, FileList *fl)
{
    PudInfo *info;
    char *suf, *cp, *lcp, *np;
    static int p, sz, szl[] = { -1, 32, 64, 96, 128 };

    if (ScenSelectMenuItems[6].d.pulldown.curopt == 0) {
	suf = ".cm";
	p = 0;
    } else {
	suf = ".pud";
	p = 1;
    }
    np = strrchr(pathbuf, '/');
    if (np) {
	np++;
    } else {
	np = pathbuf;
    }
    cp = np;
    cp--;
    fl->type = -1;
    do {
	lcp = cp++;
	cp = strstr(cp, suf);
    } while (cp != NULL);
    if (lcp >= np) {
	cp = lcp + strlen(suf);
#ifdef USE_ZLIB
	if (strcmp(cp, ".gz") == 0) {
	    *cp = 0;
	}
#endif
#ifdef USE_BZ2LIB
	if (strcmp(cp, ".bz2") == 0) {
	    *cp = 0;
	}
#endif
	if (*cp == 0) {
	    if (p) {
		if (strstr(pathbuf, ".pud")) {
		    info = GetPudInfo(pathbuf);
		} else {
		    info = NULL;
		    // info = GetCmInfo(pathbuf);
		}
		if (info) {
		    sz = szl[ScenSelectMenuItems[8].d.pulldown.curopt];
		    if (sz < 0 || (info->MapWidth == sz && info->MapHeight == sz)) {
			fl->type = 1;
			fl->name = strdup(np);
			fl->xdata = info;
			return 1;
		    } else {
			FreePudInfo(info);
		    }
		} else {
		    fl->type = 1;
		    fl->name = strdup(np);
		    fl->xdata = NULL;
		}
	    } else {
		fl->type = 1;
		fl->name = strdup(np);
		fl->xdata = NULL;
		return 1;
	    }
	}
    }
    return 0;
}

local void ScenSelectLBInit(Menuitem *mi)
{
    int i;

    ScenSelectLBExit(mi);
    if (ScenSelectMenuItems[6].d.pulldown.curopt == 0) {
	ScenSelectMenuItems[8].flags |= MenuButtonDisabled;
    } else {
	ScenSelectMenuItems[8].flags &= ~MenuButtonDisabled;
    }
    i = mi->d.listbox.noptions = ReadDataDirectory(ScenSelectPath, ScenSelectRDFilter,
						     (FileList **)&(mi->d.listbox.options));
    if (i == 0) {
	ScenSelectMenuItems[3].d.button.text = "OK";
	ScenSelectMenuItems[3].flags |= MenuButtonDisabled;
    } else {
	ScenSelectLBAction(mi, 0);
	ScenSelectMenuItems[3].flags &= ~MenuButtonDisabled;
	if (i > 5) {
	    mi[1].flags &= ~MenuButtonDisabled;
	}
    }
}

local unsigned char *ScenSelectLBRetrieve(Menuitem *mi, int i)
{
    FileList *fl;
    Menu *menu;
    PudInfo *info;
    static char buffer[1024];
    int j, n;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    if (i - mi->d.listbox.startline == mi->d.listbox.curopt) {
		if ((info = fl[i].xdata)) {
		    menu = Menus + MENU_SCEN_SELECT;
		    if (info->Description) {
			DrawText(menu->x+8,menu->y+254,LargeFont,info->Description);
		    }
		    sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
		    DrawText(menu->x+8,menu->y+254+20,LargeFont,buffer);
		    for (n = j = 0; j < 16; j++) {
			if (info->PlayerType[j] == PlayerHuman) {
			    n++;
			}
		    }
		    if (n == 1) {
			DrawText(menu->x+8,menu->y+254+40,LargeFont,"1 player");
		    } else {
			sprintf(buffer, "%d players", n);
			DrawText(menu->x+8,menu->y+254+40,LargeFont,buffer);
		    }
		}
	    }
	    strcpy(buffer, "   ");
	} else {
	    strcpy(buffer, "\260 ");
	}
	strcat(buffer, fl[i].name);
	return buffer;
    }
    return NULL;
}

local void ScenSelectTPMSAction(Menuitem *mi, int i __attribute__((unused)))
{
    mi = ScenSelectMenuItems + 1;
    ScenSelectLBInit(mi);
    mi->d.listbox.cursel = -1;
    mi->d.listbox.startline = 0;
    mi->d.listbox.curopt = 0;
    mi[1].d.vslider.percent = 0;
    MustRedraw |= RedrawMenu;
}

local void ScenSelectVSAction(Menuitem *mi, int i)
{
    int op, d1, d2;

    mi--;
    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_DOWN) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.pulldown.noptions) {
		    mi->d.listbox.curopt++;
		    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
			mi->d.listbox.curopt--;
			mi->d.listbox.startline++;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    } else if (mi[1].d.vslider.cflags&MI_CFLAGS_UP) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
		    mi->d.listbox.curopt--;
		    if (mi->d.listbox.curopt < 0) {
			mi->d.listbox.curopt++;
			mi->d.listbox.startline--;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    }
	    ScenSelectLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
	    if (i == 2) {
		mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
	    }
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.pulldown.noptions) {
			op = ((mi->d.listbox.curopt + mi->d.listbox.startline + 1) * 100) /
				 (mi->d.listbox.noptions - 1);
			d1 = mi[1].d.vslider.curper - mi[1].d.vslider.percent;
			d2 = op - mi[1].d.vslider.curper;
			if (d2 < d1) {
			    mi->d.listbox.curopt++;
			    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
				mi->d.listbox.curopt--;
				mi->d.listbox.startline++;
			    }
			}
		    }
		} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
			op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
				 (mi->d.listbox.noptions - 1);
			d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
			d2 = mi[1].d.vslider.curper - op;
			if (d2 < d1) {
			    mi->d.listbox.curopt--;
			    if (mi->d.listbox.curopt < 0) {
				mi->d.listbox.curopt++;
				mi->d.listbox.startline--;
			    }
			}
		    }
		}
		ScenSelectLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
		mi[1].d.vslider.percent = mi[1].d.vslider.curper;
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
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
	    ScenSelectMenuItems[9].flags |= MenuButtonDisabled;
	    ScenSelectMenuItems[9].d.button.text = NULL;
	}
	cp = strrchr(ScenSelectPath, '/');
	if (cp) {
	    *cp = 0;
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
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
	    if (ScenSelectMenuItems[9].flags&MenuButtonDisabled) {
		ScenSelectMenuItems[9].flags &= ~MenuButtonDisabled;
		ScenSelectMenuItems[9].d.button.text = ScenSelectDisplayPath;
	    } else {
		strcat(ScenSelectDisplayPath, "/");
	    }
	    strcat(ScenSelectDisplayPath, fl[i].name);
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	} else {
	    strcpy(ScenSelectFileName, fl[i].name);	// Final map name
	    EndMenu();
	}
    }
}

local void ScenSelectCancel(void)
{
    strcpy(ScenSelectPath, FreeCraftLibPath);
    strcpy(ScenSelectFileName, "default.pud");
    EndMenu();
}

local void CustomGameCancel(void)
{
    DestroyCursorBackground();
    StartMenusSetBackground(NULL);
    FreePudInfo(ScenSelectPudInfo);
    ScenSelectPudInfo = NULL;
    EndMenu();

}

local void CustomGameStart(void)
{
    FreePudInfo(ScenSelectPudInfo);
    ScenSelectPudInfo = NULL;
    if (ScenSelectPath[0]) {
	strcat(ScenSelectPath, "/");
    }
    strcat(ScenSelectPath, ScenSelectFileName);		// Final map name with path
    LoadMap(ScenSelectPath, &TheMap);
    CustomGameStarted = 1;
    EndMenu();
}

local void GameSetupInit(Menuitem *mi __attribute__((unused)))
{
    strcpy(ScenSelectPath, FreeCraftLibPath);
#if 0	// FIXME: as soon as .cm is supported..
#if defined(USE_CCL)
    strcpy(ScenSelectFileName, "default.cm");
#else
    strcpy(ScenSelectFileName, "default.pud");
#endif
#endif
    strcpy(ScenSelectFileName, "default.pud");
    if (ScenSelectPath[0]) {
	strcat(ScenSelectPath, "/");
    }
    strcat(ScenSelectPath, ScenSelectFileName);		// Final map name with path
    if (strstr(ScenSelectFileName, ".pud")) {
	ScenSelectPudInfo = GetPudInfo(ScenSelectPath);
    } else {
	// FIXME: GetCmInfo();
    }
    strcpy(ScenSelectPath, FreeCraftLibPath);
}

local void GameDrawFunc(Menuitem *mi)
{
    int nc, rc;
    char buffer[32];

    GetDefaultTextColors(&nc, &rc);
    StartMenusSetBackground(mi);
    SetDefaultTextColors(rc, rc);
    DrawText(OffsetX + 16, OffsetY + 360, GameFont, "Scenario:");
    DrawText(OffsetX + 16, OffsetY + 360+24 , GameFont, ScenSelectFileName);
    if (ScenSelectPudInfo) {
	if (ScenSelectPudInfo->Description) {
	    DrawText(OffsetX + 16, OffsetY + 360+24+24, GameFont, ScenSelectPudInfo->Description);
	}
	sprintf(buffer, "%d x %d", ScenSelectPudInfo->MapWidth, ScenSelectPudInfo->MapHeight);
	DrawText(OffsetX + 16, OffsetY + 360+24+24+24, GameFont, buffer);
    }
#if 0
    for (n = j = 0; j < 16; j++) {
	if (info->PlayerType[j] == PlayerHuman) {
	    n++;
	}
    }
    if (n == 1) {
	DrawText(menu->x+8,menu->y+254+40,LargeFont,"1 player");
    } else {
	sprintf(buffer, "%d players", n);
	DrawText(menu->x+8,menu->y+254+40,LargeFont,buffer);
    }
#endif
    SetDefaultTextColors(nc, rc);
}

local void GameRCSAction (Menuitem *mi __attribute__((unused)), int i)
{
    int v[] = { PlayerRaceHuman, PlayerRaceOrc, SettingsPresetMapDefault };

    GameSettings.Presets[0].Race = v[i];
}

local void GameRESAction (Menuitem *mi __attribute__((unused)), int i)
{
    int v[] = { SettingsResourcesMapDefault, SettingsResourcesLow,
		SettingsResourcesMedium, SettingsResourcesHigh };

    GameSettings.Resources = v[i];
}

local void GameUNSAction (Menuitem *mi __attribute__((unused)), int i)
{
    GameSettings.NumUnits = i ? SettingsNumUnits1 : SettingsNumUnitsMapDefault;
}

local void GameTSSAction (Menuitem *mi __attribute__((unused)), int i)
{
    int v[] = { SettingsPresetMapDefault, TilesetSummer, TilesetWinter, TilesetWasteland, TilesetSwamp };

    GameSettings.Terrain = v[i];
}

local void CustomGameOPSAction (Menuitem *mi __attribute__((unused)), int i)
{
    GameSettings.Opponents = i ? i : SettingsPresetMapDefault;
}

local void MultiGameFWSAction (Menuitem *mi __attribute__((unused)), int i)
{
    FlagRevealMap = i;
}

local void MultiGameDrawFunc(Menuitem *mi)
{
    GameDrawFunc(mi);
}

local void MultiGameSetupInit(Menuitem *mi)
{
    int i;

    GameSetupInit(mi);
    NetMultiSetupMenuItems[5] = NetMultiButtonStorage[1];
    NetMultiSetupMenuItems[5].yofs = 32;
    for (i = 1; i < 8; i++) {
	NetMultiSetupMenuItems[5 + i] = NetMultiButtonStorage[0];
	NetMultiSetupMenuItems[5 + i].yofs = 32 + i * 22;
    }
}

local void NetMultiPlayerDrawFunc(Menuitem *mi)
{
    int nc, rc;

    GetDefaultTextColors(&nc, &rc);
    SetDefaultTextColors(rc, rc);
    DrawText(mi->xofs, mi->yofs, GameFont, NetworkName);
    SetDefaultTextColors(nc, rc);
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

    if (CurrentMenu < 0) {
	return 0;
    }

    if (MenuButtonCurSel != -1 && menu->items[MenuButtonCurSel].mitype == MI_TYPE_INPUT) {
	mi = menu->items + MenuButtonCurSel;
	if (!(mi->flags & MenuButtonDisabled)) {
inkey:
	    if (key < 0) {
		key &= 0xFF;
	    }
	    if (key >= 0x80 && key < 0x100) {
		// FIXME ARI: ISO->WC2 Translation here!
		key = 0;
	    }
	    switch(key) {
		case '\b': case '\177':
		    if (mi->d.input.nch > 0) {
			strcpy(mi->d.input.buffer + (--mi->d.input.nch), "~!_");
			MustRedraw |= RedrawMenu;
		    }
		    break;
		case 9:
		    goto normkey;
		default:
		    if (key >= 32 && key < 0x100) {
			if (mi->d.input.nch < mi->d.input.maxch) {
			    mi->d.input.buffer[mi->d.input.nch++] = key;
			    strcpy(mi->d.input.buffer + mi->d.input.nch, "~!_");
			    MustRedraw |= RedrawMenu;
			}
		    }
		    break;
	    }
	    if (mi->d.input.action) {
		(*mi->d.input.action)(mi, key);
	    }
	    return 1;
	}
    }

normkey:
    mi = menu->items;
    i = menu->nitems;
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
	case 10: case 13:			// RETURN
	    if (MenuButtonCurSel != -1) {
		mi = menu->items + MenuButtonCurSel;
		switch (mi->mitype) {
		    case MI_TYPE_BUTTON:
			if (mi->d.button.handler) {
			    (*mi->d.button.handler)();
			}
			return 1;
		    case MI_TYPE_LISTBOX:
			if (mi->d.listbox.handler) {
			    (*mi->d.listbox.handler)();
			}
			return 1;
		    case MI_TYPE_VSLIDER:
			if (mi->d.vslider.handler) {
			    (*mi->d.vslider.handler)();
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
			    if (mi->d.listbox.action) {
				(*mi->d.listbox.action)(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
			    }
			    MustRedraw |= RedrawMenu;
			    break;
			case MI_TYPE_VSLIDER:
			    if (key == KeyCodeDown) {
				mi->d.vslider.cflags |= MI_CFLAGS_DOWN;
			    } else {
				mi->d.vslider.cflags |= MI_CFLAGS_UP;
			    }
			    if (mi->d.vslider.action) {
				(*mi->d.vslider.action)(mi, 2);		// 0 indicates key down
			    }
			    MustRedraw |= RedrawMenu;
			    break;
			default:
			    break;
		    }
		}
	    }
	    break;
	case 9:				// TAB			// FIXME: Add Shift-TAB
	    if (MenuButtonCurSel != -1 && !(menu->items[MenuButtonCurSel].flags&MenuButtonClicked)) {
		n = menu->nitems;
		for (i = 0; i < n; ++i) {
		    mi = menu->items + ((MenuButtonCurSel + i + 1) % n);
		    switch (mi->mitype) {
			case MI_TYPE_BUTTON:
			case MI_TYPE_PULLDOWN:
			case MI_TYPE_LISTBOX:
			case MI_TYPE_VSLIDER:
			case MI_TYPE_INPUT:
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
	    mi = menu->items;
	    i = menu->nitems;
	    while (i--) {
		switch (mi->mitype) {
		    case MI_TYPE_INPUT:
			if (!(mi->flags & MenuButtonDisabled)) {
			    menu->items[MenuButtonCurSel].flags &= ~MenuButtonSelected;
			    mi->flags |= MenuButtonSelected;
			    MenuButtonCurSel = mi - menu->items;
			    MustRedraw |= RedrawMenu;
			    goto inkey;
			}
		    default:
			break;
		}
		mi++;
	    }
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

    /* check active (popped-up) pulldown first, as it may overlay other menus! */
    mi = menu->items;
    for (i = 0; i < n; ++i) {
	if (!(mi->flags&MenuButtonDisabled)) {
	    if (mi->mitype == MI_TYPE_PULLDOWN && (mi->flags&MenuButtonClicked)) {
		xs = menu->x + mi->xofs;
		ys = menu->y + mi->yofs;
		h = mi->d.pulldown.ysize - 2;
		ys -= mi->d.pulldown.curopt * h;
		if (!(x<xs || x>xs + mi->d.pulldown.xsize || y<ys || y>ys + h*mi->d.pulldown.noptions)) {
		    j = (y - ys) / h;
		    if (j != mi->d.pulldown.cursel) {
			mi->d.pulldown.cursel = j;
			RedrawFlag = 1;
			if (mi->d.pulldown.action) {
			    (*mi->d.pulldown.action)(mi, mi->d.pulldown.cursel);
			}
		    }
		}
		MenuButtonUnderCursor = i;
		break;
	    }
	}
	mi++;
    }
    if (MenuButtonUnderCursor == -1) {
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
		    case MI_TYPE_INPUT:
			xs = menu->x + mi->xofs;
			ys = menu->y + mi->yofs;
			if (x<xs || x>xs + mi->d.input.xsize || y<ys || y>ys + mi->d.input.ysize) {
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
			// Clicked-state already checked above - there can only be one!
			xs = menu->x + mi->xofs;
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
		    case MI_TYPE_VSLIDER:
			xs = menu->x + mi->xofs;
			ys = menu->y + mi->yofs;
			if (x < xs || x > xs + mi->d.vslider.xsize || y < ys || y > ys + mi->d.vslider.ysize) {
			    if (!(mi->flags&MenuButtonClicked)) {
				if (mi->flags&MenuButtonActive) {
				    RedrawFlag = 1;
				    mi->flags &= ~MenuButtonActive;
				}
			    }
			    mi->d.vslider.cursel = 0;
			    continue;
			}
			j = y - ys;
			if (j < 20) {
			    mi->d.vslider.cursel |= MI_CFLAGS_UP;
			} else if (j > mi->d.vslider.ysize - 20) {
			    mi->d.vslider.cursel |= MI_CFLAGS_DOWN;
			} else {
			    mi->d.vslider.cursel &= ~(MI_CFLAGS_UP|MI_CFLAGS_DOWN);
			    h = (mi->d.vslider.percent * (mi->d.vslider.ysize - 54)) / 100 + 18;
			    mi->d.vslider.curper = ((j - 20) * 100) / (mi->d.vslider.ysize - 54);
			    if (mi->d.vslider.curper > 100) {
				mi->d.vslider.curper = 100;
			    }
			    if (j > h && j < h + 18) {
				mi->d.vslider.cursel |= MI_CFLAGS_KNOB;
			    } else {
				mi->d.vslider.cursel |= MI_CFLAGS_CONT;
			    }
			}
			if (mi->d.vslider.action) {
			    (*mi->d.vslider.action)(mi, 1);		// 1 indicates move
			}
			break;
		    default:
			break;
		}
		switch (mi->mitype) {
		    case MI_TYPE_BUTTON:
		    case MI_TYPE_PULLDOWN:
		    case MI_TYPE_LISTBOX:
		    case MI_TYPE_VSLIDER:
		    case MI_TYPE_INPUT:
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
		    case MI_TYPE_VSLIDER:
		    case MI_TYPE_INPUT:
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
		case MI_TYPE_VSLIDER:
		    mi->d.vslider.cflags = mi->d.vslider.cursel;
		    if (mi->d.vslider.action) {
			(*mi->d.vslider.action)(mi, 0);		// 0 indicates down
		    }
		    break;
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
				if (mi->d.pulldown.action) {
				    (*mi->d.pulldown.action)(mi, mi->d.pulldown.curopt);
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
		case MI_TYPE_VSLIDER:
		    if (mi->flags&MenuButtonClicked) {
			RedrawFlag = 1;
			mi->flags &= ~MenuButtonClicked;
			mi->d.vslider.cflags = 0;
		    }
		    break;
		default:
		    break;
	    }
	}
    }
    if (RedrawFlag) {
	MustRedraw |= RedrawMenu;

	MenuHandleMouseMove(CursorX,CursorY);
    }
}

/**
**	End process menu
**
*/
local void EndMenu(void)
{
    CursorOn = CursorOnUnknown;
    CurrentMenu = -1;
    MustRedraw |= RedrawMenu|RedrawCursor;
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
    int CurrentMenuSave = -1, MenuButtonUnderCursorSave = -1, MenuButtonCurSelSave = -1;

    // Recursion protection:
    if (Loop) {
	CurrentMenuSave = CurrentMenu;
	MenuButtonUnderCursorSave = MenuButtonUnderCursor;
	MenuButtonCurSelSave = MenuButtonCurSel;
    }

    InterfaceState = IfaceStateMenu;
    HideCursor();
    DestroyCursorBackground();
    MustRedraw |= RedrawCursor;
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
	    case MI_TYPE_VSLIDER:
	    case MI_TYPE_INPUT:
		mi->flags &= ~(MenuButtonClicked|MenuButtonActive|MenuButtonSelected);
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
	    case MI_TYPE_VSLIDER:
		mi->d.vslider.cflags = 0;
		if (mi->d.vslider.defper != -1)
		    mi->d.vslider.percent = mi->d.vslider.defper;
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
	MenuHandleMouseMove(CursorX,CursorY);	// This activates buttons as appropriate!
	MustRedraw |= RedrawCursor;
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

    if (Loop) {
	CurrentMenu = CurrentMenuSave;
	MenuButtonUnderCursor = MenuButtonUnderCursorSave;
	MenuButtonCurSel = MenuButtonCurSelSave;
    }
}

/**
**	Move buttons so they're centered on different resolutions
*/
local void MoveButtons()
{
    int menus = sizeof(Menus) / sizeof(Menu);
    int i;

    OffsetX = (VideoWidth - 640) / 2;
    OffsetY = (VideoHeight - 480) / 2;

    for (i=0; i<menus; i++) {
	Menus[i].x += OffsetX;
	Menus[i].y += OffsetY;
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

    if (last_race == -1 && VideoWidth != 640) {
	MoveButtons();
    }

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

    strcpy(ScenSelectPath, FreeCraftLibPath);
    if (ScenSelectPath[0]) {
	strcat(ScenSelectPath, "/");
    }
    strcat(ScenSelectPath, "swamp.rgb");
    if (access(ScenSelectPath, F_OK) != 0) {
	// ARI FIXME: Hack to disable Expansion Gfx..
	// also shows how to add new tilesets....
	CustomGameMenuItems[14].d.pulldown.noptions = 4;

    }
}

//@}
