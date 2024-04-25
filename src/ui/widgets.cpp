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
//      (c) Copyright 2005-2006 by Francois Beerten and Jimmy Salmon
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
#include "video.h"
#include "font.h"
#include "cursor.h"
#include "ui.h"
#include "widgets.h"
#include "network.h"
#include "netconnect.h"
#include "editor.h"
#include "sound.h"
#include "util.h"

#include <guichan/text.hpp>

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

// Guichan stuff we need
std::unique_ptr<gcn::Gui> Gui; /// A Gui object - binds it all together
static std::unique_ptr<gcn::SDLInput> Input; /// Input driver

static EventCallback GuichanCallbacks;

static std::stack<MenuScreen *> MenuStack;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

void addActionListener(gcn::Widget *widget, gcn::ActionListener *actionListener)
{
	// gcn::Widget::addActionListener is no longer virtual
	// so dispatch manually :-/

	if (auto w = dynamic_cast<ImageListBoxWidget *>(widget)) {
		w->addActionListener(actionListener);
	} else if (auto w = dynamic_cast<ListBoxWidget *>(widget)) {
		w->addActionListener(actionListener);
	} else {
		widget->addActionListener(actionListener);
	}
}

void setDirty(gcn::Widget *, bool isDirty)
{
	LuaError(Lua, "setDirty is no longer implemented\n");
}

void setDisabledColor(gcn::Widget*, const gcn::Color& color)
{
	LuaError(Lua, "widget no longer have setDisabledColor\n");
}

void setBaseColor(gcn::Widget *widget, const gcn::Color &color)
{
	// gcn::Widget::setBaseColor is no longer virtual
	// so dispatch manually :-/

	if (auto w = dynamic_cast<Windows *>(widget)) {
		w->setBaseColor(color);
	} else {
		widget->setBaseColor(color);
	}
}

void setBackgroundColor(gcn::Widget *widget, const gcn::Color &color)
{
	// gcn::Widget::setBackgroundColor is no longer virtual
	// so dispatch manually :-/

	if (auto w = dynamic_cast<Windows *>(widget)) {
		w->setBackgroundColor(color);
	} else if (auto w = dynamic_cast<ImageListBoxWidget *>(widget)) {
		w->setBackgroundColor(color);
	} else if (auto w = dynamic_cast<ListBoxWidget *>(widget)) {
		w->setBackgroundColor(color);
	} else {
		widget->setBackgroundColor(color);
	}
}

void setFont(gcn::Widget *widget, gcn::Font *font)
{
	// gcn::Widget::setFont is no longer virtual
	// so dispatch manually :-/

	if (auto w = dynamic_cast<ImageDropDownWidget *>(widget)) {
		w->setFont(font);
	} else if (auto w = dynamic_cast<gcn::DropDown *>(widget)) {
		w->setFont(font);
	} else {
		widget->setFont(font);
	}
}

static void MenuHandleMouseMove(const PixelPos &screenPos)
{
	PixelPos pos(screenPos);
	HandleCursorMove(&pos.x, &pos.y);
}
static void MenuHandleKeyDown(unsigned key, unsigned keychar)
{
	HandleKeyModifiersDown(key, keychar);
}
static void MenuHandleKeyUp(unsigned key, unsigned keychar)
{
	HandleKeyModifiersUp(key, keychar);
}
static void MenuHandleKeyRepeat(unsigned key, unsigned keychar)
{
	HandleKeyModifiersDown(key, keychar);
}

sdl2::RendererPtr guichanRenderer;

/**
**  Initializes the GUI stuff
*/
void initGuichan()
{
	auto *graphics = new gcn::SDLGraphics();

	// Set the target for the graphics object to be the screen.
	// In other words, we will draw to the screen.
	// Note, any surface will do, it doesn't have to be the screen.
	guichanRenderer.reset(SDL_CreateSoftwareRenderer(TheScreen));
	graphics->setTarget(guichanRenderer.get(), Video.Width, Video.Height);

	Input = std::make_unique<gcn::SDLInput>();

	Gui = std::make_unique<gcn::Gui>();
	Gui->setGraphics(graphics);
	Gui->setInput(Input.get());
	Gui->setTop(nullptr);

	GuichanCallbacks.ButtonPressed = [](unsigned) {};
	GuichanCallbacks.ButtonReleased = [](unsigned) {};
	GuichanCallbacks.MouseMoved = &MenuHandleMouseMove;
	GuichanCallbacks.MouseExit = &HandleMouseExit;
	GuichanCallbacks.KeyPressed = &MenuHandleKeyDown;
	GuichanCallbacks.KeyReleased = &MenuHandleKeyUp;
	GuichanCallbacks.KeyRepeated = &MenuHandleKeyRepeat;
	GuichanCallbacks.NetworkEvent = NetworkEvent;
}

/**
**  Free all guichan infrastructure
*/
void freeGuichan()
{
	if (Gui) {
		delete Gui->getGraphics();
		Gui = nullptr;
	}

	Input = nullptr;
}

/**
**  Handle input events
**
**  @param event  event to handle, null if no more events for this frame
*/
void handleInput(const SDL_Event *event)
{
	if (event) {
		if (Input) {
			try {
				Input->pushInput(*event);
			} catch (const gcn::Exception &) {
				// ignore unhandled buttons
			}
		}
	} else {
		if (Gui) {
			Gui->logic();
		}
	}
}

void DrawGuichanWidgets()
{
	if (Gui) {
		auto oldScreen = TheScreen;
		TheScreen = static_cast<gcn::SDLGraphics*>(Gui->getGraphics())->getTarget();
		Gui->draw();
		TheScreen = oldScreen;
	}
}

void setHotKey(gcn::Widget* widget, const char *keyStr)
{
	if (widget && keyStr) {
		gcn::Key key = GetHotKey(keyStr);
		widget->setHotKey(key.getValue());
	}
}

void scrollToBottom(gcn::ScrollArea *scrollArea)
{
	Assert(scrollArea);
	scrollArea->setVerticalScrollAmount(scrollArea->getVerticalMaxScroll());
}

void scrollToTop(gcn::ScrollArea *scrollArea)
{
	Assert(scrollArea);
	scrollArea->setVerticalScrollAmount(0);
}

/*----------------------------------------------------------------------------
--  LuaActionListener
----------------------------------------------------------------------------*/


/**
**  LuaActionListener constructor
**
**  @param l  Lua state
**  @param f  Listener function
*/
LuaActionListener::LuaActionListener(lua_State *l, lua_Object f) :
	callback(l, f)
{
}

/**
**  Called when an action is received from a Widget. It is used
**  to be able to receive a notification that an action has
**  occurred.
**
**  @param eventId  the identifier of the Widget
*/
void LuaActionListener::action(const gcn::ActionEvent &event) /* override */
{
	callback.call(event.getId());
}

void LuaActionListener::keyPressed(gcn::KeyEvent &event) /* override */
{
	callback.call("keyPress", to_utf8(event.getKey().getValue()));
	event.consume();
}

void LuaActionListener::keyReleased(gcn::KeyEvent &event) /* override */
{
	callback.call("keyRelease", to_utf8(event.getKey().getValue()));
	event.consume();
}

void LuaActionListener::hotKeyPressed(const gcn::Key &key) /* override */
{
	callback.call("hotKeyPress", to_utf8(key.getValue()));
}

void LuaActionListener::hotKeyReleased(const gcn::Key &key) /* override */
{
	callback.call("hotKeyRelease", to_utf8(key.getValue()));
}

void LuaActionListener::mouseEntered(gcn::MouseEvent &event) /* override */
{
	callback.call("mouseIn");
	event.consume();
}

void LuaActionListener::mouseExited(gcn::MouseEvent &event) /* override */
{
	callback.call("mouseOut");
	event.consume();
}

void LuaActionListener::mousePressed(gcn::MouseEvent &event) /* override */
{
	callback.call("mousePress", event.getButton());
	event.consume();
}

void LuaActionListener::mouseReleased(gcn::MouseEvent &event) /* override */
{
	callback.call("mouseRelease", event.getButton());
	event.consume();
}

void LuaActionListener::mouseClicked(gcn::MouseEvent &event) /* override */
{
	callback.call("mouseClick", event.getButton(), event.getClickCount());
	event.consume();
}

void LuaActionListener::mouseWheelMovedUp(gcn::MouseEvent &event) /* override */
{
	callback.call("mouseWheelUp");
	event.consume();
}

void LuaActionListener::mouseWheelMovedDown(gcn::MouseEvent &event)  /* override */ {
	callback.call("mouseWheelDown");
	event.consume();
}

void LuaActionListener::mouseMoved(gcn::MouseEvent &event) /* override */
{}


/**
**  LuaActionListener destructor
*/
LuaActionListener::~LuaActionListener()
{
}

/*----------------------------------------------------------------------------
--  CImageButton
----------------------------------------------------------------------------*/


/**
**  ImageButton constructor
*/
CImageButton::CImageButton() : Button()
{
	setForegroundColor(0xffffff);
}

/**
**  ImageButton constructor
**
**  @param caption  Caption text
*/
CImageButton::CImageButton(const std::string &caption) : Button(caption)
{
	setForegroundColor(0xffffff);
}

/**
**  Draw the image button
**
**  @param graphics  Graphics object to draw with
*/
void CImageButton::draw(gcn::Graphics *graphics) /* override */
{
	if (!normalImage) {
		if (this->getBackgroundColor().a == 0) {
			return;
		}

		Button::draw(graphics);
		return;
	}

	gcn::Image *img;

	if (!isEnabled()) {
		img = disabledImage ? disabledImage : normalImage;
	} else if (isPressed()) {
		img = pressedImage ? pressedImage : normalImage;
#if 0
	} else if (0 && hasMouse()) {
		// FIXME: add mouse-over image
		img = nullptr;
#endif
	} else {
		img = normalImage;
	}
	graphics->drawImage(img, 0, 0, 0, 0,
						img->getWidth(), img->getHeight());

	graphics->setColor(getForegroundColor());

	int textX;
	int textY = getHeight() / 2 - getFont()->getHeight() / 2;

	switch (getAlignment()) {
		case gcn::Graphics::Left:
			textX = 4;
			break;
		case gcn::Graphics::Center:
			textX = getWidth() / 2;
			break;
		case gcn::Graphics::Right:
			textX = getWidth() - 4;
			break;
		default:
			textX = 0;
			Assert(!"Unknown alignment.");
			//throw GCN_EXCEPTION("Unknown alignment.");
	}

	graphics->setFont(getFont());

	const bool is_normal = !mHasMouse;
	if (auto* font = dynamic_cast<CFont*>(getFont())) {
		font->setIsNormal(is_normal);
	}

	if (isPressed()) {
		graphics->drawText(getCaption(), textX + 4, textY + 4, getAlignment());
	} else {
		graphics->drawText(getCaption(), textX + 2, textY + 2, getAlignment());
	}
	if (auto* font = dynamic_cast<CFont*>(getFont())) {
		font->setIsNormal(true);
	}

	if (isFocused()) {
		graphics->drawRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
	}
}

/**
**  Automatically adjust the size of an image button
*/
void CImageButton::adjustSize()
{
	if (normalImage) {
		setWidth(normalImage->getWidth());
		setHeight(normalImage->getHeight());
	} else {
		Button::adjustSize();
	}
}

/*----------------------------------------------------------------------------
--  ImageRadioButton
----------------------------------------------------------------------------*/

/**
**  ImageRadioButton constructor
*/
ImageRadioButton::ImageRadioButton(const std::string &caption,
								   const std::string &group, bool marked) :
	gcn::RadioButton(caption, group, marked)
{
}

/**
**  Draw the image radio button (not the caption)
*/
void ImageRadioButton::drawBox(gcn::Graphics *graphics) /* override */
{
	gcn::Image *img = nullptr;

	if (isSelected()) {
		if (isEnabled() == false) {
			img = checkedDisabledImage;
		} else if (mMouseDown) {
			img = checkedPressedImage;
		} else {
			img = checkedNormalImage;
		}
	} else {
		if (isEnabled() == false) {
			img = uncheckedDisabledImage;
		} else if (mMouseDown) {
			img = uncheckedPressedImage;
		} else {
			img = uncheckedNormalImage;
		}
	}

	if (img) {
		graphics->drawImage(img, 0, 0, 0, (getHeight() - img->getHeight()) / 2,
							img->getWidth(), img->getHeight());
	} else {
		RadioButton::drawBox(graphics);
	}
}

/**
**  Draw the image radio button
*/
void ImageRadioButton::draw(gcn::Graphics *graphics) /* override */
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

	if (isFocused()) {
		graphics->drawRectangle(gcn::Rectangle(width - 4, 0, getWidth() - width + 3, getHeight()));
	}
}

/**
**  Mouse button pressed callback
*/
void ImageRadioButton::mousePressed(gcn::MouseEvent &event) /* override */
{
	if (event.getButton() == gcn::MouseInput::Left) {
		mMouseDown = true;
		event.consume();
	}
}

/**
**  Mouse button released callback
*/
void ImageRadioButton::mouseReleased(gcn::MouseEvent &event) /* override */
{
	if (event.getButton() == gcn::MouseInput::Left) {
		mMouseDown = false;
		event.consume();
	}
}

/**
**  Mouse clicked callback
*/
void ImageRadioButton::mouseClicked(gcn::MouseEvent &event) /* override */
{
	if (event.getButton() == gcn::MouseInput::Left) {
		setSelected(true);
		distributeActionEvent();
		event.consume();
	}
}

/**
**  Adjusts the size to fit the image and font size
*/
void ImageRadioButton::adjustSize()
{
	int width, height;

	height = getFont()->getHeight();
	if (uncheckedNormalImage) {
		width = uncheckedNormalImage->getWidth();
		width += width / 2;
		height = std::max(height, uncheckedNormalImage->getHeight());
	} else {
		width = getFont()->getHeight();
		width += width / 2;
	}

	setHeight(height);
	setWidth(getFont()->getWidth(mCaption) + width);
}


/*----------------------------------------------------------------------------
--  ImageCheckbox
----------------------------------------------------------------------------*/

/**
**  Image checkbox constructor
*/
ImageCheckBox::ImageCheckBox(const std::string &caption, bool marked) :
	gcn::CheckBox(caption, marked)
{}

/**
**  Draw the image checkbox
*/
void ImageCheckBox::draw(gcn::Graphics *graphics) /* override */
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

	if (isFocused()) {
		graphics->drawRectangle(gcn::Rectangle(width - 4, 0, getWidth() - width + 3, getHeight()));
	}
}

/**
**  Draw the checkbox (not the caption)
*/
void ImageCheckBox::drawBox(gcn::Graphics *graphics) /* override */
{
	gcn::Image *img = nullptr;

	if (isSelected()) {
		if (isEnabled() == false) {
			img = checkedDisabledImage;
		} else if (mMouseDown) {
			img = checkedPressedImage;
		} else {
			img = checkedNormalImage;
		}
	} else {
		if (isEnabled() == false) {
			img = uncheckedDisabledImage;
		} else if (mMouseDown) {
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
void ImageCheckBox::mousePressed(gcn::MouseEvent &event) /* override */
{
	if (event.getButton() == gcn::MouseInput::Left) {
		mMouseDown = true;
		event.consume();
	}
}

/**
**  Mouse button released callback
*/
void ImageCheckBox::mouseReleased(gcn::MouseEvent &event) /* override */
{
	if (event.getButton() == gcn::MouseInput::Left) {
		mMouseDown = false;
	}
}

/**
**  Mouse clicked callback
*/
void ImageCheckBox::mouseClicked(gcn::MouseEvent &event) /* override */
{
	if (event.getButton() == gcn::MouseInput::Left) {
		toggleSelected();
		event.consume();
	}
}

/**
**  Adjusts the size to fit the image and font size
*/
void ImageCheckBox::adjustSize()
{
	int width, height;

	height = getFont()->getHeight();
	if (uncheckedNormalImage) {
		width = uncheckedNormalImage->getWidth();
		width += width / 2;
		height = std::max(height, uncheckedNormalImage->getHeight());
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
ImageSlider::ImageSlider(double scaleEnd) : Slider(scaleEnd)
{}

/**
**  Image slider constructor
*/
ImageSlider::ImageSlider(double scaleStart, double scaleEnd) : Slider(scaleStart, scaleEnd)
{}

/**
**  Draw the image slider marker
*/
void ImageSlider::drawMarker(gcn::Graphics *graphics) /* override */
{
	gcn::Image *img = markerImage;

	if (isEnabled()) {
		if (img) {
			if (getOrientation() == Orientation::Horizontal) {
				int v = getMarkerPosition();
				graphics->drawImage(img, 0, 0, v, 0,
					img->getWidth(), img->getHeight());
			} else {
				int v = (getHeight() - getMarkerLength()) - getMarkerPosition();
				graphics->drawImage(img, 0, 0, 0, v,
					img->getWidth(), img->getHeight());
			}
		} else {
			Slider::drawMarker(graphics);
		}
	}
}

/**
**  Draw the image slider
*/
void ImageSlider::draw(gcn::Graphics *graphics) /* override */
{
	gcn::Image *img = nullptr;

	if (isEnabled()) {
		img = backgroundImage;
	} else {
		img = disabledBackgroundImage;
	}

	if (img) {
		graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());
		if (isEnabled()) {
			drawMarker(graphics);
		}
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

/**
**  Set the disabled background image
*/
void ImageSlider::setDisabledBackgroundImage(gcn::Image *image)
{
	disabledBackgroundImage = image;
}


/*----------------------------------------------------------------------------
--  MultiLineLabel
----------------------------------------------------------------------------*/

/**
**  MultiLineLabel constructor
*/
MultiLineLabel::MultiLineLabel(const std::string &caption)
{
	this->mCaption = caption;

	this->mLineWidth = 999999;
	this->wordWrap();
	this->adjustSize();
}

/**
**  Set the caption
*/
void MultiLineLabel::setCaption(const std::string &caption)
{
	this->mCaption = caption;
	this->wordWrap();
}

/**
**  Get the caption
*/
const std::string &MultiLineLabel::getCaption() const
{
	return this->mCaption;
}

/**
**  Set the horizontal alignment
*/
void MultiLineLabel::setAlignment(gcn::Graphics::Alignment alignment)
{
	this->mAlignment = alignment;
}

/**
**  Get the horizontal alignment
*/
gcn::Graphics::Alignment MultiLineLabel::getAlignment() const
{
	return this->mAlignment;
}

/**
**  Set the vertical alignment
*/
void MultiLineLabel::setVerticalAlignment(unsigned int alignment)
{
	this->mVerticalAlignment = alignment;
}

/**
**  Get the vertical alignment
*/
unsigned int MultiLineLabel::getVerticalAlignment() const
{
	return this->mVerticalAlignment;
}

/**
**  Set the line width
*/
void MultiLineLabel::setLineWidth(int width)
{
	this->mLineWidth = width;
	this->wordWrap();
}

/**
**  Get the line width
*/
int MultiLineLabel::getLineWidth() const
{
	return this->mLineWidth;
}

/**
**  Adjust the size
*/
void MultiLineLabel::adjustSize()
{
	int width = 0;
	for (const auto& s : this->mTextRows) {
		int w = this->getFont()->getWidth(s);
		if (width < w) {
			width = std::min(w, this->mLineWidth);
		}
	}
	this->setWidth(width);
	this->setHeight(this->getFont()->getHeight() * this->mTextRows.size());
}

/**
**  Draw the label
*/
void MultiLineLabel::draw(gcn::Graphics *graphics)
{
	graphics->setFont(getFont());
	graphics->setColor(getForegroundColor());

	int textX, textY;
	switch (this->getAlignment()) {
		case gcn::Graphics::Alignment::Left:
			textX = 0;
			break;
		case gcn::Graphics::Alignment::Center:
			textX = this->getWidth() / 2;
			break;
		case gcn::Graphics::Alignment::Right:
			textX = this->getWidth();
			break;
		default:
			textX = 0;
			Assert(!"Unknown alignment.");
			//throw GCN_EXCEPTION("Unknown alignment.");
	}
	switch (this->getVerticalAlignment()) {
		case TOP:
			textY = 0;
			break;
		case CENTER:
			textY = (this->getHeight() - (int)this->mTextRows.size() * this->getFont()->getHeight()) / 2;
			break;
		case BOTTOM:
			textY = this->getHeight() - (int)this->mTextRows.size() * this->getFont()->getHeight();
			break;
		default:
			textY = 0;
			Assert(!"Unknown alignment.");
			//throw GCN_EXCEPTION("Unknown alignment.");
	}

	for (int i = 0; i < (int)this->mTextRows.size(); ++i) {
		if (this->mTextRows[i].length() != 0)
			graphics->drawText(this->mTextRows[i], textX, textY + i * this->getFont()->getHeight(),
						   this->getAlignment());
	}
}

/**
**  Draw the border
*/
void MultiLineLabel::drawFrame(gcn::Graphics *graphics) /* override */
{
	gcn::Color faceColor = getBaseColor();
	gcn::Color highlightColor, shadowColor;
	int alpha = getBaseColor().a;
	int width = getWidth() + getFrameSize() * 2 - 1;
	int height = getHeight() + getFrameSize() * 2 - 1;
	highlightColor = faceColor + 0x303030;
	highlightColor.a = alpha;
	shadowColor = faceColor - 0x303030;
	shadowColor.a = alpha;

	for (unsigned int i = 0; i < getFrameSize(); ++i) {
		graphics->setColor(shadowColor);
		graphics->drawLine(i, i, width - i, i);
		graphics->drawLine(i, i + 1, i, height - i - 1);
		graphics->setColor(highlightColor);
		graphics->drawLine(width - i, i + 1, width - i, height - i);
		graphics->drawLine(i, height - i, width - i - 1, height - i);
	}
}

/**
**  Do word wrap
*/
void MultiLineLabel::wordWrap()
{
	gcn::Font *font = this->getFont();
	int lineWidth = this->getLineWidth();
	std::string str = this->getCaption();
	size_t pos, lastPos;
	std::string substr;
	bool done = false;
	bool first = true;

	this->mTextRows.clear();

	while (!done) {
		if (str.find('\n') != std::string::npos || font->getWidth(str) > lineWidth) {
			// string too wide or has a newline, split it up
			first = true;
			lastPos = 0;
			while (1) {
				// look for any whitespace
				pos = str.find_first_of(" \t\n", first ? 0 : lastPos + 1);
				if (pos != std::string::npos) {
					// found space, now check width
					substr = str.substr(0, pos);
					if (font->getWidth(substr) > lineWidth) {
						// sub-string is too big, use last good position
						if (first) {
							// didn't find a good last position
							substr = str.substr(0, pos);
							this->mTextRows.push_back(substr);
							str = str.substr(pos + 1);
							break;
						} else {
							substr = str.substr(0, lastPos);
							this->mTextRows.push_back(substr);
							// If we stopped at a space then skip any extra spaces but stop at a newline
							if (str[lastPos] != '\n') {
								while (str[lastPos + 1] == ' ' || str[lastPos + 1] == '\t' || str[lastPos + 1] == '\n') {
									++lastPos;
									if (str[lastPos] == '\n') {
										break;
									}
								}
							}
							str = str.substr(lastPos + 1);
							break;
						}
					} else {
						// sub-string is small enough
						// stop if we found a newline, otherwise look for next space
						if (str[pos] == '\n') {
							substr = str.substr(0, pos);
							this->mTextRows.push_back(substr);
							str = str.substr(pos + 1);
							break;
						}
					}
				} else {
					// no space found
					if (first) {
						// didn't find a good last position, we're done
						this->mTextRows.push_back(str);
						done = true;
						break;
					} else {
						substr = str.substr(0, lastPos);
						this->mTextRows.push_back(substr);
						str = str.substr(lastPos + 1);
						break;
					}
				}
				lastPos = pos;
				first = false;
			}
		} else {
			// string small enough
			this->mTextRows.push_back(str);
			done = true;
		}
	}
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
	gcn::ScrollArea(nullptr, gcn::ScrollArea::ScrollPolicy::ShowNever, gcn::ScrollArea::ScrollPolicy::ShowNever),
	speedY(1.f)
{
	container.setDimension(gcn::Rectangle(0, 0, width, height));
	container.setOpaque(false);
	setOpaque(false);
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
void ScrollingWidget::logic() /* override */
{
	if (container.getHeight() + containerY - speedY > 0) {
		// the bottom of the container is lower than the top
		// of the widget. It is thus still visible.
		containerY -= speedY;
		container.setY((int)containerY);
	} else if (!finished) {
		finished = true;
		distributeActionEvent();
	}
}

/**
**  Restart animation to the beginning.
*/
void ScrollingWidget::restart()
{
	container.setY(0);
	containerY = 0.f;
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
	this->add(&scroll, 0, 0);
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
**    if you go virtually outside the container then go back,
**    you have to wait the virtual cursor are in the container.
**    It is because x, y argument refer to a virtual cursor :(
**  @note An another thing is strange
**    when the container is a "scrollable" ScrollArea with the cursor.
**    The cursor can go outside the visual area.
*/
void Windows::mouseDragged(gcn::MouseEvent &event) /* override */
{
	auto *bcontainer = getParent();
	int criticalx;
	int criticaly;

	if (!isMovable()) {
		return;
	}
	int diffx = event.getX() - mDragOffsetX;
	int diffy = event.getY() - mDragOffsetY;
	if (blockwholewindow) {
		criticalx = getX();
		criticaly = getY();
	} else {
		criticalx = getX() + mDragOffsetX;
		criticaly = getY() + mDragOffsetY;
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
	int x = getX() + diffx;
	int y = getY() + diffy;
	setPosition(x, y);

	// Move the cursor.
	// Useful only when window reachs the limit.
	int absx;
	int absy;
	getAbsolutePosition(absx, absy);
	CursorScreenPos.x = absx + mDragOffsetX;
	CursorScreenPos.y = absy + mDragOffsetY;
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
--  CTextBox
----------------------------------------------------------------------------*/

void CTextBox::mousePressed(gcn::MouseEvent &event) /* override */
{
	TextBox::mousePressed(event);

	if (event.getButton() == gcn::MouseInput::Middle) {
		std::string str;
		if (GetClipboard(str) >= 0) {
			for (auto c : str) {
				mText->insert(c);
			}
		}
	}
}

void CTextBox::keyPressed(gcn::KeyEvent &event) /* override */
{
	using gcn::Key;
	Key key = event.getKey();
	const auto getCaretUnicodeColumn = [&]() {
		const auto &text = mText->getRow(mText->getCaretRow());
		auto pos = mText->getCaretColumn();
		int res = 0;
		while (pos != 0) {
			pos = UTF8GetPrev(text, pos);
			++res;
		}
		return res;
	};
	const auto setCaretUnicodeColumn = [&](int pos) {
		const auto &text = mText->getRow(mText->getCaretRow());
		int newCaretPosition = 0;
		for (int i = 0; i != pos; ++i) {
			newCaretPosition = UTF8GetNext(text, newCaretPosition);
		}
		mText->setCaretColumn(newCaretPosition);
	};

	if (key.getValue() == Key::Left) {
		const auto newPos = UTF8GetPrev(mText->getContent(), mText->getCaretPosition());
		mText->setCaretPosition(newPos);
	} else if (key.getValue() == Key::Right) {
		const auto newPos = UTF8GetNext(mText->getContent(), mText->getCaretPosition());
		mText->setCaretPosition(newPos);
	} else if (key.getValue() == Key::Down) {
		const auto oldCaretColumn = getCaretUnicodeColumn();
		mText->setCaretRow(mText->getCaretRow() + 1);
		setCaretUnicodeColumn(oldCaretColumn);
	} else if (key.getValue() == Key::Up) {
		const auto oldCaretColumn = getCaretUnicodeColumn();
		mText->setCaretRow(mText->getCaretRow() - 1);
		setCaretUnicodeColumn(oldCaretColumn);
	} else if (key.getValue() == Key::Home) {
		mText->setCaretColumn(0);
	} else if (key.getValue() == Key::End) {
		mText->setCaretColumn(mText->getNumberOfCharacters(mText->getCaretRow()));
	} else if (key.getValue() == Key::Enter && mEditable) {
		mText->insert('\n');
	} else if (key.getValue() == Key::Backspace && mEditable) {
		const auto diff =
			UTF8GetPrev(mText->getContent(), mText->getCaretPosition()) - mText->getCaretPosition();
		mText->remove(diff);
	} else if (key.getValue() == Key::Delete && mEditable) {
		const auto diff =
			UTF8GetNext(mText->getContent(), mText->getCaretPosition()) - mText->getCaretPosition();
		mText->remove(diff);
	} else if (key.getValue() == Key::PageUp) {
		if (Widget *par = getParent()) {
			const int rowsPerPage = par->getChildrenArea().height / getFont()->getHeight();
			const auto oldCaretColumn = getCaretUnicodeColumn();
			mText->setCaretRow(mText->getCaretRow() - rowsPerPage);
			setCaretUnicodeColumn(oldCaretColumn);
		}
	} else if (key.getValue() == Key::PageDown) {
		if (Widget *par = getParent()) {
			const int rowsPerPage = par->getChildrenArea().height / getFont()->getHeight();
			const auto oldCaretColumn = getCaretUnicodeColumn();
			mText->setCaretRow(mText->getCaretRow() + rowsPerPage);
			setCaretUnicodeColumn(oldCaretColumn);
		}
	} else if (key.getValue() == Key::Tab && mEditable) {
		mText->insert(' ');
		mText->insert(' ');
		mText->insert(' ');
		mText->insert(' ');
	} else if (event.isControlPressed() && key.getValue() == 'v' && mEditable) { // ctrl-v
		std::string str;
		if (GetClipboard(str) >= 0) {
			for (auto c : str) {
				mText->insert(c);
			}
		}
	} else if (key.isCharacter() && !event.isAltPressed() && !event.isControlPressed()
	           && mEditable) {
		for (auto c : to_utf8(key.getValue())) {
			mText->insert(c);
		}
	}
	adjustSize();
	scrollToCaret();

	event.consume();
}

/*----------------------------------------------------------------------------
--  CTextField
----------------------------------------------------------------------------*/

void CTextField::draw(gcn::Graphics *graphics) /* override */
{
	const std::string drawText =
		isPassword ? std::string(mText->getContent().size(), '*') : mText->getContent();

	gcn::Color faceColor = getBackgroundColor();
	graphics->setColor(faceColor);
	graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));

	if (isFocused()) {
		drawCaret(graphics, getFont()->getWidth(drawText.substr(0, mText->getCaretPosition())) - mXScroll);
	}

	graphics->setColor(getForegroundColor());
	gcn::Font *font = getFont();
	graphics->setFont(font);

	const int x = 1 - mXScroll;
	const int y = 1;
	if (mSelectEndOffset != 0) {
		unsigned int first;
		unsigned int len;
		getTextSelectionPositions(&first, &len);

		const int selX = font->getWidth(drawText.substr(0, first));
		const int selW = font->getWidth(drawText.substr(first, len));

		graphics->setColor(gcn::Color(127, 127, 127));
		graphics->fillRectangle(gcn::Rectangle(x + selX, y, selW, font->getHeight()));
	}

	graphics->drawText(drawText, x, y);
}

void CTextField::mousePressed(gcn::MouseEvent &event) /* override */
{
	TextField::mousePressed(event);

	if (event.getButton() == gcn::MouseInput::Left) {
		mSelectStart = mText->getCaretPosition();
		mSelectEndOffset = 0;
	} else if (event.getButton() == gcn::MouseInput::Middle) {
		std::string str;
		if (GetClipboard(str) >= 0) {
			for (auto c : str) {
				mText->insert(c);
			}
		}
	}
}

void CTextField::mouseDragged(gcn::MouseEvent &event) /* override */
{
	if (event.getButton() == gcn::MouseInput::Left) {
		mText->setCaretPosition(getFont()->getStringIndexAt(mText->getContent(), event.getX() + mXScroll));
		mSelectEndOffset = static_cast<int>(mText->getCaretPosition()) - mSelectStart;
	}
}

void CTextField::keyPressed(gcn::KeyEvent &event) /* override */
{
	using gcn::Key;
	Key key = event.getKey();

	unsigned int selFirst;
	unsigned int selLen;

	getTextSelectionPositions(&selFirst, &selLen);
	const auto deleteSelection = [&]() {
		auto oldCaretPosition = mText->getCaretPosition();

		mText->setCaretPosition(selFirst);
		mText->remove(selLen);

		if (selFirst < oldCaretPosition) {
			oldCaretPosition -= selLen;
		}
		mText->setCaretPosition(oldCaretPosition);
		mSelectStart = oldCaretPosition;
		mSelectEndOffset = 0;
	};
	const int charactersCount = mText->getNumberOfCharacters() - mText->getNumberOfRows();

	if (key.getValue() == Key::Left) {
		const auto newPos =
			std::max(0, UTF8GetPrev(mText->getContent(), mText->getCaretPosition()));
		if (event.isShiftPressed()) {
			mSelectEndOffset += newPos - mText->getCaretPosition();
		} else {
			mSelectStart = newPos;
			mSelectEndOffset = 0;
		}
		mText->setCaretPosition(newPos);
	} else if (key.getValue() == Key::Right) {
		const auto newPos =
			std::min(charactersCount, UTF8GetNext(mText->getContent(), mText->getCaretPosition()));
		if (event.isShiftPressed()) {
			mSelectEndOffset += newPos - mText->getCaretPosition();
		} else {
			mSelectStart = newPos;
			mSelectEndOffset = 0;
		}
		mText->setCaretPosition(newPos);
	} else if (key.getValue() == gcn::Key::Home
	           || (event.isControlPressed() && key.getValue() == 'a')) { // ctrl-a
		if (event.isShiftPressed()) {
			mSelectEndOffset -= mText->getCaretPosition();
		} else {
			mSelectStart = 0;
			mSelectEndOffset = 0;
		}
		mText->setCaretPosition(0);
	} else if (key.getValue() == gcn::Key::End
	           || (event.isControlPressed() && key.getValue() == 'e')) { //ctrl-e
		if (event.isShiftPressed()) {
			mSelectEndOffset += charactersCount - mText->getCaretPosition();
		} else {
			mSelectStart = charactersCount;
			mSelectEndOffset = 0;
		}
		mText->setCaretPosition(charactersCount);
	} else if (key.getValue() == Key::Enter && mEditable) {
		distributeActionEvent();
	} else if ((key.getValue() == Key::Backspace
	            || (event.isControlPressed() && key.getValue() == 'h'))
	           && mEditable) {
		if (selLen > 0) {
			deleteSelection();
		} else {
			const auto diff = UTF8GetPrev(mText->getContent(), mText->getCaretPosition())
			                - mText->getCaretPosition();
			mText->remove(diff);
		}
	} else if (key.getValue() == Key::Delete && mEditable) {
		if (selLen > 0) {
			deleteSelection();
		} else {
			const auto diff = UTF8GetNext(mText->getContent(), mText->getCaretPosition())
			                - mText->getCaretPosition();
			mText->remove(diff);
		}
	} else if (key.getValue() == Key::Tab && mEditable) {
		if (selLen > 0) {
			deleteSelection();
		}
		mText->insert(' ');
		mText->insert(' ');
		mText->insert(' ');
		mText->insert(' ');
		mSelectStart = mText->getCaretPosition();
	} else if (event.isControlPressed() && key.getValue() == 'c') {
		std::string s = std::string(mText->getContent().substr(selFirst, selLen));
		SetClipboard(s);
	} else if (event.isControlPressed() && key.getValue() == 'v' && mEditable) { // ctrl-v
		if (selLen > 0) {
			deleteSelection();
		}
		std::string str;
		if (GetClipboard(str) >= 0) {
			for (auto c : str) {
				mText->insert(c);
			}
		}
		mSelectStart = mText->getCaretPosition();
	} else if (event.isControlPressed() && key.getValue() == 'u') { // ctrl-u
		setText("");
		mSelectStart = mText->getCaretPosition();
		mSelectEndOffset = 0;
	} else if (key.isCharacter() && !event.isAltPressed() && !event.isControlPressed()
	           && mEditable) {
		if (selLen > 0) {
			deleteSelection();
		}
		for (auto c : to_utf8(key.getValue())) {
			mText->insert(c);
		}
		mSelectStart = mText->getCaretPosition();
	}
	fixScroll();
	event.consume();
}

void CTextField::getTextSelectionPositions(unsigned int *first, unsigned int *len) const
{
	if (mSelectEndOffset < 0) {
		*first = mSelectStart + mSelectEndOffset;
		*len = -mSelectEndOffset;
	} else {
		*first = mSelectStart;
		*len = mSelectEndOffset;
	}
}

/*----------------------------------------------------------------------------
--  ImageTextField
----------------------------------------------------------------------------*/

void ImageTextField::draw(gcn::Graphics *graphics) /* override */
{
	gcn::Font *font;
	int x, y;
	std::string drawText =
		isPassword ? std::string(mText->getContent().size(), '*') : mText->getContent();
	CGraphic *img = this->itemImage;
	if (!img) {
		ErrorPrint("Not all graphics for ImageTextField were set\n");
		ExitFatal(1);
	}
	img->Resize(getWidth(), img->getHeight());
	graphics->drawImage(img, 0, 0, 0, 0, getWidth(), img->getHeight());

	if (isFocused())
	{
		drawCaret(graphics, getFont()->getWidth(drawText.substr(0, mText->getCaretPosition())) - mXScroll);
	}

	graphics->setColor(getForegroundColor());
	font = getFont();
	graphics->setFont(font);

	x = 1 - mXScroll;
	y = 1;

	if (mSelectEndOffset != 0)
	{
		unsigned int first;
		unsigned int len;
		int selX;
		int selW;
		std::string tmpStr;

		getTextSelectionPositions(&first, &len);

		tmpStr = std::string(drawText.substr(0, first));
		selX = font->getWidth(tmpStr);

		tmpStr = std::string(drawText.substr(first, len));
		selW = font->getWidth(tmpStr);

		graphics->setColor(gcn::Color(127, 127, 127));
		graphics->fillRectangle(gcn::Rectangle(x + selX, y, selW, font->getHeight()));
	}

	graphics->drawText(drawText, x, y);
}

void ImageTextField::drawFrame(gcn::Graphics *graphics) /* override */
{
	gcn::Color faceColor = getBaseColor();
	gcn::Color highlightColor, shadowColor;
	int alpha = getBaseColor().a;
	int width = getWidth() + getFrameSize() * 2 - 1;
	int height = getHeight() + getFrameSize() * 2 - 1;
	height = itemImage ? std::max<int>(height, itemImage->getHeight()) : height;
	highlightColor = faceColor + 0x303030;
	highlightColor.a = alpha;
	shadowColor = faceColor - 0x303030;
	shadowColor.a = alpha;

	unsigned int i;
	for (i = 0; i < getFrameSize(); ++i)
	{
		graphics->setColor(shadowColor);
		graphics->drawLine(i,i, width - i, i);
		graphics->drawLine(i,i + 1, i, height - i - 1);
		graphics->setColor(highlightColor);
		graphics->drawLine(width - i,i + 1, width - i, height - i);
		graphics->drawLine(i,height - i, width - i - 1, height - i);
	}
}

/*----------------------------------------------------------------------------
--  StringListModel
----------------------------------------------------------------------------*/

int StringListModel::getIdxOfElement(std::string_view element)
{
	auto result = ranges::find(this->list, element);
	if (result != this->list.end()) {
		return result - this->list.begin();
	} else {
		return -1;
	}

}

/*----------------------------------------------------------------------------
--  LuaListModel
----------------------------------------------------------------------------*/

/**
**  Set the list
*/
void LuaListModel::setList(lua_State *lua, lua_Object *lo)
{
	list.clear();

	const int args = lua_rawlen(lua, *lo);
	for (int j = 0; j < args; ++j) {
		list.push_back(std::string(LuaToString(lua, *lo, j + 1)));
	}
}

int LuaListModel::getIdxOfElement(std::string_view element)
{
	auto result = ranges::find(this->list, element);
	if (result != this->list.end()) {
		return result - this->list.begin();
	} else {
		return -1;
	}
}

/*----------------------------------------------------------------------------
--  ImageListBox
----------------------------------------------------------------------------*/

ImageListBox::ImageListBox(gcn::ListModel *listModel) : gcn::ListBox(listModel)
{
}

void ImageListBox::draw(gcn::Graphics *graphics) /* override */
{
	if (mListModel == nullptr) {
		return;
	}

	graphics->setColor(getForegroundColor());
	graphics->setFont(getFont());

	int i, fontHeight;
	int y = 0;
	CGraphic *img = itemImage;
	img->Resize(getWidth(), img->getHeight());

	fontHeight = std::max<int>(getFont()->getHeight(), img->getHeight());

	/**
	* @todo Check cliprects so we do not have to iterate over elements in the list model
	*/
	for (i = 0; i < mListModel->getNumberOfElements(); ++i) {
		graphics->drawImage(img, 0, 0, 0, y, getWidth(), img->getHeight());
		if (i == mSelected) {
			graphics->drawText("~<" + mListModel->getElementAt(i) + "~>", 1, y + (fontHeight - getFont()->getHeight()) / 2);
		} else {
			graphics->drawText(mListModel->getElementAt(i), 1, y + (fontHeight - getFont()->getHeight()) / 2);
		}

		y += fontHeight;
	}
	img->SetOriginalSize();
}

void ImageListBox::drawFrame(gcn::Graphics *graphics) /* override */
{
	gcn::Color faceColor = getBaseColor();
	gcn::Color highlightColor, shadowColor;
	int alpha = getBaseColor().a;
	int width = getWidth() + getFrameSize() * 2 - 1;
	int height = getHeight() + getFrameSize() * 2 - 1;
	highlightColor = faceColor + 0x303030;
	highlightColor.a = alpha;
	shadowColor = faceColor - 0x303030;
	shadowColor.a = alpha;

	unsigned int i;
	for (i = 0; i < getFrameSize(); ++i)
	{
		graphics->setColor(shadowColor);
		graphics->drawLine(i,i, width - i, i);
		graphics->drawLine(i,i + 1, i, height - i - 1);
		graphics->setColor(highlightColor);
		graphics->drawLine(width - i,i + 1, width - i, height - i);
		graphics->drawLine(i,height - i, width - i - 1, height - i);
	}
}

void ImageListBox::adjustSize()
{
	if (mListModel != nullptr)
	{
		setHeight((itemImage ? std::max<int>(getFont()->getHeight(), itemImage->getHeight()) : getFont()->getHeight()) * mListModel->getNumberOfElements());
	}
}

void ImageListBox::mousePressed(gcn::MouseEvent &event) /* override */
{
	if (event.getButton() == gcn::MouseInput::Left) {
		const auto itemHeight = itemImage
		                          ? std::max<int>(getFont()->getHeight(), itemImage->getHeight())
		                          : getFont()->getHeight();
		setSelected(event.getY() / itemHeight);
		distributeActionEvent();
	} else if (event.getButton() == gcn::MouseInput::Right) {
		setSelected(-1);
		distributeActionEvent();
	}
}

static void scrollToRectangle(gcn::ScrollArea &scrollArea, const gcn::Rectangle &rectangle)
{
	gcn::Rectangle contentDim = scrollArea.getChildrenArea();

	if (rectangle.x + rectangle.width > scrollArea.getHorizontalScrollAmount() + contentDim.width) {
		scrollArea.setHorizontalScrollAmount(rectangle.x + rectangle.width - contentDim.width);
	}

	if (rectangle.y + rectangle.height > scrollArea.getVerticalScrollAmount() + contentDim.height) {
		scrollArea.setVerticalScrollAmount(rectangle.y + rectangle.height - contentDim.height);
	}

	if (rectangle.x < scrollArea.getHorizontalScrollAmount()) {
		scrollArea.setHorizontalScrollAmount(rectangle.x);
	}

	if (rectangle.y < scrollArea.getVerticalScrollAmount()) {
		scrollArea.setVerticalScrollAmount(rectangle.y);
	}
}

void ImageListBox::setSelected(int selected)
{
	if (mListModel == nullptr)
	{
		mSelected = -1;
	}
	else
	{
		if (selected < 0)
		{
			mSelected = -1;
		}
		else if (selected >= mListModel->getNumberOfElements())
		{
			mSelected = mListModel->getNumberOfElements() - 1;
		}
		else
		{
			mSelected = selected;
		}
		gcn::ScrollArea* scrollArea = dynamic_cast<gcn::ScrollArea *>(getParent());
		if (scrollArea != nullptr)
		{
			gcn::Rectangle scroll;
			scroll.y = (itemImage ? std::max<int>(getFont()->getHeight(), itemImage->getHeight()) : getFont()->getHeight()) * mSelected;
			scroll.height = (itemImage ? std::max<int>(getFont()->getHeight(), itemImage->getHeight()) : getFont()->getHeight());
			scrollToRectangle(*scrollArea, scroll);
		}
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
**  ImageListBoxWidget constructor.
**
**  @todo  Size should be parametrable, maybe remove default constructor?
*/
ImageListBoxWidget::ImageListBoxWidget(unsigned int width, unsigned int height) : ListBoxWidget(width, height)
{
	setDimension(gcn::Rectangle(0, 0, width, height));
	setContent(&listbox);
}


/**
**  Set the list
*/
void ListBoxWidget::setList(lua_State *lua, lua_Object *lo)
{
	lualistmodel.setList(lua, lo);
	listbox.setListModel(&lualistmodel);
	adjustSize();
}

/**
**  Set the list
*/
void ImageListBoxWidget::setList(lua_State *lua, lua_Object *lo)
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
**  Sets the ListModel index of the selected element.
**
**  @param selected  The ListModel index of the selected element.
**
**  @see gcn::ListBox
*/
void ImageListBoxWidget::setSelected(int selected)
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
	return const_cast<gcn::ListBox &>(listbox).getSelected();
}

/**
**  Gets the ListModel index of the selected element.
**
**  @return  The ListModel index of the selected element.
**
**  @see gcn::ListBox
*/
int ImageListBoxWidget::getSelected() const
{
	return const_cast<ImageListBox &>(listbox).getSelected();
}

/**
**  Set background color of the ListBoxWidget.
**
**  @param color  Color to set.
*/
void ListBoxWidget::setBackgroundColor(const gcn::Color &color)
{
	ScrollArea::setBackgroundColor(color);
	ScrollArea::setBaseColor(color);
	listbox.setBackgroundColor(color);
}

/**
**  Set background color of the ListBoxWidget.
**
**  @param color  Color to set.
*/
void ImageListBoxWidget::setBackgroundColor(const gcn::Color &color)
{
	ScrollArea::setBackgroundColor(color);
	ScrollArea::setBaseColor(color);
	listbox.setBackgroundColor(color);
}

/**
**  Set font of the ListBox.
**
**  @param font  Font to set.
*/
void ListBoxWidget::fontChanged() /* override */
{
	listbox.setFont(getFont());
	listbox.setWidth(getWidth());
	adjustSize();
}

/**
**  Set font of the ListBox.
**
**  @param font  Font to set.
*/
void ImageListBoxWidget::fontChanged() /* override */
{
	listbox.setFont(getFont());
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
**  Adjust size of the listBox.
**
**  @todo Fix width of the scroll area (depend of v-scroll or not).
*/
void ImageListBoxWidget::adjustSize()
{
	int width = listbox.getWidth();
	Assert(listbox.getListModel());
	gcn::ListModel *listmodel = listbox.getListModel();

	for (int i = 0; i < listmodel->getNumberOfElements(); ++i) {
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

/**
**  Add an action listener
*/
void ImageListBoxWidget::addActionListener(gcn::ActionListener *actionListener)
{
	listbox.addActionListener(actionListener);
}

/**
**  Draw the list box
**
**  @param  graphics Graphics to use
*/
void ImageListBoxWidget::draw(gcn::Graphics *graphics) /* override */
{
	CGraphic *img = nullptr;

	// Check if we have all required graphics
	if (!this->upButtonImage || !this->downButtonImage || !this->leftButtonImage || !this->rightButtonImage
		|| !this->upPressedButtonImage || !this->downPressedButtonImage || !this->leftPressedButtonImage || !this->rightPressedButtonImage
		|| !this->markerImage || !this->hBarButtonImage || !this->vBarButtonImage) {
		ErrorPrint("Not all graphics for ImageListBoxWidget were set\n");
		ExitFatal(1);
	}
	gcn::Rectangle rect = getChildrenArea();
	img = itemImage;
	img->Resize(rect.width, img->getHeight());
	int y = 0;
	while (y + img->getHeight() <= rect.height) {
		graphics->drawImage(img, 0, 0, 0, y, getWidth(), img->getHeight());
		y += img->getHeight();
	}
	img->SetOriginalSize();

	if (mVBarVisible)
	{
		if (mUpButtonPressed) {
			this->drawUpPressedButton(graphics);
		} else {
			this->drawUpButton(graphics);
		}
		if (mDownButtonPressed) {
			this->drawDownPressedButton(graphics);
		} else {
			this->drawDownButton(graphics);
		}
		this->drawVBar(graphics);
		this->drawVMarker(graphics);
	}
	if (mHBarVisible)
	{
		if (mLeftButtonPressed) {
			this->drawLeftPressedButton(graphics);
		} else {
			this->drawLeftButton(graphics);
		}
		if (mRightButtonPressed) {
			this->drawRightPressedButton(graphics);
		} else {
			this->drawRightButton(graphics);
		}
		this->drawHBar(graphics);
		this->drawHMarker(graphics);
	}
	if (getContent()) {
		gcn::Rectangle contdim = getChildrenArea();
		graphics->pushClipArea(getContent()->getDimension());

		if (getContent()->getFrameSize() > 0)
		{
			gcn::Rectangle rec = getContent()->getDimension();
			rec.x -= getContent()->getFrameSize();
			rec.y -= getContent()->getFrameSize();
			rec.width += 2 * getContent()->getFrameSize();
			rec.height += 2 * getContent()->getFrameSize();
			graphics->pushClipArea(rec);
			getContent()->drawFrame(graphics);
			graphics->popClipArea();
		}

		graphics->pushClipArea(contdim);
		getContent()->draw(graphics);
		graphics->popClipArea();
		graphics->popClipArea();
	}
}

/**
**  Draw the list box border
**
**  @param  graphics Graphics to use
*/
void ImageListBoxWidget::drawFrame(gcn::Graphics *graphics) /* override */
{
	gcn::Color faceColor = getBaseColor();
	gcn::Color highlightColor, shadowColor;
	int alpha = getBaseColor().a;
	int width = getWidth() + getFrameSize() * 2 - 1;
	int height = getHeight() + getFrameSize() * 2 - 1;
	highlightColor = faceColor + 0x303030;
	highlightColor.a = alpha;
	shadowColor = faceColor - 0x303030;
	shadowColor.a = alpha;

	unsigned int i;
	for (i = 0; i < getFrameSize(); ++i)
	{
		graphics->setColor(shadowColor);
		graphics->drawLine(i,i, width - i, i);
		graphics->drawLine(i,i + 1, i, height - i - 1);
		graphics->setColor(highlightColor);
		graphics->drawLine(width - i,i + 1, width - i, height - i);
		graphics->drawLine(i,height - i, width - i - 1, height - i);
	}
}

void ImageListBoxWidget::drawUpButton(gcn::Graphics* graphics)
{
	gcn::Rectangle dim = getUpButtonDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = upButtonImage;
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());
	graphics->popClipArea();
}

void ImageListBoxWidget::drawDownButton(gcn::Graphics* graphics)
{
	gcn::Rectangle dim = getDownButtonDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = downButtonImage;
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());
	graphics->popClipArea();
}

void ImageListBoxWidget::drawLeftButton(gcn::Graphics* graphics)
{
	gcn::Rectangle dim = getLeftButtonDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = leftButtonImage;
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());
	graphics->popClipArea();
}

void ImageListBoxWidget::drawRightButton(gcn::Graphics* graphics)
{
	gcn::Rectangle dim = getRightButtonDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = rightButtonImage;
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());
	graphics->popClipArea();
}

void ImageListBoxWidget::drawUpPressedButton(gcn::Graphics* graphics)
{
	gcn::Rectangle dim = getUpButtonDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = upPressedButtonImage;
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());
	graphics->popClipArea();
}

void ImageListBoxWidget::drawDownPressedButton(gcn::Graphics* graphics)
{
	gcn::Rectangle dim = getDownButtonDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = downPressedButtonImage;
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());
	graphics->popClipArea();
}

void ImageListBoxWidget::drawLeftPressedButton(gcn::Graphics* graphics)
{
	gcn::Rectangle dim = getLeftButtonDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = leftPressedButtonImage;
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());
	graphics->popClipArea();
}

void ImageListBoxWidget::drawRightPressedButton(gcn::Graphics* graphics)
{
	gcn::Rectangle dim = getRightButtonDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = rightPressedButtonImage;
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());
	graphics->popClipArea();
}

void ImageListBoxWidget::drawHBar(gcn::Graphics *graphics)
{
	gcn::Rectangle dim = getHorizontalBarDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = hBarButtonImage;
	img->Resize(dim.width, dim.height);
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());
	img->SetOriginalSize();

	graphics->popClipArea();
}

void ImageListBoxWidget::drawVBar(gcn::Graphics *graphics)
{
	gcn::Rectangle dim = getVerticalBarDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = vBarButtonImage;
	img->Resize(dim.width, dim.height);
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());
	img->SetOriginalSize();

	graphics->popClipArea();
}

void ImageListBoxWidget::drawHMarker(gcn::Graphics *graphics)
{
	gcn::Rectangle dim = getHorizontalMarkerDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = markerImage;
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());

	graphics->popClipArea();
}

void ImageListBoxWidget::drawVMarker(gcn::Graphics *graphics)
{
	gcn::Rectangle dim = getVerticalMarkerDimension();
	graphics->pushClipArea(dim);

	CGraphic *img = nullptr;

	img = markerImage;
	graphics->drawImage(img, 0, 0, 0, 0, img->getWidth(), img->getHeight());

	graphics->popClipArea();
}

gcn::Rectangle ImageListBoxWidget::getVerticalMarkerDimension()
{
	if (!mVBarVisible)
	{
		return gcn::Rectangle(0, 0, 0, 0);
	}

	int length, pos;
	gcn::Rectangle barDim = getVerticalBarDimension();

	if (getContent() && getContent()->getHeight() != 0)
	{
		length = this->markerImage->getHeight();
	}
	else
	{
		length = barDim.height;
	}

	if (length < mScrollbarWidth)
	{
		length = mScrollbarWidth;
	}

	if (length > barDim.height)
	{
		length = barDim.height;
	}

	if (getVerticalMaxScroll() != 0)
	{
		pos = ((barDim.height - length) * getVerticalScrollAmount())
			/ getVerticalMaxScroll();
	}
	else
	{
		pos = 0;
	}

	return gcn::Rectangle(barDim.x, barDim.y + pos, mScrollbarWidth, length);
}

gcn::Rectangle ImageListBoxWidget::getHorizontalMarkerDimension()
{
	if (!mHBarVisible)
	{
		return gcn::Rectangle(0, 0, 0, 0);
	}

	int length, pos;
	gcn::Rectangle barDim = getHorizontalBarDimension();

	if (getContent() && getContent()->getWidth() != 0)
	{
		length = this->markerImage->getHeight();
	}
	else
	{
		length = barDim.width;
	}

	if (length < mScrollbarWidth)
	{
		length = mScrollbarWidth;
	}

	if (length > barDim.width)
	{
		length = barDim.width;
	}

	if (getHorizontalMaxScroll() != 0)
	{
		pos = ((barDim.width - length) * getHorizontalScrollAmount())
			/ getHorizontalMaxScroll();
	}
	else
	{
		pos = 0;
	}

	return gcn::Rectangle(barDim.x + pos, barDim.y, length, mScrollbarWidth);
}


/*----------------------------------------------------------------------------
--  DropDownWidget
----------------------------------------------------------------------------*/


/**
**  Set the list
*/
void DropDownWidget::setList(lua_State *lua, lua_Object *lo)
{
	mListModel->setList(lua, lo);
	setListModel(mListModel.get());
}

/**
**  Set the drop down size
*/
void DropDownWidget::setSize(int width, int height)
{
	DropDown::setSize(width, height);
	this->mListBox->setSize(width, height);
}

/*----------------------------------------------------------------------------
--  ImageDropDownWidget
----------------------------------------------------------------------------*/

void ImageDropDownWidget::setList(lua_State *lua, lua_Object *lo)
{
	mListModel->setList(lua, lo);
	setListModel(mListModel.get());
}

/**
**  Set the drop down size
*/
void ImageDropDownWidget::setSize(int width, int height)
{
	DropDown::setSize(width, height);
	this->mListBox->setSize(width, height);
}

void ImageDropDownWidget::draw(gcn::Graphics *graphics) /* override */
{
	Assert(mScrollArea && mScrollArea->getContent() != nullptr);
	const int h = mDroppedDown ? mFoldedUpHeight : getHeight();

	CGraphic *img = this->itemImage;
	if (!this->itemImage || !this->DownNormalImage || !this->DownPressedImage) {
		ErrorPrint("Not all graphics for ImageDropDownWidget were set\n");
		ExitFatal(1);
	}

	int alpha = getBaseColor().a;
	gcn::Color faceColor = getBaseColor();
	faceColor.a = alpha;
	gcn::Color highlightColor = faceColor + 0x303030;
	highlightColor.a = alpha;
	gcn::Color shadowColor = faceColor - 0x303030;
	shadowColor.a = alpha;

	img->Resize(getWidth(), h);
	graphics->drawImage(img, 0, 0, 0, 0, getWidth(), h);
	img->SetOriginalSize();

	graphics->setFont(getFont());

	if (mListBox->getListModel() && mListBox->getSelected() >= 0)
	{
		graphics->drawText(mListBox-> getListModel()->getElementAt(mListBox->getSelected()),
		                   1,
		                   (h - getFont()->getHeight()) / 2);
	}

	if (isFocused()) {
		graphics->drawRectangle(gcn::Rectangle(0, 0, getWidth() - h, h));
	}

	drawButton(graphics);

	if (mDroppedDown)
	{
		// Draw two lines separating the ListBox with the selected
		// element view.
		graphics->setColor(highlightColor);
		graphics->drawLine(0, h, getWidth(), h);
		graphics->setColor(shadowColor);
		graphics->drawLine(0, h + 1,getWidth(),h + 1);
	}
}

void ImageDropDownWidget::drawFrame(gcn::Graphics *graphics) /* override */
{
	gcn::Color faceColor = getBaseColor();
	gcn::Color highlightColor, shadowColor;
	int alpha = getBaseColor().a;
	int width = getWidth() + getFrameSize() * 2 - 1;
	int height = getHeight() + getFrameSize() * 2 - 1;
	highlightColor = faceColor + 0x303030;
	highlightColor.a = alpha;
	shadowColor = faceColor - 0x303030;
	shadowColor.a = alpha;

	for (unsigned int i = 0; i < getFrameSize(); ++i)
	{
		graphics->setColor(shadowColor);
		graphics->drawLine(i,i, width - i, i);
		graphics->drawLine(i,i + 1, i, height - i - 1);
		graphics->setColor(highlightColor);
		graphics->drawLine(width - i,i + 1, width - i, height - i);
		graphics->drawLine(i,height - i, width - i - 1, height - i);
	}
}

void ImageDropDownWidget::drawButton(gcn::Graphics *graphics)
{
	const int h = mDroppedDown ? mFoldedUpHeight : getHeight();
	const int x = getWidth() - h;
	const int y = 0;
	CGraphic *img = mDroppedDown ? this->DownPressedImage : this->DownNormalImage;

	img->Resize(h, h);
	graphics->drawImage(img, 0, 0, x, y, h, h);
	img->SetOriginalSize();
}

std::string ImageDropDownWidget::getSelectedItem()
{
	Assert(mScrollArea && mScrollArea->getContent() != nullptr);

	return mListModel->getElementAt(mListBox->getSelected());
}

int ImageDropDownWidget::setSelectedItem(lua_State *lua, lua_Object *lo)
{
	Assert(mScrollArea && mScrollArea->getContent() != nullptr);

	auto item = LuaToString(lua, *lo);
	int idx = this->mListModel->getIdxOfElement(item);
	if (idx >= 0)
	{
		this->setSelected(idx);
	}
	return idx;
}

void ImageDropDownWidget::adjustHeight()
{
	Assert(mScrollArea && mScrollArea->getContent() != nullptr);

	int listBoxHeight = mListBox->getHeight();
	int h2 = mFoldedUpHeight ? mFoldedUpHeight : getFont()->getHeight();

	setHeight(h2);

	// The addition/subtraction of 2 compensates for the seperation lines
	// seperating the selected element view and the scroll area.

	if (mDroppedDown && getParent())
	{
		int h = getParent()->getHeight() - getY();

		if (listBoxHeight > h - h2 - 2)
		{
			mScrollArea->setHeight(h - h2 - 2);
			setHeight(h);
		}
		else
		{
			setHeight(listBoxHeight + h2 + 2);
			mScrollArea->setHeight(listBoxHeight);
		}
	}

	mScrollArea->setWidth(getWidth());
	mScrollArea->setPosition(0, h2 + 2);
}

void ImageDropDownWidget::setFont(gcn::Font *font)
{
	gcn::Widget::setFont(font);
	mListBox->setFont(font);
}

/*----------------------------------------------------------------------------
--  StatBoxWidget
----------------------------------------------------------------------------*/

/**
**  StatBoxWidget constructor
**
**  @param width   Width of the StatBoxWidget.
**  @param height  Height of the StatBoxWidget.
*/
StatBoxWidget::StatBoxWidget(int width, int height) : percent(100)
{
	setWidth(width);
	setHeight(height);

	setBackgroundColor(gcn::Color(0, 0, 0));
	setBaseColor(gcn::Color(255, 255, 255));
	setForegroundColor(gcn::Color(128, 128, 128));
}

/**
**  Draw StatBoxWidget.
**
**  @param graphics  Graphic driver used to draw.
**
**  @todo caption seem to be placed upper than the middle.
**  @todo set direction (hor./vert.) and growing direction(up/down, left/rigth).
*/
void StatBoxWidget::draw(gcn::Graphics *graphics) /* override */
{
	int width;
	int height;

	width = getWidth();
	height = getHeight();

	graphics->setColor(getBackgroundColor());
	graphics->fillRectangle(gcn::Rectangle(0, 0, width, height));

	graphics->setColor(getBaseColor());
	graphics->drawRectangle(gcn::Rectangle(1, 1, width - 2, height - 2));

	graphics->setColor(getForegroundColor());
	width = percent * width / 100;
	graphics->fillRectangle(gcn::Rectangle(2, 2, width - 4, height - 4));
	graphics->setFont(getFont());
	graphics->drawText(getCaption(),
					   (getWidth() - getFont()->getWidth(getCaption())) / 2,
					   (height - getFont()->getHeight()) / 2);
}

/**
**  Set caption of StatBoxWidget.
**
**  @param caption  New value.
*/
void StatBoxWidget::setCaption(const std::string &caption)
{
	this->caption = caption;
}

/**
**  Get caption of StatBoxWidget.
*/

const std::string &StatBoxWidget::getCaption() const
{
	return caption;
}

/**
**  Set percent of StatBoxWidget.
**
**  @param percent  New value.
*/
void StatBoxWidget::setPercent(const int percent)
{
	this->percent = percent;
}

/**
**  Get percent of StatBoxWidget.
*/
int StatBoxWidget::getPercent() const
{
	return percent;
}


/*----------------------------------------------------------------------------
--  MenuScreen
----------------------------------------------------------------------------*/


/**
**  MenuScreen constructor
*/
MenuScreen::MenuScreen() : Container(), runLoop(true)
{
	setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
	setOpaque(false);

	// The gui must be set immediately as it is used by widgets
	// when they are added to the container
	oldtop = Gui->getTop();
	Gui->setTop(this);
}

/**
**  Run the menu.  Loops until stop is called.
*/
int MenuScreen::run(bool loop)
{
	this->loopResult = 0;
	this->runLoop = loop;

	CursorState = CursorStates::Point;
	GameCursor = UI.Point.Cursor;
	CursorOn = ECursorOn::Unknown;

	if (loop) {
		const EventCallback *old_callbacks = GetCallbacks();
		SetCallbacks(&GuichanCallbacks);
		while (runLoop) {
			UpdateDisplay();
			RealizeVideoMemory();
			WaitEventsOneFrame();
		}
		SetCallbacks(old_callbacks);
		Gui->setTop(this->oldtop);
	} else {
		SetCallbacks(&GuichanCallbacks);
		MenuStack.push(this);
	}

	return this->loopResult;
}

/**
**  Stop the menu from running
*/
void MenuScreen::stop(int result, bool stopAll)
{
	if (!this->runLoop) {
		Gui->setTop(this->oldtop);
		if (MenuStack.size() > 0 && MenuStack.top() == this) {
			MenuStack.pop();
		} else {
			DebugPrint("ALARM! A menu we're trying to stop is NOT at the top of the menu stack!!");
		}
		if (stopAll) {
			while (!MenuStack.empty()) {
				MenuStack.pop();
			}
		}
		if (MenuStack.empty()) {
			//InterfaceState = IfaceState::Normal;
			if (!Editor.Running) {
				SetCallbacks(&GameCallbacks);
			} else {
				SetCallbacks(&EditorCallbacks);
			}
			GamePaused = false;
			UI.StatusLine.Clear();
			if (GameRunning) {
				UIHandleMouseMove(CursorScreenPos);
			}
		}
	}

	this->runLoop = false;
	this->loopResult = result;
}

void MenuScreen::addLogicCallback(LuaActionListener *listener)
{
	logiclistener = listener;
}

void MenuScreen::draw(gcn::Graphics *graphics) /* override */
{
	if (this->drawUnder) {
		gcn::Rectangle r = graphics->getCurrentClipArea();
		graphics->popClipArea();

		oldtop->_draw(graphics);

		graphics->pushClipArea(r);
	}
	gcn::Container::draw(graphics);
}

void MenuScreen::logic() /* override */
{
	if (NetConnectRunning == 2) {
		NetworkProcessClientRequest();
	}
	if (NetConnectRunning == 1) {
		NetworkProcessServerRequest();
	}
	if (logiclistener) {
		logiclistener->action(gcn::ActionEvent{this, ""});
	}
	Container::logic();
}

//@}
