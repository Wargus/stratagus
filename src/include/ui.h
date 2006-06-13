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
//      (c) Copyright 1999-2006 by Lutz Sammer and Jimmy Salmon
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

#include <vector>
#include <string>
#include <map>

#include "util.h"
#include "video.h"
#include "upgrade_structs.h"
#include "cursor.h"
#include "interface.h"
#include "script.h"
#include "minimap.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CFile;

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
		TextX(0), TextY(0), TextNormalColor(NULL), TextReverseColor(NULL)
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
	char *TextNormalColor;          /// Normal text color
	char *TextReverseColor;         /// Reverse text color
} ;

class ButtonStyle {
public:
	ButtonStyle() : Width(0), Height(0), Font(0),
		TextNormalColor(NULL), TextReverseColor(NULL),
		TextAlign(TextAlignUndefined), TextX(0), TextY(0) {}

	int Width;                      /// Button width
	int Height;                     /// Button height
	CFont *Font;                    /// Font
	char *TextNormalColor;          /// Normal text color
	char *TextReverseColor;         /// Reverse text color
	TextAlignment TextAlign;        /// Text alignment
	int TextX;                      /// Text X location
	int TextY;                      /// Text Y location
	ButtonStyleProperties Default;  /// Default button properties
	ButtonStyleProperties Hover;    /// Hover button properties
	ButtonStyleProperties Selected; /// Selected button properties
	ButtonStyleProperties Clicked;  /// Clicked button properties
	ButtonStyleProperties Disabled; /// Disabled button properties
};

class CheckboxStyle {
public:
	CheckboxStyle() : Width(0), Height(0), Font(0),
		TextNormalColor(NULL), TextReverseColor(NULL),
		TextAlign(TextAlignUndefined), TextX(0), TextY(0) {}

	int Width;                      /// Checkbox width
	int Height;                     /// Checkbox height
	CFont *Font;                    /// Font
	char *TextNormalColor;          /// Normal text color
	char *TextReverseColor;         /// Reverse text color
	TextAlignment TextAlign;        /// Text alignment
	int TextX;                      /// Text X location
	int TextY;                      /// Text Y location
	ButtonStyleProperties Default;  /// Default checkbox properties
	ButtonStyleProperties Hover;    /// Hover checkbox properties
	ButtonStyleProperties Selected; /// Selected checkbox properties
	ButtonStyleProperties Clicked;  /// Clicked checkbox properties
	ButtonStyleProperties Disabled; /// Disabled checkbox properties
	ButtonStyleProperties Checked;  /// Default checkbox properties
	ButtonStyleProperties CheckedHover;    /// Checked hover checkbox properties
	ButtonStyleProperties CheckedSelected; /// Checked selected checkbox properties
	ButtonStyleProperties CheckedClicked;  /// Checked clicked checkbox properties
	ButtonStyleProperties CheckedDisabled; /// Checked disabled checkbox properties
};

	/// buttons on screen themselves
class CUIButton {
public:
	CUIButton() : X(0), Y(0), Style(NULL) {}
	~CUIButton() {}

	int X;                          /// x coordinate on the screen
	int Y;                          /// y coordinate on the screen
	std::string Text;               /// button text
	ButtonStyle *Style;             /// button style
};

#define MAX_NUM_VIEWPORTS 8         /// Number of supported viewports

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
**    Viewport is bound to an unit. If the unit moves the viewport
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
**  @todo this should be later user configurable
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

#define ScPanel "sc-panel"          /// hack for transparency

/**
**  Menu panels
*/
class CMenuPanel {
public:
	CMenuPanel() : G(NULL) {}

	std::string Ident;              /// Unique identifier
	CGraphic *G;              /// Graphic
};

/**
**  Condition to show panel content.
*/
class ConditionPanel {
public:
	ConditionPanel() : ShowOnlySelected(false), HideNeutral(false),
		HideAllied(false), ShowOpponent(false), BoolFlags(NULL),
		Variables(NULL) {}
	~ConditionPanel() {
		delete[] BoolFlags;
		delete[] Variables;
	};

	bool ShowOnlySelected;      /// if true, show only for selected unit.

	bool HideNeutral;           /// if true, don't show for neutral unit.
	bool HideAllied;            /// if true, don't show for allied unit. (but show own units)
	bool ShowOpponent;          /// if true, show for opponent unit.

	char *BoolFlags;            /// array of condition about user flags.
	char *Variables;            /// array of variable to verify (enable and max > 0)
};

/**
**  Infos to display the contents of panel.
*/
class CContentType {
public:
	CContentType() : PosX(0), PosY(0), Condition(NULL) {};
	virtual ~CContentType() { delete Condition; };

		/// Tell how show the variable Index.
	virtual void Draw(const CUnit *unit, CFont *defaultfont) const = 0;

	int PosX;             /// X coordinate where to display.
	int PosY;             /// Y coordinate where to display.

	ConditionPanel *Condition; /// Condition to show the content; if NULL, no condition.
};

/**
**  Show simple text followed by variable value.
*/
class CContentTypeText : public CContentType {
public:
	CContentTypeText() : Text(NULL), Font(NULL), Centered(0), Index(-1),
		Component(VariableValue), ShowName(0), Stat(0) {};
	virtual ~CContentTypeText() {
		FreeStringDesc(Text);
		delete Text;
	};

	virtual void Draw(const CUnit *unit, CFont *defaultfont) const;

	StringDesc *Text;            /// Text to display.
	CFont *Font;                 /// Font to use.
	char Centered;               /// if true, center the display.
	int Index;                   /// Index of the variable to show, -1 if not.
	EnumVariable Component;      /// Component of the variable.
	char ShowName;               /// If true, Show name's unit.
	char Stat;                   /// true to special display.(value or value + diff)
};

/**
**  Show formatted text with variable value.
*/
class CContentTypeFormattedText : public CContentType {
public:
	CContentTypeFormattedText() : Format(NULL), Font(NULL), Centered(0),
		Index(-1), Component(VariableValue) {};
	virtual ~CContentTypeFormattedText() { delete [] Format;};

	virtual void Draw(const CUnit *unit, CFont *defaultfont) const;

	char *Format;                /// Text to display
	CFont *Font;                 /// Font to use.
	char Centered;               /// if true, center the display.
	int Index;                   /// Index of the variable to show.
	EnumVariable Component;      /// Component of the variable.
};

/**
**  Show formatted text with variable value.
*/
class CContentTypeFormattedText2 : public CContentType {
public:
	CContentTypeFormattedText2() : Format(NULL), Font(NULL), Centered(0),
		Index1(-1), Component1(VariableValue), Index2(-1), Component2(VariableValue) {};
	virtual ~CContentTypeFormattedText2() { delete [] Format;};

	virtual void Draw(const CUnit *unit, CFont *defaultfont) const;

	char *Format;                /// Text to display
	CFont *Font;                 /// Font to use.
	char Centered;               /// if true, center the display.
	int Index1;                  /// Index of the variable1 to show.
	EnumVariable Component1;     /// Component of the variable1.
	int Index2;                  /// Index of the variable to show.
	EnumVariable Component2;     /// Component of the variable.
};

/**
**  Show icon of the unit
*/
class CContentTypeIcon : public CContentType {
public:
	virtual void Draw(const CUnit *unit, CFont *defaultfont) const;

	EnumUnit UnitRef;           /// Which unit icon to display.(itself, container, ...)
};

/**
**  Show bar which change color depend of value.
*/
class CContentTypeLifeBar : public CContentType {
public:
	CContentTypeLifeBar() : Index(-1), Width(0), Height(0) {};

	virtual void Draw(const CUnit *unit, CFont *defaultfont) const;

	int Index;           /// Index of the variable to show, -1 if not.
	int Width;           /// Width of the bar.
	int Height;          /// Height of the bar.
#if 0 // FIXME : something for color and value parametrisation (not implemented)
	Color *colors;       /// array of color to show (depend of value)
	int *values;         /// list of percentage to change color.
#endif
};

/**
**  Show bar.
*/
class CContentTypeCompleteBar : public CContentType {
public:
	CContentTypeCompleteBar() : Index(-1), Width(0), Height(0), Border(0) {};

	virtual void Draw(const CUnit *unit, CFont *defaultfont) const;

	int Index;           /// Index of the variable to show, -1 if not.
	int Width;           /// Width of the bar.
	int Height;          /// Height of the bar.
	char Border;         /// True for additional border.
#if 0 // FIXME : something for color parametrisations (not implemented)
// take UI.CompletedBar color for the moment.
	Color colors;        /// Color to show (depend of value)
#endif

};

/**
**  Info for the panel.
*/
class CUnitInfoPanel {
public:
	CUnitInfoPanel() : Name(NULL), PosX(0), PosY(0), DefaultFont(0),
		Contents(), Condition(NULL) {};
	~CUnitInfoPanel() {
		delete[] Name;
		for (std::vector<CContentType *>::iterator content = Contents.begin();
			content != Contents.end(); ++content) {
			delete *content;
		}
		Contents.clear();
		delete Condition;
	};


	char *Name;            /// Ident of the panel.
	int PosX;              /// X coordinate of the panel.
	int PosY;              /// Y coordinate of the panel.
	CFont *DefaultFont;    /// Default font for content.

	std::vector<CContentType *>Contents; /// Array of contents to display.

	ConditionPanel *Condition; /// Condition to show the panel; if NULL, no condition.
};

extern std::vector<CUnitInfoPanel *> AllPanels;  /// Array of panels.


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
	CButtonPanel() : G(NULL), X(0), Y(0), ShowCommandKey(false)
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
#define MaxResourceInfo  MaxCosts + 2 /// +2 for food and score

class CInfoPanel
{
public:
	CInfoPanel() : G(NULL), X(0), Y(0) {}

	void Draw();

	CGraphic *G;
	int X;
	int Y;
};

class CStatusLine
{
public:
	CStatusLine() : Width(0), TextX(0), TextY(0), Font(0)
	{
		StatusLine[0] = '\0';
	}

	void Draw();
	void Set(const char *status);
	void Clear();

	int Width;
	int TextX;
	int TextY;
	CFont *Font;

private:
	char StatusLine[256];
};

/**
**  Defines the user interface.
*/
class CUserInterface {
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

	// Fillers
	std::vector<CFiller> Fillers;       /// Filler graphics

	CResourceInfo Resources[MaxResourceInfo];/// Icon+Text of all resources

	// Info panel
	CInfoPanel InfoPanel;               /// Info panel
	char *PanelIndex;                   /// Index of the InfoPanel.
	char NumberPanel;                   /// Number of Panel.

	CUIButton *SingleSelectedButton;    /// Button for single selected unit

	std::vector<CUIButton> SelectedButtons;/// Selected buttons
	CFont *MaxSelectedFont;             /// Font type to use
	int MaxSelectedTextX;               /// position to place '+#' text
	int MaxSelectedTextY;               /// if > maximum units selected

	CUIButton *SingleTrainingButton;    /// Button for single training
	char *SingleTrainingText;           /// Text for single training
	CFont *SingleTrainingFont;          /// Font for single traning
	int SingleTrainingTextX;            /// X text position single training
	int SingleTrainingTextY;            /// Y text position single training

	std::vector<CUIButton> TrainingButtons;/// Training buttons
	char *TrainingText;                 /// Multiple Training Text
	CFont *TrainingFont;                /// Multiple Training Font
	int TrainingTextX;                  /// Multiple Training X Text position
	int TrainingTextY;                  /// Multiple Training Y Text position

	CUIButton *UpgradingButton;         /// Button info for upgrade

	CUIButton *ResearchingButton;       /// Button info for researching

	std::vector<CUIButton> TransportingButtons;/// Button info for transporting

	// Completed bar
	SDL_Color CompletedBarColorRGB;     /// color for completed bar
	Uint32 CompletedBarColor;           /// color for completed bar
	int CompletedBarShadow;             /// should complete bar have shadow

	// Button panel
	CButtonPanel ButtonPanel;

	// Pie Menu
	CGraphic *PieMenuBackgroundG;       /// Optional background image for the piemenu
	int PieMouseButton;/// Which mouse button pops up the piemenu. Deactivate with the NoButton value.
	int PieX[8];                        /// X position of the pies
	int PieY[8];                        /// Y position of the pies

	// Map area
	ViewportModeType ViewportMode;      /// Current viewport mode
	CViewport *MouseViewport;           /// Viewport containing mouse
	CViewport *SelectedViewport;        /// Current selected active viewport
	int NumViewports;                   /// # Viewports currently used
	CViewport Viewports[MAX_NUM_VIEWPORTS]; /// Parameters of all viewports
	CMapArea MapArea;                   /// geometry of the whole map area

	// Menu buttons
	CUIButton MenuButton;               /// menu button
	CUIButton NetworkMenuButton;        /// network menu button
	CUIButton NetworkDiplomacyButton;   /// network diplomacy button

	// The minimap
	CMinimap Minimap;                   /// minimap
	Uint32 ViewportCursorColor;         /// minimap cursor color

	// The status line
	CStatusLine StatusLine;             /// status line

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

	std::vector<CMenuPanel> MenuPanels;/// Menu panels

	CGraphic *VictoryBackgroundG;       /// Victory background graphic
	CGraphic *DefeatBackgroundG;        /// Defeat background graphic
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CUserInterface UI;                           /// The user interface

	/// Hash table of all the button styles
extern std::map<std::string, ButtonStyle *> ButtonStyleHash;

	/// Hash table of all the checkbox styles
extern std::map<std::string, CheckboxStyle *> CheckboxStyleHash;

extern char RightButtonAttacks;         /// right button 0 move, 1 attack.
extern ButtonAction *CurrentButtons;    /// Current Selected Buttons

extern int SpeedKeyScroll;              /// Keyboard Scrolling Speed, in Frames
extern int SpeedMouseScroll;            /// Mouse Scrolling Speed, in Frames

extern char DefaultGroupKeys[];         /// Default group keys
extern char *UiGroupKeys;               /// Up to 11 keys used for group selection

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
	/// Register ccl features
extern void UserInterfaceCclRegister(void);

	/// Find a button style
extern ButtonStyle *FindButtonStyle(const char *style);
	/// Find a checkbox style
extern CheckboxStyle *FindCheckboxStyle(const char *style);

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

	/// Check if mouse scrolling is enabled
extern bool GetMouseScroll(void);
	/// Enable/disable scrolling with the mouse
extern void SetMouseScroll(bool enabled);
	/// Get speed of mouse scrolling
extern int GetMouseScrollSpeed(void);
	/// Set speed of mouse scrolling
extern void SetMouseScrollSpeed(int speed);
	/// Check if keyboard scrolling is enabled
extern bool GetKeyScroll(void);
	/// Enable/disable scrolling with the keyboard
extern void SetKeyScroll(bool enabled);
	/// Get speed of keyboard scrolling
extern int GetKeyScrollSpeed(void);
	/// Set speed of keyboard scrolling
extern void SetKeyScrollSpeed(int speed);
	/// Check if mouse grabbing is enabled
extern bool GetGrabMouse(void);
	/// Enable/disable grabbing the mouse
extern void SetGrabMouse(bool enabled);
	/// Check if scrolling stops when leaving the window
extern bool GetLeaveStops(void);
	/// Enable/disable leaving the window stops scrolling
extern void SetLeaveStops(bool enabled);

extern int AddHandler(lua_State *l);
extern void CallHandler(unsigned int handle, int value);


//@}

#endif // !__UI_H__
