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
//      (c) Copyright 1999-2015 by Lutz Sammer, Jimmy Salmon, Martin Renold
//      and Andrettin
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
#include "ui/contenttype.h"
#include "ui/popup.h"
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
** <b>Description</b>
**
**  Set speed of key scroll
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>SetKeyScrollSpeed</strong>(4)</code></div>
*/
static int CclSetKeyScrollSpeed(lua_State *l)
{
	LuaCheckArgs(l, 1);
	UI.KeyScrollSpeed = LuaToNumber(l, 1);
	return 0;
}

/**
** <b>Description</b>
**
**  Get speed of key scroll
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>scroll_speed = <strong>GetKeyScrollSpeed</strong>()
**		print(scroll_speed)</code></div>
*/
static int CclGetKeyScrollSpeed(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushnumber(l, UI.KeyScrollSpeed);
	return 1;
}

/**
** <b>Description</b>
**
**  Set speed of mouse scroll
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>SetMouseScrollSpeed</strong>(2)</code></div>
*/
static int CclSetMouseScrollSpeed(lua_State *l)
{
	LuaCheckArgs(l, 1);
	UI.MouseScrollSpeed = LuaToNumber(l, 1);
	return 0;
}

/**
** <b>Description</b>
**
**  Get speed of mouse scroll
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>scroll_speed = <strong>GetMouseScrollSpeed</strong>()
**		print(scroll_speed)</code></div>
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

/**
** <b>Description</b>
**
**  Set the video resolution.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>SetVideoResolution</strong>(640,480)</code></div>
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
** <b>Description</b>
**
**  Get the video resolution.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>width,height = <strong>GetVideoResolution</strong>()
**		print("Resolution  is " .. width .. "x" .. height)</code></div>
*/
static int CclGetVideoResolution(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushnumber(l, Video.Width);
	lua_pushnumber(l, Video.Height);
	return 2;
}

/**
** <b>Description</b>
**
**  Set the video fullscreen mode.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>-- Full Screen mode enabled
**		<strong>SetVideoFullScreen</strong>(true)
**		-- Full Screen mode disabled
**		<strong>SetVideoFullScreen</strong>(false)</code></div>
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
** <b>Description</b>
**
**  Get the video fullscreen mode.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>fullscreenmode = <strong>GetVideoFullScreen</strong>()
**		print(fullscreenmode)</code></div>
*/
static int CclGetVideoFullScreen(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushboolean(l, Video.FullScreen);
	return 1;
}

/**
** Request a specific initial window size
*/
static int CclSetWindowSize(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (CclInConfigFile) {
		// May have been set from the command line
		if (!Video.WindowWidth || !Video.WindowHeight) {
			Video.WindowWidth = LuaToNumber(l, 1);
			Video.WindowHeight = LuaToNumber(l, 2);
		}
	}
	return 0;
}

/**
** For games with non-square pixels, this sets the scale of vertical pixels versus horizontal pixels.
** e.g., if your assets are 320x200, but you render at 320x240, this is 1.2.
*/
static int CclSetVerticalPixelSize(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (CclInConfigFile) {
		luaL_checktype(l, 1, LUA_TNUMBER);
		Video.VerticalPixelSize = static_cast<double>(lua_tonumber(l, 1));
	}
	return 0;
}

static int CclShowTitleScreens(lua_State *l)
{
	LuaCheckArgs(l, 0);
	ShowTitleScreens();
	lua_pushboolean(l, 1);
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
		TitleScreens = nullptr;
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
			const std::string_view value = LuaToString(l, -2);
			if (value == "Image") {
				TitleScreens[j]->File = LuaToString(l, -1);
			} else if (value == "Music") {
				TitleScreens[j]->Music = LuaToString(l, -1);
			} else if (value == "Timeout") {
				TitleScreens[j]->Timeout = LuaToNumber(l, -1);
			} else if (value == "Iterations") {
				TitleScreens[j]->Iterations = LuaToNumber(l, -1);
			} else if (value == "Editor") {
				TitleScreens[j]->Editor = LuaToNumber(l, -1);
			} else if (value == "Labels") {
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
						const std::string_view value = LuaToString(l, -2);
						if (value == "Text") {
							TitleScreens[j]->Labels[k]->Text = LuaToString(l, -1);
						} else if (value == "Font") {
							TitleScreens[j]->Labels[k]->Font = CFont::Get(LuaToString(l, -1));
						} else if (value == "Pos") {
							CclGetPos(l, &TitleScreens[j]->Labels[k]->Xofs, &TitleScreens[j]->Labels[k]->Yofs);
						} else if (value == "Flags") {
							if (!lua_istable(l, -1)) {
								LuaError(l, "incorrect argument");
							}
							const int subsubargs = lua_rawlen(l, -1);
							for (int subk = 0; subk < subsubargs; ++subk) {
								const std::string_view value = LuaToString(l, -1, subk + 1);
								if (value == "center") {
									TitleScreens[j]->Labels[k]->Flags |= TitleFlagCenter;
								} else {
									LuaError(l, "incorrect flag");
								}
							}
						} else {
							LuaError(l, "Unsupported key: %s" _C_ value.data());
						}
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				}
			} else {
				LuaError(l, "Unsupported key: %s" _C_ value.data());
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
EnumVariable Str2EnumVariable(lua_State *l, std::string_view s)
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
		if (s == list[i].s) {
			return list[i].e;
		}
	}
	LuaError(l, "'%s' is a invalid variable component" _C_ s.data());
	return VariableValue;
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
		const std::string_view key = LuaToString(l, -2);
		if (key == "ShowOnlySelected") {
			condition->ShowOnlySelected = LuaToBoolean(l, -1);
		} else if (key == "HideNeutral") {
			condition->HideNeutral = LuaToBoolean(l, -1);
		} else if (key == "HideAllied") {
			condition->HideAllied = LuaToBoolean(l, -1);
		} else if (key == "ShowOpponent") {
			condition->ShowOpponent = LuaToBoolean(l, -1);
		} else {
			int index = UnitTypeVar.BoolFlagNameLookup[key.data()];
			if (index != -1) {
				if (!condition->BoolFlags) {
					size_t new_bool_size = UnitTypeVar.GetNumberBoolFlag();
					condition->BoolFlags = new char[new_bool_size];
					memset(condition->BoolFlags, 0, new_bool_size * sizeof(char));
				}
				condition->BoolFlags[index] = Ccl2Condition(l, LuaToString(l, -1));
				continue;
			}
			index = UnitTypeVar.VariableNameLookup[key.data()];
			if (index != -1) {
				if (!condition->Variables) {
					size_t new_variables_size = UnitTypeVar.GetNumberVariable();
					condition->Variables = new char[new_variables_size];
					memset(condition->Variables, 0, new_variables_size * sizeof(char));
				}
				condition->Variables[index] = Ccl2Condition(l, LuaToString(l, -1));
				continue;
			}
			LuaError(l, "'%s' invalid for Condition in DefinePanelContents" _C_ key.data());
		}
	}
	return condition;
}

static CContentType *CclParseContent(lua_State *l)
{
	Assert(lua_istable(l, -1));

	CContentType *content = nullptr;
	ConditionPanel *condition = nullptr;
	PixelPos pos(0, 0);

	for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {
		std::string_view key = LuaToString(l, -2);
		if (key == "Pos") {
			CclGetPos(l, &pos.x, &pos.y);
		} else if (key == "More") {
			Assert(lua_istable(l, -1));
			lua_rawgeti(l, -1, 1); // Method name
			lua_rawgeti(l, -2, 2); // Method data
			key = LuaToString(l, -2);
			if (key == "Text") {
				content = new CContentTypeText;
			} else if (key == "FormattedText") {
				content = new CContentTypeFormattedText;
			} else if (key == "FormattedText2") {
				content = new CContentTypeFormattedText2;
			} else if (key == "Icon") {
				content = new CContentTypeIcon;
			} else if (key == "Graphic") {
				content = new CContentTypeGraphic;
			} else if (key == "LifeBar") {
				content = new CContentTypeLifeBar;
			} else if (key == "CompleteBar") {
				content = new CContentTypeCompleteBar;
			} else {
				LuaError(l, "Invalid drawing method '%s' in DefinePanelContents" _C_ key.data());
			}
			content->Parse(l);
			lua_pop(l, 2); // Pop Variable Name and Method
		} else if (key == "Condition") {
			condition = ParseConditionPanel(l);
		} else {
			LuaError(l, "'%s' invalid for Contents in DefinePanelContents" _C_ key.data());
		}
	}
	content->Pos = pos;
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
			const std::string_view key = LuaToString(l, -2);

			if (key == "Ident") {
				infopanel->Name = LuaToString(l, -1);
			} else if (key == "Pos") {
				CclGetPos(l, &infopanel->PosX, &infopanel->PosY);
			} else if (key == "DefaultFont") {
				infopanel->DefaultFont = CFont::Get(LuaToString(l, -1));
			} else if (key == "Condition") {
				infopanel->Condition = ParseConditionPanel(l);
			} else if (key == "Contents") {
				Assert(lua_istable(l, -1));
				for (size_t j = 0; j < lua_rawlen(l, -1); j++, lua_pop(l, 1)) {
					lua_rawgeti(l, -1, j + 1);
					infopanel->Contents.push_back(CclParseContent(l));
				}
			} else {
				LuaError(l, "'%s' invalid for DefinePanelContents" _C_ key.data());
			}
		}
		for (CContentType *content : infopanel->Contents) { // Default value for invalid value.
			content->Pos.x += infopanel->PosX;
			content->Pos.y += infopanel->PosY;
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
		const std::string_view key = LuaToString(l, -2);

		if (key == "Ident") {
			popup->Ident = LuaToString(l, -1);
		} else if (key == "DefaultFont") {
			popup->DefaultFont = CFont::Get(LuaToString(l, -1));
		} else if (key == "BackgroundColor") {
			popup->BackgroundColor = LuaToUnsignedNumber(l, -1);
		} else if (key == "BorderColor") {
			popup->BorderColor = LuaToUnsignedNumber(l, -1);
		} else if (key == "Margin") {
			CclGetPos(l, &popup->MarginX, &popup->MarginY);
		} else if (key == "MinWidth") {
			popup->MinWidth = LuaToNumber(l, -1);
		} else if (key == "MinHeight") {
			popup->MinHeight = LuaToNumber(l, -1);
		} else if (key == "Contents") {
			Assert(lua_istable(l, -1));
			for (size_t j = 0; j < lua_rawlen(l, -1); j++, lua_pop(l, 1)) {
				lua_rawgeti(l, -1, j + 1);
				popup->Contents.push_back(CPopupContentType::ParsePopupContent(l));
			}
		} else {
			LuaError(l, "'%s' invalid for DefinePopups" _C_ key.data());
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
		const std::string_view value = LuaToString(l, j + 1);
		++j;
		if (value == "mode") {
			UI.ViewportMode = (ViewportModeType)LuaToNumber(l, j + 1);
		} else if (value == "viewport") {
			if (!lua_istable(l, j + 1) && lua_rawlen(l, j + 1) != 3) {
				LuaError(l, "incorrect argument");
			}
			UI.Viewports[i].MapPos.x = LuaToNumber(l, j + 1, 1);
			UI.Viewports[i].MapPos.y = LuaToNumber(l, j + 1, 2);
			const int slot = LuaToNumber(l, j + 1, 3);
			if (slot != -1) {
				UI.Viewports[i].Unit = &UnitManager->GetSlotUnit(slot);
			}
			++i;
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value.data());
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
** <b>Description</b>
**
**  Enable/disable the fancy buildings.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>-- Enable fancy buildings
**		  <strong>SetFancyBuildings</strong>(true)
**		  -- Disable fancy buildings
**		  <strong>SetFancyBuildings</strong>(false)</code></div>
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
**  @return       Button style, nullptr if not found.
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
		std::string_view value = LuaToString(l, -2);
		if (value == "File") {
			file = LuaToString(l, -1);
		} else if (value == "Size") {
			CclGetPos(l, &w, &h);
		} else if (value == "Frame") {
			p->Frame = LuaToNumber(l, -1);
		} else if (value == "Border") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				value = LuaToString(l, -2);
				if (value == "Color") {
					p->BorderColorRGB.Parse(l);
				} else if (value == "Size") {
					p->BorderSize = LuaToNumber(l, -1);
				} else if (value == "SolidColor") {
					p->BorderColorRGB.Parse(l);
					p->BorderColor = 1; // XXX: see uibuttons_proc.cpp#DrawUIButton
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value.data());
				}
				lua_pop(l, 1);
			}
		} else if (value == "TextPos") {
			CclGetPos(l, &p->TextPos.x, &p->TextPos.y);
		} else if (value == "TextAlign") {
			value = LuaToString(l, -1);
			if (value == "Center") {
				p->TextAlign = TextAlignCenter;
			} else if (value == "Right") {
				p->TextAlign = TextAlignRight;
			} else if (value == "Left") {
				p->TextAlign = TextAlignLeft;
			} else {
				LuaError(l, "Invalid text alignment: %s" _C_ value.data());
			}
		} else if (value == "TextNormalColor") {
			p->TextNormalColor = LuaToString(l, -1);
		} else if (value == "TextReverseColor") {
			p->TextReverseColor = LuaToString(l, -1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value.data());
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
		std::string_view value = LuaToString(l, -2);

		if (value == "Size") {
			CclGetPos(l, &b->Width, &b->Height);
		} else if (value == "Font") {
			b->Font = CFont::Get(LuaToString(l, -1));
		} else if (value == "TextNormalColor") {
			b->TextNormalColor = LuaToString(l, -1);
		} else if (value == "TextReverseColor") {
			b->TextReverseColor = LuaToString(l, -1);
		} else if (value == "TextPos") {
			CclGetPos(l, &b->TextX, &b->TextY);
		} else if (value == "TextAlign") {
			value = LuaToString(l, -1);
			if (value == "Center") {
				b->TextAlign = TextAlignCenter;
			} else if (value == "Right") {
				b->TextAlign = TextAlignRight;
			} else if (value == "Left") {
				b->TextAlign = TextAlignLeft;
			} else {
				LuaError(l, "Invalid text alignment: %s" _C_ value.data());
			}
		} else if (value == "Default") {
			ParseButtonStyleProperties(l, &b->Default);
		} else if (value == "Hover") {
			ParseButtonStyleProperties(l, &b->Hover);
		} else if (value == "Clicked") {
			ParseButtonStyleProperties(l, &b->Clicked);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value.data());
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
**  Clear all buttons
**
**  @param l  Lua state.
*/
static int CclClearButtons(lua_State *l)
{
	LuaCheckArgs(l, 0);
	CleanButtons();
	return 0;
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
		std::string_view value = LuaToString(l, -2);
		if (value == "Pos") {
			ba.Pos = LuaToNumber(l, -1);
		} else if (value == "Level") {
			ba.Level = LuaToNumber(l, -1);
		} else if (value == "AlwaysShow") {
			ba.AlwaysShow = LuaToBoolean(l, -1);
		} else if (value == "Icon") {
			ba.Icon.Name = LuaToString(l, -1);
		} else if (value == "Action") {
			value = LuaToString(l, -1);
			if (value == "move") {
				ba.Action = ButtonCmd::Move;
			} else if (value == "stop") {
				ba.Action = ButtonCmd::Stop;
			} else if (value == "attack") {
				ba.Action = ButtonCmd::Attack;
			} else if (value == "repair") {
				ba.Action = ButtonCmd::Repair;
			} else if (value == "harvest") {
				ba.Action = ButtonCmd::Harvest;
			} else if (value == "button") {
				ba.Action = ButtonCmd::Button;
			} else if (value == "build") {
				ba.Action = ButtonCmd::Build;
			} else if (value == "train-unit") {
				ba.Action = ButtonCmd::Train;
			} else if (value == "patrol") {
				ba.Action = ButtonCmd::Patrol;
			} else if (value == "explore") {
				ba.Action = ButtonCmd::Explore;
			} else if (value == "stand-ground") {
				ba.Action = ButtonCmd::StandGround;
			} else if (value == "attack-ground") {
				ba.Action = ButtonCmd::AttackGround;
			} else if (value == "return-goods") {
				ba.Action = ButtonCmd::Return;
			} else if (value == "cast-spell") {
				ba.Action = ButtonCmd::SpellCast;
			} else if (value == "research") {
				ba.Action = ButtonCmd::Research;
			} else if (value == "upgrade-to") {
				ba.Action = ButtonCmd::UpgradeTo;
			} else if (value == "unload") {
				ba.Action = ButtonCmd::Unload;
			} else if (value == "cancel") {
				ba.Action = ButtonCmd::Cancel;
			} else if (value == "cancel-upgrade") {
				ba.Action = ButtonCmd::CancelUpgrade;
			} else if (value == "cancel-train-unit") {
				ba.Action = ButtonCmd::CancelTrain;
			} else if (value == "cancel-build") {
				ba.Action = ButtonCmd::CancelBuild;
			} else if (value == "callback") {
				ba.Action = ButtonCmd::CallbackAction;
			} else {
				LuaError(l, "Unsupported button action: %s" _C_ value.data());
			}
		} else if (value == "Value") {
			if (!lua_isnumber(l, -1) && !lua_isstring(l, -1) && !lua_isfunction(l, -1)) {
				LuaError(l, "incorrect argument");
			}

			if (lua_isfunction(l, -1)) {
				ba.Payload = new LuaCallback(l, -1);
			} else {
				char buf[64];
				const char *s2;

				if (lua_isnumber(l, -1)) {
					snprintf(buf, sizeof(buf), "%ld", (long int)lua_tonumber(l, -1));
					s2 = buf;
				} else {
					s2 = lua_tostring(l, -1);
				}
				ba.ValueStr = s2;
			}
		} else if (value == "Allowed") {
			value = LuaToString(l, -1);
			if (value == "check-true") {
				ba.Allowed = ButtonCheckTrue;
			} else if (value == "check-false") {
				ba.Allowed = ButtonCheckFalse;
			} else if (value == "check-upgrade") {
				ba.Allowed = ButtonCheckUpgrade;
			} else if (value == "check-individual-upgrade") {
				ba.Allowed = ButtonCheckIndividualUpgrade;
			} else if (value == "check-unit-variable") {
				ba.Allowed = ButtonCheckUnitVariable;
			} else if (value == "check-units-or") {
				ba.Allowed = ButtonCheckUnitsOr;
			} else if (value == "check-units-and") {
				ba.Allowed = ButtonCheckUnitsAnd;
			} else if (value == "check-units-not") {
				ba.Allowed = ButtonCheckUnitsNot;
			} else if (value == "check-units-nor") {
				ba.Allowed = ButtonCheckUnitsNor;
			} else if (value == "check-network") {
				ba.Allowed = ButtonCheckNetwork;
			} else if (value == "check-no-network") {
				ba.Allowed = ButtonCheckNoNetwork;
			} else if (value == "check-no-work") {
				ba.Allowed = ButtonCheckNoWork;
			} else if (value == "check-no-research") {
				ba.Allowed = ButtonCheckNoResearch;
			} else if (value == "check-attack") {
				ba.Allowed = ButtonCheckAttack;
			} else if (value == "check-upgrade-to") {
				ba.Allowed = ButtonCheckUpgradeTo;
			} else if (value == "check-research") {
				ba.Allowed = ButtonCheckResearch;
			} else if (value == "check-single-research") {
				ba.Allowed = ButtonCheckSingleResearch;
			} else if (value == "check-debug") {
				ba.Allowed = ButtonCheckDebug;
			} else {
				LuaError(l, "Unsupported action: %s" _C_ value.data());
			}
		} else if (value == "AllowArg") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			std::string allowstr;
			const unsigned int subargs = lua_rawlen(l, -1);

			for (unsigned int k = 0; k < subargs; ++k) {
				const char *s2 = LuaToString(l, -1, k + 1);
				allowstr += s2;
				if (k != subargs - 1) {
					allowstr += ",";
				}
			}
			ba.AllowStr = allowstr;
		} else if (value == "Key") {
			std::string key(LuaToString(l, -1));
			ba.Key = GetHotKey(key);
		} else if (value == "Hint") {
			ba.Hint = LuaToString(l, -1);
		} else if (value == "Description") {
			ba.Description = LuaToString(l, -1);
		} else if (value == "CommentSound") {
			ba.CommentSound.Name = LuaToString(l, -1);
		} else if (value == "ButtonCursor") {
			ba.ButtonCursor = LuaToString(l, -1);
		} else if (value == "Popup") {
			ba.Popup = LuaToString(l, -1);
		} else if (value == "ForUnit") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			// FIXME: ba.UnitMask shouldn't be a string
			std::string umask = ",";
			const unsigned subargs = lua_rawlen(l, -1);
			for (unsigned int k = 0; k < subargs; ++k) {
				const char *s2 = LuaToString(l, -1, k + 1);
				umask += s2;
				umask += ",";
			}
			ba.UnitMask = umask;
			if (!strncmp(ba.UnitMask.c_str(), ",*,", 3)) {
				ba.UnitMask = "*";
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value.data());
		}
		lua_pop(l, 1);
	}
	AddButton(ba.Pos, ba.Level, ba.Icon.Name, ba.Action, ba.ValueStr, ba.Payload,
			  ba.Allowed, ba.AllowStr, ba.Key, ba.Hint, ba.Description, ba.CommentSound.Name,
			  ba.ButtonCursor, ba.UnitMask, ba.Popup, ba.AlwaysShow);
	return 0;
}

static int CclCopyButtonsForUnitType(lua_State *l)
{
	LuaCheckArgs(l, 2);

	// Slot identifier
	const std::string_view fromName = LuaToString(l, 1);
	CUnitType *from = UnitTypeByIdent(fromName);
	const std::string_view toName = LuaToString(l, 2);
	CUnitType *to = UnitTypeByIdent(toName);
	if (!to) {
		LuaError(l, "Unknown unit-type '%s'\n" _C_ toName.data());
	}
	if (!from) {
		LuaError(l, "Unknown unit-type '%s'\n" _C_ fromName.data());
	}

	for (auto btn : UnitButtonTable) {
		if (btn->UnitMask.find(fromName) != std::string::npos) {
			btn->UnitMask += toName;
			btn->UnitMask += ",";
		}
	}

	return 0;
}

/**
**  Run the set-selection-changed-hook.
*/
void SelectionChanged()
{
	// We Changed out selection, anything pending buttonwise must be cleared
	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
	CurrentButtonLevel = 0;
	LastDrawnButtonPopup = nullptr;

	UI.ButtonPanel.Update();
	GameCursor = UI.Point.Cursor;
	CursorBuilding = nullptr;
	CursorState = CursorStates::Point;
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
	if (lua_gettop(l) < 1) {
		LuaError(l, "incorrect argument");
	}

	const std::string_view style = LuaToString(l, 1);
	if (style == "rectangle") {
		LuaCheckArgs(l, 1);
		DrawSelection = DrawSelectionRectangle;
	} else if (style == "alpha-rectangle") {
		LuaCheckArgs(l, 1);
		DrawSelection = DrawSelectionRectangleWithTrans;
	} else if (style == "circle") {
		LuaCheckArgs(l, 1);
		DrawSelection = DrawSelectionCircle;
	} else if (style == "alpha-circle") {
		LuaCheckArgs(l, 1);
		DrawSelection = DrawSelectionCircleWithTrans;
	} else if (style == "corners") {
		LuaCheckArgs(l, 1);
		DrawSelection = DrawSelectionCorners;
	} else if (style == "ellipse") {
		LuaCheckArgs(l, 2);
		float factor = LuaToFloat(l, 2);
		DrawSelection = DrawSelectionEllipse(factor);
	} else {
		LuaError(l, "Unsupported selection style");
	}
	return 0;
}

/**
** <b>Description</b>
**
**  Add a new message.
**
** Example:
**
** <div class="example"><code><strong>AddMessage</strong>("Hello World!")</code></div>
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
** <b>Description</b>
**
** Set basic map caracteristics.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>PresentMap</strong>("Map description", 1, 128, 128, 17)</code></div>
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
** <b>Description</b>
**
** Define the lua file that will build the map
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>-- Load map setup from file
**		<strong>DefineMapSetup</strong>("Setup.sms")</code></div>
*/
static int CclDefineMapSetup(lua_State *l)
{
	LuaCheckArgs(l, 1);
	Map.Info.Filename = LuaToString(l, 1);

	return 0;
}
/**
** <b>Description</b>
**
** Declare which codepage the font files are in. Text is handled internally
** as UTF-8 everywhere, but the font rendering system uses graphics with 256
** symbols. Commonly, DOS and early Windows games used codepage 437 or 1252 for
** western European languages, or 866 for Russian and some other cyrillic
** writing systems. These are the only ones that are currently supported, but
** more can easily be added. All text is mapped into the codepage that is set
** for the font files. If the codepage is not one of the supported ones, or if
** something doesn't map (for example, some accented characters with codepage
** 866, or cyrillic letters with codepage 437), a simple "visual" mapping to
** 7-bit ASCII is used to at least print something that may be recognizable.
*/
static int CclSetFontCodePage(lua_State *l)
{
	LuaCheckArgs(l, 1);
	FontCodePage = LuaToNumber(l, 1);

	return 0;
}

/**
**  Register CCL features for UI.
*/
void UserInterfaceCclRegister()
{
	CursorCclRegister();
	lua_register(Lua, "AddMessage", CclAddMessage);

	lua_register(Lua, "SetKeyScrollSpeed", CclSetKeyScrollSpeed);
	lua_register(Lua, "GetKeyScrollSpeed", CclGetKeyScrollSpeed);
	lua_register(Lua, "SetMouseScrollSpeed", CclSetMouseScrollSpeed);
	lua_register(Lua, "GetMouseScrollSpeed", CclGetMouseScrollSpeed);
	lua_register(Lua, "SetMouseScrollSpeedDefault", CclSetMouseScrollSpeedDefault);
	lua_register(Lua, "GetMouseScrollSpeedDefault", CclGetMouseScrollSpeedDefault);
	lua_register(Lua, "SetMouseScrollSpeedControl", CclSetMouseScrollSpeedControl);
	lua_register(Lua, "GetMouseScrollSpeedControl", CclGetMouseScrollSpeedControl);

	lua_register(Lua, "SetClickMissile", CclSetClickMissile);
	lua_register(Lua, "SetDamageMissile", CclSetDamageMissile);

	lua_register(Lua, "SetVideoResolution", CclSetVideoResolution);
	lua_register(Lua, "GetVideoResolution", CclGetVideoResolution);
	lua_register(Lua, "SetVideoFullScreen", CclSetVideoFullScreen);
	lua_register(Lua, "GetVideoFullScreen", CclGetVideoFullScreen);
	lua_register(Lua, "SetWindowSize", CclSetWindowSize);
	lua_register(Lua, "SetVerticalPixelSize", CclSetVerticalPixelSize);

	lua_register(Lua, "SetFontCodePage", CclSetFontCodePage);

	lua_register(Lua, "SetTitleScreens", CclSetTitleScreens);
	lua_register(Lua, "ShowTitleScreens", CclShowTitleScreens);

	lua_register(Lua, "DefinePanelContents", CclDefinePanelContents);
	lua_register(Lua, "DefinePopup", CclDefinePopup);
	lua_register(Lua, "DefineViewports", CclDefineViewports);

	lua_register(Lua, "RightButtonAttacks", CclRightButtonAttacks);
	lua_register(Lua, "RightButtonMoves", CclRightButtonMoves);
	lua_register(Lua, "SetFancyBuildings", CclSetFancyBuildings);

	lua_register(Lua, "DefineButton", CclDefineButton);
	lua_register(Lua, "ClearButtons", CclClearButtons);

	lua_register(Lua, "CopyButtonsForUnitType", CclCopyButtonsForUnitType);

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
