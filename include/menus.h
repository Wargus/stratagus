//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name menus.h - The menu headerfile. */
//
//      (c) Copyright 1999-2006 by Andreas Arens and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//      $Id$

#ifndef __MENUS_H__
#define __MENUS_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <string>
#include <vector>
#include <map>

#include "ui.h"
#include "util.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Defines/Declarations
----------------------------------------------------------------------------*/

#define MI_FLAGS_NONE       0  /// no flags
#define MI_FLAGS_ACTIVE     1  /// cursor on item
#define MI_FLAGS_CLICKED    2  /// mouse button pressed down on item
#define MI_FLAGS_SELECTED   4  /// selected item
#define MI_FLAGS_DISABLED   8  /// grayed out item
#define MI_FLAGS_INVISIBLE 16  /// invisible item

/**
**  Menu button referencing
**  Each button is 300 x 144  => 50 buttons (53 for Expansion GFX)
**  For multi-version buttons: button - 1 == disabled, + 1 == depressed
**  For gems: -1 == disabled, +1 == depressed, +2 checked,
**  +3 checked+depressed
*/
typedef int MenuButtonId;

/// @todo FILL IN THIS TABLE!!!!

	/// Offsets into NetMultiSetupMenuItems
#define SERVER_PLAYER_STATE 5
#define SERVER_PLAYER_READY 32
#define SERVER_PLAYER_LAG   46
#define SERVER_PLAYER_TEXT  60

	/// Offsets into NetMultiClientMenuItems
#define CLIENT_PLAYER_STATE  5
#define CLIENT_RACE         21
#define CLIENT_RESOURCE     23
#define CLIENT_UNITS        25
#define CLIENT_FOG_OF_WAR   27
#define CLIENT_TILESET      29
#define CLIENT_GAMETYPE     31
#define CLIENT_PLAYER_READY 32
#define CLIENT_PLAYER_TEXT  46

class ButtonStyle;
class CMapInfo;
class CMap;
class CGraphic;
struct _menuitem_;

/*----------------------------------------------------------------------------
--  Menus
----------------------------------------------------------------------------*/

typedef void (*MenuitemTextActionType)(struct _menuitem_ *);
typedef void (*MenuitemButtonHandlerType)(void);
typedef void (*MenuitemPulldownActionType)(struct _menuitem_ *, int);
typedef void (*MenuitemInputActionType)(struct _menuitem_ *, int);
typedef void (*MenuitemCheckboxActionType)(struct _menuitem_ *);

/**
**  Menuitem definition.
**  @todo docu.
*/
typedef struct _menuitem_text_ {
	StringDesc *text;
	TextAlignment Align;
	char *normalcolor;
	char *reversecolor;
	MenuitemTextActionType action;
} MenuitemText;
typedef struct _menuitem_button_ {
	char *Text;
	ButtonStyle *Style;
	MenuitemButtonHandlerType Handler;
	unsigned HotKey;
} MenuitemButton;
typedef struct _menuitem_pulldown_ {
	char **options;
	int xsize;
	int ysize;
	MenuButtonId button;
	MenuitemPulldownActionType action;
	int noptions;
	int defopt;
	int curopt;
	int cursel;  /* used in popup state */
} MenuitemPulldown;
typedef struct _menuitem_input_ {
	char *buffer;
	unsigned int iflags;
	int xsize;
	int ysize;
	MenuButtonId button;
	MenuitemInputActionType action;  /// for key
	int nch;
	int maxch;
	char *normalcolor;
	char *reversecolor;
} MenuitemInput;
typedef struct _menuitem_checkbox_ {
	char *Text;
	unsigned int Checked : 1;
	CheckboxStyle *Style;
	MenuitemCheckboxActionType Action;
} MenuitemCheckbox;

struct _menu_;
typedef enum {
	MiTypeText, MiTypeButton, MiTypePulldown,
	MiTypeInput, MiTypeCheckbox
} MiTypeType;

typedef struct _menuitem_ {
	MiTypeType MiType;
	int XOfs;
	int YOfs;
	unsigned Flags;
	char *Id;
	CFont *Font;
	unsigned int LuaHandle;
	struct _menu_ *Menu;  /// backpointer for speedups
	union {
		MenuitemText Text;
		MenuitemButton Button;
		MenuitemPulldown Pulldown;
		MenuitemInput Input;
		MenuitemCheckbox Checkbox;
		// ... add here ...
	} D;
} Menuitem;

typedef void (*InitFuncType)(struct _menu_ *);
typedef void (*ExitFuncType)(struct _menu_ *);
typedef void (*NetActionType)(void);

/**
**  Menu definition.
*/
typedef struct _menu_ {
	/// @todo char *Name; /// menu name
	int       X;          /// menu area x pos
	int       Y;          /// menu area y pos
	int       Width;      /// menu area width
	int       Height;     /// menu area height
	char     *Panel;      /// optional background panel
	CGraphic *BackgroundG;/// optional background image behind the menu panel
	int       DefSel;     /// initial selected item number (or -1)
	int       NumItems;   /// number of items to follow
	Menuitem *Items;      /// buttons, etc
	InitFuncType InitFunc;  /// constructor
	ExitFuncType ExitFunc;  /// destructor
	NetActionType NetAction;/// network action callback
} Menu;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern char MenuMapFullPath[1024];   /// Full path to currently selected map

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Draw menu button
extern void DrawMenuButton(ButtonStyle *style, unsigned flags,
	int x, int y, const char *text);

	/// The scenario path received from server, Update the client menu
extern int NetClientSelectScenario(void);
	/// Notify menu display code to update info
extern void NetConnectForceDisplayUpdate(void);
	/// Compare Local State <-> Server's state, force Update when changes
extern void NetClientCheckLocalState(void);

	/// Edit resource properties
extern void EditorEditResource(void);
	/// Edit ai properties
extern void EditorEditAiProperties(void);

	/// Pre menu setup
extern void PreMenuSetup(void);

//@}

#endif // !__MENUS_H__
