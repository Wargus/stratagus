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
/**@name script.h - The clone configuration language headerfile. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
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

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <memory>
#include <string>
#ifdef __cplusplus
extern "C" {
#endif
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#ifdef __cplusplus
}
#endif

#include "filesystem.h"
#include "stratagus.h"
#include "vec2i.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CUnitType;
class CFile;
class CFont;

struct LuaUserData {
	int Type;
	void *Data;
};

enum {
	LuaUnitType = 100,
	LuaSoundType
};

/**
**	original lua_isstring() returns true for either a string or a number
**	this do strict checking for strings only
**/
extern int lua_isstring_strict(lua_State *luaStack, int idx);

extern lua_State *Lua;

extern int LuaLoadFile(const fs::path &file, const std::string &strArg = "", bool exitOnError = true);
extern int LuaCall(int narg, int clear, bool exitOnError = true);
extern int LuaCall(lua_State *L, int narg, int nresults, int base, bool exitOnError = true);

#define LuaError(l, format, ...) \
 do { \
  PrintFunction(); \
  fprintf(stdout, format, ##__VA_ARGS__); \
  fprintf(stdout, "\n"); \
  lua_pushfstring(l, format, ##__VA_ARGS__); \
  lua_error(l); \
 } while (0)

static void LuaCheckArgs(lua_State *l, const int args)
{
	if (lua_gettop(l) != args) {
		LuaError(l, "incorrect argument");
	}
}

static void LuaCheckArgs_min(lua_State *l, const int args)
{
	if (lua_gettop(l) < args) {
		LuaError(l, "incorrect argument");
	}
}

static const int LuaGetArgsNum(lua_State *l)
{
	return lua_gettop(l);
}

#if LUA_VERSION_NUM <= 501

inline size_t lua_rawlen(lua_State *l, int index)
{
	return luaL_getn(l, index);
}

#endif

/// All possible value for a number.
enum class EBinOp {
	Add,         /// a + b.
	Sub,         /// a - b.
	Mul,         /// a * b.
	Div,         /// a / b.
	Min,         /// Min(a, b).
	Max,         /// Max(a, b).
	Gt,          /// a  > b.
	GtEq,        /// a >= b.
	Lt,          /// a  < b.
	LtEq,        /// a <= b.
	Eq,          /// a == b.
	NEq,         /// a <> b.
};

/**
**  Enumeration to know which variable to be selected.
*/
enum class EnumVariable {
	Value = 0,  /// Value of the variable.
	Max,        /// Max of the variable.
	Increase,   /// Increase value of the variable.
	Diff,       /// (Max - Value)
	Percent,    /// (100 * Value / Max)
	Name        /// Name of the variable.
};

/**
**  Number description.
**  Use to describe complex number in script to use when game running.
** [Deprecated]: give access to lua.
*/
struct INumberDesc
{
	virtual ~INumberDesc() = default;
	virtual int eval() const = 0;
};

/**
** Unit description
**  Use to describe complex unit in script to use when game running.
*/
struct IUnitDesc
{
	virtual ~IUnitDesc() = default;
	virtual CUnit* eval() const = 0;
};

/**
** String description
**  Use to describe complex string in script to use when game running.
*/
struct IStringDesc
{
	virtual ~IStringDesc() = default;
	virtual std::string eval() const = 0;
};

class NumberDescLuaFunction : public INumberDesc
{
public:
	explicit NumberDescLuaFunction(int index) : index(index) {}
	int eval() const override;

private:
	int index;
};

class NumberDescInt : public INumberDesc
{
public:
	explicit NumberDescInt(int n) : n(n) {}
	int eval() const override;

private:
	int n;
};

/// for Bin operand  a ?? b
class NumberDescBinOp : public INumberDesc
{
public:
	NumberDescBinOp(EBinOp type,
	                std::unique_ptr<INumberDesc> left,
	                std::unique_ptr<INumberDesc> right) :
		type(type),
		left(std::move(left)),
		right(std::move(right))
	{}
	int eval() const override;

private:
	EBinOp type;
	std::unique_ptr<INumberDesc> left;
	std::unique_ptr<INumberDesc> right;
};

class NumberDescRand : public INumberDesc
{
public:
	explicit NumberDescRand(std::unique_ptr<INumberDesc> n) : n(std::move(n)) {}
	int eval() const override;

private:
	std::unique_ptr<INumberDesc> n;
};


class NumberDescUnitStat : public INumberDesc
{
public:
	NumberDescUnitStat(std::unique_ptr<IUnitDesc> unitDesc,
	                   int varIndex,
	                   EnumVariable component,
	                   int loc) :
		unitDesc(std::move(unitDesc)),
		varIndex(varIndex),
		component(component),
		loc(loc)
	{}
	int eval() const override;

private:
	std::unique_ptr<IUnitDesc> unitDesc; /// Which unit.
	int varIndex;                 /// Which index variable.
	EnumVariable component;    /// Which component.
	int loc;                   /// Location of Variables[].
};

class NumberDescTypeStat : public INumberDesc
{
public:
	NumberDescTypeStat(CUnitType **type,
	                   int varIndex,
	                   EnumVariable component,
	                   int loc) :
		type(type),
		varIndex(varIndex),
		component(component),
		loc(loc)
	{}
	int eval() const override;

private:
	CUnitType **type; /// Which unitType.
	int varIndex;                 /// Which index variable.
	EnumVariable component;    /// Which component.
	int loc;                   /// Location of Variables[].
};

class NumberDescVideoTextLength : public INumberDesc
{
public:
	NumberDescVideoTextLength(std::unique_ptr<IStringDesc> string, CFont *font) :
		string(std::move(string)),
		font(font)
	{}
	int eval() const override;

private:
	std::unique_ptr<IStringDesc> string;
	CFont *font;
};

class NumberDescStringFind : public INumberDesc
{
public:
	NumberDescStringFind(std::unique_ptr<IStringDesc> string, char c) :
		string(std::move(string)),
		c(c)
	{}
	int eval() const override;

private:
	std::unique_ptr<IStringDesc> string;
	char c;
};

class NumberDescIf : public INumberDesc
{
public:
	NumberDescIf(std::unique_ptr<INumberDesc> cond,
	             std::unique_ptr<INumberDesc> trueValue,
	             std::unique_ptr<INumberDesc> falseValue) :
		cond(std::move(cond)),
		trueValue(std::move(trueValue)),
		falseValue(std::move(falseValue))
	{}
	int eval() const override;

private:
	std::unique_ptr<INumberDesc> cond;   /// Branch condition.
	std::unique_ptr<INumberDesc> trueValue;  /// Number if Cond is true.
	std::unique_ptr<INumberDesc> falseValue; /// Number if Cond is false.
};

class NumberDescPlayerData : public INumberDesc
{
public:
	NumberDescPlayerData(std::unique_ptr<INumberDesc> playerIndex,
	                     std::unique_ptr<IStringDesc> dataType,
	                     std::unique_ptr<IStringDesc> resType) :
		playerIndex(std::move(playerIndex)),
		dataType(std::move(dataType)),
		resType(std::move(resType))
	{}
	int eval() const override;

private:
	std::unique_ptr<INumberDesc> playerIndex;   /// Number of player
	std::unique_ptr<IStringDesc> dataType; /// Player's data
	std::unique_ptr<IStringDesc> resType;  /// Resource type
};

/**
**  Unit description.
*/
class UnitDescRef : public IUnitDesc {
public:
	explicit UnitDescRef(CUnit **AUnit) : AUnit(AUnit) {}

	CUnit *eval() const override { return *AUnit; }

private:
	CUnit **AUnit; /// Address of the unit.
};

class StringDescLuaFunction : public IStringDesc
{
public:
	explicit StringDescLuaFunction(int index) : index(index) {}

	std::string eval() const override;

private:
	int index; /// index of the lua function.
};

class StringDescString : public IStringDesc
{
public:
	explicit StringDescString(std::string s) : s(std::move(s)) {}
	std::string eval() const override;

private:
	std::string s;
};

class StringDescInverseVideo : public IStringDesc
{
public:
	explicit StringDescInverseVideo(std::unique_ptr<IStringDesc> string) : string(std::move(string))
	{}
	std::string eval() const override;

private:
	std::unique_ptr<IStringDesc> string;
};

class StringDescConcat : public IStringDesc
{
public:
	explicit StringDescConcat(std::vector<std::unique_ptr<IStringDesc>> strings) :
		strings(std::move(strings))
	{}
	std::string eval() const override;

private:
	std::vector<std::unique_ptr<IStringDesc>> strings;
};

class StringDescNumber : public IStringDesc
{
public:
	explicit StringDescNumber(std::unique_ptr<INumberDesc> number) : number(std::move(number)) {}
	std::string eval() const override;

private:
	std::unique_ptr<INumberDesc> number;
};

class StringDescUnit : public IStringDesc
{
public:
	explicit StringDescUnit(std::unique_ptr<IUnitDesc> unitDesc) : unitDesc(std::move(unitDesc)) {}
	std::string eval() const override;

private:
	std::unique_ptr<IUnitDesc> unitDesc;
};

class StringDescIf : public IStringDesc
{
public:
	StringDescIf(std::unique_ptr<INumberDesc> cond,
	             std::unique_ptr<IStringDesc> trueValue,
	             std::unique_ptr<IStringDesc> falseValue) :
		cond(std::move(cond)),
		trueValue(std::move(trueValue)),
		falseValue(std::move(falseValue))
	{}
	std::string eval() const override;

private:
	std::unique_ptr<INumberDesc> cond; /// Branch condition.
	std::unique_ptr<IStringDesc> trueValue; /// String if Cond is true.
	std::unique_ptr<IStringDesc> falseValue; /// String if Cond is false.
};

class StringDescSubString : public IStringDesc
{
public:
	StringDescSubString(std::unique_ptr<IStringDesc> string,
	                    std::unique_ptr<INumberDesc> begin,
	                    std::unique_ptr<INumberDesc> end) :
		string(std::move(string)),
		begin(std::move(begin)),
		end(std::move(end))
	{}
	std::string eval() const override;

private:
	std::unique_ptr<IStringDesc> string; /// Original string.
	std::unique_ptr<INumberDesc> begin; /// Begin of result string.
	std::unique_ptr<INumberDesc> end; /// End of result string.
};

class StringDescLine : public IStringDesc
{
public:
	StringDescLine(std::unique_ptr<IStringDesc> string,
	               std::unique_ptr<INumberDesc> line,
	               std::unique_ptr<INumberDesc> maxLen,
	               CFont *font) :
		string(std::move(string)),
		line(std::move(line)),
		maxLen(std::move(maxLen)),
		font(font)
	{}
	std::string eval() const override;

private:
	std::unique_ptr<IStringDesc> string; /// Original string.
	std::unique_ptr<INumberDesc> line; /// Line number.
	std::unique_ptr<INumberDesc> maxLen; /// Max length of line.
	CFont *font;         /// Font to consider (else (-1) consider just char).
};

class StringDescPlayerName : public IStringDesc
{
public:
	explicit StringDescPlayerName(std::unique_ptr<INumberDesc> playerIndex) :
		playerIndex(std::move(playerIndex))
	{}
	std::string eval() const override;

private:
	std::unique_ptr<INumberDesc> playerIndex;
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern bool CclInConfigFile;        /// True while config file parsing

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern std::string_view LuaToString(lua_State *l, int narg);
extern int LuaToNumber(lua_State *l, int narg);
extern float LuaToFloat(lua_State *l, int narg);
extern unsigned int LuaToUnsignedNumber(lua_State *l, int narg);
extern bool LuaToBoolean(lua_State *l, int narg);

extern std::string_view LuaToString(lua_State *l, int index, int subIndex);
extern int LuaToNumber(lua_State *l, int index, int subIndex);
extern unsigned int LuaToUnsignedNumber(lua_State *l, int index, int subIndex);
extern bool LuaToBoolean(lua_State *l, int index, int subIndex);

extern void LuaGarbageCollect();  /// Perform garbage collection
extern void InitLua();                /// Initialise Lua
extern void LoadCcl(const fs::path &filename, const std::string &luaArgStr = "");  /// Load ccl config file
extern void SavePreferences();        /// Save user preferences
extern int CclCommand(const std::string &command, bool exitOnError = true);

extern void ScriptRegister();

extern std::string SaveGlobal(lua_State *l); /// For saving lua state

CUnit *CclGetUnitFromRef(lua_State *l);

/**
**  Get a position from lua state
**
**  @param l  Lua state.
**  @param x  pointer to output x position.
**  @param y  pointer to output y position.
*/
template <typename T>
void CclGetPos(lua_State *l, T *x , T *y, const int offset = -1)
{
	if (!lua_istable(l, offset) || lua_rawlen(l, offset) != 2) {
		LuaError(l, "incorrect argument");
	}
	*x = LuaToNumber(l, offset, 1);
	*y = LuaToNumber(l, offset, 2);
}

template <typename T>
void CclGetPos(lua_State *l, Vec2T<T> *pos, const int offset = -1)
{
	CclGetPos(l, &pos->x, &pos->y, offset);
}

extern std::unique_ptr<INumberDesc> Damage;  /// Damage calculation for missile.

/// transform string in corresponding index.
extern EnumVariable Str2EnumVariable(lua_State *l, std::string_view s);
extern std::unique_ptr<INumberDesc> CclParseNumberDesc(lua_State *l); /// Parse a number description.
extern std::unique_ptr<IUnitDesc> CclParseUnitDesc(lua_State *l);     /// Parse a unit description.
extern CUnitType **CclParseTypeDesc(lua_State *l);   /// Parse a unit type description.
std::unique_ptr<IStringDesc> CclParseStringDesc(lua_State *l);        /// Parse a string description.

extern int EvalNumber(const INumberDesc &numberdesc); /// Evaluate the number.
extern CUnit *EvalUnit(const IUnitDesc &unitdesc);    /// Evaluate the unit.
std::string EvalString(const IStringDesc &s);         /// Evaluate the string.

//@}

#endif // !__SCRIPT_H__
