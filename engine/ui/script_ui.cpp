//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "script.h"
#include "interface.h"
#include "ui.h"
#include "video.h"
#include "map.h"
#include "menus.h"
#include "font.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"
#include "title.h"

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
**  Set which missile is used for right click
**
**  @param l  Lua state.
*/
static int CclSetClickMissile(lua_State *l)
{
	int args;

	args = lua_gettop(l);
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
	int args;

	args = lua_gettop(l);
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
	const char *value;
	int i;
	int args;
	int j;
	int subargs;
	int k;

	if (TitleScreens) {
		for (i = 0; TitleScreens[i]; ++i) {
			delete TitleScreens[i];
		}
		delete[] TitleScreens;
		TitleScreens = NULL;
	}

	args = lua_gettop(l);
	TitleScreens = new TitleScreen *[args + 1];
	memset(TitleScreens, 0, (args + 1) * sizeof(TitleScreen *));

	for (j = 0; j < args; ++j) {
		LuaCheckTable(l, j + 1);
		TitleScreens[j] = new TitleScreen;
		TitleScreens[j]->Iterations = 1;
		lua_pushnil(l);
		while (lua_next(l, j + 1)) {
			value = LuaToString(l, -2);
			if (!strcmp(value, "Image")) {
				TitleScreens[j]->File = LuaToString(l, -1);
			} else if (!strcmp(value, "Music")) {
				TitleScreens[j]->Music = LuaToString(l, -1);
			} else if (!strcmp(value, "StretchImage")) {
				TitleScreens[j]->StretchImage = LuaToBoolean(l, -1);
			} else if (!strcmp(value, "Timeout")) {
				TitleScreens[j]->Timeout = LuaToNumber(l, -1);
			} else if (!strcmp(value, "Iterations")) {
				TitleScreens[j]->Iterations = LuaToNumber(l, -1);
			} else if (!strcmp(value, "Labels")) {
				LuaCheckTable(l, -1);
				subargs = lua_objlen(l, -1);
				TitleScreens[j]->Labels = new TitleScreenLabel *[subargs + 1];
				memset(TitleScreens[j]->Labels, 0, (subargs + 1) * sizeof(TitleScreenLabel *));
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, -1, k + 1);
					LuaCheckTable(l, -1);
					TitleScreens[j]->Labels[k] = new TitleScreenLabel;
					lua_pushnil(l);
					while (lua_next(l, -2)) {
						value = LuaToString(l, -2);
						if (!strcmp(value, "Text")) {
							TitleScreens[j]->Labels[k]->Text = LuaToString(l, -1);
						} else if (!strcmp(value, "Font")) {
							TitleScreens[j]->Labels[k]->Font = CFont::Get(LuaToString(l, -1));
						} else if (!strcmp(value, "Pos")) {
							LuaCheckTableSize(l, -1, 2);
							TitleScreens[j]->Labels[k]->Xofs = LuaToNumber(l, -1, 1);
							TitleScreens[j]->Labels[k]->Yofs = LuaToNumber(l, -1, 2);
						} else if (!strcmp(value, "Flags")) {
							int subsubargs;
							int subk;

							LuaCheckTable(l, -1);
							subsubargs = lua_objlen(l, -1);
							for (subk = 0; subk < subsubargs; ++subk) {
								value = LuaToString(l, -1, subk + 1);
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
**  Define a cursor.
**
**  @param l  Lua state.
*/
static int CclDefineCursor(lua_State *l)
{
	const char *value;
	std::string name;
	std::string file;
	int hotx;
	int hoty;
	int w;
	int h;
	int rate;
	int i;
	CCursor *ct;

	LuaCheckArgs(l, 1);
	LuaCheckTable(l, 1);
	hotx = hoty = w = h = rate = 0;
	lua_pushnil(l);
	while (lua_next(l, 1)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Name")) {
			name = LuaToString(l, -1);
		} else if (!strcmp(value, "File")) {
			file = LuaToString(l, -1);
		} else if (!strcmp(value, "HotSpot")) {
			LuaCheckTableSize(l, -1, 2);
			hotx = LuaToNumber(l, -1, 1);
			hoty = LuaToNumber(l, -1, 2);
		} else if (!strcmp(value, "Size")) {
			LuaCheckTableSize(l, -1, 2);
			w = LuaToNumber(l, -1, 1);
			h = LuaToNumber(l, -1, 2);
		} else if (!strcmp(value, "Rate")) {
			rate = LuaToNumber(l, -1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}

	Assert(!name.empty() && !file.empty() && w && h);

	//
	//  Look if this kind of cursor already exists.
	//
	ct = NULL;
	i = 0;
	if (AllCursors.size()) {
		for (; i < (int)AllCursors.size(); ++i) {
			if (AllCursors[i].Ident == name) {
				ct = &AllCursors[i];
				break;
			}
		}
	}
	//
	//  Not found, make a new slot.
	//
	if (!ct) {
		CCursor c;
		AllCursors.push_back(c);
		ct = &AllCursors.back();
		ct->Ident = name;
	}

	ct->G = CGraphic::New(file, w, h);
	ct->HotX = hotx;
	ct->HotY = hoty;
	ct->FrameRate = rate;

	return 0;
}

/**
**  Set the current game cursor.
**
**  @param l  Lua state.
*/
static int CclSetGameCursor(lua_State *l)
{
	LuaCheckArgs(l, 1);
	GameCursor = CursorByIdent(LuaToString(l, 1));
	return 0;
}

/**
**  Define the viewports.
**
**  @param l  Lua state.
*/
static int CclDefineViewports(lua_State *l)
{
	const char *value;
	int i;
	int args;
	int slot;

	i = 0;
	args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;
		if (!strcmp(value, "mode")) {
			UI.ViewportMode = (ViewportModeType)(int)LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "viewport")) {
			LuaCheckTableSize(l, j + 1, 3);
			UI.Viewports[i].MapX = LuaToNumber(l, j + 1, 1);
			UI.Viewports[i].MapY = LuaToNumber(l, j + 1, 2);
			slot = (int)LuaToNumber(l, j + 1, 3);
			if (slot != -1) {
				UI.Viewports[i].Unit = UnitSlots[slot];
			}
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
	const char *value;
	std::string file;
	int w;
	int h;

	LuaCheckTable(l, -1);

	w = h = 0;

	lua_pushnil(l);
	while (lua_next(l, -2)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "File")) {
			file = LuaToString(l, -1);
		} else if (!strcmp(value, "Size")) {
			LuaCheckTableSize(l, -1, 2);
			w = LuaToNumber(l, -1, 1);
			h = LuaToNumber(l, -1, 2);
		} else if (!strcmp(value, "Frame")) {
			p->Frame = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Border")) {
			LuaCheckTable(l, -1);
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "Color")) {
					LuaCheckTableSize(l, -1, 3);
					p->BorderColorRGB.r = LuaToNumber(l, -1, 1);
					p->BorderColorRGB.g = LuaToNumber(l, -1, 2);
					p->BorderColorRGB.b = LuaToNumber(l, -1, 3);
				} else if (!strcmp(value, "Size")) {
					p->BorderSize = LuaToNumber(l, -1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "TextPos")) {
			LuaCheckTableSize(l, -1, 2);
			p->TextX = LuaToNumber(l, -1, 1);
			p->TextY = LuaToNumber(l, -1, 2);
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
	const char *style;
	const char *value;
	ButtonStyle *b;

	LuaCheckArgs(l, 2);
	LuaCheckTable(l, 2);

	style = LuaToString(l, 1);
	b = ButtonStyleHash[style];
	if (!b) {
		b = ButtonStyleHash[style] = new ButtonStyle;
		// Set to bogus value to see if it was set later
		b->Default.TextX = b->Hover.TextX = b->Clicked.TextX = 0xFFFFFF;
	}

	lua_pushnil(l);
	while (lua_next(l, 2)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Size")) {
			LuaCheckTableSize(l, -1, 2);
			b->Width = LuaToNumber(l, -1, 1);
			b->Height = LuaToNumber(l, -1, 2);
		} else if (!strcmp(value, "Font")) {
			b->Font = CFont::Get(LuaToString(l, -1));
		} else if (!strcmp(value, "TextNormalColor")) {
			b->TextNormalColor = LuaToString(l, -1);
		} else if (!strcmp(value, "TextReverseColor")) {
			b->TextReverseColor = LuaToString(l, -1);
		} else if (!strcmp(value, "TextPos")) {
			LuaCheckTableSize(l, -1, 2);
			b->TextX = LuaToNumber(l, -1, 1);
			b->TextY = LuaToNumber(l, -1, 2);
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

	if (b->Default.TextX == 0xFFFFFF) {
		b->Default.TextX = b->TextX;
		b->Default.TextY = b->TextY;
	}
	if (b->Hover.TextX == 0xFFFFFF) {
		b->Hover.TextX = b->TextX;
		b->Hover.TextY = b->TextY;
	}
	if (b->Clicked.TextX == 0xFFFFFF) {
		b->Clicked.TextX = b->TextX;
		b->Clicked.TextY = b->TextY;
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
	lua_pushstring(l, "_handlers_");
	lua_gettable(l, LUA_GLOBALSINDEX);
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_pushstring(l, "_handlers_");
		lua_newtable(l);
		lua_settable(l, LUA_GLOBALSINDEX);
		lua_pushstring(l, "_handlers_");
		lua_gettable(l, LUA_GLOBALSINDEX);
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
	lua_pushstring(Lua, "_handlers_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
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
	char buf[64];
	const char *value;
	const char *s2;
	ButtonAction ba;

	LuaCheckArgs(l, 1);
	LuaCheckTable(l, 1);

	//
	// Parse the arguments
	//
	lua_pushnil(l);
	while (lua_next(l, 1)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Pos")) {
			ba.Pos = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Level")) {
			ba.Level = LuaToNumber(l, -1);
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
			} else if (!strcmp(value, "cast-spell")) {
				ba.Action = ButtonSpellCast;
			} else if (!strcmp(value, "unload")) {
				ba.Action = ButtonUnload;
			} else if (!strcmp(value, "cancel")) {
				ba.Action = ButtonCancel;
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
			if (lua_isnumber(l, -1)) {
				sprintf_s(buf, sizeof(buf), "%ld", (long int)lua_tonumber(l, -1));
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
			} else if (!strcmp(value, "check-attack")) {
				ba.Allowed = ButtonCheckAttack;
			} else {
				LuaError(l, "Unsupported action: %s" _C_ value);
			}
		} else if (!strcmp(value, "AllowArg")) {
			int subargs;
			int k;
			std::string allowstr;

			LuaCheckTable(l, -1);
			subargs = lua_objlen(l, -1);
			for (k = 0; k < subargs; ++k) {
				s2 = LuaToString(l, -1, k + 1);
				allowstr += s2;
				if (k != subargs - 1) {
					allowstr += ",";
				}
			}
			ba.AllowStr = allowstr;
		} else if (!strcmp(value, "Hint")) {
			ba.Hint = LuaToString(l, -1);
		} else if (!strcmp(value, "ForUnit")) {
			int subargs;
			int k;
			std::string umask;

			LuaCheckTable(l, -1);
			// FIXME: ba.UnitMask shouldn't be a string
			umask = ",";
			subargs = lua_objlen(l, -1);
			for (k = 0; k < subargs; ++k) {
				s2 = LuaToString(l, -1, k + 1);
				umask += s2;
				umask += ",";
			}
			ba.UnitMask = umask;
			if (ba.UnitMask.compare(0, 3, ",*,") == 0) {
				ba.UnitMask = "*";
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}
	AddButton(ba.Pos, ba.Level, ba.Icon.Name, ba.Action, ba.ValueStr,
		ba.Allowed, ba.AllowStr, ba.Hint, ba.UnitMask);

	return 0;
}

/**
**  Run the set-selection-changed-hook.
*/
void SelectionChanged(void)
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
void SelectedUnitChanged(void)
{
	UI.ButtonPanel.Update();
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
	// Number of players in LuaToNumber(l, 2); // Not used yet.
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
void UserInterfaceCclRegister(void)
{
	lua_register(Lua, "AddMessage", CclAddMessage);

	lua_register(Lua, "SetMouseScrollSpeedDefault", CclSetMouseScrollSpeedDefault);
	lua_register(Lua, "SetMouseScrollSpeedControl", CclSetMouseScrollSpeedControl);

	lua_register(Lua, "SetClickMissile", CclSetClickMissile);
	lua_register(Lua, "SetDamageMissile", CclSetDamageMissile);

	lua_register(Lua, "SetMaxOpenGLTexture", CclSetMaxOpenGLTexture);
	lua_register(Lua, "SetVideoResolution", CclSetVideoResolution);
	lua_register(Lua, "GetVideoResolution", CclGetVideoResolution);
	lua_register(Lua, "SetVideoFullScreen", CclSetVideoFullScreen);
	lua_register(Lua, "GetVideoFullScreen", CclGetVideoFullScreen);

	lua_register(Lua, "SetTitleScreens", CclSetTitleScreens);

	lua_register(Lua, "DefineCursor", CclDefineCursor);
	lua_register(Lua, "SetGameCursor", CclSetGameCursor);
	lua_register(Lua, "DefineViewports", CclDefineViewports);

	lua_register(Lua, "RightButtonAttacks", CclRightButtonAttacks);
	lua_register(Lua, "RightButtonMoves", CclRightButtonMoves);

	lua_register(Lua, "DefineButton", CclDefineButton);

	lua_register(Lua, "DefineButtonStyle", CclDefineButtonStyle);

	lua_register(Lua, "PresentMap", CclPresentMap);
	lua_register(Lua, "DefineMapSetup", CclDefineMapSetup);

	lua_register(Lua, "SetGroupKeys", CclSetGroupKeys);
}

//@}
