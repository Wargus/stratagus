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
/**@name ui.h		-	The user interface header file. */
//
//	(c) Copyright 1999-2002 by Lutz Sammer
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

#ifndef __UI_H__
#define __UI_H__

//@{

// FIXME: this only the start of the new user interface
// FIXME: all user interface variables should go here and be configurable

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "video.h"
#include "upgrade_structs.h"
#include "cursor.h"

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

/**
**	Defines the default SVGALIB mouse speed adjust (must be > 0)
*/
#define MOUSEADJUST	15

/**
**	Defines the default SVGALIB mouse speed scale
*/
#define MOUSESCALE	1

    /// MACRO - HARDCODED NUMBER OF BUTTONS on screen
#define MaxButtons	19

    /// typedef for buttons on screen themselves
typedef struct _button_ Button;

    /// buttons on screen themselves
struct _button_ {
    int		X;			/// x coordinate on the screen
    int		Y;			/// y coordinate on the screen
    int	    Width;			/// width of the button on the screen
    int	    Height;			/// height of the button on the screen
};

#define MAX_NUM_VIEWPORTS 8		/// Number of supported viewports

#if !defined(__STRUCT_VIEWPORT__)
#define __STRUCT_VIEWPORT__		/// protect duplicate viewport typedef
typedef struct _viewport_ Viewport;	/// Viewport typedef
#endif

/**
**	A map viewport.
**
**	A part of the map displayed on sceen.
**
**	Viewport::X Viewport::Y
**	Viewport::EndX Viewport::EndY
**
**		upper left corner of this viewport is located at pixel
**		coordinates (X, Y) with respect to upper left corner of
**		freecraft's window, similarly lower right corner of this
**		viewport is (EndX, EndY) pixels away from the UL corner of
**		freecraft's window.
**
**	Viewport::MapX Viewport::MapY
**	Viewport::MapWidth Viewport::MapHeight
**
**		Tile coordinates of UL corner of this viewport with respect to
**		UL corner of the whole map.
**
**	Viewport::Unit
**
**		Viewport is bound to an unit. If the unit moves the viewport
**		changes the position together with the unit.
**		@todo binding to a group.
*/
struct _viewport_ {
    int X;			/// Screen pixel left corner x coordinate
    int Y;			/// Screen pixel upper corner y coordinate
    int EndX;			/// Screen pixel right x coordinate
    int EndY;			/// Screen pixel bottom y coordinate

    int MapX;			/// Map tile left corner x coordinate
    int MapY;			/// Map tile upper corner y coordinate
    int MapWidth;		/// Width in map tiles
    int MapHeight;		/// Height in map tiles

    Unit* Unit;			/// Bound to this unit
};

/**
**	Enumeration of the different predefined viewport configurations.
**
**	@todo this should be later user configurable
*/
typedef enum {
    VIEWPORT_SINGLE,		/// Old single viewport
    VIEWPORT_SPLIT_HORIZ,	/// Two viewports split horizontal
    VIEWPORT_SPLIT_HORIZ3,	/// Three viewports split horiontal
    VIEWPORT_SPLIT_VERT,	/// Two viewports split vertical
    VIEWPORT_QUAD,		/// Four viewports split symmetric
    NUM_VIEWPORT_MODES		/// Number of different viewports.
} ViewportMode;

/**
**	Panel types used in menus (and stored in ui global below)
*/
enum {
    ImageNone,
    ImagePanel1,	// 256 x 288
    ImagePanel2,	// 288 x 256
    ImagePanel3,	// 384 x 256
    ImagePanel4,	// 288 x 128
    ImagePanel5,	// 352 x 352
};

/**
**	Defines the user interface.
*/
typedef struct _ui_ {
    // to select the correct user interface.
    char*	Name;			/// interface name to select
    int		Width;			/// useable for this width
    int		Height;			/// useable for this height

    int		Contrast;		/// General Contrast
    int		Brightness;		/// General Brightness
    int		Saturation;		/// General Saturation

    int		MouseScroll;		/// Enable mouse scrolling
    int		KeyScroll;		/// Enable keyboard scrolling
	/// Middle mouse button map move with reversed directions
    char	ReverseMouseMove;

    int		WarpX;			/// Cursor warp X position
    int		WarpY;			/// Cursor warp Y position

    int		MouseAdjust;		/// Mouse speed adjust
    int		MouseScale;		/// Mouse speed scale

    char	NormalFontColor;	/// Color for normal text displayed
    char	ReverseFontColor;	/// Color for reverse text displayed

    //	Fillers
    GraphicConfig Filler1;		/// filler 1 graphic
    int		Filler1X;		/// filler 1 X position
    int		Filler1Y;		/// filler 1 Y position

    //	Resource line
    GraphicConfig Resource;		/// Resource background
    int		ResourceX;		/// Resource X position
    int		ResourceY;		/// Resource Y position

    int		OriginalResources;	/// original resource mode

    struct {
#if 0
	int	IconX;			/// Icon X position
	int	IconY;			/// Icon Y position
	IconConfig	Icon;		/// Icon for resource
	int	TextX;			/// Text X position
	int	TextX;			/// Text Y position
	char*	Text;			/// Text for resource
	int	AmountX;		/// Amount X position
	int	AmountY;		/// Amount Y position
#endif
	GraphicConfig Icon;		/// icon image
	int	IconRow;		/// icon image row (frame)
	int	IconX;			/// icon X position
	int	IconY;			/// icon Y position
	int	IconW;			/// icon W position
	int	IconH;			/// icon H position
	int	TextX;			/// text X position
	int	TextY;			/// text Y position
    }		Resources[MaxCosts];	/// Icon+Text of all resources

    GraphicConfig FoodIcon;		/// units icon image
    int		FoodIconRow;		/// units icon image row (frame)
    int		FoodIconX;		/// units icon X position
    int		FoodIconY;		/// units icon Y position
    int		FoodIconW;		/// units icon W position
    int		FoodIconH;		/// units icon H position
    int		FoodTextX;		/// units text X position
    int		FoodTextY;		/// units text Y position

    GraphicConfig ScoreIcon;		/// score icon image
    int		ScoreIconRow;		/// score icon image row (frame)
    int		ScoreIconX;		/// score icon X position
    int		ScoreIconY;		/// score icon Y position
    int		ScoreIconW;		/// score icon W position
    int		ScoreIconH;		/// score icon H position
    int		ScoreTextX;		/// score text X position
    int		ScoreTextY;		/// score text Y position

    // Info panel
    GraphicConfig InfoPanel;		/// Info panel background
    int		InfoPanelX;		/// Info panel screen X position
    int		InfoPanelY;		/// Info panel screen Y position
    int		InfoPanelW;		/// Info panel width
    int		InfoPanelH;		/// Info panel height

    // Complete bar
    int		CompleteBarColor;	/// color for complete bar
    int		CompleteBarX;		/// complete bar X position
    int		CompleteBarY;		/// complete bar Y position
    int		CompleteTextX;		/// complete text X position
    int		CompleteTextY;		/// complete text Y position

    // Button panel
    GraphicConfig ButtonPanel;		/// Button panel background
    int		ButtonPanelX;		/// Button panel screen X position
    int		ButtonPanelY;		/// Button panel screen Y position

    // Map area
    ViewportMode ViewportMode;		/// Current viewport mode
    Viewport*	MouseViewport;		/// Viewport containing mouse
    Viewport*	SelectedViewport;	/// Current selected active viewport
    int		NumViewports;		/// # Viewports currently used
    Viewport	Viewports[MAX_NUM_VIEWPORTS];	/// Parameters of all viewports
    // Map* attributes of Viewport are unused here:
    Viewport	MapArea;		/// geometry of the whole map area

    // The menu button
    GraphicConfig MenuButton;		/// menu button background
    int		MenuButtonX;		/// menu button screen X position
    int		MenuButtonY;		/// menu button screen Y position

    // The minimap
    GraphicConfig Minimap;		/// minimap panel background
    int		MinimapX;		/// minimap screen X position
    int		MinimapY;		/// minimap screen Y position
    int		ViewportCursorColor;	/// minimap cursor color

    // The status line
    GraphicConfig StatusLine;		/// Status line background
    int		StatusLineX;		/// status line screeen X position
    int		StatusLineY;		/// status line screeen Y position

	/// all buttons (1 Menu, 9 Group, 9 Command)
    Button	Buttons[MaxButtons];
	/// used for displaying unit training queues
    Button	Buttons2[6];

    // Offsets for 640x480 center used by menus
    int		Offset640X;		/// Offset for 640x480 X position
    int		Offset480Y;		/// Offset for 640x480 Y position

    //
    //	Cursors used.
    //
    CursorConfig	Point;		/// General pointing cursor
    CursorConfig	Glass;		/// HourGlass, system is waiting
    CursorConfig	Cross;		/// Multi-select cursor.
    CursorConfig	YellowHair;	/// Yellow action,attack cursor.
    CursorConfig	GreenHair;	/// Green action,attack cursor.
    CursorConfig	RedHair;	/// Red action,attack cursor.
    CursorConfig	Scroll;		/// Cursor for scrolling map around.

    CursorConfig	ArrowE;		/// Cursor pointing east
    CursorConfig	ArrowNE;	/// Cursor pointing north east
    CursorConfig	ArrowN;		/// Cursor pointing north
    CursorConfig	ArrowNW;	/// Cursor pointing north west
    CursorConfig	ArrowW;		/// Cursor pointing west
    CursorConfig	ArrowSW;	/// Cursor pointing south west
    CursorConfig	ArrowS;		/// Cursor pointing south
    CursorConfig	ArrowSE;	/// Cursor pointing south east

// FIXME: could use different sounds/speach for the errors
// Is in gamesounds?
//    SoundConfig	PlacementError;		/// played on placements errors
//    SoundConfig	PlacementSuccess;	/// played on placements success
//    SoundConfig	Click;			/// click noice used often

    GraphicConfig	GameMenuePanel;	/// Panel 256 x 288
    GraphicConfig	Menue1Panel;	/// Panel 288 x 256
    GraphicConfig	Menue2Panel;	/// Panel 384 x 256
    GraphicConfig	VictoryPanel;	/// Panel 288 x 128
    GraphicConfig	ScenarioPanel;	/// Panel 352 x 352
} UI;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern UI TheUI;			/// The user interface
extern UI** UI_Table;			/// All available user interfaces

extern char RightButtonAttacks;		/// right button 0 move, 1 attack.
extern char FancyBuildings;		/// Mirror buildings 1 yes, 0 now.

extern int SpeedKeyScroll;		/// Keyboard Scrolling Speed, in Frames
extern int SpeedMouseScroll;		/// Mouse Scrolling Speed, in Frames

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void InitUserInterface(const char*);	/// initialize the ui
extern void LoadUserInterface(void);		/// load ui graphics
extern void SaveUserInterface(FILE*);		/// save the ui state
extern void CleanUserInterface(void);		/// clean up the ui
extern void UserInterfaceCclRegister(void);	/// register ccl features

    /// Called if the mouse is moved in Normal interface state
extern void UIHandleMouseMove(int x,int y);
    /// Called if any mouse button is pressed down
extern void UIHandleButtonDown(unsigned button);
    /// Called if any mouse button is released up
extern void UIHandleButtonUp(unsigned button);

    /// Restrict mouse cursor to viewport
extern void RestrictCursorToViewport(void);
    /// Restrict mouse cursor to minimap
extern void RestrictCursorToMinimap(void);

    /// Get viewport for screen pixel position
extern Viewport* GetViewport(int, int);
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
extern void ViewportSetViewpoint(Viewport* vp, int x, int y);
    /// Center map on point in viewport
extern void ViewportCenterViewpoint(Viewport* vp, int x, int y);

    /// Returns true, if unit is visible on current map view
extern int UnitVisibleInViewport(const Viewport* vp, const Unit* unit);

//@}

#endif	// !__UI_H__
