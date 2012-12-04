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
//

#ifndef __UI_H__
#define __UI_H__

//@{

/// @todo this only the start of the new user interface
/// @todo all user interface variables should go here and be configurable

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#ifndef __CURSOR_H__
#include "cursor.h"
#endif
#ifndef __INTERFACE_H__
#include "interface.h"
#endif
#ifndef __SCRIPT_H__
#include "script.h"
#endif
#ifndef __MINIMAP_H__
#include "minimap.h"
#endif

#include "color.h"
#include "viewport.h"

#include <vector>
#include <string>
#include <map>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CFile;
class CFont;
class LuaActionListener;
class CPopup;

/*----------------------------------------------------------------------------
--  Definitions
----------------------------------------------------------------------------*/

enum TextAlignment {
	TextAlignUndefined,
	TextAlignCenter,
	TextAlignLeft,
	TextAlignRight
};

class ButtonStyleProperties
{
public:
	ButtonStyleProperties() : Sprite(NULL), Frame(0), BorderColor(0),
		BorderSize(0), TextAlign(TextAlignUndefined),
		TextPos(0, 0)
	{}

	CGraphic *Sprite;
	int Frame;
	CColor BorderColorRGB;
	IntColor BorderColor;
	int BorderSize;
	TextAlignment TextAlign;        /// Text alignment
	PixelPos TextPos;               /// Text location
	std::string TextNormalColor;    /// Normal text color
	std::string TextReverseColor;   /// Reverse text color
};

class ButtonStyle
{
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
class CUIButton
{
public:
	CUIButton() : X(0), Y(0), Style(NULL), Callback(NULL) {}
	~CUIButton() {}

	bool Contains(const PixelPos &screenPos) const;

public:
	int X;                          /// x coordinate on the screen
	int Y;                          /// y coordinate on the screen
	std::string Text;               /// button text
	ButtonStyle *Style;             /// button style
	LuaActionListener *Callback;    /// callback function
};

#define MAX_NUM_VIEWPORTS 8         /// Number of supported viewports

/**
**  Enumeration of the different predefined viewport configurations.
**
**  @todo this should be later user configurable
*/
enum ViewportModeType {
	VIEWPORT_SINGLE = 0,                /// Old single viewport
	VIEWPORT_SPLIT_HORIZ,           /// Two viewports split horizontal
	VIEWPORT_SPLIT_HORIZ3,          /// Three viewports split horiontal
	VIEWPORT_SPLIT_VERT,            /// Two viewports split vertical
	VIEWPORT_QUAD,                  /// Four viewports split symmetric
	NUM_VIEWPORT_MODES             /// Number of different viewports.
};

class CMapArea
{
public:
	CMapArea() : X(0), Y(0), EndX(0), EndY(0),
		ScrollPaddingLeft(0), ScrollPaddingRight(0),
		ScrollPaddingTop(0), ScrollPaddingBottom(0) {}

	bool Contains(const PixelPos &screenPos) const;

public:
	int X;                          /// Screen pixel left corner x coordinate
	int Y;                          /// Screen pixel upper corner y coordinate
	int EndX;                       /// Screen pixel right x coordinate
	int EndY;                       /// Screen pixel bottom y coordinate
	int ScrollPaddingLeft;          /// Scrollable area past the left of map
	int ScrollPaddingRight;         /// Scrollable area past the right of map
	int ScrollPaddingTop;           /// Scrollable area past the top of map
	int ScrollPaddingBottom;        /// Scrollable area past the bottom of map
};

/**
**  Condition to show panel content.
*/
class ConditionPanel
{
public:
	ConditionPanel() : ShowOnlySelected(false), HideNeutral(false),
		HideAllied(false), ShowOpponent(false), BoolFlags(NULL),
		Variables(NULL) {}
	~ConditionPanel() {
		delete[] BoolFlags;
		delete[] Variables;
	}

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
class CContentType
{
public:
	CContentType() : PosX(0), PosY(0), Condition(NULL) {}
	virtual ~CContentType() { delete Condition; }

	/// Tell how show the variable Index.
	virtual void Draw(const CUnit &unit, CFont *defaultfont) const = 0;

	virtual void Parse(lua_State *l) = 0;

	int PosX;             /// X coordinate where to display.
	int PosY;             /// Y coordinate where to display.

	ConditionPanel *Condition; /// Condition to show the content; if NULL, no condition.
};

/**
**  Show simple text followed by variable value.
*/
class CContentTypeText : public CContentType
{
public:
	CContentTypeText() : Text(NULL), Font(NULL), Centered(0), Index(-1),
		Component(VariableValue), ShowName(0), Stat(0) {}
	virtual ~CContentTypeText() {
		FreeStringDesc(Text);
		delete Text;
	}

	virtual void Draw(const CUnit &unit, CFont *defaultfont) const;
	virtual void Parse(lua_State *l);

private:
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
class CContentTypeFormattedText : public CContentType
{
public:
	CContentTypeFormattedText() : Font(NULL), Centered(false),
		Index(-1), Component(VariableValue) {}
	virtual ~CContentTypeFormattedText() {}

	virtual void Draw(const CUnit &unit, CFont *defaultfont) const;
	virtual void Parse(lua_State *l);

private:
	std::string Format;          /// Text to display
	CFont *Font;                 /// Font to use.
	bool Centered;               /// if true, center the display.
	int Index;                   /// Index of the variable to show.
	EnumVariable Component;      /// Component of the variable.
};

/**
**  Show formatted text with variable value.
*/
class CContentTypeFormattedText2 : public CContentType
{
public:
	CContentTypeFormattedText2() : Font(NULL), Centered(false),
		Index1(-1), Component1(VariableValue), Index2(-1), Component2(VariableValue) {}
	virtual ~CContentTypeFormattedText2() {}

	virtual void Draw(const CUnit &unit, CFont *defaultfont) const;
	virtual void Parse(lua_State *l);

private:
	std::string Format;          /// Text to display
	CFont *Font;                 /// Font to use.
	bool Centered;               /// if true, center the display.
	int Index1;                  /// Index of the variable1 to show.
	EnumVariable Component1;     /// Component of the variable1.
	int Index2;                  /// Index of the variable to show.
	EnumVariable Component2;     /// Component of the variable.
};

/**
**  Show icon of the unit
*/
class CContentTypeIcon : public CContentType
{
public:
	virtual void Draw(const CUnit &unit, CFont *defaultfont) const;
	virtual void Parse(lua_State *l);

private:
	EnumUnit UnitRef;           /// Which unit icon to display.(itself, container, ...)
};

/**
**  Show bar which change color depend of value.
*/
class CContentTypeLifeBar : public CContentType
{
public:
	CContentTypeLifeBar() : Index(-1), Width(0), Height(0) {}

	virtual void Draw(const CUnit &unit, CFont *defaultfont) const;
	virtual void Parse(lua_State *l);

private:
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
class CContentTypeCompleteBar : public CContentType
{
public:
	CContentTypeCompleteBar() : Index(-1), Width(0), Height(0), Border(0), Color(0) {}

	virtual void Draw(const CUnit &unit, CFont *defaultfont) const;
	virtual void Parse(lua_State *l);

private:
	int Index;           /// Index of the variable to show, -1 if not.
	int Width;           /// Width of the bar.
	int Height;          /// Height of the bar.
	char Border;         /// True for additional border.
	char Color;          /// Color to show
};

/**
**  Info for the panel.
*/
class CUnitInfoPanel
{
public:
	CUnitInfoPanel() : PosX(0), PosY(0), DefaultFont(0),
		Contents(), Condition(NULL) {}
	~CUnitInfoPanel() {
		for (std::vector<CContentType *>::iterator content = Contents.begin();
			 content != Contents.end(); ++content) {
			delete *content;
		}
		delete Condition;
	}


	std::string Name;      /// Ident of the panel.
	int PosX;              /// X coordinate of the panel.
	int PosY;              /// Y coordinate of the panel.
	CFont *DefaultFont;    /// Default font for content.

	std::vector<CContentType *>Contents; /// Array of contents to display.

	ConditionPanel *Condition; /// Condition to show the panel; if NULL, no condition.
};


class CFiller
{
	struct bits_map {
		bits_map() : Width(0), Height(0), bstore(NULL) {}
		~bits_map();

		void Init(CGraphic *g);

		bool TransparentPixel(int x, int y) {
			if (bstore) {
				const unsigned int x_index = x / 32;
				y *= Width;
				y /= 32;
				x -= (x_index * 32);
				return ((bstore[y + x_index] & (1 << x)) == 0);
			}
			return false;
		};

		int Width;
		int Height;
		unsigned int *bstore;
	};

	bits_map map;
public:
	CFiller() : G(NULL), X(0), Y(0) {}

	void Load();

	bool OnGraphic(int x, int y) {
		x -= X;
		y -= Y;
		if (x >= 0 && y >= 0 && x < map.Width && y < map.Height) {
			return !map.TransparentPixel(x, y);
		}
		return false;
	}
	CGraphic *G;         /// Graphic
	int X;               /// X coordinate
	int Y;               /// Y coordinate
};

class CButtonPanel
{
public:
	CButtonPanel() : G(NULL), X(0), Y(0), ShowCommandKey(true)
	{}

	void Draw();
	void Update();
	void DoClicked(int button);
	int DoKey(int key);

private:
	void DoClicked_SelectTarget(int button);

	void DoClicked_Unload(int button);
	void DoClicked_SpellCast(int button);
	void DoClicked_Repair(int button);
	void DoClicked_Return();
	void DoClicked_Stop();
	void DoClicked_StandGround();
	void DoClicked_Button(int button);
	void DoClicked_CancelUpgrade();
	void DoClicked_CancelTrain();
	void DoClicked_CancelBuild();
	void DoClicked_Build(int button);
	void DoClicked_Train(int button);
	void DoClicked_UpgradeTo(int button);
	void DoClicked_Research(int button);


public:
	CGraphic *G;
	int X;
	int Y;
	std::vector<CUIButton> Buttons;
	CColor AutoCastBorderColorRGB;
	bool ShowCommandKey;
};

class CPieMenu
{
public:
	CPieMenu() : G(NULL), MouseButton(NoButton) {
		memset(this->X, 0, sizeof(this->X));
		memset(this->Y, 0, sizeof(this->Y));
	}

	CGraphic *G;         /// Optional background image
	int MouseButton;     /// Which mouse button pops up the piemenu, deactivate with NoButton
	int X[9];            /// X position of the pies
	int Y[9];            /// Y position of the pies

	void SetRadius(int radius) {
		const int coeffX[] = {    0,  193, 256, 193,   0, -193, -256, -193, 0};
		const int coeffY[] = { -256, -193,   0, 193, 256,  193,    0, -193, 0};
		for (int i = 0; i < 9; ++i) {
			this->X[i] = (coeffX[i] * radius) >> 8;
			this->Y[i] = (coeffY[i] * radius) >> 8;
		}
	}
};

/// Popup System

#define MARGIN_X 4
#define MARGIN_Y 2

class PopupConditionPanel
{
public:
	PopupConditionPanel() :  HasHint(false), HasDescription(false), HasDependencies(false),
		ButtonAction(-1), BoolFlags(NULL), Variables(NULL) {}
	~PopupConditionPanel() {
		delete[] BoolFlags;
		delete[] Variables;
	}

	bool HasHint;               /// check if button has hint.
	bool HasDescription;        /// check if button has description.
	bool HasDependencies;       /// check if button has dependencies or restrictions.
	int ButtonAction;           /// action type of button
	std::string ButtonValue;    /// value used in ValueStr field of button

	char *BoolFlags;            /// array of condition about user flags.
	char *Variables;            /// array of variable to verify (enable and max > 0)
};

class CPopupContentType
{
public:
	CPopupContentType() : pos(0, 0),
		MarginX(MARGIN_X), MarginY(MARGIN_Y), minSize(0, 0),
		Wrap(true), Condition(NULL) {}
	virtual ~CPopupContentType() { delete Condition; }

	/// Tell how show the variable Index.
	virtual void Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const ButtonAction &button, int *Costs) const = 0;
	/// Get the content's width
	virtual int GetWidth(const ButtonAction &button, int *Costs) const = 0;
	/// Get the content's height
	virtual int GetHeight(const ButtonAction &button, int *Costs) const = 0;

	virtual void Parse(lua_State *l) = 0;

	static CPopupContentType *ParsePopupContent(lua_State *l);

public:
	PixelPos pos;               /// position to draw.

	int MarginX;                /// Left and right margin width.
	int MarginY;                /// Upper and lower margin height.
	PixelSize minSize;          /// Minimal size covered by content type.
	bool Wrap;                  /// If true, the next content will be placed on the next "line".
protected:
	std::string TextColor;      /// Color used for plain text in content.
	std::string HighlightColor; /// Color used for highlighted letters.
public:
	PopupConditionPanel *Condition; /// Condition to show the content; if NULL, no condition.
};

enum PopupButtonInfo_Types {
	PopupButtonInfo_Hint,
	PopupButtonInfo_Description,
	PopupButtonInfo_Dependencies
};

class CPopupContentTypeButtonInfo : public CPopupContentType
{
public:
	CPopupContentTypeButtonInfo() : InfoType(0), MaxWidth(0), Font(NULL) {}
	virtual ~CPopupContentTypeButtonInfo() {}

	virtual void Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const ButtonAction &button, int *Costs) const;

	virtual int GetWidth(const ButtonAction &button, int *Costs) const;
	virtual int GetHeight(const ButtonAction &button, int *Costs) const;

	virtual void Parse(lua_State *l);

private:
	int InfoType;                /// Type of information to show.
	unsigned int MaxWidth;       /// Maximum width of multilined information.
	CFont *Font;                 /// Font to use.
};

class CPopupContentTypeText : public CPopupContentType
{
public:
	CPopupContentTypeText() : MaxWidth(0), Font(NULL) {}
	virtual ~CPopupContentTypeText() {}

	virtual void Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const ButtonAction &button, int *Costs) const;

	virtual int GetWidth(const ButtonAction &button, int *Costs) const;
	virtual int GetHeight(const ButtonAction &button, int *Costs) const;

	virtual void Parse(lua_State *l);

private:
	std::string Text;            /// Text to display
	unsigned int MaxWidth;       /// Maximum width of multilined text.
	CFont *Font;                 /// Font to use.
};

class CPopupContentTypeCosts : public CPopupContentType
{
public:
	CPopupContentTypeCosts() : Font(NULL), Centered(0) {}
	virtual ~CPopupContentTypeCosts() {}

	virtual void Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const ButtonAction &button, int *Costs) const;

	virtual int GetWidth(const ButtonAction &button, int *Costs) const;
	virtual int GetHeight(const ButtonAction &button, int *Costs) const;

	virtual void Parse(lua_State *l);

private:
	CFont *Font;                 /// Font to use.
	char Centered;               /// if true, center the display.
};

class CPopupContentTypeLine : public CPopupContentType
{
public:
	CPopupContentTypeLine();
	virtual ~CPopupContentTypeLine() {}

	virtual void Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const ButtonAction &button, int *Costs) const;

	virtual int GetWidth(const ButtonAction &button, int *Costs) const;
	virtual int GetHeight(const ButtonAction &button, int *Costs) const;

	virtual void Parse(lua_State *l);

private:
	IntColor Color;  /// Color used for line.
	unsigned int Width;     /// line height
	unsigned int Height;    /// line height
};

class CPopupContentTypeVariable : public CPopupContentType
{
public:
	CPopupContentTypeVariable() : Text(NULL), Font(NULL), Centered(0), Index(-1) {}
	virtual ~CPopupContentTypeVariable() {
		FreeStringDesc(Text);
		delete Text;
	}

	virtual void Draw(int x, int y, const CPopup &popup, const unsigned int popupWidth, const ButtonAction &button, int *Costs) const;

	virtual int GetWidth(const ButtonAction &button, int *Costs) const;
	virtual int GetHeight(const ButtonAction &button, int *Costs) const;

	virtual void Parse(lua_State *l);

private:
	StringDesc *Text;            /// Text to display.
	CFont *Font;                 /// Font to use.
	char Centered;               /// if true, center the display.
	int Index;                   /// Index of the variable to show, -1 if not.
};

class CPopup
{
public:
	CPopup();
	~CPopup();

	std::vector<CPopupContentType *> Contents; /// Array of contents to display.
	std::string Ident;                         /// Ident of the popup.
	int MarginX;                               /// Left and right margin width.
	int MarginY;                               /// Upper and lower margin height.
	int MinWidth;                              /// Minimal width covered by popup.
	int MinHeight;                             /// Minimal height covered by popup.
	CFont *DefaultFont;                        /// Default font for content.
	IntColor BackgroundColor;                  /// Color used for popup's background.
	IntColor BorderColor;                      /// Color used for popup's borders.
};

class CResourceInfo
{
public:
	CResourceInfo() : G(NULL), IconFrame(0), IconX(0), IconY(0), IconWidth(-1),
		TextX(-1), TextY(-1) {}

	CGraphic *G;   /// icon graphic
	int IconFrame; /// icon frame
	int IconX;     /// icon X position
	int IconY;     /// icon Y position
	int IconWidth; /// icon W size
	int TextX;     /// text X position
	int TextY;     /// text Y position
};
#define MaxResourceInfo  MaxCosts + 4 /// +4 for food and score and mana and free workers count

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
	/// Mouse Scroll Speed (screenpixels per mousepixel)
	int MouseScrollSpeed;
	/// Middle-Mouse Scroll Speed (screenpixels per mousepixel)
	int MouseScrollSpeedDefault;
	/// Middle-Mouse Scroll Speed with Control pressed
	int MouseScrollSpeedControl;

	PixelPos MouseWarpPos;              /// Cursor warp screen position

	std::string NormalFontColor;        /// Color for normal text displayed
	std::string ReverseFontColor;       /// Color for reverse text displayed

	std::vector<CFiller> Fillers;       /// Filler graphics

	CResourceInfo Resources[MaxResourceInfo];/// Icon+Text of all resources

	CInfoPanel InfoPanel;               /// Info panel
	std::vector<CUnitInfoPanel *> InfoPanelContents;/// Info panel contents

	std::vector<CPopup *> ButtonPopups; /// Popup windows for buttons

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

	CUIButton *UpgradingButton;         /// Button info for upgrade

	CUIButton *ResearchingButton;       /// Button info for researching

	std::vector<CUIButton> TransportingButtons;/// Button info for transporting

	// Completed bar
	CColor CompletedBarColorRGB;     /// color for completed bar
	IntColor CompletedBarColor;      /// color for completed bar
	bool CompletedBarShadow;         /// should complete bar have shadow

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

	// Menu buttons
	CUIButton MenuButton;               /// menu button
	CUIButton NetworkMenuButton;        /// network menu button
	CUIButton NetworkDiplomacyButton;   /// network diplomacy button

	// The minimap
	CMinimap Minimap;                   /// minimap
	IntColor ViewportCursorColor;       /// minimap cursor color

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

	CGraphic *VictoryBackgroundG;       /// Victory background graphic
	CGraphic *DefeatBackgroundG;        /// Defeat background graphic
};

/*
 *	Basic Shared Pointer for Current Selected Buttons
 *	parallel drawing problems.
 */
class ButtonActionProxy
{
	ButtonAction *ptr;		// pointer to the ButtonAction array
	int *count;				// shared number of owners

	void dispose() {
		if (count == NULL || --*count == 0) {
			delete count;
			delete[] ptr;
		}
	}

	ButtonActionProxy &operator= (ButtonAction *p) {
		if (this->ptr != p) {
			if (count == NULL || --*count == 0) {
				delete[] ptr;
				if (count) {
					*count = 1;
				}
			}
			ptr = p;
		}
		return *this;
	}

	friend void CButtonPanel::Update();
public:

	ButtonActionProxy(): ptr(0), count(0) {}

	ButtonActionProxy(const ButtonActionProxy &p)
		: ptr(p.ptr), count(p.count) {
		if (!count) {
			count = new int(1);
			*count = 1;
		}
		++*count;
	}

	~ButtonActionProxy() {
		dispose();
	}

	void Reset() {
		dispose();
		count = NULL;
		ptr = NULL;
	}

	ButtonAction &operator[](unsigned int index) {
		return ptr[index];
	}

	bool IsValid() {
		return ptr != NULL;
	}

};

extern ButtonActionProxy CurrentButtons;    /// Current Selected Buttons

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CUserInterface UI;                           /// The user interface

extern std::string ClickMissile;            /// Missile to show when you click
extern std::string DamageMissile;           /// Missile to show damage caused


/// Hash table of all the button styles
extern std::map<std::string, ButtonStyle *> ButtonStyleHash;

extern bool RightButtonAttacks;         /// right button attacks

extern const char DefaultGroupKeys[];         /// Default group keys
extern std::string UiGroupKeys;               /// Up to 11 keys used for group selection

extern bool FancyBuildings;             /// Mirror buildings 1 yes, 0 now.

// only exported to save them

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Initialize the ui
extern void InitUserInterface();
/// Save the ui state
extern void SaveUserInterface(CFile &file);
/// Clean up the ui module
extern void CleanUserInterface();
#ifdef DEBUG
extern void FreeButtonStyles();
#endif
/// Register ccl features
extern void UserInterfaceCclRegister();

/// return popup by ident string
extern CPopup *PopupByIdent(const std::string &ident);

/// Find a button style
extern ButtonStyle *FindButtonStyle(const std::string &style);

/// Called if the mouse is moved in Normal interface state
extern void UIHandleMouseMove(const PixelPos &pos);
/// Called if any mouse button is pressed down
extern void UIHandleButtonDown(unsigned button);
/// Called if any mouse button is released up
extern void UIHandleButtonUp(unsigned button);

/// Restrict mouse cursor to viewport
extern void RestrictCursorToViewport();
/// Restrict mouse cursor to minimap
extern void RestrictCursorToMinimap();

/// Get viewport for screen pixel position
extern CViewport *GetViewport(const PixelPos &screenPos);
/// Cycle through all available viewport modes
extern void CycleViewportMode(int);
/// Select viewport mode
extern void SetViewportMode(ViewportModeType mode);
extern void CheckViewportMode();

/// Use the mouse to scroll the map
extern void MouseScrollMap(int x, int y);
/// Check if mouse scrolling is enabled
extern bool GetMouseScroll();
/// Enable/disable scrolling with the mouse
extern void SetMouseScroll(bool enabled);
/// Check if keyboard scrolling is enabled
extern bool GetKeyScroll();
/// Enable/disable scrolling with the keyboard
extern void SetKeyScroll(bool enabled);
/// Check if mouse grabbing is enabled
extern bool GetGrabMouse();
/// Enable/disable grabbing the mouse
extern void SetGrabMouse(bool enabled);
/// Check if scrolling stops when leaving the window
extern bool GetLeaveStops();
/// Enable/disable leaving the window stops scrolling
extern void SetLeaveStops(bool enabled);

extern int AddHandler(lua_State *l);
extern void CallHandler(unsigned int handle, int value);

/// Show load progress
extern void ShowLoadProgress(const char *fmt, ...) PRINTF_VAARG_ATTRIBUTE(1, 2);

//@}

#endif // !__UI_H__
