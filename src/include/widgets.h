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
//      (c) Copyright 2005-2006 by Fran�ois Beerten and Jimmy Salmon
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

#ifndef __WIDGETS_H__
#define __WIDGETS_H__

#include <guichan.h>
#include <guichan/gsdl.h>
#include "font.h"

#include "luacallback.h"


extern bool GuichanActive;

void initGuichan();
void freeGuichan();
void handleInput(const SDL_Event *event);

class LuaActionListener : public gcn::ActionListener
{
	LuaCallback callback;
public:
	LuaActionListener(lua_State *lua, lua_Object function);
	virtual void action(const std::string &eventId);
	virtual ~LuaActionListener();
};

class MyOpenGLGraphics : public gcn::Graphics
{
public:
	virtual void _beginDraw();
	virtual void _endDraw();

	virtual void drawImage(const gcn::Image *image, int srcX, int srcY,
						   int dstX, int dstY, int width, int height);

	virtual void drawPoint(int x, int y);
	virtual void drawLine(int x1, int y1, int x2, int y2);
	virtual void drawRectangle(const gcn::Rectangle &rectangle);
	virtual void fillRectangle(const gcn::Rectangle &rectangle);

	virtual void setColor(const gcn::Color &color) { mColor = color; }
	virtual const gcn::Color &getColor() { return mColor; }

private:
	gcn::Color mColor;
};

class ImageWidget : public gcn::Icon
{
public:
	explicit ImageWidget(gcn::Image *img) : gcn::Icon(img) {}
};

class ButtonWidget : public gcn::Button
{
public:
	explicit ButtonWidget(const std::string &caption) : Button(caption) {
		this->setHotKey(GetHotKey(caption));
	}
};

class ImageButton : public gcn::Button
{
public:
	ImageButton();
	explicit ImageButton(const std::string &caption);

	virtual void draw(gcn::Graphics *graphics);
	virtual void adjustSize();

	void setNormalImage(gcn::Image *image) { normalImage = image; adjustSize(); }
	void setPressedImage(gcn::Image *image) { pressedImage = image; }
	void setDisabledImage(gcn::Image *image) { disabledImage = image; }

	gcn::Image *normalImage;
	gcn::Image *pressedImage;
	gcn::Image *disabledImage;
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
	explicit ImageSlider(double scaleEnd = 1.0);
	ImageSlider(double scaleStart, double scaleEnd);

	virtual void drawMarker(gcn::Graphics *graphics);
	virtual void draw(gcn::Graphics *graphics);

	void setMarkerImage(gcn::Image *image);
	void setBackgroundImage(gcn::Image *image);

	gcn::Image *markerImage;
	gcn::Image *backgroundImage;
};

class MultiLineLabel : public gcn::Widget
{
public:
	MultiLineLabel();
	explicit MultiLineLabel(const std::string &caption);

	virtual void setCaption(const std::string &caption);
	virtual const std::string &getCaption() const;
	virtual void setAlignment(unsigned int alignment);
	virtual unsigned int getAlignment();
	virtual void setVerticalAlignment(unsigned int alignment);
	virtual unsigned int getVerticalAlignment();
	virtual void setLineWidth(int width);
	virtual int getLineWidth();
	virtual void adjustSize();
	virtual void draw(gcn::Graphics *graphics);
	virtual void drawBorder(gcn::Graphics *graphics);

	enum {
		LEFT = 0,
		CENTER,
		RIGHT,
		TOP,
		BOTTOM
	};

private:
	void wordWrap();

	std::string mCaption;
	std::vector<std::string> mTextRows;
	unsigned int mAlignment;
	unsigned int mVerticalAlignment;
	int mLineWidth;
};

class ScrollingWidget : public gcn::ScrollArea
{
public:
	ScrollingWidget(int width, int height);
	void add(gcn::Widget *widget, int x, int y);
	void restart();
	void setSpeed(float speed) { this->speedY = speed; }
	float getSpeed() const { return this->speedY; }
private:
	virtual void logic();
private:
	gcn::Container container; /// Data container
	float speedY;             /// vertical speed of the container (positive number: go up).
	float containerY;         /// Y position of the container
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
	void adjustSize();
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
	virtual void setSize(int width, int height);
};

class StatBoxWidget : public gcn::Widget
{
public:
	StatBoxWidget(int width, int height);

	virtual void draw(gcn::Graphics *graphics);
	void setCaption(const std::string &s);
	const std::string &getCaption() const;
	void setPercent(const int percent);
	int getPercent() const;

private:
	int width;            /// width of the widget.
	int height;           /// height of the widget.
	std::string caption;  /// caption of the widget.
	unsigned int percent; /// percent value of the widget.
};

class MenuScreen : public gcn::Container
{
public:
	MenuScreen();

	int run(bool loop = true);
	void stop(int result = 0, bool stopAll = false);
	void stopAll(int result = 0) { stop(result, true); }
	void addLogicCallback(LuaActionListener *listener);
	virtual void draw(gcn::Graphics *graphics);
	virtual void logic();
	void setDrawMenusUnder(bool drawUnder) { this->drawUnder = drawUnder; }
	bool getDrawMenusUnder() const { return this->drawUnder; }

private:
	bool runLoop;
	int loopResult;
	gcn::Widget *oldtop;
	LuaActionListener *logiclistener;
	bool drawUnder;
};

#endif
