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
//	(c) Copyright 1999-2001 by Andreas Arens
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
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
	    unsigned tflags;
	} text;
	struct {
	    unsigned char *text;
	    unsigned xsize;
	    unsigned ysize;
	    MenuButtonId button;
	    void (*handler)(void);
	    int hotkey;
	} button;
	struct {
	    unsigned char **options;
	    unsigned xsize;
	    unsigned ysize;
	    MenuButtonId button;
	    void (*action)(struct _menuitem_ *, int);
	    int noptions;
	    int defopt;
	    int curopt;
	    int cursel;		/* used in popup state */
	} pulldown;
	struct {
	    void *options;
	    unsigned xsize;
	    unsigned ysize;
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
	    unsigned xsize;
	    unsigned ysize;
	    void (*action)(struct _menuitem_ *, int);
	    int defper;
	    int percent;
	    int curper;		/* used in mouse-move state */
	    int cursel;		/* used in mouse-over state */
	    void (*handler)(void);	/* for return key */
	} vslider;
	struct {
	    void (*draw)(struct _menuitem_ *);
	} drawfunc;
	struct {
	    unsigned char *buffer;
	    unsigned xsize;
	    unsigned ysize;
	    MenuButtonId button;
	    void (*action)(struct _menuitem_ *, int);	/* for key */
	    int nch;
	    int maxch;
	} input;
	struct {
	    unsigned state;
	    unsigned xsize;
	    unsigned ysize;
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

    /// for MI_TYPE_TEXT
#define MI_TFLAGS_CENTERED 1
#define MI_TFLAGS_RALIGN 2

    /// for MI_TYPE_xSLIDER
#define MI_CFLAGS_UP 1
#define MI_CFLAGS_DOWN 2
#define MI_CFLAGS_LEFT MI_CURC_UP
#define MI_CFLAGS_RIGHT MI_CURC_DOWN
#define MI_CFLAGS_KNOB 4
#define MI_CFLAGS_CONT 8

    /// for MI_TYPE_GEM
#define MI_GSTATE_CHECKED 1
#define MI_GSTATE_INVISIBLE 2
#define MI_GSTATE_PASSIVE 3

/**
**	Menu definition.
*/
typedef struct _menus_ {
    unsigned x;				/// menu area x pos
    unsigned y;				/// menu area y pos
    unsigned xsize;			/// menu area x size
    unsigned ysize;			/// menu area y size
    int	image;				/// optional background panel image #
    int defsel;				/// initial selected item number (or -1)
    int nitems;				/// number of items to follow
    Menuitem *items;			/// buttons, etc
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
#define MENU_MAX 8			/// highest available menu id (for ccl)

/// FIXME: FILL IN THIS TABLE!!!!

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern  int CurrentMenu;		/// Currently processed menu

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
extern int MenuKey(int key);
    /// Called if the mouse is moved in Menu interface state
extern void MenuHandleMouseMove(int x,int y);
    /// Called if any mouse button is pressed down
extern void MenuHandleButtonDown(int b);
    /// Called if any mouse button is released up
extern void MenuHandleButtonUp(int b);

//@}

#endif	// !__MENUS_H__
