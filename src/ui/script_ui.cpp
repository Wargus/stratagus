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
/**@name script_ui.c - The ui ccl functions. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer, Jimmy Salmon, Martin Renold
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
#include "util.h"
#include "unit.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/


char* ClickMissile;              ///< FIXME:docu
char* DamageMissile;             ///< FIXME:docu

typedef struct _info_text_ {
	char* Text;                  ///< FIXME:docu
	int Font;                    ///< FIXME:docu
	int X;                       ///< FIXME:docu
	int Y;                       ///< FIXME:docu
} InfoText;                      ///< FIXME:docu

_ButtonStyleHash ButtonStyleHash;
_CheckboxStyleHash CheckboxStyleHash;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


/**
**  Enable/disable the global color cycling.
**
**  @param l  Lua state.
**
**  @return   The old state of color cylce all.
*/
static int CclSetColorCycleAll(lua_State* l)
{
	lua_Number old;

	if (lua_gettop(l) != 1 || (!lua_isnumber(l, 1) && !lua_isboolean(l, 1))) {
		LuaError(l, "incorrect argument");
	}
	old = ColorCycleAll;
	if (lua_isnumber(l, 1)) {
		ColorCycleAll = lua_tonumber(l, 1);
	} else {
		ColorCycleAll = lua_toboolean(l, 1);
	}

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Set speed of middle-mouse scroll
**
**  @param l  Lua state.
**
**  @return   The old value.
*/
static int CclSetMouseScrollSpeedDefault(lua_State* l)
{
	lua_Number old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = TheUI.MouseScrollSpeedDefault;
	TheUI.MouseScrollSpeedDefault = LuaToNumber(l, 1);

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Set speed of ctrl-middle-mouse scroll
**
**  @param l  Lua state.
**
**  @return   The old value.
*/
static int CclSetMouseScrollSpeedControl(lua_State* l)
{
	lua_Number old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = TheUI.MouseScrollSpeedControl;
	TheUI.MouseScrollSpeedControl = LuaToNumber(l, 1);

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Set which missile is used for right click
**
**  @param l  Lua state.
**
**  @return   old value
*/
static int CclSetClickMissile(lua_State* l)
{
	char* old;
	int args;

	args = lua_gettop(l);
	if (args > 1 || (args == 1 && (!lua_isnil(l, 1) && !lua_isstring(l, 1)))) {
		LuaError(l, "incorrect argument");
	}
	old = NULL;
	if (ClickMissile) {
		old = strdup(ClickMissile);
		free(ClickMissile);
		ClickMissile = NULL;
	}
	if (args == 1 && !lua_isnil(l, 1)) {
		ClickMissile = strdup(lua_tostring(l, 1));
	}

	lua_pushstring(l, old);
	free(old);
	return 1;
}

/**
**  Set which missile shows Damage
**
**  @param l  Lua state.
**
**  @return   old value
*/
static int CclSetDamageMissile(lua_State* l)
{
	char* old;
	int args;

	args = lua_gettop(l);
	if (args > 1 || (args == 1 && (!lua_isnil(l, 1) && !lua_isstring(l, 1)))) {
		LuaError(l, "incorrect argument");
	}
	old = NULL;
	if (DamageMissile) {
		old = strdup(DamageMissile);
		free(DamageMissile);
		DamageMissile = NULL;
	}
	if (args == 1 && !lua_isnil(l, 1)) {
		DamageMissile = strdup(lua_tostring(l, 1));
	}

	lua_pushstring(l, old);
	free(old);
	return 1;
}

/**
**  Set the video resolution.
**
**  @param l  Lua state.
*/
static int CclSetVideoResolution(lua_State* l)
{
	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}
	if (CclInConfigFile) {
		// May have been set from the command line
		if (!VideoWidth || !VideoHeight) {
			VideoWidth = LuaToNumber(l, 1);
			VideoHeight = LuaToNumber(l, 2);
		}
	}
	return 0;
}

/**
**  Set the video fullscreen mode.
**
**  @param l  Lua state.
**
**  @return   Old fullscreen mode
*/
static int CclSetVideoFullScreen(lua_State* l)
{
	int old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = VideoFullScreen;
	if (CclInConfigFile) {
		// May have been set from the command line
		if (!VideoForceFullScreen) {
			VideoFullScreen = LuaToBoolean(l, 1);
		}
	}

	lua_pushboolean(l, old);
	return 1;
}

/**
**  Default title screens.
**
**  @param l  Lua state.
**
**  @return   None
*/
static int CclSetTitleScreens(lua_State* l)
{
	const char* value;
	int i;
	int args;
	int j;
	int subargs;
	int k;

	if (TitleScreens) {
		for (i = 0; TitleScreens[i]; ++i) {
			free(TitleScreens[i]->File);
			free(TitleScreens[i]->Music);
			if (TitleScreens[i]->Labels) {
				for (j = 0; TitleScreens[i]->Labels[j]; ++j) {
					free(TitleScreens[i]->Labels[j]->Text);
					free(TitleScreens[i]->Labels[j]);
				}
				free(TitleScreens[i]->Labels);
			}
			free(TitleScreens[i]);
		}
		free(TitleScreens);
		TitleScreens = NULL;
	}

	args = lua_gettop(l);
	TitleScreens = calloc(args + 1, sizeof(*TitleScreens));

	for (j = 0; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			LuaError(l, "incorrect argument");
		}
		TitleScreens[j] = calloc(1, sizeof(**TitleScreens));
		TitleScreens[j]->Iterations = 1;
		lua_pushnil(l);
		while (lua_next(l, j + 1)) {
			value = LuaToString(l, -2);
			if (!strcmp(value, "Image")) {
				TitleScreens[j]->File = strdup(LuaToString(l, -1));
			} else if (!strcmp(value, "Music")) {
				TitleScreens[j]->Music = strdup(LuaToString(l, -1));
			} else if (!strcmp(value, "Timeout")) {
				TitleScreens[j]->Timeout = LuaToNumber(l, -1);
			} else if (!strcmp(value, "Iterations")) {
				TitleScreens[j]->Iterations = LuaToNumber(l, -1);
			} else if (!strcmp(value, "Labels")) {
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				subargs = luaL_getn(l, -1);
				TitleScreens[j]->Labels = calloc(subargs + 1, sizeof(*TitleScreens[j]->Labels));
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, -1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					TitleScreens[j]->Labels[k] = calloc(1, sizeof(**TitleScreens[j]->Labels));
					lua_pushnil(l);
					while (lua_next(l, -2)) {
						value = LuaToString(l, -2);
						if (!strcmp(value, "Text")) {
							TitleScreens[j]->Labels[k]->Text = strdup(LuaToString(l, -1));
						} else if (!strcmp(value, "Font")) {
							TitleScreens[j]->Labels[k]->Font = FontByIdent(LuaToString(l, -1));
						} else if (!strcmp(value, "Pos")) {
							if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
								LuaError(l, "incorrect argument");
							}
							lua_rawgeti(l, -1, 1);
							TitleScreens[j]->Labels[k]->Xofs = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_rawgeti(l, -1, 2);
							TitleScreens[j]->Labels[k]->Yofs = LuaToNumber(l, -1);
							lua_pop(l, 1);
						} else if (!strcmp(value, "Flags")) {
							int subsubargs;
							int subk;

							if (!lua_istable(l, -1)) {
								LuaError(l, "incorrect argument");
							}
							subsubargs = luaL_getn(l, -1);
							for (subk = 0; subk < subsubargs; ++subk) {
								lua_rawgeti(l, -1, subk + 1);
								value = LuaToString(l, -1);
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
**  Default menu music.
**
**  @param l  Lua state.
**
**  @return   Old menu music.
*/
static int CclSetMenuMusic(lua_State* l)
{
	char* old;

	old = NULL;
	if (MenuMusic) {
		old = strdup(MenuMusic);
	}
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	if (MenuMusic) {
		free(MenuMusic);
		MenuMusic = NULL;
	}
	MenuMusic = strdup(LuaToString(l, 1));

	lua_pushstring(l, old);
	free(old);
	return 1;
}

/**
**  Display a picture.
**
**  @param l  Lua state.
*/
static int CclDisplayPicture(lua_State* l)
{
	char* name;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	name = strdup(LuaToString(l, 1));
	SetClipping(0, 0, VideoWidth - 1, VideoHeight - 1);
	DisplayPicture(name);
	Invalidate();
	free(name);

	return 0;
}

/**
**  Process a menu.
**
**  @param l  Lua state.
*/
static int CclProcessMenu(lua_State* l)
{
	char* mid;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	mid = strdup(LuaToString(l, 1));
	if (FindMenu(mid)) {
		ProcessMenu(mid, 1);
	}
	free(mid);

	return 0;
}

/**
**  Define a cursor.
**
**  @param l  Lua state.
*/
static int CclDefineCursor(lua_State* l)
{
	const char* value;
	const char* name;
	const char* race;
	const char* file;
	int hotx;
	int hoty;
	int w;
	int h;
	int rate;
	int i;
	CursorType* ct;

	if (lua_gettop(l) != 1 || !lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	name = race = file = NULL;
	hotx = hoty = w = h = 0;
	rate = 200;
	lua_pushnil(l);
	while (lua_next(l, 1)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Name")) {
			name = LuaToString(l, -1);
		} else if (!strcmp(value, "Race")) {
			race = LuaToString(l, -1);
		} else if (!strcmp(value, "File")) {
			file = LuaToString(l, -1);
		} else if (!strcmp(value, "HotSpot")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			hotx = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			hoty = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "Size")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			w = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			h = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "Rate")) {
			rate = LuaToNumber(l, -1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}

	Assert(name && file && w && h);

	if (!strcmp(race, "any")) {
		race = NULL;
	}

	//
	//  Look if this kind of cursor already exists.
	//
	ct = NULL;
	i = 0;
	if (Cursors) {
		for (; Cursors[i].OType; ++i) {
			//
			//  Race not same, not found.
			//
			if (Cursors[i].Race && race) {
				if (strcmp(Cursors[i].Race, race)) {
					continue;
				}
			} else if (Cursors[i].Race != race) {
				continue;
			}
			if (!strcmp(Cursors[i].Ident, name)) {
				ct = &Cursors[i];
				break;
			}
		}
	}
	//
	//  Not found, make a new slot.
	//
	if (!ct) {
		ct = calloc(i + 2, sizeof(CursorType));
		memcpy(ct, Cursors, sizeof(CursorType) * i);
		free(Cursors);
		Cursors = ct;
		ct = &Cursors[i];
		ct->OType = CursorTypeType;
		ct->Ident = strdup(name);
		ct->Race = race ? strdup(race) : NULL;
	}

	free(ct->File);
	ct->File = strdup(file);
	ct->HotX = hotx;
	ct->HotY = hoty;
	ct->Width = w;
	ct->Height = h;
	ct->FrameRate = rate;

	return 0;
}

/**
**  Set the current game cursor.
**
**  @param l  Lua state.
*/
static int CclSetGameCursor(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	GameCursor = CursorTypeByIdent(LuaToString(l, 1));
	return 0;
}

/**
**  Define a menu item
**
**  @param l      Lua state.
**  @param value  Button type.
*/
static MenuButtonId scm2buttonid(lua_State* l, const char* value)
{
	MenuButtonId id;

	if (!strcmp(value, "up-arrow")) {
		id = MBUTTON_UP_ARROW;
	} else if (!strcmp(value, "down-arrow")) {
		id = MBUTTON_DOWN_ARROW;
	} else if (!strcmp(value, "left-arrow")) {
		id = MBUTTON_LEFT_ARROW;
	} else if (!strcmp(value, "right-arrow")) {
		id = MBUTTON_RIGHT_ARROW;
	} else if (!strcmp(value, "s-knob")) {
		id = MBUTTON_S_KNOB;
	} else if (!strcmp(value, "s-vcont")) {
		id = MBUTTON_S_VCONT;
	} else if (!strcmp(value, "s-hcont")) {
		id = MBUTTON_S_HCONT;
	} else if (!strcmp(value, "pulldown")) {
		id = MBUTTON_PULLDOWN;
	} else if (!strcmp(value, "vthin")) {
		id = MBUTTON_VTHIN;
	} else if (!strcmp(value, "sc-gem-round")) {
		id = MBUTTON_SC_GEM_ROUND;
	} else if (!strcmp(value, "sc-gem-square")) {
		id = MBUTTON_SC_GEM_SQUARE;
	} else if (!strcmp(value, "sc-up-arrow")) {
		id = MBUTTON_SC_UP_ARROW;
	} else if (!strcmp(value, "sc-down-arrow")) {
		id = MBUTTON_SC_DOWN_ARROW;
	} else if (!strcmp(value, "sc-left-arrow")) {
		id = MBUTTON_SC_LEFT_ARROW;
	} else if (!strcmp(value, "sc-right-arrow")) {
		id = MBUTTON_SC_RIGHT_ARROW;
	} else if (!strcmp(value, "sc-s-knob")) {
		id = MBUTTON_SC_S_KNOB;
	} else if (!strcmp(value, "sc-s-vcont")) {
		id = MBUTTON_SC_S_VCONT;
	} else if (!strcmp(value, "sc-s-hcont")) {
		id = MBUTTON_SC_S_HCONT;
	} else if (!strcmp(value, "sc-pulldown")) {
		id = MBUTTON_SC_PULLDOWN;
	} else if (!strcmp(value, "sc-button-left")) {
		id = MBUTTON_SC_BUTTON_LEFT;
	} else if (!strcmp(value, "sc-button")) {
		id = MBUTTON_SC_BUTTON;
	} else if (!strcmp(value, "sc-button-right")) {
		id = MBUTTON_SC_BUTTON_RIGHT;
	} else {
		LuaError(l, "Unsupported button: %s" _C_ value);
		id = 0;
	}
	return id;
}

/**
**  Parse info panel text
**
**  @param l     Lua state.
**  @param text  FIXME: docu.
*/
static void CclParseInfoText(lua_State* l, InfoText* text)
{
	const char* value;
	int args;
	int j;

	memset(text, 0, sizeof(*text));

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;
		if (!strcmp(value, "text")) {
			lua_rawgeti(l, -1, j + 1);
			text->Text = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
		} else if (!strcmp(value, "font")) {
			lua_rawgeti(l, -1, j + 1);
			text->Font = FontByIdent(LuaToString(l, -1));
			lua_pop(l, 1);
		} else if (!strcmp(value, "pos")) {
			lua_rawgeti(l, -1, j + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			text->X = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			text->Y = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
}

/**
**  Parse icon
**
**  @param l     Lua state.
**  @param icon  FIXME: docu.
*/
static void CclParseIcon(lua_State* l, Button* icon)
{
	const char* value;
	int args;
	int j;

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;
		if (!strcmp(value, "pos")) {
			lua_rawgeti(l, -1, j + 1);
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			icon->X = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			icon->Y = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "style")) {
			lua_rawgeti(l, -1, j + 1);
			icon->Style = FindButtonStyle(LuaToString(l, -1));
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
}

/**
**  Parse info panel selected section
**
**  @param l   Lua state.
**  @param ui  FIXME: docu.
*/
static void CclParseSelected(lua_State* l, UI* ui)
{
	const char* value;
	InfoText text;
	int args;
	int j;
	int subargs;
	int k;

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;
		if (!strcmp(value, "single")) {
			lua_rawgeti(l, -1, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "text")) {
					lua_rawgeti(l, -1, k + 1);
					CclParseInfoText(l, &text);
					lua_pop(l, 1);
					ui->SingleSelectedText = text.Text;
					ui->SingleSelectedFont = text.Font;
					ui->SingleSelectedTextX = text.X;
					ui->SingleSelectedTextY = text.Y;
				} else if (!strcmp(value, "icon")) {
					lua_rawgeti(l, -1, k + 1);
					ui->SingleSelectedButton = calloc(1, sizeof(Button));
					CclParseIcon(l, ui->SingleSelectedButton);
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
			lua_pop(l, 1);
		} else if (!strcmp(value, "multiple")) {
			lua_rawgeti(l, -1, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "text")) {
					lua_rawgeti(l, -1, k + 1);
					CclParseInfoText(l, &text);
					lua_pop(l, 1);
					ui->SelectedText = text.Text;
					ui->SelectedFont = text.Font;
					ui->SelectedTextX = text.X;
					ui->SelectedTextY = text.Y;
				} else if (!strcmp(value, "icons")) {
					int i;

					lua_rawgeti(l, -1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					ui->NumSelectedButtons = luaL_getn(l, -1);
					ui->SelectedButtons = calloc(ui->NumSelectedButtons,
						sizeof(Button));
					for (i = 0; i < ui->NumSelectedButtons; ++i) {
						lua_rawgeti(l, -1, i + 1);
						CclParseIcon(l, &ui->SelectedButtons[i]);
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				} else if (!strcmp(value, "max-text")) {
					lua_rawgeti(l, -1, k + 1);
					CclParseInfoText(l, &text);
					lua_pop(l, 1);
					ui->MaxSelectedFont = text.Font;
					ui->MaxSelectedTextX = text.X;
					ui->MaxSelectedTextY = text.Y;
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
}

/**
**  Parse info panel training section
**
**  @param l   Lua state.
**  @param ui  FIXME: docu.
*/
static void CclParseTraining(lua_State* l, UI* ui)
{
	const char* value;
	InfoText text;
	int args;
	int j;
	int subargs;
	int k;

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;
		if (!strcmp(value, "single")) {
			lua_rawgeti(l, -1, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "text")) {
					lua_rawgeti(l, -1, k + 1);
					CclParseInfoText(l, &text);
					lua_pop(l, 1);
					ui->SingleTrainingText = text.Text;
					ui->SingleTrainingFont = text.Font;
					ui->SingleTrainingTextX = text.X;
					ui->SingleTrainingTextY = text.Y;
				} else if (!strcmp(value, "icon")) {
					lua_rawgeti(l, -1, k + 1);
					ui->SingleTrainingButton = calloc(1, sizeof(Button));
					CclParseIcon(l, ui->SingleTrainingButton);
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
			lua_pop(l, 1);
		} else if (!strcmp(value, "multiple")) {
			lua_rawgeti(l, -1, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "text")) {
					lua_rawgeti(l, -1, k + 1);
					CclParseInfoText(l, &text);
					lua_pop(l, 1);
					ui->TrainingText = text.Text;
					ui->TrainingFont = text.Font;
					ui->TrainingTextX = text.X;
					ui->TrainingTextY = text.Y;
				} else if (!strcmp(value, "icons")) {
					int i;

					lua_rawgeti(l, -1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					ui->NumTrainingButtons = luaL_getn(l, -1);
					ui->TrainingButtons = calloc(ui->NumTrainingButtons,
						sizeof(Button));
					for (i = 0; i < ui->NumTrainingButtons; ++i) {
						lua_rawgeti(l, -1, i + 1);
						CclParseIcon(l, &ui->TrainingButtons[i]);
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
}

/**
**  Parse info panel upgrading section
**
**  @param l   Lua state.
**  @param ui  FIXME: docu.
*/
static void CclParseUpgrading(lua_State* l, UI* ui)
{
	const char* value;
	InfoText text;
	int args;
	int j;

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;
		if (!strcmp(value, "text")) {
			lua_rawgeti(l, -1, j + 1);
			CclParseInfoText(l, &text);
			lua_pop(l, 1);
			ui->UpgradingText = text.Text;
			ui->UpgradingFont = text.Font;
			ui->UpgradingTextX = text.X;
			ui->UpgradingTextY = text.Y;
		} else if (!strcmp(value, "icon")) {
			lua_rawgeti(l, -1, j + 1);
			ui->UpgradingButton = calloc(1, sizeof(Button));
			CclParseIcon(l, ui->UpgradingButton);
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
}

/**
**  Parse info panel researching section
**
**  @param l   Lua state.
**  @param ui  FIXME: docu.
*/
static void CclParseResearching(lua_State* l, UI* ui)
{
	const char* value;
	InfoText text;
	int args;
	int j;

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;
		if (!strcmp(value, "text")) {
			lua_rawgeti(l, -1, j + 1);
			CclParseInfoText(l, &text);
			lua_pop(l, 1);
			ui->ResearchingText = text.Text;
			ui->ResearchingFont = text.Font;
			ui->ResearchingTextX = text.X;
			ui->ResearchingTextY = text.Y;
		} else if (!strcmp(value, "icon")) {
			lua_rawgeti(l, -1, j + 1);
			ui->ResearchingButton = calloc(1, sizeof(Button));
			CclParseIcon(l, ui->ResearchingButton);
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
}

/**
**  Parse info panel transporting section
**
**  @param l   Lua state.
**  @param ui  FIXME: docu.
*/
static void CclParseTransporting(lua_State* l, UI* ui)
{
	const char* value;
	InfoText text;
	int args;
	int j;

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;
		if (!strcmp(value, "text")) {
			lua_rawgeti(l, -1, j + 1);
			CclParseInfoText(l, &text);
			lua_pop(l, 1);
			ui->TransportingText = text.Text;
			ui->TransportingFont = text.Font;
			ui->TransportingTextX = text.X;
			ui->TransportingTextY = text.Y;
		} else if (!strcmp(value, "icons")) {
			int i;

			lua_rawgeti(l, -1, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			ui->NumTransportingButtons = luaL_getn(l, -1);
			ui->TransportingButtons = calloc(ui->NumTransportingButtons,
				sizeof(Button));
			for (i = 0; i < ui->NumTransportingButtons; ++i) {
				lua_rawgeti(l, -1, i + 1);
				CclParseIcon(l, &ui->TransportingButtons[i]);
				lua_pop(l, 1);
			}
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
}

/**
**  Parse button panel icons section
**
**  @param l   Lua state.
**  @param ui  FIXME: docu.
*/
static void CclParseButtonIcons(lua_State* l, UI* ui)
{
	int i;

	ui->NumButtonButtons = luaL_getn(l, -1);
	ui->ButtonButtons = calloc(ui->NumButtonButtons, sizeof(Button));
	for (i = 0; i < ui->NumButtonButtons; ++i) {
		lua_rawgeti(l, -1, i + 1);
		CclParseIcon(l, &ui->ButtonButtons[i]);
		lua_pop(l, 1);
	}
}

/**
**  Define the look+feel of the user interface.
**
**  @param l  Lua state.
*/
static int CclDefineUI(lua_State* l)
{
	const char* value;
	char* str;
	int x;
	int y;
	int i;
	UI* ui;
	void* v;
	int args;
	int subargs;
	int j;
	int k;

	j = 0;
	args = lua_gettop(l);

	// Get identifier
	str = strdup(LuaToString(l, j + 1));
	++j;
	x = LuaToNumber(l, j + 1);
	++j;
	y = LuaToNumber(l, j + 1);
	++j;

	// Find slot: new or redefinition
	ui = NULL;
	i = 0;
	if (UI_Table) {
		for (; UI_Table[i]; ++i) {
			if (UI_Table[i]->Width == x && UI_Table[i]->Height == y &&
					!strcmp(UI_Table[i]->Name, str)) {
				CleanUI(UI_Table[i]);
				ui = calloc(1, sizeof(UI));
				UI_Table[i] = ui;
				break;
			}
		}
	}
	if (!ui) {
		ui = calloc(1, sizeof(UI));
		v = malloc(sizeof(UI*) * (i + 2));
		memcpy(v, UI_Table, i * sizeof(UI*));
		free(UI_Table);
		UI_Table = v;
		UI_Table[i] = ui;
		UI_Table[i + 1] = NULL;
	}

	ui->Name = str;
	ui->Width = x;
	ui->Height = y;

	//
	// Some value defaults
	//

	// This save the setup values FIXME: They are set by CCL.

	ui->MouseScroll = TheUI.MouseScroll;
	ui->KeyScroll = TheUI.KeyScroll;
	ui->MouseScrollSpeedDefault = TheUI.MouseScrollSpeedDefault;
	ui->MouseScrollSpeedControl = TheUI.MouseScrollSpeedControl;

	ui->MouseWarpX = -1;
	ui->MouseWarpY = -1;

	ui->InfoPanel.File = NULL;
	ui->InfoPanelX = -1;
	ui->InfoPanelY = -1;

	ui->ButtonPanel.File = NULL;
	ui->ButtonPanelX = -1;
	ui->ButtonPanelY = -1;

	ui->MinimapTransparent = 0;

	ui->MinimapPosX = -1;
	ui->MinimapPosY = -1;
	for (i = 0; i < MaxCosts + 2; ++i) {
		ui->Resources[i].TextX = -1;
	}
	//
	// Parse the arguments, already the new tagged format.
	//  maxy: this could be much simpler
	//

	for (; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;
		if (!strcmp(value, "normal-font-color")) {
			ui->NormalFontColor = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "reverse-font-color")) {
			ui->ReverseFontColor = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "filler")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			ui->NumFillers++;
			ui->Filler = realloc(ui->Filler, ui->NumFillers * sizeof(*ui->Filler));
			ui->FillerX = realloc(ui->FillerX, ui->NumFillers * sizeof(*ui->FillerX));
			ui->FillerY = realloc(ui->FillerY, ui->NumFillers * sizeof(*ui->FillerY));
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "file")) {
					lua_rawgeti(l, j + 1, k + 1);
					ui->Filler[ui->NumFillers - 1].File = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "pos")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					ui->FillerX[ui->NumFillers - 1] = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					ui->FillerY[ui->NumFillers - 1] = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "resources")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				int res;
				int subk;
				int subsubargs;

				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				for (res = 0; res < MaxCosts; ++res) {
					if (!strcmp(value, DefaultResourceNames[res])) {
						break;
					}
				}
				if (res == MaxCosts) {
					if (!strcmp(value, "food")) {
						res = FoodCost;
					} else if (!strcmp(value, "score")) {
						res = ScoreCost;
					} else {
						LuaError(l, "Resource not found: %s" _C_ value);
					}
				}
				lua_rawgeti(l, j + 1, k + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				subsubargs = luaL_getn(l, -1);
				for (subk = 0; subk < subsubargs; ++subk) {
					lua_rawgeti(l, -1, subk + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					++subk;
					if (!strcmp(value, "pos")) {
						lua_rawgeti(l, -1, subk + 1);
						if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
							LuaError(l, "incorrect argument");
						}
						lua_rawgeti(l, -1, 1);
						ui->Resources[res].IconX = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_rawgeti(l, -1, 2);
						ui->Resources[res].IconY = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "file")) {
						lua_rawgeti(l, -1, subk + 1);
						ui->Resources[res].Icon.File = strdup(LuaToString(l, -1));
						lua_pop(l, 1);
					} else if (!strcmp(value, "frame")) {
						lua_rawgeti(l, -1, subk + 1);
						ui->Resources[res].IconFrame = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "size")) {
						lua_rawgeti(l, -1, subk + 1);
						if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
							LuaError(l, "incorrect argument");
						}
						lua_rawgeti(l, -1, 1);
						ui->Resources[res].IconW = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_rawgeti(l, -1, 2);
						ui->Resources[res].IconH = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "text-pos")) {
						lua_rawgeti(l, -1, subk + 1);
						if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
							LuaError(l, "incorrect argument");
						}
						lua_rawgeti(l, -1, 1);
						ui->Resources[res].TextX = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_rawgeti(l, -1, 2);
						ui->Resources[res].TextY = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_pop(l, 1);
					} else {
						LuaError(l, "Unsupported tag: %s" _C_ value);
					}
				}
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "info-panel")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "panel")) {
					int subk;
					int subsubargs;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					subsubargs = luaL_getn(l, -1);
					for (subk = 0; subk < subsubargs; ++subk) {
						lua_rawgeti(l, -1, subk + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						++subk;
						if (!strcmp(value, "file")) {
							lua_rawgeti(l, -1, subk + 1);
							ui->InfoPanel.File = strdup(LuaToString(l, -1));
							lua_pop(l, 1);
						} else if (!strcmp(value, "pos")) {
							lua_rawgeti(l, -1, subk + 1);
							if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
								LuaError(l, "incorrect argument");
							}
							lua_rawgeti(l, -1, 1);
							ui->InfoPanelX = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_rawgeti(l, -1, 2);
							ui->InfoPanelY = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_pop(l, 1);
						} else if (!strcmp(value, "size")) {
							lua_rawgeti(l, -1, subk + 1);
							if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
								LuaError(l, "incorrect argument");
							}
							lua_rawgeti(l, -1, 1);
							ui->InfoPanelW = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_rawgeti(l, -1, 2);
							ui->InfoPanelH = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_pop(l, 1);
						} else {
							LuaError(l, "Unsupported tag: %s" _C_ value);
						}
					}
					lua_pop(l, 1);
				} else if (!strcmp(value, "selected")) {
					lua_rawgeti(l, j + 1, k + 1);
					CclParseSelected(l, ui);
				} else if (!strcmp(value, "training")) {
					lua_rawgeti(l, j + 1, k + 1);
					CclParseTraining(l, ui);
				} else if (!strcmp(value, "upgrading")) {
					lua_rawgeti(l, j + 1, k + 1);
					CclParseUpgrading(l, ui);
				} else if (!strcmp(value, "researching")) {
					lua_rawgeti(l, j + 1, k + 1);
					CclParseResearching(l, ui);
				} else if (!strcmp(value, "transporting")) {
					lua_rawgeti(l, j + 1, k + 1);
					CclParseTransporting(l, ui);
				} else if (!strcmp(value, "completed-bar")) {
					int subsubargs;
					int subk;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					subsubargs = luaL_getn(l, -1);
					for (subk = 0; subk < subsubargs; ++subk) {
						lua_rawgeti(l, -1, subk + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						++subk;
						if (!strcmp(value, "color")) {
							lua_rawgeti(l, -1, subk + 1);
							if (!lua_istable(l, -1) || luaL_getn(l, -1) != 3) {
								LuaError(l, "incorrect argument");
							}
							lua_rawgeti(l, -1, 1);
							ui->CompletedBarColorRGB.r = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_rawgeti(l, -1, 2);
							ui->CompletedBarColorRGB.g = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_rawgeti(l, -1, 3);
							ui->CompletedBarColorRGB.b = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_pop(l, 1);
						} else if (!strcmp(value, "pos")) {
							lua_rawgeti(l, -1, subk + 1);
							if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
								LuaError(l, "incorrect argument");
							}
							lua_rawgeti(l, -1, 1);
							ui->CompletedBarX = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_rawgeti(l, -1, 2);
							ui->CompletedBarY = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_pop(l, 1);
						} else if (!strcmp(value, "size")) {
							lua_rawgeti(l, -1, subk + 1);
							if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
								LuaError(l, "incorrect argument");
							}
							lua_rawgeti(l, -1, 1);
							ui->CompletedBarW = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_rawgeti(l, -1, 2);
							ui->CompletedBarH = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_pop(l, 1);
						} else if (!strcmp(value, "text")) {
							InfoText text;

							lua_rawgeti(l, -1, subk + 1);
							CclParseInfoText(l, &text);
							lua_pop(l, 1);
							ui->CompletedBarText = text.Text;
							ui->CompletedBarFont = text.Font;
							ui->CompletedBarTextX = text.X;
							ui->CompletedBarTextY = text.Y;
						} else if (!strcmp(value, "has-shadow")) {
							ui->CompletedBarShadow = 1;
							--subk;
						} else {
							LuaError(l, "Unsupported tag: %s" _C_ value);
						}
					}
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "button-panel")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "panel")) {
					int subk;
					int subsubargs;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					subsubargs = luaL_getn(l, -1);
					for (subk = 0; subk < subsubargs; ++subk) {
						lua_rawgeti(l, -1, subk + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						++subk;
						if (!strcmp(value, "file")) {
							lua_rawgeti(l, -1, subk + 1);
							ui->ButtonPanel.File = strdup(LuaToString(l, -1));
							lua_pop(l, 1);
						} else if (!strcmp(value, "pos")) {
							lua_rawgeti(l, -1, subk + 1);
							if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
								LuaError(l, "incorrect argument");
							}
							lua_rawgeti(l, -1, 1);
							ui->ButtonPanelX = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_rawgeti(l, -1, 2);
							ui->ButtonPanelY = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_pop(l, 1);
						} else {
							LuaError(l, "Unsupported tag: %s" _C_ value);
						}
					}
					lua_pop(l, 1);
				} else if (!strcmp(value, "icons")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					CclParseButtonIcons(l, ui);
					lua_pop(l, 1);
				} else if (!strcmp(value, "auto-cast-border-color")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					ui->ButtonAutoCastBorderColorRGB.r = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					ui->ButtonAutoCastBorderColorRGB.g = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 3);
					ui->ButtonAutoCastBorderColorRGB.b = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "piemenu")) {
			k = 0;
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "file")) {
					lua_rawgeti(l, j + 1, k + 1);
					ui->PieMenuBackground.File = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "radius")) {
					// Position of the pies in a piemenu
					int coeffX[] = {    0,  193, 256, 193,   0, -193, -256, -193};
					int coeffY[] = { -256, -193,   0, 193, 256,  193,    0, -193};
					int pie;
					int radius;

					lua_rawgeti(l, j + 1, k + 1);
					radius = LuaToNumber(l, -1);
					lua_pop(l, 1);
					for (pie = 0; pie < 8; ++pie) {
						ui->PieX[pie]= (coeffX[pie] * radius) >> 8;
						ui->PieY[pie]= (coeffY[pie] * radius) >> 8;
					}
				} else if (!strcmp(value, "mouse-button")) {
					const char* button;

					lua_rawgeti(l, j + 1, k + 1);
					button = LuaToString(l, -1);
					if (!strcmp(button, "right")) {
						ui->PieMouseButton = RightButton;
					} else if (!strcmp(button, "middle")) {
						ui->PieMouseButton = MiddleButton;
					} else if (!strcmp(button, "left")) {
						ui->PieMouseButton = LeftButton;
					} else {
						ui->PieMouseButton = NoButton;
					}
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
			lua_pop(l, 1);
		} else if (!strcmp(value, "map-area")) {
			int w;
			int h;

			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			w = 0;
			h = 0;
			lua_pushnil(l);
			while (lua_next(l, j + 1)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "Pos")) {
					if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					ui->MapArea.X = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					ui->MapArea.Y = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "Size")) {
					if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					w = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					h = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
				lua_pop(l, 1);
			}
			ui->MapArea.EndX = ui->MapArea.X + w - 1;
			ui->MapArea.EndY = ui->MapArea.Y + h - 1;
		} else if (!strcmp(value, "menu-panel")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				int subk;
				int subsubargs;

				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "menu-button")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					subsubargs = luaL_getn(l, -1);
					for (subk = 0; subk < subsubargs; ++subk) {
						lua_rawgeti(l, -1, subk + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						++subk;
						if (!strcmp(value, "pos")) {
							lua_rawgeti(l, -1, subk + 1);
							if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
								LuaError(l, "incorrect argument");
							}
							lua_rawgeti(l, -1, 1);
							ui->MenuButton.X = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_rawgeti(l, -1, 2);
							ui->MenuButton.Y = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_pop(l, 1);
						} else if (!strcmp(value, "caption")) {
							lua_rawgeti(l, -1, subk + 1);
							ui->MenuButton.Text = strdup(LuaToString(l, -1));
							lua_pop(l, 1);
						} else if (!strcmp(value, "style")) {
							lua_rawgeti(l, -1, subk + 1);
							ui->MenuButton.Style = FindButtonStyle(LuaToString(l, -1));
							if (!ui->MenuButton.Style) {
								LuaError(l, "Invalid button style: %s" _C_
									LuaToString(l, -1));
							}
							lua_pop(l, 1);
						} else {
							LuaError(l, "Unsupported tag: %s" _C_ value);
						}
					}
				} else if (!strcmp(value, "network-menu-button")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					subsubargs = luaL_getn(l, -1);
					for (subk = 0; subk < subsubargs; ++subk) {
						lua_rawgeti(l, -1, subk + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						++subk;
						if (!strcmp(value, "pos")) {
							lua_rawgeti(l, -1, subk + 1);
							if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
								LuaError(l, "incorrect argument");
							}
							lua_rawgeti(l, -1, 1);
							ui->NetworkMenuButton.X = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_rawgeti(l, -1, 2);
							ui->NetworkMenuButton.Y = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_pop(l, 1);
						} else if (!strcmp(value, "caption")) {
							lua_rawgeti(l, -1, subk + 1);
							ui->NetworkMenuButton.Text = strdup(LuaToString(l, -1));
							lua_pop(l, 1);
						} else if (!strcmp(value, "style")) {
							lua_rawgeti(l, -1, subk + 1);
							ui->NetworkMenuButton.Style = FindButtonStyle(LuaToString(l, -1));
							if (!ui->NetworkMenuButton.Style) {
								LuaError(l, "Invalid button style: %s" _C_
									LuaToString(l, -1));
							}
							lua_pop(l, 1);
						} else {
							LuaError(l, "Unsupported tag: %s" _C_ value);
						}
					}
				} else if (!strcmp(value, "network-diplomacy-button")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					subsubargs = luaL_getn(l, -1);
					for (subk = 0; subk < subsubargs; ++subk) {
						lua_rawgeti(l, -1, subk + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						++subk;
						if (!strcmp(value, "pos")) {
							lua_rawgeti(l, -1, subk + 1);
							if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
								LuaError(l, "incorrect argument");
							}
							lua_rawgeti(l, -1, 1);
							ui->NetworkDiplomacyButton.X = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_rawgeti(l, -1, 2);
							ui->NetworkDiplomacyButton.Y = LuaToNumber(l, -1);
							lua_pop(l, 1);
							lua_pop(l, 1);
						} else if (!strcmp(value, "caption")) {
							lua_rawgeti(l, -1, subk + 1);
							ui->NetworkDiplomacyButton.Text = strdup(LuaToString(l, -1));
							lua_pop(l, 1);
						} else if (!strcmp(value, "style")) {
							lua_rawgeti(l, -1, subk + 1);
							ui->NetworkDiplomacyButton.Style = FindButtonStyle(LuaToString(l, -1));
							if (!ui->NetworkDiplomacyButton.Style) {
								LuaError(l, "Invalid button style: %s" _C_
									LuaToString(l, -1));
							}
							lua_pop(l, 1);
						} else {
							LuaError(l, "Unsupported tag: %s" _C_ value);
						}
					}
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "minimap")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			lua_pushnil(l);
			while (lua_next(l, j + 1)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "Pos")) {
					if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					ui->MinimapPosX = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					ui->MinimapPosY = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "Size")) {
					if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					ui->MinimapW = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					ui->MinimapH = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "Transparent")) {
					ui->MinimapTransparent = LuaToBoolean(l, -1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "status-line")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "text-pos")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					ui->StatusLineTextX = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					ui->StatusLineTextY = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "width")) {
					lua_rawgeti(l, j + 1, k + 1);
					ui->StatusLineW = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "font")) {
					lua_rawgeti(l, j + 1, k + 1);
					ui->StatusLineFont = FontByIdent(LuaToString(l, -1));
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "cursors")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			lua_pushnil(l);
			while (lua_next(l, j + 1)) {
				value = LuaToString(l, -2);
				if (!strcmp(value, "Point")) {
					ui->Point.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "Glass")) {
					ui->Glass.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "Cross")) {
					ui->Cross.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "Yellow")) {
					ui->YellowHair.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "Green")) {
					ui->GreenHair.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "Red")) {
					ui->RedHair.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "Scroll")) {
					ui->Scroll.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "ArrowE")) {
					ui->ArrowE.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "ArrowNE")) {
					ui->ArrowNE.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "ArrowN")) {
					ui->ArrowN.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "ArrowNW")) {
					ui->ArrowNW.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "ArrowW")) {
					ui->ArrowW.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "ArrowSW")) {
					ui->ArrowSW.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "ArrowS")) {
					ui->ArrowS.Name = strdup(LuaToString(l, -1));
				} else if (!strcmp(value, "ArrowSE")) {
					ui->ArrowSE.Name = strdup(LuaToString(l, -1));
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "menu-panels")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				MenuPanel** menupanel;

				menupanel = &ui->MenuPanels;
				while (*menupanel) {
					menupanel = &(*menupanel)->Next;
				}
				*menupanel = calloc(1, sizeof(**menupanel));
				lua_rawgeti(l, j + 1, k + 1);
				(*menupanel)->Ident = strdup(LuaToString(l, -1));
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, j + 1, k + 1);
				(*menupanel)->Panel.File = strdup(LuaToString(l, -1));
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "victory-background")) {
			// Backgrounds
			ui->VictoryBackground.File = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "defeat-background")) {
			ui->DefeatBackground.File = strdup(LuaToString(l, j + 1));
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	return 0;
}

/**
**  Define the viewports.
**
**  @param l  Lua state.
*/
static int CclDefineViewports(lua_State* l)
{
	const char* value;
	UI* ui;
	int i;
	int args;
	int j;

	i = 0;
	ui = &TheUI;
	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;
		if (!strcmp(value, "mode")) {
			ui->ViewportMode = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "viewport")) {
			if (!lua_istable(l, j + 1) && luaL_getn(l, j + 1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, j + 1, 1);
			ui->Viewports[i].MapX = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 2);
			ui->Viewports[i].MapY = LuaToNumber(l, -1);
			lua_pop(l, 1);
			++i;
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	ui->NumViewports = i;

	return 0;
}

/**
**  Enable/disable scrolling with the mouse.
**
**  @param l  Lua state.
**
**  @return   The old state of scrolling.
*/
static int CclSetMouseScroll(lua_State* l)
{
	int old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = TheUI.MouseScroll;
	TheUI.MouseScroll = LuaToBoolean(l, 1);

	lua_pushboolean(l, old);
	return 1;
}

/**
**  Set speed of mouse scrolling
**
**  @param l  Lua state.
**
**  @return   old scroll speed.
*/
static int CclSetMouseScrollSpeed(lua_State* l)
{
	int speed;
	lua_Number old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = SpeedMouseScroll;
	speed = LuaToNumber(l, 1);
	if (speed < 1 || speed > FRAMES_PER_SECOND) {
		SpeedMouseScroll = MOUSE_SCROLL_SPEED;
	} else {
		SpeedMouseScroll = speed;
	}

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Enable/disable grabbing the mouse.
**
**  @param l  Lua state.
**
**  @return   FIXME: not supported: The old state of grabbing.
*/
static int CclSetGrabMouse(lua_State* l)
{
	if (lua_gettop(l) != 1 || !lua_isboolean(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	if (lua_toboolean(l, 1)) {
		ToggleGrabMouse(1);
	} else {
		ToggleGrabMouse(-1);
	}

	return 0;
}

/**
**  Enable/disable leaving the window stops scrolling.
**
**  @param l  Lua state.
**
**  @return   The old state of stopping.
*/
static int CclSetLeaveStops(lua_State* l)
{
	int old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = LeaveStops;
	LeaveStops = LuaToBoolean(l, 1);

	lua_pushboolean(l, old);
	return 1;
}

/**
**  Enable/disable scrolling with the keyboard.
**
**  @param l  Lua state.
**
**  @return   The old state of scrolling.
*/
static int CclSetKeyScroll(lua_State* l)
{
	int old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = TheUI.KeyScroll;
	TheUI.KeyScroll = LuaToBoolean(l, 1);

	lua_pushboolean(l, old);
	return 1;
}

/**
**  Set speed of keyboard scrolling
**
**  @param l  Lua state.
**
**  @return   old scroll speed.
*/
static int CclSetKeyScrollSpeed(lua_State* l)
{
	int speed;
	lua_Number old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = SpeedKeyScroll;
	speed = LuaToNumber(l, 1);
	if (speed < 1 || speed > FRAMES_PER_SECOND) {
		SpeedKeyScroll = KEY_SCROLL_SPEED;
	} else {
		SpeedKeyScroll = speed;
	}

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Enable/disable display of command keys in panels.
**
**  @param l  Lua state.
**
**  @return   The old state of scrolling.
*/
static int CclSetShowCommandKey(lua_State* l)
{
	int old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = ShowCommandKey;
	ShowCommandKey = LuaToBoolean(l, 1);
	UpdateButtonPanel();

	lua_pushboolean(l, old);
	return 1;
}

/**
**  Fighter right button attacks as default.
**
**  @param l  Lua state.
*/
static int CclRightButtonAttacks(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	RightButtonAttacks = 1;

	return 0;
}

/**
**  Fighter right button moves as default.
**
**  @param l  Lua state.
*/
static int CclRightButtonMoves(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	RightButtonAttacks = 0;

	return 0;
}

/**
**  Enable/disable the fancy buildings.
**
**  @param l  Lua state.
**
**  @return   The old state of fancy buildings flag.
*/
static int CclSetFancyBuildings(lua_State* l)
{
	int old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = FancyBuildings;
	FancyBuildings = LuaToBoolean(l, 1);

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Find a button style
**
**  @param style  Name of the style to find.
**
**  @return       Button style, NULL if not found.
*/
ButtonStyle* FindButtonStyle(const char* style)
{
	ButtonStyle** s;

	s = (ButtonStyle**)hash_find(ButtonStyleHash, style);
	if (!s) {
		return NULL;
	} else {
		return *s;
	}
}

/**
**  Find a checkbox style
**
**  @param style  Name of the style to find.
**
**  @return       Checkbox style, NULL if not found.
*/
CheckboxStyle* FindCheckboxStyle(const char* style)
{
	CheckboxStyle** s;

	s = (CheckboxStyle**)hash_find(CheckboxStyleHash, style);
	if (!s) {
		return NULL;
	} else {
		return *s;
	}
}

/**
**  Parse button style properties
**
**  @param l  Lua state.
**  @param p  Properties to fill in.
*/
static void ParseButtonStyleProperties(lua_State* l, ButtonStyleProperties* p)
{
	const char* value;

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}

	lua_pushnil(l);
	while (lua_next(l, -2)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "File")) {
			value = LuaToString(l, -1);
			if (!p->File || strcmp(p->File, value)) {
				free(p->File);
				VideoSafeFree(p->Sprite);
				p->File = strdup(LuaToString(l, -1));
			}
		} else if (!strcmp(value, "Size")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			p->Width = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			p->Height = LuaToNumber(l, -1);
			lua_pop(l, 1);
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
					if (!lua_istable(l, -1) || luaL_getn(l, -1) != 3) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					p->BorderColorRGB.r = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					p->BorderColorRGB.g = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 3);
					p->BorderColorRGB.b = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "Size")) {
					p->BorderSize = LuaToNumber(l, -1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "TextPos")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			p->TextX = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			p->TextY = LuaToNumber(l, -1);
			lua_pop(l, 1);
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
			free(p->TextNormalColor);
			p->TextNormalColor = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "TextReverseColor")) {
			free(p->TextReverseColor);
			p->TextReverseColor = strdup(LuaToString(l, -1));
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}
}

/**
**  Define a button style
**
**  @param l  Lua state.
*/
static int CclDefineButtonStyle(lua_State* l)
{
	const char* style;
	const char* value;
	ButtonStyle* b;
	ButtonStyle** bp;

	if (lua_gettop(l) != 2 || !lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	style = LuaToString(l, 1);
	bp = (ButtonStyle**)hash_find(ButtonStyleHash, (char*)style);
	if (!bp) {
		b = calloc(1, sizeof(*b));
		*(ButtonStyle**)hash_add(ButtonStyleHash, style) = b;
		// Set to bogus value to see if it was set later
		b->Default.TextX = b->Hover.TextX = b->Selected.TextX =
			b->Clicked.TextX = b->Disabled.TextX = 0xFFFFFF;
	} else {
		b = *bp;
	}

	lua_pushnil(l);
	while (lua_next(l, 2)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Size")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			b->Width = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			b->Height = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "Font")) {
			b->Font = FontByIdent(LuaToString(l, -1));
		} else if (!strcmp(value, "TextNormalColor")) {
			free(b->TextNormalColor);
			b->TextNormalColor = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "TextReverseColor")) {
			free(b->TextReverseColor);
			b->TextReverseColor = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "TextPos")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			b->TextX = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			b->TextY = LuaToNumber(l, -1);
			lua_pop(l, 1);
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
		} else if (!strcmp(value, "Selected")) {
			ParseButtonStyleProperties(l, &b->Selected);
		} else if (!strcmp(value, "Clicked")) {
			ParseButtonStyleProperties(l, &b->Clicked);
		} else if (!strcmp(value, "Disabled")) {
			ParseButtonStyleProperties(l, &b->Disabled);
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
	if (b->Selected.TextX == 0xFFFFFF) {
		b->Selected.TextX = b->TextX;
		b->Selected.TextY = b->TextY;
	}
	if (b->Clicked.TextX == 0xFFFFFF) {
		b->Clicked.TextX = b->TextX;
		b->Clicked.TextY = b->TextY;
	}
	if (b->Disabled.TextX == 0xFFFFFF) {
		b->Disabled.TextX = b->TextX;
		b->Disabled.TextY = b->TextY;
	}

	if (b->Default.TextAlign == TextAlignUndefined) {
		b->Default.TextAlign = b->TextAlign;
	}
	if (b->Hover.TextAlign == TextAlignUndefined) {
		b->Hover.TextAlign = b->TextAlign;
	}
	if (b->Selected.TextAlign == TextAlignUndefined) {
		b->Selected.TextAlign = b->TextAlign;
	}
	if (b->Clicked.TextAlign == TextAlignUndefined) {
		b->Clicked.TextAlign = b->TextAlign;
	}
	if (b->Disabled.TextAlign == TextAlignUndefined) {
		b->Disabled.TextAlign = b->TextAlign;
	}

	return 0;
}

/**
**  Define a checkbox style
**
**  @param l  Lua state.
*/
static int CclDefineCheckboxStyle(lua_State* l)
{
	const char* style;
	const char* value;
	CheckboxStyle* c;
	CheckboxStyle** cp;

	if (lua_gettop(l) != 2 || !lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	style = LuaToString(l, 1);
	cp = (CheckboxStyle**)hash_find(CheckboxStyleHash, (char*)style);
	if (!cp) {
		c = calloc(1, sizeof(*c));
		*(CheckboxStyle**)hash_add(CheckboxStyleHash, style) = c;
		// Set to bogus value to see if it was set later
		c->Default.TextX = c->Hover.TextX = c->Selected.TextX =
			c->Clicked.TextX = c->Disabled.TextX =
			c->Checked.TextX = c->CheckedHover.TextX = c->CheckedSelected.TextX =
			c->CheckedClicked.TextX = c->CheckedDisabled.TextX = 0xFFFFFF;
	} else {
		c = *cp;
	}

	lua_pushnil(l);
	while (lua_next(l, 2)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Size")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			c->Width = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			c->Height = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "Font")) {
			c->Font = FontByIdent(LuaToString(l, -1));
		} else if (!strcmp(value, "TextNormalColor")) {
			free(c->TextNormalColor);
			c->TextNormalColor = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "TextReverseColor")) {
			free(c->TextReverseColor);
			c->TextReverseColor = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "TextPos")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			c->TextX = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			c->TextY = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "TextAlign")) {
			value = LuaToString(l, -1);
			if (!strcmp(value, "Center")) {
				c->TextAlign = TextAlignCenter;
			} else if (!strcmp(value, "Right")) {
				c->TextAlign = TextAlignRight;
			} else if (!strcmp(value, "Left")) {
				c->TextAlign = TextAlignLeft;
			} else {
				LuaError(l, "Invalid text alignment: %s" _C_ value);
			}
		} else if (!strcmp(value, "Default")) {
			ParseButtonStyleProperties(l, &c->Default);
		} else if (!strcmp(value, "Hover")) {
			ParseButtonStyleProperties(l, &c->Hover);
		} else if (!strcmp(value, "Selected")) {
			ParseButtonStyleProperties(l, &c->Selected);
		} else if (!strcmp(value, "Clicked")) {
			ParseButtonStyleProperties(l, &c->Clicked);
		} else if (!strcmp(value, "Disabled")) {
			ParseButtonStyleProperties(l, &c->Disabled);
		} else if (!strcmp(value, "Checked")) {
			ParseButtonStyleProperties(l, &c->Checked);
		} else if (!strcmp(value, "CheckedHover")) {
			ParseButtonStyleProperties(l, &c->CheckedHover);
		} else if (!strcmp(value, "CheckedSelected")) {
			ParseButtonStyleProperties(l, &c->CheckedSelected);
		} else if (!strcmp(value, "CheckedClicked")) {
			ParseButtonStyleProperties(l, &c->CheckedClicked);
		} else if (!strcmp(value, "CheckedDisabled")) {
			ParseButtonStyleProperties(l, &c->CheckedDisabled);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}

	if (c->Default.TextX == 0xFFFFFF) {
		c->Default.TextX = c->TextX;
		c->Default.TextY = c->TextY;
	}
	if (c->Hover.TextX == 0xFFFFFF) {
		c->Hover.TextX = c->TextX;
		c->Hover.TextY = c->TextY;
	}
	if (c->Selected.TextX == 0xFFFFFF) {
		c->Selected.TextX = c->TextX;
		c->Selected.TextY = c->TextY;
	}
	if (c->Clicked.TextX == 0xFFFFFF) {
		c->Clicked.TextX = c->TextX;
		c->Clicked.TextY = c->TextY;
	}
	if (c->Disabled.TextX == 0xFFFFFF) {
		c->Disabled.TextX = c->TextX;
		c->Disabled.TextY = c->TextY;
	}
	if (c->Checked.TextX == 0xFFFFFF) {
		c->Checked.TextX = c->TextX;
		c->Checked.TextY = c->TextY;
	}
	if (c->CheckedHover.TextX == 0xFFFFFF) {
		c->CheckedHover.TextX = c->TextX;
		c->CheckedHover.TextY = c->TextY;
	}
	if (c->CheckedSelected.TextX == 0xFFFFFF) {
		c->CheckedSelected.TextX = c->TextX;
		c->CheckedSelected.TextY = c->TextY;
	}
	if (c->CheckedClicked.TextX == 0xFFFFFF) {
		c->CheckedClicked.TextX = c->TextX;
		c->CheckedClicked.TextY = c->TextY;
	}
	if (c->CheckedDisabled.TextX == 0xFFFFFF) {
		c->CheckedDisabled.TextX = c->TextX;
		c->CheckedDisabled.TextY = c->TextY;
	}

	if (c->Default.TextAlign == TextAlignUndefined) {
		c->Default.TextAlign = c->TextAlign;
	}
	if (c->Hover.TextAlign == TextAlignUndefined) {
		c->Hover.TextAlign = c->TextAlign;
	}
	if (c->Selected.TextAlign == TextAlignUndefined) {
		c->Selected.TextAlign = c->TextAlign;
	}
	if (c->Clicked.TextAlign == TextAlignUndefined) {
		c->Clicked.TextAlign = c->TextAlign;
	}
	if (c->Disabled.TextAlign == TextAlignUndefined) {
		c->Disabled.TextAlign = c->TextAlign;
	}
	if (c->Checked.TextAlign == TextAlignUndefined) {
		c->Checked.TextAlign = c->TextAlign;
	}
	if (c->CheckedHover.TextAlign == TextAlignUndefined) {
		c->CheckedHover.TextAlign = c->TextAlign;
	}
	if (c->CheckedSelected.TextAlign == TextAlignUndefined) {
		c->CheckedSelected.TextAlign = c->TextAlign;
	}
	if (c->CheckedClicked.TextAlign == TextAlignUndefined) {
		c->CheckedClicked.TextAlign = c->TextAlign;
	}
	if (c->CheckedDisabled.TextAlign == TextAlignUndefined) {
		c->CheckedDisabled.TextAlign = c->TextAlign;
	}

	return 0;
}

/**
**  Define a menu
**
**  @param l  Lua state.
*/
static int CclDefineMenu(lua_State* l)
{
	const char* value;
	Menu* menu;
	Menu item;
	char* name;
	void** func;
	int args;
	int j;

	name = NULL;
	TheUI.Offset640X = (VideoWidth - 640) / 2;
	TheUI.Offset480Y = (VideoHeight - 480) / 2;

	//
	// Parse the arguments, already the new tagged format.
	//
	memset(&item, 0, sizeof(Menu));

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;
		if (!strcmp(value, "geometry")) {
			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 4) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, j + 1, 1);
			item.X = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 2);
			item.Y = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 3);
			item.Width = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 4);
			item.Height = LuaToNumber(l, -1);
			lua_pop(l, 1);

		} else if (!strcmp(value, "name")) {
			name = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "panel")) {
			if (strcmp(LuaToString(l, j + 1), "none")) {
				item.Panel = strdup(LuaToString(l, j + 1));
			}
		} else if (!strcmp(value, "background")) {
			item.Background = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "default")) {
			item.DefSel = LuaToNumber(l, j + 1);
/*
		} else if (!strcmp(value, "nitems")) {
			item.nitems = LuaToNumber(l, j + 1);
*/
		} else if (!strcmp(value, "netaction")) {
			value = LuaToString(l, j + 1);
			func = (void**)hash_find(MenuFuncHash, value);
			if (func != NULL) {
				item.NetAction = (void*)*func;
			} else {
				LuaError(l, "Can't find function: %s" _C_ value);
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	if (name) {
		menu = FindMenu(name);
		if (!menu) {
			menu = malloc(sizeof(Menu));
			*(Menu**)hash_add(MenuHash, name) = menu;
		} else {
			int i;
			int mitype;

			free(menu->Panel);
			free(menu->Background);
			VideoSafeFree(menu->BackgroundG);
			for (i = 0; i < menu->NumItems; ++i) {
				mitype = menu->Items[i].MiType;
				if (mitype == MI_TYPE_TEXT) {
					if (menu->Items[i].D.Text.text) {
						free(menu->Items[i].D.Text.text);
					}
					if (menu->Items[i].D.Text.normalcolor) {
						free(menu->Items[i].D.Text.normalcolor);
					}
					if (menu->Items[i].D.Text.reversecolor) {
						free(menu->Items[i].D.Text.normalcolor);
					}
				} else if (mitype == MI_TYPE_BUTTON) {
					if (menu->Items[i].D.Button.Text) {
						free(menu->Items[i].D.Button.Text);
					}
				} else if (mitype == MI_TYPE_PULLDOWN) {
					int j;
					j = menu->Items[i].D.Pulldown.noptions-1;
					for (; j >= 0; --j) {
						free(menu->Items[i].D.Pulldown.options[j]);
					}
					free(menu->Items[i].D.Pulldown.options);
				} else if (mitype == MI_TYPE_LISTBOX) {
				} else if (mitype == MI_TYPE_INPUT) {
				} else if (mitype == MI_TYPE_CHECKBOX) {
				}
			}
			free(menu->Items);
			menu->Items = NULL;
		}
		menu->NumItems = 0; // reset to zero
		memcpy(menu, &item, sizeof(Menu));
		//move the buttons for different resolutions..
		if (VideoWidth != 640) {
			if (VideoWidth == 0) {
				if (DEFAULT_VIDEO_WIDTH != 640) {
					menu->X += (DEFAULT_VIDEO_WIDTH - 640) / 2;
				}
				if (DEFAULT_VIDEO_HEIGHT != 480) {
					menu->Y += (DEFAULT_VIDEO_HEIGHT - 480) / 2;
				}
			} else {
				//printf("VideoWidth = %d\n", VideoWidth);
				menu->X += TheUI.Offset640X;
				menu->Y += TheUI.Offset480Y;
			}
		}
		//printf("Me:%s\n", name);
		free(name);
	} else {
		fprintf(stderr, "Name of menu is missed, skip definition\n");
	}

	return 0;
}

/**
**  FIXME: docu
*/
static int scm2hotkey(lua_State* l, const char* value)
{
	int len;
	int key;
	int f;

	key = 0;
	len = strlen(value);

	if (len == 0) {
		key = 0;
	} else if (len == 1) {
		key = value[0];
	} else if (!strcmp(value, "esc")) {
		key = 27;
	} else if (value[0] == 'f' && len > 1 && len < 4) {
		f = atoi(value + 1);
		if (f > 0 && f < 13) {
			key = KeyCodeF1 + f - 1; // if key-order in include/interface.h is linear
		} else {
			LuaError(l, "Unknown key: %s" _C_ value);
		}
	} else {
		LuaError(l, "Unknown key %s" _C_ value);
	}
	return key;
}

/**
**  FIXME: docu
*/
static int scm2style(lua_State* l, const char* value)
{
	int id;

	if (!strcmp(value, "sc-vslider")) {
		id = MI_STYLE_SC_VSLIDER;
	} else if (!strcmp(value, "sc-hslider")) {
		id = MI_STYLE_SC_HSLIDER;
	} else {
		LuaError(l, "Unsupported style: %s" _C_ value);
		return 0;
	}
	return id;
}

/**
**  FIXME: docu
*/
static int CclDefineMenuItem(lua_State* l)
{
	const char* value;
	char* s1;
	char* name;
	Menuitem *item;
	Menu** tmp;
	Menu* menu;
	void** func;
	int args;
	int subargs;
	int j;
	int k;

	name = NULL;
	item = (Menuitem*)calloc(1, sizeof(Menuitem));

	//
	// Parse the arguments, already the new tagged format.
	//
	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;
		if (!strcmp(value, "pos")) {
			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, j + 1, 1);
			item->XOfs = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 2);
			item->YOfs = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "menu")) {
			name = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "flags")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);

				if (!strcmp(value, "active")) {
					item->Flags |= MenuButtonActive;
				} else if (!strcmp(value, "clicked")) {
					item->Flags |= MenuButtonClicked;
				} else if (!strcmp(value, "selected")) {
					item->Flags |= MenuButtonSelected;
				} else if (!strcmp(value, "disabled")) {
					item->Flags |= MenuButtonDisabled;
				} else {
					LuaError(l, "Unknown flag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "font")) {
			item->Font = FontByIdent(LuaToString(l, j + 1));
		} else if (!strcmp(value, "init")) {
			if (!lua_isstring(l, j + 1) && !lua_isnil(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			if (lua_isstring(l, j + 1)) {
				value = lua_tostring(l, j + 1);
				func = (void**)hash_find(MenuFuncHash, value);
				if (func != NULL) {
					item->InitFunc = (void*)*func;
				} else {
					LuaError(l, "Can't find function: %s" _C_ value);
				}
			} else {
				item->InitFunc = NULL;
			}
		} else if (!strcmp(value, "exit")) {
			if (!lua_isstring(l, j + 1) && !lua_isnil(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			if (lua_isstring(l, j + 1)) {
				value = lua_tostring(l, j + 1);
				func = (void**)hash_find(MenuFuncHash, value);
				if (func != NULL) {
					item->ExitFunc = (void*)*func;
				} else {
					LuaError(l, "Can't find function: %s" _C_ value);
				}
			} else {
				item->ExitFunc = NULL;
			}
/* Menu types */
		} else if (!item->MiType) {
			if (!strcmp(value, "text")) {
				if (!lua_istable(l, j + 1)) {
					LuaError(l, "incorrect argument");
				}
				item->MiType = MI_TYPE_TEXT;
				item->D.Text.text = NULL;

				subargs = luaL_getn(l, j + 1);
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, j + 1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					++k;
					if (!strcmp(value, "align")) {
						lua_rawgeti(l, j + 1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						if (!strcmp(value, "left")) {
							item->D.Text.align = MI_TFLAGS_LALIGN;
						} else if (!strcmp(value, "right")) {
							item->D.Text.align = MI_TFLAGS_RALIGN;
						} else if (!strcmp(value, "center")) {
							item->D.Text.align = MI_TFLAGS_CENTERED;
						}
					} else if (!strcmp(value, "caption")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							s1 = strdup(lua_tostring(l, -1));
							item->D.Text.text = s1;
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.Text.text = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "func")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.Text.action = (void*)*func;
							} else {
								LuaError(l, "Can't find function: %s" _C_ value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.Text.action = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "color-normal")) {
						lua_rawgeti(l, j + 1, k + 1);
						s1 = strdup(LuaToString(l, -1));
						lua_pop(l, 1);
						item->D.Text.normalcolor = s1;
					} else if (!strcmp(value, "color-reverse")) {
						lua_rawgeti(l, j + 1, k + 1);
						s1 = strdup(LuaToString(l, -1));
						lua_pop(l, 1);
						item->D.Text.reversecolor = s1;
					} else {
						LuaError(l, "Unsupported property: %s" _C_ value);
					}
				}
			} else if (!strcmp(value, "button")) {
				if (!lua_istable(l, j + 1)) {
					LuaError(l, "incorrect argument");
				}
				item->MiType = MI_TYPE_BUTTON;

				subargs = luaL_getn(l, j + 1);
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, j + 1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					++k;

					if (!strcmp(value, "caption")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							s1 = strdup(lua_tostring(l, -1));
							item->D.Button.Text = s1;
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.Button.Text = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "hotkey")) {
						lua_rawgeti(l, j + 1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						item->D.Button.HotKey = scm2hotkey(l, value);
					} else if (!strcmp(value, "func")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							//item->D.Button.Handler = hash_mini_get(MenuHndlrHash, s1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.Button.Handler = (void*)*func;
							} else {
								LuaError(l, "Can't find function: %s" _C_ value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.Button.Handler = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "style")) {
						lua_rawgeti(l, j + 1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						item->D.Button.Style = FindButtonStyle(value);
						if (!item->D.Button.Style) {
							LuaError(l, "Invalid button style: %s" _C_ value);
						}
					} else {
						LuaError(l, "Unsupported property: %s" _C_ value);
					}
				}
			} else if (!strcmp(value, "pulldown")) {
				if (!lua_istable(l, j + 1)) {
					LuaError(l, "incorrect argument");
				}
				item->MiType = MI_TYPE_PULLDOWN;

				subargs = luaL_getn(l, j + 1);
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, j + 1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					++k;
					if (!strcmp(value, "size")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
							LuaError(l, "incorrect argument");
						}
						lua_rawgeti(l, -1, 1);
						item->D.Pulldown.xsize = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_rawgeti(l, -1, 2);
						item->D.Pulldown.ysize = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "options")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_istable(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}

						if (!lua_isnil(l, -1)) {
							int subsubargs;
							int subk;

							subsubargs = luaL_getn(l, -1);
							item->D.Pulldown.noptions = subsubargs;
							if (item->D.Pulldown.options) {
									free(item->D.Pulldown.options);
							}
							item->D.Pulldown.options = (unsigned char**)malloc(sizeof(unsigned char*) * subsubargs);
							for (subk = 0; subk < subsubargs; ++subk) {
								lua_rawgeti(l, -1, subk + 1);
								s1 = strdup(LuaToString(l, -1));
								lua_pop(l, 1);
								item->D.Pulldown.options[subk] = s1;
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "func")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.Pulldown.action = (void*)*func;
							} else {
								LuaError(l, "Can't find function: %s" _C_ value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.Pulldown.action = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "style")) {
						lua_rawgeti(l, j + 1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						item->D.Pulldown.button = scm2buttonid(l, value);
					} else if (!strcmp(value, "state")) {
						lua_rawgeti(l, j + 1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						if (!strcmp(value, "passive")) {
							item->D.Pulldown.state = MI_PSTATE_PASSIVE;
						} else {
							LuaError(l, "Unsupported property: %s" _C_ value);
						}
					} else if (!strcmp(value, "default")) {
						lua_rawgeti(l, j + 1, k + 1);
						item->D.Pulldown.defopt = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "current")) {
						lua_rawgeti(l, j + 1, k + 1);
						item->D.Pulldown.curopt = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else {
						LuaError(l, "Unsupported property: %s" _C_ value);
					}
				}
			} else if (!strcmp(value, "listbox")) {
				if (!lua_istable(l, j + 1)) {
					LuaError(l, "incorrect argument");
				}
				item->MiType = MI_TYPE_LISTBOX;

				subargs = luaL_getn(l, j + 1);
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, j + 1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					++k;
					if (!strcmp(value, "size")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
							LuaError(l, "incorrect argument");
						}
						lua_rawgeti(l, -1, 1);
						item->D.Listbox.xsize = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_rawgeti(l, -1, 2);
						item->D.Listbox.ysize = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "func")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.Listbox.action = (void*)*func;
							} else {
								LuaError(l, "Can't find function: %s" _C_ value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.Listbox.action = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "handler")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.Listbox.handler = (void*)*func;
							} else {
								LuaError(l, "Can't find function: %s" _C_ value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.Listbox.handler = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "retopt")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.Listbox.retrieveopt = (void*)(*func);
							} else {
								LuaError(l, "Can't find function: %s" _C_ value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.Listbox.retrieveopt = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "style")) {
						lua_rawgeti(l, j + 1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						item->D.Listbox.button = scm2buttonid(l, value);
					} else if (!strcmp(value, "default")) {
						lua_rawgeti(l, j + 1, k + 1);
						item->D.Listbox.defopt = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "startline")) {
						lua_rawgeti(l, j + 1, k + 1);
						item->D.Listbox.startline = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "nlines")) {
						lua_rawgeti(l, j + 1, k + 1);
						item->D.Listbox.nlines = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "current")) {
						lua_rawgeti(l, j + 1, k + 1);
						item->D.Listbox.curopt = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else {
						LuaError(l, "Unsupported property: %s" _C_ value);
					}
				}
			} else if (!strcmp(value, "vslider")) {
				if (!lua_istable(l, j + 1)) {
					LuaError(l, "incorrect argument");
				}
				item->MiType = MI_TYPE_VSLIDER;
				item->D.VSlider.defper = -1;

				subargs = luaL_getn(l, j + 1);
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, j + 1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					++k;
					if (!strcmp(value, "size")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
							LuaError(l, "incorrect argument");
						}
						lua_rawgeti(l, -1, 1);
						item->D.VSlider.xsize = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_rawgeti(l, -1, 2);
						item->D.VSlider.ysize = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "flags")) {
						int subk;
						int subsubargs;

						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument");
						}

						subsubargs = luaL_getn(l, -1);
						for (subk = 0; subk < subsubargs; ++subk) {
							lua_rawgeti(l, -1, subk + 1);
							value = LuaToString(l, -1);
							lua_pop(l, 1);

							if (!strcmp(value, "up")) {
								item->D.VSlider.cflags |= MI_CFLAGS_UP;
							} else if (!strcmp(value, "down")) {
								item->D.VSlider.cflags |= MI_CFLAGS_DOWN;
							} else if (!strcmp(value, "left")) {
								item->D.VSlider.cflags |= MI_CFLAGS_LEFT;
							} else if (!strcmp(value, "right")) {
								item->D.VSlider.cflags |= MI_CFLAGS_RIGHT;
							} else if (!strcmp(value, "knob")) {
								item->D.VSlider.cflags |= MI_CFLAGS_KNOB;
							} else if (!strcmp(value, "cont")) {
								item->D.VSlider.cflags |= MI_CFLAGS_CONT;
							} else {
								LuaError(l, "Unknown flag: %s" _C_ value);
							}
						}
					} else if (!strcmp(value, "func")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.VSlider.action = (void*)*func;
							} else {
								lua_pushfstring(l, "Can't find function: %s", value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.VSlider.action = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "handler")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.VSlider.handler = (void*)*func;
							} else {
								LuaError(l, "Can't find function: %s" _C_ value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.VSlider.handler = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "default")) {
						lua_rawgeti(l, j + 1, k + 1);
						item->D.VSlider.defper = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "current")) {
						lua_rawgeti(l, j + 1, k + 1);
						item->D.VSlider.percent = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "style")) {
						lua_rawgeti(l, j + 1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						item->D.VSlider.style = scm2style(l, value);
					} else {
						LuaError(l, "Unsupported property: %s" _C_ value);
					}
				}
			} else if (!strcmp(value, "drawfunc")) {
				if (!lua_isstring(l, j + 1) && !lua_isnil(l, j + 1)) {
					LuaError(l, "incorrect argument");
				}
				item->MiType = MI_TYPE_DRAWFUNC;

				if (lua_isstring(l, j + 1)) {
					value = lua_tostring(l, j + 1);
					func = (void**)hash_find(MenuFuncHash, value);
					if (func != NULL) {
						item->D.DrawFunc.draw = (void*)*func;
					} else {
						LuaError(l, "Can't find function: %s" _C_ value);
					}
				} else {
					item->D.DrawFunc.draw = NULL;
				}
			} else if (!strcmp(value, "input")) {
				if (!lua_istable(l, j + 1)) {
					LuaError(l, "incorrect argument");
				}
				item->MiType = MI_TYPE_INPUT;

				subargs = luaL_getn(l, j + 1);
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, j + 1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					++k;
					if (!strcmp(value, "size")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
							LuaError(l, "incorrect argument");
						}
						lua_rawgeti(l, -1, 1);
						item->D.Input.xsize = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_rawgeti(l, -1, 2);
						item->D.Input.ysize = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "func")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.Input.action = (void*)*func;
							} else {
								lua_pushfstring(l, "Can't find function: %s", value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.Input.action = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "style")) {
						lua_rawgeti(l, j + 1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						item->D.Input.button = scm2buttonid(l, value);
					} else if (!strcmp(value, "maxch")) {
						lua_rawgeti(l, j + 1, k + 1);
						item->D.Input.maxch = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "color-normal")) {
						lua_rawgeti(l, j + 1, k + 1);
						s1 = strdup(LuaToString(l, -1));
						lua_pop(l, 1);
						item->D.Input.normalcolor = s1;
					} else if (!strcmp(value, "color-reverse")) {
						lua_rawgeti(l, j + 1, k + 1);
						s1 = strdup(LuaToString(l, -1));
						lua_pop(l, 1);
						item->D.Input.reversecolor = s1;
					} else {
						LuaError(l, "Unsupported property: %s" _C_ value);
					}
				}
			} else if (!strcmp(value, "checkbox")) {
				if (!lua_istable(l, j + 1)) {
					LuaError(l, "incorrect argument");
				}
				item->MiType = MI_TYPE_CHECKBOX;

				subargs = luaL_getn(l, j + 1);
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, j + 1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					++k;
					if (!strcmp(value, "state")) {
						lua_rawgeti(l, j + 1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						if (!strcmp(value, "unchecked")) {
							item->D.Checkbox.State = MI_CSTATE_UNCHECKED;
						} else if (!strcmp(value, "passive")) {
							item->D.Checkbox.State = MI_CSTATE_PASSIVE;
						} else if (!strcmp(value, "invisible")) {
							item->D.Checkbox.State = MI_CSTATE_INVISIBLE;
						} else if (!strcmp(value, "checked")) {
							item->D.Checkbox.State = MI_CSTATE_CHECKED;
						}
					} else if (!strcmp(value, "func")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.Checkbox.Action = (void*)*func;
							} else {
								lua_pushfstring(l, "Can't find function: %s", value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.Checkbox.Action = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "style")) {
						lua_rawgeti(l, j + 1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						item->D.Checkbox.Style = FindCheckboxStyle(value);
						if (!item->D.Checkbox.Style) {
							LuaError(l, "Invalid button style: %s" _C_ value);
						}
					} else if (!strcmp(value, "text")) {
						lua_rawgeti(l, j + 1, k + 1);
						s1 = strdup(LuaToString(l, -1));
						lua_pop(l, 1);
						item->D.Checkbox.Text = s1;
					} else {
						LuaError(l, "Unsupported property: %s" _C_ value);
					}
				}
			} else if (!strcmp(value, "hslider")) {
				if (!lua_istable(l, j + 1)) {
					LuaError(l, "incorrect argument");
				}
				item->MiType = MI_TYPE_HSLIDER;
				item->D.HSlider.defper = -1;

				subargs = luaL_getn(l, j + 1);
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, j + 1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					++k;
					if (!strcmp(value, "size")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
							LuaError(l, "incorrect argument");
						}
						lua_rawgeti(l, -1, 1);
						item->D.HSlider.xsize = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_rawgeti(l, -1, 2);
						item->D.HSlider.ysize = LuaToNumber(l, -1);
						lua_pop(l, 1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "flags")) {
						int subk;
						int subsubargs;

						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument");
						}

						subsubargs = luaL_getn(l, -1);
						for (subk = 0; subk < subsubargs; ++subk) {
							lua_rawgeti(l, -1, subk + 1);
							value = LuaToString(l, -1);
							lua_pop(l, 1);

							if (!strcmp(value, "up")) {
								item->D.HSlider.cflags |= MI_CFLAGS_UP;
							} else if (!strcmp(value, "down")) {
								item->D.HSlider.cflags |= MI_CFLAGS_DOWN;
							} else if (!strcmp(value, "left")) {
								item->D.HSlider.cflags |= MI_CFLAGS_LEFT;
							} else if (!strcmp(value, "right")) {
								item->D.HSlider.cflags |= MI_CFLAGS_RIGHT;
							} else if (!strcmp(value, "knob")) {
								item->D.HSlider.cflags |= MI_CFLAGS_KNOB;
							} else if (!strcmp(value, "cont")) {
								item->D.HSlider.cflags |= MI_CFLAGS_CONT;
							} else {
								LuaError(l, "Unknown flag: %s" _C_ value);
							}
						}
					} else if (!strcmp(value, "func")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.HSlider.action = (void*)*func;
							} else {
								LuaError(l, "Can't find function: %s" _C_ value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.HSlider.action = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "handler")) {
						lua_rawgeti(l, j + 1, k + 1);
						if (!lua_isstring(l, -1) && !lua_isnil(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						if (lua_isstring(l, -1)) {
							value = lua_tostring(l, -1);
							func = (void**)hash_find(MenuFuncHash, value);
							if (func != NULL) {
								item->D.HSlider.handler = (void*)*func;
							} else {
								LuaError(l, "Can't find function: %s" _C_ value);
							}
						} else {
							lua_pushnumber(l, 0);
							lua_rawseti(l, j + 1, k + 1);
							subargs = luaL_getn(l, j + 1);
							item->D.HSlider.handler = NULL;
						}
						lua_pop(l, 1);
					} else if (!strcmp(value, "default")) {
						lua_rawgeti(l, j + 1, k + 1);
						item->D.HSlider.defper = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "current")) {
						lua_rawgeti(l, j + 1, k + 1);
						item->D.HSlider.percent = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "style")) {
						lua_rawgeti(l, j + 1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						item->D.HSlider.style = scm2style(l, value);
					} else {
						LuaError(l, "Unsupported property: %s" _C_ value);
					}
				}
			} else {
				LuaError(l, "Unsupported tag: %s" _C_ value);
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	if ((tmp = (Menu**)hash_find(MenuHash, name))) {
		menu = *tmp;
		if (menu->Items) {
			menu->Items = (Menuitem*)realloc(menu->Items, sizeof(Menuitem) * (menu->NumItems + 1));
		} else {
			menu->Items = (Menuitem*)malloc(sizeof(Menuitem));
		}
		item->Menu = menu;
		memcpy(menu->Items + menu->NumItems, item, sizeof(Menuitem));
		menu->NumItems++;
	}
	free(name);
	free(item);


	return 0;
}

/**
**  Define menu graphics
**
**  @param l  Lua state.
*/
static int CclDefineMenuGraphics(lua_State* l)
{
	int i;
	int j;
	int t;
	int tables;
	int args;
	const char* value;

	if (lua_gettop(l) != 1 || !lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

	tables = luaL_getn(l, 1);
	i = 0;

	for (t = 0; t < tables; ++t) {
		lua_rawgeti(l, 1, t + 1);
		args = luaL_getn(l, -1);
		if (!lua_istable(l, -1)) {
			LuaError(l, "incorrect argument");
		}
		for (j = 0; j < args; ++j) {
			lua_rawgeti(l, -1, j + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			if (!strcmp(value, "file")) {
				++j;
				lua_rawgeti(l, -1, j + 1);
				if (MenuButtonGfx.File[i]) {
					free(MenuButtonGfx.File[i]);
				}
				MenuButtonGfx.File[i] = strdup(LuaToString(l, -1));
				lua_pop(l, 1);
			} else if (!strcmp(value, "size")) {
				++j;
				lua_rawgeti(l, -1, j + 1);
				if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
					LuaError(l, "incorrect argument");
				}
				lua_rawgeti(l, -1, 1);
				MenuButtonGfx.Width[i] = LuaToNumber(l, -1);
				lua_pop(l, 1);
				lua_rawgeti(l, -1, 2);
				MenuButtonGfx.Height[i] = LuaToNumber(l, -1);
				lua_pop(l, 1);
				lua_pop(l, 1);
			} else {
				LuaError(l, "incorrect argument");
			}
		}
		++i;
		lua_pop(l, 1);
	}

	return 0;
}

/**
**  Define a button.
**
**  @param l  Lua state.
*/
static int CclDefineButton(lua_State* l)
{
	char buf[64];
	const char* value;
	char* s1;
	const char* s2;
	ButtonAction ba;

	if (lua_gettop(l) != 1 || !lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

	memset(&ba, 0, sizeof(ba));
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
			ba.Icon.Name = strdup(LuaToString(l, -1));
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
			if (lua_isnumber(l, -1)) {
				sprintf(buf, "%ld", (long int)lua_tonumber(l, -1));
				s1 = strdup(buf);
			} else {
				s1 = strdup(lua_tostring(l, -1));
			}
			ba.ValueStr = s1;
		} else if (!strcmp(value, "Allowed")) {
			value = LuaToString(l, -1);
			if (!strcmp(value, "check-true")) {
				ba.Allowed = ButtonCheckTrue;
			} else if (!strcmp(value, "check-false")) {
				ba.Allowed = ButtonCheckFalse;
			} else if (!strcmp(value, "check-upgrade")) {
				ba.Allowed = ButtonCheckUpgrade;
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
			int subargs;
			int k;

			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			s1 = strdup("");
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				s2 = LuaToString(l, -1);
				lua_pop(l, 1);
				s1 = realloc(s1, strlen(s1) + strlen(s2) + 2);
				strcat(s1, s2);
				if (k != subargs - 1) {
					strcat(s1, ",");
				}
			}
			ba.AllowStr = s1;
		} else if (!strcmp(value, "Key")) {
			ba.Key = *LuaToString(l, -1);
		} else if (!strcmp(value, "Hint")) {
			ba.Hint = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "ForUnit")) {
			int subargs;
			int k;

			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			// FIXME: ba.UnitMask shouldn't be a string
			s1 = strdup(",");
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				s2 = LuaToString(l, -1);
				lua_pop(l, 1);
				s1 = realloc(s1, strlen(s1) + strlen(s2) + 2);
				strcat(s1, s2);
				strcat(s1, ",");
			}
			ba.UnitMask = s1;
			if (!strncmp(ba.UnitMask, ",*,", 3)) {
				free(ba.UnitMask);
				ba.UnitMask = strdup("*");
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}
	AddButton(ba.Pos, ba.Level, ba.Icon.Name, ba.Action, ba.ValueStr,
		ba.Allowed, ba.AllowStr, ba.Key, ba.Hint, ba.UnitMask);
	if (ba.ValueStr) {
		free(ba.ValueStr);
	}
	if (ba.AllowStr) {
		free(ba.AllowStr);
	}
	if (ba.Hint) {
		free(ba.Hint);
	}
	if (ba.UnitMask) {
		free(ba.UnitMask);
	}

	return 0;
}

/**
**  Run the set-selection-changed-hook.
*/
void SelectionChanged(void)
{
	UpdateButtonPanel();
}

/**
**  The selected unit has been altered.
*/
void SelectedUnitChanged(void)
{
	UpdateButtonPanel();
}

/**
**  The next 6 functions set color cycling index
**
**  @param l  Lua state.
*/
static int CclSetColorWaterCycleStart(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	ColorWaterCycleStart = LuaToNumber(l, 1);

	lua_pushnumber(l, ColorWaterCycleStart);
	return 1;
}

/**
**  FIXME: docu
**
**  @param l  Lua state.
*/
static int CclSetColorWaterCycleEnd(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	ColorWaterCycleEnd = LuaToNumber(l, 1);

	lua_pushnumber(l, ColorWaterCycleEnd);
	return 1;
}

/**
**  FIXME: docu
**
**  @param l  Lua state.
*/
static int CclSetColorIconCycleStart(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	ColorIconCycleStart = LuaToNumber(l, 1);

	lua_pushnumber(l, ColorIconCycleStart);
	return 1;
}

/**
**  FIXME: docu
**
**  @param l  Lua state.
*/
static int CclSetColorIconCycleEnd(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	ColorIconCycleEnd = LuaToNumber(l, 1);

	lua_pushnumber(l, ColorIconCycleEnd);
	return 1;
}

/**
**  FIXME: docu
**
**  @param l  Lua state.
*/
static int CclSetColorBuildingCycleStart(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	ColorBuildingCycleStart = LuaToNumber(l, 1);

	lua_pushnumber(l, ColorBuildingCycleStart);
	return 1;
}

/**
**  FIXME: docu
**
**  @param l  Lua state.
*/
static int CclSetColorBuildingCycleEnd(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	ColorBuildingCycleEnd = LuaToNumber(l, 1);

	lua_pushnumber(l, ColorBuildingCycleEnd);
	return 1;
}

/**
**  Set double-click delay.
**
**  @param l  Lua state.
**  @return   Old delay
*/
static int CclSetDoubleClickDelay(lua_State* l)
{
	lua_Number i;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	i = DoubleClickDelay;
	DoubleClickDelay = LuaToNumber(l, 1);

	lua_pushnumber(l, i);
	return 1;
}

/**
**  Set hold-click delay.
**
**  @param l  Lua state.
**
**  @return   Old delay
*/
static int CclSetHoldClickDelay(lua_State* l)
{
	lua_Number i;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	i = HoldClickDelay;
	HoldClickDelay = LuaToNumber(l, 1);

	lua_pushnumber(l, i);
	return 1;
}

/**
**  Set selection style.
**
**  @param l  Lua state.
**
**  @return   Old style
*/
static int CclSetSelectionStyle(lua_State* l)
{
	char* old;
	const char* style;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	old = NULL;

	style = LuaToString(l, 1);
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

	lua_pushstring(l, old);
	free(old);
	return 1;
}

/**
**  Set display of sight range.
**
**  @param l  Lua state.
**
**  @return   The old state of display of sight.
*/
static int CclSetShowSightRange(lua_State* l)
{
	lua_Number old;
	int args;

	args = lua_gettop(l);
	if (args > 1 || (args == 1 &&
			(!lua_isnil(l, 1) && !lua_isboolean(l, 1) && !lua_isstring(l, 1)))) {
		LuaError(l, "incorrect argument");
	}

	old = ShowSightRange;
	if (args == 1 && !lua_isnil(l, 1)) {
		if (lua_isstring(l, 1)) {
			const char* flag;

			flag = lua_tostring(l, 1);
			if (!strcmp(flag, "rectangle")) {
				ShowSightRange = 1;
			} else if (!strcmp(flag, "circle")) {
				ShowSightRange = 2;
			} else {
				LuaError(l, "Unsupported selection style");
			}
		} else {
			int flag;

			flag = lua_toboolean(l, 1);
			if (flag) {
				ShowSightRange = 3;
			} else {
				ShowSightRange = 0;
			}
		}
	} else {
		ShowSightRange = 0;
	}

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Set display of reaction range.
**
**  @param l  Lua state.
**
**  @return   The old state of display of reaction.
*/
static int CclSetShowReactionRange(lua_State* l)
{
	lua_Number old;

	old = ShowReactionRange;

	if (lua_gettop(l) != 1 || (!lua_isboolean(l, 1) && !lua_isstring(l, 1))) {
		LuaError(l, "incorrect argument");
	}
	if (lua_isstring(l, 1)) {
		const char* flag;

		flag = lua_tostring(l, 1);
		if (!strcmp(flag, "rectangle")) {
			ShowReactionRange = 1;
		} else if (!strcmp(flag, "circle")) {
			ShowReactionRange = 2;
		} else {
			LuaError(l, "Unsupported selection style");
		}
	} else {
		int flag;

		flag = lua_toboolean(l, 1);
		if (flag) {
			ShowReactionRange = 3;
		} else {
			ShowReactionRange = 0;
		}
	}

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Set display of attack range.
**
**  @param l  Lua state.
**
**  @return   The old state of display of attack.
*/
static int CclSetShowAttackRange(lua_State* l)
{
	int old;

	old = ShowAttackRange;
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	ShowAttackRange = LuaToBoolean(l, 1);

	lua_pushboolean(l, old);
	return 1;
}

/**
**  Set display of orders.
**
**  @param l  Lua state.
**
**  @return   The old state of display of orders.
*/
static int CclSetShowOrders(lua_State* l)
{
	int old;

	old = ShowOrders;
	if (lua_gettop(l) != 1 || (!lua_isboolean(l, 1) && !lua_isnumber(l, 1))) {
		LuaError(l, "incorrect argument");
	}
	if (lua_isboolean(l, 1)) {
		ShowOrders = lua_toboolean(l, 1);
		if (ShowOrders) {
			ShowOrders = SHOW_ORDERS_ALWAYS;
		}
	} else {
		ShowOrders = lua_tonumber(l, 1);
	}

	lua_pushboolean(l, old);
	return 1;
}

/**
**  Add a new message.
**
**  @param l  Lua state.
*/
static int CclAddMessage(lua_State* l)
{
	const char* str;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	str = LuaToString(l, 1);
	SetMessage("%s", str);

	return 0;
}

/**
**  Reset the keystroke help array
**
**  @param l  Lua state.
*/
static int CclResetKeystrokeHelp(lua_State* l)
{
	int n;

	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	n = nKeyStrokeHelps * 2;
	while (n--) {
		free(KeyStrokeHelps[n]);
	}
	if (KeyStrokeHelps) {
		free(KeyStrokeHelps);
		KeyStrokeHelps = NULL;
	}
	nKeyStrokeHelps = 0;

	return 0;
}

/**
**  FIXME: docu
**
**  @param l  Lua state.
*/
static int CclSetGroupKeys(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	UiGroupKeys = strdup(LuaToString(l, 1));
	return 0;
}

/**
**  Add a keystroke help
**
**  @param l  Lua state.
*/
static int CclAddKeystrokeHelp(lua_State* l)
{
	char* s1;
	char* s2;
	int n;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}

	s1 = strdup(LuaToString(l, 1));
	s2 = strdup(LuaToString(l, 2));

	n = nKeyStrokeHelps;
	if (!n) {
		n = 1;
		KeyStrokeHelps = malloc(2 * sizeof(char*));
	} else {
		++n;
		KeyStrokeHelps = realloc(KeyStrokeHelps, n * 2 * sizeof(char*));
	}
	if (KeyStrokeHelps) {
		nKeyStrokeHelps = n;
		--n;
		KeyStrokeHelps[n * 2] = s1;
		KeyStrokeHelps[n * 2 + 1] = s2;
	}

	return 0;
}

/**
**  Register CCL features for UI.
*/
void UserInterfaceCclRegister(void)
{
	lua_register(Lua, "AddMessage", CclAddMessage);

	lua_register(Lua, "SetColorCycleAll", CclSetColorCycleAll);
	lua_register(Lua, "SetMouseScrollSpeedDefault", CclSetMouseScrollSpeedDefault);
	lua_register(Lua, "SetMouseScrollSpeedControl", CclSetMouseScrollSpeedControl);

	lua_register(Lua, "SetClickMissile", CclSetClickMissile);
	lua_register(Lua, "SetDamageMissile", CclSetDamageMissile);

	lua_register(Lua, "SetVideoResolution", CclSetVideoResolution);
	lua_register(Lua, "SetVideoFullScreen", CclSetVideoFullScreen);

	lua_register(Lua, "SetTitleScreens", CclSetTitleScreens);
	lua_register(Lua, "SetMenuMusic", CclSetMenuMusic);

	lua_register(Lua, "DisplayPicture", CclDisplayPicture);
	lua_register(Lua, "ProcessMenu", CclProcessMenu);

	lua_register(Lua, "DefineCursor", CclDefineCursor);
	lua_register(Lua, "SetGameCursor", CclSetGameCursor);
	lua_register(Lua, "DefineUI", CclDefineUI);
	lua_register(Lua, "DefineViewports", CclDefineViewports);

	lua_register(Lua, "SetGrabMouse", CclSetGrabMouse);
	lua_register(Lua, "SetLeaveStops", CclSetLeaveStops);
	lua_register(Lua, "SetKeyScroll", CclSetKeyScroll);
	lua_register(Lua, "SetKeyScrollSpeed", CclSetKeyScrollSpeed);
	lua_register(Lua, "SetMouseScroll", CclSetMouseScroll);
	lua_register(Lua, "SetMouseScrollSpeed", CclSetMouseScrollSpeed);

	lua_register(Lua, "SetShowCommandKey", CclSetShowCommandKey);
	lua_register(Lua, "RightButtonAttacks", CclRightButtonAttacks);
	lua_register(Lua, "RightButtonMoves", CclRightButtonMoves);
	lua_register(Lua, "SetFancyBuildings", CclSetFancyBuildings);

	lua_register(Lua, "DefineButton", CclDefineButton);

	lua_register(Lua, "DefineButtonStyle", CclDefineButtonStyle);
	lua_register(Lua, "DefineCheckboxStyle", CclDefineCheckboxStyle);

	lua_register(Lua, "DefineMenuItem", CclDefineMenuItem);
	lua_register(Lua, "DefineMenu", CclDefineMenu);
	lua_register(Lua, "DefineMenuGraphics", CclDefineMenuGraphics);

	//
	// Color cycling
	//
	lua_register(Lua, "SetColorWaterCycleStart", CclSetColorWaterCycleStart);
	lua_register(Lua, "SetColorWaterCycleEnd", CclSetColorWaterCycleEnd);
	lua_register(Lua, "SetColorIconCycleStart", CclSetColorIconCycleStart);
	lua_register(Lua, "SetColorIconCycleEnd", CclSetColorIconCycleEnd);
	lua_register(Lua, "SetColorBuildingCycleStart", CclSetColorBuildingCycleStart);
	lua_register(Lua, "SetColorBuildingCycleEnd", CclSetColorBuildingCycleEnd);

	//
	// Correct named functions
	//
	lua_register(Lua, "SetDoubleClickDelay", CclSetDoubleClickDelay);
	lua_register(Lua, "SetHoldClickDelay", CclSetHoldClickDelay);

	//
	// Look and feel of units
	//
	lua_register(Lua, "SetSelectionStyle", CclSetSelectionStyle);
	lua_register(Lua, "SetShowSightRange", CclSetShowSightRange);
	lua_register(Lua, "SetShowReactionRange", CclSetShowReactionRange);
	lua_register(Lua, "SetShowAttackRange", CclSetShowAttackRange);
	lua_register(Lua, "SetShowOrders", CclSetShowOrders);

	//
	// Keystroke helps
	//
	lua_register(Lua, "ResetKeystrokeHelp", CclResetKeystrokeHelp);
	lua_register(Lua, "AddKeystrokeHelp", CclAddKeystrokeHelp);
	lua_register(Lua, "SetGroupKeys", CclSetGroupKeys);

	InitMenuFuncHash();
}

//@}
