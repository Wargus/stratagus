//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
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
#include "etlib/hash.h"

/*----------------------------------------------------------------------------
--	Defines/Declarations
----------------------------------------------------------------------------*/

// DISABLED grays out the menu item
#define MI_DISABLED		-1
#define MI_ENABLED		0

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
**	For gems: -1 == disabled, +1 == depressed, +2 checked,
**	+3 checked+depressed
*/
typedef int MenuButtonId;

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
struct _menuitem_;
typedef struct _menuitem_text_ {
    unsigned char *text;
    unsigned int tflags;
} MenuitemText;
typedef struct _menuitem_button_ {
    unsigned char *text;
    int xsize;
    int ysize;
    MenuButtonId button;
    void (*handler)(void);
    unsigned hotkey;
} MenuitemButton;
typedef struct _menuitem_pulldown_ {
    unsigned char **options;
    int xsize;
    int ysize;
    MenuButtonId button;
    void (*action)(struct _menuitem_ *, int);
    int noptions;
    int defopt;
    int curopt;
    int cursel;		/* used in popup state */
    unsigned int state;
} MenuitemPulldown;
typedef struct _menuitem_listbox_ {
    void *options;
    int xsize;
    int ysize;
    MenuButtonId button;
    void (*action)(struct _menuitem_ *, int);
    int noptions;
    int defopt;
    int curopt;
    int cursel;		/* used in mouse-over state */
    int nlines;
    int startline;
    int dohandler;
    void *(*retrieveopt)(struct _menuitem_ *, int);
    void (*handler)(void);	/* for return key */
} MenuitemListbox;
typedef struct _menuitem_vslider_ {
    unsigned cflags;
    int xsize;		// x-size of slider, not including buttons
    int ysize;		// y-size of slider, not including buttons
    void (*action)(struct _menuitem_ *, int);
    int defper;
    int percent;	// percent of the way to bottom (0 to 100)
    int curper;		/* used in mouse-move state */
    int cursel;		/* used in mouse-over state */
    void (*handler)(void);	/* for return key */
} MenuitemVslider;
typedef struct _menuitem_hslider_ {
    unsigned cflags;
    int xsize;		// x-size of slider, not including buttons
    int ysize;		// y-size of slider, not including buttons
    void (*action)(struct _menuitem_ *, int);
    int defper;
    int percent;	// percent of the way to right (0 to 100)
    int curper;		/* used in mouse-move state */
    int cursel;		/* used in mouse-over state */
    void (*handler)(void);	/* for return key */
} MenuitemHslider;
typedef struct _menuitem_drawfunc_ {
    void (*draw)(struct _menuitem_ *);
} MenuitemDrawfunc;
typedef struct _menuitem_input_ {
    unsigned char *buffer;
    int xsize;
    int ysize;
    MenuButtonId button;
    void (*action)(struct _menuitem_ *, int);	/* for key */
    int nch;
    int maxch;
} MenuitemInput;
typedef struct _menuitem_gem_ {
    unsigned int state;
    int xsize;
    int ysize;
    MenuButtonId button;
    void (*action)(struct _menuitem_ *);
} MenuitemGem;

struct _menus_;
typedef struct _menuitem_ {
    int mitype;					/// FIXME: write docu
    int xofs;
    int yofs;
    unsigned flags;
    int font;
    void (*initfunc)(struct _menuitem_ *);	/// constructor
    void (*exitfunc)(struct _menuitem_ *);	/// destructor
    struct _menus_ *menu;			/// backpointer for speedups
    union {
	MenuitemText text;
	MenuitemButton button;
	MenuitemPulldown pulldown;
	MenuitemListbox listbox;
	MenuitemVslider vslider;
	MenuitemHslider hslider;
	MenuitemDrawfunc drawfunc;
	MenuitemInput input;
	MenuitemGem gem;
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
#define MI_PSTATE_PASSIVE 1		/// Pulldown is passive (grey) drawn

    /// for MI_TYPE_GEM
#define MI_GSTATE_UNCHECKED 0		/// Gem has no check mark
#define MI_GSTATE_PASSIVE 1		/// Gem is passive (grey) drawn
#define MI_GSTATE_INVISIBLE 2		/// Gem is not drawn
#define MI_GSTATE_CHECKED 4		/// Gem is with check mark drawn

/**
**	Menu definition.
*/
typedef struct _menus_ {
    // FIXME: char* Name;			/// menu name
    int x;				/// menu area x pos
    int y;				/// menu area y pos
    int xsize;				/// menu area x size
    int ysize;				/// menu area y size
    int	image;				/// optional background panel image #
    int defsel;				/// initial selected item number (or -1)
    int nitems;				/// number of items to follow
    Menuitem *items;			/// buttons, etc
    void (*netaction)(void);		/// network action callback
} Menu;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int GuiGameStarted;		/// Game Started?
extern Menu *CurrentMenu;		/// Currently processed menu

extern MapInfo *MenuMapInfo;		/// MapInfo of map used in gui menus
extern char MenuMapFullPath[1024];	/// Full path to currently selected map

extern int nKeyStrokeHelps;		/// Number of loaded keystroke helps
extern char **KeyStrokeHelps;		/// Keystroke help pairs

#define MENUS_MAXMENU 128		/// FIXME: wrong place, docu
#define MENUS_MAXFUNC 128		/// FIXME: wrong place, docu

#ifdef DOXYGEN                          // no real code, only for document

#else

    /// Hash table of all the menus
typedef hashtable(Menu*,MENUS_MAXMENU) _MenuHash;
extern _MenuHash MenuHash;
    /// Hash table of all the menu functions
typedef hashtable(void*,MENUS_MAXFUNC) _MenuFuncHash;
extern _MenuFuncHash MenuFuncHash;

#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Initialize the hash tables for the menus
extern void InitMenuFuncHash(void);

    /// Set-up menus for a specific race
extern void InitMenus(int race);

    /// Draw menu
extern void DrawMenu(Menu *menu);
    /// Draw menu button
extern void DrawMenuButton(MenuButtonId button,unsigned flags,int w,int h,int x,int y,const int font,const unsigned char *text);
    /// Set menu backgound and draw it
extern void MenusSetBackground(void);
    /// Draw and process a menu
extern void ProcessMenu(const char *MenuId, int Loop);
    /// End the current menu
extern void EndMenu(void);
    /// Find a menu by id
extern Menu *FindMenu(const char *MenuId);

    /// Invalidate previously redrawn menu areas
extern void InvalidateMenuAreas(void);

    /// The scenario path received from server, Update the client menu
extern int NetClientSelectScenario(void);
    /// State info received from server, Update the client menu.
extern void NetClientUpdateState(void);
    /// Notify menu display code to update info
extern void NetConnectForceDisplayUpdate(void);
    /// Compare Local State <-> Server's state, force Update when changes
extern void NetClientCheckLocalState(void);

    /// Sound options menu
extern void SoundOptionsMenu(void);
    /// Speed options menu
extern void SpeedOptionsMenu(void);
    /// Preferences menu
extern void PreferencesMenu(void);
    /// Diplomacy menu
extern void DiplomacyMenu(void);

    /// Save game menu
extern void SaveGameMenu(void);
    /// Load game menu
extern void LoadGameMenu(void);

    /// Restart confirm menu
extern void RestartConfirmMenu(void);
    /// Quit to menu confirm menu
extern void QuitToMenuConfirmMenu(void);
    /// Exit confirm menu
extern void ExitConfirmMenu(void);

    /// Initialize the (ccl-loaded) menus data
extern void InitMenuData(void);
    /// Post-Initialize the (ccl-loaded) menus
extern void InitMenuFunctions(void);

    /// Edit resource properties
extern void EditorEditResource(void);
    /// Edit ai properties
extern void EditorEditAiProperties(void);

    /// Save map from the editor
extern int EditorSaveMenu(void);
    /// Load map from the editor
extern void EditorLoadMenu(void);

    /// Error menu
extern void ErrorMenu(char *);

    /// Menu Loop
extern void MenuLoop(char *filename, WorldMap *map);

//@}

#endif	// !__MENUS_H__
