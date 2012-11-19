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
/**@name script_ui.cpp - The ui ccl functions. */
//
//      (c) Copyright 1999-2007 by Lutz Sammer, Jimmy Salmon, Martin Renold
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "ui.h"

#include "font.h"
#include "interface.h"
#include "map.h"
#include "menus.h"
#include "script.h"
#include "spells.h"
#include "title.h"
#include "util.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

std::string ClickMissile;        /// FIXME:docu
std::string DamageMissile;       /// FIXME:docu
std::map<std::string, ButtonStyle *> ButtonStyleHash;

static int HandleCount = 1;     /// Lua handler count

CPreference Preference;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


/**
**  Set speed of mouse scroll
**
**  @param l  Lua state.
*/
static int CclSetMouseScrollSpeed(lua_State *l)
{
	LuaCheckArgs(l, 1);
	UI.MouseScrollSpeed = LuaToNumber(l, 1);
	return 0;
}

/**
**  Get speed of mouse scroll
**
**  @param l  Lua state.
*/
static int CclGetMouseScrollSpeed(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushnumber(l, UI.MouseScrollSpeed);
	return 1;
}

/**
**  Set speed of middle-mouse scroll
**
**  @param l  Lua state.
*/
static int CclSetMouseScrollSpeedDefault(lua_State *l)
{
	LuaCheckArgs(l, 1);
	UI.MouseScrollSpeedDefault = LuaToNumber(l, 1);
	return 0;
}

/**
**  Get speed of middle-mouse scroll
**
**  @param l  Lua state.
*/
static int CclGetMouseScrollSpeedDefault(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushnumber(l, UI.MouseScrollSpeedDefault);
	return 0;
}

/**
**  Set speed of ctrl-middle-mouse scroll
**
**  @param l  Lua state.
*/
static int CclSetMouseScrollSpeedControl(lua_State *l)
{
	LuaCheckArgs(l, 1);
	UI.MouseScrollSpeedControl = LuaToNumber(l, 1);
	return 0;
}

/**
**  Get speed of ctrl-middle-mouse scroll
**
**  @param l  Lua state.
*/
static int CclGetMouseScrollSpeedControl(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushnumber(l, UI.MouseScrollSpeedControl);
	return 0;
}

/**
**  Set which missile is used for right click
**
**  @param l  Lua state.
*/
static int CclSetClickMissile(lua_State *l)
{
	const int args = lua_gettop(l);
	if (args > 1 || (args == 1 && (!lua_isnil(l, 1) && !lua_isstring(l, 1)))) {
		LuaError(l, "incorrect argument");
	}
	ClickMissile.clear();
	if (args == 1 && !lua_isnil(l, 1)) {
		ClickMissile = lua_tostring(l, 1);
	}
	return 0;
}

/**
**  Set which missile shows Damage
**
**  @param l  Lua state.
*/
static int CclSetDamageMissile(lua_State *l)
{
	const int args = lua_gettop(l);

	if (args > 1 || (args == 1 && (!lua_isnil(l, 1) && !lua_isstring(l, 1)))) {
		LuaError(l, "incorrect argument");
	}
	DamageMissile.clear();
	if (args == 1 && !lua_isnil(l, 1)) {
		DamageMissile = lua_tostring(l, 1);
	}
	return 0;
}

static int CclSetMaxOpenGLTexture(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (CclInConfigFile) {
		GLMaxTextureSizeOverride = LuaToNumber(l, 1);
	}
	return 0;
}

static int CclSetUseOpenGL(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (CclInConfigFile) {
		// May have been set from the command line
		if (!ForceUseOpenGL) {
			UseOpenGL = LuaToBoolean(l, 1);
		}
	}
	return 0;
}

/**
**  Set the video resolution.
**
**  @param l  Lua state.
*/
static int CclSetVideoResolution(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (CclInConfigFile) {
		// May have been set from the command line
		if (!Video.Width || !Video.Height) {
			Video.Width = LuaToNumber(l, 1);
			Video.Height = LuaToNumber(l, 2);
		}
	}
	return 0;
}

/**
**  Get the video resolution.
**
**  @param l  Lua state.
*/
static int CclGetVideoResolution(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushnumber(l, Video.Width);
	lua_pushnumber(l, Video.Height);
	return 2;
}

/**
**  Set the video fullscreen mode.
**
**  @param l  Lua state.
*/
static int CclSetVideoFullScreen(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (CclInConfigFile) {
		// May have been set from the command line
		if (!VideoForceFullScreen) {
			Video.FullScreen = LuaToBoolean(l, 1);
		}
	}
	return 0;
}

/**
**  Get the video fullscreen mode.
**
**  @param l  Lua state.
*/
static int CclGetVideoFullScreen(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushboolean(l, Video.FullScreen);
	return 1;
}

/**
**  Default title screens.
**
**  @param l  Lua state.
*/
static int CclSetTitleScreens(lua_State *l)
{
	if (TitleScreens) {
		for (int i = 0; TitleScreens[i]; ++i) {
			delete TitleScreens[i];
		}
		delete[] TitleScreens;
		TitleScreens = NULL;
	}

	const int args = lua_gettop(l);
	TitleScreens = new TitleScreen *[args + 1];
	memset(TitleScreens, 0, (args + 1) * sizeof(TitleScreen *));

	for (int j = 0; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			LuaError(l, "incorrect argument");
		}
		TitleScreens[j] = new TitleScreen;
		TitleScreens[j]->Iterations = 1;
		lua_pushnil(l);
		while (lua_next(l, j + 1)) {
			const char *value = LuaToString(l, -2);
			if (!strcmp(value, "Image")) {
				TitleScreens[j]->File = LuaToString(l, -1);
			} else if (!strcmp(value, "Music")) {
				TitleScreens[j]->Music = LuaToString(l, -1);
			} else if (!strcmp(value, "Timeout")) {
				TitleScreens[j]->Timeout = LuaToNumber(l, -1);
			} else if (!strcmp(value, "Iterations")) {
				TitleScreens[j]->Iterations = LuaToNumber(l, -1);
			} else if (!strcmp(value, "Editor")) {
				TitleScreens[j]->Editor = LuaToNumber(l, -1);
			} else if (!strcmp(value, "Labels")) {
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				const int subargs = lua_rawlen(l, -1);
				TitleScreens[j]->Labels = new TitleScreenLabel *[subargs + 1];
				memset(TitleScreens[j]->Labels, 0, (subargs + 1) * sizeof(TitleScreenLabel *));
				for (int k = 0; k < subargs; ++k) {
					lua_rawgeti(l, -1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					TitleScreens[j]->Labels[k] = new TitleScreenLabel;
					lua_pushnil(l);
					while (lua_next(l, -2)) {
						const char *value = LuaToString(l, -2);
						if (!strcmp(value, "Text")) {
							TitleScreens[j]->Labels[k]->Text = LuaToString(l, -1);
						} else if (!strcmp(value, "Font")) {
							TitleScreens[j]->Labels[k]->Font = CFont::Get(LuaToString(l, -1));
						} else if (!strcmp(value, "Pos")) {
							CclGetPos(l, &TitleScreens[j]->Labels[k]->Xofs, &TitleScreens[j]->Labels[k]->Yofs);
						} else if (!strcmp(value, "Flags")) {
							if (!lua_istable(l, -1)) {
								LuaError(l, "incorrect argument");
							}
							const int subsubargs = lua_rawlen(l, -1);
							for (int subk = 0; subk < subsubargs; ++subk) {
								lua_rawgeti(l, -1, subk + 1);
								const char *value = LuaToString(l, -1);
								lua_pop(l, 1);
								if (!strcmp(value, "center")) {
									TitleScreens[j]->Labels[k]->Flags |= TitleFlagCenter;
								} else {
									LuaError(l, "incorrect flag");
								}
							}
						} else {
							LuaError(l, "Unsupported key: %s" _C_ value);
						}
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				}
			} else {
				LuaError(l, "Unsupported key: %s" _C_ value);
			}
			lua_pop(l, 1);
		}
	}
	return 0;
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
EnumVariable Str2EnumVariable(lua_State *l, const char *s)
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
		{0, VariableValue}
	}; // List of possible values.

	for (int i = 0; list[i].s; ++i) {
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
		{0, UnitRefItSelf}
	}; // List of possible values.

	for (int i = 0; list[i].s; ++i) {
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
	Assert(lua_istable(l, -1));

	ConditionPanel *condition = new ConditionPanel;

	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);
		if (!strcmp(key, "ShowOnlySelected")) {
			condition->ShowOnlySelected = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "HideNeutral")) {
			condition->HideNeutral = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "HideAllied")) {
			condition->HideAllied = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "ShowOpponent")) {
			condition->ShowOpponent = LuaToBoolean(l, -1);
		} else {
			int index = UnitTypeVar.BoolFlagNameLookup[key];
			if (index != -1) {
				if (!condition->BoolFlags) {
					size_t new_bool_size = UnitTypeVar.GetNumberBoolFlag();
					condition->BoolFlags = new char[new_bool_size];
					memset(condition->BoolFlags, 0, new_bool_size * sizeof(char));
				}
				condition->BoolFlags[index] = Ccl2Condition(l, LuaToString(l, -1));
				continue;
			}
			index = UnitTypeVar.VariableNameLookup[key];
			if (index != -1) {
				if (!condition->Variables) {
					size_t new_variables_size = UnitTypeVar.GetNumberVariable();
					condition->Variables = new char[new_variables_size];
					memset(condition->Variables, 0, new_variables_size * sizeof(char));
				}
				condition->Variables[index] = Ccl2Condition(l, LuaToString(l, -1));
				continue;
			}
			LuaError(l, "'%s' invalid for Condition in DefinePanels" _C_ key);
		}
	}
	return condition;
}

/* virtual */ void CContentTypeText::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1) || lua_isstring(l, -1));

	if (lua_isstring(l, -1)) {
		this->Text = CclParseStringDesc(l);
		lua_pushnil(l); // ParseStringDesc eat token
	} else {
		for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
			const char *key = LuaToString(l, -2);
			if (!strcmp(key, "Text")) {
				this->Text = CclParseStringDesc(l);
				lua_pushnil(l); // ParseStringDesc eat token
			} else if (!strcmp(key, "Font")) {
				this->Font = CFont::Get(LuaToString(l, -1));
			} else if (!strcmp(key, "Centered")) {
				this->Centered = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "Variable")) {
				const char *const name = LuaToString(l, -1);
				this->Index = UnitTypeVar.VariableNameLookup[name];
				if (this->Index == -1) {
					LuaError(l, "unknown variable '%s'" _C_ LuaToString(l, -1));
				}
			} else if (!strcmp(key, "Component")) {
				this->Component = Str2EnumVariable(l, LuaToString(l, -1));
			} else if (!strcmp(key, "Stat")) {
				this->Stat = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "ShowName")) {
				this->ShowName = LuaToBoolean(l, -1);
			} else {
				LuaError(l, "'%s' invalid for method 'Text' in DefinePanels" _C_ key);
			}
		}
	}
}

/* virtual */ void CContentTypeFormattedText::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1));

	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);
		if (!strcmp(key, "Format")) {
			this->Format = LuaToString(l, -1);
		} else if (!strcmp(key, "Font")) {
			this->Font = CFont::Get(LuaToString(l, -1));
		} else if (!strcmp(key, "Variable")) {
			const char *const name = LuaToString(l, -1);
			this->Index = UnitTypeVar.VariableNameLookup[name];
			if (this->Index == -1) {
				LuaError(l, "unknown variable '%s'" _C_ name);
			}
		} else if (!strcmp(key, "Component")) {
			this->Component = Str2EnumVariable(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Centered")) {
			this->Centered = LuaToBoolean(l, -1);
		} else {
			LuaError(l, "'%s' invalid for method 'FormattedText' in DefinePanels" _C_ key);
		}
	}
}

/* virtual */ void CContentTypeFormattedText2::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1));
	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);
		if (!strcmp(key, "Format")) {
			this->Format = LuaToString(l, -1);
		} else if (!strcmp(key, "Font")) {
			this->Font = CFont::Get(LuaToString(l, -1));
		} else if (!strcmp(key, "Variable")) {
			const char *const name = LuaToString(l, -1);
			this->Index1 = UnitTypeVar.VariableNameLookup[name];
			this->Index2 = this->Index1;
			if (this->Index1 == -1) {
				LuaError(l, "unknown variable '%s'" _C_ name);
			}
		} else if (!strcmp(key, "Component")) {
			this->Component1 = Str2EnumVariable(l, LuaToString(l, -1));
			this->Component2 = Str2EnumVariable(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Variable1")) {
			const char *const name = LuaToString(l, -1);
			this->Index1 = UnitTypeVar.VariableNameLookup[name];
			if (this->Index1 == -1) {
				LuaError(l, "unknown variable '%s'" _C_ name);
			}
		} else if (!strcmp(key, "Component1")) {
			this->Component1 = Str2EnumVariable(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Variable2")) {
			const char *const name = LuaToString(l, -1);
			this->Index2 = UnitTypeVar.VariableNameLookup[name];
			if (this->Index2 == -1) {
				LuaError(l, "unknown variable '%s'" _C_ LuaToString(l, -1));
			}
		} else if (!strcmp(key, "Component2")) {
			this->Component2 = Str2EnumVariable(l, LuaToString(l, -1));
		} else if (!strcmp(key, "Centered")) {
			this->Centered = LuaToBoolean(l, -1);
		} else {
			LuaError(l, "'%s' invalid for method 'FormattedText2' in DefinePanels" _C_ key);
		}
	}

}

/* virtual */ void CContentTypeIcon::Parse(lua_State *l)
{
	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);
		if (!strcmp(key, "Unit")) {
			this->UnitRef = Str2EnumUnit(l, LuaToString(l, -1));
		} else {
			LuaError(l, "'%s' invalid for method 'Icon' in DefinePanels" _C_ key);
		}
	}
}

/* virtual */ void CContentTypeLifeBar::Parse(lua_State *l)
{
	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);
		if (!strcmp(key, "Variable")) {
			const char *const name = LuaToString(l, -1);
			this->Index = UnitTypeVar.VariableNameLookup[name];
			if (this->Index == -1) {
				LuaError(l, "unknown variable '%s'" _C_ name);
			}
		} else if (!strcmp(key, "Height")) {
			this->Height = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Width")) {
			this->Width = LuaToNumber(l, -1);
		} else {
			LuaError(l, "'%s' invalid for method 'LifeBar' in DefinePanels" _C_ key);
		}
	}
	// Default value and checking errors.
	if (this->Height <= 0) {
		this->Height = 5; // Default value.
	}
	if (this->Width <= 0) {
		this->Width = 50; // Default value.
	}
	if (this->Index == -1) {
		LuaError(l, "variable undefined for LifeBar");
	}
}

/* virtual */ void CContentTypeCompleteBar::Parse(lua_State *l)
{
	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);

		if (!strcmp(key, "Variable")) {
			const char *const name = LuaToString(l, -1);
			this->Index = UnitTypeVar.VariableNameLookup[name];
			if (this->Index == -1) {
				LuaError(l, "unknown variable '%s'" _C_ name);
			}
		} else if (!strcmp(key, "Height")) {
			this->Height = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Width")) {
			this->Width = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Border")) {
			this->Border = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "Color")) {
			//FIXME: need more general way
			const char *const color = LuaToString(l, -1);
			if (!strcmp(color, "red")) {
				this->Color = 1;
			} else if (!strcmp(color, "yellow")) {
				this->Color = 2;
			} else if (!strcmp(color, "green")) {
				this->Color = 3;
			} else if (!strcmp(color, "gray")) {
				this->Color = 4;
			} else if (!strcmp(color, "white")) {
				this->Color = 5;
			} else if (!strcmp(color, "orange")) {
				this->Color = 6;
			} else if (!strcmp(color, "blue")) {
				this->Color = 7;
			} else if (!strcmp(color, "dark-green")) {
				this->Color = 8;
			} else if (!strcmp(color, "black")) {
				this->Color = 9;
			} else {
				LuaError(l, "incorrect color: '%s' " _C_ color);
			}
		} else {
			LuaError(l, "'%s' invalid for method 'CompleteBar' in DefinePanels" _C_ key);
		}
	}
	// Default value and checking errors.
	if (this->Height <= 0) {
		this->Height = 5; // Default value.
	}
	if (this->Width <= 0) {
		this->Width = 50; // Default value.
	}
	if (this->Index == -1) {
		LuaError(l, "variable undefined for CompleteBar");
	}
}

static CContentType *CclParseContent(lua_State *l)
{
	Assert(lua_istable(l, -1));

	CContentType *content = NULL;
	ConditionPanel *condition = NULL;
	int posX = 0;
	int posY = 0;

	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);
		if (!strcmp(key, "Pos")) {
			CclGetPos(l, &posX, &posY);
		} else if (!strcmp(key, "More")) {
			Assert(lua_istable(l, -1));
			lua_rawgeti(l, -1, 1); // Method name
			lua_rawgeti(l, -2, 2); // Method data
			key = LuaToString(l, -2);
			if (!strcmp(key, "Text")) {
				content = new CContentTypeText;
			} else if (!strcmp(key, "FormattedText")) {
				content = new CContentTypeFormattedText;
			} else if (!strcmp(key, "FormattedText2")) {
				content = new CContentTypeFormattedText2;
			} else if (!strcmp(key, "Icon")) {
				content = new CContentTypeIcon;
			} else if (!strcmp(key, "LifeBar")) {
				content = new CContentTypeLifeBar;
			} else if (!strcmp(key, "CompleteBar")) {
				content = new CContentTypeCompleteBar;
			} else {
				LuaError(l, "Invalid drawing method '%s' in DefinePanels" _C_ key);
			}
			content->Parse(l);
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
	const int nargs = lua_gettop(l);

	for (int i = 0; i < nargs; i++) {
		Assert(lua_istable(l, i + 1));
		CUnitInfoPanel *infopanel = new CUnitInfoPanel;

		for (lua_pushnil(l); lua_next(l, i + 1); lua_pop(l, 1)) {
			const char *key = LuaToString(l, -2);

			if (!strcmp(key, "Ident")) {
				infopanel->Name = LuaToString(l, -1);
			} else if (!strcmp(key, "Pos")) {
				CclGetPos(l, &infopanel->PosX, &infopanel->PosY);
			} else if (!strcmp(key, "DefaultFont")) {
				infopanel->DefaultFont = CFont::Get(LuaToString(l, -1));
			} else if (!strcmp(key, "Condition")) {
				infopanel->Condition = ParseConditionPanel(l);
			} else if (!strcmp(key, "Contents")) {
				Assert(lua_istable(l, -1));
				for (size_t j = 0; j < lua_rawlen(l, -1); j++, lua_pop(l, 1)) {
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
		size_t j;
		for (j = 0; j < UI.InfoPanelContents.size(); ++j) {
			if (infopanel->Name == UI.InfoPanelContents[j]->Name) {
				DebugPrint("Redefinition of Panel '%s'\n" _C_ infopanel->Name.c_str());
				delete UI.InfoPanelContents[j];
				UI.InfoPanelContents[j] = infopanel;
				break;
			}
		}
		if (j == UI.InfoPanelContents.size()) {
			UI.InfoPanelContents.push_back(infopanel);
		}
	}
	return 0;
}

/**
**  Parse the popup conditions.
**
**  @param l   Lua State.
*/
static PopupConditionPanel *ParsePopupConditions(lua_State *l)
{
	Assert(lua_istable(l, -1));

	PopupConditionPanel *condition = new PopupConditionPanel;
	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);

		if (!strcmp(key, "HasHint")) {
			condition->HasHint = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "HasDescription")) {
			condition->HasDescription = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "HasDependencies")) {
			condition->HasDependencies = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "ButtonValue")) {
			condition->ButtonValue = LuaToString(l, -1);
		} else if (!strcmp(key, "ButtonAction")) {
			const char *value = LuaToString(l, -1);
			if (!strcmp(value, "move")) {
				condition->ButtonAction = ButtonMove;
			} else if (!strcmp(value, "stop")) {
				condition->ButtonAction = ButtonStop;
			} else if (!strcmp(value, "attack")) {
				condition->ButtonAction = ButtonAttack;
			} else if (!strcmp(value, "repair")) {
				condition->ButtonAction = ButtonRepair;
			} else if (!strcmp(value, "harvest")) {
				condition->ButtonAction = ButtonHarvest;
			} else if (!strcmp(value, "button")) {
				condition->ButtonAction = ButtonButton;
			} else if (!strcmp(value, "build")) {
				condition->ButtonAction = ButtonBuild;
			} else if (!strcmp(value, "train-unit")) {
				condition->ButtonAction = ButtonTrain;
			} else if (!strcmp(value, "patrol")) {
				condition->ButtonAction = ButtonPatrol;
			} else if (!strcmp(value, "stand-ground")) {
				condition->ButtonAction = ButtonStandGround;
			} else if (!strcmp(value, "attack-ground")) {
				condition->ButtonAction = ButtonAttackGround;
			} else if (!strcmp(value, "return-goods")) {
				condition->ButtonAction = ButtonReturn;
			} else if (!strcmp(value, "cast-spell")) {
				condition->ButtonAction = ButtonSpellCast;
			} else if (!strcmp(value, "research")) {
				condition->ButtonAction = ButtonResearch;
			} else if (!strcmp(value, "upgrade-to")) {
				condition->ButtonAction = ButtonUpgradeTo;
			} else if (!strcmp(value, "unload")) {
				condition->ButtonAction = ButtonUnload;
			} else if (!strcmp(value, "cancel")) {
				condition->ButtonAction = ButtonCancel;
			} else if (!strcmp(value, "cancel-upgrade")) {
				condition->ButtonAction = ButtonCancelUpgrade;
			} else if (!strcmp(value, "cancel-train-unit")) {
				condition->ButtonAction = ButtonCancelTrain;
			} else if (!strcmp(value, "cancel-build")) {
				condition->ButtonAction = ButtonCancelBuild;
			} else {
				LuaError(l, "Unsupported button action: %s" _C_ value);
			}
		} else {
			int index = UnitTypeVar.BoolFlagNameLookup[key];
			if (index != -1) {
				if (!condition->BoolFlags) {
					size_t new_bool_size = UnitTypeVar.GetNumberBoolFlag();
					condition->BoolFlags = new char[new_bool_size];
					memset(condition->BoolFlags, 0, new_bool_size * sizeof(char));
				}
				condition->BoolFlags[index] = Ccl2Condition(l, LuaToString(l, -1));
				continue;
			}
			index = UnitTypeVar.VariableNameLookup[key];
			if (index != -1) {
				if (!condition->Variables) {
					size_t new_variables_size = UnitTypeVar.GetNumberVariable();
					condition->Variables = new char[new_variables_size];
					memset(condition->Variables, 0, new_variables_size * sizeof(char));
				}
				condition->Variables[index] = Ccl2Condition(l, LuaToString(l, -1));
				continue;
			}
			LuaError(l, "'%s' invalid for Condition in DefinePopups" _C_ key);
		}
	}
	return condition;
}

/* virtual*/ void CPopupContentTypeButtonInfo::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1));

	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);
		if (!strcmp(key, "InfoType")) {
			std::string temp(LuaToString(l, -1));
			if (temp == "Hint") {
				this->InfoType = PopupButtonInfo_Hint;
			} else if (temp == "Description") {
				this->InfoType = PopupButtonInfo_Description;
			} else if (temp == "Dependencies") {
				this->InfoType = PopupButtonInfo_Dependencies;
			}
		} else if (!strcmp(key, "MaxWidth")) {
			this->MaxWidth = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Font")) {
			this->Font = CFont::Get(LuaToString(l, -1));
		} else {
			LuaError(l, "'%s' invalid for method 'Name' in DefinePopups" _C_ key);
		}
	}
}

/* virtual*/ void CPopupContentTypeText::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1));

	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);
		if (!strcmp(key, "Text")) {
			this->Text = LuaToString(l, -1);
		} else if (!strcmp(key, "MaxWidth")) {
			this->MaxWidth = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Font")) {
			this->Font = CFont::Get(LuaToString(l, -1));
		} else {
			LuaError(l, "'%s' invalid for method 'Text' in DefinePopups" _C_ key);
		}
	}
}

/* virtual*/ void CPopupContentTypeCosts::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1) || lua_isnil(l, -1));

	if (!lua_isnil(l, -1)) {
		for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
			const char *key = LuaToString(l, -2);
			if (!strcmp(key, "Font")) {
				this->Font = CFont::Get(LuaToString(l, -1));
			} else if (!strcmp(key, "Centered")) {
				this->Centered = LuaToBoolean(l, -1);
			} else {
				LuaError(l, "'%s' invalid for method 'Costs' in DefinePopups" _C_ key);
			}
		}
	}
}

/* virtual*/ void CPopupContentTypeLine::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1) || lua_isnil(l, -1));

	if (!lua_isnil(l, -1)) {
		for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
			const char *key = LuaToString(l, -2);
			if (!strcmp(key, "Width")) {
				this->Width = LuaToNumber(l, -1);
			} else if (!strcmp(key, "Height")) {
				this->Height = LuaToNumber(l, -1);
			} else if (!strcmp(key, "Color")) {
				this->Color = LuaToNumber(l, -1);
			} else {
				LuaError(l, "'%s' invalid for method 'Costs' in DefinePopups" _C_ key);
			}
		}
	}
}

/* virtual*/ void CPopupContentTypeVariable::Parse(lua_State *l)
{
	Assert(lua_istable(l, -1) || lua_isstring(l, -1));

	if (lua_isstring(l, -1)) {
		this->Text = CclParseStringDesc(l);
		lua_pushnil(l); // ParseStringDesc eat token
	} else {
		for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
			const char *key = LuaToString(l, -2);
			if (!strcmp(key, "Text")) {
				this->Text = CclParseStringDesc(l);
				lua_pushnil(l); // ParseStringDesc eat token
			} else if (!strcmp(key, "Font")) {
				this->Font = CFont::Get(LuaToString(l, -1));
			} else if (!strcmp(key, "Centered")) {
				this->Centered = LuaToBoolean(l, -1);
			} else if (!strcmp(key, "Variable")) {
				const char *const name = LuaToString(l, -1);
				this->Index = UnitTypeVar.VariableNameLookup[name];
				if (this->Index == -1) {
					LuaError(l, "unknown variable '%s'" _C_ LuaToString(l, -1));
				}
			} else {
				LuaError(l, "'%s' invalid for method 'Text' in DefinePopups" _C_ key);
			}
		}
	}
}

/* static */ CPopupContentType *CPopupContentType::ParsePopupContent(lua_State *l)
{
	Assert(lua_istable(l, -1));

	bool wrap = true;
	int marginX = MARGIN_X;
	int marginY = MARGIN_Y;
	int minWidth = 0;
	int minHeight = 0;
	std::string textColor("white");
	std::string highColor("red");
	CPopupContentType *content = NULL;
	PopupConditionPanel *condition = NULL;

	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);

		if (!strcmp(key, "Wrap")) {
			wrap = LuaToBoolean(l, -1);
		} else if (!strcmp(key, "TextColor")) {
			textColor = LuaToString(l, -1);
		} else if (!strcmp(key, "HighlightColor")) {
			highColor = LuaToString(l, -1);
		} else if (!strcmp(key, "Margin")) {
			CclGetPos(l, &marginX, &marginY);
		} else if (!strcmp(key, "MinWidth")) {
			minWidth = LuaToNumber(l, -1);
		} else if (!strcmp(key, "MinHeight")) {
			minHeight = LuaToNumber(l, -1);
		} else if (!strcmp(key, "More")) {
			Assert(lua_istable(l, -1));
			lua_rawgeti(l, -1, 1); // Method name
			lua_rawgeti(l, -2, 2); // Method data
			key = LuaToString(l, -2);
			if (!strcmp(key, "ButtonInfo")) {
				content = new CPopupContentTypeButtonInfo;
			} else if (!strcmp(key, "Text")) {
				content = new CPopupContentTypeText;
			} else if (!strcmp(key, "Costs")) {
				content = new CPopupContentTypeCosts;
			} else if (!strcmp(key, "Line")) {
				content = new CPopupContentTypeLine;
			} else if (!strcmp(key, "Variable")) {
				content = new CPopupContentTypeVariable;
			} else {
				LuaError(l, "Invalid drawing method '%s' in DefinePopups" _C_ key);
			}
			content->Parse(l);
			lua_pop(l, 2); // Pop Variable Name and Method
		} else if (!strcmp(key, "Condition")) {
			condition = ParsePopupConditions(l);
		} else {
			LuaError(l, "'%s' invalid for Contents in DefinePopups" _C_ key);
		}
	}
	content->Wrap = wrap;
	content->MarginX = marginX;
	content->MarginY = marginY;
	content->minSize.x = minWidth;
	content->minSize.y = minHeight;
	content->Condition = condition;
	content->TextColor = textColor;
	content->HighlightColor = highColor;
	return content;
}

/**
**  Define the Panels.
**  Define what is shown in the panel(text, icon, variables)
**
**  @param l  Lua state.
**  @return   0.
*/
static int CclDefinePopup(lua_State *l)
{
	Assert(lua_istable(l, 1));

	CPopup *popup = new CPopup;

	for (lua_pushnil(l); lua_next(l, 1); lua_pop(l, 1)) {
		const char *key = LuaToString(l, -2);

		if (!strcmp(key, "Ident")) {
			popup->Ident = LuaToString(l, -1);
		} else if (!strcmp(key, "DefaultFont")) {
			popup->DefaultFont = CFont::Get(LuaToString(l, -1));
		} else if (!strcmp(key, "BackgroundColor")) {
			popup->BackgroundColor = LuaToNumber(l, -1);
		} else if (!strcmp(key, "BorderColor")) {
			popup->BorderColor = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Margin")) {
			CclGetPos(l, &popup->MarginX, &popup->MarginY);
		} else if (!strcmp(key, "MinWidth")) {
			popup->MinWidth = LuaToNumber(l, -1);
		} else if (!strcmp(key, "MinHeight")) {
			popup->MinHeight = LuaToNumber(l, -1);
		} else if (!strcmp(key, "Contents")) {
			Assert(lua_istable(l, -1));
			for (size_t j = 0; j < lua_rawlen(l, -1); j++, lua_pop(l, 1)) {
				lua_rawgeti(l, -1, j + 1);
				popup->Contents.push_back(CPopupContentType::ParsePopupContent(l));
			}
		} else {
			LuaError(l, "'%s' invalid for DefinePopups" _C_ key);
		}
	}
	for (size_t j = 0; j < UI.ButtonPopups.size(); ++j) {
		if (popup->Ident == UI.ButtonPopups[j]->Ident) {
			DebugPrint("Redefinition of Popup '%s'\n" _C_ popup->Ident.c_str());
			delete UI.ButtonPopups[j];
			UI.ButtonPopups[j] = popup;
			return 0;
		}
	}
	UI.ButtonPopups.push_back(popup);
	return 0;
}

/**
**  Define the viewports.
**
**  @param l  Lua state.
*/
static int CclDefineViewports(lua_State *l)
{
	int i = 0;
	const int args = lua_gettop(l);

	for (int j = 0; j < args; ++j) {
		const char *value = LuaToString(l, j + 1);
		++j;
		if (!strcmp(value, "mode")) {
			UI.ViewportMode = (ViewportModeType)LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "viewport")) {
			if (!lua_istable(l, j + 1) && lua_rawlen(l, j + 1) != 3) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, j + 1, 1);
			UI.Viewports[i].MapPos.x = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 2);
			UI.Viewports[i].MapPos.y = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 3);
			const int slot = LuaToNumber(l, -1);
			if (slot != -1) {
				UI.Viewports[i].Unit = &UnitManager.GetSlotUnit(slot);
			}
			lua_pop(l, 1);
			++i;
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	UI.NumViewports = i;
	return 0;
}

/**
**  Fighter right button attacks as default.
**
**  @param l  Lua state.
*/
static int CclRightButtonAttacks(lua_State *l)
{
	LuaCheckArgs(l, 0);
	RightButtonAttacks = true;
	return 0;
}

/**
**  Fighter right button moves as default.
**
**  @param l  Lua state.
*/
static int CclRightButtonMoves(lua_State *l)
{
	LuaCheckArgs(l, 0);
	RightButtonAttacks = false;
	return 0;
}

/**
**  Enable/disable the fancy buildings.
**
**  @param l  Lua state.
*/
static int CclSetFancyBuildings(lua_State *l)
{
	LuaCheckArgs(l, 1);
	FancyBuildings = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Find a button style
**
**  @param style  Name of the style to find.
**
**  @return       Button style, NULL if not found.
*/
ButtonStyle *FindButtonStyle(const std::string &style)
{
	return ButtonStyleHash[style];
}

/**
**  Parse button style properties
**
**  @param l  Lua state.
**  @param p  Properties to fill in.
*/
static void ParseButtonStyleProperties(lua_State *l, ButtonStyleProperties *p)
{
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	std::string file;
	int w = 0;
	int h = 0;

	lua_pushnil(l);
	while (lua_next(l, -2)) {
		const char *value = LuaToString(l, -2);
		if (!strcmp(value, "File")) {
			file = LuaToString(l, -1);
		} else if (!strcmp(value, "Size")) {
			CclGetPos(l, &w, &h);
		} else if (!strcmp(value, "Frame")) {
			p->Frame = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Border")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "Color")) {
					p->BorderColorRGB.Parse(l);
				} else if (!strcmp(value, "Size")) {
					p->BorderSize = LuaToNumber(l, -1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "TextPos")) {
			CclGetPos(l, &p->TextPos.x, &p->TextPos.y);
		} else if (!strcmp(value, "TextAlign")) {
			value = LuaToString(l, -1);
			if (!strcmp(value, "Center")) {
				p->TextAlign = TextAlignCenter;
			} else if (!strcmp(value, "Right")) {
				p->TextAlign = TextAlignRight;
			} else if (!strcmp(value, "Left")) {
				p->TextAlign = TextAlignLeft;
			} else {
				LuaError(l, "Invalid text alignment: %s" _C_ value);
			}
		} else if (!strcmp(value, "TextNormalColor")) {
			p->TextNormalColor = LuaToString(l, -1);
		} else if (!strcmp(value, "TextReverseColor")) {
			p->TextReverseColor = LuaToString(l, -1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}

	if (!file.empty()) {
		p->Sprite = CGraphic::New(file, w, h);
	}
}

/**
**  Define a button style
**
**  @param l  Lua state.
*/
static int CclDefineButtonStyle(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}
	const char *style = LuaToString(l, 1);
	ButtonStyle *&b = ButtonStyleHash[style];
	if (!b) {
		b = new ButtonStyle;
		// Set to bogus value to see if it was set later
		b->Default.TextPos.x = b->Hover.TextPos.x = b->Clicked.TextPos.x = 0xFFFFFF;
	}

	lua_pushnil(l);
	while (lua_next(l, 2)) {
		const char *value = LuaToString(l, -2);

		if (!strcmp(value, "Size")) {
			CclGetPos(l, &b->Width, &b->Height);
		} else if (!strcmp(value, "Font")) {
			b->Font = CFont::Get(LuaToString(l, -1));
		} else if (!strcmp(value, "TextNormalColor")) {
			b->TextNormalColor = LuaToString(l, -1);
		} else if (!strcmp(value, "TextReverseColor")) {
			b->TextReverseColor = LuaToString(l, -1);
		} else if (!strcmp(value, "TextPos")) {
			CclGetPos(l, &b->TextX, &b->TextY);
		} else if (!strcmp(value, "TextAlign")) {
			value = LuaToString(l, -1);
			if (!strcmp(value, "Center")) {
				b->TextAlign = TextAlignCenter;
			} else if (!strcmp(value, "Right")) {
				b->TextAlign = TextAlignRight;
			} else if (!strcmp(value, "Left")) {
				b->TextAlign = TextAlignLeft;
			} else {
				LuaError(l, "Invalid text alignment: %s" _C_ value);
			}
		} else if (!strcmp(value, "Default")) {
			ParseButtonStyleProperties(l, &b->Default);
		} else if (!strcmp(value, "Hover")) {
			ParseButtonStyleProperties(l, &b->Hover);
		} else if (!strcmp(value, "Clicked")) {
			ParseButtonStyleProperties(l, &b->Clicked);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}

	if (b->Default.TextPos.x == 0xFFFFFF) {
		b->Default.TextPos.x = b->TextX;
		b->Default.TextPos.y = b->TextY;
	}
	if (b->Hover.TextPos.x == 0xFFFFFF) {
		b->Hover.TextPos.x = b->TextX;
		b->Hover.TextPos.y = b->TextY;
	}
	if (b->Clicked.TextPos.x == 0xFFFFFF) {
		b->Clicked.TextPos.x = b->TextX;
		b->Clicked.TextPos.y = b->TextY;
	}

	if (b->Default.TextAlign == TextAlignUndefined) {
		b->Default.TextAlign = b->TextAlign;
	}
	if (b->Hover.TextAlign == TextAlignUndefined) {
		b->Hover.TextAlign = b->TextAlign;
	}
	if (b->Clicked.TextAlign == TextAlignUndefined) {
		b->Clicked.TextAlign = b->TextAlign;
	}
	return 0;
}

/**
**  Add a Lua handler
**  FIXME: when should these be freed?
*/
int AddHandler(lua_State *l)
{
	lua_getglobal(l, "_handlers_");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_setglobal(l, "_handlers_");
		lua_getglobal(l, "_handlers_");
	}
	lua_pushvalue(l, -2);
	lua_rawseti(l, -2, HandleCount);
	lua_pop(l, 1);

	return HandleCount++;
}

/**
**  Call a Lua handler
*/
void CallHandler(unsigned int handle, int value)
{
	lua_getglobal(Lua, "_handlers_");
	lua_rawgeti(Lua, -1, handle);
	lua_pushnumber(Lua, value);
	LuaCall(1, 1);
	lua_pop(Lua, 1);
}

/**
**  Define a button.
**
**  @param l  Lua state.
*/
static int CclDefineButton(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	ButtonAction ba;

	//
	// Parse the arguments
	//
	lua_pushnil(l);
	while (lua_next(l, 1)) {
		const char *value = LuaToString(l, -2);
		if (!strcmp(value, "Pos")) {
			ba.Pos = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Level")) {
			ba.Level = LuaToNumber(l, -1);
		} else if (!strcmp(value, "AlwaysShow")) {
			ba.AlwaysShow = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Icon")) {
			ba.Icon.Name = LuaToString(l, -1);
		} else if (!strcmp(value, "Action")) {
			value = LuaToString(l, -1);
			if (!strcmp(value, "move")) {
				ba.Action = ButtonMove;
			} else if (!strcmp(value, "stop")) {
				ba.Action = ButtonStop;
			} else if (!strcmp(value, "attack")) {
				ba.Action = ButtonAttack;
			} else if (!strcmp(value, "repair")) {
				ba.Action = ButtonRepair;
			} else if (!strcmp(value, "harvest")) {
				ba.Action = ButtonHarvest;
			} else if (!strcmp(value, "button")) {
				ba.Action = ButtonButton;
			} else if (!strcmp(value, "build")) {
				ba.Action = ButtonBuild;
			} else if (!strcmp(value, "train-unit")) {
				ba.Action = ButtonTrain;
			} else if (!strcmp(value, "patrol")) {
				ba.Action = ButtonPatrol;
			} else if (!strcmp(value, "stand-ground")) {
				ba.Action = ButtonStandGround;
			} else if (!strcmp(value, "attack-ground")) {
				ba.Action = ButtonAttackGround;
			} else if (!strcmp(value, "return-goods")) {
				ba.Action = ButtonReturn;
			} else if (!strcmp(value, "cast-spell")) {
				ba.Action = ButtonSpellCast;
			} else if (!strcmp(value, "research")) {
				ba.Action = ButtonResearch;
			} else if (!strcmp(value, "upgrade-to")) {
				ba.Action = ButtonUpgradeTo;
			} else if (!strcmp(value, "unload")) {
				ba.Action = ButtonUnload;
			} else if (!strcmp(value, "cancel")) {
				ba.Action = ButtonCancel;
			} else if (!strcmp(value, "cancel-upgrade")) {
				ba.Action = ButtonCancelUpgrade;
			} else if (!strcmp(value, "cancel-train-unit")) {
				ba.Action = ButtonCancelTrain;
			} else if (!strcmp(value, "cancel-build")) {
				ba.Action = ButtonCancelBuild;
			} else {
				LuaError(l, "Unsupported button action: %s" _C_ value);
			}
		} else if (!strcmp(value, "Value")) {
			if (!lua_isnumber(l, -1) && !lua_isstring(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			char buf[64];
			const char *s2;

			if (lua_isnumber(l, -1)) {
				snprintf(buf, sizeof(buf), "%ld", (long int)lua_tonumber(l, -1));
				s2 = buf;
			} else {
				s2 = lua_tostring(l, -1);
			}
			ba.ValueStr = s2;
		} else if (!strcmp(value, "Allowed")) {
			value = LuaToString(l, -1);
			if (!strcmp(value, "check-true")) {
				ba.Allowed = ButtonCheckTrue;
			} else if (!strcmp(value, "check-false")) {
				ba.Allowed = ButtonCheckFalse;
			} else if (!strcmp(value, "check-upgrade")) {
				ba.Allowed = ButtonCheckUpgrade;
			} else if (!strcmp(value, "check-unit-variable")) {
				ba.Allowed = ButtonCheckUnitVariable;
			} else if (!strcmp(value, "check-units-or")) {
				ba.Allowed = ButtonCheckUnitsOr;
			} else if (!strcmp(value, "check-units-and")) {
				ba.Allowed = ButtonCheckUnitsAnd;
			} else if (!strcmp(value, "check-network")) {
				ba.Allowed = ButtonCheckNetwork;
			} else if (!strcmp(value, "check-no-network")) {
				ba.Allowed = ButtonCheckNoNetwork;
			} else if (!strcmp(value, "check-no-work")) {
				ba.Allowed = ButtonCheckNoWork;
			} else if (!strcmp(value, "check-no-research")) {
				ba.Allowed = ButtonCheckNoResearch;
			} else if (!strcmp(value, "check-attack")) {
				ba.Allowed = ButtonCheckAttack;
			} else if (!strcmp(value, "check-upgrade-to")) {
				ba.Allowed = ButtonCheckUpgradeTo;
			} else if (!strcmp(value, "check-research")) {
				ba.Allowed = ButtonCheckResearch;
			} else if (!strcmp(value, "check-single-research")) {
				ba.Allowed = ButtonCheckSingleResearch;
			} else {
				LuaError(l, "Unsupported action: %s" _C_ value);
			}
		} else if (!strcmp(value, "AllowArg")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			std::string allowstr;
			const unsigned int subargs = lua_rawlen(l, -1);

			for (unsigned int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				const char *s2 = LuaToString(l, -1);
				lua_pop(l, 1);
				allowstr += s2;
				if (k != subargs - 1) {
					allowstr += ",";
				}
			}
			ba.AllowStr = allowstr;
		} else if (!strcmp(value, "Key")) {
			ba.Key = *LuaToString(l, -1);
		} else if (!strcmp(value, "Hint")) {
			ba.Hint = LuaToString(l, -1);
		} else if (!strcmp(value, "Description")) {
			ba.Description = LuaToString(l, -1);
		} else if (!strcmp(value, "CommentSound")) {
			ba.CommentSound.Name = LuaToString(l, -1);
		} else if (!strcmp(value, "ButtonCursor")) {
			ba.ButtonCursor = LuaToString(l, -1);
		} else if (!strcmp(value, "Popup")) {
			ba.Popup = LuaToString(l, -1);
		} else if (!strcmp(value, "ForUnit")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			// FIXME: ba.UnitMask shouldn't be a string
			std::string umask = ",";
			const unsigned subargs = lua_rawlen(l, -1);
			for (unsigned int k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				const char *s2 = LuaToString(l, -1);
				lua_pop(l, 1);
				umask += s2;
				umask += ",";
			}
			ba.UnitMask = umask;
			if (!strncmp(ba.UnitMask.c_str(), ",*,", 3)) {
				ba.UnitMask = "*";
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}
	AddButton(ba.Pos, ba.Level, ba.Icon.Name, ba.Action, ba.ValueStr,
			  ba.Allowed, ba.AllowStr, /*ba.Key,*/ ba.Hint, ba.Description, ba.CommentSound.Name,
			  ba.ButtonCursor, ba.UnitMask, ba.Popup, ba.AlwaysShow);
	return 0;
}

/**
**  Run the set-selection-changed-hook.
*/
void SelectionChanged()
{
	// We Changed out selection, anything pending buttonwise must be cleared
	UI.StatusLine.Clear();
	ClearCosts();
	CurrentButtonLevel = 0;
	UI.ButtonPanel.Update();
	GameCursor = UI.Point.Cursor;
	CursorBuilding = NULL;
	CursorState = CursorStatePoint;
	UI.ButtonPanel.Update();
}

/**
**  The selected unit has been altered.
*/
void SelectedUnitChanged()
{
	UI.ButtonPanel.Update();
}

/**
**  Set selection style.
**
**  @param l  Lua state.
*/
static int CclSetSelectionStyle(lua_State *l)
{
	LuaCheckArgs(l, 1);

	const char *style = LuaToString(l, 1);
	if (!strcmp(style, "rectangle")) {
		DrawSelection = DrawSelectionRectangle;
	} else if (!strcmp(style, "alpha-rectangle")) {
		DrawSelection = DrawSelectionRectangleWithTrans;
	} else if (!strcmp(style, "circle")) {
		DrawSelection = DrawSelectionCircle;
	} else if (!strcmp(style, "alpha-circle")) {
		DrawSelection = DrawSelectionCircleWithTrans;
	} else if (!strcmp(style, "corners")) {
		DrawSelection = DrawSelectionCorners;
	} else {
		LuaError(l, "Unsupported selection style");
	}
	return 0;
}

/**
**  Add a new message.
**
**  @param l  Lua state.
*/
static int CclAddMessage(lua_State *l)
{
	LuaCheckArgs(l, 1);
	SetMessage("%s", LuaToString(l, 1));
	return 0;
}

/**
**  Set the keys which are use for grouping units, helpful for other keyboards
**
**  @param l  Lua state.
*/
static int CclSetGroupKeys(lua_State *l)
{
	LuaCheckArgs(l, 1);
	UiGroupKeys = LuaToString(l, 1);
	return 0;
}

/**
** Set basic map caracteristics.
**
**  @param l  Lua state.
*/
static int CclPresentMap(lua_State *l)
{
	LuaCheckArgs(l, 5);

	Map.Info.Description = LuaToString(l, 1);
	// Number of players in LuaToNumber(l, 3); // Not used yet.
	Map.Info.MapWidth = LuaToNumber(l, 3);
	Map.Info.MapHeight = LuaToNumber(l, 4);
	Map.Info.MapUID = LuaToNumber(l, 5);

	return 0;
}

/**
** Define the lua file that will build the map
**
**  @param l  Lua state.
*/
static int CclDefineMapSetup(lua_State *l)
{
	LuaCheckArgs(l, 1);
	Map.Info.Filename = LuaToString(l, 1);

	return 0;
}

/**
**  Register CCL features for UI.
*/
void UserInterfaceCclRegister()
{
	CursorCclRegister();
	lua_register(Lua, "AddMessage", CclAddMessage);

	lua_register(Lua, "SetMouseScrollSpeed", CclSetMouseScrollSpeed);
	lua_register(Lua, "GetMouseScrollSpeed", CclGetMouseScrollSpeed);
	lua_register(Lua, "SetMouseScrollSpeedDefault", CclSetMouseScrollSpeedDefault);
	lua_register(Lua, "GetMouseScrollSpeedDefault", CclGetMouseScrollSpeedDefault);
	lua_register(Lua, "SetMouseScrollSpeedControl", CclSetMouseScrollSpeedControl);
	lua_register(Lua, "GetMouseScrollSpeedControl", CclGetMouseScrollSpeedControl);

	lua_register(Lua, "SetClickMissile", CclSetClickMissile);
	lua_register(Lua, "SetDamageMissile", CclSetDamageMissile);

	lua_register(Lua, "SetMaxOpenGLTexture", CclSetMaxOpenGLTexture);
	lua_register(Lua, "SetUseOpenGL", CclSetUseOpenGL);
	lua_register(Lua, "SetVideoResolution", CclSetVideoResolution);
	lua_register(Lua, "GetVideoResolution", CclGetVideoResolution);
	lua_register(Lua, "SetVideoFullScreen", CclSetVideoFullScreen);
	lua_register(Lua, "GetVideoFullScreen", CclGetVideoFullScreen);

	lua_register(Lua, "SetTitleScreens", CclSetTitleScreens);

	lua_register(Lua, "DefinePanelContents", CclDefinePanelContents);
	lua_register(Lua, "DefinePopup", CclDefinePopup);
	lua_register(Lua, "DefineViewports", CclDefineViewports);

	lua_register(Lua, "RightButtonAttacks", CclRightButtonAttacks);
	lua_register(Lua, "RightButtonMoves", CclRightButtonMoves);
	lua_register(Lua, "SetFancyBuildings", CclSetFancyBuildings);

	lua_register(Lua, "DefineButton", CclDefineButton);

	lua_register(Lua, "DefineButtonStyle", CclDefineButtonStyle);

	lua_register(Lua, "PresentMap", CclPresentMap);
	lua_register(Lua, "DefineMapSetup", CclDefineMapSetup);

	//
	// Look and feel of units
	//
	lua_register(Lua, "SetSelectionStyle", CclSetSelectionStyle);

	lua_register(Lua, "SetGroupKeys", CclSetGroupKeys);
}

//@}
