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
/**@name widgets.cpp - The stratagus ui widgets. */
//
//      (c) Copyright 2005 by François Beerten and Jimmy Salmon
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
#include "video.h"
#include "font.h"
#include "cursor.h"
#include "ui.h"
#include "widgets.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

// Guichan stuff we need
gcn::Gui *gui;         /// A Gui object - binds it all together
gcn::SDLInput *input;  /// Input driver

bool guichanActive = true;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Initializes the GUI stuff
**
**  @param width   FIXME: docu
**  @param height  FIXME: docu
*/
void initGuichan(int width, int height)
{
	// FIXME: opengl
	gcn::SDLGraphics *graphics;   // Graphics driver

#if 0
	// We want unicode
	SDL_EnableUNICODE(1);
#endif

	graphics = new gcn::SDLGraphics();
	// Set the target for the graphics object to be the screen.
	// In other words, we will draw to the screen.
	// Note, any surface will do, it doesn't have to be the screen.
	graphics->setTarget(TheScreen);
	input = new gcn::SDLInput();
	
	gui = new gcn::Gui();
	gui->setGraphics(graphics);
	gui->setInput(input);
	gui->setTop(NULL);

	guichanActive = true;
}

/**
**  FIXME: docu
*/
void freeGuichan() 
{
	// FIXME do a full cleanup
	delete gui;
	delete input;

	gui = NULL;
	input = NULL;
}

/**
**  FIXME: docu
**
**  @param event  FIXME: docu
*/
void handleInput(const SDL_Event *event) 
{
	if (input && guichanActive) {
		input->pushInput(*event);
	}
}

void DrawGuichanWidgets() 
{
	if (gui && guichanActive) {
		gui->logic();
		gui->draw();
	}
}

/*----------------------------------------------------------------------------
--  LuaActionListener
----------------------------------------------------------------------------*/


/**
**  FIXME: docu
**
**  @param l  FIXME: docu
**  @param f  FIXME: docu
*/
LuaActionListener::LuaActionListener(lua_State *l, lua_Object f) :
	luastate(l)
{
	if (!lua_isfunction(l, f)) {
		LuaError(l, "Argument isnt a function");
		Assert(0);
	}
	lua_pushvalue(l, f);
	luaref = luaL_ref(l, LUA_REGISTRYINDEX);
}

/**
**  FIXME: docu
**
**  @param eventId  FIXME: docu
*/
void LuaActionListener::action(const std::string &eventId) 
{
	int status;
	int base;
	base = lua_gettop(luastate);
	lua_getglobal(luastate, "_TRACEBACK");
	lua_rawgeti(luastate, LUA_REGISTRYINDEX, luaref);
	lua_pushstring(luastate, eventId.c_str());
	status = lua_pcall(luastate, 1, 0, base); //FIXME call error reporting function
	if (status) {
		const char *msg;
		msg = lua_tostring(luastate, -1);
		if (msg == NULL) {
			msg = "(error with no message)";
		}
		fprintf(stderr, "%s\n", msg);
		lua_pop(luastate, 1);
	}
}

/**
**  FIXME: docu
*/
LuaActionListener::~LuaActionListener()
{
	luaL_unref(luastate, LUA_REGISTRYINDEX, luaref);
}


/*----------------------------------------------------------------------------
--  ImageButton
----------------------------------------------------------------------------*/


/**
**  FIXME: docu
**
**  @param caption  FIXME: docu
**  @param a        FIXME: docu
**  @param b        FIXME: docu
*/
ImageButton::ImageButton(char *caption, gcn::Image *a, gcn::Image *b) : Button(caption)
{
	setForegroundColor(0xffffff);
	normalImage = a;
	pressedImage = b;
	adjustSize();
}

/**
**  FIXME: docu
**
**  @param graphics  FIXME: docu
*/
void ImageButton::draw(gcn::Graphics *graphics) 
{
	gcn::Color faceColor = getBaseColor();
	gcn::Color highlightColor, shadowColor;
	int alpha = getBaseColor().a;

	if (isPressed()) {
		faceColor = faceColor - 0x303030;
		faceColor.a = alpha;
		highlightColor = faceColor - 0x303030;
		highlightColor.a = alpha;
		shadowColor = faceColor + 0x303030;
		shadowColor.a = alpha;
		graphics->drawImage(pressedImage, 0, 0, 0, 0,
			pressedImage->getWidth(), pressedImage->getHeight());
	} else if (0 && hasMouse()) {
		highlightColor = faceColor + 0x303030;
		highlightColor.a = alpha;
		shadowColor = faceColor - 0x303030;
		shadowColor.a = alpha;
	} else {
		highlightColor = faceColor + 0x303030;
		highlightColor.a = alpha;
		shadowColor = faceColor - 0x303030;
		shadowColor.a = alpha;
		graphics->drawImage(normalImage, 0, 0, 0, 0,
			normalImage->getWidth(), normalImage->getHeight());
	}
	graphics->setColor(faceColor);
    
	graphics->setColor(highlightColor);
	graphics->drawLine(0, 0, getWidth() - 1, 0);
	graphics->drawLine(0, 1, 0, getHeight() - 1);
    
	graphics->setColor(shadowColor);
	graphics->drawLine(getWidth() - 1, 1, getWidth() - 1, getHeight() - 1);
	graphics->drawLine(1, getHeight() - 1, getWidth() - 1, getHeight() - 1);

	graphics->setColor(getForegroundColor());

	int textX;
	int textY = getHeight() / 2 - getFont()->getHeight() / 2;
        
	switch (getAlignment()) {
		case gcn::Graphics::LEFT:
			textX = 4;
			break;
		case gcn::Graphics::CENTER:
			textX = getWidth() / 2;
			break;
		case gcn::Graphics::RIGHT:
			textX = getWidth() - 4;
			break;
		default:
			throw GCN_EXCEPTION("Unknown alignment.");
	}

	graphics->setFont(getFont());       
	if (isPressed()) {
		graphics->drawText(getCaption(), textX + 4, textY + 4, getAlignment());
	} else {
		graphics->drawText(getCaption(), textX + 2, textY + 2, getAlignment());
	}
    
	if (hasFocus()) {
		graphics->drawRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
	}
}

/**
**  FIXME: docu
*/
void ImageButton::adjustSize()
{
	setWidth(normalImage->getWidth());
	setHeight(normalImage->getHeight());
}


/*----------------------------------------------------------------------------
--  Windows
----------------------------------------------------------------------------*/


/**
**  FIXME: docu
*/
Windows::Windows(const std::string &title, int width, int height) : Window(title), blockwholewindow(true)
{
	container.setDimension(gcn::Rectangle(0, 0, width, height));
	scroll.setDimension(gcn::Rectangle(0, 0, width, height));
	this->setContent(&scroll);
	scroll.setContent(&container);
	this->resizeToContent();
}

/**
**  FIXME: docu
*/
void Windows::add(gcn::Widget *widget, int x, int y)
{
	container.add(widget, x, y);
	if (x + widget->getWidth() > container.getWidth()) {
		container.setWidth(x + widget->getWidth());
	}
	if (y + widget->getHeight() > container.getHeight()) {
		container.setHeight(y + widget->getHeight());
	}
}

/**
**  Move the window when it is dragged.
**
**  @param x   X coordinate of the mouse relative to the window.
**  @param y   Y coordinate of the mouse relative to the widndow.
**
**  @note Once dragged, without release the mouse,
**    if you go virtualy outside the container then go back,
**    you have to wait the virtual cursor are in the container.
**    It is because x, y argument refer to a virtual cursor :(
**  @note An another thing is strange
**    when the container is a "scrollable" ScrollArea with the cursor.
**    The cursor can go outside the visual area.
*/
void Windows::mouseMotion(int x, int y)
{
	gcn::BasicContainer *bcontainer = getParent();
	int diffx;
	int diffy;
	int criticalx;
	int criticaly;
	int absx;
	int absy;

	if (!mMouseDrag || !isMovable()) {
		return;
	}

	diffx = x - mMouseXOffset;
	diffy = y - mMouseYOffset;
	if (blockwholewindow) {
		criticalx = getX();
		criticaly = getY();
	} else {
	criticalx = getX() + mMouseXOffset;
	criticaly = getY() + mMouseYOffset;
	}


	if (criticalx + diffx < 0) {
		diffx = -criticalx;
	}
	if (criticaly + diffy < 0) {
		diffy = -criticaly;
	}

	if (blockwholewindow) {
		criticalx = getX() + getWidth();
		criticaly = getY() + getHeight();
	}
	if (criticalx + diffx >= bcontainer->getWidth()) {
		diffx = bcontainer->getWidth() - criticalx;
	}
	if (criticaly + diffy >= bcontainer->getHeight()) {
		diffy = bcontainer->getHeight() - criticaly;
	}

	// Place the window.
	x = getX() + diffx;
	y = getY() + diffy;
	setPosition(x, y);

	// Move the cursor.
	// Usefull only when window reachs the limit.
	getAbsolutePosition(absx, absy);
	CursorX = absx + mMouseXOffset;
	CursorY = absy + mMouseYOffset;
}

/*----------------------------------------------------------------------------
--  LuaListModel
----------------------------------------------------------------------------*/


/**
**  FIXME: docu
*/
void LuaListModel::setList(lua_State *lua, lua_Object *lo)
{
	int args;
	int j;

	list.clear();

	args = luaL_getn(lua, *lo);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(lua, *lo, j + 1);
		list.push_back(std::string(LuaToString(lua, -1)));
		lua_pop(lua, 1);
	}
}


/*----------------------------------------------------------------------------
--  ListBoxWidget
----------------------------------------------------------------------------*/


/**
**  FIXME: docu
*/
void ListBoxWidget::setList(lua_State *lua, lua_Object *lo)
{
	listmodel.setList(lua, lo);
	setListModel(&listmodel);
}


/*----------------------------------------------------------------------------
--  DropDownWidget
----------------------------------------------------------------------------*/


/**
**  FIXME: docu
*/
void DropDownWidget::setList(lua_State *lua, lua_Object *lo) 
{
	listmodel.setList(lua, lo);
	setListModel(&listmodel);
}

/*----------------------------------------------------------------------------
--  MenuScreen
----------------------------------------------------------------------------*/
MenuScreen::MenuScreen() : 
Container(), 
runLoop(true)
{
	setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
	setOpaque(false);
	oldtop = gui->getTop();
	gui->setTop(this);
}

void MenuScreen::run() 
{
	SetVideoSync();
	while (runLoop) {
		UpdateDisplay();
		RealizeVideoMemory();
		WaitEventsOneFrame(&MenuCallbacks);
	}
	gui->setTop(oldtop);
}

void MenuScreen::stop()
{
	runLoop = false;
}

//@}
