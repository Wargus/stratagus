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
/**@name popup.h - The Popup header file. */
//
//      (c) Copyright 2012 by Joris Dauphin
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

#ifndef __POPUP_H__
#define __POPUP_H__

//@{

#include "script.h"
#include "color.h"
#include "vec2i.h"
#include <vector>
#include <string>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class ButtonAction;
class CFont;
class CPopup;

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


//@}

#endif // !__UI_H__
