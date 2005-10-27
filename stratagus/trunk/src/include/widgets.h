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
/**@name widgets.h - The widgets headerfile. */
//
//      (c) Copyright 1998-2005 by François Beerten
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

#include <guichan.h>
#include <guichan/gsdl.h>
typedef int lua_Object; // from tolua++.h

extern gcn::Container *top;

void initGuichan(int w, int h);
void freeGuichan();
void handleInput(const SDL_Event *event);

class LuaActionListener : public gcn::ActionListener
{
	lua_State *luastate;
	int luaref;
public:
	//LuaActionListener(lua_State *lua, int luaref) : luastate(lua), luaref(luaref) {}
	LuaActionListener(lua_State *lua, lua_Object luaref);
	virtual void action(const std::string &eventId);
	~LuaActionListener();
};

class ImageButton : public gcn::Button
{
public:
	gcn::Image *normalImage;
	gcn::Image *pressedImage;
	ImageButton(char *caption, gcn::Image *normal = NULL, gcn::Image *pressed = NULL);

	virtual void draw(gcn::Graphics *graphics);
	virtual void adjustSize();
};

class ImageWidget : public gcn::Icon
{
public:
	ImageWidget(gcn::Image *img) : gcn::Icon(img) {}
};

class LuaListModel : public gcn::ListModel
{
	std::vector<std::string> list;
public:
	LuaListModel() {}

	void setList(lua_State *lua, lua_Object *lo); 
	virtual int getNumberOfElements() {return list.size();}
	virtual std::string getElementAt(int i) {return list[i];}
};

class ListBoxWidget : public gcn::ListBox
{
	LuaListModel listmodel;
public:
	ListBoxWidget();
	void setList(lua_State *lua, lua_Object *lo);
};
