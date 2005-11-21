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

/**
**  FIXME: docu
*/
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
**  LuaActionListener constructor
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
**  LuaActionListener destructor
*/
LuaActionListener::~LuaActionListener()
{
	luaL_unref(luastate, LUA_REGISTRYINDEX, luaref);
}


/*----------------------------------------------------------------------------
--  ImageButton
----------------------------------------------------------------------------*/


/**
**  ImageButton constructor
*/
ImageButton::ImageButton() :
	Button(), normalImage(NULL), pressedImage(NULL),
	disabledImage(NULL), hotKey(0)
{
	setForegroundColor(0xffffff);
}

/**
**  ImageButton constructor
**
**  @param caption  Caption text
*/
ImageButton::ImageButton(const std::string &caption) :
	Button(caption), normalImage(NULL), pressedImage(NULL),
	disabledImage(NULL), hotKey(0)
{
	setForegroundColor(0xffffff);
}

/**
**  Draw the image button
**
**  @param graphics  Graphics object to draw with
*/
void ImageButton::draw(gcn::Graphics *graphics) 
{
	if (!normalImage) {
		Button::draw(graphics);
		return;
	}

	gcn::Image *img;

	if (!isEnabled()) {
		img = disabledImage ? disabledImage : normalImage;
	} else if (isPressed()) {
		img = pressedImage ? pressedImage : normalImage;
	} else if (0 && hasMouse()) {
		// FIXME: add mouse-over image
		img = NULL;
	} else {
		img = normalImage;
	}
	graphics->drawImage(img, 0, 0, 0, 0,
		img->getWidth(), img->getHeight());
    
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
**  Automatically adjust the size of an image button
*/
void ImageButton::adjustSize()
{
	if (normalImage) {
		setWidth(normalImage->getWidth());
		setHeight(normalImage->getHeight());
	} else {
		Button::adjustSize();
	}
}

/**
**  Check if a key is a hotkey
*/
static bool isHotKey(const gcn::Key &key, int hotKey)
{
	if (isascii(key.getValue())) {
		return tolower(key.getValue()) == hotKey;
	} else {
		return key.getValue() == hotKey;
	}
}

/**
**  Key pressed callback
**
**  @param key  Key that is pressed
*/
void ImageButton::keyPress(const gcn::Key &key)
{
	Button::keyPress(key);
	if (isHotKey(key, hotKey)) {
		mKeyDown = true;
	}
}

/**
**  Key released callback
**
**  @param key  Key that is released
*/
void ImageButton::keyRelease(const gcn::Key &key)
{
	Button::keyRelease(key);
	if (mKeyDown && isHotKey(key, hotKey)) {
		mKeyDown = false;
		generateAction();
	}
}

/**
**  Set the hot key
**
**  @param key  The hot key
*/
void ImageButton::setHotKey(const int key)
{
	if (isascii(key)) {
		hotKey = tolower(key);
	} else {
		hotKey = key;
	}
}

/**
**  Set the hot key
**
**  @param key  The hot key
*/
void ImageButton::setHotKey(const char *key)
{
	Assert(key);
	Assert(strlen(key) == 1);
	Assert(isalnum(key[0]));
	hotKey = tolower(key[0]);
}


/*----------------------------------------------------------------------------
--  ImageCheckbox
----------------------------------------------------------------------------*/


/**
**  Image checkbox constructor
*/
ImageCheckBox::ImageCheckBox() : gcn::CheckBox(),
	uncheckedNormalImage(NULL), uncheckedPressedImage(NULL),
	checkedNormalImage(NULL), checkedPressedImage(NULL),
	mMouseDown(false)
{
}

/**
**  Image checkbox constructor
*/
ImageCheckBox::ImageCheckBox(const std::string &caption, bool marked) :
	gcn::CheckBox(caption, marked),
	uncheckedNormalImage(NULL), uncheckedPressedImage(NULL),
	checkedNormalImage(NULL), checkedPressedImage(NULL),
	mMouseDown(false)
{
}

/**
**  Draw the image checkbox
*/
void ImageCheckBox::draw(gcn::Graphics *graphics)
{
	drawBox(graphics);

	graphics->setFont(getFont());
	graphics->setColor(getForegroundColor());

	int width;
	if (uncheckedNormalImage) {
		width = uncheckedNormalImage->getWidth();
		width += width / 2;
	} else {
		width = getHeight();
		width += width / 2;
	}

	graphics->drawText(getCaption(), width - 2, 0);

	if (hasFocus()) {
		graphics->drawRectangle(gcn::Rectangle(width - 4, 0, getWidth() - width + 3, getHeight()));
	}
}

/**
**  Draw the checkbox (not the caption)
*/
void ImageCheckBox::drawBox(gcn::Graphics *graphics)
{
	gcn::Image *img = NULL;

	if (mMarked) {
		if (mMouseDown) {
			img = checkedPressedImage;
		} else {
			img = checkedNormalImage;
		}
	} else {
		if (mMouseDown) {
			img = uncheckedPressedImage;
		} else {
			img = uncheckedNormalImage;
		}
	}

	if (img) {
		graphics->drawImage(img, 0, 0, 0, (getHeight() - img->getHeight()) / 2,
			img->getWidth(), img->getHeight());
	} else {
		CheckBox::drawBox(graphics);
	}
}

/**
**  Mouse button pressed callback
*/
void ImageCheckBox::mousePress(int x, int y, int button)
{
	if (button == gcn::MouseInput::LEFT && hasMouse()) {
		mMouseDown = true;
	}
}

/**
**  Mouse button released callback
*/
void ImageCheckBox::mouseRelease(int x, int y, int button)
{
	if (button == gcn::MouseInput::LEFT) {
		mMouseDown = false;
	}
}

/**
**  Mouse clicked callback
*/
void ImageCheckBox::mouseClick(int x, int y, int button, int count)
{
	if (button == gcn::MouseInput::LEFT) {
		toggle();
	}
}

/**
**  Adjusts the CheckBox size to fit the font size
*/
void ImageCheckBox::adjustSize()
{
	int width, height;

	height = getFont()->getHeight();
	if (uncheckedNormalImage) {
		width = uncheckedNormalImage->getWidth();
		width += width / 2;
		if (uncheckedNormalImage->getHeight() > height) {
			height = uncheckedNormalImage->getHeight();
		}
	} else {
		width = getFont()->getHeight();
		width += width / 2;
	}

	setHeight(height);
	setWidth(getFont()->getWidth(mCaption) + width);
}


/*----------------------------------------------------------------------------
--  ImageSlider
----------------------------------------------------------------------------*/


/**
**  Image slider constructor
*/
ImageSlider::ImageSlider(double scaleEnd) :
	Slider(scaleEnd), markerImage(NULL), backgroundImage(NULL)
{
}

/**
**  Image slider constructor
*/
ImageSlider::ImageSlider(double scaleStart, double scaleEnd) :
	Slider(scaleStart, scaleEnd), markerImage(NULL), backgroundImage(NULL)
{
}

/**
**  Draw the image slider marker
*/
void ImageSlider::drawMarker(gcn::Graphics *graphics)
{
	if (markerImage) {
		if (getOrientation() == HORIZONTAL) {
			int v = getMarkerPosition();
			graphics->drawImage(markerImage, 0, 0, v, 0,
				markerImage->getWidth(), markerImage->getHeight());
		} else {
			int v = (getHeight() - getMarkerLength()) - getMarkerPosition();
			graphics->drawImage(markerImage, 0, 0, 0, v,
				markerImage->getWidth(), markerImage->getHeight());
		}
	} else {
		Slider::drawMarker(graphics);
	}
}

/**
**  Draw the image slider
*/
void ImageSlider::draw(gcn::Graphics *graphics)
{
	if (backgroundImage) {
		graphics->drawImage(backgroundImage, 0, 0, 0, 0,
			backgroundImage->getWidth(), backgroundImage->getHeight());
		drawMarker(graphics);
	} else {
		Slider::draw(graphics);
	}
}

/**
**  Set the marker image
*/
void ImageSlider::setMarkerImage(gcn::Image *image)
{
	markerImage = image;
	setMarkerLength(image->getWidth());
}

/**
**  Set the background image
*/
void ImageSlider::setBackgroundImage(gcn::Image *image)
{
	backgroundImage = image;
}


/*----------------------------------------------------------------------------
--  ScrollingWidget
----------------------------------------------------------------------------*/


/**
**  ScrollingWidget constructor.
**
**  @param width   Width of the widget.
**  @param height  Height of the widget.
*/
ScrollingWidget::ScrollingWidget(int width, int height) :
	gcn::ScrollArea(NULL, gcn::ScrollArea::SHOW_NEVER, gcn::ScrollArea::SHOW_NEVER),
	speedY(1), finished(false)
{
	container.setDimension(gcn::Rectangle(0, 0, width, height));
	container.setOpaque(false);
	setContent(&container);
	setDimension(gcn::Rectangle(0, 0, width, height));
}

/**
**  Add a widget in the window.
**
**  @param widget  Widget to add.
**  @param x       Position of the widget in the window.
**  @param y       Position of the widget in the window.
*/
void ScrollingWidget::add(gcn::Widget *widget, int x, int y)
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
**  Scrolling the content when possible.
*/
void ScrollingWidget::logic()
{
	if (speedY - container.getY() < container.getHeight() - getHeight()) {
		container.setY(container.getY() - speedY);
	} else if (!finished){
		finished = true;
		generateAction();
	}
}

/**
**  Restart animation to the beginning.
*/
void ScrollingWidget::restart()
{
	container.setY(0);
	finished = (container.getHeight() == getHeight());
}


/*----------------------------------------------------------------------------
--  Windows
----------------------------------------------------------------------------*/


/**
**  Windows constructor.
**
**  @param title   Title of the window.
**  @param width   Width of the window.
**  @param height  Height of the window.
*/
Windows::Windows(const std::string &title, int width, int height) :
	Window(title), blockwholewindow(true)
{
	container.setDimension(gcn::Rectangle(0, 0, width, height));
	scroll.setDimension(gcn::Rectangle(0, 0, width, height));
	this->setContent(&scroll);
	scroll.setContent(&container);
	this->resizeToContent();
}

/**
**  Add a widget in the window.
**
**  @param widget  Widget to add.
**  @param x       Position of the widget in the window.
**  @param y       Position of the widget in the window.
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

/**
**  Set background color of the window.
**
**  @param color  Color to set.
*/
void Windows::setBackgroundColor(const gcn::Color &color)
{
	Window::setBackgroundColor(color);
	scroll.setBackgroundColor(color);
}

/**
**  Set base color of the windows.
**
**  @param color  Color to set.
*/
void Windows::setBaseColor(const gcn::Color &color)
{
	Window::setBaseColor(color);
	container.setBaseColor(color);
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
**  ListBoxWidget constructor.
**
**  @todo  Size should be parametrable, maybe remove default constructor?
*/
ListBoxWidget::ListBoxWidget(unsigned int width, unsigned int height)
{
	setDimension(gcn::Rectangle(0, 0, width, height));
	setContent(&listbox);
	setBackgroundColor(gcn::Color(128, 128, 128));
}


/**
**  FIXME: docu
*/
void ListBoxWidget::setList(lua_State *lua, lua_Object *lo)
{
	lualistmodel.setList(lua, lo);
	listbox.setListModel(&lualistmodel);
	adjustSize();
}

/**
**  Sets the ListModel index of the selected element.
**
**  @param selected  The ListModel index of the selected element.
**
**  @see gcn::ListBox
*/
void ListBoxWidget::setSelected(int selected)
{
	listbox.setSelected(selected);
}

/**
**  Gets the ListModel index of the selected element.
**
**  @return  The ListModel index of the selected element.
**
**  @see gcn::ListBox
*/
int ListBoxWidget::getSelected() const
{
	return const_cast<gcn::ListBox &> (listbox).getSelected();
}

/**
**  Set background color of the ListBoxWidget.
**
**  @param color  Color to set.
*/
void ListBoxWidget::setBackgroundColor(const gcn::Color &color)
{
	ScrollArea::setBackgroundColor(color);
	listbox.setBackgroundColor(color);
}

/**
**  Set font of the ListBox.
**
**  @param font  Font to set.
*/
void ListBoxWidget::setFont(gcn::Font *font)
{
	listbox.setFont(font);
	listbox.setWidth(getWidth());
	adjustSize();
}

/**
**  Adjust size of the listBox.
**
**  @todo Fix width of the scroll area (depend of v-scroll or not).
*/
void ListBoxWidget::adjustSize()
{
	int i;
	int width;
	gcn::ListModel *listmodel;

	width = listbox.getWidth();
	Assert(listbox.getListModel());
	listmodel = listbox.getListModel();
	for (i = 0; i < listmodel->getNumberOfElements(); ++i) {
		if (width < listbox.getFont()->getWidth(listmodel->getElementAt(i))) {
			width = listbox.getFont()->getWidth(listmodel->getElementAt(i));
		}
	}
	if (width != listbox.getWidth()) {
		listbox.setWidth(width);
	}
}

/**
**  Add an action listener
*/
void ListBoxWidget::addActionListener(gcn::ActionListener *actionListener)
{
	listbox.addActionListener(actionListener);
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


/**
**  MenuScreen constructor
*/
MenuScreen::MenuScreen() : 
	Container(), runLoop(true)
{
	setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
	setOpaque(false);
	oldtop = gui->getTop();
	gui->setTop(this);
}

/**
**  Run the menu.  Loops until stop is called.
*/
int MenuScreen::run() 
{
	loopResult = 0;
	SetVideoSync();
	while (runLoop) {
		UpdateDisplay();
		RealizeVideoMemory();
		WaitEventsOneFrame(&MenuCallbacks);
	}
	gui->setTop(oldtop);
	return loopResult;
}

/**
**  Stop the menu from running
*/
void MenuScreen::stop(int result)
{
	runLoop = false;
	loopResult = result;
}

//@}
