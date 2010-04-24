//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name ui.h - The user interface header file. */
//
//      (c) Copyright 1999-2007 by Lutz Sammer and Jimmy Salmon
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

#ifndef __UI_H__
#define __UI_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <vector>
#include <string>
#include <map>

#include "SDL.h"
#include "upgrade_structs.h"
#include "cursor.h"
#include "interface.h"
#include "minimap.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CFile;
class CFont;
class LuaActionListener;
struct lua_State;

/*----------------------------------------------------------------------------
--  Definitions
----------------------------------------------------------------------------*/

enum TextAlignment {
	TextAlignUndefined,
	TextAlignCenter,
	TextAlignLeft,
	TextAlignRight,
};

class ButtonStyleProperties {
public:
	ButtonStyleProperties() : Sprite(NULL), Frame(0), BorderColor(0),
		BorderSize(0), TextAlign(TextAlignUndefined),
		TextX(0), TextY(0)
	{
		BorderColorRGB.r = BorderColorRGB.g = BorderColorRGB.b = 0;
	}

	CGraphic *Sprite;
	int Frame;
	SDL_Color BorderColorRGB;
	Uint32 BorderColor;
	int BorderSize;
	TextAlignment TextAlign;        /// Text alignment
	int TextX;                      /// Text X location
	int TextY;                      /// Text Y location
	std::string TextNormalColor;    /// Normal text color
	std::string TextReverseColor;   /// Reverse text color
} ;

class ButtonStyle {
public:
	ButtonStyle() : Width(0), Height(0), Font(0),
		TextAlign(TextAlignUndefined), TextX(0), TextY(0) {}

	int Width;                      /// Button width
	int Height;                     /// Button height
	CFont *Font;                    /// Font
	std::string TextNormalColor;    /// Normal text color
	std::string TextReverseColor;   /// Reverse text color
	TextAlignment TextAlign;        /// Text alignment
	int TextX;                      /// Text X location
	int TextY;                      /// Text Y location
	ButtonStyleProperties Default;  /// Default button properties
	ButtonStyleProperties Hover;    /// Hover button properties
	ButtonStyleProperties Clicked;  /// Clicked button properties
};

	/// buttons on screen themselves
class CUIButton {
public:
	CUIButton() : X(0), Y(0), Style(NULL), Callback(NULL) {}
	~CUIButton() {}

	int X;                          /// x coordinate on the screen
	int Y;                          /// y coordinate on the screen
	std::string Text;               /// button text
	ButtonStyle *Style;             /// button style
	LuaActionListener *Callback;    /// callback function
};

#define MAX_NUM_VIEWPORTS 8         /// Number of supported viewports
#define MAX_SAVED_MAP_POSITIONS 3   /// Number of supported saved map positions

/**
**  A map viewport.
**
**  A part of the map displayed on sceen.
**
**  CViewport::X CViewport::Y
**  CViewport::EndX CViewport::EndY
**
**    upper left corner of this viewport is located at pixel
**    coordinates (X, Y) with respect to upper left corner of
**    stratagus's window, similarly lower right corner of this
**    viewport is (EndX, EndY) pixels away from the UL corner of
**    stratagus's window.
**
**  CViewport::MapX CViewport::MapY
**  CViewport::MapWidth CViewport::MapHeight
**
**    Tile coordinates of UL corner of this viewport with respect to
**    UL corner of the whole map.
**
**  CViewport::Unit
**
**    Viewport is bound to a unit. If the unit moves the viewport
**    changes the position together with the unit.
**    @todo binding to a group.
*/
class CViewport {
public:
	CViewport() : X(0), Y(0), EndX(0), EndY(0), MapX(0), MapY(0),
		OffsetX(0), OffsetY(0), MapWidth(0), MapHeight(0), Unit(NULL) {};

	/// Check if X and Y pixels are within map area
	bool IsInsideMapArea(int x, int y) const;
	/// Convert screen X pixel to map tile
	int Viewport2MapX(int x) const;
	/// Convert screen Y pixel to map tile
	int Viewport2MapY(int y) const;
	/// Convert map tile to screen X pixel
	int Map2ViewportX(int x) const;
	/// Convert map tile to screen Y pixel
	int Map2ViewportY(int y) const;
	/// Convert map pixel coordinates into viewport coordinates
	void MapPixel2Viewport(int &x, int &y) const;

	/// Set the current map view to x,y(upper,left corner)
	void Set(int x, int y, int offsetx, int offsety);
	/// Center map on point in viewport
	void Center(int x, int y, int offsetx, int offsety);
protected:
	/// Draw the map background
	void DrawMapBackgroundInViewport() const;
	/// Draw the map fog of war
	void DrawMapFogOfWar() const;
public:
	/// Draw the full Viewport.
	void Draw() const;
	void DrawBorder() const;
	/// Check if any part of an area is visible in viewport
	bool AnyMapAreaVisibleInViewport(int sx, int sy, int ex, int ey) const;

	int X;                      /// Screen pixel left corner x coordinate
	int Y;                      /// Screen pixel upper corner y coordinate
	int EndX;                   /// Screen pixel right x coordinate
	int EndY;                   /// Screen pixel bottom y coordinate

	int MapX;                   /// Map tile left corner x coordinate
	int MapY;                   /// Map tile upper corner y coordinate
	int OffsetX;                /// X Offset within MapX
	int OffsetY;                /// Y Offset within MapY
	int MapWidth;               /// Width in map tiles
	int MapHeight;              /// Height in map tiles

	CUnit *Unit;                /// Bound to this unit
};

/**
**  Enumeration of the different predefined viewport configurations.
**
**  Add new types of viewports at the end of the enumeration for 
**  backward compatibility of save games.
*/
enum ViewportModeType {
	VIEWPORT_SINGLE,                /// Old single viewport
	VIEWPORT_SPLIT_HORIZ,           /// Two viewports split horizontal
	VIEWPORT_SPLIT_HORIZ3,          /// Three viewports split horiontal
	VIEWPORT_SPLIT_VERT,            /// Two viewports split vertical
	VIEWPORT_QUAD,                  /// Four viewports split symmetric
	NUM_VIEWPORT_MODES,             /// Number of different viewports.
};

class CMapArea
{
public:
	CMapArea() : X(0), Y(0), EndX(0), EndY(0),
		ScrollPaddingLeft(0), ScrollPaddingRight(0),
		ScrollPaddingTop(0), ScrollPaddingBottom(0) {}

	int X;                          /// Screen pixel left corner x coordinate
	int Y;                          /// Screen pixel upper corner y coordinate
	int EndX;                       /// Screen pixel right x coordinate
	int EndY;                       /// Screen pixel bottom y coordinate
	int ScrollPaddingLeft;          /// Scrollable area past the left of map
	int ScrollPaddingRight;         /// Scrollable area past the right of map
	int ScrollPaddingTop;           /// Scrollable area past the top of map
	int ScrollPaddingBottom;        /// Scrollable area past the bottom of map
};

class CFiller
{
public:
	CFiller() : G(NULL), X(0), Y(0) {}

	CGraphic *G;         /// Graphic
	int X;               /// X coordinate
	int Y;               /// Y coordinate
};

class CButtonPanel
{
public:
	CButtonPanel() : G(NULL), X(0), Y(0), ShowCommandKey(true)
	{
		AutoCastBorderColorRGB.r = 0;
		AutoCastBorderColorRGB.g = 0;
		AutoCastBorderColorRGB.b = 0;
	}

	void Draw();
	void Update();
	void DoClicked(int button);
	int DoKey(int key);

	CGraphic *G;
	int X;
	int Y;
	std::vector<CUIButton> Buttons;
	SDL_Color AutoCastBorderColorRGB;
	bool ShowCommandKey;
};

class CPieMenu {
public:
	CPieMenu() : G(NULL), MouseButton(NoButton)
	{
		memset(this->X, 0, sizeof(this->X));
		memset(this->Y, 0, sizeof(this->Y));
	}

	CGraphic *G;         /// Optional background image
	int MouseButton;     /// Which mouse button pops up the piemenu, deactivate with NoButton
	int X[8];            /// X position of the pies
	int Y[8];            /// Y position of the pies

	void SetRadius(int radius) {
		const int coeffX[] = {    0,  193, 256, 193,   0, -193, -256, -193};
		const int coeffY[] = { -256, -193,   0, 193, 256,  193,    0, -193};
		for (int i = 0; i < 8; ++i) {
			this->X[i] = (coeffX[i] * radius) >> 8;
			this->Y[i] = (coeffY[i] * radius) >> 8;
		}
	}
};

class CResourceInfo {
public:
	CResourceInfo() : G(NULL), IconFrame(0), IconX(0), IconY(0),
		TextX(-1), TextY(-1) {}

	CGraphic *G;                      /// icon graphic
	int IconFrame;                    /// icon frame
	int IconX;                        /// icon X position
	int IconY;                        /// icon Y position
	int TextX;                        /// text X position
	int TextY;                        /// text Y position
};

class CInfoPanel
{
public:
	CInfoPanel() : X(0), Y(0) {}

	void Draw();

	int X;
	int Y;
};

class CStatusLine
{
public:
	CStatusLine() : Width(0), TextX(0), TextY(0), Font(0) {}

	void Draw();
	void Set(const std::string &status);
	inline const std::string &Get() { return this->StatusLine; }
	void Clear();

	int Width;
	int TextX;
	int TextY;
	CFont *Font;

private:
	std::string StatusLine;
};

class CUITimer
{
public:
	CUITimer() : X(0), Y(0), Font(NULL) {}

	int X;
	int Y;
	CFont *Font;
};


/**
**  Defines the user interface.
*/
class CUserInterface
{
public:
	CUserInterface();
	~CUserInterface();

	void Load();


	bool MouseScroll;                   /// Enable mouse scrolling
	bool KeyScroll;                     /// Enable keyboard scrolling
		/// Middle-Mouse Scroll Speed (screenpixels per mousepixel)
	int MouseScrollSpeedDefault;
		/// Middle-Mouse Scroll Speed with Control pressed
	int MouseScrollSpeedControl;

	int MouseWarpX;                     /// Cursor warp X position
	int MouseWarpY;                     /// Cursor warp Y position

	std::string NormalFontColor;        /// Color for normal text displayed
	std::string ReverseFontColor;       /// Color for reverse text displayed

	std::vector<CFiller> Fillers;       /// Filler graphics

	CResourceInfo Resources[MaxCosts];/// Icon+Text of all resources

	CInfoPanel InfoPanel;               /// Info panel

	CUIButton *SingleSelectedButton;    /// Button for single selected unit

	std::vector<CUIButton> SelectedButtons;/// Selected buttons
	CFont *MaxSelectedFont;             /// Font type to use
	int MaxSelectedTextX;               /// position to place '+#' text
	int MaxSelectedTextY;               /// if > maximum units selected

	CUIButton *SingleTrainingButton;    /// Button for single training
	std::string SingleTrainingText;     /// Text for single training
	CFont *SingleTrainingFont;          /// Font for single traning
	int SingleTrainingTextX;            /// X text position single training
	int SingleTrainingTextY;            /// Y text position single training

	std::vector<CUIButton> TrainingButtons;/// Training buttons
	std::string TrainingText;           /// Multiple Training Text
	CFont *TrainingFont;                /// Multiple Training Font
	int TrainingTextX;                  /// Multiple Training X Text position
	int TrainingTextY;                  /// Multiple Training Y Text position

	std::vector<CUIButton> TransportingButtons;/// Button info for transporting

	// Completed bar
	SDL_Color CompletedBarColorRGB;     /// color for completed bar
	Uint32 CompletedBarColor;           /// color for completed bar
	bool CompletedBarShadow;             /// should complete bar have shadow

	// Button panel
	CButtonPanel ButtonPanel;

	// Pie Menu
	CPieMenu PieMenu;

	// Map area
	ViewportModeType ViewportMode;      /// Current viewport mode
	CViewport *MouseViewport;           /// Viewport containing mouse
	CViewport *SelectedViewport;        /// Current selected active viewport
	int NumViewports;                   /// # Viewports currently used
	CViewport Viewports[MAX_NUM_VIEWPORTS]; /// Parameters of all viewports
	CMapArea MapArea;                   /// geometry of the whole map area
	CFont *MessageFont;                 /// Font used for messages
	int MessageScrollSpeed;             /// Scroll speed in seconds for messages

	// Saved map positions
	int SavedMapPositionX[MAX_SAVED_MAP_POSITIONS]; /// Saved map position X
	int SavedMapPositionY[MAX_SAVED_MAP_POSITIONS]; /// Saved map position Y

	// Menu buttons
	CUIButton MenuButton;               /// menu button
	CUIButton NetworkMenuButton;        /// network menu button
	CUIButton NetworkDiplomacyButton;   /// network diplomacy button

	// The minimap
	CMinimap Minimap;                   /// minimap
	Uint32 ViewportCursorColor;         /// minimap cursor color

	// The status line
	CStatusLine StatusLine;             /// status line

	// Game timer
	CUITimer Timer;                     /// game timer

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

/// @todo could use different sounds/speech for the errors
/// Is in gamesounds?
/// SoundConfig PlacementError;         /// played on placements errors
/// SoundConfig PlacementSuccess;       /// played on placements success
/// SoundConfig Click;                  /// click noice used often
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CUserInterface UI;                           /// The user interface

	/// Hash table of all the button styles
extern std::map<std::string, ButtonStyle *> ButtonStyleHash;

extern bool RightButtonAttacks;         /// right button attacks
extern ButtonAction *CurrentButtons;    /// Current Selected Buttons

extern std::string UiGroupKeys;         /// Up to 11 keys used for group selection

// only exported to save them

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Initialize the ui
extern void InitUserInterface(void);
	/// Save the ui state
extern void SaveUserInterface(CFile *file);
	/// Clean up the ui module
extern void CleanUserInterface(void);
#ifdef DEBUG
extern void FreeButtonStyles();
#endif
	/// Register ccl features
extern void UserInterfaceCclRegister(void);

	/// Find a button style
extern ButtonStyle *FindButtonStyle(const std::string &style);

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
extern CViewport *GetViewport(int x, int y);
	/// Cycle through all available viewport modes
extern void CycleViewportMode(int);
	/// Select viewport mode
extern void SetViewportMode(ViewportModeType mode);

	/// Use the mouse to scroll the map
extern void MouseScrollMap(int x, int y);
	/// Check if mouse scrolling is enabled
extern bool GetMouseScroll(void);
	/// Enable/disable scrolling with the mouse
extern void SetMouseScroll(bool enabled);
	/// Check if keyboard scrolling is enabled
extern bool GetKeyScroll(void);
	/// Enable/disable scrolling with the keyboard
extern void SetKeyScroll(bool enabled);
	/// Check if mouse grabbing is enabled
extern bool GetGrabMouse(void);
	/// Enable/disable grabbing the mouse
extern void SetGrabMouse(bool enabled);
	/// Check if scrolling stops when leaving the window
extern bool GetLeaveStops(void);
	/// Enable/disable leaving the window stops scrolling
extern void SetLeaveStops(bool enabled);
	/// Set saved map position
extern void SetSavedMapPosition(int index, int x, int y);
	/// Recall map position
extern void RecallSavedMapPosition(int index);

extern int AddHandler(lua_State *l);
extern void CallHandler(unsigned int handle, int value);


//@}

#endif // !__UI_H__
