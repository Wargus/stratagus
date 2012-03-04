/*      _______   __   __   __   ______   __   __   _______   __   __
 *     / _____/\ / /\ / /\ / /\ / ____/\ / /\ / /\ / ___  /\ /  |\/ /\
 *    / /\____\// / // / // / // /\___\// /_// / // /\_/ / // , |/ / /
 *   / / /__   / / // / // / // / /    / ___  / // ___  / // /| ' / /
 *  / /_// /\ / /_// / // / // /_/_   / / // / // /\_/ / // / |  / /
 * /______/ //______/ //_/ //_____/\ /_/ //_/ //_/ //_/ //_/ /|_/ /
 * \______\/ \______\/ \_\/ \_____\/ \_\/ \_\/ \_\/ \_\/ \_\/ \_\/
 *
 * Copyright (c) 2004, 2005 darkbits                        Js_./
 * Per Larsson a.k.a finalman                          _RqZ{a<^_aa
 * Olof Naessén a.k.a jansem/yakslem                _asww7!uY`>  )\a//
 *                                                 _Qhm`] _f "'c  1!5m
 * Visit: http://guichan.darkbits.org             )Qk<P ` _: :+' .'  "{[
 *                                               .)j(] .d_/ '-(  P .   S
 * License: (BSD)                                <Td/Z <fP"5(\"??"\a.  .L
 * Redistribution and use in source and          _dV>ws?a-?'      ._/L  #'
 * binary forms, with or without                 )4d[#7r, .   '     )d`)[
 * modification, are permitted provided         _Q-5'5W..j/?'   -?!\)cam'
 * that the following conditions are met:       j<<WP+k/);.        _W=j f
 * 1. Redistributions of source code must       .$%w\/]Q  . ."'  .  mj$
 *    retain the above copyright notice,        ]E.pYY(Q]>.   a     J@\
 *    this list of conditions and the           j(]1u<sE"L,. .   ./^ ]{a
 *    following disclaimer.                     4'_uomm\.  )L);-4     (3=
 * 2. Redistributions in binary form must        )_]X{Z('a_"a7'<a"a,  ]"[
 *    reproduce the above copyright notice,       #}<]m7`Za??4,P-"'7. ).m
 *    this list of conditions and the            ]d2e)Q(<Q(  ?94   b-  LQ/
 *    following disclaimer in the                <B!</]C)d_, '(<' .f. =C+m
 *    documentation and/or other materials      .Z!=J ]e []('-4f _ ) -.)m]'
 *    provided with the distribution.          .w[5]' _[ /.)_-"+?   _/ <W"
 * 3. Neither the name of Guichan nor the      :$we` _! + _/ .        j?
 *    names of its contributors may be used     =3)= _f  (_yQmWW$#(    "
 *    to endorse or promote products derived     -   W,  sQQQQmZQ#Wwa]..
 *    from this software without specific        (js, \[QQW$QWW#?!V"".
 *    prior written permission.                    ]y:.<\..          .
 *                                                 -]n w/ '         [.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT       )/ )/           !
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY         <  (; sac    ,    '
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING,               ]^ .-  %
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF            c <   r
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR            aga<  <La
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE          5%  )P'-3L
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR        _bQf` y`..)a
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          ,J?4P'.P"_(\?d'.,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES               _Pa,)!f/<[]/  ?"
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT      _2-..:. .r+_,.. .
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     ?a.<%"'  " -'.a_ _,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION)                     ^
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * For comments regarding functions please see the header file.
 */
#include <assert.h>
#include "guichan/widgets/window.h"
#include "guichan/exception.h"
#include "guichan/mouseinput.h"

namespace gcn
{
    Window::Window()
    {
        mContent = NULL;
        mMouseDrag = false;
        setBorderSize(1);
        setPadding(2);
        setTitleBarHeight(16);
        setAlignment(Graphics::CENTER);
        addMouseListener(this);
        setMovable(true);
        setOpaque(true);
    }

    Window::Window(const std::string& caption)
    {
        mContent = NULL;
        mMouseDrag = false;
        setCaption(caption);
        setBorderSize(1);
        setPadding(2);
        setTitleBarHeight(16);
        setAlignment(Graphics::CENTER);
        addMouseListener(this);
        setMovable(true);
        setOpaque(true);
    }

    Window::Window(Widget* content, const std::string& caption)
    {
        mContent = NULL;
        mMouseDrag = false;
        setContent(content);
        setCaption(caption);
        setBorderSize(1);
        setPadding(2);
        setTitleBarHeight(16);
        setAlignment(Graphics::CENTER);
        addMouseListener(this);
        setMovable(true);
        setOpaque(true);
    }

    Window::~Window()
    {
        setContent(NULL);
    }

    void Window::setPadding(unsigned int padding)
    {
        mPadding = padding;
        repositionContent();
    }

    unsigned int Window::getPadding() const
    {
        return mPadding;
    }

    void Window::setTitleBarHeight(unsigned int height)
    {
        mTitleBarHeight = height;
        repositionContent();
    }

    unsigned int Window::getTitleBarHeight()
    {
        return mTitleBarHeight;
    }

    void Window:: _announceDeath(Widget *)
    {
        mContent = NULL;
    }

    void Window::setContent(Widget* widget)
    {
        if (getContent() != NULL)
        {
            getContent()->_setParent(NULL);
            getContent()->_setFocusHandler(NULL);
        }

        if (widget != NULL)
        {
            widget->_setParent(this);
            widget->_setFocusHandler(_getFocusHandler());
        }

        mContent = widget;
        repositionContent();
    }

    Widget* Window::getContent() const
    {
        return mContent;
    }

    void Window::setCaption(const std::string& caption)
    {
        mCaption = caption;
        setDirty(true);
    }

    const std::string& Window::getCaption() const
    {
        return mCaption;
    }

    void Window::setAlignment(unsigned int alignment)
    {
        mAlignment = alignment;
    }

    unsigned int Window::getAlignment() const
    {
        return mAlignment;
    }

    void Window::draw(Graphics* graphics)
    {
        Color faceColor = getBaseColor();
        Color highlightColor, shadowColor;
        int alpha = getBaseColor().a;

        highlightColor = faceColor + 0x303030;
        highlightColor.a = alpha;
        shadowColor = faceColor - 0x303030;
        shadowColor.a = alpha;

        Rectangle d = getContentDimension();

        // Fill the background around the content
        graphics->setColor(faceColor);
        // Fill top
        graphics->fillRectangle(Rectangle(0,0,getWidth(),d.y - 1));
        // Fill left
        graphics->fillRectangle(Rectangle(0,d.y - 1, d.x - 1, getHeight() - d.y + 1));
        // Fill right
        graphics->fillRectangle(Rectangle(d.x + d.width + 1,
                                          d.y - 1,
                                          getWidth() - d.x - d.width - 1,
                                          getHeight() - d.y + 1));
        // Fill bottom
        graphics->fillRectangle(Rectangle(d.x - 1,
                                          d.y + d.height + 1,
                                          d.width + 2,
                                          getHeight() - d.height - d.y - 1));

        if (isOpaque())
        {
            graphics->fillRectangle(d);
        }

        // Construct a rectangle one pixel bigger than the content
        d.x -= 1;
        d.y -= 1;
        d.width += 2;
        d.height += 2;

        // Draw a border around the content
        graphics->setColor(shadowColor);
        // Top line
        graphics->drawLine(d.x,
                           d.y,
                           d.x + d.width - 2,
                           d.y);

        // Left line
        graphics->drawLine(d.x,
                           d.y + 1,
                           d.x,
                           d.y + d.height - 1);

        graphics->setColor(highlightColor);
        // Right line
        graphics->drawLine(d.x + d.width - 1,
                           d.y,
                           d.x + d.width - 1,
                           d.y + d.height - 2);
        // Bottom line
        graphics->drawLine(d.x + 1,
                           d.y + d.height - 1,
                           d.x + d.width - 1,
                           d.y + d.height - 1);

        drawContent(graphics);

        int textX = 0;
        int textY;
        textY = ((int)getTitleBarHeight() - getFont()->getHeight()) / 2;
        switch (getAlignment())
        {
          case Graphics::LEFT:
              textX = 4;
              break;
          case Graphics::CENTER:
              textX = getWidth() / 2;
              break;
          case Graphics::RIGHT:
              textX = getWidth() - 4;
              break;
          default:
              //throw GCN_EXCEPTION("Unknown alignment.");
              assert(!"Unknown alignment.");
        }

        graphics->setColor(getForegroundColor());
        graphics->setFont(getFont());
        graphics->drawText(getCaption(), textX, textY, getAlignment());
    }

    void Window::drawBorder(Graphics* graphics)
    {
        Color faceColor = getBaseColor();
        Color highlightColor, shadowColor;
        int alpha = getBaseColor().a;
        int width = getWidth() + getBorderSize() * 2 - 1;
        int height = getHeight() + getBorderSize() * 2 - 1;
        highlightColor = faceColor + 0x303030;
        highlightColor.a = alpha;
        shadowColor = faceColor - 0x303030;
        shadowColor.a = alpha;

        unsigned int i;
        for (i = 0; i < getBorderSize(); ++i)
        {
            graphics->setColor(highlightColor);
            graphics->drawLine(i,i, width - i, i);
            graphics->drawLine(i,i + 1, i, height - i - 1);
            graphics->setColor(shadowColor);
            graphics->drawLine(width - i,i + 1, width - i, height - i);
            graphics->drawLine(i,height - i, width - i - 1, height - i);
        }
    }

    void Window::drawContent(Graphics* graphics)
    {
        if (getContent() != NULL)
        {
            graphics->pushClipArea(getContentDimension());
            graphics->pushClipArea(Rectangle(0, 0, getContent()->getWidth(),
                                             getContent()->getHeight()));
            getContent()->draw(graphics);
            graphics->popClipArea();
            graphics->popClipArea();
        }
    }

    void Window::mousePress(int x, int y, int button)
    {
        if (getParent() != NULL)
        {
            getParent()->moveToTop(this);
        }

        if (isMovable() && hasMouse()
            && y < (int)(getTitleBarHeight() + getPadding()) && button == 1)
        {
            mMouseDrag = true;
            mMouseXOffset = x;
            mMouseYOffset = y;
        }
    }

    void Window::mouseRelease(int, int, int button)
    {
        if (button == 1)
        {
            mMouseDrag = false;
        }
    }

    void Window::mouseMotion(int x, int y)
    {
        if (mMouseDrag && isMovable())
        {
            setPosition(x - mMouseXOffset + getX(),
                        y - mMouseYOffset + getY());
			setDirty(true);
        }
    }

    void Window::moveToTop(Widget* widget)
    {
        if (widget != getContent())
        {
            //throw GCN_EXCEPTION("Widget is not content of window.");
            assert(!"Widget is not content of window.");
        }
    }

    void Window::moveToBottom(Widget* widget)
    {
        if (widget != getContent())
        {
            //throw GCN_EXCEPTION("Widget is not content of window");
            assert(!"Widget is not content of window.");
        }
    }

    void Window::getDrawSize(int& width, int& height, Widget* widget)
    {
        if (widget != getContent())
        {
            //throw GCN_EXCEPTION("Widget is not content of window");
            assert(!"Widget is not content of window.");
        }

        Rectangle d = getContentDimension();
        width = d.width;
        height = d.height;
    }

    void Window::repositionContent()
    {
        if (getContent() == NULL)
        {
            return;
        }

        Rectangle d = getContentDimension();
        mContent->setPosition(d.x, d.y);
    }

    Rectangle Window::getContentDimension()
    {
        return Rectangle(getPadding(),
                         getTitleBarHeight(),
                         getWidth() - getPadding() * 2,
                         getHeight() - getPadding() - getTitleBarHeight());
    }

    void Window::setMovable(bool movable)
    {
        mMovable = movable;
    }

    bool Window::isMovable() const
    {
        return mMovable;
    }

    void Window::resizeToContent()
    {
        if (getContent() != NULL)
        {
            setSize(getContent()->getWidth() + 2*getPadding(),
                    getContent()->getHeight() + getPadding()
                    + getTitleBarHeight());
        }
    }

    void Window::_mouseInputMessage(const MouseInput &mouseInput)
    {
        BasicContainer::_mouseInputMessage(mouseInput);

        if (getContent() != NULL)
        {
            if (getContentDimension().isPointInRect(mouseInput.x, mouseInput.y) &&
                getContent()->getDimension().isPointInRect(mouseInput.x, mouseInput.y))
            {
                if (!getContent()->hasMouse())
                {
                    getContent()->_mouseInMessage();
                }

                MouseInput mi = mouseInput;
                mi.x -= getContent()->getX();
                mi.y -= getContent()->getY();
                getContent()->_mouseInputMessage(mi);
            }
            else if (getContent()->hasMouse())
            {
                getContent()->_mouseOutMessage();
            }
        }
    }

    void Window::_mouseOutMessage()
    {
        BasicContainer::_mouseOutMessage();

        if (getContent() != NULL && getContent()->hasMouse())
        {
            getContent()->_mouseOutMessage();
        }
    }

    void Window::_setFocusHandler(FocusHandler *focusHandler)
    {
        if (getContent() != NULL)
        {
            getContent()->_setFocusHandler(focusHandler);
        }

        BasicContainer::_setFocusHandler(focusHandler);
    }

    void Window::setOpaque(bool opaque)
    {
        mOpaque = opaque;
    }

    bool Window::isOpaque()
    {
        return mOpaque;
    }

    void Window::logic()
    {
        if (getContent() != NULL)
        {
            getContent()->logic();
        }
    }

	void Window::setDirty(bool dirty)
	{
		if (mContent != NULL)
		{
			mContent->setDirty(dirty);
		}
		mDirty = dirty;
	}

	bool Window::getDirty() const
	{
		if (mDirty == true)
		{
			return true;
		}

		if (mContent != NULL && mContent->getDirty())
		{
			return true;
		}

		return false;
	}
}
