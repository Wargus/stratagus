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
/**@name menus.h	-	The menu headerfile. */
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

#ifndef __MENUS_H__
#define __MENUS_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "map.h"

/*----------------------------------------------------------------------------
--	Defines/Declarations
----------------------------------------------------------------------------*/

#define MenuButtonActive	1	/// cursor on button
#define MenuButtonClicked	2	/// mouse button down on button
#define MenuButtonSelected	4	/// selected button
#define MenuButtonDisabled	8	/// button cannot be depressed

#define MENUBUTTONHEIGHT 144		/// Height of button in the graphic
#define MENUBUTTONWIDTH 300		/// Width of button in the graphic

/**
**	Menu button referencing
**	Each button is 300 x 144  =>	50 buttons (53 for Expansion GFX)
**	For multi-version buttons: button - 1 == disabled, + 1 == depressed
**	For gems: -1 == disabled, +1 == depressed, +2 checked, +3 checked+depressed
*/
typedef unsigned MenuButtonId;

/// FIXME: FILL IN THIS TABLE!!!!

#define MBUTTON_MAIN		 4	/// FIXME: write docu
#define MBUTTON_GM_HALF		10
#define MBUTTON_132		13
#define MBUTTON_GM_FULL		16
#define MBUTTON_GEM_ROUND	19
#define MBUTTON_GEM_SQUARE	24
#define MBUTTON_UP_ARROW	29
#define MBUTTON_DOWN_ARROW	32
#define MBUTTON_LEFT_ARROW	35
#define MBUTTON_RIGHT_ARROW	38
#define MBUTTON_S_KNOB		40
#define MBUTTON_S_VCONT		42
#define MBUTTON_S_HCONT		44
#define MBUTTON_PULLDOWN	46
#define MBUTTON_VTHIN		48
#define MBUTTON_FOLDER		51	/* expansion gfx only */

//	For the game speed slider in the speed settings screen.
#define MIN_GAME_SPEED		50
#define MAX_GAME_SPEED		250

/*----------------------------------------------------------------------------
--	Menus
----------------------------------------------------------------------------*/

/**
**	Menuitem definition.
*/
typedef struct _menuitem_ {
    int mitype;					/// FIXME: write docu
    unsigned xofs;
    unsigned yofs;
    unsigned flags;
    int font;
    void (*initfunc)(struct _menuitem_ *);	/// constructor
    void (*exitfunc)(struct _menuitem_ *);	/// destructor
    union {
	struct {
	    unsigned char *text;
	    unsigned int tflags;
	} text;
	struct {
	    unsigned char *text;
	    unsigned int xsize;
	    unsigned int ysize;
	    MenuButtonId button;
	    void (*handler)(void);
	    int hotkey;
	} button;
	struct {
	    unsigned char **options;
	    unsigned int xsize;
	    unsigned int ysize;
	    MenuButtonId button;
	    void (*action)(struct _menuitem_ *, int);
	    int noptions;
	    int defopt;
	    int curopt;
	    int cursel;		/* used in popup state */
	    unsigned int state;
	} pulldown;
	struct {
	    void *options;
	    unsigned int xsize;
	    unsigned int ysize;
	    MenuButtonId button;
	    void (*action)(struct _menuitem_ *, int);
	    int noptions;
	    int defopt;
	    int curopt;
	    int cursel;		/* used in mouse-over state */
	    int nlines;
	    int startline;
	    void *(*retrieveopt)(struct _menuitem_ *, int);
	    void (*handler)(void);	/* for return key */
	} listbox;
	struct {
	    unsigned cflags;
	    unsigned int xsize;	// x-size of slider, not including buttons
	    unsigned int ysize;	// y-size of slider, not including buttons
	    void (*action)(struct _menuitem_ *, int);
	    int defper;
	    int percent;	// percent of the way to bottom (0 to 100)
	    int curper;		/* used in mouse-move state */
	    int cursel;		/* used in mouse-over state */
	    void (*handler)(void);	/* for return key */
	} vslider;
	struct {
	    unsigned cflags;
	    unsigned int xsize; // x-size of slider, not including buttons
	    unsigned int ysize; // y-size of slider, not including buttons
	    void (*action)(struct _menuitem_ *, int);
	    int defper;
	    int percent;	// percent of the way to right (0 to 100)
	    int curper;		/* used in mouse-move state */
	    int cursel;		/* used in mouse-over state */
	    void (*handler)(void);	/* for return key */
	} hslider;
	struct {
	    void (*draw)(struct _menuitem_ *);
	} drawfunc;
	struct {
	    unsigned char *buffer;
	    unsigned int xsize;
	    unsigned int ysize;
	    MenuButtonId button;
	    void (*action)(struct _menuitem_ *, int);	/* for key */
	    int nch;
	    int maxch;
	} input;
	struct {
	    unsigned int state;
	    unsigned int xsize;
	    unsigned int ysize;
	    MenuButtonId button;
	    void (*action)(struct _menuitem_ *);
	} gem;
	/// ... add here ...

    } d;
} Menuitem;

#define MI_TYPE_TEXT 1			/// FIXME: write docu
#define MI_TYPE_BUTTON 2
#define MI_TYPE_PULLDOWN 3
#define MI_TYPE_LISTBOX 4
#define MI_TYPE_VSLIDER 5
#define MI_TYPE_DRAWFUNC 6
#define MI_TYPE_INPUT 7
#define MI_TYPE_GEM 8
#define MI_TYPE_HSLIDER 9

    /// for MI_TYPE_TEXT
#define MI_TFLAGS_CENTERED 1
#define MI_TFLAGS_RALIGN 2
#define MI_TFLAGS_LALIGN 4

    /// for MI_TYPE_xSLIDER
#define MI_CFLAGS_UP 1
#define MI_CFLAGS_DOWN 2
#define MI_CFLAGS_LEFT 1
#define MI_CFLAGS_RIGHT 2
#define MI_CFLAGS_KNOB 4
#define MI_CFLAGS_CONT 8

    /// for MI_TYPE_PULLDOWN
#define MI_PSTATE_PASSIVE 1

    /// for MI_TYPE_GEM
#define MI_GSTATE_UNCHECKED 0
#define MI_GSTATE_PASSIVE 1
#define MI_GSTATE_INVISIBLE 2
#define MI_GSTATE_CHECKED 4

/**
**	Menu definition.
*/
typedef struct _menus_ {
    unsigned int x;			/// menu area x pos
    unsigned int y;			/// menu area y pos
    unsigned int xsize;			/// menu area x size
    unsigned int ysize;			/// menu area y size
    int	image;				/// optional background panel image #
    int defsel;				/// initial selected item number (or -1)
    int nitems;				/// number of items to follow
    Menuitem *items;			/// buttons, etc
    void (*netaction)(void);		/// network action callback
} Menu;


#define MENU_GAME 0			/// FIXME: write docu
#define MENU_VICTORY 1
#define MENU_LOST 2
#define MENU_SCEN_SELECT 3
#define MENU_PRG_START 4
#define MENU_CUSTOM_GAME_SETUP 5
#define MENU_ENTER_NAME 6
#define MENU_NET_CREATE_JOIN 7
#define MENU_NET_MULTI_SETUP 8
#define MENU_NET_ENTER_SERVER_IP 9
#define MENU_NET_MULTI_CLIENT 10
#define MENU_NET_CONNECTING 11
#define MENU_CAMPAIGN_SELECT 12
#define MENU_CAMPAIGN_CONT 13
#define MENU_OBJECTIVES 14
#define MENU_END_SCENARIO 15
#define MENU_SOUND_OPTIONS 16
#define MENU_PREFERENCES 17
#define MENU_SPEED_SETTINGS 18
#define MENU_GAME_OPTIONS 19
#define MENU_NET_ERROR 20
#define MENU_MAX 20			/// highest available menu id (for ccl)

/// FIXME: FILL IN THIS TABLE!!!!

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int CurrentMenu;			/// Currently processed menu
extern char ScenSelectFullPath[1024];	/// Full path to currently selected map
extern MapInfo *ScenSelectPudInfo;	/// MapInfo of currently selected map

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Set-up menus for a specific race
extern void InitMenus(unsigned int race);

    /// draw menu
extern void DrawMenu(int MenuId);
    /// draw menu button
extern void DrawMenuButton(MenuButtonId button,unsigned flags,unsigned w,unsigned h,unsigned x,unsigned y,const int font,const unsigned char *text);
    /// Draw and process a menu
extern void ProcessMenu(int MenuId, int Loop);
    /// Keyboard handler for menus
extern int MenuHandleKeyboard(int key, int keychar);
    /// Called if the mouse is moved in Menu interface state
extern void MenuHandleMouseMove(int x,int y);
    /// Called if any mouse button is pressed down
extern void MenuHandleButtonDown(int b);
    /// Called if any mouse button is released up
extern void MenuHandleButtonUp(int b);

    /// The scenario path received from server
    /// Update the client menu.
extern int NetClientSelectScenario(void);
    /// State info received from server
    /// Update the client menu.
extern void NetClientUpdateState(void);
    /// Notify menu display code to update info
extern void NetConnectForceDisplayUpdate(void);
    /// Compare Local State with Server's information
    /// and force Update when changes have occured.
extern void NetClientCheckLocalState(void);

    /// Sound options menu
extern void SoundOptions(void);
    /// Speed options menu
extern void SpeedSettings(void);
    /// Preferences menu
extern void Preferences(void);

//@}

#endif	// !__MENUS_H__
