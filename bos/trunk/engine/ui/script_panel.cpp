//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name script_panel.cpp - The panel configuration. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer, Jimmy Salmon and Joris Dauphin.
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"
#include "script.h"
#include "iolib.h"
#include "trigger.h"
#include "unit.h"
#include "unittype.h"
#include "font.h"
#include "interface.h"
#include "ui.h"
#include "spells.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct NumberDesc;
struct UnitDesc;
struct StringDesc;
struct UStrInt;
enum EnumVariable;

static UStrInt GetComponent(const CUnit *unit, int index, EnumVariable e, int t);

static EnumVariable Str2EnumVariable(lua_State *l, const char *s);
static NumberDesc *CclParseNumberDesc(lua_State *l);
static UnitDesc *CclParseUnitDesc(lua_State *l);
static StringDesc *CclParseStringDesc(lua_State *l);

static int EvalNumber(const NumberDesc *numberdesc);
static CUnit *EvalUnit(const UnitDesc *unitdesc);
static char *EvalString(const StringDesc *s);

static void FreeNumberDesc(NumberDesc *number);
static void FreeUnitDesc(UnitDesc *unitdesc);
static void FreeStringDesc(StringDesc *s);

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static int NumberCounter = 0; /// Counter for lua function.
static int StringCounter = 0; /// Counter for lua function.

/// Useful for getComponent.
enum UStrIntType {
	USTRINT_STR, USTRINT_INT
};
struct UStrInt {
	union {const char *s; int i;};
	UStrIntType type;
};

/**
**  All possible value for a number.
*/
enum ENumber
{
	ENumber_Lua,         /// a lua function.
	ENumber_Dir,         /// directly a number.
	ENumber_Add,         /// a + b.
	ENumber_Sub,         /// a - b.
	ENumber_Mul,         /// a * b.
	ENumber_Div,         /// a / b.
	ENumber_Min,         /// Min(a, b).
	ENumber_Max,         /// Max(a, b).
	ENumber_Rand,        /// Rand(a) : number in [0..a-1].

	ENumber_Gt,          /// a  > b.
	ENumber_GtEq,        /// a >= b.
	ENumber_Lt,          /// a  < b.
	ENumber_LtEq,        /// a <= b.
	ENumber_Eq,          /// a == b.
	ENumber_NEq,         /// a <> b.

	ENumber_VideoTextLength, /// VideoTextLength(font, string).
	ENumber_StringFind,      /// strchr(string, char) - s.


	ENumber_UnitStat     /// Property of Unit.
// FIXME: add others.
};

/**
**  All possible value for a unit.
*/
enum EUnit
{
	EUnit_Ref           /// Unit direct reference.
// FIXME: add others.
};

/**
**  All possible value for a string.
*/
enum EString
{
	EString_Lua,          /// a lua function.
	EString_Dir,          /// directly a string.
	EString_Concat,       /// a + b [+ c ...].
	EString_String,       /// Convert number in string.
	EString_InverseVideo, /// Inverse video for the string ("a" -> "~<a~>").
	EString_If,           /// If cond then String1 else String2.
	EString_UnitName,     /// UnitType Name.
	EString_SubString,    /// SubString.
	EString_Line          /// line n of the string.
// FIXME: add others.
};

/**
**  Enumeration to know which variable to be selected.
*/
enum EnumVariable
{
	VariableValue = 0,  /// Value of the variable.
	VariableMax,        /// Max of the variable.
	VariableIncrease,   /// Increase value of the variable.
	VariableDiff,       /// (Max - Value)
	VariablePercent,    /// (100 * Value / Max)
	VariableName        /// Name of the variable.
};

/**
**  Enumeration of unit
*/
enum EnumUnit
{
	UnitRefItSelf = 0,      /// unit.
	UnitRefInside,          /// unit->Inside.
	UnitRefContainer,       /// Unit->Container.
	UnitRefWorker,          /// unit->Data.Built.Worker
	UnitRefGoal,            /// unit->Goal
};

/**
**  For Bin operand  a ?? b
*/
struct BinaryOp
{
	NumberDesc *Left;           /// Left operand.
	NumberDesc *Right;          /// Right operand.
};

/**
**  Number description.
*/
struct NumberDesc
{
	ENumber e;       /// which number.
	union {
		unsigned int Index; /// index of the lua function.
		int Val;       /// Direct value.
		NumberDesc *N; /// Other number.
		BinaryOp BinOp;   /// For binary operand.
		struct {
			UnitDesc *Unit;            /// Which unit.
			int Index;                 /// Which index variable.
			EnumVariable Component;    /// Which component.
			int Loc;                   /// Location of Variables[].
		} UnitStat;
		struct {
			StringDesc *String; /// String.
			CFont *Font;        /// Font.
		} VideoTextLength;
		struct {
			StringDesc *String; /// String.
			char C;             /// Char.
		} StringFind;

	} D;
};

/**
**  Unit description.
*/
struct UnitDesc
{
	EUnit e;       /// which unit;
	union {
		CUnit **AUnit; /// Adress of the unit.
	} D;
};

/**
**  String description.
*/
struct StringDesc {
	EString e;       /// which number.
	union {
		unsigned int Index; /// index of the lua function.
		char *Val;       /// Direct value.
		struct {
			StringDesc **Strings;  /// Array of operands.
			int n;                 /// number of operand to concat
		} Concat; /// for Concat two string.
		NumberDesc *Number;  /// Number.
		StringDesc *String;  /// String.
		UnitDesc *Unit;      /// Unit desciption.
		struct {
			NumberDesc *Cond;  /// Branch condition.
			StringDesc *True;  /// String if Cond is true.
			StringDesc *False; /// String if Cond is false.
		} If; /// conditional string.
		struct {
			StringDesc *String;  /// Original string.
			NumberDesc *Begin;   /// Begin of result string.
			NumberDesc *End;     /// End of result string.
		} SubString; /// For extract a substring
		struct {
			StringDesc *String;  /// Original string.
			NumberDesc *Line;    /// Line number.
			NumberDesc *MaxLen;  /// Max lenght of line.
			CFont *Font;         /// Font to consider (else (-1) consider just char).
		} Line; /// For specific line.
	} D;
};

/**
**  Show simple text followed by variable value.
*/
class CContentTypeText : public CContentType {
public:
	CContentTypeText() : Text(NULL), Font(NULL), Centered(0), Index(-1),
		Component(VariableValue), ShowName(0), Stat(0) {}
	virtual ~CContentTypeText() {
		FreeStringDesc(Text);
		delete Text;
	}

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
		Index(-1), Component(VariableValue) {}
	virtual ~CContentTypeFormattedText() { delete[] this->Format; }

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
		Index1(-1), Component1(VariableValue), Index2(-1), Component2(VariableValue) {}
	virtual ~CContentTypeFormattedText2() { delete[] Format; }

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
	CContentTypeLifeBar() : Index(-1), Width(0), Height(0) {}

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
	CContentTypeCompleteBar() : Index(-1), Width(0), Height(0), Border(0) {}

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

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Return enum from string about variable component.
**
**  @param l Lua State.
**  @param s string to convert.
**
**  @return  Corresponding value.
**  @note    Stop on error.
*/
static EnumVariable Str2EnumVariable(lua_State *l, const char *s)
{
	static struct {
		const char *s;
		EnumVariable e;
	} list[] = {
		{"Value", VariableValue},
		{"Max", VariableMax},
		{"Increase", VariableIncrease},
		{"Diff", VariableDiff},
		{"Percent", VariablePercent},
		{"Name", VariableName},
		{0, VariableValue}}; // List of possible values.
	int i; // Iterator.

	for (i = 0; list[i].s; i++) {
		if (!strcmp(s, list[i].s)) {
			return list[i].e;
		}
	}
	LuaError(l, "'%s' is a invalid variable component" _C_ s);
	return VariableValue;
}

/**
**  Return enum from string about variable component.
**
**  @param l Lua State.
**  @param s string to convert.
**
**  @return  Corresponding value.
**  @note    Stop on error.
*/
static EnumUnit Str2EnumUnit(lua_State *l, const char *s)
{
	static struct {
		const char *s;
		EnumUnit e;
	} list[] = {
		{"ItSelf", UnitRefItSelf},
		{"Inside", UnitRefInside},
		{"Container", UnitRefContainer},
		{"Worker", UnitRefWorker},
		{"Goal", UnitRefGoal},
		{0, UnitRefItSelf}}; // List of possible values.
	int i; // Iterator.

	for (i = 0; list[i].s; i++) {
		if (!strcmp(s, list[i].s)) {
			return list[i].e;
		}
	}
	LuaError(l, "'%s' is a invalid Unit reference" _C_ s);
	return UnitRefItSelf;
}

/**
**  Parse the condition Panel.
**
**  @param l   Lua State.
*/
static ConditionPanel *ParseConditionPanel(lua_State *l)
{
	ConditionPanel *condition; // Condition parsed
	const char *key;           // key of lua table.
	int i;                     // iterator for flags and variable.

	Assert(lua_istable(l, -1));

	condition = new ConditionPanel;
	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		key = LuaToString(l, -2);
		if (!strcmp(key, "ShowOnlySelected")) {
			condition->ShowOnlySelected = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "HideNeutral")) {
				condition->HideNeutral = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "HideAllied")) {
				condition->HideAllied = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "ShowOpponent")) {
				condition->ShowOpponent = LuaToBoolean(l, -1);
		} else {
			for (i = 0; i < UnitTypeVar.NumberBoolFlag; ++i) {
				if (!strcmp(key, UnitTypeVar.BoolFlagName[i])) {
					if (!condition->BoolFlags) {
						condition->BoolFlags = new char[UnitTypeVar.NumberBoolFlag];
						memset(condition->BoolFlags, 0, UnitTypeVar.NumberBoolFlag * sizeof(char));
					}
					condition->BoolFlags[i] = Ccl2Condition(l, LuaToString(l, -1));
					break;
				}
			}
			if (i != UnitTypeVar.NumberBoolFlag) { // key is a flag
				continue;
			}
			i = GetVariableIndex(key);
			if (i != -1) {
				if (!condition->Variables) {
					condition->Variables = new char[UnitTypeVar.NumberVariable];
					memset(condition->Variables, 0, UnitTypeVar.NumberVariable * sizeof(char));
				}
				condition->Variables[i] = Ccl2Condition(l, LuaToString(l, -1));
				continue;
			}
			LuaError(l, "'%s' invalid for Condition in DefinePanels" _C_ key);
		}
	}
	return condition;
}

/**
**  CclParseContent
*/
static CContentType *CclParseContent(lua_State *l)
{
	CContentType *content;
	const char *key;
	int posX = 0;
	int posY = 0;
	ConditionPanel *condition;

	Assert(lua_istable(l, -1));
	content = NULL;
	condition = NULL;
	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		key = LuaToString(l, -2);
		if (!strcmp(key, "Pos")) {
			Assert(lua_istable(l, -1));
			lua_rawgeti(l, -1, 1); // X
			lua_rawgeti(l, -2, 2); // Y
			posX = LuaToNumber(l, -2);
			posY = LuaToNumber(l, -1);
			lua_pop(l, 2); // Pop X and Y
		} else if (!strcmp(key, "More")) {
			Assert(lua_istable(l, -1));
			lua_rawgeti(l, -1, 1); // Method name
			lua_rawgeti(l, -2, 2); // Method data
			key = LuaToString(l, -2);
			if (!strcmp(key, "Text")) {
				CContentTypeText *contenttext = new CContentTypeText;

				Assert(lua_istable(l, -1) || lua_isstring(l, -1));
				if (lua_isstring(l, -1)) {
					contenttext->Text = CclParseStringDesc(l);
					lua_pushnil(l); // ParseStringDesc eat token
				} else {
					for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
						key = LuaToString(l, -2);
						if (!strcmp(key, "Text")) {
							contenttext->Text = CclParseStringDesc(l);
							lua_pushnil(l); // ParseStringDesc eat token
						} else if (!strcmp(key, "Font")) {
							contenttext->Font = CFont::Get(LuaToString(l, -1));
						} else if (!strcmp(key, "Centered")) {
							contenttext->Centered = LuaToBoolean(l, -1);
						} else if (!strcmp(key, "Variable")) {
							contenttext->Index = GetVariableIndex(LuaToString(l, -1));
							if (contenttext->Index == -1) {
								LuaError(l, "unknown variable '%s'" _C_ LuaToString(l, -1));
							}
						} else if (!strcmp(key, "Component")) {
							contenttext->Component = Str2EnumVariable(l, LuaToString(l, -1));
						} else if (!strcmp(key, "Stat")) {
							contenttext->Stat = LuaToBoolean(l, -1);
						} else if (!strcmp(key, "ShowName")) {
							contenttext->ShowName = LuaToBoolean(l, -1);
						} else {
							LuaError(l, "'%s' invalid for method 'Text' in DefinePanels" _C_ key);
						}
					}
				}
				content = contenttext;
			} else if (!strcmp(key, "FormattedText")) {
				CContentTypeFormattedText *contentformattedtext = new CContentTypeFormattedText;

				Assert(lua_istable(l, -1));
				for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
					key = LuaToString(l, -2);
					if (!strcmp(key, "Format")) {
						contentformattedtext->Format = new_strdup(LuaToString(l, -1));
					} else if (!strcmp(key, "Font")) {
						contentformattedtext->Font = CFont::Get(LuaToString(l, -1));
					} else if (!strcmp(key, "Variable")) {
						contentformattedtext->Index = GetVariableIndex(LuaToString(l, -1));
						if (contentformattedtext->Index == -1) {
							LuaError(l, "unknown variable '%s'" _C_ LuaToString(l, -1));
						}
					} else if (!strcmp(key, "Component")) {
						contentformattedtext->Component = Str2EnumVariable(l, LuaToString(l, -1));
					} else if (!strcmp(key, "Centered")) {
						contentformattedtext->Centered = LuaToBoolean(l, -1);
					} else {
						LuaError(l, "'%s' invalid for method 'FormattedText' in DefinePanels" _C_ key);
					}
				}
				content = contentformattedtext;
			} else if (!strcmp(key, "FormattedText2")) {
				CContentTypeFormattedText2 *contentformattedtext2 = new CContentTypeFormattedText2;

				Assert(lua_istable(l, -1));
				for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
					key = LuaToString(l, -2);
						if (!strcmp(key, "Format")) {
							contentformattedtext2->Format = new_strdup(LuaToString(l, -1));
						} else if (!strcmp(key, "Font")) {
							contentformattedtext2->Font = CFont::Get(LuaToString(l, -1));
						} else if (!strcmp(key, "Variable")) {
						contentformattedtext2->Index1 = GetVariableIndex(LuaToString(l, -1));
						contentformattedtext2->Index2 = GetVariableIndex(LuaToString(l, -1));
						if (contentformattedtext2->Index1 == -1) {
							LuaError(l, "unknown variable '%s'" _C_ LuaToString(l, -1));
						}
					} else if (!strcmp(key, "Component")) {
						contentformattedtext2->Component1 = Str2EnumVariable(l, LuaToString(l, -1));
						contentformattedtext2->Component2 = Str2EnumVariable(l, LuaToString(l, -1));
					} else if (!strcmp(key, "Variable1")) {
						contentformattedtext2->Index1 = GetVariableIndex(LuaToString(l, -1));
						if (contentformattedtext2->Index1 == -1) {
							LuaError(l, "unknown variable '%s'" _C_ LuaToString(l, -1));
						}
					} else if (!strcmp(key, "Component1")) {
						contentformattedtext2->Component1 = Str2EnumVariable(l, LuaToString(l, -1));
					} else if (!strcmp(key, "Variable2")) {
						contentformattedtext2->Index2 = GetVariableIndex(LuaToString(l, -1));
						if (contentformattedtext2->Index2 == -1) {
							LuaError(l, "unknown variable '%s'" _C_ LuaToString(l, -1));
						}
					} else if (!strcmp(key, "Component2")) {
						contentformattedtext2->Component2 = Str2EnumVariable(l, LuaToString(l, -1));
					} else if (!strcmp(key, "Centered")) {
						contentformattedtext2->Centered = LuaToBoolean(l, -1);
					} else {
						LuaError(l, "'%s' invalid for method 'FormattedText2' in DefinePanels" _C_ key);
					}
				}
				content = contentformattedtext2;
			} else if (!strcmp(key, "Icon")) {
				CContentTypeIcon *contenticon = new CContentTypeIcon;

				for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
					key = LuaToString(l, -2);
					if (!strcmp(key, "Unit")) {
						contenticon->UnitRef = Str2EnumUnit(l, LuaToString(l, -1));
					} else {
						LuaError(l, "'%s' invalid for method 'Icon' in DefinePanels" _C_ key);
					}
				}
				content = contenticon;
			} else if (!strcmp(key, "LifeBar")) {
				CContentTypeLifeBar *contentlifebar = new CContentTypeLifeBar;

				for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
					key = LuaToString(l, -2);
					if (!strcmp(key, "Variable")) {
						contentlifebar->Index = GetVariableIndex(LuaToString(l, -1));
						if (contentlifebar->Index == -1) {
							LuaError(l, "unknown variable '%s'" _C_ LuaToString(l, -1));
						}
					} else if (!strcmp(key, "Height")) {
						contentlifebar->Height = LuaToNumber(l, -1);
					} else if (!strcmp(key, "Width")) {
						contentlifebar->Width = LuaToNumber(l, -1);
					} else {
						LuaError(l, "'%s' invalid for method 'LifeBar' in DefinePanels" _C_ key);
					}
				}
				// Default value and checking errors.
				if (contentlifebar->Height <= 0) {
					contentlifebar->Height = 5; // Default value.
				}
				if (contentlifebar->Width <= 0) {
					contentlifebar->Width = 50; // Default value.
				}
				if (contentlifebar->Index == -1) {
					LuaError(l, "variable undefined for LifeBar");
				}
				content = contentlifebar;
			} else if (!strcmp(key, "CompleteBar")) {
				CContentTypeCompleteBar *contenttypecompletebar = new CContentTypeCompleteBar;

				for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
					key = LuaToString(l, -2);
					if (!strcmp(key, "Variable")) {
						contenttypecompletebar->Index = GetVariableIndex(LuaToString(l, -1));
						if (contenttypecompletebar->Index == -1) {
							LuaError(l, "unknown variable '%s'" _C_ LuaToString(l, -1));
						}
					} else if (!strcmp(key, "Height")) {
						contenttypecompletebar->Height = LuaToNumber(l, -1);
					} else if (!strcmp(key, "Width")) {
						contenttypecompletebar->Width = LuaToNumber(l, -1);
					} else if (!strcmp(key, "Border")) {
						contenttypecompletebar->Border = LuaToBoolean(l, -1);
					} else {
						LuaError(l, "'%s' invalid for method 'CompleteBar' in DefinePanels" _C_ key);
					}
				}
				// Default value and checking errors.
				if (contenttypecompletebar->Height <= 0) {
					contenttypecompletebar->Height = 5; // Default value.
				}
				if (contenttypecompletebar->Width <= 0) {
					contenttypecompletebar->Width = 50; // Default value.
				}
				if (contenttypecompletebar->Index == -1) {
					LuaError(l, "variable undefined for CompleteBar");
				}
				content = contenttypecompletebar;
			} else {
				LuaError(l, "Invalid drawing method '%s' in DefinePanels" _C_ key);
			}
			lua_pop(l, 2); // Pop Variable Name and Method
		} else if (!strcmp(key, "Condition")) {
			condition = ParseConditionPanel(l);
		} else {
			LuaError(l, "'%s' invalid for Contents in DefinePanels" _C_ key);
		}
	}
	content->PosX = posX;
	content->PosY = posY;
	content->Condition = condition;
	return content;
}


/**
**  Define the Panels.
**  Define what is shown in the panel(text, icon, variables)
**
**  @param l  Lua state.
**  @return   0.
*/
static int CclDefinePanelContents(lua_State *l)
{
	int i;                  // iterator for arguments.
	int j;                  // iterator for contents and panels.
	int nargs;              // number of arguments.
	const char *key;        // key of lua table.
	CUnitInfoPanel *infopanel;   // variable for transit.

	nargs = lua_gettop(l);
	for (i = 0; i < nargs; i++) {
		Assert(lua_istable(l, i + 1));
		infopanel = new CUnitInfoPanel;
		for (lua_pushnil(l); lua_next(l, i + 1); lua_pop(l, 1)) {
			key = LuaToString(l, -2);
			if (!strcmp(key, "Ident")) {
				infopanel->Name = LuaToString(l, -1);
			} else if (!strcmp(key, "Pos")) {
				Assert(lua_istable(l, -1));
				lua_rawgeti(l, -1, 1); // X
				lua_rawgeti(l, -2, 2); // Y
				infopanel->PosX = LuaToNumber(l, -2);
				infopanel->PosY = LuaToNumber(l, -1);
				lua_pop(l, 2); // Pop X and Y
			} else if (!strcmp(key, "DefaultFont")) {
				infopanel->DefaultFont = CFont::Get(LuaToString(l, -1));
			} else if (!strcmp(key, "Condition")) {
				infopanel->Condition = ParseConditionPanel(l);
			} else if (!strcmp(key, "Contents")) {
				Assert(lua_istable(l, -1));
				for (j = 0; j < luaL_getn(l, -1); j++, lua_pop(l, 1)) {
					lua_rawgeti(l, -1, j + 1);
					infopanel->Contents.push_back(CclParseContent(l));
				}
			} else {
				LuaError(l, "'%s' invalid for DefinePanels" _C_ key);
			}
		}
		for (std::vector<CContentType *>::iterator content = infopanel->Contents.begin();
				content != infopanel->Contents.end(); ++content) { // Default value for invalid value.
			(*content)->PosX += infopanel->PosX;
			(*content)->PosY += infopanel->PosY;
		}
		for (j = 0; j < (int)UI.InfoPanelContents.size(); ++j) {
			if (infopanel->Name == UI.InfoPanelContents[j]->Name) {
				DebugPrint("Redefinition of Panel '%s'\n" _C_ infopanel->Name.c_str());
				delete UI.InfoPanelContents[j];
				UI.InfoPanelContents[j] = infopanel;
				break;
			}
		}
		if (j == (int)UI.InfoPanelContents.size()) {
			UI.InfoPanelContents.push_back(infopanel);
		}
	}
	return 0;
}

/**
**  Parse binary operation with number.
**
**  @param l       lua state.
**  @param binop   Where to stock info (must be malloced)
*/
static void ParseBinOp(lua_State *l, BinaryOp *binop)
{
	Assert(l);
	Assert(binop);
	Assert(lua_istable(l, -1));
	Assert(luaL_getn(l, -1) == 2);

	lua_rawgeti(l, -1, 1); // left
	binop->Left = CclParseNumberDesc(l);
	lua_rawgeti(l, -1, 2); // right
	binop->Right = CclParseNumberDesc(l);
	lua_pop(l, 1); // table.
}

/**
**  Convert the string to the corresponding data (which is a unit).
**
**  @param l   lua state.
**  @param s   Ident.
**
**  @return    The reference of the unit.
**
**  @todo better check for error (restrict param).
*/
static CUnit **Str2UnitRef(lua_State *l, const char *s)
{
	CUnit **res; // Result.

	Assert(l);
	Assert(s);
	res = NULL;
	if (!strcmp(s, "Attacker")) {
		res = &TriggerData.Attacker;
	} else if (!strcmp(s, "Defender")) {
		res = &TriggerData.Defender;
	} else if (!strcmp(s, "Active")) {
		res = &TriggerData.Active;
	} else {
		LuaError(l, "Invalid unit reference '%s'\n" _C_ s);
	}
	Assert(res); // Must check for error.
	return res;
}

/**
**  Return unit referernce definition.
**
**  @param l  lua state.
**
**  @return   unit referernce definition.
*/
static UnitDesc *CclParseUnitDesc(lua_State *l)
{
	UnitDesc *res;  // Result

	res = new UnitDesc;
	if (lua_isstring(l, -1)) {
		res->e = EUnit_Ref;
		res->D.AUnit = Str2UnitRef(l, LuaToString(l, -1));
	} else {
		LuaError(l, "Parse Error in ParseUnit\n");
	}
	lua_pop(l, 1);
	return res;
}


/**
**  Add a Lua handler
**
**  @param l          lua state.
**  @param tablename  name of the lua table.
**  @param counter    Counter for the handler
**
**  @return handle of the function.
*/
static int ParseLuaFunction(lua_State *l, const char *tablename, int *counter)
{
	lua_pushstring(l, tablename);
	lua_gettable(l, LUA_GLOBALSINDEX);
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_pushstring(l, tablename);
		lua_newtable(l);
		lua_settable(l, LUA_GLOBALSINDEX);
		lua_pushstring(l, tablename);
		lua_gettable(l, LUA_GLOBALSINDEX);
	}
	lua_pushvalue(l, -2);
	lua_rawseti(l, -2, *counter);
	lua_pop(l, 1);
	return (*counter)++;
}

/**
**  Call a Lua handler
**
**  @param handler  handler of the lua function to call.
**
**  @return  lua function result.
*/
static int CallLuaNumberFunction(unsigned int handler)
{
	int narg;
	int res;

	narg = lua_gettop(Lua);
	lua_pushstring(Lua, "_numberfunction_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	lua_rawgeti(Lua, -1, handler);
	LuaCall(0, 0);
	if (lua_gettop(Lua) - narg != 2) {
		LuaError(Lua, "Function must return one value.");
	}
	res = LuaToNumber(Lua, -1);
	lua_pop(Lua, 2);
	return res;
}

/**
**  Call a Lua handler
**
**  @param handler  handler of the lua function to call.
**
**  @return         lua function result.
*/
static char *CallLuaStringFunction(unsigned int handler)
{
	int narg;
	char *res;

	narg = lua_gettop(Lua);
	lua_pushstring(Lua, "_stringfunction_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	lua_rawgeti(Lua, -1, handler);
	LuaCall(0, 0);
	if (lua_gettop(Lua) - narg != 2) {
		LuaError(Lua, "Function must return one value.");
	}
	res = new_strdup(LuaToString(Lua, -1));
	lua_pop(Lua, 2);
	return res;
}

/**
**  Return number.
**
**  @param l  lua state.
**
**  @return   number.
*/
static NumberDesc *CclParseNumberDesc(lua_State *l)
{
	NumberDesc *res;
	int nargs;
	const char *key;

	res = new NumberDesc;
	if (lua_isnumber(l, -1)) {
		res->e = ENumber_Dir;
		res->D.Val = LuaToNumber(l, -1);
	} else if (lua_isfunction(l, -1)) {
		res->e = ENumber_Lua;
		res->D.Index = ParseLuaFunction(l, "_numberfunction_", &NumberCounter);
	} else if (lua_istable(l, -1)) {
		nargs = luaL_getn(l, -1);
		if (nargs != 2) {
			LuaError(l, "Bad number of args in parse Number table\n");
		}
		lua_rawgeti(l, -1, 1); // key
		key = LuaToString(l, -1);
		lua_pop(l, 1);
		lua_rawgeti(l, -1, 2); // table
		if (!strcmp(key, "Add")) {
			res->e = ENumber_Add;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Sub")) {
			res->e = ENumber_Sub;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Mul")) {
			res->e = ENumber_Mul;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Div")) {
			res->e = ENumber_Div;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Min")) {
			res->e = ENumber_Min;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Max")) {
			res->e = ENumber_Max;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Rand")) {
			res->e = ENumber_Rand;
			res->D.N = CclParseNumberDesc(l);
		} else if (!strcmp(key, "GreaterThan")) {
			res->e = ENumber_Gt;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "GreaterThanOrEq")) {
			res->e = ENumber_GtEq;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "LessThan")) {
			res->e = ENumber_Lt;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "LessThanOrEq")) {
			res->e = ENumber_LtEq;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "Equal")) {
			res->e = ENumber_Eq;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "NotEqual")) {
			res->e = ENumber_NEq;
			ParseBinOp(l, &res->D.BinOp);
		} else if (!strcmp(key, "UnitVar")) {
			Assert(lua_istable(l, -1));

			res->e = ENumber_UnitStat;
			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				key = LuaToString(l, -2);
				if (!strcmp(key, "Unit")) {
					res->D.UnitStat.Unit = CclParseUnitDesc(l);
					lua_pushnil(l);
				} else if (!strcmp(key, "Variable")) {
					res->D.UnitStat.Index = GetVariableIndex(LuaToString(l, -1));
					if (res->D.UnitStat.Index == -1) {
						LuaError(l, "Bad variable name :'%s'" _C_ LuaToString(l, -1));
					}
				} else if (!strcmp(key, "Component")) {
					res->D.UnitStat.Component = Str2EnumVariable(l, LuaToString(l, -1));
				} else if (!strcmp(key, "Loc")) {
					res->D.UnitStat.Loc = LuaToNumber(l, -1);
					if (res->D.UnitStat.Loc < 0 || 2 < res->D.UnitStat.Loc) {
						LuaError(l, "Bad Loc number :'%d'" _C_ (int) LuaToNumber(l, -1));
					}
				} else {
					LuaError(l, "Bad param %s for Unit" _C_ key);
				}
			}
			lua_pop(l, 1); // pop the table.
		} else if (!strcmp(key, "VideoTextLength")) {
			Assert(lua_istable(l, -1));
			res->e = ENumber_VideoTextLength;

			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
				key = LuaToString(l, -2);
				if (!strcmp(key, "Text")) {
					res->D.VideoTextLength.String = CclParseStringDesc(l);
					lua_pushnil(l);
				} else if (!strcmp(key, "Font")) {
					res->D.VideoTextLength.Font = CFont::Get(LuaToString(l, -1));
					if (!res->D.VideoTextLength.Font) {
						LuaError(l, "Bad Font name :'%s'" _C_ LuaToString(l, -1));
					}
				} else {
					LuaError(l, "Bad param %s for VideoTextLength" _C_ key);
				}
			}
			lua_pop(l, 1); // pop the table.
		} else if (!strcmp(key, "StringFind")) {
			Assert(lua_istable(l, -1));
			res->e = ENumber_StringFind;
			if (luaL_getn(l, -1) != 2) {
				LuaError(l, "Bad param for StringFind");
			}
			lua_rawgeti(l, -1, 1); // left
			res->D.StringFind.String = CclParseStringDesc(l);

			lua_rawgeti(l, -1, 2); // right
			res->D.StringFind.C = *LuaToString(l, -1);
			lua_pop(l, 1); // pop the char

			lua_pop(l, 1); // pop the table.
		} else {
			lua_pop(l, 1);
			LuaError(l, "unknow condition '%s'"_C_ key);
		}
	} else {
		LuaError(l, "Parse Error in ParseNumber");
	}
	lua_pop(l, 1);
	return res;
}

/**
**  Return String description.
**
**  @param l  lua state.
**
**  @return   String description.
*/
static StringDesc *CclParseStringDesc(lua_State *l)
{
	StringDesc *res;      // Result.
	int nargs;            // Size of table.
	const char *key;      // Key.

	res = new StringDesc;
	if (lua_isstring(l, -1)) {
		res->e = EString_Dir;
		res->D.Val = new_strdup(LuaToString(l, -1));
	} else if (lua_isfunction(l, -1)) {
		res->e = EString_Lua;
		res->D.Index = ParseLuaFunction(l, "_stringfunction_", &StringCounter);
	} else if (lua_istable(l, -1)) {
		nargs = luaL_getn(l, -1);
		if (nargs != 2) {
			LuaError(l, "Bad number of args in parse String table\n");
		}
		lua_rawgeti(l, -1, 1); // key
		key = LuaToString(l, -1);
		lua_pop(l, 1);
		lua_rawgeti(l, -1, 2); // table
		if (!strcmp(key, "Concat")){
			int i; // iterator.

			res->e = EString_Concat;
			res->D.Concat.n = luaL_getn(l, -1);
			if (res->D.Concat.n < 1) {
				LuaError(l, "Bad number of args in Concat\n");
			}
			res->D.Concat.Strings = new StringDesc *[res->D.Concat.n];
			for (i = 0; i < res->D.Concat.n; ++i) {
				lua_rawgeti(l, -1, 1 + i);
				res->D.Concat.Strings[i] = CclParseStringDesc(l);
			}
			lua_pop(l, 1); // table.
		} else if (!strcmp(key, "String")) {
			res->e = EString_String;
			res->D.Number = CclParseNumberDesc(l);
		} else if (!strcmp(key, "InverseVideo")) {
			res->e = EString_InverseVideo;
			res->D.String = CclParseStringDesc(l);
		} else if (!strcmp(key, "UnitName")) {
			res->e = EString_UnitName;
			res->D.Unit = CclParseUnitDesc(l);
		} else if (!strcmp(key, "If")) {
			res->e = EString_If;
			if (luaL_getn(l, -1) != 2 && luaL_getn(l, -1) != 3) {
				LuaError(l, "Bad number of args in If\n");
			}
			lua_rawgeti(l, -1, 1); // Condition.
			res->D.If.Cond = CclParseNumberDesc(l);
			lua_rawgeti(l, -1, 2); // Then.
			res->D.If.True = CclParseStringDesc(l);
			if (luaL_getn(l, -1) == 3) {
				lua_rawgeti(l, -1, 3); // Else.
				res->D.If.False = CclParseStringDesc(l);
			}
			lua_pop(l, 1); // table.
		} else if (!strcmp(key, "SubString")) {
			res->e = EString_SubString;
			if (luaL_getn(l, -1) != 2 && luaL_getn(l, -1) != 3) {
				LuaError(l, "Bad number of args in SubString\n");
			}
			lua_rawgeti(l, -1, 1); // String.
			res->D.SubString.String = CclParseStringDesc(l);
			lua_rawgeti(l, -1, 2); // Begin.
			res->D.SubString.Begin = CclParseNumberDesc(l);
			if (luaL_getn(l, -1) == 3) {
				lua_rawgeti(l, -1, 3); // End.
				res->D.SubString.End = CclParseNumberDesc(l);
			}
			lua_pop(l, 1); // table.
		} else if (!strcmp(key, "Line")) {
			res->e = EString_Line;
			if (luaL_getn(l, -1) < 2 || luaL_getn(l, -1) > 4) {
				LuaError(l, "Bad number of args in Line\n");
			}
			lua_rawgeti(l, -1, 1); // Line.
			res->D.Line.Line = CclParseNumberDesc(l);
			lua_rawgeti(l, -1, 2); // String.
			res->D.Line.String = CclParseStringDesc(l);
			if (luaL_getn(l, -1) >= 3) {
				lua_rawgeti(l, -1, 3); // Lenght.
				res->D.Line.MaxLen = CclParseNumberDesc(l);
			}
			res->D.Line.Font = NULL;
			if (luaL_getn(l, -1) >= 4) {
				lua_rawgeti(l, -1, 4); // Font.
				res->D.Line.Font = CFont::Get(LuaToString(l, -1));
				if (!res->D.Line.Font) {
					LuaError(l, "Bad Font name :'%s'" _C_ LuaToString(l, -1));
				}
				lua_pop(l, 1); // font name.
			}
			lua_pop(l, 1); // table.
		} else {
			lua_pop(l, 1);
			LuaError(l, "unknow condition '%s'"_C_ key);
		}
	} else {
		LuaError(l, "Parse Error in ParseString");
	}
	lua_pop(l, 1);
	return res;
}

/**
**  compute the Unit expression
**
**  @param unitdesc  struct with definition of the calculation.
**
**  @return          the result unit.
*/
static CUnit *EvalUnit(const UnitDesc *unitdesc)
{
	Assert(unitdesc);

	if (NumSelected > 0) {
		TriggerData.Active = Selected[0];
	} else {
		TriggerData.Active = UnitUnderCursor;
	}
	switch (unitdesc->e) {
		case EUnit_Ref :
			return *unitdesc->D.AUnit;
	}
	return NULL;
}

/**
**  compute the number expression
**
**  @param number  struct with definition of the calculation.
**
**  @return        the result number.
**
**  @todo Manage better the error (div/0, unit==NULL, ...).
*/
static int EvalNumber(const NumberDesc *number)
{
	CUnit *unit;
	char *s;
	char *s2;
	int a;
	int b;

	Assert(number);
	switch (number->e) {
		case ENumber_Lua :     // a lua function.
			return CallLuaNumberFunction(number->D.Index);
		case ENumber_Dir :     // directly a number.
			return number->D.Val;
		case ENumber_Add :     // a + b.
			return EvalNumber(number->D.BinOp.Left) + EvalNumber(number->D.BinOp.Right);
		case ENumber_Sub :     // a - b.
			return EvalNumber(number->D.BinOp.Left) - EvalNumber(number->D.BinOp.Right);
		case ENumber_Mul :     // a * b.
			return EvalNumber(number->D.BinOp.Left) * EvalNumber(number->D.BinOp.Right);
		case ENumber_Div :     // a / b.
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			if (!b) { // FIXME : manage better this.
				return 0;
			}
			return a / b;
		case ENumber_Min :     // a <= b ? a : b
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return std::min(a, b);
		case ENumber_Max :     // a >= b ? a : b
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return std::max(a, b);
		case ENumber_Gt  :     // a > b  ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a > b ? 1 : 0);
		case ENumber_GtEq :    // a >= b ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a >= b ? 1 : 0);
		case ENumber_Lt  :     // a < b  ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a < b ? 1 : 0);
		case ENumber_LtEq :    // a <= b ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a <= b ? 1 : 0);
		case ENumber_Eq  :     // a == b ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a == b ? 1 : 0);
		case ENumber_NEq  :    // a != b ? 1 : 0
			a = EvalNumber(number->D.BinOp.Left);
			b = EvalNumber(number->D.BinOp.Right);
			return (a != b ? 1 : 0);

		case ENumber_Rand :    // random(a) [0..a-1]
			a = EvalNumber(number->D.N);
			return SyncRand() % a;
		case ENumber_UnitStat : // property of unit.
			unit = EvalUnit(number->D.UnitStat.Unit);
			if (unit != NULL) {
				return GetComponent(unit, number->D.UnitStat.Index,
					number->D.UnitStat.Component, number->D.UnitStat.Loc).i;
			} else { // ERROR.
				return 0;
			}
		case ENumber_VideoTextLength : // VideoTextLength(font, s)
			if (number->D.VideoTextLength.String != NULL &&
					(s = EvalString(number->D.VideoTextLength.String)) != NULL) {
				a = number->D.VideoTextLength.Font->Width(s);
				delete s;
				return a;
			} else { // ERROR.
				return 0;
			}
		case ENumber_StringFind : // strchr(s, c) - s
			if (number->D.StringFind.String != NULL &&
					(s = EvalString(number->D.StringFind.String)) != NULL) {
				s2 = strchr(s, number->D.StringFind.C);
				a = s2 ? s2 - s : -1;
				delete s;
				return a;
			} else { // ERROR.
				return 0;
			}
	}
	return 0;
}

/**
**  compute the string expression
**
**  @param s  struct with definition of the calculation.
**
**  @return   the result string.
**
**  @todo Manage better the error.
*/
static char *EvalString(const StringDesc *s)
{
	char *res;   // Result string.
	int i;       // Iterator.
	char *tmp1;  // Temporary string.
	char *tmp2;  // Temporary string.
	const CUnit *unit;  // Temporary unit
	char *str;

	Assert(s);
	switch (s->e) {
		case EString_Lua :     // a lua function.
			return CallLuaStringFunction(s->D.Index);
		case EString_Dir :     // directly a string.
			return new_strdup(s->D.Val);
		case EString_Concat :     // a + b -> "ab"
			tmp1 = EvalString(s->D.Concat.Strings[0]);
			if (!tmp1) {
				tmp1 = new char[1];
				tmp1[0] = '\0';
			}
			res = tmp1;
			for (i = 1; i < s->D.Concat.n; i++) {
				tmp2 = EvalString(s->D.Concat.Strings[i]);
				if (tmp2) {
					res = strdcat(tmp1, tmp2);
					delete[] tmp1;
					delete[] tmp2;
					tmp1 = res;
				}
			}
			return res;
		case EString_String :     // 42 -> "42".
			res = new char[10]; // Should be enough ?
			sprintf(res, "%d", EvalNumber(s->D.Number));
			return res;
		case EString_InverseVideo : // "a" -> "~<a~>"
			tmp1 = EvalString(s->D.String);
			// FIXME replace existing "~<" by "~>" in tmp1.
			res = strdcat3("~<", tmp1, "~>");
			delete[] tmp1;
			return res;
		case EString_UnitName : // name of the UnitType
			unit = EvalUnit(s->D.Unit);
			if (unit != NULL) {
				return new_strdup(unit->Type->Name.c_str());
			} else { // ERROR.
				return NULL;
			}
		case EString_If : // cond ? True : False;
			if (EvalNumber(s->D.If.Cond)) {
				return EvalString(s->D.If.True);
			} else if (s->D.If.False) {
				return EvalString(s->D.If.False);
			} else {
				str = new char[1];
				str[0] = '\0';
				return str;
			}
		case EString_SubString : // substring(s, begin, end)
			if (s->D.SubString.String != NULL &&
					(tmp1 = EvalString(s->D.SubString.String)) != NULL) {
				int begin;
				int end;

				begin = EvalNumber(s->D.SubString.Begin);
				if ((unsigned) begin > strlen(tmp1) && begin > 0) {
					delete[] tmp1;
					str = new char[1];
					str[0] = '\0';
					return str;
				}
				res = new_strdup(tmp1 + begin);
				delete[] tmp1;
				if (s->D.SubString.End) {
					end = EvalNumber(s->D.SubString.End);
				} else {
					end = -1;
				}
				if ((unsigned)end < strlen(res) && end >= 0) {
					res[end] = '\0';
				}
				return res;
			} else { // ERROR.
				str = new char[1];
				str[0] = '\0';
				return str;
			}
		case EString_Line : // line n of the string
			if (s->D.Line.String == NULL ||
					(tmp1 = EvalString(s->D.Line.String)) == NULL) {
				str = new char[1];
				str[0] = '\0';
				return str; // ERROR.
			} else {
				int line;
				int maxlen;
				CFont *font;

				line = EvalNumber(s->D.Line.Line);
				if (line <= 0) {
					delete[] tmp1;
					str = new char[1];
					str[0] = '\0';
					return str;
				}
				if (s->D.Line.MaxLen) {
					maxlen = EvalNumber(s->D.Line.MaxLen);
					if (maxlen < 0) {
						maxlen = 0;
					}
				} else {
					maxlen = 0;
				}
				font = s->D.Line.Font;
				res = GetLineFont(line, tmp1, maxlen, font);
				delete[] tmp1;
				if (!res) { // ERROR.
					str = new char[1];
					str[0] = '\0';
					res = str;
				}
				return res;
			}
	}
	return NULL;
}


/**
**  Free the unit expression content. (not the pointer itself).
**
**  @param unitdesc  struct to free
*/
static void FreeUnitDesc(UnitDesc *unitdesc)
{
	// Nothing to free mow.
}

/**
**  Free the number expression content. (not the pointer itself).
**
**  @param number  struct to free
*/
static void FreeNumberDesc(NumberDesc *number)
{
	if (number == 0) {
		return;
	}
	switch (number->e) {
		case ENumber_Lua :     // a lua function.
			// FIXME: when lua table should be freed ?
		case ENumber_Dir :     // directly a number.
			break;
		case ENumber_Add :     // a + b.
		case ENumber_Sub :     // a - b.
		case ENumber_Mul :     // a * b.
		case ENumber_Div :     // a / b.
		case ENumber_Min :     // a <= b ? a : b
		case ENumber_Max :     // a >= b ? a : b
		case ENumber_Gt  :     // a > b  ? 1 : 0
		case ENumber_GtEq :    // a >= b ? 1 : 0
		case ENumber_Lt  :     // a < b  ? 1 : 0
		case ENumber_LtEq :    // a <= b ? 1 : 0
		case ENumber_NEq  :    // a <> b ? 1 : 0
		case ENumber_Eq  :     // a == b ? 1 : 0
			FreeNumberDesc(number->D.BinOp.Left);
			FreeNumberDesc(number->D.BinOp.Right);
			delete number->D.BinOp.Left;
			delete number->D.BinOp.Right;
			break;
		case ENumber_Rand :    // random(a) [0..a-1]
			FreeNumberDesc(number->D.N);
			delete number->D.N;
			break;
		case ENumber_UnitStat : // property of unit.
			FreeUnitDesc(number->D.UnitStat.Unit);
			delete number->D.UnitStat.Unit;
			break;
		case ENumber_VideoTextLength : // VideoTextLength(font, s)
			FreeStringDesc(number->D.VideoTextLength.String);
			delete number->D.VideoTextLength.String;
			break;
		case ENumber_StringFind : // strchr(s, c) - s.
			FreeStringDesc(number->D.StringFind.String);
			delete number->D.StringFind.String;
			break;
	}
}

/**
**  Free the String expression content. (not the pointer itself).
**
**  @param s  struct to free
*/
static void FreeStringDesc(StringDesc *s)
{
	if (!s) {
		return;
	}

	switch (s->e) {
		case EString_Lua :     // a lua function.
			// FIXME: when lua table should be freed ?
			break;
		case EString_Dir :     // directly a string.
			delete[] s->D.Val;
			break;
		case EString_Concat :  // "a" + "b" -> "ab"
			for (int i = 0; i < s->D.Concat.n; i++) {
				FreeStringDesc(s->D.Concat.Strings[i]);
				delete s->D.Concat.Strings[i];
			}
			delete[] s->D.Concat.Strings;

			break;
		case EString_String : // 42 -> "42"
			FreeNumberDesc(s->D.Number);
			delete s->D.Number;
			break;
		case EString_InverseVideo : // "a" -> "~<a~>"
			FreeStringDesc(s->D.String);
			delete s->D.String;
			break;
		case EString_UnitName : // Name of the UnitType
			FreeUnitDesc(s->D.Unit);
			delete s->D.Unit;
			break;
		case EString_If : // cond ? True : False;
			FreeNumberDesc(s->D.If.Cond);
			delete s->D.If.Cond;
			FreeStringDesc(s->D.If.True);
			delete s->D.If.True;
			FreeStringDesc(s->D.If.False);
			delete s->D.If.False;
			break;
		case EString_SubString : // substring(s, begin, end)
			FreeStringDesc(s->D.SubString.String);
			delete s->D.SubString.String;
			FreeNumberDesc(s->D.SubString.Begin);
			delete s->D.SubString.Begin;
			FreeNumberDesc(s->D.SubString.End);
			delete s->D.SubString.End;
			break;
		case EString_Line : // line n of the string
			FreeStringDesc(s->D.Line.String);
			delete s->D.Line.String;
			FreeNumberDesc(s->D.Line.Line);
			delete s->D.Line.Line;
			FreeNumberDesc(s->D.Line.MaxLen);
			delete s->D.Line.MaxLen;
			break;
	}
}

/*............................................................................
..  Aliases
............................................................................*/

/**
**  Make alias for some unit Variable function.
**
**  @param l  lua State.
**  @param s  FIXME: docu
**
**  @return   the lua table {"UnitVar", {Unit = s, Variable = arg1,
**                           Component = "Value" or arg2, Loc = [012]}
*/
static int AliasUnitVar(lua_State *l, const char *s)
{
	int nargs; // number of args in lua.

	Assert(0 < lua_gettop(l) && lua_gettop(l) <= 3);
	nargs = lua_gettop(l);
	lua_newtable (l);
	lua_pushnumber(l, 1);
	lua_pushstring(l, "UnitVar");
	lua_rawset(l, -3);
	lua_pushnumber(l, 2);
	lua_newtable (l);

	lua_pushstring(l, "Unit");
	lua_pushstring(l, s);
	lua_rawset(l, -3);
	lua_pushstring(l, "Variable");
	lua_pushvalue(l, 1);
	lua_rawset(l, -3);
	lua_pushstring(l, "Component");
	if (nargs >= 2) {
		lua_pushvalue(l, 2);
	} else {
		lua_pushstring(l, "Value");
	}
	lua_rawset(l, -3);
	lua_pushstring(l, "Loc");
	if (nargs >= 3) {
		//  Warning: type is for unit->Stats->Var...
		//           and Initial is for unit->Type->Var... (no upgrade modification)
		const char *sloc[] = {"Unit", "Initial", "Type", NULL};
		int i;
		const char *key;

		key = LuaToString(l, 3);
		for (i = 0; sloc[i] != NULL; i++) {
			if (!strcmp(key, sloc[i])) {
				lua_pushnumber(l, i);
				break ;
			}
		}
		if (sloc[i] == NULL) {
			LuaError(l, "Bad loc :'%s'" _C_ key);
		}
	} else {
		lua_pushnumber(l, 0);
	}
	lua_rawset(l, -3);

	lua_rawset(l, -3);
	return 1;
}

/**
**  Return equivalent lua table for .
**  {"Unit", {Unit = "Attacker", Variable = arg1, Component = "Value" or arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclUnitAttackerVar(lua_State *l)
{
	if (lua_gettop(l) == 0 || lua_gettop(l) > 3) {
		LuaError(l, "Bad number of arg for AttackerVar()\n");
	}
	return AliasUnitVar(l, "Attacker");
}

/**
**  Return equivalent lua table for .
**  {"Unit", {Unit = "Defender", Variable = arg1, Component = "Value" or arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclUnitDefenderVar(lua_State *l)
{
	if (lua_gettop(l) == 0 || lua_gettop(l) > 3) {
		LuaError(l, "Bad number of arg for DefenderVar()\n");
	}
	return AliasUnitVar(l, "Defender");
}

/**
**  Return equivalent lua table for .
**  {"Unit", {Unit = "Active", Variable = arg1, Component = "Value" or arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclActiveUnitVar(lua_State *l)
{
	if (lua_gettop(l) == 0 || lua_gettop(l) > 3) {
		LuaError(l, "Bad number of arg for ActiveUnitVar()\n");
	}
	return AliasUnitVar(l, "Active");
}


/**
**  Make alias for some function.
**
**  @param l  lua State.
**  @param s  FIXME: docu
**
**  @return the lua table {s, {arg1, arg2, ..., argn}} or {s, arg1}
*/
static int Alias(lua_State *l, const char *s)
{
	int i;     // iterator on argument.
	int narg;  // number of argument

	narg = lua_gettop(l);
	Assert(narg);
	lua_newtable (l);
	lua_pushnumber(l, 1);
	lua_pushstring(l, s);
	lua_rawset(l, -3);
	lua_pushnumber(l, 2);
	if (narg > 1) {
		lua_newtable (l);
		for (i = 1; i <= narg; i++) {
			lua_pushnumber(l, i);
			lua_pushvalue(l, i);
			lua_rawset(l, -3);
		}
	} else {
		lua_pushvalue(l, 1);
	}
	lua_rawset(l, -3);
	return 1;
}

/**
**  Return equivalent lua table for add.
**  {"Add", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclAdd(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Add");
}

/**
**  Return equivalent lua table for add.
**  {"Div", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclSub(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Sub");
}
/**
**  Return equivalent lua table for add.
**  {"Mul", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclMul(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Mul");
}
/**
**  Return equivalent lua table for add.
**  {"Div", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclDiv(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Div");
}
/**
**  Return equivalent lua table for add.
**  {"Min", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclMin(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Min");
}
/**
**  Return equivalent lua table for add.
**  {"Max", {arg1, arg2, argn}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclMax(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Max");
}
/**
**  Return equivalent lua table for add.
**  {"Rand", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclRand(lua_State *l)
{
	LuaCheckArgs(l, 1);
	return Alias(l, "Rand");
}
/**
**  Return equivalent lua table for GreaterThan.
**  {"GreaterThan", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclGreaterThan(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "GreaterThan");
}
/**
**  Return equivalent lua table for GreaterThanOrEq.
**  {"GreaterThanOrEq", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclGreaterThanOrEq(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "GreaterThanOrEq");
}
/**
**  Return equivalent lua table for LessThan.
**  {"LessThan", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclLessThan(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "LessThan");
}
/**
**  Return equivalent lua table for LessThanOrEq.
**  {"LessThanOrEq", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclLessThanOrEq(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "LessThanOrEq");
}
/**
**  Return equivalent lua table for Equal.
**  {"Equal", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclEqual(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "Equal");
}
/**
**  Return equivalent lua table for NotEqual.
**  {"NotEqual", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclNotEqual(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "NotEqual");
}

/**
**  Return equivalent lua table for Concat.
**  {"Concat", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclConcat(lua_State *l)
{
	if (lua_gettop(l) < 1) { // FIXME do extra job for 1.
		LuaError(l, "Bad number of arg for Concat()\n");
	}
	return Alias(l, "Concat");
}

/**
**  Return equivalent lua table for String.
**  {"String", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclString(lua_State *l)
{
	LuaCheckArgs(l, 1);
	return Alias(l, "String");
}

/**
**  Return equivalent lua table for InverseVideo.
**  {"InverseVideo", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclInverseVideo(lua_State *l)
{
	LuaCheckArgs(l, 1);
	return Alias(l, "InverseVideo");
}

/**
**  Return equivalent lua table for UnitName.
**  {"UnitName", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclUnitName(lua_State *l)
{
	LuaCheckArgs(l, 1);
	return Alias(l, "UnitName");
}

/**
**  Return equivalent lua table for If.
**  {"If", {arg1}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclIf(lua_State *l)
{
	if (lua_gettop(l) != 2 && lua_gettop(l) != 3) {
		LuaError(l, "Bad number of arg for If()\n");
	}
	return Alias(l, "If");
}

/**
**  Return equivalent lua table for SubString.
**  {"SubString", {arg1, arg2, arg3}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclSubString(lua_State *l)
{
	if (lua_gettop(l) != 2 && lua_gettop(l) != 3) {
		LuaError(l, "Bad number of arg for SubString()\n");
	}
	return Alias(l, "SubString");
}

/**
**  Return equivalent lua table for Line.
**  {"Line", {arg1, arg2[, arg3]}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclLine(lua_State *l)
{
	if (lua_gettop(l) < 2 || lua_gettop(l) > 4) {
		LuaError(l, "Bad number of arg for Line()\n");
	}
	return Alias(l, "Line");
}

/**
**  Return equivalent lua table for Line.
**  {"Line", "arg1"}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclGameInfo(lua_State *l)
{
	LuaCheckArgs(l, 1);
	return Alias(l, "GameInfo");
}

/**
**  Return equivalent lua table for VideoTextLength.
**  {"VideoTextLength", {Text = arg1, Font = arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclVideoTextLength(lua_State *l)
{
	LuaCheckArgs(l, 2);
	lua_newtable (l);
	lua_pushnumber(l, 1);
	lua_pushstring(l, "VideoTextLength");
	lua_rawset(l, -3);
	lua_pushnumber(l, 2);

	lua_newtable (l);
	lua_pushstring(l, "Font");
	lua_pushvalue(l, 1);
	lua_rawset(l, -3);
	lua_pushstring(l, "Text");
	lua_pushvalue(l, 2);
	lua_rawset(l, -3);

	lua_rawset(l, -3);
	return 1;
}

/**
**  Return equivalent lua table for StringFind.
**  {"StringFind", {arg1, arg2}}
**
**  @param l  Lua state.
**
**  @return   equivalent lua table.
*/
static int CclStringFind(lua_State *l)
{
	LuaCheckArgs(l, 2);
	return Alias(l, "StringFind");
}

/**
**  Get unit from a unit depending of the relation.
**
**  @param unit  unit reference.
**  @param e     relation with unit.
**
**  @return      The desired unit.
*/
static const CUnit *GetUnitRef(const CUnit *unit, EnumUnit e)
{
	Assert(unit);
	switch (e) {
		case UnitRefItSelf:
			return unit;
		case UnitRefInside:
			return unit->UnitInside;
		case UnitRefContainer:
			return unit->Container;
		case UnitRefWorker :
			if (unit->Orders[0]->Action == UnitActionBuilt) {
				return unit->Data.Built.Worker;
			} else {
				return NoUnitP;
			}
		case UnitRefGoal:
			return unit->Goal;
		default:
			Assert(0);
	}
	return NoUnitP;
}

/**
**  Return the value corresponding.
**
**  @param unit   Unit.
**  @param index  Index of the variable.
**  @param e      Component of the variable.
**  @param t      Which var use (0:unit, 1:Type, 2:Stats)
**
**  @return       Value corresponding
*/
static UStrInt GetComponent(const CUnit *unit, int index, EnumVariable e, int t)
{
	UStrInt val;
	CVariable *var;

	Assert(unit);
	Assert(0 <= index && index < UnitTypeVar.NumberVariable);

	switch (t) {
		case 0: // Unit:
			var = &unit->Variable[index];
			break;
		case 1: // Type:
			var = &unit->Type->Variable[index];
			break;
		case 2: // Stats:
			var = &unit->Stats->Variables[index];
			break;
		default:
			DebugPrint("Bad value for GetComponent: t = %d" _C_ t);
			var = &unit->Variable[index];
			break;
	}

	switch (e) {
		case VariableValue:
			val.type = USTRINT_INT;
			val.i = var->Value;
			break;
		case VariableMax:
			val.type = USTRINT_INT;
			val.i = var->Max;
			break;
		case VariableIncrease:
			val.type = USTRINT_INT;
			val.i = var->Increase;
			break;
		case VariableDiff:
			val.type = USTRINT_INT;
			val.i = var->Max - var->Value;
			break;
		case VariablePercent:
			Assert(unit->Variable[index].Max != 0);
			val.type = USTRINT_INT;
			val.i = 100 * var->Value / var->Max;
			break;
		case VariableName:
			if (index == GIVERESOURCE_INDEX) {
				int i;
				for (i = 0; i < MaxCosts; ++i) {
					if (unit->ResourcesHeld[i] != 0) {
						break;
					}
				}
				Assert(i != MaxCosts);
				val.type = USTRINT_STR;
				val.s = DefaultResourceNames[i].c_str();
			} else {
				val.type = USTRINT_STR;
				val.s = UnitTypeVar.VariableName[index];
			}
			break;
	}
	return val;
}

/**
**  Draw text with variable.
**
**  @param unit         unit with variable to show.
**  @param defaultfont  default font if no specific font in extra data.
*/
void CContentTypeText::Draw(const CUnit *unit, CFont *defaultfont) const
{
	char *text;             // Optional text to display.
	CFont *font;            // Font to use.
	int x;                  // X coordinate to display.
	int y;                  // Y coordinate to display.

	x = this->PosX;
	y = this->PosY;
	font = this->Font ? this->Font : defaultfont;
	Assert(font);

	Assert(unit || this->Index == -1);
	Assert(this->Index == -1 || (0 <= this->Index && this->Index < UnitTypeVar.NumberVariable));

	if (this->Text) {
		text = EvalString(this->Text);
		if (this->Centered) {
			VideoDrawTextCentered(x, y, font, text);
		} else {
			VideoDrawText(x, y, font, text);
		}
		x += font->Width(text);
		delete[] text;
	}

	if (this->ShowName) {
		VideoDrawTextCentered(x, y, font, unit->Type->Name);
		return;
	}

	if (this->Index != -1) {
		if (!this->Stat) {
			EnumVariable component = this->Component;
			switch (component) {
				case VariableValue:
				case VariableMax:
				case VariableIncrease:
				case VariableDiff:
				case VariablePercent:
					VideoDrawNumber(x, y, font, GetComponent(unit, this->Index, component, 0).i);
					break;
				case VariableName:
					VideoDrawText(x, y, font, GetComponent(unit, this->Index, component, 0).s);
					break;
				default:
					Assert(0);
			}
		} else {
			int value = unit->Type->Variable[this->Index].Value;
			int diff = unit->Stats->Variables[this->Index].Value - value;

			if (!diff) {
				VideoDrawNumber(x, y, font, value);
			} else {
				char buf[64];
				sprintf(buf, diff > 0 ? "%d~<+%d~>" : "%d~<-%d~>", value, diff);
				VideoDrawText(x, y, font, buf);
			}
		}
	}
}

/**
**  Draw formatted text with variable value.
**
**  @param unit         unit with variable to show.
**  @param defaultfont  default font if no specific font in extra data.
**
**  @note text is limited to 256 chars. (enough?)
**  @note text must have exactly 1 %d.
**  @bug if text format is incorrect.
*/
void CContentTypeFormattedText::Draw(const CUnit *unit, CFont *defaultfont) const
{
	CFont *font;
	char buf[256];
	UStrInt usi1;

	Assert(unit);
	font = this->Font ? this->Font : defaultfont;
	Assert(font);

	Assert(0 <= this->Index && this->Index < UnitTypeVar.NumberVariable);
	usi1 = GetComponent(unit, this->Index, this->Component, 0);
	if (usi1.type == USTRINT_STR) {
		sprintf(buf, this->Format, usi1.s);
	} else {
		sprintf(buf, this->Format, usi1.i);
	}

	if (this->Centered) {
		VideoDrawTextCentered(this->PosX, this->PosY, font, buf);
	} else {
		VideoDrawText(this->PosX, this->PosY, font, buf);
	}
}

/**
**  Draw formatted text with variable value.
**
**  @param unit         unit with variable to show.
**  @param defaultfont  default font if no specific font in extra data.
**
**  @note text is limited to 256 chars. (enough?)
**  @note text must have exactly 2 %d.
**  @bug if text format is incorrect.
*/
void CContentTypeFormattedText2::Draw(const CUnit *unit, CFont *defaultfont) const
{
	CFont *font;
	char buf[256];
	UStrInt usi1, usi2;

	Assert(unit);
	font = this->Font ? this->Font : defaultfont;
	Assert(font);

	usi1 = GetComponent(unit, this->Index1, this->Component1, 0);
	usi2 = GetComponent(unit, this->Index2, this->Component2, 0);
	if (usi1.type == USTRINT_STR) {
		if (usi2.type == USTRINT_STR) {
			sprintf(buf, this->Format, usi1.s, usi2.s);
		} else {
			sprintf(buf, this->Format, usi1.s, usi2.i);
		}
	} else {
		if (usi2.type == USTRINT_STR) {
			sprintf(buf, this->Format, usi1.i, usi2.s);
		} else {
			sprintf(buf, this->Format, usi1.i, usi2.i);
		}
	}
	if (this->Centered) {
		VideoDrawTextCentered(this->PosX, this->PosY, font, buf);
	} else {
		VideoDrawText(this->PosX, this->PosY, font, buf);
	}
}

/**
**  Draw icon for unit.
**
**  @param unit         unit with icon to show.
**  @param defaultfont  unused.
*/
void CContentTypeIcon::Draw(const CUnit *unit, CFont *defaultfont) const
{
	Assert(unit);
	unit = GetUnitRef(unit, this->UnitRef);
	if (unit && unit->Type->Icon.Icon) {
		unit->Type->Icon.Icon->DrawIcon(unit->Player, this->PosX, this->PosY);
	}
}

/**
**  Draw life bar of a unit using selected variable.
**  Placed under icons on top-panel.
**
**  @param unit         Pointer to unit.
**  @param defaultfont  FIXME: docu
**
**  @todo Color and percent value Parametrisation.
*/
void CContentTypeLifeBar::Draw(const CUnit *unit, CFont *defaultfont) const
{
	Assert(unit);
	Assert(0 <= this->Index && this->Index < UnitTypeVar.NumberVariable);
	if (!unit->Variable[this->Index].Max) {
		return;
	}

	Uint32 color;
	int f = (100 * unit->Variable[this->Index].Value) / unit->Variable[this->Index].Max;

	if (f > 75) {
		color = ColorDarkGreen;
	} else if (f > 50) {
		color = ColorYellow;
	} else if (f > 25) {
		color = ColorOrange;
	} else {
		color = ColorRed;
	}

	// Border
	Video.FillRectangleClip(ColorBlack, this->PosX - 1, this->PosY - 1,
		this->Width + 2, this->Height + 2);

	Video.FillRectangleClip(color, this->PosX, this->PosY,
		(f * this->Width) / 100, this->Height);
}

/**
**  Draw life bar of a unit using selected variable.
**  Placed under icons on top-panel.
**
**  @param unit         Pointer to unit.
**  @param defaultfont  FIXME: docu
**
**  @todo Color and percent value Parametrisation.
*/
void CContentTypeCompleteBar::Draw(const CUnit *unit, CFont *defaultfont) const
{
	Assert(unit);
	Assert(0 <= this->Index && this->Index < UnitTypeVar.NumberVariable);
	if (!unit->Variable[this->Index].Max) {
		return;
	}

	int x = this->PosX;
	int y = this->PosY;
	int w = this->Width;
	int h = this->Height;

	Assert(w > 0);
	Assert(h > 4);

	int f = (100 * unit->Variable[this->Index].Value) / unit->Variable[this->Index].Max;

	if (!this->Border) {
		Video.FillRectangleClip(UI.CompletedBarColor, x, y, f * w / 100, h);
		if (UI.CompletedBarShadow) {
			// Shadow
			Video.DrawVLine(ColorGray, x + f * w / 100, y, h);
			Video.DrawHLine(ColorGray, x, y + h, f * w / 100);

			// |~  Light
			Video.DrawVLine(ColorWhite, x, y, h);
			Video.DrawHLine(ColorWhite, x, y, f * w / 100);
		}
	} else {
		Video.DrawRectangleClip(ColorGray, x,     y,     w + 4, h );
		Video.DrawRectangleClip(ColorBlack,x + 1, y + 1, w + 2, h - 2);
		Video.FillRectangleClip(ColorBlue, x + 2, y + 2, f * w / 100, h - 4);
	}
}

/**
**  Register alias functions
*/
void PanelRegister()
{
	lua_register(Lua, "DefinePanelContents", CclDefinePanelContents);

	// Number.
	lua_register(Lua, "Add", CclAdd);
	lua_register(Lua, "Sub", CclSub);
	lua_register(Lua, "Mul", CclMul);
	lua_register(Lua, "Div", CclDiv);
	lua_register(Lua, "Min", CclMin);
	lua_register(Lua, "Max", CclMax);
	lua_register(Lua, "Rand", CclRand);

	lua_register(Lua, "GreaterThan", CclGreaterThan);
	lua_register(Lua, "LessThan", CclLessThan);
	lua_register(Lua, "Equal", CclEqual);
	lua_register(Lua, "GreaterThanOrEq", CclGreaterThanOrEq);
	lua_register(Lua, "LessThanOrEq", CclLessThanOrEq);
	lua_register(Lua, "NotEqual", CclNotEqual);
	lua_register(Lua, "VideoTextLength", CclVideoTextLength);
	lua_register(Lua, "StringFind", CclStringFind);


	// Unit
	lua_register(Lua, "AttackerVar", CclUnitAttackerVar);
	lua_register(Lua, "DefenderVar", CclUnitDefenderVar);
	lua_register(Lua, "ActiveUnitVar", CclActiveUnitVar);


	// String.
	lua_register(Lua, "Concat", CclConcat);
	lua_register(Lua, "String", CclString);
	lua_register(Lua, "InverseVideo", CclInverseVideo);
	lua_register(Lua, "UnitName", CclUnitName);
	lua_register(Lua, "SubString", CclSubString);
	lua_register(Lua, "Line", CclLine);
	lua_register(Lua, "GameInfo", CclGameInfo);

	lua_register(Lua, "If", CclIf);
}

//@}
