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
/**@name ui.h - The user interface header file. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer and Jimmy Salmon
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

#ifndef __UI_H__
#define __UI_H__

//@{

/// @todo this only the start of the new user interface
/// @todo all user interface variables should go here and be configurable

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "util.h"
#include "video.h"
#include "upgrade_structs.h"
#include "cursor.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct _unit_;
struct _button_action_;
struct _CL_File_;

/*----------------------------------------------------------------------------
--  Definitions
----------------------------------------------------------------------------*/

typedef enum _text_alignment_ {
	TextAlignUndefined,
	TextAlignCenter,
	TextAlignLeft,
	TextAlignRight,
} TextAlignment;

typedef struct _button_style_properties_ {
	Graphic* Sprite;
	int Frame;
	SDL_Color BorderColorRGB;
	Uint32 BorderColor;
	int BorderSize;
	TextAlignment TextAlign;        ///< Text alignment
	int TextX;                      ///< Text X location
	int TextY;                      ///< Text Y location
	char* TextNormalColor;          ///< Normal text color
	char* TextReverseColor;         ///< Reverse text color
} ButtonStyleProperties;

typedef struct _button_style_ {
	int Width;                      ///< Button width
	int Height;                     ///< Button height
	int Font;                       ///< Font
	char* TextNormalColor;          ///< Normal text color
	char* TextReverseColor;         ///< Reverse text color
	TextAlignment TextAlign;        ///< Text alignment
	int TextX;                      ///< Text X location
	int TextY;                      ///< Text Y location
	ButtonStyleProperties Default;  ///< Default button properties
	ButtonStyleProperties Hover;    ///< Hover button properties
	ButtonStyleProperties Selected; ///< Selected button properties
	ButtonStyleProperties Clicked;  ///< Clicked button properties
	ButtonStyleProperties Disabled; ///< Disabled button properties
} ButtonStyle;

typedef struct _checkbox_style_ {
	int Width;                      ///< Checkbox width
	int Height;                     ///< Checkbox height
	int Font;                       ///< Font
	char* TextNormalColor;          ///< Normal text color
	char* TextReverseColor;         ///< Reverse text color
	TextAlignment TextAlign;        ///< Text alignment
	int TextX;                      ///< Text X location
	int TextY;                      ///< Text Y location
	ButtonStyleProperties Default;  ///< Default checkbox properties
	ButtonStyleProperties Hover;    ///< Hover checkbox properties
	ButtonStyleProperties Selected; ///< Selected checkbox properties
	ButtonStyleProperties Clicked;  ///< Clicked checkbox properties
	ButtonStyleProperties Disabled; ///< Disabled checkbox properties
	ButtonStyleProperties Checked;  ///< Default checkbox properties
	ButtonStyleProperties CheckedHover;    ///< Checked hover checkbox properties
	ButtonStyleProperties CheckedSelected; ///< Checked selected checkbox properties
	ButtonStyleProperties CheckedClicked;  ///< Checked clicked checkbox properties
	ButtonStyleProperties CheckedDisabled; ///< Checked disabled checkbox properties
} CheckboxStyle;

	/// buttons on screen themselves
typedef struct _button_ {
	int X;                          ///< x coordinate on the screen
	int Y;                          ///< y coordinate on the screen
	char* Text;                     ///< button text
	ButtonStyle* Style;             ///< button style
} Button;

#define MAX_NUM_VIEWPORTS 8         ///< Number of supported viewports

#if !defined(__STRUCT_VIEWPORT__)
#define __STRUCT_VIEWPORT__         ///< protect duplicate viewport typedef
typedef struct _viewport_ Viewport; ///< Viewport typedef
#endif

/**
**  A map viewport.
**
**  A part of the map displayed on sceen.
**
**  Viewport::X Viewport::Y
**  Viewport::EndX Viewport::EndY
**
**    upper left corner of this viewport is located at pixel
**    coordinates (X, Y) with respect to upper left corner of
**    stratagus's window, similarly lower right corner of this
**    viewport is (EndX, EndY) pixels away from the UL corner of
**    stratagus's window.
**
**  Viewport::MapX Viewport::MapY
**  Viewport::MapWidth Viewport::MapHeight
**
**    Tile coordinates of UL corner of this viewport with respect to
**    UL corner of the whole map.
**
**  Viewport::Unit
**
**    Viewport is bound to an unit. If the unit moves the viewport
**    changes the position together with the unit.
**    @todo binding to a group.
*/
struct _viewport_ {
	int X;                      ///< Screen pixel left corner x coordinate
	int Y;                      ///< Screen pixel upper corner y coordinate
	int EndX;                   ///< Screen pixel right x coordinate
	int EndY;                   ///< Screen pixel bottom y coordinate

	int MapX;                   ///< Map tile left corner x coordinate
	int MapY;                   ///< Map tile upper corner y coordinate
	int OffsetX;                ///< Map tile offset
	int OffsetY;                ///< Map tile offset
	int MapWidth;               ///< Width in map tiles
	int MapHeight;              ///< Height in map tiles

	struct _unit_* Unit;        ///< Bound to this unit
};

/**
**  Enumeration of the different predefined viewport configurations.
**
**  @todo this should be later user configurable
*/
typedef enum {
	VIEWPORT_SINGLE,                ///< Old single viewport
	VIEWPORT_SPLIT_HORIZ,           ///< Two viewports split horizontal
	VIEWPORT_SPLIT_HORIZ3,          ///< Three viewports split horiontal
	VIEWPORT_SPLIT_VERT,            ///< Two viewports split vertical
	VIEWPORT_QUAD,                  ///< Four viewports split symmetric
	NUM_VIEWPORT_MODES,             ///< Number of different viewports.
} ViewportMode;

#define ScPanel "sc-panel"          ///< hack for transparency

/**
**  Menu panels
*/
typedef struct _menu_panel_ {
	char*                Ident;     ///< Unique identifier
	Graphic*             G;         ///< Graphic
	struct _menu_panel_* Next;      ///< Next pointer
} MenuPanel;

/**
**  Defines the user interface.
*/
typedef struct _ui_ {
	char* Name;                         ///< interface name to select
	int Width;                          ///< useable for this width
	int Height;                         ///< useable for this height

	int MouseScroll;                    ///< Enable mouse scrolling
	int KeyScroll;                      ///< Enable keyboard scrolling
		/// Middle-Mouse Scroll Speed (screenpixels per mousepixel)
	int MouseScrollSpeedDefault;
		/// Middle-Mouse Scroll Speed with Control pressed
	int MouseScrollSpeedControl;

	int MouseWarpX;                     ///< Cursor warp X position
	int MouseWarpY;                     ///< Cursor warp Y position

	char* NormalFontColor;              ///< Color for normal text displayed
	char* ReverseFontColor;             ///< Color for reverse text displayed

	// Fillers
	Graphic** Filler;                   ///< Filler graphics
	int* FillerX;                       ///< Filler X positions
	int* FillerY;                       ///< Filler Y positions
	int  NumFillers;                    ///< Number of fillers

	struct {
		GraphicConfig Icon;             ///< icon image
		int IconFrame;                  ///< icon frame
		int IconX;                      ///< icon X position
		int IconY;                      ///< icon Y position
		int IconW;                      ///< icon W position
		int IconH;                      ///< icon H position
		int TextX;                      ///< text X position
		int TextY;                      ///< text Y position
	} Resources[MaxCosts + 2];          ///< Icon+Text of all resources
                                        ///< +2 for food and score

	// Info panel
	GraphicConfig InfoPanel;            ///< Info panel background
	int InfoPanelX;                     ///< Info panel screen X position
	int InfoPanelY;                     ///< Info panel screen Y position
	int InfoPanelW;                     ///< Info panel width
	int InfoPanelH;                     ///< Info panel height

	Button* SingleSelectedButton;       ///< FIXME: docu
	char*   SingleSelectedText;         ///< FIXME: docu
	int     SingleSelectedFont;         ///< FIXME: docu
	int     SingleSelectedTextX;        ///< FIXME: docu
	int     SingleSelectedTextY;        ///< FIXME: docu
	Button* SelectedButtons;            ///< Selected buttons
	int     NumSelectedButtons;         ///< Number of selected buttons
	char*   SelectedText;               ///< FIXME: docu
	int     SelectedFont;               ///< FIXME: docu
	int     SelectedTextX;              ///< FIXME: docu
	int     SelectedTextY;              ///< FIXME: docu
	int     MaxSelectedFont;            ///< FIXME: docu
	int     MaxSelectedTextX;           ///< position to place '+#' text
	int     MaxSelectedTextY;           ///< if > maximum units selected

	Button* SingleTrainingButton;       ///< FIXME: docu
	char*   SingleTrainingText;         ///< FIXME: docu
	int     SingleTrainingFont;         ///< FIXME: docu
	int     SingleTrainingTextX;        ///< FIXME: docu
	int     SingleTrainingTextY;        ///< FIXME: docu
	Button* TrainingButtons;            ///< Training buttons
	int     NumTrainingButtons;         ///< Number of training buttons
	char*   TrainingText;               ///< FIXME: docu
	int     TrainingFont;               ///< FIXME: docu
	int     TrainingTextX;              ///< FIXME: docu
	int     TrainingTextY;              ///< FIXME: docu

	Button* UpgradingButton; ///< FIXME: docu
	char*   UpgradingText; ///< FIXME: docu
	int     UpgradingFont; ///< FIXME: docu
	int     UpgradingTextX; ///< FIXME: docu
	int     UpgradingTextY; ///< FIXME: docu

	Button* ResearchingButton; ///< FIXME: docu
	char*   ResearchingText; ///< FIXME: docu
	int     ResearchingFont; ///< FIXME: docu
	int     ResearchingTextX; ///< FIXME: docu
	int     ResearchingTextY; ///< FIXME: docu

	Button* TransportingButtons; ///< FIXME: docu
	int     NumTransportingButtons;     ///< Number of transporting buttons
	char*   TransportingText; ///< FIXME: docu
	int     TransportingFont; ///< FIXME: docu
	int     TransportingTextX; ///< FIXME: docu
	int     TransportingTextY; ///< FIXME: docu

	// Completed bar
	SDL_Color CompletedBarColorRGB;     ///< color for completed bar
	Uint32    CompletedBarColor;        ///< color for completed bar
	int       CompletedBarShadow;       ///< should complete bar have shadow
	int       CompletedBarX;            ///< completed bar X position
	int       CompletedBarY;            ///< completed bar Y position
	int       CompletedBarW;            ///< completed bar width
	int       CompletedBarH;            ///< completed bar height
	char*     CompletedBarText;         ///< completed bar text
	int       CompletedBarFont;         ///< completed bar font
	int       CompletedBarTextX;        ///< completed bar text X position
	int       CompletedBarTextY;        ///< completed bar text Y position

	// Button panel
	GraphicConfig ButtonPanel;          ///< Button panel background
	int           ButtonPanelX;         ///< Button panel screen X position
	int           ButtonPanelY;         ///< Button panel screen Y position
	Button*       ButtonButtons;        ///< Button panel buttons
	int           NumButtonButtons;     ///< Number of button panel buttons
	SDL_Color     ButtonAutoCastBorderColorRGB; ///< Auto cast border color

	// Pie Menu
	GraphicConfig PieMenuBackground;      ///< Optional background image for the piemenu
	enum _mouse_buttons_ PieMouseButton; ///< Which mouse button pops up the piemenu. Deactivate with the NoButton value.
	int PieX[8];                         ///< X position of the pies
	int PieY[8];                         ///< Y position of the pies

	// Map area
	ViewportMode ViewportMode;          ///< Current viewport mode
	Viewport*    MouseViewport;         ///< Viewport containing mouse
	Viewport*    SelectedViewport;      ///< Current selected active viewport
	int          NumViewports;          ///< # Viewports currently used
	Viewport     Viewports[MAX_NUM_VIEWPORTS]; ///< Parameters of all viewports
	// Map* attributes of Viewport are unused here:
	Viewport     MapArea;               ///< geometry of the whole map area

	/// Menu buttons
	Button MenuButton;                  ///< menu button
	Button NetworkMenuButton;           ///< network menu button
	Button NetworkDiplomacyButton;      ///< network diplomacy button

	// The minimap
	int           MinimapW;             ///< minimap screen Width
	int           MinimapH;             ///< minimap screen Height
	int           MinimapPosX;          ///< minimap screen X position
	int           MinimapPosY;          ///< minimap screen Y position
	int           MinimapTransparent;   ///< unexplored areas are transparent
	Uint32        ViewportCursorColor;  ///< minimap cursor color

	// The status line
	int           StatusLineW;          ///< status line screen width
	int           StatusLineTextX;      ///< status line screen text X position
	int           StatusLineTextY;      ///< status line screen text Y position
	int           StatusLineFont;       ///< Status line font

	// Offsets for 640x480 center used by menus
	int Offset640X;                     ///< Offset for 640x480 X position
	int Offset480Y;                     ///< Offset for 640x480 Y position

	//
	//  Cursors used.
	//
	CursorConfig Point;                 ///< General pointing cursor
	CursorConfig Glass;                 ///< HourGlass, system is waiting
	CursorConfig Cross;                 ///< Multi-select cursor.
	CursorConfig YellowHair;            ///< Yellow action,attack cursor.
	CursorConfig GreenHair;             ///< Green action,attack cursor.
	CursorConfig RedHair;               ///< Red action,attack cursor.
	CursorConfig Scroll;                ///< Cursor for scrolling map around.

	CursorConfig ArrowE;                ///< Cursor pointing east
	CursorConfig ArrowNE;               ///< Cursor pointing north east
	CursorConfig ArrowN;                ///< Cursor pointing north
	CursorConfig ArrowNW;               ///< Cursor pointing north west
	CursorConfig ArrowW;                ///< Cursor pointing west
	CursorConfig ArrowSW;               ///< Cursor pointing south west
	CursorConfig ArrowS;                ///< Cursor pointing south
	CursorConfig ArrowSE;               ///< Cursor pointing south east

/// @todo could use different sounds/speech for the errors
/// Is in gamesounds?
/// SoundConfig PlacementError;         ///< played on placements errors
/// SoundConfig PlacementSuccess;       ///< played on placements success
/// SoundConfig Click;                  ///< click noice used often

	MenuPanel* MenuPanels;              ///< Menu panels

	GraphicConfig VictoryBackground;    ///< FIXME: docu
	GraphicConfig DefeatBackground;     ///< FIXME: docu
} UI;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern UI TheUI;                        ///< The user interface
extern UI** UI_Table;                   ///< All available user interfaces

	/// Hash table of all the button styles
typedef hashtable(ButtonStyle*, 128) _ButtonStyleHash;
extern _ButtonStyleHash ButtonStyleHash;

	/// Hash table of all the checkbox styles
typedef hashtable(CheckboxStyle*,128) _CheckboxStyleHash;
extern _CheckboxStyleHash CheckboxStyleHash;

extern char RightButtonAttacks;         ///< right button 0 move, 1 attack.
extern struct _button_action_* CurrentButtons;    ///< Current Selected Buttons
extern char FancyBuildings;             ///< Mirror buildings 1 yes, 0 now.

extern int SpeedKeyScroll;              ///< Keyboard Scrolling Speed, in Frames
extern int SpeedMouseScroll;            ///< Mouse Scrolling Speed, in Frames

extern char DefaultGroupKeys[];    ///< Default group keys
extern char* UiGroupKeys;               ///< Up to 11 keys used for group selection

// only exported to save them

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Initialize the ui
extern void InitUserInterface(const char* race_name);
	/// Load ui graphics
extern void LoadUserInterface(void);
	/// Save the ui state
extern void SaveUserInterface(struct _CL_File_* file);
	/// Clean up a ui
extern void CleanUI(UI* ui);
	/// Clean up the ui module
extern void CleanUserInterface(void);
	/// Register ccl features
extern void UserInterfaceCclRegister(void);

	/// Find a button style
extern ButtonStyle* FindButtonStyle(const char* style);
	/// Find a checkbox style
extern CheckboxStyle* FindCheckboxStyle(const char* style);

	/// Called if the mouse is moved in Normal interface state
extern void UIHandleMouseMove(int x, int y);
	/// Called if any mouse button is pressed down
extern void UIHandleButtonDown(unsigned button);
	/// Called if any mouse button is released up
extern void UIHandleButtonUp(unsigned button);

	/// Restrict mouse cursor to viewport
extern void RestrictCursorToViewport(void);
	/// Restrict mouse cursor to minimap
extern void RestrictCursorToMinimap(void);

	/// Get viewport for screen pixel position
extern Viewport* GetViewport(int x, int y);
	/// Get viewport for tile map position
extern Viewport* MapTileGetViewport(int, int);
	/// Cycle through all available viewport modes
extern void CycleViewportMode(int);
	/// Select viewport mode
extern void SetViewportMode(ViewportMode mode);

	/// Convert screen X pixel to map tile
extern int Viewport2MapX(const Viewport* vp, int x);
	/// Convert screen Y pixel to map tile
extern int Viewport2MapY(const Viewport* vp, int y);
	/// Convert map tile to screen X pixel
extern int Map2ViewportX(const Viewport* vp, int x);
	/// Convert map tile to screen Y pixel
extern int Map2ViewportY(const Viewport* vp, int y);

	/// Set the current map view to x,y(upper,left corner)
extern void ViewportSetViewpoint(Viewport* vp, int x, int y, int offsetx, int offsety);
	/// Center map on point in viewport
extern void ViewportCenterViewpoint(Viewport* vp, int x, int y, int offsetx, int offsety);

//@}

#endif // !__UI_H__
