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

#ifndef __WIDGETS_H__
#define __WIDGETS_H__

#include <guichan.h>
#include <guichan/gsdl.h>
typedef int lua_Object; // from tolua++.h

extern bool guichanActive;

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

class ImageWidget : public gcn::Icon
{
public:
	ImageWidget(gcn::Image *img) : gcn::Icon(img) {}
};

class ButtonWidget : public gcn::Button 
{
public:
	ButtonWidget(const std::string &caption) : Button(caption) {}
}; 

class ImageButton : public gcn::Button
{
public:
	ImageButton();
	ImageButton(const std::string &caption);

	virtual void draw(gcn::Graphics *graphics);
	virtual void adjustSize();

	virtual void keyPress(const gcn::Key &key);
	virtual void keyRelease(const gcn::Key &key);

	void setNormalImage(gcn::Image *image) { normalImage = image; adjustSize(); }
	void setPressedImage(gcn::Image *image) { pressedImage = image; }
	void setDisabledImage(gcn::Image *image) { disabledImage = image; }

	void setHotKey(const int key);
	void setHotKey(const char *key);

	gcn::Image *normalImage;
	gcn::Image *pressedImage;
	gcn::Image *disabledImage;
	int hotKey;
};

class ImageRadioButton : public gcn::RadioButton
{
public:
	ImageRadioButton();
	ImageRadioButton(const std::string &caption, const std::string &group,
		bool marked);

	virtual void drawBox(gcn::Graphics *graphics);
	virtual void draw(gcn::Graphics *graphics);

	virtual void mousePress(int x, int y, int button);
	virtual void mouseRelease(int x, int y, int button);
	virtual void mouseClick(int x, int y, int button, int count);
	virtual void adjustSize();

	void setUncheckedNormalImage(gcn::Image *image) { uncheckedNormalImage = image; }
	void setUncheckedPressedImage(gcn::Image *image) { uncheckedPressedImage = image; }
	void setCheckedNormalImage(gcn::Image *image) { checkedNormalImage = image; }
	void setCheckedPressedImage(gcn::Image *image) { checkedPressedImage = image; }

	gcn::Image *uncheckedNormalImage;
	gcn::Image *uncheckedPressedImage;
	gcn::Image *checkedNormalImage;
	gcn::Image *checkedPressedImage;
	bool mMouseDown;
};

class ImageCheckBox : public gcn::CheckBox
{
public:
	ImageCheckBox();
	ImageCheckBox(const std::string &caption, bool marked = false);

	virtual void draw(gcn::Graphics *graphics);
	virtual void drawBox(gcn::Graphics *graphics);

	virtual void mousePress(int x, int y, int button);
	virtual void mouseRelease(int x, int y, int button);
	virtual void mouseClick(int x, int y, int button, int count);
	virtual void adjustSize();

	void setUncheckedNormalImage(gcn::Image *image) { uncheckedNormalImage = image; }
	void setUncheckedPressedImage(gcn::Image *image) { uncheckedPressedImage = image; }
	void setCheckedNormalImage(gcn::Image *image) { checkedNormalImage = image; }
	void setCheckedPressedImage(gcn::Image *image) { checkedPressedImage = image; }

	gcn::Image *uncheckedNormalImage;
	gcn::Image *uncheckedPressedImage;
	gcn::Image *checkedNormalImage;
	gcn::Image *checkedPressedImage;
	bool mMouseDown;
};

class ImageSlider : public gcn::Slider
{
public:
	ImageSlider(double scaleEnd = 1.0);
	ImageSlider(double scaleStart, double scaleEnd);

	virtual void drawMarker(gcn::Graphics* graphics);
	virtual void draw(gcn::Graphics* graphics);

	void setMarkerImage(gcn::Image *image);
	void setBackgroundImage(gcn::Image *image);

	gcn::Image *markerImage;
	gcn::Image *backgroundImage;
};


class ScrollingWidget : public gcn::ScrollArea
{
public:
	ScrollingWidget(int width, int height);
	void add(gcn::Widget *widget, int x, int y);
	void restart();
private:
	virtual void logic();
private:
	gcn::Container container; /// Data container
	int speedY;               /// vertical speed of the container (positive number: go up).
	bool finished;            /// True while scrolling ends.
};

class Windows : public gcn::Window
{
public:
	Windows(const std::string &text, int width, int height);
	void add(gcn::Widget *widget, int x, int y);
private:
	virtual void mouseMotion(int x, int y);
	virtual void setBackgroundColor(const gcn::Color &color);
	virtual void setBaseColor(const gcn::Color &color);
private:
	gcn::ScrollArea scroll;   /// To use scroll bar.
	gcn::Container container; /// data container.
	bool blockwholewindow;    /// Manage condition limit of moveable windows. @see mouseMotion.
                              /// @todo Method to set this variable. Maybe set the variable static.
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

class ListBoxWidget : public gcn::ScrollArea
{
public:
	ListBoxWidget(unsigned int width, unsigned int height);
	void setList(lua_State *lua, lua_Object *lo);
	void setSelected(int i);
	int getSelected() const;
	virtual void setBackgroundColor(const gcn::Color &color);
	virtual void setFont(gcn::Font *font);
	virtual void addActionListener(gcn::ActionListener *actionListener);
private:
	void ListBoxWidget::adjustSize();
private:
	LuaListModel lualistmodel;
	gcn::ListBox listbox;
};

class DropDownWidget : public gcn::DropDown
{
	LuaListModel listmodel;
public:
	DropDownWidget() {}
	void setList(lua_State *lua, lua_Object *lo);
};

class MenuScreen : public gcn::Container
{
	bool runLoop;
	int loopResult;
	gcn::Widget *oldtop;
public:
	MenuScreen();
	int run();
	void stop(int result = 0);
};

#endif

