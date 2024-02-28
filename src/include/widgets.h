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

#include "font.h"
#include "luacallback.h"

#include <functional>
#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/mouselistener.hpp>

extern bool GuichanActive;

void initGuichan();
void freeGuichan();
void handleInput(const SDL_Event *event);

#if 1 // ToLua++
void addActionListener(gcn::Widget *, gcn::ActionListener *);
void setBackgroundColor(gcn::Widget *, const gcn::Color &);
void setBaseColor(gcn::Widget *, const gcn::Color &color);
void setDirty(gcn::Widget *, bool isDirty);
void setDisabledColor(gcn::Widget *, const gcn::Color &);
void setFont(gcn::Widget *widget, gcn::Font *font);
void setHotKey(gcn::Widget *, const char *key);
void scrollToBottom(gcn::ScrollArea *);
void scrollToTop(gcn::ScrollArea *);
#endif

class LuaActionListener : public gcn::ActionListener, public gcn::KeyListener, public gcn::MouseListener
{
	LuaCallback callback;
public:
	LuaActionListener(lua_State *lua, lua_Object function);
	~LuaActionListener() override;
	void action(const gcn::ActionEvent &actionEvent) override;
	void keyPressed(gcn::KeyEvent&) override;
	void keyReleased(gcn::KeyEvent&) override;
	void hotKeyPressed(const gcn::Key&) override;
	void hotKeyReleased(const gcn::Key&) override;
	void mouseEntered(gcn::MouseEvent&) override;
	void mouseExited(gcn::MouseEvent&) override;
	void mousePressed(gcn::MouseEvent&) override;
	void mouseReleased(gcn::MouseEvent&) override;
	void mouseClicked(gcn::MouseEvent&) override;
	void mouseWheelMovedUp(gcn::MouseEvent&) override;
	void mouseWheelMovedDown(gcn::MouseEvent&) override;
	void mouseMoved(gcn::MouseEvent&) override;
};

class LambdaActionListener : public gcn::ActionListener
{
	std::function<void(const std::string &)> lambda;
public:
	explicit LambdaActionListener(std::function<void(const std::string &)> l) : lambda(l) {}

	void action(const gcn::ActionEvent &actionEvent) override { lambda(actionEvent.getId()); }
};

class ImageWidget : public gcn::Icon
{
public:
	explicit ImageWidget(gcn::Image *img) : gcn::Icon(img) {}
};

class ButtonWidget : public gcn::Button
{
public:
	explicit ButtonWidget(const std::string &caption) : Button(caption)
	{
		::setHotKey(this, caption.c_str());
	}
};

class CImageButton : public gcn::Button
{
public:
	CImageButton();
	explicit CImageButton(const std::string &caption);

	void draw(gcn::Graphics *graphics) override;
	void adjustSize();

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

	void drawBox(gcn::Graphics *graphics) override;
	void draw(gcn::Graphics *graphics) override;

	void mousePressed(gcn::MouseEvent&) override;
	void mouseReleased(gcn::MouseEvent&) override;
	void mouseClicked(gcn::MouseEvent&) override;
	void adjustSize();

	void setUncheckedNormalImage(gcn::Image *image) { uncheckedNormalImage = image; }
	void setUncheckedPressedImage(gcn::Image *image) { uncheckedPressedImage = image; }
	void setUncheckedDisabledImage(gcn::Image *image) { uncheckedDisabledImage = image; }
	void setCheckedNormalImage(gcn::Image *image) { checkedNormalImage = image; }
	void setCheckedPressedImage(gcn::Image *image) { checkedPressedImage = image; }
	void setCheckedDisabledImage(gcn::Image *image) { checkedDisabledImage = image; }

	gcn::Image *uncheckedNormalImage;
	gcn::Image *uncheckedPressedImage;
	gcn::Image *uncheckedDisabledImage;
	gcn::Image *checkedNormalImage;
	gcn::Image *checkedPressedImage;
	gcn::Image *checkedDisabledImage;
	bool mMouseDown;
};

class ImageCheckBox : public gcn::CheckBox
{
public:
	ImageCheckBox();
	ImageCheckBox(const std::string &caption, bool marked = false);

	void draw(gcn::Graphics *graphics) override;
	void drawBox(gcn::Graphics *graphics) override;

	void mousePressed(gcn::MouseEvent&) override;
	void mouseReleased(gcn::MouseEvent &) override;
	void mouseClicked(gcn::MouseEvent &) override;
	void adjustSize();

	void setUncheckedNormalImage(gcn::Image *image) { uncheckedNormalImage = image; }
	void setUncheckedPressedImage(gcn::Image *image) { uncheckedPressedImage = image; }
	void setUncheckedDisabledImage(gcn::Image *image) { uncheckedDisabledImage = image; }
	void setCheckedNormalImage(gcn::Image *image) { checkedNormalImage = image; }
	void setCheckedPressedImage(gcn::Image *image) { checkedPressedImage = image; }
	void setCheckedDisabledImage(gcn::Image *image) { checkedDisabledImage = image; }

	gcn::Image *uncheckedNormalImage;
	gcn::Image *uncheckedPressedImage;
	gcn::Image *uncheckedDisabledImage;
	gcn::Image *checkedNormalImage;
	gcn::Image *checkedPressedImage;
	gcn::Image *checkedDisabledImage;
	bool mMouseDown;
};

class ImageSlider : public gcn::Slider
{
public:
	explicit ImageSlider(double scaleEnd = 1.0);
	ImageSlider(double scaleStart, double scaleEnd);

	void drawMarker(gcn::Graphics *graphics) override;
	void draw(gcn::Graphics *graphics) override;

	void setMarkerImage(gcn::Image *image);
	void setBackgroundImage(gcn::Image *image);
	void setDisabledBackgroundImage(gcn::Image *image);

	gcn::Image *markerImage;
	gcn::Image *backgroundImage;
	gcn::Image *disabledBackgroundImage;
};

class MultiLineLabel : public gcn::Widget
{
public:
	MultiLineLabel();
	explicit MultiLineLabel(const std::string &caption);

	void setCaption(const std::string &caption);
	const std::string &getCaption() const;
	void setAlignment(gcn::Graphics::Alignment alignment);
	void setAlignment(unsigned int alignment) { setAlignment(static_cast<gcn::Graphics::Alignment>(alignment)); } // TOLUA
	gcn::Graphics::Alignment getAlignment();
	void setVerticalAlignment(unsigned int alignment);
	unsigned int getVerticalAlignment();
	void setLineWidth(int width);
	int getLineWidth();
	void adjustSize();
	void draw(gcn::Graphics *graphics) override;
	void drawFrame(gcn::Graphics *graphics) override;

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
	gcn::Graphics::Alignment mAlignment;
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
	void logic() override;
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

	void setBackgroundColor(const gcn::Color &color);
	void setBaseColor(const gcn::Color &color);
private:
	void mouseDragged(gcn::MouseEvent &) override;

private:
	gcn::ScrollArea scroll;   /// To use scroll bar.
	gcn::Container container; /// data container.
	bool blockwholewindow;    /// Manage condition limit of moveable windows. @see mouseMotion.
	/// @todo Method to set this variable. Maybe set the variable static.
};

// Similar to gcn::TextBox but
// - handles clipboard
// - handles UTF-8
// - handles more shortcuts
class CTextBox : public gcn::TextBox
{
public:
	using gcn::TextBox::TextBox;
	void mousePressed(gcn::MouseEvent &) override;
	void keyPressed(gcn::KeyEvent &) override;
};

// Similar to gcn::TextField but
// - handles selection
// - have password mode
// - handles clipboard
// - handles UTF-8
// - handles more shortcuts
class CTextField : public gcn::TextField
{
public:
	CTextField() = default;
	explicit CTextField(const std::string &text) : TextField(text) {}

	void draw(gcn::Graphics *graphics) override;
	void mousePressed(gcn::MouseEvent &) override;
	void mouseDragged(gcn::MouseEvent &) override;
	void keyPressed(gcn::KeyEvent &) override;

	void setPassword(bool flag) { isPassword = flag; }
	void getTextSelectionPositions(unsigned int *first, unsigned int *len) const;
protected:
	int mSelectStart = 0;
	int mSelectEndOffset = 0;
	bool isPassword = false;
};

class ImageTextField : public CTextField
{
public:
	ImageTextField() : CTextField(), itemImage(nullptr) {}
	ImageTextField(const std::string &text) : CTextField(text), itemImage(nullptr) {}
	void draw(gcn::Graphics *graphics) override;
	void drawFrame(gcn::Graphics *graphics) override;
	void setItemImage(CGraphic *image) { itemImage = image; }
private:
	CGraphic *itemImage;
};

class StringListModel : public gcn::ListModel
{
	std::vector<std::string> list;
public:
	explicit StringListModel(std::vector<std::string> l) : list(l) {}

	int getNumberOfElements() override { return list.size(); }
	std::string getElementAt(int i) override { return list[i]; }
	int getIdxOfElement(std::string_view element);
};

class LuaListModel : public gcn::ListModel
{
	std::vector<std::string> list;
public:
	LuaListModel() {}

	void setList(lua_State *lua, lua_Object *lo);
	int getNumberOfElements() override { return list.size(); }
	std::string getElementAt(int i) override { return list[i]; }
	int getIdxOfElement(std::string_view element);
};

class ImageListBox : public gcn::ListBox
{
public:
	ImageListBox();
	explicit ImageListBox(gcn::ListModel *listModel);
	void draw(gcn::Graphics *graphics) override;
	void drawFrame(gcn::Graphics *graphics) override;
	void mousePressed(gcn::MouseEvent &) override;
	void setItemImage(CGraphic *image) { itemImage = image; }
	void adjustSize();
	void setSelected(int selected);
	void logic() { adjustSize(); }

private:
	CGraphic *itemImage = nullptr;
};

class ListBoxWidget : public gcn::ScrollArea
{
public:
	ListBoxWidget(unsigned int width, unsigned int height);
	void setList(lua_State *lua, lua_Object *lo);
	void setSelected(int i);
	int getSelected() const;
	void setBackgroundColor(const gcn::Color &color);
	void fontChanged() override;
	void addActionListener(gcn::ActionListener *actionListener);

private:
	void adjustSize();
private:
	LuaListModel lualistmodel;
	gcn::ListBox listbox;
};

class ImageListBoxWidget : public ListBoxWidget
{
public:
	ImageListBoxWidget(unsigned int width, unsigned int height);
	void setList(lua_State *lua, lua_Object *lo);
	void setSelected(int i);
	int getSelected() const;
	void setBackgroundColor(const gcn::Color &color);
	void fontChanged() override;
	void addActionListener(gcn::ActionListener *actionListener);

	void setItemImage(CGraphic *image) {
		itemImage = image;
		listbox.setItemImage(image);
	}
	void setUpButtonImage(CGraphic *image) { upButtonImage = image; }
	void setUpPressedButtonImage(CGraphic *image) { upPressedButtonImage = image; }
	void setDownButtonImage(CGraphic *image) { downButtonImage = image; }
	void setDownPressedButtonImage(CGraphic *image) { downPressedButtonImage = image; }
	void setLeftButtonImage(CGraphic *image) { leftButtonImage = image; }
	void setLeftPressedButtonImage(CGraphic *image) { leftPressedButtonImage = image; }
	void setRightButtonImage(CGraphic *image) { rightButtonImage = image; }
	void setRightPressedButtonImage(CGraphic *image) { rightPressedButtonImage = image; }
	void setHBarImage(CGraphic *image) {
		hBarButtonImage = image;
		mScrollbarWidth = std::min<int>(image->getWidth(), image->getHeight());
	}
	void setVBarImage(CGraphic *image) {
		vBarButtonImage = image;
		mScrollbarWidth = std::min<int>(image->getWidth(), image->getHeight());
	}
	void setMarkerImage(CGraphic *image) { markerImage = image; }

	void draw(gcn::Graphics *graphics) override;
	void drawFrame(gcn::Graphics *graphics) override;
	gcn::Rectangle getVerticalMarkerDimension();
	gcn::Rectangle getHorizontalMarkerDimension();

private:
	void adjustSize();

	void drawUpButton(gcn::Graphics *graphics);
	void drawDownButton(gcn::Graphics *graphics);
	void drawLeftButton(gcn::Graphics *graphics);
	void drawRightButton(gcn::Graphics *graphics);
	void drawUpPressedButton(gcn::Graphics *graphics);
	void drawDownPressedButton(gcn::Graphics *graphics);
	void drawLeftPressedButton(gcn::Graphics *graphics);
	void drawRightPressedButton(gcn::Graphics *graphics);
	void drawHMarker(gcn::Graphics *graphics);
	void drawVMarker(gcn::Graphics *graphics);
	void drawHBar(gcn::Graphics *graphics);
	void drawVBar(gcn::Graphics *graphics);
private:
	CGraphic *itemImage = nullptr;
	CGraphic *upButtonImage = nullptr;
	CGraphic *upPressedButtonImage = nullptr;
	CGraphic *downButtonImage = nullptr;
	CGraphic *downPressedButtonImage = nullptr;
	CGraphic *leftButtonImage = nullptr;
	CGraphic *leftPressedButtonImage = nullptr;
	CGraphic *rightButtonImage = nullptr;
	CGraphic *rightPressedButtonImage = nullptr;
	CGraphic *hBarButtonImage = nullptr;
	CGraphic *vBarButtonImage = nullptr;
	CGraphic *markerImage = nullptr;

	LuaListModel lualistmodel;
	ImageListBox listbox;
};

class DropDownWidget : public gcn::DropDown
{
	DropDownWidget(std::unique_ptr<LuaListModel> listmodel, gcn::ListBox *listBox) :
		gcn::DropDown(listmodel.get(), nullptr, listBox),
		mListModel(std::move(listmodel))
	{}

public:
	explicit DropDownWidget(gcn::ListBox *listBox = nullptr) :
		DropDownWidget(std::make_unique<LuaListModel>(), listBox)
	{}

	void setList(lua_State *lua, lua_Object *lo);
	void setSize(int width, int height);
protected:
	std::unique_ptr<LuaListModel> mListModel;
};

class ImageDropDownWidget : public DropDownWidget
{
private:
	ImageDropDownWidget(std::unique_ptr<ImageListBox> imageListBox) :
		DropDownWidget(imageListBox.get()),
		mImageListBox(std::move(imageListBox))
	{
		this->mListBox->addActionListener(this);
	}

public:
	ImageDropDownWidget() : ImageDropDownWidget(std::make_unique<ImageListBox>()) {}
	void setItemImage(CGraphic *image) {
		this->itemImage = image;
		this->mImageListBox->setItemImage(image);
	}
	void setDownNormalImage(CGraphic *image) { DownNormalImage = image; }
	void setDownPressedImage(CGraphic *image) { DownPressedImage = image; }

	void draw(gcn::Graphics *graphics) override;
	void drawFrame(gcn::Graphics *graphics) override;
	void drawButton(gcn::Graphics *graphics) override;
	void setList(lua_State *lua, lua_Object *lo);
	void setSize(int width, int height);
	std::string getSelectedItem();
	int setSelectedItem(lua_State *lua, lua_Object *lo);
	void adjustHeight();
	void setFont(gcn::Font *font);

private:
	CGraphic *itemImage = nullptr;
	CGraphic *DownNormalImage = nullptr;
	CGraphic *DownPressedImage = nullptr;
	std::unique_ptr<ImageListBox> mImageListBox;
};

class StatBoxWidget : public gcn::Widget
{
public:
	StatBoxWidget(int width, int height);

	void draw(gcn::Graphics *graphics) override;
	void setCaption(const std::string &s);
	const std::string &getCaption() const;
	void setPercent(const int percent);
	int getPercent() const;

private:
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
	void draw(gcn::Graphics *graphics) override;
	void logic() override;
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
