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
#include "guichan/exception.h"
#include "guichan/widgets/container.h"

namespace gcn
{

    Container::Container()
    {
        mWidgetWithMouse = NULL;
        mOpaque = true;
    }

    Container::~Container()
    {
        clear();
    }

    void Container::logic()
    {
        logicChildren();
    }

    void Container::draw(Graphics* graphics)
    {
        if (isOpaque())
        {
            graphics->setColor(getBaseColor());
            graphics->fillRectangle(Rectangle(0, 0, getWidth(), getHeight()));
        }

        drawChildren(graphics);
    }

    void Container::drawBorder(Graphics* graphics)
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
            graphics->setColor(shadowColor);
            graphics->drawLine(i,i, width - i, i);
            graphics->drawLine(i,i + 1, i, height - i - 1);
            graphics->setColor(highlightColor);
            graphics->drawLine(width - i,i + 1, width - i, height - i);
            graphics->drawLine(i,height - i, width - i - 1, height - i);
        }
    }

    void Container::logicChildren()
    {
        WidgetIterator iter;
        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
        {
            (*iter)->logic();
        }
    }

    void Container::drawChildren(Graphics* graphics)
    {
        WidgetIterator iter;
        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
        {
            if ((*iter)->isVisible())
            {
                // If the widget has a border,
                // draw it before drawing the widget
                if ((*iter)->getBorderSize() > 0)
                {
                    Rectangle rec = (*iter)->getDimension();
                    rec.x -= (*iter)->getBorderSize();
                    rec.y -= (*iter)->getBorderSize();
                    rec.width += 2 * (*iter)->getBorderSize();
                    rec.height += 2 * (*iter)->getBorderSize();
                    graphics->pushClipArea(rec);
                    (*iter)->drawBorder(graphics);
                    graphics->popClipArea();
                }

                graphics->pushClipArea((*iter)->getDimension());
                (*iter)->draw(graphics);
                graphics->popClipArea();
            }
        }
    }

    void Container::setOpaque(bool opaque)
    {
        mOpaque = opaque;
    }

    bool Container::isOpaque() const
    {
        return mOpaque;
    }

    void Container::moveToTop(Widget* widget)
    {
        WidgetIterator iter;
        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
        {
            if (*iter == widget)
            {
                mWidgets.erase(iter);
                mWidgets.push_back(widget);
                return;
            }
        }
		assert(!"There is no such widget in this container.");
        //throw GCN_EXCEPTION("There is no such widget in this container.");
    }

    void Container::moveToBottom(Widget* widget)
    {
        WidgetIterator iter;
        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
        {
            if (*iter == widget)
            {
                mWidgets.erase(iter);
                mWidgets.push_front(widget);
                return;
            }
        }
		assert(!"There is no such widget in this container.");
        //throw GCN_EXCEPTION("There is no such widget in this container.");
    }

    void Container::_announceDeath(Widget *widget)
    {
        if (mWidgetWithMouse == widget)
        {
            mWidgetWithMouse = NULL;
        }

        WidgetIterator iter;
        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
        {
            if (*iter == widget)
            {
                mWidgets.erase(iter);
                return;
            }
        }
		assert(!"There is no such widget in this container.");
        //throw GCN_EXCEPTION("There is no such widget in this container.");
    }

    void Container::getDrawSize(int& width, int& height, Widget* widget)
    {
        WidgetIterator iter;
        bool contains = false;

        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
        {
            if (widget == *iter)
            {
                contains = true;
                break;
            }
        }

        if (contains)
        {
            Rectangle widgetDim = widget->getDimension();
            Rectangle dim = getDimension();

            width = widgetDim.width;
            height = widgetDim.height;

            if (widgetDim.x < 0)
            {
                width += widgetDim.x;
            }

            if (widgetDim.y < 0)
            {
                height += widgetDim.y;
            }

            if (widgetDim.x + widgetDim.width > dim.width)
            {
                width -= (widgetDim.x + widgetDim.width) - dim.width;
            }

            if (widgetDim.y + widgetDim.height > dim.height)
            {
                height -= (widgetDim.y + widgetDim.height) - dim.height;
            }

            if (width < 0)
            {
                width = 0;
            }

            if (height < 0)
            {
                height = 0;
            }
        }
        else
        {
        	assert(!"Widget not in container.");
            //throw GCN_EXCEPTION("Widget not in container.");
        }
    }

    void Container::add(Widget* widget)
    {
        mWidgets.push_back(widget);
        widget->_setFocusHandler(_getFocusHandler());
        widget->_setParent(this);
    }

    void Container::add(Widget* widget, int x, int y)
    {
        widget->setPosition(x, y);
        add(widget);
    }

    void Container::remove(Widget* widget)
    {
        if (mWidgetWithMouse == widget)
        {
            mWidgetWithMouse = NULL;
        }

        WidgetIterator iter;
        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
        {
            if (*iter == widget)
            {
                mWidgets.erase(iter);
                widget->_setFocusHandler(NULL);
                widget->_setParent(NULL);
                return;
            }
        }
		assert(!"There is no such widget in this container.");
        //throw GCN_EXCEPTION("There is no such widget in this container.");
    }

    void Container::clear()
    {
        mWidgetWithMouse = NULL;

        WidgetIterator iter;

        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
        {
            (*iter)->_setFocusHandler(NULL);
            (*iter)->_setParent(NULL);
        }

        mWidgets.clear();
    }

    void Container::_setFocusHandler(FocusHandler* focusHandler)
    {
        Widget::_setFocusHandler(focusHandler);

        WidgetIterator iter;
        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
        {
            (*iter)->_setFocusHandler(focusHandler);
        }
    }

    void Container::_mouseInputMessage(const MouseInput &mouseInput)
    {
        Widget* tempWidgetWithMouse = NULL;

        WidgetIterator iter;
        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
        {
            if ((*iter)->getDimension().isPointInRect(mouseInput.x, mouseInput.y)
                && (*iter)->isVisible())
            {
                tempWidgetWithMouse = (*iter);
            }
        }

        if (tempWidgetWithMouse != mWidgetWithMouse)
        {
            if (mWidgetWithMouse)
            {
                mWidgetWithMouse->_mouseOutMessage();
            }

            if (tempWidgetWithMouse)
            {
                tempWidgetWithMouse->_mouseInMessage();
            }

            mWidgetWithMouse = tempWidgetWithMouse;
        }

        if (mWidgetWithMouse != NULL)
        {
            MouseInput mi = mouseInput;
            mi.x -= mWidgetWithMouse->getX();
            mi.y -= mWidgetWithMouse->getY();
            mWidgetWithMouse->_mouseInputMessage(mi);
        }

         if (mWidgetWithMouse == NULL)
         {
             BasicContainer::_mouseInputMessage(mouseInput);
         }
    }

    void Container::_mouseOutMessage()
    {
        if (mWidgetWithMouse)
        {
            mWidgetWithMouse->_mouseOutMessage();
            mWidgetWithMouse = NULL;
        }

        Widget::_mouseOutMessage();
    }

	void Container::setDirty(bool dirty)
	{
        WidgetConstIterator iter;
        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
		{
			(*iter)->setDirty(dirty);
		}
		mDirty = dirty;
	}

	bool Container::getDirty() const
	{
		if (mDirty == true)
		{
			return true;
		}

		WidgetConstIterator iter;
        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
		{
			if ((*iter)->getDirty())
			{
				return true;
			}
		}

		return false;
	}
}
