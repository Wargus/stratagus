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
#include <guisan.hpp>
#include <guisan/sdl.hpp>
#include <guisan/keylistener.hpp>
#include <guisan/mouselistener.hpp>

extern bool GuichanActive;

void initGuichan();
void freeGuichan();
void handleInput(const SDL_Event *event);

#if USING_TOLUAPP
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
	LuaCallbackImpl callback;
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
	explicit ImageWidget(gcn::Image *img) : gcn::Icon(img) {} // TODO: Remove (used for Movie in ToLua++)
	explicit ImageWidget(std::shared_ptr<gcn::Image> img) : gcn::Icon(img.get()), img(img) {}

private:
	std::shared_ptr<gcn::Image> img;
};

class ButtonWidget : public gcn::Button
{
public:
	explicit ButtonWidget(const std::string &caption) : Button(caption)
	{
		gcn::Key key = GetHotKey(caption);
		setHotKey(key.getValue());
	}
};

class CImageButton : public gcn::Button
{
public:
	CImageButton();
	explicit CImageButton(const std::string &caption);

	void draw(gcn::Graphics *graphics) override;

	void adjustSize();

	void setNormalImage(std::shared_ptr<gcn::Image> image) { normalImage = image; adjustSize(); }
	void setPressedImage(std::shared_ptr<gcn::Image> image) { pressedImage = image; }
	void setDisabledImage(std::shared_ptr<gcn::Image> image) { disabledImage = image; }

	std::shared_ptr<gcn::Image> normalImage;
	std::shared_ptr<gcn::Image> pressedImage;
	std::shared_ptr<gcn::Image> disabledImage;
};

class ImageRadioButton : public gcn::RadioButton
{
public:
	ImageRadioButton() = default;
	ImageRadioButton(const std::string &caption, const std::string &group,
					 bool marked);

	void drawBox(gcn::Graphics *graphics) override;
	void draw(gcn::Graphics *graphics) override;
	void mousePressed(gcn::MouseEvent&) override;
	void mouseReleased(gcn::MouseEvent&) override;
	void mouseClicked(gcn::MouseEvent&) override;

	void adjustSize();
	void setUncheckedNormalImage(std::shared_ptr<gcn::Image> image) { uncheckedNormalImage = image; }
	void setUncheckedPressedImage(std::shared_ptr<gcn::Image> image) { uncheckedPressedImage = image; }
	void setUncheckedDisabledImage(std::shared_ptr<gcn::Image> image) { uncheckedDisabledImage = image; }
	void setCheckedNormalImage(std::shared_ptr<gcn::Image> image) { checkedNormalImage = image; }
	void setCheckedPressedImage(std::shared_ptr<gcn::Image> image) { checkedPressedImage = image; }
	void setCheckedDisabledImage(std::shared_ptr<gcn::Image> image) { checkedDisabledImage = image; }

	std::shared_ptr<gcn::Image> uncheckedNormalImage;
	std::shared_ptr<gcn::Image> uncheckedPressedImage;
	std::shared_ptr<gcn::Image> uncheckedDisabledImage;
	std::shared_ptr<gcn::Image> checkedNormalImage;
	std::shared_ptr<gcn::Image> checkedPressedImage;
	std::shared_ptr<gcn::Image> checkedDisabledImage;
	bool mMouseDown = false;
};

class ImageCheckBox : public gcn::CheckBox
{
public:
	ImageCheckBox() = default;
	ImageCheckBox(const std::string &caption, bool marked = false);

	void draw(gcn::Graphics *graphics) override;
	void drawBox(gcn::Graphics *graphics) override;
	void mousePressed(gcn::MouseEvent&) override;
	void mouseReleased(gcn::MouseEvent &) override;
	void mouseClicked(gcn::MouseEvent &) override;

	void adjustSize();
	void setUncheckedNormalImage(std::shared_ptr<gcn::Image> image) { uncheckedNormalImage = image; }
	void setUncheckedPressedImage(std::shared_ptr<gcn::Image> image) { uncheckedPressedImage = image; }
	void setUncheckedDisabledImage(std::shared_ptr<gcn::Image> image) { uncheckedDisabledImage = image; }
	void setCheckedNormalImage(std::shared_ptr<gcn::Image> image) { checkedNormalImage = image; }
	void setCheckedPressedImage(std::shared_ptr<gcn::Image> image) { checkedPressedImage = image; }
	void setCheckedDisabledImage(std::shared_ptr<gcn::Image> image) { checkedDisabledImage = image; }

	std::shared_ptr<gcn::Image> uncheckedNormalImage;
	std::shared_ptr<gcn::Image> uncheckedPressedImage;
	std::shared_ptr<gcn::Image> uncheckedDisabledImage;
	std::shared_ptr<gcn::Image> checkedNormalImage;
	std::shared_ptr<gcn::Image> checkedPressedImage;
	std::shared_ptr<gcn::Image> checkedDisabledImage;
	bool mMouseDown = false;
};

class ImageSlider : public gcn::Slider
{
public:
	explicit ImageSlider(double scaleEnd = 1.0);
	ImageSlider(double scaleStart, double scaleEnd);

	void drawMarker(gcn::Graphics *graphics) override;
	void draw(gcn::Graphics *graphics) override;

	void setMarkerImage(std::shared_ptr<gcn::Image> image);
	void setBackgroundImage(std::shared_ptr<gcn::Image> image);
	void setDisabledBackgroundImage(std::shared_ptr<gcn::Image> image);

	std::shared_ptr<gcn::Image> markerImage;
	std::shared_ptr<gcn::Image> backgroundImage;
	std::shared_ptr<gcn::Image> disabledBackgroundImage;
};

class MultiLineLabel : public gcn::Widget
{
public:
	MultiLineLabel() = default;
	explicit MultiLineLabel(const std::string &caption);

	void draw(gcn::Graphics *graphics) override;
	void drawFrame(gcn::Graphics *graphics) override;

	void setCaption(const std::string &caption);
	const std::string &getCaption() const;
	void setAlignment(gcn::Graphics::Alignment alignment);
	void setAlignment(unsigned int alignment) { setAlignment(static_cast<gcn::Graphics::Alignment>(alignment)); } // TOLUA
	gcn::Graphics::Alignment getAlignment() const;
	void setVerticalAlignment(unsigned int alignment);
	unsigned int getVerticalAlignment() const;
	void setLineWidth(int width);
	int getLineWidth() const;
	void adjustSize();

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
	gcn::Graphics::Alignment mAlignment = gcn::Graphics::Alignment::Left;
	unsigned int mVerticalAlignment = TOP;
	int mLineWidth = 0;
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
	float speedY; /// vertical speed of the container (positive number: go up).
	float containerY = 0; /// Y position of the container
	bool finished = false; /// True while scrolling ends.
};

class Windows : public gcn::Window
{
public:
	Windows(const std::string &text, int width, int height);
	void add(gcn::Widget *widget, int x, int y) override;

	void setBackgroundColor(const gcn::Color &color);
	void setBaseColor(const gcn::Color &color);

private:
	void mouseDragged(gcn::MouseEvent &) override;

private:
	gcn::ScrollArea scroll; /// To use scroll bar.
	gcn::Container container; /// data container.
	bool blockwholewindow; /// Manage condition limit of moveable windows. @see mouseMotion.
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
	ImageTextField() = default;
	explicit ImageTextField(const std::string &text) : CTextField(text) {}

	void draw(gcn::Graphics *graphics) override;
	void drawFrame(gcn::Graphics *graphics) override;

	void setItemImage(std::shared_ptr<CGraphic> image) { itemImage = image; }

private:
	std::shared_ptr<CGraphic> itemImage;
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
	LuaListModel() = default;

	int getNumberOfElements() override { return list.size(); }
	std::string getElementAt(int i) override { return list[i]; }

	void setList(lua_State *lua, lua_Object *lo);
	int getIdxOfElement(std::string_view element);
};

class ImageListBox : public gcn::ListBox
{
public:
	ImageListBox() = default;
	explicit ImageListBox(gcn::ListModel *listModel);

	void draw(gcn::Graphics *graphics) override;
	void drawFrame(gcn::Graphics *graphics) override;
	void mousePressed(gcn::MouseEvent &) override;
	void logic() override { adjustSize(); }

	void setItemImage(std::shared_ptr<CGraphic> image) { itemImage = image; }
	void adjustSize();
	void setSelected(int selected);

private:
	std::shared_ptr<CGraphic> itemImage = nullptr;
};

class ListBoxWidget : public gcn::ScrollArea
{
public:
	ListBoxWidget(unsigned int width, unsigned int height);

	void fontChanged() override;

	void setList(lua_State *lua, lua_Object *lo);
	void setSelected(int i);
	int getSelected() const;
	void setBackgroundColor(const gcn::Color &color);
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
	void addActionListener(gcn::ActionListener *actionListener);
	void setItemImage(std::shared_ptr<CGraphic> image)
	{
		itemImage = image;
		listbox.setItemImage(image);
	}
	void setUpButtonImage(std::shared_ptr<CGraphic> image) { upButtonImage = image; }
	void setUpPressedButtonImage(std::shared_ptr<CGraphic> image) { upPressedButtonImage = image; }
	void setDownButtonImage(std::shared_ptr<CGraphic> image) { downButtonImage = image; }
	void setDownPressedButtonImage(std::shared_ptr<CGraphic> image)
	{
		downPressedButtonImage = image;
	}
	void setLeftButtonImage(std::shared_ptr<CGraphic> image) { leftButtonImage = image; }
	void setLeftPressedButtonImage(std::shared_ptr<CGraphic> image)
	{
		leftPressedButtonImage = image;
	}
	void setRightButtonImage(std::shared_ptr<CGraphic> image) { rightButtonImage = image; }
	void setRightPressedButtonImage(std::shared_ptr<CGraphic> image)
	{
		rightPressedButtonImage = image;
	}
	void setHBarImage(std::shared_ptr<CGraphic> image)
	{
		hBarButtonImage = image;
		mScrollbarWidth = std::min<int>(image->getWidth(), image->getHeight());
	}
	void setVBarImage(std::shared_ptr<CGraphic> image)
	{
		vBarButtonImage = image;
		mScrollbarWidth = std::min<int>(image->getWidth(), image->getHeight());
	}
	void setMarkerImage(std::shared_ptr<CGraphic> image) { markerImage = image; }
	gcn::Rectangle getVerticalMarkerDimension();
	gcn::Rectangle getHorizontalMarkerDimension();

	void draw(gcn::Graphics *graphics) override;
	void drawFrame(gcn::Graphics *graphics) override;
	void fontChanged() override;

private:
	void drawUpButton(gcn::Graphics *graphics) override;
	void drawDownButton(gcn::Graphics *graphics) override;
	void drawLeftButton(gcn::Graphics *graphics) override;
	void drawRightButton(gcn::Graphics *graphics) override;
	void drawHMarker(gcn::Graphics *graphics) override;
	void drawVMarker(gcn::Graphics *graphics) override;
	void drawHBar(gcn::Graphics *graphics) override;
	void drawVBar(gcn::Graphics *graphics) override;

	void drawUpPressedButton(gcn::Graphics *graphics);
	void drawDownPressedButton(gcn::Graphics *graphics);
	void drawLeftPressedButton(gcn::Graphics *graphics);
	void drawRightPressedButton(gcn::Graphics *graphics);
	void adjustSize();
private:
	std::shared_ptr<CGraphic> itemImage;
	std::shared_ptr<CGraphic> upButtonImage;
	std::shared_ptr<CGraphic> upPressedButtonImage;
	std::shared_ptr<CGraphic> downButtonImage;
	std::shared_ptr<CGraphic> downPressedButtonImage;
	std::shared_ptr<CGraphic> leftButtonImage;
	std::shared_ptr<CGraphic> leftPressedButtonImage;
	std::shared_ptr<CGraphic> rightButtonImage;
	std::shared_ptr<CGraphic> rightPressedButtonImage;
	std::shared_ptr<CGraphic> hBarButtonImage;
	std::shared_ptr<CGraphic> vBarButtonImage;
	std::shared_ptr<CGraphic> markerImage;

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
	explicit ImageDropDownWidget(std::unique_ptr<ImageListBox> imageListBox) :
		DropDownWidget(imageListBox.get()),
		mImageListBox(std::move(imageListBox))
	{
		this->mListBox->addActionListener(this);
	}

public:
	ImageDropDownWidget() : ImageDropDownWidget(std::make_unique<ImageListBox>()) {}
	void setItemImage(std::shared_ptr<CGraphic> image)
	{
		this->itemImage = image;
		this->mImageListBox->setItemImage(image);
	}
	void setDownNormalImage(std::shared_ptr<CGraphic> image) { DownNormalImage = image; }
	void setDownPressedImage(std::shared_ptr<CGraphic> image) { DownPressedImage = image; }

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
	std::shared_ptr<CGraphic> itemImage;
	std::shared_ptr<CGraphic> DownNormalImage;
	std::shared_ptr<CGraphic> DownPressedImage;
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

	void draw(gcn::Graphics *graphics) override;
	void logic() override;

	int run(bool loop = true);
	void stop(int result = 0, bool stopAll = false);
	void stopAll(int result = 0) { stop(result, true); }
	void addLogicCallback(LuaActionListener *listener);
	void setDrawMenusUnder(bool drawUnder) { this->drawUnder = drawUnder; }
	bool getDrawMenusUnder() const { return this->drawUnder; }

private:
	bool runLoop = true;
	int loopResult = 0;
	gcn::Widget *oldtop = nullptr;
	LuaActionListener *logiclistener = nullptr;
	bool drawUnder = false;
};

#endif
