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

#include <assert.h>
#include "guichan/exception.h"
#include "guichan/widgets/dropdown.h"

namespace gcn
{
    DropDown::DropDown()
    {
        mDroppedDown = false;
        mPushed = false;
        mOldH = 0;

        setWidth(100);
        setFocusable(true);

        mDefaultScrollArea = new ScrollArea();
        mDefaultScrollArea->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
        mDefaultListBox = new ListBox();

        mScrollArea = mDefaultScrollArea;
        mScrollArea->_setFocusHandler(&mFocusHandler);
        mScrollArea->_setParent(this);

        mListBox = mDefaultListBox;
        mListBox->addActionListener(this);
        mScrollArea->setContent(mListBox);

        addMouseListener(this);
        addKeyListener(this);
        adjustHeight();
        setBorderSize(1);
    }

    DropDown::DropDown(ListModel *listModel)
    {
        setWidth(100);
        setFocusable(true);
        mDroppedDown = false;
        mPushed = false;
        mOldH = 0;

        mDefaultScrollArea = new ScrollArea();
        mDefaultScrollArea->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
        mDefaultListBox = new ListBox();

        mScrollArea = mDefaultScrollArea;
        mScrollArea->_setParent(this);
        mListBox = mDefaultListBox;
        mListBox->addActionListener(this);

        mScrollArea->setContent(mListBox);
        mScrollArea->_setFocusHandler(&mFocusHandler);
        mScrollArea->_setParent(this);

        setListModel(listModel);

        if (mListBox->getSelected() < 0)
        {
            mListBox->setSelected(0);
        }

        addMouseListener(this);
        addKeyListener(this);
        adjustHeight();
        setBorderSize(1);
    }

    DropDown::DropDown(ListModel *listModel,
                       ScrollArea *scrollArea,
                       ListBox *listBox)
    {
        setWidth(100);
        setFocusable(true);
        mDroppedDown = false;
        mPushed = false;
        mOldH = 0;

        mDefaultScrollArea = NULL;
        mDefaultListBox = NULL;

        mScrollArea = scrollArea;
        mScrollArea->_setFocusHandler(&mFocusHandler);

        mListBox = listBox;
        mListBox->addActionListener(this);
        mScrollArea->setContent(mListBox);
        mScrollArea->_setParent(this);

        setListModel(listModel);

        if (mListBox->getSelected() < 0)
        {
            mListBox->setSelected(0);
        }

        addMouseListener(this);
        addKeyListener(this);
        adjustHeight();
        setBorderSize(1);
    }

    DropDown::~DropDown()
    {
        if (mScrollArea != NULL)
        {
            mScrollArea->_setFocusHandler(NULL);
        }

        if (mDefaultScrollArea != NULL)
        {
            delete mDefaultScrollArea;
        }

        if (mDefaultListBox != NULL)
        {
            delete mDefaultListBox;
        }

        if (widgetExists(mListBox))
        {
            mListBox->removeActionListener(this);
        }
    }

    void DropDown::logic()
    {
        if (mScrollArea == NULL || mScrollArea->getContent() == NULL)
        {
            //throw GCN_EXCEPTION("ScrollArea or ListBox is NULL.");
            assert(!"ScrollArea or ListBox is NULL.");
        }

        mScrollArea->logic();
        mFocusHandler.applyChanges();
    }

    void DropDown::draw(Graphics* graphics)
    {
        if (mScrollArea == NULL || mScrollArea->getContent() == NULL)
        {
            //throw GCN_EXCEPTION("ScrollArea or ListBox is NULL.");
           assert(!"ScrollArea or ListBox is NULL.");
        }

        int h;

        if (mDroppedDown)
        {
            h = mOldH;
        }
        else
        {
            h = getHeight();
        }

        int alpha = getBaseColor().a;
        Color faceColor = getBaseColor();
        faceColor.a = alpha;
        Color highlightColor = faceColor + 0x303030;
        highlightColor.a = alpha;
        Color shadowColor = faceColor - 0x303030;
        shadowColor.a = alpha;


        graphics->setColor(getBackgroundColor());
        graphics->fillRectangle(Rectangle(0, 0, getWidth(), h));

        graphics->setColor(getForegroundColor());
        graphics->setFont(getFont());

        if (mListBox->getListModel() && mListBox->getSelected() >= 0)
        {
            graphics->drawText(mListBox->getListModel()->getElementAt(mListBox->getSelected()),
                1, (h - getFont()->getHeight()) / 2);
        }

        if (hasFocus())
        {
            graphics->drawRectangle(Rectangle(0, 0, getWidth() - h, h));
        }

        drawButton(graphics);

         if (mDroppedDown)
         {
             graphics->pushClipArea(mScrollArea->getDimension());
             mScrollArea->draw(graphics);
             graphics->popClipArea();

            // Draw two lines separating the ListBox with se selected
            // element view.
            graphics->setColor(highlightColor);
            graphics->drawLine(0, h, getWidth(), h);
            graphics->setColor(shadowColor);
            graphics->drawLine(0, h + 1,getWidth(),h + 1);
         }
    }

    void DropDown::drawBorder(Graphics* graphics)
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

    void DropDown::drawButton(Graphics *graphics)
    {
        Color faceColor, highlightColor, shadowColor;
        int offset;
        int alpha = getBaseColor().a;

        if (mPushed)
        {
            faceColor = getBaseColor() - 0x303030;
            faceColor.a = alpha;
            highlightColor = faceColor - 0x303030;
            highlightColor.a = alpha;
            shadowColor = faceColor + 0x303030;
            shadowColor.a = alpha;
            offset = 1;
        }
        else
        {
            faceColor = getBaseColor();
            faceColor.a = alpha;
            highlightColor = faceColor + 0x303030;
            highlightColor.a = alpha;
            shadowColor = faceColor - 0x303030;
            shadowColor.a = alpha;
            offset = 0;
        }

        int h;
        if (mDroppedDown)
        {
            h = mOldH;
        }
        else
        {
            h = getHeight();
        }
        int x = getWidth() - h;
        int y = 0;

        graphics->setColor(faceColor);
        graphics->fillRectangle(Rectangle(x+1, y+1, h-2, h-2));

        graphics->setColor(highlightColor);
        graphics->drawLine(x, y, x+h-1, y);
        graphics->drawLine(x, y+1, x, y+h-1);

        graphics->setColor(shadowColor);
        graphics->drawLine(x+h-1, y+1, x+h-1, y+h-1);
        graphics->drawLine(x+1, y+h-1, x+h-2, y+h-1);

        graphics->setColor(getForegroundColor());

        int i;
        int hh = h / 3;
        int hx = x + h / 2;
        int hy = y + (h * 2) / 3;
        for (i=0; i<hh; i++)
        {
            graphics->drawLine(hx - i + offset,
                               hy - i + offset,
                               hx + i + offset,
                               hy - i + offset);
        }
    }

    int DropDown::getSelected()
    {
        if (mScrollArea == NULL || mScrollArea->getContent() == NULL)
        {
            assert(!"ScrollArea or ListBox is NULL.");
            //throw GCN_EXCEPTION("ScrollArea or ListBox is NULL.");
        }

        return mListBox->getSelected();
    }

    void DropDown::setSelected(int selected)
    {
        if (mScrollArea == NULL || mScrollArea->getContent() == NULL)
        {
            assert(!"ScrollArea or ListBox is NULL.");
            //throw GCN_EXCEPTION("ScrollArea or ListBox is NULL.");
        }

        if (selected >= 0)
        {
            mListBox->setSelected(selected);
        }
    }

    bool DropDown::keyPress(const Key& key)
    {
        if (mScrollArea == NULL || mScrollArea->getContent() == NULL)
        {
            //throw GCN_EXCEPTION("ScrollArea or ListBox is NULL.");
            assert(!"ScrollArea or ListBox is NULL.");
        }

        if ((key.getValue() == Key::K_ENTER || key.getValue() == Key::K_SPACE)
            && !mDroppedDown)
        {
            dropDown();
            return true;
        }
		return false;
    }

    void DropDown::mousePress(int, int y, int button)
    {
        if (button == MouseInput::LEFT && hasMouse() && !mDroppedDown)
        {
            mPushed = true;
            dropDown();
        }
        // Fold up the listbox if the upper part is clicked after fold down
        else if (button == MouseInput::LEFT && hasMouse() && mDroppedDown
                 && y < mOldH)
        {
            foldUp();
        }
        else if (!hasMouse())
        {
            foldUp();
        }
    }

    void DropDown::mouseRelease(int, int, int button)
    {
        if (button == MouseInput::LEFT)
        {
            mPushed = false;
        }
    }

    void DropDown::setListModel(ListModel *listModel)
    {
        if (mScrollArea == NULL || mScrollArea->getContent() == NULL)
        {
            //throw GCN_EXCEPTION("ScrollArea or ListBox is NULL.");
            assert(!"ScrollArea or ListBox is NULL.");
        }

        mListBox->setListModel(listModel);

        if (mListBox->getSelected() < 0)
        {
            mListBox->setSelected(0);
        }

        adjustHeight();
    }

    ListModel *DropDown::getListModel()
    {
        if (mScrollArea == NULL || mScrollArea->getContent() == NULL)
        {
            //throw GCN_EXCEPTION("ScrollArea or ListBox is NULL.");
            assert(!"ScrollArea or ListBox is NULL.");
        }

        return mListBox->getListModel();
    }

    void DropDown::setScrollArea(ScrollArea *scrollArea)
    {
        mScrollArea->_setFocusHandler(NULL);
        mScrollArea->_setParent(NULL);
        mScrollArea = scrollArea;
        mScrollArea->_setFocusHandler(&mFocusHandler);
        mScrollArea->setContent(mListBox);
        mScrollArea->_setParent(this);
        adjustHeight();
    }

    ScrollArea *DropDown::getScrollArea()
    {
        return mScrollArea;
    }

    void DropDown::setListBox(ListBox *listBox)
    {
        listBox->setSelected(mListBox->getSelected());
        listBox->setListModel(mListBox->getListModel());
        listBox->addActionListener(this);

        if (mScrollArea->getContent() != NULL)
        {
            mListBox->removeActionListener(this);
        }

        mListBox = listBox;

        mScrollArea->setContent(mListBox);

        if (mListBox->getSelected() < 0)
        {
            mListBox->setSelected(0);
        }
    }

    ListBox *DropDown::getListBox()
    {
        return mListBox;
    }

    void DropDown::adjustHeight()
    {
        if (mScrollArea == NULL || mScrollArea->getContent() == NULL)
        {
            //throw GCN_EXCEPTION("ScrollArea or ListBox is NULL.");
            assert(!"ScrollArea or ListBox is NULL.");
        }

        int listBoxHeight = mListBox->getHeight();
        int h2 = mOldH ? mOldH : getFont()->getHeight();

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

    void DropDown::dropDown()
    {
        if (!mDroppedDown)
        {
            mDroppedDown = true;
            mOldH = getHeight();
            adjustHeight();

            if (getParent())
            {
                getParent()->moveToTop(this);
            }
        }

        mFocusHandler.requestFocus(mScrollArea->getContent());
    }

    void DropDown::foldUp()
    {
        if (mDroppedDown)
        {
            mDroppedDown = false;
            mFocusHandler.focusNone();
            adjustHeight();
        }
    }

    bool DropDown::_keyInputMessage(const KeyInput& keyInput)
    {
        if (mDroppedDown)
        {
            if (mScrollArea == NULL || mScrollArea->getContent() == NULL)
            {
                //throw GCN_EXCEPTION("ScrollArea or ListBox is NULL.");
                assert(!"ScrollArea or ListBox is NULL.");
            }

            if (mFocusHandler.getFocused() != NULL)
            {
                return mFocusHandler.getFocused()->_keyInputMessage(keyInput);
            }
            else
            {
                return false;
            }
        }
        else
        {
            return BasicContainer::_keyInputMessage(keyInput);
        }
    }

    void DropDown::_mouseInputMessage(const MouseInput &mouseInput)
    {
        BasicContainer::_mouseInputMessage(mouseInput);

        if (mDroppedDown)
        {
            if (mScrollArea == NULL || mScrollArea->getContent() == NULL)
            {
                //throw GCN_EXCEPTION("ScrollArea or ListBox is NULL.");
                assert(!"ScrollArea or ListBox is NULL.");
            }

            if (mouseInput.y >= mOldH)
            {
                MouseInput mi = mouseInput;
                mi.y -= mScrollArea->getY();
                mScrollArea->_mouseInputMessage(mi);

                if (mListBox->hasFocus())
                {
                    mi.y -= mListBox->getY();
                    mListBox->_mouseInputMessage(mi);
                }
            }
        }
    }

    void DropDown::lostFocus()
    {
        foldUp();
    }

    void DropDown::moveToTop(Widget*)
    {
        if (getParent())
        {
            getParent()->moveToTop(this);
        }
    }

    void DropDown::moveToBottom(Widget*)
    {
        if (getParent())
        {
            getParent()->moveToBottom(this);
        }
    }

    void DropDown::_announceDeath(Widget* widget)
    {
        if (widget == mScrollArea)
        {
            mScrollArea = NULL;
        }
        else
        {
            assert(!"Death announced for unknown widget..");
            //throw GCN_EXCEPTION("Death announced for unknown widget..");
        }
    }

    void DropDown::action(const std::string&)
    {
        foldUp();
        generateAction();
    }

    void DropDown::getDrawSize(int& width, int& height, Widget* widget)
    {
        if (widget == mScrollArea)
        {
            if (mDroppedDown)
            {
                height = getHeight() - mOldH;
                width = getWidth();
            }
            else
            {
                width = height = 0;
            }
        }
        else
        {
			assert(!"DropDown::getDrawSize. widget is not the ScrollArea (wieeerd...)");
            //throw GCN_EXCEPTION("DropDown::getDrawSize. widget is not the ScrollArea (wieeerd...)");
        }
    }

    void DropDown::setBaseColor(const Color& color)
    {
        if (mDefaultScrollArea == mScrollArea && mScrollArea != NULL)
        {
            mScrollArea->setBaseColor(color);
        }

        if (mDefaultListBox == mListBox && mListBox != NULL)
        {
            mListBox->setBaseColor(color);
        }

        Widget::setBaseColor(color);
    }

    void DropDown::setBackgroundColor(const Color& color)
    {
        if (mDefaultScrollArea == mScrollArea && mScrollArea != NULL)
        {
            mScrollArea->setBackgroundColor(color);
        }

        if (mDefaultListBox == mListBox && mListBox != NULL)
        {
            mListBox->setBackgroundColor(color);
        }

        Widget::setBackgroundColor(color);
    }

    void DropDown::setForegroundColor(const Color& color)
    {
        if (mDefaultScrollArea == mScrollArea && mScrollArea != NULL)
        {
            mScrollArea->setForegroundColor(color);
        }

        if (mDefaultListBox == mListBox && mListBox != NULL)
        {
            mListBox->setForegroundColor(color);
        }

        Widget::setForegroundColor(color);
    }

    void DropDown::setFont(Font *font)
    {
        Widget::setFont(font);
        mListBox->setFont(font);
    }

	bool DropDown::getDirty() const
	{
		if (mDroppedDown)
		{
			return mScrollArea->getDirty();
		}
		return mDirty;
	}
}

