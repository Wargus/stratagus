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
#define MBUTTON_UP_ARROW	29
#define MBUTTON_DOWN_ARROW	32
#define MBUTTON_LEFT_ARROW	35
#define MBUTTON_RIGHT_ARROW	38
#define MBUTTON_S_KNOB		40
#define MBUTTON_S_VCONT		42
#define MBUTTON_S_HCONT		44
#define MBUTTON_PULLDOWN	46
#define MBUTTON_VTHIN		48
#define MBUTTON_FOLDER		51

/*----------------------------------------------------------------------------
--	Menus
----------------------------------------------------------------------------*/

/**
**	Menuitem definition.
*/
typedef struct _menuitem_ {
    int mitype;
    unsigned xofs;
    unsigned yofs;
    unsigned flags;
    int font;
    void (*initfunc)(struct _menuitem_ *);	// constructor
    void (*exitfunc)(struct _menuitem_ *);	// destructor
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
	/// ... add here ...

    } d;
} Menuitem;

#define MI_TYPE_TEXT 1
#define MI_TYPE_BUTTON 2
#define MI_TYPE_PULLDOWN 3
#define MI_TYPE_LISTBOX 4
#define MI_TYPE_VSLIDER 5
#define MI_TYPE_DRAWFUNC 6

    /// for MI_TYPE_TEXT
#define MI_TFLAGS_CENTERED 1
#define MI_TFLAGS_RALIGN 2

    /// for MI_TYPE_xSLIDER
#define MI_CFLAGS_UP 1
#define MI_CFLAGS_DOWN 2
#define MI_CFLAGS_LEFT MI_CURC_UP
#define MI_CFLAGS_RIGHT MI_CURC_DOWN
#define MI_CFLAGS_KNOB 4
#define MI_CFLAGS_CONT 8		/// unused right now

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
#define MENU_SCEN_SELECT 3
#define MENU_PRG_START 4
#define MENU_CUSTOM_GAME_SETUP 5
#define MENU_MAX  5			/// highest available menu id (for ccl)

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
