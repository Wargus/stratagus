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
/*
**	(c) Copyright 1999,2000 by Andreas Arens
**
**	$Id$
*/

#ifndef __MENUS_H__
#define __MENUS_H__

//@{

/*----------------------------------------------------------------------------
--	Menubuttons
----------------------------------------------------------------------------*/

#define MenuButtonActive	1	/// cursor on button
#define MenuButtonClicked	2	/// mouse button down on button
#define MenuButtonSelected	4	/// selected button
#define MenuButtonDisabled	8	/// button cannot be depressed

#define MENUBUTTONHEIGHT 144
#define MENUBUTTONWIDTH 300
/**
**	Menu button referencing
**	Each button is 300 x 144  =>	53 buttons
**	For multi-version buttons: button - 1 == disabled, + 1 == depressed
*/
typedef unsigned MenuButtonId;

/// FIXME: FILL IN THIS TABLE!!!!

#define MBUTTON_MAIN		 4
#define MBUTTON_GM_HALF		10
#define MBUTTON_132		13
#define MBUTTON_GM_FULL		16
#define MBUTTON_DOWN_ARROW	32
#define MBUTTON_PULLDOWN	46

/*----------------------------------------------------------------------------
--	Menus
----------------------------------------------------------------------------*/

/**
**	Menuitem definition.
*/
typedef struct _menuitem_ {
    int mitype;
    union {
	struct {
	    unsigned xofs;
	    unsigned yofs;
	    unsigned flags;
	    int font;
	    unsigned char *text;
	} text;
	struct {
	    unsigned xofs;
	    unsigned yofs;
	    unsigned flags;
	    int font;
	    unsigned char *text;
	    unsigned xsize;
	    unsigned ysize;
	    MenuButtonId button;
	    void (*handler)(void);
	    int hotkey;
	} button;
	struct {
	    unsigned xofs;
	    unsigned yofs;
	    unsigned flags;
	    int font;
	    unsigned char **options;
	    unsigned xsize;
	    unsigned ysize;
	    MenuButtonId button;
	    void (*handler)(void);
	    int noptions;
	    int defopt;
	    int curopt;
	    int cursel;		/* used in popup state */
	} pulldown;
	/// ... add here ...
    } d;
} Menuitem;

#define MI_TYPE_TEXT 1
#define MI_TYPE_BUTTON 2
#define MI_TYPE_PULLDOWN 3

    /// for MI_TYPE_TEXT
#define MI_FLAGS_CENTERED 1
#define MI_FLAGS_RALIGN 2

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


#define MENU_GAME 0
#define MENU_VICTORY 1
#define MENU_LOST 2
#define MENU_SCEN_SELECT 3		/// FIXME: WIP
#define MENU_MAX  3			/// highest available menu id (for ccl)

/// FIXME: FILL IN THIS TABLE!!!!

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern  int CurrentMenu;

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
    /// Called if the mouse is moved


//@}

#endif	// !__MENUS_H__
