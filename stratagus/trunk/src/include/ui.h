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

// FIXME: this only the start of the new user interface
// FIXME: all user interface variables should go here and be configurable

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "video.h"
#include "upgrade_structs.h"
#include "cursor.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--  Definitons
----------------------------------------------------------------------------*/

	/// buttons on screen themselves
typedef struct _button_ {
	int X;                          /// x coordinate on the screen
	int Y;                          /// y coordinate on the screen
	int Width;                      /// width of the button on the screen
	int Height;                     /// height of the button on the screen
} Button;

#define MAX_NUM_VIEWPORTS 8         /// Number of supported viewports

#if !defined(__STRUCT_VIEWPORT__)
#define __STRUCT_VIEWPORT__         /// protect duplicate viewport typedef
typedef struct _viewport_ Viewport; /// Viewport typedef
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
	int X;                      /// Screen pixel left corner x coordinate
	int Y;                      /// Screen pixel upper corner y coordinate
	int EndX;                   /// Screen pixel right x coordinate
	int EndY;                   /// Screen pixel bottom y coordinate

	int MapX;                   /// Map tile left corner x coordinate
	int MapY;                   /// Map tile upper corner y coordinate
	int OffsetX;                /// Map tile offset
	int OffsetY;                /// Map tile offset
	int MapWidth;               /// Width in map tiles
	int MapHeight;              /// Height in map tiles

    char MustRedrawTile[MAXMAP_W * MAXMAP_H];   /// Must redraw tile
    char MustRedrawRow[MAXMAP_W];               /// Must redraw row

	Unit* Unit;                 /// Bound to this unit
};

/**
**  Enumeration of the different predefined viewport configurations.
**
**  @todo this should be later user configurable
*/
typedef enum {
	VIEWPORT_SINGLE,                /// Old single viewport
	VIEWPORT_SPLIT_HORIZ,           /// Two viewports split horizontal
	VIEWPORT_SPLIT_HORIZ3,          /// Three viewports split horiontal
	VIEWPORT_SPLIT_VERT,            /// Two viewports split vertical
	VIEWPORT_QUAD,                  /// Four viewports split symmetric
	NUM_VIEWPORT_MODES,             /// Number of different viewports.
} ViewportMode;

#define ScPanel "sc-panel"          /// hack for transparency

/**
**  Menu panels
*/
typedef struct _menu_panel_ {
	char*                Ident;     /// Unique identifier
	GraphicConfig        Panel;     /// Panel
	struct _menu_panel_* Next;      /// Next pointer
} MenuPanel;

/**
**  Defines the user interface.
*/
typedef struct _ui_ {
	char* Name;                         /// interface name to select
	int Width;                          /// useable for this width
	int Height;                         /// useable for this height

	int MouseScroll;                    /// Enable mouse scrolling
	int KeyScroll;                      /// Enable keyboard scrolling
		/// Middle-Mouse Scroll Speed (screenpixels per mousepixel)
	int MouseScrollSpeedDefault;
		/// Middle-Mouse Scroll Speed with Control pressed
	int MouseScrollSpeedControl;

	int MouseWarpX;                     /// Cursor warp X position
	int MouseWarpY;                     /// Cursor warp Y position

	char* NormalFontColor;              /// Color for normal text displayed
	char* ReverseFontColor;             /// Color for reverse text displayed

	// Fillers
	GraphicConfig* Filler;              /// Filler graphics
	int* FillerX;                       /// Filler X positions
	int* FillerY;                       /// Filler Y positions
	int  NumFillers;                    /// Number of fillers

	// Resource line
	GraphicConfig Resource;             /// Resource background
	int ResourceX;                      /// Resource X position
	int ResourceY;                      /// Resource Y position

	struct {
		GraphicConfig Icon;             /// icon image
		int IconFrame;                  /// icon frame
		int IconX;                      /// icon X position
		int IconY;                      /// icon Y position
		int IconW;                      /// icon W position
		int IconH;                      /// icon H position
		int TextX;                      /// text X position
		int TextY;                      /// text Y position
	} Resources[MaxCosts + 2];          /// Icon+Text of all resources
                                        /// +2 for food and score

	// Info panel
	GraphicConfig InfoPanel;            /// Info panel background
	int InfoPanelX;                     /// Info panel screen X position
	int InfoPanelY;                     /// Info panel screen Y position
	int InfoPanelW;                     /// Info panel width
	int InfoPanelH;                     /// Info panel height

	Button* SingleSelectedButton;       /// FIXME: docu
	char*   SingleSelectedText;         /// FIXME: docu
	int     SingleSelectedFont;         /// FIXME: docu
	int     SingleSelectedTextX;        /// FIXME: docu
	int     SingleSelectedTextY;        /// FIXME: docu
	Button* SelectedButtons;            /// Selected buttons
	int     NumSelectedButtons;         /// Number of selected buttons
	char*   SelectedText;               /// FIXME: docu
	int     SelectedFont;               /// FIXME: docu
	int     SelectedTextX;              /// FIXME: docu
	int     SelectedTextY;              /// FIXME: docu
	int     MaxSelectedFont;            /// FIXME: docu
	int     MaxSelectedTextX;           /// position to place '+#' text
	int     MaxSelectedTextY;           /// if > maximum units selected

	Button* SingleTrainingButton;       /// FIXME: docu
	char*   SingleTrainingText;         /// FIXME: docu
	int     SingleTrainingFont;         /// FIXME: docu
	int     SingleTrainingTextX;        /// FIXME: docu
	int     SingleTrainingTextY;        /// FIXME: docu
	Button* TrainingButtons;            /// Training buttons
	int     NumTrainingButtons;         /// Number of training buttons
	char*   TrainingText;               /// FIXME: docu
	int     TrainingFont;               /// FIXME: docu
	int     TrainingTextX;              /// FIXME: docu
	int     TrainingTextY;              /// FIXME: docu

	Button* UpgradingButton;
	char*   UpgradingText;
	int     UpgradingFont;
	int     UpgradingTextX;
	int     UpgradingTextY;

	Button* ResearchingButton;
	char*   ResearchingText;
	int     ResearchingFont;
	int     ResearchingTextX;
	int     ResearchingTextY;

	Button* TransportingButtons;
	int     NumTransportingButtons;     /// Number of transporting buttons
	char*   TransportingText;
	int     TransportingFont;
	int     TransportingTextX;
	int     TransportingTextY;

	// Completed bar
	SDL_Color CompletedBarColorRGB;     /// color for completed bar
	Uint32    CompletedBarColor;        /// color for completed bar
	int       CompletedBarShadow;       /// should complete bar have shadow
	int       CompletedBarX;            /// completed bar X position
	int       CompletedBarY;            /// completed bar Y position
	int       CompletedBarW;            /// completed bar width
	int       CompletedBarH;            /// completed bar height
	char*     CompletedBarText;         /// completed bar text
	int       CompletedBarFont;         /// completed bar font
	int       CompletedBarTextX;        /// completed bar text X position
	int       CompletedBarTextY;        /// completed bar text Y position

	// Button panel
	GraphicConfig ButtonPanel;          /// Button panel background
	Button*       ButtonButtons;        /// Button panel buttons
	int           NumButtonButtons;     /// Number of button panel buttons
	int           ButtonPanelX;         /// Button panel screen X position
	int           ButtonPanelY;         /// Button panel screen Y position
	int           CommandKeyFont;       /// Command key font

	// Map area
	ViewportMode ViewportMode;          /// Current viewport mode
	Viewport*    MouseViewport;         /// Viewport containing mouse
	Viewport*    SelectedViewport;      /// Current selected active viewport
	int          NumViewports;          /// # Viewports currently used
	Viewport     Viewports[MAX_NUM_VIEWPORTS]; /// Parameters of all viewports
	// Map* attributes of Viewport are unused here:
	Viewport     MapArea;               /// geometry of the whole map area

	// The menu panel
	GraphicConfig MenuPanel;            /// menu panel background
	int           MenuPanelX;           /// menu panel screen X position
	int           MenuPanelY;           /// menu panel screen Y position

	/// Menu buttons
	struct {
		int   X;                        /// button screen X position
		int   Y;                        /// button screen Y position
		char* Text;                     /// button caption
		int   Font;                     /// button caption font
		int   Width;                    /// button width
		int   Height;                   /// button height
		int   Button;                   /// button style
	} MenuButton,
	  NetworkMenuButton,
	  NetworkDiplomacyButton;

	// The minimap
	GraphicConfig MinimapPanel;         /// minimap panel background
	int           MinimapPanelX;        /// minimap panel screen X position
	int           MinimapPanelY;        /// minimap panel screen Y position
	int           MinimapW;             /// minimap screen Width
	int           MinimapH;             /// minimap screen Height
	int           MinimapPosX;          /// minimap screen X position
	int           MinimapPosY;          /// minimap screen Y position
	int           MinimapTransparent;   /// unexplored areas are transparent
	Uint32        ViewportCursorColor;  /// minimap cursor color

	// The status line
	GraphicConfig StatusLine;           /// Status line background
	int           StatusLineX;          /// status line screen X position
	int           StatusLineY;          /// status line screen Y position
	int           StatusLineTextX;      /// status line screen text X position
	int           StatusLineTextY;      /// status line screen text Y position
	int           StatusLineFont;       /// Status line font

	// Offsets for 640x480 center used by menus
	int Offset640X;                     /// Offset for 640x480 X position
	int Offset480Y;                     /// Offset for 640x480 Y position

	//
	//  Cursors used.
	//
	CursorConfig Point;                 /// General pointing cursor
	CursorConfig Glass;                 /// HourGlass, system is waiting
	CursorConfig Cross;                 /// Multi-select cursor.
	CursorConfig YellowHair;            /// Yellow action,attack cursor.
	CursorConfig GreenHair;             /// Green action,attack cursor.
	CursorConfig RedHair;               /// Red action,attack cursor.
	CursorConfig Scroll;                /// Cursor for scrolling map around.

	CursorConfig ArrowE;                /// Cursor pointing east
	CursorConfig ArrowNE;               /// Cursor pointing north east
	CursorConfig ArrowN;                /// Cursor pointing north
	CursorConfig ArrowNW;               /// Cursor pointing north west
	CursorConfig ArrowW;                /// Cursor pointing west
	CursorConfig ArrowSW;               /// Cursor pointing south west
	CursorConfig ArrowS;                /// Cursor pointing south
	CursorConfig ArrowSE;               /// Cursor pointing south east

// FIXME: could use different sounds/speech for the errors
// Is in gamesounds?
//	SoundConfig PlacementError;         /// played on placements errors
//	SoundConfig PlacementSuccess;       /// played on placements success
//	SoundConfig Click;                  /// click noice used often

	MenuPanel* MenuPanels;              /// Menu panels

	GraphicConfig VictoryBackground;    /// FIXME: docu
	GraphicConfig DefeatBackground;     /// FIXME: docu
} UI;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern UI TheUI;                        /// The user interface
extern UI** UI_Table;                   /// All available user interfaces

extern char RightButtonAttacks;         /// right button 0 move, 1 attack.
extern ButtonAction* CurrentButtons;    /// Current Selected Buttons
extern char FancyBuildings;             /// Mirror buildings 1 yes, 0 now.

extern int SpeedKeyScroll;              /// Keyboard Scrolling Speed, in Frames
extern int SpeedMouseScroll;            /// Mouse Scrolling Speed, in Frames

extern char* UiGroupKeys;               /// up to 11 keys used for group selection

// only exported to save them

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Initialize the ui
extern void InitUserInterface(const char* race_name);
	/// Load ui graphics
extern void LoadUserInterface(void);
	/// Save the ui state
extern void SaveUserInterface(CLFile* file);
	/// Clean up a ui
extern void CleanUI(UI* ui);
	/// Clean up the ui module
extern void CleanUserInterface(void);
	/// Register ccl features
extern void UserInterfaceCclRegister(void);

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

#endif		// !__UI_H__
