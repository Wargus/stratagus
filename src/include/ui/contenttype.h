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
/**@name contenttype.h - content type header file. */
//
//      (c) Copyright 1999-2012 by Lutz Sammer and Jimmy Salmon
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

#ifndef __CONTENT_TYPE_H__
#define __CONTENT_TYPE_H__

//@{

#include "color.h"
#include "script.h"
#include "vec2i.h"

#include <optional>
#include <vector>

class CUnit;
class CFont;
class ConditionPanel;

/**
**  Infos to display the contents of panel.
*/
class CContentType
{
public:
	CContentType() = default;
	virtual ~CContentType() = default;

	/// Tell how show the variable Index.
	virtual void Draw(const CUnit &unit, CFont *defaultfont) const = 0;

	virtual void Parse(lua_State *l) = 0;

public:
	PixelPos Pos{0, 0}; /// Coordinate where to display.
	std::unique_ptr<ConditionPanel> Condition; /// Condition to show the content; if nullptr, no condition.
};

/**
**  Show simple text followed by variable value.
*/
class CContentTypeText : public CContentType
{
public:
	CContentTypeText() = default;

	void Draw(const CUnit &unit, CFont *defaultfont) const override;
	void Parse(lua_State *l) override;

private:
	std::unique_ptr<IStringDesc> Text; /// Text to display.
	CFont *Font = nullptr;            /// Font to use.
	bool Centered = false;            /// if true, center the display.
	int Index = -1;                   /// Index of the variable to show, -1 if not.
	EnumVariable Component = EnumVariable::Value; /// Component of the variable.
	bool ShowName = false;            /// If true, Show name's unit.
	bool Stat = false;                /// true to special display.(value or value + diff)
};

/**
**  Show formatted text with variable value.
*/
class CContentTypeFormattedText : public CContentType
{
public:
	CContentTypeFormattedText() = default;

	void Draw(const CUnit &unit, CFont *defaultfont) const override;
	void Parse(lua_State *l) override;

private:
	std::string Format;    /// Text to display
	CFont *Font = nullptr; /// Font to use.
	bool Centered = false; /// if true, center the display.
	int Index = -1;        /// Index of the variable to show.
	EnumVariable Component = EnumVariable::Value; /// Component of the variable.
};

/**
**  Show formatted text with variable value.
*/
class CContentTypeFormattedText2 : public CContentType
{
public:
	CContentTypeFormattedText2() = default;

	void Draw(const CUnit &unit, CFont *defaultfont) const override;
	void Parse(lua_State *l) override;

private:
	std::string Format;    /// Text to display
	CFont *Font = nullptr; /// Font to use.
	bool Centered = false; /// if true, center the display.
	int Index1 = -1;       /// Index of the variable1 to show.
	EnumVariable Component1 = EnumVariable::Value; /// Component of the variable1.
	int Index2 = -1;       /// Index of the variable to show.
	EnumVariable Component2 = EnumVariable::Value; /// Component of the variable.
};

/**
**  Enumeration of unit
*/
enum class EnumUnit
{
	UnitRefItSelf = 0, /// unit.
	UnitRefInside, /// unit->Inside.
	UnitRefContainer, /// Unit->Container.
	UnitRefWorker, /// unit->Data.Built.Worker
	UnitRefGoal /// unit->Goal
};

/**
**  Show icon of the unit
*/
class CContentTypeIcon : public CContentType
{
public:
	void Draw(const CUnit &unit, CFont *defaultfont) const override;
	void Parse(lua_State *l) override;

private:
	EnumUnit UnitRef = EnumUnit::UnitRefItSelf; /// Which unit icon to display.(itself, container, ...)
	bool ButtonIcon = true;
	bool SingleSelectionIcon = false;
	bool GroupSelectionIcon = false;
	bool TransportIcon = false;
};

/**
**  Show a graphic
*/
class CContentTypeGraphic : public CContentType
{
public:
	void Draw(const CUnit &unit, CFont *defaultfont) const override;
	void Parse(lua_State *l) override;

private:
	std::string graphic;
	int frame = 0;
};

/**
**  Show bar which change color depend of value.
*/
class CContentTypeLifeBar : public CContentType
{
public:
	CContentTypeLifeBar() = default;

	void Draw(const CUnit &unit, CFont *defaultfont) const override;
	void Parse(lua_State *l) override;

private:
	int Index = -1;        /// Index of the variable to show, -1 if not.
	std::unique_ptr<INumberDesc> ValueFunc;/// Handler of the value function
	int ValueMax = -1;     /// Max, when used with a value function
	int Width = 0;         /// Width of the bar.
	int Height = 0;        /// Height of the bar.
	std::optional<IntColor> borderColor = 1; /// additional border color.
	std::vector<unsigned int> colors; /// array of color to show (depend of value)
	std::vector<unsigned int> values; /// list of percentage to change color.
};

/**
**  Show bar.
*/
class CContentTypeCompleteBar : public CContentType
{
public:
	CContentTypeCompleteBar() = default;

	void Draw(const CUnit &unit, CFont *defaultfont) const override;
	void Parse(lua_State *l) override;

private:
	int varIndex = 1;    /// Index of the variable to show, -1 if not.
	int width = 0;       /// Width of the bar.
	int height = 0;      /// Height of the bar.
	bool hasBorder = false; /// True for additional border.
	int colorIndex = -1;  /// Index of Color to show.
};

//@}

#endif // __CONTENT_TYPE_H__
