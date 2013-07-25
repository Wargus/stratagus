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

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

// Guichan stuff we need
gcn::Gui *Gui;         /// A Gui object - binds it all together
gcn::SDLInput *Input;  /// Input driver

static EventCallback GuichanCallbacks;

static std::stack<MenuScreen *> MenuStack;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


static void MenuHandleButtonDown(unsigned)
{
}
static void MenuHandleButtonUp(unsigned)
{
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
	Input->processKeyRepeat();
	HandleKeyModifiersDown(key, keychar);
}


/**
**  Initializes the GUI stuff
*/
void initGuichan()
{
	gcn::Graphics *graphics;

#if defined(USE_OPENGL) || defined(USE_GLES)
	if (UseOpenGL) {
		graphics = new MyOpenGLGraphics();
	} else
#endif
	{
		graphics = new gcn::SDLGraphics();

		// Set the target for the graphics object to be the screen.
		// In other words, we will draw to the screen.
		// Note, any surface will do, it doesn't have to be the screen.
		((gcn::SDLGraphics *)graphics)->setTarget(TheScreen);
	}

	Input = new gcn::SDLInput();

	Gui = new gcn::Gui();
	Gui->setGraphics(graphics);
	Gui->setInput(Input);
	Gui->setTop(NULL);

#if defined(USE_OPENGL) || defined(USE_GLES)
	Gui->setUseDirtyDrawing(!UseOpenGL);
#else
	Gui->setUseDirtyDrawing(1);
#endif

	GuichanCallbacks.ButtonPressed = &MenuHandleButtonDown;
	GuichanCallbacks.ButtonReleased = &MenuHandleButtonUp;
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
		delete Gui;
		Gui = NULL;
	}

	delete Input;
	Input = NULL;
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
#if defined(USE_OPENGL) || defined(USE_GLES)
		Gui->setUseDirtyDrawing(!UseOpenGL && !GameRunning && !Editor.Running);
#else
		Gui->setUseDirtyDrawing(!GameRunning && !Editor.Running);
#endif
		Gui->draw();
	}
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
void LuaActionListener::action(const std::string &eventId)
{
	callback.pushPreamble();
	callback.pushString(eventId.c_str());
	callback.run();
}

/**
**  LuaActionListener destructor
*/
LuaActionListener::~LuaActionListener()
{
}

#if defined(USE_OPENGL) || defined(USE_GLES)

/*----------------------------------------------------------------------------
--  MyOpenGLGraphics
----------------------------------------------------------------------------*/

void MyOpenGLGraphics::_beginDraw()
{
	gcn::Rectangle area(0, 0, Video.Width, Video.Height);
	pushClipArea(area);
}

void MyOpenGLGraphics::_endDraw()
{
	popClipArea();
}

void MyOpenGLGraphics::drawImage(const gcn::Image *image, int srcX, int srcY,
								 int dstX, int dstY, int width, int height)
{
	const gcn::ClipRectangle &r = this->getCurrentClipArea();
	int right = std::min<int>(r.x + r.width - 1, Video.Width - 1);
	int bottom = std::min<int>(r.y + r.height - 1, Video.Height - 1);

	if (r.x > right || r.y > bottom) {
		return;
	}

	PushClipping();
	SetClipping(r.x, r.y, right, bottom);
	((CGraphic *)image)->DrawSubClip(srcX, srcY, width, height,
									 dstX + mClipStack.top().xOffset, dstY + mClipStack.top().yOffset);
	PopClipping();
}

void MyOpenGLGraphics::drawPoint(int x, int y)
{
	gcn::Color c = this->getColor();
	Video.DrawPixelClip(Video.MapRGBA(0, c.r, c.g, c.b, c.a),
						x + mClipStack.top().xOffset, y + mClipStack.top().yOffset);
}

void MyOpenGLGraphics::drawLine(int x1, int y1, int x2, int y2)
{
	gcn::Color c = this->getColor();
	const PixelPos pos1(x1 + mClipStack.top().xOffset, y1 + mClipStack.top().yOffset);
	const PixelPos pos2(x2 + mClipStack.top().xOffset, y2 + mClipStack.top().yOffset);

	Video.DrawLineClip(Video.MapRGBA(0, c.r, c.g, c.b, c.a), pos1, pos2);
}

void MyOpenGLGraphics::drawRectangle(const gcn::Rectangle &rectangle)
{
	gcn::Color c = this->getColor();
	if (c.a == 0) {
		return;
	}

	const gcn::ClipRectangle top = mClipStack.top();
	gcn::Rectangle area = gcn::Rectangle(rectangle.x + top.xOffset,
										 rectangle.y + top.yOffset,
										 rectangle.width, rectangle.height);

	if (!area.intersect(top)) {
		return;
	}

	int x1 = std::max<int>(area.x, top.x);
	int y1 = std::max<int>(area.y, top.y);
	int x2 = std::min<int>(area.x + area.width, top.x + top.width);
	int y2 = std::min<int>(area.y + area.height, top.y + top.height);

	Video.DrawTransRectangle(Video.MapRGB(0, c.r, c.g, c.b),
							 x1, y1, x2 - x1, y2 - y1, mColor.a);
}

void MyOpenGLGraphics::fillRectangle(const gcn::Rectangle &rectangle)
{
	const gcn::Color c = this->getColor();

	if (c.a == 0) {
		return;
	}

	const gcn::ClipRectangle top = mClipStack.top();
	gcn::Rectangle area = gcn::Rectangle(rectangle.x + top.xOffset,
										 rectangle.y + top.yOffset,
										 rectangle.width, rectangle.height);

	if (!area.intersect(top)) {
		return;
	}

	int x1 = std::max<int>(area.x, top.x);
	int y1 = std::max<int>(area.y, top.y);
	int x2 = std::min<int>(area.x + area.width, top.x + top.width);
	int y2 = std::min<int>(area.y + area.height, top.y + top.height);

	Video.FillTransRectangle(Video.MapRGB(0, c.r, c.g, c.b),
							 x1, y1, x2 - x1, y2 - y1, c.a);
}

#endif

/*----------------------------------------------------------------------------
--  ImageButton
----------------------------------------------------------------------------*/


/**
**  ImageButton constructor
*/
ImageButton::ImageButton() :
	Button(), normalImage(NULL), pressedImage(NULL),
	disabledImage(NULL)
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
	disabledImage(NULL)
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
			textX = 0;
			Assert(!"Unknown alignment.");
			//throw GCN_EXCEPTION("Unknown alignment.");
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

/*----------------------------------------------------------------------------
--  ImageRadioButton
----------------------------------------------------------------------------*/


/**
**  ImageRadioButton constructor
*/
ImageRadioButton::ImageRadioButton() : gcn::RadioButton(),
	uncheckedNormalImage(NULL), uncheckedPressedImage(NULL),
	checkedNormalImage(NULL), checkedPressedImage(NULL),
	mMouseDown(false)
{
}

/**
**  ImageRadioButton constructor
*/
ImageRadioButton::ImageRadioButton(const std::string &caption,
								   const std::string &group, bool marked) :
	gcn::RadioButton(caption, group, marked),
	uncheckedNormalImage(NULL), uncheckedPressedImage(NULL),
	checkedNormalImage(NULL), checkedPressedImage(NULL),
	mMouseDown(false)
{
}

/**
**  Draw the image radio button (not the caption)
*/
void ImageRadioButton::drawBox(gcn::Graphics *graphics)
{
	gcn::Image *img = NULL;

	if (isMarked()) {
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
		RadioButton::drawBox(graphics);
	}
}

/**
**  Draw the image radio button
*/
void ImageRadioButton::draw(gcn::Graphics *graphics)
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
**  Mouse button pressed callback
*/
void ImageRadioButton::mousePress(int, int, int button)
{
	if (button == gcn::MouseInput::LEFT && hasMouse()) {
		mMouseDown = true;
	}
}

/**
**  Mouse button released callback
*/
void ImageRadioButton::mouseRelease(int, int, int button)
{
	if (button == gcn::MouseInput::LEFT) {
		mMouseDown = false;
	}
}

/**
**  Mouse clicked callback
*/
void ImageRadioButton::mouseClick(int, int, int button, int)
{
	if (button == gcn::MouseInput::LEFT) {
		setMarked(true);
		generateAction();
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
void ImageCheckBox::mousePress(int, int, int button)
{
	if (button == gcn::MouseInput::LEFT && hasMouse()) {
		mMouseDown = true;
	}
}

/**
**  Mouse button released callback
*/
void ImageCheckBox::mouseRelease(int, int, int button)
{
	if (button == gcn::MouseInput::LEFT) {
		mMouseDown = false;
	}
}

/**
**  Mouse clicked callback
*/
void ImageCheckBox::mouseClick(int, int, int button, int)
{
	if (button == gcn::MouseInput::LEFT) {
		toggle();
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
--  MultiLineLabel
----------------------------------------------------------------------------*/


/**
**  MultiLineLabel constructor
*/
MultiLineLabel::MultiLineLabel()
{
	this->mAlignment = LEFT;
	this->mVerticalAlignment = TOP;
	this->mLineWidth = 0;
}

/**
**  MultiLineLabel constructor
*/
MultiLineLabel::MultiLineLabel(const std::string &caption)
{
	this->mCaption = caption;
	this->mAlignment = LEFT;
	this->mVerticalAlignment = TOP;

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
	this->setDirty(true);
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
void MultiLineLabel::setAlignment(unsigned int alignment)
{
	this->mAlignment = alignment;
}

/**
**  Get the horizontal alignment
*/
unsigned int MultiLineLabel::getAlignment()
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
unsigned int MultiLineLabel::getVerticalAlignment()
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
int MultiLineLabel::getLineWidth()
{
	return this->mLineWidth;
}

/**
**  Adjust the size
*/
void MultiLineLabel::adjustSize()
{
	int width = 0;
	for (int i = 0; i < (int)this->mTextRows.size(); ++i) {
		int w = this->getFont()->getWidth(this->mTextRows[i]);
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
		case LEFT:
			textX = 0;
			break;
		case CENTER:
			textX = this->getWidth() / 2;
			break;
		case RIGHT:
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
		graphics->drawText(this->mTextRows[i], textX, textY + i * this->getFont()->getHeight(),
						   this->getAlignment());
	}
}

/**
**  Draw the border
*/
void MultiLineLabel::drawBorder(gcn::Graphics *graphics)
{
	gcn::Color faceColor = getBaseColor();
	gcn::Color highlightColor, shadowColor;
	int alpha = getBaseColor().a;
	int width = getWidth() + getBorderSize() * 2 - 1;
	int height = getHeight() + getBorderSize() * 2 - 1;
	highlightColor = faceColor + 0x303030;
	highlightColor.a = alpha;
	shadowColor = faceColor - 0x303030;
	shadowColor.a = alpha;

	for (unsigned int i = 0; i < getBorderSize(); ++i) {
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
	gcn::ScrollArea(NULL, gcn::ScrollArea::SHOW_NEVER, gcn::ScrollArea::SHOW_NEVER),
	speedY(1.f), containerY(0.f), finished(false)
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
	setDirty(true);
	if (container.getHeight() + containerY - speedY > 0) {
		// the bottom of the container is lower than the top
		// of the widget. It is thus still visible.
		containerY -= speedY;
		container.setY((int)containerY);
	} else if (!finished) {
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
**    if you go virtually outside the container then go back,
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
	// Useful only when window reachs the limit.
	getAbsolutePosition(absx, absy);
	CursorScreenPos.x = absx + mMouseXOffset;
	CursorScreenPos.y = absy + mMouseYOffset;
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
**  Set the list
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
	return const_cast<gcn::ListBox &>(listbox).getSelected();
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
**  Set the list
*/
void DropDownWidget::setList(lua_State *lua, lua_Object *lo)
{
	listmodel.setList(lua, lo);
	setListModel(&listmodel);
}

/**
**  Set the drop down size
*/
void DropDownWidget::setSize(int width, int height)
{
	DropDown::setSize(width, height);
	this->getListBox()->setSize(width, height);
}

/*----------------------------------------------------------------------------
--  DropDownWidget
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
void StatBoxWidget::draw(gcn::Graphics *graphics)
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
	this->setDirty(true);
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
	this->setDirty(true);
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
MenuScreen::MenuScreen() :
	Container(), runLoop(true), logiclistener(0), drawUnder(false)
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

	CursorState = CursorStatePoint;
	GameCursor = UI.Point.Cursor;
	CursorOn = CursorOnUnknown;

	CallbackMusicOn();

	if (loop) {
		const EventCallback *old_callbacks = GetCallbacks();
		SetCallbacks(&GuichanCallbacks);
		while (runLoop) {
			UpdateDisplay();
			RealizeVideoMemory();
			CheckMusicFinished();
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
		Assert(MenuStack.top() == this);
		MenuStack.pop();
		if (stopAll) {
			while (!MenuStack.empty()) {
				MenuStack.pop();
			}
		}
		if (MenuStack.empty()) {
			//InterfaceState = IfaceStateNormal;
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

void MenuScreen::draw(gcn::Graphics *graphics)
{
	if (this->drawUnder) {
		gcn::Rectangle r = Gui->getGraphics()->getCurrentClipArea();
		Gui->getGraphics()->popClipArea();
		Gui->draw(oldtop);
		Gui->getGraphics()->pushClipArea(r);
	}
	gcn::Container::draw(graphics);
}

void MenuScreen::logic()
{
	if (NetConnectRunning == 2) {
		NetworkProcessClientRequest();
	}
	if (NetConnectRunning == 1) {
		NetworkProcessServerRequest();
	}
	if (logiclistener) {
		logiclistener->action("");
	}
	Container::logic();
}

//@}
