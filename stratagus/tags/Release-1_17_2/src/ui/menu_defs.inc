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
/**@name menus_defs.inc	-	The menu and buttons definitions. */
//
//	(c) Copyright 1999-2002 by Andreas Arens
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
--	Definitions
----------------------------------------------------------------------------*/

#ifdef OLD_MENU
/**
**	Items for the Game Menu
*/
local Menuitem GameMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, InitGameMenu, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#ifdef DEBUG
    { MI_TYPE_BUTTON, 16 + 12 + 106, 40, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#else
    { MI_TYPE_BUTTON, 16 + 12 + 106, 40, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#endif
    { MI_TYPE_BUTTON, 16, 40 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36 + 36 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitGameMenuItems() {
    MenuitemText   i0 = { "Game Menu", MI_TFLAGS_CENTERED};
    MenuitemButton i1 = { "Save (~<F11~>)", 106, 27, MBUTTON_GM_HALF, GameMenuSave, KeyCodeF11};
    MenuitemButton i2 = { "Load (~<F12~>)", 106, 27, MBUTTON_GM_HALF, GameMenuLoad, KeyCodeF12};
    MenuitemButton i3 = { "Options (~<F5~>)", 224, 27, MBUTTON_GM_FULL, GameOptions, KeyCodeF5};
    MenuitemButton i4 = { "Help (~<F1~>)", 224, 27, MBUTTON_GM_FULL, HelpMenu, KeyCodeF1};
    MenuitemButton i5 = { "Scenario ~!Objectives", 224, 27, MBUTTON_GM_FULL, GameMenuObjectives, 'o'};
    MenuitemButton i6 = { "~!End Scenario", 224, 27, MBUTTON_GM_FULL, GameMenuEndScenario, 'e'};
    MenuitemButton i7 = { "Return to Game (~<Esc~>)", 224, 27, MBUTTON_GM_FULL, GameMenuReturn, '\033'};
    GameMenuItems[0].d.text   = i0;
    GameMenuItems[1].d.button = i1;
    GameMenuItems[2].d.button = i2;
    GameMenuItems[3].d.button = i3;
    GameMenuItems[4].d.button = i4;
    GameMenuItems[5].d.button = i5;
    GameMenuItems[6].d.button = i6;
    GameMenuItems[7].d.button = i7;
}

/**
**	Items for the Victory Menu
*/
local Menuitem VictoryMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 90, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 56, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitVictoryMenuItems() {
    MenuitemText   i0 = { "Congratulations!", MI_TFLAGS_CENTERED};
    MenuitemText   i1 = { "You are victorious!", MI_TFLAGS_CENTERED};
    MenuitemButton i2 = { "~!Victory", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'v'};
    MenuitemButton i3 = { "Save Game (~<F11~>)", 224, 27, MBUTTON_GM_FULL, NULL, KeyCodeF11};
    VictoryMenuItems[0].d.text   = i0;
    VictoryMenuItems[1].d.text   = i1;
    VictoryMenuItems[2].d.button = i2;
    VictoryMenuItems[3].d.button = i3;
}

/**
**	Items for the Lost Menu
*/
local Menuitem LostMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 90, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitLostMenuItems() {
    MenuitemText   i0 = { "You failed to", MI_TFLAGS_CENTERED};
    MenuitemText   i1 = { "achieve victory!", MI_TFLAGS_CENTERED};
    MenuitemButton i2 = { "~!OK", 224, 27, MBUTTON_GM_FULL, GameMenuEnd, 'o'};
    LostMenuItems[0].d.text   = i0;
    LostMenuItems[1].d.text   = i1;
    LostMenuItems[2].d.button = i2;
}

local Menuitem TipsMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, InitTips, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 14, 256-75, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14+22, 256-75+4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 14, 256-40, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 168, 256-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*0, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*6, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 35+16*7, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitTipsMenuItems() {
    MenuitemText   i0  = { "Freecraft Tips", MI_TFLAGS_CENTERED};
    MenuitemGem    i1  = { MI_GSTATE_CHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetTips};
    MenuitemText   i2  = { "Show tips at startup", MI_TFLAGS_LALIGN};
    MenuitemButton i3  = { "~!Next Tip", 106, 27, MBUTTON_GM_HALF, ShowNextTip, 'n'};
    MenuitemButton i4  = { "~!Close", 106, 27, MBUTTON_GM_HALF, TipsMenuEnd, 'c'};
    MenuitemText   i5  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i6  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i7  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i8  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i9  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i10 = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i11 = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText   i12 = { NULL, MI_TFLAGS_LALIGN};
    TipsMenuItems[0].d.text   = i0;
    TipsMenuItems[1].d.gem    = i1;
    TipsMenuItems[2].d.text   = i2;
    TipsMenuItems[3].d.button = i3;
    TipsMenuItems[4].d.button = i4;
    TipsMenuItems[5].d.text   = i5;
    TipsMenuItems[6].d.text   = i6;
    TipsMenuItems[7].d.text   = i7;
    TipsMenuItems[8].d.text   = i8;
    TipsMenuItems[9].d.text   = i9;
    TipsMenuItems[10].d.text  = i10;
    TipsMenuItems[11].d.text  = i11;
    TipsMenuItems[12].d.text  = i12;
}

local Menuitem ObjectivesMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*7, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 14, 38+21*8, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitObjectivesMenuItems() {
    MenuitemText   i0  = { "Objectives", MI_TFLAGS_CENTERED};
    MenuitemText   i1  = { NULL, 0};
    MenuitemButton i10 = { "~!OK", 224, 27, MBUTTON_GM_FULL, EndMenu, 'o'};
    ObjectivesMenuItems[0].d.text    = i0;
    ObjectivesMenuItems[1].d.text    = i1;
    ObjectivesMenuItems[2].d.text    = i1;
    ObjectivesMenuItems[3].d.text    = i1;
    ObjectivesMenuItems[4].d.text    = i1;
    ObjectivesMenuItems[5].d.text    = i1;
    ObjectivesMenuItems[6].d.text    = i1;
    ObjectivesMenuItems[7].d.text    = i1;
    ObjectivesMenuItems[8].d.text    = i1;
    ObjectivesMenuItems[9].d.text    = i1;
    ObjectivesMenuItems[10].d.button = i10;
}

local Menuitem EndScenarioMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitEndScenarioMenuItems() {
    MenuitemText   i0 = { "End Scenario", MI_TFLAGS_CENTERED};
    MenuitemButton i1 = { "~!Restart Scenario", 224, 27, MBUTTON_GM_FULL, EndScenarioRestart, 'r'};
    MenuitemButton i2 = { "~!Surrender", 224, 27, MBUTTON_GM_FULL, EndScenarioSurrender, 's'};
    MenuitemButton i3 = { "~!Quit to Menu", 224, 27, MBUTTON_GM_FULL, EndScenarioQuitMenu, 'q'};
    MenuitemButton i4 = { "E~!xit Program", 224, 27, MBUTTON_GM_FULL, GameMenuExit, 'x'};
    MenuitemButton i5 = { "Previous (~!E~!s~!c)", 224, 27, MBUTTON_GM_FULL, EndMenu, '\033'};
    EndScenarioMenuItems[0].d.text  = i0;
    EndScenarioMenuItems[1].d.button = i1;
    EndScenarioMenuItems[2].d.button = i2;
    EndScenarioMenuItems[3].d.button = i3;
    EndScenarioMenuItems[4].d.button = i4;
    EndScenarioMenuItems[5].d.button = i5;
}

/**
**	Items for the SelectScen Menu
*/
local unsigned char *ssmtoptions[] = {
    // "All scenarios (cm+pud)",
    "Freecraft scenario (cm)",
    "Foreign scenario (pud)"
};

local unsigned char *ssmsoptions[] = {
    "Any size",
    "32 x 32",
    "64 x 64",
    "96 x 96",
    "128 x 128",
    "256 x 256",
    "512 x 512",
    "1024 x 1024",
    // FIXME: We support bigger sizes
};
/**
**	Scenario selection menu.
*/
local Menuitem ScenSelectMenuItems[] = {
    { MI_TYPE_TEXT, 176, 8, 0, LargeFont, ScenSelectInit, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_LISTBOX, 24, 140, 0, GameFont, ScenSelectLBInit, ScenSelectLBExit, NULL, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 312, 140, 0, 0, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 48, 318, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 198, 318, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 132, 40, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 140, 40, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 132, 80, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 140, 80, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 22, 112, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitScenSelectMenuItems() {
    MenuitemText    i0 = { "Select scenario", MI_TFLAGS_CENTERED};

    MenuitemListbox  i1 = { NULL, 288, 6*18, MBUTTON_PULLDOWN, ScenSelectLBAction, 0, 0, 0, 0, 6, 0, 0,
			    (void *)ScenSelectLBRetrieve, ScenSelectOk};
    MenuitemVslider  i2 = { 0, 18, 6*18, ScenSelectVSAction, -1, 0, 0, 0, ScenSelectOk};

    MenuitemButton   i3 = { "OK", 106, 27, MBUTTON_GM_HALF, ScenSelectOk, 0};
    MenuitemButton   i4 = { "Cancel", 106, 27, MBUTTON_GM_HALF, ScenSelectCancel, 0};

    MenuitemText     i5 = { "Type:", MI_TFLAGS_RALIGN};
    MenuitemPulldown i6 = { ssmtoptions, 192, 20, MBUTTON_PULLDOWN, ScenSelectTPMSAction, 2, 1, 1, 0, 0};
    MenuitemText     i7 = { "Map size:", MI_TFLAGS_RALIGN};
    MenuitemPulldown i8 = { ssmsoptions, 192, 20, MBUTTON_PULLDOWN, ScenSelectTPMSAction, 8, 0, 0, 0, 0};
    MenuitemButton   i9 = { NULL, 36, 24, MBUTTON_FOLDER, ScenSelectFolder, 0};
    ScenSelectMenuItems[0].d.text     = i0;
    ScenSelectMenuItems[1].d.listbox  = i1;
    ScenSelectMenuItems[2].d.vslider  = i2;
    ScenSelectMenuItems[3].d.button   = i3;
    ScenSelectMenuItems[4].d.button   = i4;
    ScenSelectMenuItems[5].d.text     = i5;
    ScenSelectMenuItems[6].d.pulldown = i6;
    ScenSelectMenuItems[7].d.text     = i7;
    ScenSelectMenuItems[8].d.pulldown = i8;
    ScenSelectMenuItems[9].d.button   = i9;
}

/**
**	Items for the Program Start Menu
*/
local Menuitem PrgStartMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, PrgStartInit, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#ifdef DEBUG
    { MI_TYPE_BUTTON, 208, 145 + 36 * 3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#else
    { MI_TYPE_BUTTON, 208, 145 + 36 * 3, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 4, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#endif
    { MI_TYPE_BUTTON, 208, 145 + 36 * 5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 145 + 36 * 7, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitPrgStartMenuItems() {
    MenuitemDrawfunc i0 = { NameLineDrawFunc };
    MenuitemButton   i1 = { "~!Single Player Game", 224, 27, MBUTTON_GM_FULL, SinglePlayerGameMenu, 's'};
    MenuitemButton   i2 = { "~!Multi Player Game", 224, 27, MBUTTON_GM_FULL, MultiPlayerGameMenu, 'm'};
    MenuitemButton   i3 = { "~!Campaign Game", 224, 27, MBUTTON_GM_FULL, CampaignGameMenu, 'c'};
    MenuitemButton   i4 = { "~!Load Game", 224, 27, MBUTTON_GM_FULL, GameMenuLoad, 'l'};
    MenuitemButton   i5 = { "~!Options", 224, 27, MBUTTON_GM_FULL, GlobalOptions, 'o'};
    MenuitemButton   i6 = { "~!Editor", 224, 27, MBUTTON_GM_FULL, StartEditor, 'e'};
    MenuitemButton   i7 = { "S~!how Credits", 224, 27, MBUTTON_GM_FULL, GameShowCredits, 'h'};
    MenuitemButton   i8 = { "E~!xit Program", 224, 27, MBUTTON_GM_FULL, GameMenuExit, 'x'};
    PrgStartMenuItems[0].d.drawfunc = i0;
    PrgStartMenuItems[1].d.button   = i1;
    PrgStartMenuItems[2].d.button   = i2;
    PrgStartMenuItems[3].d.button   = i3;
    PrgStartMenuItems[4].d.button   = i4;
    PrgStartMenuItems[5].d.button   = i5;
    PrgStartMenuItems[6].d.button   = i6;
    PrgStartMenuItems[7].d.button   = i7;
    PrgStartMenuItems[8].d.button   = i8;
}

/**
**	Items for the Custom Game Setup Menu
*/
local unsigned char *rcsoptions[] = {
    "Human",
    "Orc",
    "Map Default",
    // FIXME: we support more and other race names
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
    // FIXME: we support more and other unit names
};

// FIXME: Must load this from some place.
local unsigned char *tssoptions[] = {
    "Map Default",
    "Forest",
    "Winter",
    "Wasteland",
    "Orc Swamp",	// hmm. need flag if XPN-GFX is present! - tmp-hack in InitMenus()..
			// FIXME: Some time later _all_ tilesets should be a dynamic (ccl-defineable) resource..
};

/**
**	Game type options.
**
**	@todo Needs to be configurable from CCL.
*/
local unsigned char* gatoptions[] = {
    "Use map settings",
    "Melee",
    "Free for all",
    "One on one",
    "Capture the flag",
    "Greed",
    "Slaughter",
    "Sudden death",
    "Team melee",
    "Team free for all",
    "Team capture the flag",
    "Top vs bottom",
    "Left vs right",
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
    // FIXME: We support upto 15 opponents
};

local unsigned char *mgfwsoptions[] = {
    "On",
    "Off",
};

/**
**	Single player custom game menu.
*/
local Menuitem CustomGameMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, GameSetupInit, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640/2+12, 192, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36+36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 220, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 640-224-16, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 640-224-16, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640-224-16, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
#ifdef DEBUG
    { MI_TYPE_PULLDOWN, 640-224-16, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
#else
    { MI_TYPE_PULLDOWN, 640-224-16, 10+300, MenuButtonDisabled, GameFont, NULL, NULL, NULL, {{NULL,0}} },
#endif
};
local void InitCustomGameMenuItems() {
    MenuitemDrawfunc i0  = { GameDrawFunc };
    MenuitemText     i1  = { "~<Single Player Game Setup~>", MI_TFLAGS_CENTERED};
    MenuitemButton   i2  = { "S~!elect Scenario", 224, 27, MBUTTON_GM_FULL, ScenSelectMenu, 'e'};
    MenuitemButton   i3  = { "~!Start Game", 224, 27, MBUTTON_GM_FULL, CustomGameStart, 's'};
    MenuitemButton   i4  = { "~!Cancel Game", 224, 27, MBUTTON_GM_FULL, GameCancel, 'c'};
    MenuitemText     i5  = { "~<Your Race:~>", 0};
    MenuitemPulldown i6  = { rcsoptions, 152, 20, MBUTTON_PULLDOWN, GameRCSAction, 3, 2, 2, 0, 0};
    MenuitemText     i7  = { "~<Resources:~>", 0};
    MenuitemPulldown i8  = { resoptions, 152, 20, MBUTTON_PULLDOWN, GameRESAction, 4, 0, 0, 0, 0};
    MenuitemText     i9  = { "~<Units:~>", 0};
    MenuitemPulldown i10 = { unsoptions, 190, 20, MBUTTON_PULLDOWN, GameUNSAction, 2, 0, 0, 0, 0};
    MenuitemText     i11 = { "~<Opponents:~>", 0};
    MenuitemPulldown i12 = { cgopsoptions, 152, 20, MBUTTON_PULLDOWN, CustomGameOPSAction, 8, 0, 0, 0, 0};
    MenuitemText     i13 = { "~<Map Tileset:~>", 0};
    MenuitemPulldown i14 = { tssoptions, 152, 20, MBUTTON_PULLDOWN, GameTSSAction, 5, 0, 0, 0, 0};
    MenuitemText     i15 = { "~<Game Type:~>", 0};
    MenuitemPulldown i16 = { gatoptions, 190, 20, MBUTTON_PULLDOWN, GameGATAction, 13, 0, 0, 0, 0};
    CustomGameMenuItems[0].d.drawfunc  = i0;
    CustomGameMenuItems[1].d.text      = i1;
    CustomGameMenuItems[2].d.button    = i2;
    CustomGameMenuItems[3].d.button    = i3;
    CustomGameMenuItems[4].d.button    = i4;
    CustomGameMenuItems[5].d.text      = i5;
    CustomGameMenuItems[6].d.pulldown  = i6;
    CustomGameMenuItems[7].d.text      = i7;
    CustomGameMenuItems[8].d.pulldown  = i8;
    CustomGameMenuItems[9].d.text      = i9;
    CustomGameMenuItems[10].d.pulldown = i10;
    CustomGameMenuItems[11].d.text     = i11;
    CustomGameMenuItems[12].d.pulldown = i12;
    CustomGameMenuItems[13].d.text     = i13;
    CustomGameMenuItems[14].d.pulldown = i14;
    CustomGameMenuItems[15].d.text     = i15;
    CustomGameMenuItems[16].d.pulldown = i16;
}

/**
**	Items for the Enter Name Menu
*/
local Menuitem EnterNameMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_INPUT, 40, 38, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 24, 80, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 154, 80, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitEnterNameMenuItems() {
    MenuitemText   i0 = { "Enter your name:", MI_TFLAGS_CENTERED};
    MenuitemInput  i1 = { NULL, 212, 20, MBUTTON_PULLDOWN, EnterNameAction, 0, 0};
    MenuitemButton i2 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};
    MenuitemButton i3 = { "~!Cancel", 106, 27, MBUTTON_GM_HALF, EnterNameCancel, 'c'};
    EnterNameMenuItems[0].d.text   = i0;
    EnterNameMenuItems[1].d.input  = i1;
    EnterNameMenuItems[2].d.button = i2;
    EnterNameMenuItems[3].d.button = i3;
}

/**
**	Items for the Enter Server Menu
*/
local Menuitem EnterServerIPMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_INPUT, 40, 38, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 24, 80, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 154, 80, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitEnterServerIPMenuItems() {
    MenuitemText   i0 = { "Enter server IP-address:", MI_TFLAGS_CENTERED};
    MenuitemInput  i1 = { NULL, 212, 20, MBUTTON_PULLDOWN, EnterServerIPAction, 0, 0};
    MenuitemButton i2 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};
    MenuitemButton i3 = { "~!Cancel", 106, 27, MBUTTON_GM_HALF, EnterServerIPCancel, 'c'};
    EnterServerIPMenuItems[0].d.text   = i0;
    EnterServerIPMenuItems[1].d.input  = i1;
    EnterServerIPMenuItems[2].d.button = i2;
    EnterServerIPMenuItems[3].d.button = i3;
}

/**
**	Items for the Net Create Join Menu
*/
local Menuitem NetCreateJoinMenuItems[] = {
    { MI_TYPE_BUTTON, 208, 320, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 320 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 320 + 36 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitNetCreateJoinMenuItems() {
    MenuitemButton i0 = { "~!Join Game", 224, 27, MBUTTON_GM_FULL, JoinNetGameMenu, 'j'};
    MenuitemButton i1 = { "~!Create Game", 224, 27, MBUTTON_GM_FULL, CreateNetGameMenu, 'c'};
    MenuitemButton i2 = { "~!Previous Menu", 224, 27, MBUTTON_GM_FULL, EndMenu, 'p'};
    NetCreateJoinMenuItems[0].d.button = i0;
    NetCreateJoinMenuItems[1].d.button = i1;
    NetCreateJoinMenuItems[2].d.button = i2;
}

/**
**	Multi player custom game menu (server side).
*/
local Menuitem NetMultiSetupMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, MultiGameSetupInit, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640/2+12, 8, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36+36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    // 8+7 player slots (content here is overwritten!)
//#define SERVER_PLAYER_STATE	5
    { MI_TYPE_PULLDOWN, 40, 32+22*0, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*6, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*7, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*0, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*6, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 40, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640-224-16, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 640-224-16, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 220, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    // 7+7 player ready buttons
//#define SERVER_PLAYER_READY	30
    { MI_TYPE_GEM, 10, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*7, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 330, 32+22*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    // FIXME: Slot 15 is reserved for neutral computer
    //{ MI_TYPE_GEM, 330, 32+22*7, 0, LargeFont, NULL, NULL, NULL,
    //	{ gem:{ MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL,0} } },

    // 7+7 player lag buttons
//#define SERVER_PLAYER_LAG	44
    { MI_TYPE_GEM, 218, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 218, 32+22*7, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 538, 32+22*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 538, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    // FIXME: Slot 15 is reserved for neutral computer
    //{ MI_TYPE_GEM, 538, 32+22*7, 0, LargeFont, NULL, NULL, NULL,
    //	{ gem:{ MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL,0} } },
};
local void InitNetMultiSetupMenuItems() {
    MenuitemDrawfunc i0  = { MultiGameDrawFunc };
    MenuitemText     i1  = { "~<Multi Player Setup~>", MI_TFLAGS_CENTERED};
    MenuitemButton   i2  = { "S~!elect Scenario", 224, 27, MBUTTON_GM_FULL, MultiScenSelectMenu, 'e'};
    MenuitemButton   i3  = { "~!Start Game", 224, 27, MBUTTON_GM_FULL, MultiGameStart, 's'};
    MenuitemButton   i4  = { "~!Cancel Game", 224, 27, MBUTTON_GM_FULL, MultiGameCancel, 'c'};

    MenuitemPulldown i5  = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i6  = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i7  = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i8  = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i9  = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i10 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i11 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i12 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i13 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i14 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i15 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i16 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i17 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i18 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i19 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};

    MenuitemText     i20 = { "~<Your Race:~>", 0};
    MenuitemPulldown i21 = { rcsoptions, 152, 20, MBUTTON_PULLDOWN, GameRCSAction, 3, 2, 2, 0, 0};
    MenuitemText     i22 = { "~<Resources:~>", 0};
    MenuitemPulldown i23 = { resoptions, 152, 20, MBUTTON_PULLDOWN, GameRESAction, 4, 0, 0, 0, 0};
    MenuitemText     i24 = { "~<Units:~>", 0};
    MenuitemPulldown i25 = { unsoptions, 190, 20, MBUTTON_PULLDOWN, GameUNSAction, 2, 0, 0, 0, 0};
    MenuitemText     i26 = { "~<Fog of War:~>", 0};
    MenuitemPulldown i27 = { mgfwsoptions, 152, 20, MBUTTON_PULLDOWN, MultiGameFWSAction, 2, 0, 0, 0, 0};
    MenuitemText     i28 = { "~<Map Tileset:~>", 0};
    MenuitemPulldown i29 = { tssoptions, 152, 20, MBUTTON_PULLDOWN, GameTSSAction, 5, 0, 0, 0, 0};

    MenuitemGem      i30 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i31 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i32 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i33 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i34 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i35 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i36 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};

    MenuitemGem      i37 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i38 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i39 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i40 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i41 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i42 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};
    MenuitemGem      i43 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_SQUARE, NULL};

    MenuitemGem      i44 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i45 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i46 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i47 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i48 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i49 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i50 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};

    MenuitemGem      i51 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i52 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i53 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i54 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i55 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i56 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};
    MenuitemGem      i57 = { MI_GSTATE_PASSIVE, 18, 18, MBUTTON_GEM_ROUND, NULL};

    NetMultiSetupMenuItems[0].d.drawfunc  = i0;
    NetMultiSetupMenuItems[1].d.text      = i1;
    NetMultiSetupMenuItems[2].d.button    = i2;
    NetMultiSetupMenuItems[3].d.button    = i3;
    NetMultiSetupMenuItems[4].d.button    = i4;
    NetMultiSetupMenuItems[5].d.pulldown  = i5;
    NetMultiSetupMenuItems[6].d.pulldown  = i6;
    NetMultiSetupMenuItems[7].d.pulldown  = i7;
    NetMultiSetupMenuItems[8].d.pulldown  = i8;
    NetMultiSetupMenuItems[9].d.pulldown  = i9;
    NetMultiSetupMenuItems[10].d.pulldown = i10;
    NetMultiSetupMenuItems[11].d.pulldown = i11;
    NetMultiSetupMenuItems[12].d.pulldown = i12;
    NetMultiSetupMenuItems[13].d.pulldown = i13;
    NetMultiSetupMenuItems[14].d.pulldown = i14;
    NetMultiSetupMenuItems[15].d.pulldown = i15;
    NetMultiSetupMenuItems[16].d.pulldown = i16;
    NetMultiSetupMenuItems[17].d.pulldown = i17;
    NetMultiSetupMenuItems[18].d.pulldown = i18;
    NetMultiSetupMenuItems[19].d.pulldown = i19;
    NetMultiSetupMenuItems[20].d.text     = i20;
    NetMultiSetupMenuItems[21].d.pulldown = i21;
    NetMultiSetupMenuItems[22].d.text     = i22;
    NetMultiSetupMenuItems[23].d.pulldown = i23;
    NetMultiSetupMenuItems[24].d.text     = i24;
    NetMultiSetupMenuItems[25].d.pulldown = i25;
    NetMultiSetupMenuItems[26].d.text     = i26;
    NetMultiSetupMenuItems[27].d.pulldown = i27;
    NetMultiSetupMenuItems[28].d.text     = i28;
    NetMultiSetupMenuItems[29].d.pulldown = i29;
    NetMultiSetupMenuItems[30].d.gem      = i30;
    NetMultiSetupMenuItems[31].d.gem      = i31;
    NetMultiSetupMenuItems[32].d.gem      = i32;
    NetMultiSetupMenuItems[33].d.gem      = i33;
    NetMultiSetupMenuItems[34].d.gem      = i34;
    NetMultiSetupMenuItems[35].d.gem      = i35;
    NetMultiSetupMenuItems[36].d.gem      = i36;
    NetMultiSetupMenuItems[37].d.gem      = i37;
    NetMultiSetupMenuItems[38].d.gem      = i38;
    NetMultiSetupMenuItems[39].d.gem      = i39;
    NetMultiSetupMenuItems[40].d.gem      = i40;
    NetMultiSetupMenuItems[41].d.gem      = i41;
    NetMultiSetupMenuItems[42].d.gem      = i42;
    NetMultiSetupMenuItems[43].d.gem      = i43;
    NetMultiSetupMenuItems[44].d.gem      = i44;
    NetMultiSetupMenuItems[45].d.gem      = i45;
    NetMultiSetupMenuItems[46].d.gem      = i46;
    NetMultiSetupMenuItems[47].d.gem      = i47;
    NetMultiSetupMenuItems[48].d.gem      = i48;
    NetMultiSetupMenuItems[49].d.gem      = i49;
    NetMultiSetupMenuItems[50].d.gem      = i50;
    NetMultiSetupMenuItems[51].d.gem      = i51;
    NetMultiSetupMenuItems[52].d.gem      = i52;
    NetMultiSetupMenuItems[53].d.gem      = i53;
    NetMultiSetupMenuItems[54].d.gem      = i54;
    NetMultiSetupMenuItems[55].d.gem      = i55;
    NetMultiSetupMenuItems[56].d.gem      = i56;
    NetMultiSetupMenuItems[57].d.gem      = i57;
}

/**
**	Multi player client game menu.
*/
local Menuitem NetMultiClientMenuItems[] = {
    { MI_TYPE_DRAWFUNC, 0, 0, 0, GameFont, MultiGameClientInit, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 640/2+12, 8, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 640-224-16, 360, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 640-224-16, 360+36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 640-224-16, 360+36+36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    // 8+7 player slots
//#define CLIENT_PLAYER_STATE	5
    { MI_TYPE_PULLDOWN, 40, 32, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*6, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 40, 32+22*7, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*4, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 360, 32+22*6, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 40, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
//#define CLIENT_RACE	21
    { MI_TYPE_PULLDOWN, 40, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
//#define CLIENT_RESOURCE	23
    { MI_TYPE_PULLDOWN, 220, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 640-224-16, 10+240-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
//#define CLIENT_UNITS	25
    { MI_TYPE_PULLDOWN, 640-224-16, 10+240, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 40, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
//#define CLIENT_FOG_OF_WAR	27
    { MI_TYPE_PULLDOWN, 40, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 220, 10+300-20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
//#define CLIENT_TILESET	29
    { MI_TYPE_PULLDOWN, 220, 10+300, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    // 7+7 player state buttons
//#define CLIENT_PLAYER_READY	30
    { MI_TYPE_GEM, 10, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 10, 32+22*7, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*4, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 330, 32+22*6, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    //{ MI_TYPE_GEM, 330, 32+22*7, 0, LargeFont, NULL, NULL, NULL,
    //	{ gem:{ 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction} } },
};
local void InitNetMultiClientMenuItems() {
    MenuitemDrawfunc i0 = { MultiGameClientDrawFunc };

    MenuitemText     i1 = { "~<Multi Player Game~>", MI_TFLAGS_CENTERED};

    MenuitemButton   i2 = { "~!Ready", 224, 27, MBUTTON_GM_FULL, MultiClientReady, 'r'};
    MenuitemButton   i3 = { "~!Not Ready", 224, 27, MBUTTON_GM_FULL, MultiClientNotReady, 'n'};

    MenuitemButton   i4 = { "~!Cancel Game", 224, 27, MBUTTON_GM_FULL, MultiClientCancel, 'c'};

    MenuitemPulldown i5 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i6 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i7 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i8 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i9 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i10 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i11 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i12 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i13 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i14 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i15 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i16 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i17 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i18 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemPulldown i19 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};

    MenuitemText     i20 = { "~<Your Race:~>", 0};
    MenuitemPulldown i21 = { rcsoptions, 152, 20, MBUTTON_PULLDOWN, MultiClientRCSAction, 3, 2, 2, 0, 0};
    MenuitemText     i22 = { "~<Resources:~>", 0};
    MenuitemPulldown i23 = { resoptions, 152, 20, MBUTTON_PULLDOWN, GameRESAction, 4, 0, 0, 0, MI_PSTATE_PASSIVE};
    MenuitemText     i24 = { "~<Units:~>", 0};
    MenuitemPulldown i25 = { unsoptions, 190, 20, MBUTTON_PULLDOWN, GameUNSAction, 2, 0, 0, 0, MI_PSTATE_PASSIVE};
    MenuitemText     i26 = { "~<Fog of War:~>", 0};
    MenuitemPulldown i27 = { mgfwsoptions, 152, 20, MBUTTON_PULLDOWN, MultiGameFWSAction, 2, 0, 0, 0, MI_PSTATE_PASSIVE};
    MenuitemText     i28 = { "~<Map Tileset:~>", 0};
    MenuitemPulldown i29 = { tssoptions, 152, 20, MBUTTON_PULLDOWN, GameTSSAction, 5, 0, 0, 0, MI_PSTATE_PASSIVE};

    MenuitemGem      i30 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i31 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i32 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i33 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i34 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i35 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i36 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i37 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i38 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i39 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i40 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i41 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i42 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};
    MenuitemGem      i43 = { 0, 18, 18, MBUTTON_GEM_SQUARE, MultiClientGemAction};

    NetMultiClientMenuItems[0].d.drawfunc  = i0;
    NetMultiClientMenuItems[1].d.text      = i1;
    NetMultiClientMenuItems[2].d.button    = i2;
    NetMultiClientMenuItems[3].d.button    = i3;
    NetMultiClientMenuItems[4].d.button    = i4;
    NetMultiClientMenuItems[5].d.pulldown  = i5;
    NetMultiClientMenuItems[6].d.pulldown  = i6;
    NetMultiClientMenuItems[7].d.pulldown  = i7;
    NetMultiClientMenuItems[8].d.pulldown  = i8;
    NetMultiClientMenuItems[9].d.pulldown  = i9;
    NetMultiClientMenuItems[10].d.pulldown = i10;
    NetMultiClientMenuItems[11].d.pulldown = i11;
    NetMultiClientMenuItems[12].d.pulldown = i12;
    NetMultiClientMenuItems[13].d.pulldown = i13;
    NetMultiClientMenuItems[14].d.pulldown = i14;
    NetMultiClientMenuItems[15].d.pulldown = i15;
    NetMultiClientMenuItems[16].d.pulldown = i16;
    NetMultiClientMenuItems[17].d.pulldown = i17;
    NetMultiClientMenuItems[18].d.pulldown = i18;
    NetMultiClientMenuItems[19].d.pulldown = i19;
    NetMultiClientMenuItems[20].d.text     = i20;
    NetMultiClientMenuItems[21].d.pulldown = i21;
    NetMultiClientMenuItems[22].d.text     = i22;
    NetMultiClientMenuItems[23].d.pulldown = i23;
    NetMultiClientMenuItems[24].d.text     = i24;
    NetMultiClientMenuItems[25].d.pulldown = i25;
    NetMultiClientMenuItems[26].d.text     = i26;
    NetMultiClientMenuItems[27].d.pulldown = i27;
    NetMultiClientMenuItems[28].d.text     = i28;
    NetMultiClientMenuItems[29].d.pulldown = i29;
    NetMultiClientMenuItems[30].d.gem      = i30;
    NetMultiClientMenuItems[31].d.gem      = i31;
    NetMultiClientMenuItems[32].d.gem      = i32;
    NetMultiClientMenuItems[33].d.gem      = i33;
    NetMultiClientMenuItems[34].d.gem      = i34;
    NetMultiClientMenuItems[35].d.gem      = i35;
    NetMultiClientMenuItems[36].d.gem      = i36;
    NetMultiClientMenuItems[37].d.gem      = i37;
    NetMultiClientMenuItems[38].d.gem      = i38;
    NetMultiClientMenuItems[39].d.gem      = i39;
    NetMultiClientMenuItems[40].d.gem      = i40;
    NetMultiClientMenuItems[41].d.gem      = i41;
    NetMultiClientMenuItems[42].d.gem      = i42;
    NetMultiClientMenuItems[43].d.gem      = i43;
}

local Menuitem NetErrorMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 38, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 92, 80, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitNetErrorMenuItems() {
    MenuitemText   i0 = { "Error:", MI_TFLAGS_CENTERED};
    MenuitemText   i1 = { NULL, MI_TFLAGS_CENTERED};
    MenuitemButton i2 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};
    NetErrorMenuItems[0].d.text   = i0;
    NetErrorMenuItems[1].d.text   = i1;
    NetErrorMenuItems[2].d.button = i2;
}

/**
**	Items for the Connecting Network Menu
*/
local Menuitem ConnectingMenuItems[] = {
    { MI_TYPE_TEXT, 144, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 32, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 144, 53, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 32, 90, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitConnectingMenuItems() {
    MenuitemText   i0 = { "Connecting to server", MI_TFLAGS_CENTERED};
    MenuitemText   i1 = { NetServerText, MI_TFLAGS_CENTERED};
    MenuitemText   i2 = { NetTriesText , MI_TFLAGS_CENTERED};
    MenuitemButton i3 = { "~!Cancel", 224, 27, MBUTTON_GM_FULL, NetConnectingCancel, 'c'};
    ConnectingMenuItems[0].d.text   = i0;
    ConnectingMenuItems[1].d.text   = i1;
    ConnectingMenuItems[2].d.text   = i2;
    ConnectingMenuItems[3].d.button = i3;
}

/**
**	Items for the Campaign Select Menu
*/
local Menuitem CampaignSelectMenuItems[] = {
    { MI_TYPE_BUTTON, 208, 212 + 36 * 0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 3, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 4, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 212 + 36 * 5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitCampaignSelectMenuItems() {
    MenuitemButton i0 = { NULL, 224, 27, MBUTTON_GM_FULL, CampaignMenu1, 'a'};
    MenuitemButton i1 = { NULL, 224, 27, MBUTTON_GM_FULL, CampaignMenu2, 'm'};
    MenuitemButton i2 = { NULL, 224, 27, MBUTTON_GM_FULL, CampaignMenu3, 'l'};
    MenuitemButton i3 = { NULL, 224, 27, MBUTTON_GM_FULL, CampaignMenu4, 'y'};
    MenuitemButton i4 = { "~!Select Campaign", 224, 27, MBUTTON_GM_FULL, SelectCampaignMenu, 's'};
    MenuitemButton i5 = { "~!Previous Menu", 224, 27, MBUTTON_GM_FULL, EndMenu, 'p'};
    CampaignSelectMenuItems[0].d.button = i0;
    CampaignSelectMenuItems[1].d.button = i1;
    CampaignSelectMenuItems[2].d.button = i2;
    CampaignSelectMenuItems[3].d.button = i3;
    CampaignSelectMenuItems[4].d.button = i4;
    CampaignSelectMenuItems[5].d.button = i5;
}

/**
**	Items for the Campaign Continue Menu
*/
local Menuitem CampaignContMenuItems[] = {
    { MI_TYPE_BUTTON, 508, 320 + 36 + 36 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitCampaignContMenuItems() {
    MenuitemButton i0 = { "~!Continue", 106, 27, MBUTTON_GM_HALF, EndMenu, 'c'};
    CampaignContMenuItems[0].d.button = i0;
}

local Menuitem SoundOptionsMenuItems[] = {
    { MI_TYPE_TEXT, 176, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 16, 36*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*1.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 44, 36*2 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 218, 36*2 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 240, 36*1.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 266, 36*1.5 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 16, 36*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*3.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 44, 36*4 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 218, 36*4 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 240, 36*3.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 266, 36*3.5 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_TEXT, 16, 36*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*5.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 44, 36*6 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 218, 36*6 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 240, 36*5.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 266, 36*5.5 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 32, 36*6.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 58, 36*6.5 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_GEM, 154, 36*6.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 180, 36*6.5 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 176 - (106 / 2), 352 - 11 - 27, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitSoundOptionsMenuItems() {
    MenuitemText    i0  = { "Sound Options", MI_TFLAGS_CENTERED};

    MenuitemText    i1  = { "Master Volume", MI_TFLAGS_LALIGN};
    MenuitemHslider i2  = { 0, 11*18, 18, MasterVolumeHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i3  = { "min", MI_TFLAGS_CENTERED};
    MenuitemText    i4  = { "max", MI_TFLAGS_CENTERED};
    MenuitemGem     i5  = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetMasterPower};
    MenuitemText    i6  = { "Enabled", MI_TFLAGS_LALIGN};

    MenuitemText    i7  = { "Music Volume", MI_TFLAGS_LALIGN};
    MenuitemHslider i8  = { 0, 11*18, 18, MusicVolumeHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i9  = { "min", MI_TFLAGS_CENTERED};
    MenuitemText    i10 = { "max", MI_TFLAGS_CENTERED};
    MenuitemGem     i11 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetMusicPower};
    MenuitemText    i12 = { "Enabled", MI_TFLAGS_LALIGN};

    MenuitemText    i13 = { "CD Volume", MI_TFLAGS_LALIGN};
    MenuitemHslider i14 = { 0, 11*18, 18, CdVolumeHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i15 = { "min", MI_TFLAGS_CENTERED};
    MenuitemText    i16 = { "max", MI_TFLAGS_CENTERED};
    MenuitemGem     i17 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetCdPower};
    MenuitemText    i18 = { "Enabled", MI_TFLAGS_LALIGN};
    MenuitemGem     i19 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_ROUND, SetCdModeAll};
    MenuitemText    i20 = { "All Tracks", MI_TFLAGS_LALIGN};
    MenuitemGem     i21 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_ROUND, SetCdModeRandom};
    MenuitemText    i22 = { "Random Tracks", MI_TFLAGS_LALIGN};
    MenuitemButton  i23 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};

    SoundOptionsMenuItems[0].d.text     = i0;
    SoundOptionsMenuItems[1].d.text     = i1;
    SoundOptionsMenuItems[2].d.hslider  = i2;
    SoundOptionsMenuItems[3].d.text     = i3;
    SoundOptionsMenuItems[4].d.text     = i4;
    SoundOptionsMenuItems[5].d.gem      = i5;
    SoundOptionsMenuItems[6].d.text     = i6;
    SoundOptionsMenuItems[7].d.text     = i7;
    SoundOptionsMenuItems[8].d.hslider  = i8;
    SoundOptionsMenuItems[9].d.text     = i9;
    SoundOptionsMenuItems[10].d.text    = i10;
    SoundOptionsMenuItems[11].d.gem     = i11;
    SoundOptionsMenuItems[12].d.text    = i12;
    SoundOptionsMenuItems[13].d.text    = i13;
    SoundOptionsMenuItems[14].d.hslider = i14;
    SoundOptionsMenuItems[15].d.text    = i15;
    SoundOptionsMenuItems[16].d.text    = i16;
    SoundOptionsMenuItems[17].d.gem     = i17;
    SoundOptionsMenuItems[18].d.text    = i18;
    SoundOptionsMenuItems[19].d.gem     = i19;
    SoundOptionsMenuItems[20].d.text    = i20;
    SoundOptionsMenuItems[21].d.gem     = i21;
    SoundOptionsMenuItems[22].d.text    = i22;
    SoundOptionsMenuItems[23].d.button  = i23;
}

local Menuitem PreferencesMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 16, 36*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 46, 36*1 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_GEM, 16, 36*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 46, 36*2 + 2, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

    { MI_TYPE_BUTTON, 128 - (106 / 2), 245, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitPreferencesMenuItems() {
    MenuitemText   i0 = { "Preferences", MI_TFLAGS_CENTERED};
    MenuitemGem    i1 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetFogOfWar};
    MenuitemText   i2 = { "Fog of War Enabled", MI_TFLAGS_LALIGN};
    MenuitemGem    i3 = { MI_GSTATE_UNCHECKED, 18, 18, MBUTTON_GEM_SQUARE, SetCommandKey};
    MenuitemText   i4 = { "Show command key", MI_TFLAGS_LALIGN};
    MenuitemButton i5 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};
    PreferencesMenuItems[0].d.text   = i0;
    PreferencesMenuItems[1].d.gem    = i1;
    PreferencesMenuItems[2].d.text   = i2;
    PreferencesMenuItems[3].d.gem    = i3;
    PreferencesMenuItems[4].d.text   = i4;
    PreferencesMenuItems[5].d.button = i5;
}

local Menuitem SpeedSettingsMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 16, 36*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*1.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 34, 36*2 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 230, 36*2 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 16, 36*3, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*3.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 34, 36*4 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 230, 36*4 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 16, 36*5, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_HSLIDER, 32, 36*5.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 34, 36*6 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 230, 36*6 + 6, 0, SmallFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 128 - (106 / 2), 245, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitSpeedSettingsMenuItems() {
    MenuitemText    i0  = { "Speed Settings", MI_TFLAGS_CENTERED};
    MenuitemText    i1  = { "Game Speed", MI_TFLAGS_LALIGN};
    MenuitemHslider i2  = { 0, 11*18, 18, GameSpeedHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i3  = { "slow", MI_TFLAGS_LALIGN};
    MenuitemText    i4  = { "fast", MI_TFLAGS_RALIGN};
    MenuitemText    i5  = { "Mouse Scroll", MI_TFLAGS_LALIGN};
    MenuitemHslider i6  = { 0, 11*18, 18, MouseScrollHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i7  = { "off", MI_TFLAGS_LALIGN};
    MenuitemText    i8  = { "fast", MI_TFLAGS_RALIGN};
    MenuitemText    i9  = { "Keyboard Scroll", MI_TFLAGS_LALIGN};
    MenuitemHslider i10 = { 0, 11*18, 18, KeyboardScrollHSAction, -1, 0, 0, 0, ScenSelectOk};
    MenuitemText    i11 = { "off", MI_TFLAGS_LALIGN};
    MenuitemText    i12 = { "fast", MI_TFLAGS_RALIGN};
    MenuitemButton  i13 = { "~!OK", 106, 27, MBUTTON_GM_HALF, EndMenu, 'o'};
    SpeedSettingsMenuItems[0].d.text     = i0;
    SpeedSettingsMenuItems[1].d.text     = i1;
    SpeedSettingsMenuItems[2].d.hslider  = i2;
    SpeedSettingsMenuItems[3].d.text     = i3;
    SpeedSettingsMenuItems[4].d.text     = i4;
    SpeedSettingsMenuItems[5].d.text     = i5;
    SpeedSettingsMenuItems[6].d.hslider  = i6;
    SpeedSettingsMenuItems[7].d.text     = i7;
    SpeedSettingsMenuItems[8].d.text     = i8;
    SpeedSettingsMenuItems[9].d.text     = i9;
    SpeedSettingsMenuItems[10].d.hslider = i10;
    SpeedSettingsMenuItems[11].d.text    = i11;
    SpeedSettingsMenuItems[12].d.text    = i12;
    SpeedSettingsMenuItems[13].d.button  = i13;
}

local Menuitem GameOptionsMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#ifdef WITH_SOUND
    { MI_TYPE_BUTTON, 16, 40 + 36*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#else
    { MI_TYPE_BUTTON, 16, 40 + 36*0, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
#endif
    { MI_TYPE_BUTTON, 16, 40 + 36*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 128 - (224 / 2), 245, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitGameOptionsMenuItems() {
    MenuitemText   i0 = { "Game Options", MI_TFLAGS_CENTERED};
    MenuitemButton i1 = { "Sound (~!F~!7)", 224, 27, MBUTTON_GM_FULL, SoundOptions, KeyCodeF7};
    MenuitemButton i2 = { "Speeds (~!F~!8)", 224, 27, MBUTTON_GM_FULL, SpeedSettings, KeyCodeF8};
    MenuitemButton i3 = { "Preferences (~!F~!9)", 224, 27, MBUTTON_GM_FULL, Preferences, KeyCodeF9};
    MenuitemButton i4 = { "Previous (~!E~!s~!c)", 224, 27, MBUTTON_GM_FULL, EndMenu, '\033'};
    GameOptionsMenuItems[0].d.text   = i0;
    GameOptionsMenuItems[1].d.button = i1;
    GameOptionsMenuItems[2].d.button = i2;
    GameOptionsMenuItems[3].d.button = i3;
    GameOptionsMenuItems[4].d.button = i4;
}

local Menuitem HelpMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36*1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 128 - (224 / 2), 288-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitHelpMenuItems() {
    MenuitemText   i0 = { "Help Menu", MI_TFLAGS_CENTERED};
    MenuitemButton i1 = { "Keystroke ~!Help", 224, 27, MBUTTON_GM_FULL, KeystrokeHelpMenu, 'h'};
    MenuitemButton i2 = { "Freecraft ~!Tips", 224, 27, MBUTTON_GM_FULL, ShowTipsMenu, 't'};
    MenuitemButton i3 = { "Previous (~!E~!s~!c)", 224, 27, MBUTTON_GM_FULL, EndMenu, '\033'};
    HelpMenuItems[0].d.text   = i0;
    HelpMenuItems[1].d.button = i1;
    HelpMenuItems[2].d.button = i2;
    HelpMenuItems[3].d.button = i3;
}

local Menuitem KeystrokeHelpMenuItems[] = {
    { MI_TYPE_TEXT, 352/2, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 352 - 18 - 16, 40+20, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 352/2 - (224 / 2), 352-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_DRAWFUNC, 16, 40+20, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitKeystrokeHelpMenuItems() {
    MenuitemText     i0 = { "Keystroke Help Menu", MI_TFLAGS_CENTERED};
    MenuitemVslider  i1 = { 0, 18, 12*18, KeystrokeHelpVSAction, -1, 0, 0, 0, NULL};
    MenuitemButton   i2 = { "Previous (~!E~!s~!c)", 224, 27, MBUTTON_GM_FULL, EndMenu, '\033'};
    MenuitemDrawfunc i3 = { KeystrokeHelpDrawFunc };
    KeystrokeHelpMenuItems[0].d.text     = i0;
    KeystrokeHelpMenuItems[1].d.vslider  = i1;
    KeystrokeHelpMenuItems[2].d.button   = i2;
    KeystrokeHelpMenuItems[3].d.drawfunc = i3;
}

local Menuitem SaveGameMenuItems[] = {
    { MI_TYPE_TEXT, 384/2, 11, 0, LargeFont, InitSaveGameMenu, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_INPUT, 16, 11+36*1, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_LISTBOX, 16, 11+36*1.5, 0, GameFont, SaveLBInit, SaveLBExit, NULL, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 384-16-16, 11+36*1.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 2*384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 3*384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitSaveGameMenuItems() {
    MenuitemText    i0 = { "Save Game", MI_TFLAGS_CENTERED};
    MenuitemInput   i1 = { NULL, 384-16-16, 16, MBUTTON_PULLDOWN, EnterSaveGameAction, 0, 0};
    MenuitemListbox i2 = { NULL, 384-16-16-16, 7*18, MBUTTON_PULLDOWN, SaveLBAction, 0, 0, 0, 0, 7, 0, 0,
			   (void *)SaveLBRetrieve, ScenSelectOk};
    MenuitemVslider i3 = { 0, 18, 7*18, SaveVSAction, -1, 0, 0, 0, SaveOk};
    MenuitemButton  i4 = { "Save (~<F11~>)", 106, 27, MBUTTON_GM_HALF, SaveAction, KeyCodeF11};
    MenuitemButton  i5 = { "Delete (~<F5~>)", 106, 27, MBUTTON_GM_HALF, FcDeleteMenu, KeyCodeF5};
    MenuitemButton  i6 = { "Cancel (~<Esc~>)", 106, 27, MBUTTON_GM_HALF, EndMenu, '\033'};
    SaveGameMenuItems[0].d.text    = i0;
    SaveGameMenuItems[1].d.input   = i1;
    SaveGameMenuItems[2].d.listbox = i2;
    SaveGameMenuItems[3].d.vslider = i3;
    SaveGameMenuItems[4].d.button  = i4;
    SaveGameMenuItems[5].d.button  = i5;
    SaveGameMenuItems[6].d.button  = i6;
}

local Menuitem LoadGameMenuItems[] = {
    { MI_TYPE_TEXT, 384/2, 11, 0, LargeFont, InitLoadGameMenu, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_LISTBOX, 16, 11+36*1.5, 0, GameFont, LoadLBInit, LoadLBExit, NULL, {{NULL,0}} },
    { MI_TYPE_VSLIDER, 384-16-16, 11+36*1.5, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 3*384/3 - 106 - 10, 256-16-27, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitLoadGameMenuItems() {
    MenuitemText    i0 = { "Load Game", MI_TFLAGS_CENTERED};
    MenuitemListbox i1 = { NULL, 384-16-16-16, 7*18, MBUTTON_PULLDOWN, LoadLBAction, 0, 0, 0, 0, 7, 0, 0,
			   (void *)LoadLBRetrieve, ScenSelectOk};
    MenuitemVslider i2 = { 0, 18, 7*18, LoadVSAction, -1, 0, 0, 0, LoadOk};
    MenuitemButton  i3 = { "Load (~<F12~>)", 106, 27, MBUTTON_GM_HALF, LoadAction, KeyCodeF12};
    MenuitemButton  i4 = { "~!Cancel (~<Esc~>)", 106, 27, MBUTTON_GM_HALF, EndMenu, '\033'};
    LoadGameMenuItems[0].d.text    = i0;
    LoadGameMenuItems[1].d.listbox = i1;
    LoadGameMenuItems[2].d.vslider = i2;
    LoadGameMenuItems[3].d.button  = i3;
    LoadGameMenuItems[4].d.button  = i4;
}

local Menuitem ConfirmSaveMenuItems[] = {
    { MI_TYPE_TEXT, 288/2, 11, 0, LargeFont, ConfirmSaveInit, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*1.5, 0, GameFont, NULL, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*2.5, 0, GameFont, NULL, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_BUTTON, 16, 128-27*1.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 288-16-106, 128-27*1.5, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitConfirmSaveMenuItems() {
    MenuitemText    i0 = { "Overwrite File", MI_TFLAGS_CENTERED};
    MenuitemText    i1 = { "Are you sure you want to overwrite", MI_TFLAGS_LALIGN};
    MenuitemText    i2 = { NULL, MI_TFLAGS_LALIGN};
    MenuitemButton  i3 = { "~!Ok", 106, 27, MBUTTON_GM_HALF, ConfirmSaveFile, 'o'};
    MenuitemButton  i4 = { "~!Cancel", 106, 27, MBUTTON_GM_HALF, EndMenu, 'c'};
    ConfirmSaveMenuItems[0].d.text = i0;
    ConfirmSaveMenuItems[1].d.text = i1;
    ConfirmSaveMenuItems[2].d.text = i2;
    ConfirmSaveMenuItems[3].d.button = i3;
    ConfirmSaveMenuItems[4].d.button = i4;
}

local Menuitem ConfirmDeleteMenuItems[] = {
    { MI_TYPE_TEXT, 288/2, 11, 0, LargeFont, FcDeleteInit, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*1.5, 0, GameFont, NULL, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_TEXT, 16, 11+20*2.5, 0, GameFont, NULL, NULL, NULL, {{NULL, 0}} },
    { MI_TYPE_BUTTON, 16, 128-27*1.5, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 288-16-106, 128-27*1.5, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitConfirmDeleteMenuItems() {
    MenuitemText    i0 = { "Delete File", MI_TFLAGS_CENTERED};
    MenuitemText    i1 = { "Are you sure you want to delete", MI_TFLAGS_LALIGN};
    MenuitemText    i2 = { NULL, MI_TFLAGS_LALIGN};
    MenuitemButton  i3 = { "~!Ok", 106, 27, MBUTTON_GM_HALF, FcDeleteFile, 'o'};
    MenuitemButton  i4 = { "~!Cancel", 106, 27, MBUTTON_GM_HALF, EndMenu, 'c'};
    ConfirmDeleteMenuItems[0].d.text = i0;
    ConfirmDeleteMenuItems[1].d.text = i1;
    ConfirmDeleteMenuItems[2].d.text = i2;
    ConfirmDeleteMenuItems[3].d.button = i3;
    ConfirmDeleteMenuItems[4].d.button = i4;
}

local Menuitem EditorSelectMenuItems[] = {
    { MI_TYPE_BUTTON, 208, 320 + 36 * 0, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 320 + 36 * 1, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 208, 320 + 36 * 2, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitEditorSelectMenuItems() {
    MenuitemButton i0 = { "~!New Map", 224, 27, MBUTTON_GM_FULL, EditorNewMap, 'n'};
    MenuitemButton i1 = { "~!Load Map", 224, 27, MBUTTON_GM_FULL, EditorLoadMap, 'l'};
    MenuitemButton i2 = { "~!Cancel", 224, 27, MBUTTON_GM_FULL, EndMenu, 'c'};
    EditorSelectMenuItems[0].d.button = i0;
    EditorSelectMenuItems[1].d.button = i1;
    EditorSelectMenuItems[2].d.button = i2;
}

local Menuitem EditorLoadMapMenuItems[] = {
    { MI_TYPE_TEXT, 352/2, 11, 0, LargeFont, EditorLoadInit, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_LISTBOX, (352-18-288)/2, 11+98, 0, GameFont, EditorLoadLBInit, EditorLoadLBExit, NULL, {{NULL,0}} },
    { MI_TYPE_VSLIDER, (352-18-288)/2+288, 11+98, 0, 0, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 48, 308, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 198, 308, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, (352-18-288)/2-2, 11+98-28, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },

};
local void InitEditorLoadMapMenuItems() {
    MenuitemText    i0 = { "Select map", MI_TFLAGS_CENTERED};

    MenuitemListbox  i1 = { NULL, 288, 6*18, MBUTTON_PULLDOWN, EditorLoadLBAction, 0, 0, 0, 0, 6, 0, 0,
			    (void *)EditorLoadLBRetrieve, EditorLoadOk};
    MenuitemVslider  i2 = { 0, 18, 6*18, EditorLoadVSAction, -1, 0, 0, 0, EditorLoadOk};

    MenuitemButton   i3 = { "OK", 106, 27, MBUTTON_GM_HALF, EditorLoadOk, 0};
    MenuitemButton   i4 = { "Cancel", 106, 27, MBUTTON_GM_HALF, EditorLoadCancel, 0};
    MenuitemButton   i5 = { NULL, 36, 24, MBUTTON_FOLDER, EditorLoadFolder, 0};
    EditorLoadMapMenuItems[0].d.text     = i0;
    EditorLoadMapMenuItems[1].d.listbox  = i1;
    EditorLoadMapMenuItems[2].d.vslider  = i2;
    EditorLoadMapMenuItems[3].d.button   = i3;
    EditorLoadMapMenuItems[4].d.button   = i4;
    EditorLoadMapMenuItems[5].d.button   = i5;
}

/**
**	Items for the main Editor Menu
*/
local Menuitem EditorMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, InitGameMenu, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16 + 12 + 106, 40, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 40 + 36 + 36 + 36, MenuButtonDisabled, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40-36, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 16, 288-40, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitEditorMenuItems() {
    MenuitemText   i0 = { "Editor Menu", MI_TFLAGS_CENTERED};
    MenuitemButton i1 = { "Save (~<F11~>)", 106, 27, MBUTTON_GM_HALF, GameMenuSave, KeyCodeF11};
    MenuitemButton i2 = { "Load (~<F12~>)", 106, 27, MBUTTON_GM_HALF, GameMenuLoad, KeyCodeF12};
    MenuitemButton i3 = { "Map Properties (~<F5~>)", 224, 27, MBUTTON_GM_FULL, EditorMapProperties, KeyCodeF5};
    MenuitemButton i4 = { "Player Properties (~<F6~>)", 224, 27, MBUTTON_GM_FULL, EditorPlayerProperties, KeyCodeF6};
    MenuitemButton i5 = { "", 224, 27, MBUTTON_GM_FULL, GameMenuObjectives, 'o'};
    MenuitemButton i6 = { "E~!xit to Menu", 224, 27, MBUTTON_GM_FULL, EditorQuitMenu, 'x'};
    MenuitemButton i7 = { "Return to Editor (~<Esc~>)", 224, 27, MBUTTON_GM_FULL, GameMenuReturn, '\033'};
    EditorMenuItems[0].d.text   = i0;
    EditorMenuItems[1].d.button = i1;
    EditorMenuItems[2].d.button = i2;
    EditorMenuItems[3].d.button = i3;
    EditorMenuItems[4].d.button = i4;
    EditorMenuItems[5].d.button = i5;
    EditorMenuItems[6].d.button = i6;
    EditorMenuItems[7].d.button = i7;
}

local unsigned char *mptssoptions[] = {
    "Forest",
    "Winter",
    "Wasteland",
    "Orc Swamp",
};

local unsigned char *veroptions[] = {
    "Original",
    "Expansion",
    "FreeCraft",
};

local Menuitem EditorMapPropertiesMenuItems[] = {
    { MI_TYPE_TEXT, 288/2, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, (288-260)/2, 11+36, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_INPUT, (288-260)/2, 11+36+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, (288-260)/2, 11+36*2+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, 288-(288-260)/2-152, 11+36*2+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, (288-260)/2, 11+36*3+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 288-(288-260)/2-152, 11+36*3+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_TEXT, (288-260)/2, 11+36*4+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_PULLDOWN, 288-(288-260)/2-152, 11+36*4+22, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, (288-106*2)/4, 256 - 11 - 27, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_BUTTON, 288-(288-106*2)/4 - 106, 256 - 11 - 27, MenuButtonSelected, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitEditorMapPropertiesMenuItems() {
    MenuitemText     i0  = { "Map Properties", MI_TFLAGS_CENTERED};
    MenuitemText     i1  = { "Map Description:", MI_TFLAGS_LALIGN};
    MenuitemInput    i2  = { NULL, 260, 16, MBUTTON_PULLDOWN, EditorEnterMapDescriptionAction, 0, 0};
    MenuitemText     i3  = { "Size:", MI_TFLAGS_LALIGN};
    MenuitemText     i4  = { NULL, MI_TFLAGS_LALIGN};
    MenuitemText     i5  = { "Tileset:", MI_TFLAGS_LALIGN};
    MenuitemPulldown i6  = { mptssoptions, 152, 20, MBUTTON_PULLDOWN, NULL, 4, 0, 0, 0, 0};
    MenuitemText     i7  = { "Version:", MI_TFLAGS_LALIGN};
    MenuitemPulldown i8  = { veroptions, 152, 20, MBUTTON_PULLDOWN, NULL, 3, 0, 0, 0, 0};
    MenuitemButton   i9  = { "OK", 106, 27, MBUTTON_GM_HALF, EditorMapPropertiesOk, 0};
    MenuitemButton   i10 = { "Cancel (~<Esc~>)", 106, 27, MBUTTON_GM_HALF, EditorEndMenu, '\033'};
    EditorMapPropertiesMenuItems[0].d.text     = i0;
    EditorMapPropertiesMenuItems[1].d.text     = i1;
    EditorMapPropertiesMenuItems[2].d.input    = i2;
    EditorMapPropertiesMenuItems[3].d.text     = i3;
    EditorMapPropertiesMenuItems[4].d.text     = i4;
    EditorMapPropertiesMenuItems[5].d.text     = i5;
    EditorMapPropertiesMenuItems[6].d.pulldown = i6;
    EditorMapPropertiesMenuItems[7].d.text     = i7;
    EditorMapPropertiesMenuItems[8].d.pulldown = i8;
    EditorMapPropertiesMenuItems[9].d.button   = i9;
    EditorMapPropertiesMenuItems[10].d.button  = i10;
}

local Menuitem EditorPlayerPropertiesMenuItems[] = {
    { MI_TYPE_TEXT, 128, 11, 0, LargeFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitEditorPlayerPropertiesMenuItems() {
    MenuitemText   i0 = { "Player Properties", MI_TFLAGS_CENTERED};
    EditorPlayerPropertiesMenuItems[0].d.text   = i0;
}


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
	GameMenuItems,
	NULL,
    },
    {
	// Victory Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-128)/2,
	288, 128,
	ImagePanel4,
	2, 4,
	VictoryMenuItems,
	NULL,
    },
    {
	// Lost Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-128)/2,
	288, 128,
	ImagePanel4,
	2, 3,
	LostMenuItems,
	NULL,
    },
    {
	// SelectScen Menu
	(640-352)/2,
	(480-352)/2,
	352, 352,
	ImagePanel5,
	4, 10,
	ScenSelectMenuItems,
	NULL,
    },
    {
	// PrgStart Menu
	0,
	0,
	640, 480,
	ImageNone,
	1, 9,
	PrgStartMenuItems,
	NULL,
    },
    {
	// CustomGame Menu
	0,
	0,
	640, 480,
	ImageNone,
	3, 17,
	CustomGameMenuItems,
	NULL,
    },
    {
	// Enter Name Menu
	(640-288)/2,
	260,
	288, 128,
	ImagePanel4,
	2, 4,
	EnterNameMenuItems,
	NULL,
    },
    {
	// Net Create Join Menu
	0,
	0,
	640, 480,
	ImageNone,
	2, 3,
	NetCreateJoinMenuItems,
	NULL,
    },
    {
	// Multi Player Setup Menu
	0,
	0,
	640, 480,
	ImageNone,
	3, 58,
	NetMultiSetupMenuItems,
	NULL,
    },
    {
	// Enter Server Menu
	(640-288)/2,
	260,
	288, 128,
	ImagePanel4,
	3, 4,
	EnterServerIPMenuItems,
	NULL,
    },
    {
	// Multi Player Client Menu
	0,
	0,
	640, 480,
	ImageNone,
	4, 44,
	NetMultiClientMenuItems,
	TerminateNetConnect,
    },
    {
	// Connecting Menu
	(640-288)/2,
	260,
	288, 128,
	ImagePanel4,
	2, 4,
	ConnectingMenuItems,
	TerminateNetConnect,
    },
    {
	// Campaign Select Menu
	0,
	0,
	640, 480,
	ImageNone,
	0, 6,
	CampaignSelectMenuItems,
	NULL,
    },
    {
	// Campaign Continue Menu
	0,
	0,
	640, 480,
	ImageNone,
	0, 1,
	CampaignContMenuItems,
	NULL,
    },
    {
	// Objectives Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	10, 11,
	ObjectivesMenuItems,
	NULL,
    },
    {
	// End Scenario Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	5, 6,
	EndScenarioMenuItems,
	NULL,
    },
    {
	// Sound Options Menu
	176+(14*TileSizeX-352)/2,
	16+(14*TileSizeY-352)/2,
	352, 352,
	ImagePanel5,
	24, 24,
	SoundOptionsMenuItems,
	NULL,
    },
    {
	// Preferences Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	6, 6,
	PreferencesMenuItems,
	NULL,
    },
    {
	// Speed Settings Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	14, 14,
	SpeedSettingsMenuItems,
	NULL,
    },
    {
	// Game Options Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	4, 5,
	GameOptionsMenuItems,
	NULL,
    },
    {
	// Net Errors Menu
	(640-288)/2,
	260,
	288, 128,
	ImagePanel4,
	2, 3,
	NetErrorMenuItems,
	NULL,
    },
    {
	// Tips Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-256)/2,
	288, 256,
	ImagePanel2,
	4, 13,
	TipsMenuItems,
	NULL,
    },
    {
	// Help Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	3, 4,
	HelpMenuItems,
	NULL,
    },
    {
	// Keystroke Help Menu
	176+(14*TileSizeX-352)/2,
	16+(14*TileSizeY-352)/2,
	352, 352,
	ImagePanel5,
	2, 4,
	KeystrokeHelpMenuItems,
	NULL,
    },
    {
	// Save Menu
	176+(14*TileSizeX-384)/2,
	16+(14*TileSizeY-256)/2,
	384, 256,
	ImagePanel3,
	6, 7,
	SaveGameMenuItems,
	NULL,
    },
    {
	// Load Menu
	176+(14*TileSizeX-384)/2,
	16+(14*TileSizeY-256)/2,
	384, 256,
	ImagePanel3,
	4, 5,
	LoadGameMenuItems,
	NULL,
    },
    {
	// Confirm Save Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-256)/2,
	288, 128,
	ImagePanel4,
	1, 5,
	ConfirmSaveMenuItems,
	NULL,
    },
    {
	// Confirm Delete Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-256)/2,
	288, 128,
	ImagePanel4,
	1, 5,
	ConfirmDeleteMenuItems,
	NULL,
    },
    {
	// Editor Select Menu
	0,
	0,
	640, 480,
	ImageNone,
	0, 3,
	EditorSelectMenuItems,
	NULL,
    },
    {
	// Editor Load Map Menu
	(640-352)/2,
	(480-352)/2,
	352, 352,
	ImagePanel5,
	4, 6,
	EditorLoadMapMenuItems,
	NULL,
    },
    {
	// Editor Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	7, 8,
	EditorMenuItems,
	NULL,
    },
    {
	// Editor Map Properties Menu
	176+(14*TileSizeX-288)/2,
	16+(14*TileSizeY-256)/2,
	288, 256,
	ImagePanel2,
	10, 11,
	EditorMapPropertiesMenuItems,
	NULL,
    },
    {
	// Editor Player Properties Menu
	176+(14*TileSizeX-256)/2,
	16+(14*TileSizeY-288)/2,
	256, 288,
	ImagePanel1,
	0, 1,
	EditorPlayerPropertiesMenuItems,
	NULL,
    },
};

/**
**	Initialize the old style menu data
*/
local void InitOldMenuData(void)
{
    // There go all my Gnuish compile time inits -
    // Why is ANSI C so dumb that it cannot
    // even initialize unions sanely?
    InitGameMenuItems();
    InitVictoryMenuItems();
    InitLostMenuItems();
    InitTipsMenuItems();
    InitObjectivesMenuItems();
    InitEndScenarioMenuItems();
    InitScenSelectMenuItems();
    InitPrgStartMenuItems();
    InitCustomGameMenuItems();
    InitEnterNameMenuItems();
    InitEnterServerIPMenuItems();
    InitNetCreateJoinMenuItems();
    InitNetMultiSetupMenuItems();
    InitNetMultiClientMenuItems();
    InitNetErrorMenuItems();
    InitConnectingMenuItems();
    InitCampaignSelectMenuItems();
    InitCampaignContMenuItems();
    InitSoundOptionsMenuItems();
    InitPreferencesMenuItems();
    InitSpeedSettingsMenuItems();
    InitGameOptionsMenuItems();
    InitHelpMenuItems();
    InitKeystrokeHelpMenuItems();
    InitSaveGameMenuItems();
    InitLoadGameMenuItems();
    InitConfirmSaveMenuItems();
    InitConfirmDeleteMenuItems();
    InitEditorSelectMenuItems();
    InitEditorLoadMapMenuItems();
    InitEditorMenuItems();
    InitEditorMapPropertiesMenuItems();
    InitEditorPlayerPropertiesMenuItems();

    *(Menu **)hash_add(MenuHash,"menu-game") = Menus + 0;
    *(Menu **)hash_add(MenuHash,"menu-victory") = Menus + 1;
    *(Menu **)hash_add(MenuHash,"menu-defeated") = Menus + 2;
    *(Menu **)hash_add(MenuHash,"menu-select-scenario") = Menus + 3;
    *(Menu **)hash_add(MenuHash,"menu-program-start") = Menus + 4;
    *(Menu **)hash_add(MenuHash,"menu-custom-game") = Menus + 5;
    *(Menu **)hash_add(MenuHash,"menu-enter-name") = Menus + 6;
    *(Menu **)hash_add(MenuHash,"menu-create-join-menu") = Menus + 7;
    *(Menu **)hash_add(MenuHash,"menu-multi-setup") = Menus + 8;
    *(Menu **)hash_add(MenuHash,"menu-enter-server") = Menus + 9;
    *(Menu **)hash_add(MenuHash,"menu-net-multi-client") = Menus + 10;
    *(Menu **)hash_add(MenuHash,"menu-net-connecting") = Menus + 11;
    *(Menu **)hash_add(MenuHash,"menu-campaign-select") = Menus + 12;
    *(Menu **)hash_add(MenuHash,"menu-campaign-continue") = Menus + 13;
    *(Menu **)hash_add(MenuHash,"menu-objectives") = Menus + 14;
    *(Menu **)hash_add(MenuHash,"menu-end-scenario") = Menus + 15;
    *(Menu **)hash_add(MenuHash,"menu-sound-options") = Menus + 16;
    *(Menu **)hash_add(MenuHash,"menu-preferences") = Menus + 17;
    *(Menu **)hash_add(MenuHash,"menu-settings-speed") = Menus + 18;
    *(Menu **)hash_add(MenuHash,"menu-game-options") = Menus + 19;
    *(Menu **)hash_add(MenuHash,"menu-net-error") = Menus + 20;
    *(Menu **)hash_add(MenuHash,"menu-tips") = Menus + 21;
    *(Menu **)hash_add(MenuHash,"menu-help") = Menus + 22;
    *(Menu **)hash_add(MenuHash,"menu-keystroke-help") = Menus + 23;
    *(Menu **)hash_add(MenuHash,"menu-save-game") = Menus + 24;
    *(Menu **)hash_add(MenuHash,"menu-load-game") = Menus + 25;
    *(Menu **)hash_add(MenuHash,"menu-save-confirm") = Menus + 26;
    *(Menu **)hash_add(MenuHash,"menu-delete-confirm") = Menus + 27;
    *(Menu **)hash_add(MenuHash,"menu-editor-select") = Menus + 28;
    *(Menu **)hash_add(MenuHash,"menu-editor-load-map") = Menus + 29;
    *(Menu **)hash_add(MenuHash,"menu-editor") = Menus + 30;
    *(Menu **)hash_add(MenuHash,"menu-editor-map-properties") = Menus + 31;
    *(Menu **)hash_add(MenuHash,"menu-editor-player-properties") = Menus + 32;
}
#endif	/// OLD_MENU


#ifdef SAVE_MENU_CCL	// {

typedef char char30[30];
global hashtable(char30,MENUS_MAXFUNC) MenuFuncHash2;

#undef HASHADD
#define HASHADD(x,y) { \
    *(void **)hash_add(MenuFuncHash,(y)) = (void *)(x); \
    sprintf(buf,"%p",(x)); \
    strcpy((char*)hash_add(MenuFuncHash2,buf), (y)); \
}

/**
**	FIXME: docu wrong names, global variable naming is: MenuNames
*/
local char *menu_names[] = {
    "menu-game",
    "menu-victory",
    "menu-defeated",
    "menu-select-scenario",
    "menu-program-start",
    "menu-global-options",
    "menu-custom-game",
    "menu-enter-name",
    "menu-create-join-menu",
    "menu-multi-setup",
    "menu-enter-server",
    "menu-net-multi-client",
    "menu-net-connecting",
    "menu-campaign-select",
    "menu-campaign-continue",
    "menu-objectives",
    "menu-end-scenario",
    "menu-sound-options",
    "menu-preferences",
    "menu-settings-speed",
    "menu-game-options",
    "menu-net-error",
    "menu-tips",
    "menu-help",
    "menu-keystroke-help",
    "menu-save-game",
    "menu-load-game",
    "menu-save-confirm",
    "menu-delete-confirm",
    "menu-editor-select",
    "menu-editor-load-map",
    "menu-editor",
    "menu-editor-map-properties",
    "menu-editor-player-properties",
};

/**
**	FIXME: docu wrong names, global variable naming is: MenuNames
*/
local char *menu_flags[] = {
    "",
    "'flags '(active)",
    "'flags '(clicked)",
    "",
    "'flags '(selected)",
    "",
    "",
    "",
    "'flags '(disabled)",
};

/**
**	FIXME: docu wrong names, global variable naming is: MenuNames
*/
local char *images[] = {
    "none",
    "panel1",
    "panel2",
    "panel3",
    "panel4",
    "panel5",
};

/**
**	FIXME: docu wrong names, global variable naming is: MenuNames
**	FIXME: should use the names of the real fonts.
*/
local char *font_names[] = {
    "small",
    "game",
    "large",
    "small-title",
    "large-title",
};

/**
**	FIXME: docu wrong names, global variable naming is: MenuNames
*/
local char *text_flags[] = {
    "none",
    "center",
    "right",
    "",
    "left",
};

/**
**	FIXME: docu wrong names, global variable naming is: MenuNames
*/
local char *gem_state[] = {
    "unchecked",
    "passive",
    "invisible",
    "",
    "checked",
};

/**
**	FIXME: docu wrong names, global variable naming is: MenuNames
*/
local char *hotkey2str(int key, char *buf)
{
    if (!key) {
	buf[0] = '\0';
    } else if (('a' <= key && key <= 'z') || ('A' <= key && key <= 'Z')) {
	buf[0] = key;
	buf[1] = '\0';
    } else if (KeyCodeF1 <= key && key <= KeyCodeF12) {
	buf[0] = 'f';
	if (key >= KeyCodeF10) {
	    buf[1] = '1';
	    buf[2] = key-10-KeyCodeF1+1 + '0';
	    buf[3] = '\0';
	} else {
	    buf[1] = key-KeyCodeF1+1 + '0';
	    buf[2] = '\0';
	}
    } else if (key == '\033') {
	strcpy(buf,"esc");
    }
    return buf;
}

/**
**	FIXME: docu wrong names, global variable naming is: MenuNames
*/
local char *button_style(int style)
{
    switch (style) {
	case MBUTTON_MAIN:
	    return "main";
	case MBUTTON_GM_HALF:
	    return "gm-half";
	case MBUTTON_132:
	    return "132";
	case MBUTTON_GM_FULL:
	    return "gm-full";
	case MBUTTON_GEM_ROUND:
	    return "gem-round";
	case MBUTTON_GEM_SQUARE:
	    return "gem-square";
	case MBUTTON_UP_ARROW:
	    return "up-arrow";
	case MBUTTON_DOWN_ARROW:
	    return "down-arrow";
	case MBUTTON_LEFT_ARROW:
	    return "left-arrow";
	case MBUTTON_RIGHT_ARROW:
	    return "right-arrow";
	case MBUTTON_S_KNOB:
	    return "s-knob";
	case MBUTTON_S_VCONT:
	    return "s-vcont";
	case MBUTTON_S_HCONT:
	    return "s-hcont";
	case MBUTTON_PULLDOWN:
	    return "pulldown";
	case MBUTTON_VTHIN:
	    return "vthin";
	case MBUTTON_FOLDER:
	    return "folder";
    }
    fprintf(stderr,"button_style not found: %d\n", style);
    return "";
}

/**
**	Save state of units to file.
**
**	@param file	Output file.
**
**	@todo Better readable formating of output.
*/
global void SaveMenus(FILE* file)
{
    Menu *menu;
    int i, j, n;
    int offsetX, offsetY;
    char hotkey[10];
    char func[10];
    char func2[10];
    char func3[10];
    char initfunc[40];
    char exitfunc[40];
    char netaction[40];

    offsetX = (VideoWidth - 640) / 2;
    offsetY = (VideoHeight - 480) / 2;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; $Id$\n\n");

    for (i=0; i<sizeof(menu_names)/sizeof(*menu_names); ++i) {
	menu = FindMenu(menu_names[i]);
	fprintf(file,";;\n;; %s\n;;\n", menu_names[i]);
	if (menu->netaction) {
	    sprintf(func,"%p",menu->netaction);
	    sprintf(netaction," 'netaction '%s",(char*)hash_find(MenuFuncHash2,func));
	} else {
	    netaction[0] = '\0';
	}
	fprintf(file,"(define-menu 'name '%s 'geometry '(%d %d %d %d)\n"
		     "    'image '%s 'default '%d%s)\n",
		     menu_names[i],
		     menu->x - offsetX, menu->y - offsetY,
		     menu->xsize, menu->ysize,
		     images[menu->image],
		     menu->defsel,
		     netaction);
	for (j=0; j<menu->nitems; ++j) {
	    if (menu->items[j].initfunc) {
		sprintf(func,"%p",menu->items[j].initfunc);
		sprintf(initfunc," 'init '%s",(char*)hash_find(MenuFuncHash2,func));
	    } else {
		initfunc[0] = '\0';
	    }
	    if (menu->items[j].exitfunc) {
		sprintf(func,"%p",menu->items[j].exitfunc);
		sprintf(exitfunc," 'exit '%s",(char*)hash_find(MenuFuncHash2,func));
	    } else {
		exitfunc[0] = '\0';
	    }

	    fprintf(file,"(define-menu-item 'pos (list %d %d) 'font '%s %s%s%s\n",
			 menu->items[j].xofs, menu->items[j].yofs,
			 font_names[menu->items[j].font],
			 menu_flags[menu->items[j].flags],
			 initfunc,
			 exitfunc);
	    switch (menu->items[j].mitype) {
		case MI_TYPE_TEXT:
		    if (menu->items[j].d.text.text) {
			fprintf(file,"    'text '(\"%s\" %s)\n",
			    (char*)menu->items[j].d.text.text
			    text_flags[menu->items[j].d.text.tflags]);
		    } else {
			fprintf(file,"    'text '(() %s)\n",
			    text_flags[menu->items[j].d.text.tflags]);
		    }
		    break;
		case MI_TYPE_BUTTON:
		    sprintf(func,"%p",menu->items[j].d.button.handler);
		    // FIXME: Johns "null" is not good, use nil or ()
		    //		or use no style item.
		    fprintf(file,"    'button '(size (%d %d)\n"
			         "            caption %c%s%c\n"
				 "            hotkey \"%s\"\n"
				 "            func %s\n"
				 "            style %s)\n",
				 menu->items[j].d.button.xsize,
				 menu->items[j].d.button.ysize,
				 menu->items[j].d.button.text ? '"' : '('
				 menu->items[j].d.button.text
				    ?  menu->items[j].d.button.text
				    : (unsigned char*)NULL,
				 menu->items[j].d.button.text ? '"' : ')'
				 hotkey2str(menu->items[j].d.button.hotkey,
					hotkey),
				 (char*)hash_find(MenuFuncHash2,func),
				 button_style(menu->items[j].d.button.button));
		    break;
		case MI_TYPE_PULLDOWN:
		    sprintf(func,"%p",menu->items[j].d.pulldown.action);
		    fprintf(file,"    'pulldown '(size (%d %d)\n"
			         "              style %s\n"
				 "              func %s\n",
				 menu->items[j].d.pulldown.xsize,
				 menu->items[j].d.pulldown.ysize,
				 button_style(menu->items[j].d.pulldown.button),
				 (char*)hash_find(MenuFuncHash2,func));
		    fprintf(file,"              options (");
		    for (n=0; n<menu->items[j].d.pulldown.noptions; ++n) {
			fprintf(file,"\"%s\" ", menu->items[j].d.pulldown.options[n]);
		    }

		    fprintf(file,")\n");
		    if (menu->items[j].d.pulldown.state & MI_PSTATE_PASSIVE) {
			fprintf(file,"              state passive\n");
		    }
		    fprintf(file,"              default %d\n"
				 "              current %d)\n",
				 menu->items[j].d.pulldown.defopt,
				 menu->items[j].d.pulldown.curopt);
		    break;
		case MI_TYPE_LISTBOX:
		    sprintf(func,"%p",menu->items[j].d.listbox.action);
		    sprintf(func2,"%p",menu->items[j].d.listbox.retrieveopt);
		    sprintf(func3,"%p",menu->items[j].d.listbox.handler);
		    fprintf(file,"    'listbox '(size (%d %d)\n"
				 "             style %s\n"
				 "             func %s\n"
				 "             retopt %s\n"
				 "             handler %s\n"
				 "             nlines %d)\n",
				 menu->items[j].d.listbox.xsize,
				 menu->items[j].d.listbox.ysize,
				 button_style(menu->items[j].d.listbox.button),
				 (char*)hash_find(MenuFuncHash2,func),
				 (char*)hash_find(MenuFuncHash2,func2),
				 (char*)hash_find(MenuFuncHash2,func3),
				 menu->items[j].d.listbox.nlines);
		    break;
		case MI_TYPE_VSLIDER:
		    sprintf(func,"%p",menu->items[j].d.vslider.action);
		    sprintf(func2,"%p",menu->items[j].d.vslider.handler);
		    fprintf(file,"    'vslider '(size (%d %d)\n"
			         "             func %s\n"
				 "             handler %s)\n",
				 menu->items[j].d.vslider.xsize,
				 menu->items[j].d.vslider.ysize,
				 (char*)hash_find(MenuFuncHash2,func),
				 (char*)hash_find(MenuFuncHash2,func2));
		    break;
		case MI_TYPE_DRAWFUNC:
		    sprintf(func,"%p",menu->items[j].d.drawfunc.draw);
		    fprintf(file,"    'drawfunc '%s\n",
			         (char*)hash_find(MenuFuncHash2,func));
		    break;
		case MI_TYPE_INPUT:
		    sprintf(func,"%p",menu->items[j].d.input.action);
		    fprintf(file,"    'input '(size (%d %d)\n"
			         "           func %s\n"
				 "           style %s)\n",
				 menu->items[j].d.input.xsize,
				 menu->items[j].d.input.ysize,
				 (char*)hash_find(MenuFuncHash2,func),
				 button_style(menu->items[j].d.input.button));
		    break;
		case MI_TYPE_GEM:
		    sprintf(func,"%p",menu->items[j].d.gem.action);
		    fprintf(file,"    'gem '(size (%d %d)\n"
			         "         state %s\n"
				 "         func %s\n"
				 "         style %s)\n",
				 menu->items[j].d.gem.xsize,
				 menu->items[j].d.gem.ysize,
				 gem_state[menu->items[j].d.gem.state],
				 (char*)hash_find(MenuFuncHash2,func),
				 button_style(menu->items[j].d.gem.button));
		    break;
		case MI_TYPE_HSLIDER:
		    sprintf(func,"%p",menu->items[j].d.hslider.action);
		    sprintf(func2,"%p",menu->items[j].d.hslider.handler);
		    fprintf(file,"    'hslider '(size (%d %d)\n"
			         "             func %s\n"
				 "             handler %s)\n",
				 menu->items[j].d.hslider.xsize,
				 menu->items[j].d.hslider.ysize,
				 (char*)hash_find(MenuFuncHash2,func),
				 (char*)hash_find(MenuFuncHash2,func2));
		    break;
		default:
		    abort();
	    }
	    fprintf(file,"    'menu '%s)\n",menu_names[i]);
	}
	fprintf(file,"\n\n");
    }
}
#endif	// } SAVE_MENU_CCL


    /// Offsets into NetMultiSetupMenuItems
#define SERVER_PLAYER_STATE	5
    /// Offsets into NetMultiSetupMenuItems
#define SERVER_PLAYER_READY	32
    /// Offsets into NetMultiSetupMenuItems
#define SERVER_PLAYER_LAG	46

    /// Offsets into NetMultiClientMenuItems
#define CLIENT_PLAYER_STATE	5
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_RACE		21
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_RESOURCE		23
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_UNITS		25
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_FOG_OF_WAR	27
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_TILESET		29
	/// Offsets into NetMultiClientMenuItems
#define CLIENT_GAMETYPE		31
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_PLAYER_READY	32

//@}
