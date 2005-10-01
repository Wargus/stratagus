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
//      (c) Copyright 1999-2005 by Lutz Sammer and Jimmy Salmon
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
	int Font;                       /// Font
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
	int Font;                       /// Font
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
class Button {
public:
	Button() : X(0), Y(0), Text(NULL), Style(NULL) {}
	~Button() { delete[] Text; }

	int X;                          /// x coordinate on the screen
	int Y;                          /// y coordinate on the screen
	char *Text;                     /// button text
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
	/// Check if any part of an area is visible in viewport
	int AnyMapAreaVisibleInViewport(int sx, int sy, int ex, int ey) const;

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

#define ScPanel "sc-panel"          /// hack for transparency

/**
**  Menu panels
*/
class MenuPanel {
public:
	MenuPanel() : Ident(NULL), G(NULL), Next(NULL) {}

	char      *Ident;           /// Unique identifier
	CGraphic   *G;              /// Graphic
	MenuPanel *Next;            /// Next pointer
};

/**
**  Condition to show panel content.
*/
class ConditionPanel {
public:
	ConditionPanel() : ShowOnlySelected(false), HideNeutral(false),
		HideAllied(false), ShowOpponent(false), BoolFlags(NULL),
		Variables(NULL) {}

	bool ShowOnlySelected;      /// if true, show only for selected unit.

	bool HideNeutral;           /// if true, don't show for neutral unit.
	bool HideAllied;            /// if true, don't show for allied unit. (but show own units)
	bool ShowOpponent;          /// if true, show for opponent unit.

	char *BoolFlags;            /// array of condition about user flags.
	char *Variables;            /// array of variable to verify (enable and max > 0)
};

class ContentType;
typedef void FDrawData(const CUnit *unit, ContentType *content, int defaultfont);

/**
**  Infos to display the contents of panel.
*/
class ContentType {
public:
	int PosX;             /// X coordinate where to display.
	int PosY;             /// Y coordinate where to display.

	FDrawData *DrawData;  /// Tell how show the variable Index.

	union {
		struct {
			StringDesc *Text;            /// Text to display.
			int Font;                    /// Font to use.
			char Centered;               /// if true, center the display.
			int Index;                   /// Index of the variable to show, -1 if not.
			EnumVariable Component;      /// Component of the variable.
			char ShowName;               /// If true, Show name's unit.
			char Stat;                   /// true to special display.(value or value + diff)
		} SimpleText;   /// Show simple text followed by variable value.
		struct {
			char *Format;                /// Text to display
			int Font;                    /// Font to use.
			int Index;                   /// Index of the variable to show.
			EnumVariable Component;      /// Component of the variable.
			char Centered;               /// if true, center the display.
		} FormattedText;   /// Show formatted text with variable value.
		struct {
			char *Format;                /// Text to display
			int Font;                    /// Font to use.
			int Index1;                  /// Index of the variable1 to show.
			EnumVariable Component1;     /// Component of the variable1.
			int Index2;                  /// Index of the variable to show.
			EnumVariable Component2;     /// Component of the variable.
			char Centered;               /// if true, center the display.
		} FormattedText2;   /// Show formatted text with 2 variable value.
		struct {
			EnumUnit UnitRef;           /// Which unit icon to display.(itself, container, ...)
		} Icon;         /// Show icon of the unit
		struct {
			int Index;           /// Index of the variable to show, -1 if not.
			int Width;           /// Width of the bar.
			int Height;          /// Height of the bar.
#if 0 // FIXME : something for color and value parametrisation (not implemented)
			Color *colors;       /// array of color to show (depend of value)
			int *values;         /// list of percentage to change color.
#endif
		} LifeBar;      /// Show bar which change color depend of value.
		struct {
			int Index;           /// Index of the variable to show, -1 if not.
			int Width;           /// Width of the bar.
			int Height;          /// Height of the bar.
			char Border;         /// True for additional border.
#if 0 // FIXME : something for color parametrisations (not implemented)
// take UI.CompletedBar color for the moment.
			Color colors;        /// Color to show (depend of value)
#endif
		} CompleteBar;  /// Show bar.
	} Data;

// FIXME : Complete this.

	ConditionPanel *Condition; /// Condition to show the content; if NULL, no condition.
};

/**
**  Info for the panel.
*/
class InfoPanel {
public:
	InfoPanel() : Name(NULL), PosX(0), PosY(0), DefaultFont(0),
		Contents(NULL), NContents(0), Condition(NULL) {}

	char *Name;            /// Ident of the panel.
	int PosX;              /// X coordinate of the panel.
	int PosY;              /// Y coordinate of the panel.
	int DefaultFont;       /// Default font for content.

	ContentType *Contents; /// Array of contents to display.
	int NContents;         /// Number of content.

	ConditionPanel *Condition; /// Condition to show the panel; if NULL, no condition.
};

extern std::vector<InfoPanel> AllPanels;  /// Array of panels.


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
	std::vector<Button> Buttons;
	SDL_Color AutoCastBorderColorRGB;
	bool ShowCommandKey;
};

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
	CStatusLine() : W(0), TextX(0), TextY(0), Font(0)
	{
		StatusLine[0] = '\0';
	}

	void Draw();
	void Set(const char *status);
	void Clear();

	int W;
	int TextX;
	int TextY;
	int Font;

private:
	char StatusLine[256];
};

/**
**  Defines the user interface.
*/
class CUserInterface {
public:
	char *Name;                         /// interface name to select
	int Width;                          /// useable for this width
	int Height;                         /// useable for this height

	bool MouseScroll;                   /// Enable mouse scrolling
	bool KeyScroll;                     /// Enable keyboard scrolling
		/// Middle-Mouse Scroll Speed (screenpixels per mousepixel)
	int MouseScrollSpeedDefault;
		/// Middle-Mouse Scroll Speed with Control pressed
	int MouseScrollSpeedControl;

	int MouseWarpX;                     /// Cursor warp X position
	int MouseWarpY;                     /// Cursor warp Y position

	char *NormalFontColor;              /// Color for normal text displayed
	char *ReverseFontColor;             /// Color for reverse text displayed

	// Fillers
	std::vector<CGraphic *> Filler;      /// Filler graphics
	std::vector<int> FillerX;           /// Filler X positions
	std::vector<int> FillerY;           /// Filler Y positions

	struct {
		CGraphic *G;                     /// icon graphic
		int IconFrame;                  /// icon frame
		int IconX;                      /// icon X position
		int IconY;                      /// icon Y position
		int TextX;                      /// text X position
		int TextY;                      /// text Y position
	} Resources[MaxCosts + 2];          /// Icon+Text of all resources
                                        /// +2 for food and score

	// Info panel
	CInfoPanel InfoPanel;
	char*   PanelIndex;                 /// Index of the InfoPanel.
	char    NumberPanel;                /// Number of Panel.

	Button* SingleSelectedButton;       /// Button for single selected unit

	std::vector<Button> SelectedButtons;/// Selected buttons
	int     MaxSelectedFont;            /// Font type to use
	int     MaxSelectedTextX;           /// position to place '+#' text
	int     MaxSelectedTextY;           /// if > maximum units selected

	Button* SingleTrainingButton;       /// Button for single training
	char*   SingleTrainingText;         /// Text for single training
	int     SingleTrainingFont;         /// Font for single traning
	int     SingleTrainingTextX;        /// X text position single training
	int     SingleTrainingTextY;        /// Y text position single training

	std::vector<Button> TrainingButtons;/// Training buttons
	char*   TrainingText;               /// Multiple Training Text
	int     TrainingFont;               /// Multiple Training Font
	int     TrainingTextX;              /// Multiple Training X Text position
	int     TrainingTextY;              /// Multiple Training Y Text position

	Button* UpgradingButton;            /// Button info for upgrade

	Button* ResearchingButton;          /// Button info for researching

	std::vector<Button> TransportingButtons;/// Button info for transporting

	// Completed bar
	SDL_Color CompletedBarColorRGB;     /// color for completed bar
	Uint32    CompletedBarColor;        /// color for completed bar
	int       CompletedBarShadow;       /// should complete bar have shadow

	// Button panel
	CButtonPanel  ButtonPanel;

	// Pie Menu
	CGraphic *PieMenuBackgroundG;        /// Optional background image for the piemenu
	int PieMouseButton;/// Which mouse button pops up the piemenu. Deactivate with the NoButton value.
	int PieX[8];                        /// X position of the pies
	int PieY[8];                        /// Y position of the pies

	// Map area
	ViewportModeType ViewportMode;      /// Current viewport mode
	CViewport*    MouseViewport;        /// Viewport containing mouse
	CViewport*    SelectedViewport;     /// Current selected active viewport
	int          NumViewports;          /// # Viewports currently used
	CViewport     Viewports[MAX_NUM_VIEWPORTS]; /// Parameters of all viewports
	// Map* attributes of Viewport are unused here:
	CViewport     MapArea;               /// geometry of the whole map area

	/// Menu buttons
	Button MenuButton;                  /// menu button
	Button NetworkMenuButton;           /// network menu button
	Button NetworkDiplomacyButton;      /// network diplomacy button

	// The minimap
	CMinimap Minimap;
	Uint32        ViewportCursorColor;  /// minimap cursor color

	// The status line
	CStatusLine   StatusLine;

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

	MenuPanel *MenuPanels;              /// Menu panels

	CGraphic *VictoryBackgroundG;        /// Victory background graphic
	CGraphic *DefeatBackgroundG;         /// Defeat background graphic
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CUserInterface UI;                           /// The user interface
extern CUserInterface **UI_Table;                   /// All available user interfaces

	/// Hash table of all the button styles
extern std::map<std::string, ButtonStyle *> ButtonStyleHash;

	/// Hash table of all the checkbox styles
extern std::map<std::string, CheckboxStyle *> CheckboxStyleHash;

extern char RightButtonAttacks;         /// right button 0 move, 1 attack.
extern ButtonAction *CurrentButtons;    /// Current Selected Buttons
extern bool FancyBuildings;             /// Mirror buildings 1 yes, 0 now.

extern int SpeedKeyScroll;              /// Keyboard Scrolling Speed, in Frames
extern int SpeedMouseScroll;            /// Mouse Scrolling Speed, in Frames

extern char DefaultGroupKeys[];    /// Default group keys
extern char *UiGroupKeys;               /// Up to 11 keys used for group selection

// only exported to save them

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Initialize the ui
extern void InitUserInterface(const char *race_name);
	/// Load ui graphics
extern void LoadUserInterface(void);
	/// Save the ui state
extern void SaveUserInterface(CFile *file);
	/// Clean up the Panel.
extern void CleanPanel(InfoPanel *panel);
	/// Clean up a ui
extern void CleanUI(CUserInterface *ui);
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

extern FDrawData DrawSimpleText;
extern FDrawData DrawFormattedText;
extern FDrawData DrawFormattedText2;
extern FDrawData DrawPanelIcon;
extern FDrawData DrawLifeBar;
extern FDrawData DrawCompleteBar;

extern int AddHandler(struct lua_State *l);
extern void CallHandler(unsigned int handle, int value);


//@}

#endif // !__UI_H__
