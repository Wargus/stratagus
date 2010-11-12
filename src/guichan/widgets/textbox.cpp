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

#include <typeinfo>

#include "guichan/basiccontainer.h"
#include "guichan/keyinput.h"
#include "guichan/mouseinput.h"
#include "guichan/widgets/scrollarea.h"
#include "guichan/widgets/textbox.h"
#include "guichan/exception.h"
#include "util.h"

namespace gcn
{
    TextBox::TextBox()
    {
        mCaretColumn = 0;
        mCaretRow = 0;
        mEditable = true;
        mOpaque = true;

        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        adjustSize();
        setBorderSize(1);
    }

    TextBox::TextBox(const std::string& text)
    {
        mCaretColumn = 0;
        mCaretRow = 0;
        mEditable = true;
        mOpaque = true;

        setText(text);

        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        adjustSize();
        setBorderSize(1);
    }

    void TextBox::setText(const std::string& text)
    {
        mCaretColumn = 0;
        mCaretRow = 0;

        mTextRows.clear();

        std::string::size_type pos, lastPos = 0;
        int length;
        do
        {
            pos = text.find("\n", lastPos);

            if (pos != std::string::npos)
            {
                length = pos - lastPos;
            }
            else
            {
                length = text.size() - lastPos;
            }
            std::string sub = text.substr(lastPos, length);
            mTextRows.push_back(sub);
            lastPos = pos + 1;

        } while (pos != std::string::npos);

        adjustSize();
    }

    void TextBox::draw(Graphics* graphics)
    {
        unsigned int i;

        if (mOpaque)
        {
            graphics->setColor(getBackgroundColor());
            graphics->fillRectangle(Rectangle(0, 0, getWidth(), getHeight()));
        }

        if (hasFocus() && isEditable())
        {
            drawCaret(graphics, getFont()->getWidth(mTextRows[mCaretRow].substr(0, mCaretColumn)), mCaretRow * getFont()->getHeight());
        }

        graphics->setColor(getForegroundColor());
        graphics->setFont(getFont());

        for (i = 0; i < mTextRows.size(); i++)
        {
            // Move the text one pixel so we can have a caret before a letter.
            graphics->drawText(mTextRows[i], 1, i * getFont()->getHeight());
        }
    }

    void TextBox::drawBorder(Graphics* graphics)
    {
        int width = getWidth() + getBorderSize() * 2 - 1;
        int height = getHeight() + getBorderSize() * 2 - 1;

        graphics->setColor(getBackgroundColor());

        unsigned int i;
        for (i = 0; i < getBorderSize(); ++i)
        {
            graphics->drawLine(i,i, width - i, i);
            graphics->drawLine(i,i + 1, i, height - i - 1);
            graphics->drawLine(width - i,i + 1, width - i, height - i);
            graphics->drawLine(i,height - i, width - i - 1, height - i);
        }
    }

    void TextBox::drawCaret(Graphics* graphics, int x, int y)
    {
        graphics->setColor(getForegroundColor());
        graphics->drawLine(x, getFont()->getHeight() + y, x, y);
    }

    void TextBox::mousePress(int x, int y, int button)
    {
        if (hasMouse() && button == MouseInput::LEFT)
        {
            mCaretRow = y / getFont()->getHeight();

            if (mCaretRow >= (int)mTextRows.size())
            {
                mCaretRow = mTextRows.size() - 1;
            }

            mCaretColumn = getFont()->getStringIndexAt(mTextRows[mCaretRow], x);
        }
        else if (hasMouse() && button == MouseInput::MIDDLE)
        {
            std::string str;
            if (GetClipboard(str) >= 0) {
                for (size_t i = 0; i < str.size(); ++i) {
                    keyPress(Key(str[i]));
                }
            }
        }
    }

	static int FindNext(const std::string &text, int curpos)
	{
		if (curpos < 0) return 0;
		while (curpos < (int)text.size()) {
			if ((text[curpos] & 0xC0) != 0x80) {
				return curpos;
			}
			++curpos;
		}
		return text.size();
	}

    bool TextBox::keyPress(const Key& key)
    {
        bool ret = false;

        if (key.getValue() == Key::K_LEFT)
        {
            mCaretColumn = UTF8GetPrev(mTextRows[mCaretRow], mCaretColumn);
            if (mCaretColumn < 0)
            {
                --mCaretRow;

                if (mCaretRow < 0)
                {
                    mCaretRow = 0;
                    mCaretColumn = 0;
                }
                else
                {
                    mCaretColumn = mTextRows[mCaretRow].size();
                }
            }
            ret = true;
        }

        else if (key.getValue() == Key::K_RIGHT)
        {
            mCaretColumn = UTF8GetNext(mTextRows[mCaretRow], mCaretColumn);
            if (mCaretColumn > (int)mTextRows[mCaretRow].size())
            {
                ++mCaretRow;

                if (mCaretRow >= (int)mTextRows.size())
                {
                    mCaretRow = mTextRows.size() - 1;
                    if (mCaretRow < 0)
                    {
                        mCaretRow = 0;
                    }

                    mCaretColumn = mTextRows[mCaretRow].size();
                }
                else
                {
                    mCaretColumn = 0;
                }
            }
            ret = true;
        }

        else if (key.getValue() == Key::K_DOWN)
        {
            setCaretRow(mCaretRow + 1);
            ret = true;
        }

        else if (key.getValue() == Key::K_UP)
        {
            setCaretRow(mCaretRow - 1);
            ret = true;
        }

        else if (key.getValue() == Key::K_HOME)
        {
            mCaretColumn = 0;
            ret = true;
        }

        else if (key.getValue() == Key::K_END)
        {
            mCaretColumn = mTextRows[mCaretRow].size();
            ret = true;
        }

        else if (key.getValue() == Key::K_ENTER && mEditable)
        {
            mTextRows.insert(mTextRows.begin() + mCaretRow + 1,
                             mTextRows[mCaretRow].substr(mCaretColumn, mTextRows[mCaretRow].size() - mCaretColumn));
            mTextRows[mCaretRow].resize(mCaretColumn);
            ++mCaretRow;
            mCaretColumn = 0;
            ret = true;
        }

        else if (key.getValue() == Key::K_BACKSPACE
                 && mCaretColumn != 0
                 && mEditable)
        {
			int newpos = UTF8GetPrev(mTextRows[mCaretRow], mCaretColumn);
            mTextRows[mCaretRow].erase(newpos, mCaretColumn - newpos);
            mCaretColumn = newpos;
            ret = true;
        }

        else if (key.getValue() == Key::K_BACKSPACE
                 && mCaretColumn == 0
                 && mCaretRow != 0
                 && mEditable)
        {
            mCaretColumn = mTextRows[mCaretRow - 1].size();
            mTextRows[mCaretRow - 1] += mTextRows[mCaretRow];
            mTextRows.erase(mTextRows.begin() + mCaretRow);
            --mCaretRow;
            ret = true;
        }

        else if (key.getValue() == Key::K_DELETE
                 && mCaretColumn < (int)mTextRows[mCaretRow].size()
                 && mEditable)
        {
			int newpos = UTF8GetNext(mTextRows[mCaretRow], mCaretColumn);
            mTextRows[mCaretRow].erase(mCaretColumn, newpos - mCaretColumn);
            ret = true;
        }

        else if (key.getValue() == Key::K_DELETE
                 && mCaretColumn == (int)mTextRows[mCaretRow].size()
                 && mCaretRow < ((int)mTextRows.size() - 1)
                 && mEditable)
        {
            mTextRows[mCaretRow] += mTextRows[mCaretRow + 1];
            mTextRows.erase(mTextRows.begin() + mCaretRow + 1);
            ret = true;
        }

        else if(key.getValue() == Key::K_PAGE_UP)
        {
            int w, h, rowsPerPage;
            getParent()->getDrawSize(w, h, this);
            rowsPerPage = h / getFont()->getHeight();
            mCaretRow -= rowsPerPage;

            if (mCaretRow < 0)
            {
                mCaretRow = 0;
            }
            ret = true;
        }

        else if(key.getValue() == Key::K_PAGE_DOWN)
        {
            int w, h, rowsPerPage;
            getParent()->getDrawSize(w, h, this);
            rowsPerPage = h / getFont()->getHeight();
            mCaretRow += rowsPerPage;

            if (mCaretRow >= (int)mTextRows.size())
            {
                mCaretRow = mTextRows.size() - 1;
            }
            ret = true;
        }

        else if(key.getValue() == Key::K_TAB
                && mEditable)
        {
            mTextRows[mCaretRow].insert(mCaretColumn,std::string("    "));
            mCaretColumn += 4;
            ret = true;
        }

        else if (key.getValue() == 'v' - 'a' + 1 && mEditable) // ctrl-v
        {
            std::string str;
            if (GetClipboard(str) >= 0) {
                for (size_t i = 0; i < str.size(); ++i) {
                    keyPress(Key(str[i]));
                }
                ret = true;
            }
        }

        else if (key.isCharacter()
                 && mEditable)
        {
            mTextRows[mCaretRow].insert(mCaretColumn,key.toString());
            mCaretColumn = UTF8GetNext(mTextRows[mCaretRow], mCaretColumn);
            ret = true;
        }

        adjustSize();
        scrollToCaret();
        return ret;
    }

    void TextBox::adjustSize()
    {
        unsigned int i;
        int width = 0;
        for (i = 0; i < mTextRows.size(); ++i)
        {
            int w = getFont()->getWidth(mTextRows[i]);
            if (width < w)
            {
                width = w;
            }
        }

        setWidth(width + 1);
        setHeight(getFont()->getHeight() * mTextRows.size());
    }

    void TextBox::setCaretPosition(unsigned int position)
    {
        int row;

        for (row = 0; row < (int)mTextRows.size(); row++)
        {
            if (position <= mTextRows[row].size())
            {
                mCaretRow = row;
                mCaretColumn = position;
                return; // we are done
            }
            else
            {
                position--;
            }
        }

        // position beyond end of text
        mCaretRow = mTextRows.size() - 1;
        mCaretColumn = mTextRows[mCaretRow].size();
    }

    unsigned int TextBox::getCaretPosition() const
    {
        int pos = 0, row;

        for (row = 0; row < mCaretRow; row++)
        {
            pos += mTextRows[row].size();
        }

        return pos + mCaretColumn;
    }

    void TextBox::setCaretRowColumn(int row, int column)
    {
        setCaretRow(row);
        setCaretColumn(column);
    }

    void TextBox::setCaretRow(int row)
    {
        mCaretRow = row;

        if (mCaretRow >= (int)mTextRows.size())
        {
            mCaretRow = mTextRows.size() - 1;
        }

        if (mCaretRow < 0)
        {
            mCaretRow = 0;
        }

        setCaretColumn(mCaretColumn);
    }

    unsigned int TextBox::getCaretRow() const
    {
        return mCaretRow;
    }

    void TextBox::setCaretColumn(int column)
    {
        mCaretColumn = column;

        if (mCaretColumn > (int)mTextRows[mCaretRow].size())
        {
            mCaretColumn = mTextRows[mCaretRow].size();
        }

        if (mCaretColumn < 0)
        {
            mCaretColumn = 0;
        }

		mCaretColumn = FindNext(mTextRows[mCaretRow], mCaretColumn);
    }

    unsigned int TextBox::getCaretColumn() const
    {
        return mCaretColumn;
    }

    const std::string& TextBox::getTextRow(int row) const
    {
        return mTextRows[row];
    }

    void TextBox::setTextRow(int row, const std::string& text)
    {
        mTextRows[row] = text;

        if (mCaretRow == row)
        {
            setCaretColumn(mCaretColumn);
        }

        adjustSize();
    }

    unsigned int TextBox::getNumberOfRows() const
    {
        return mTextRows.size();
    }

    std::string TextBox::getText() const
    {
        if (mTextRows.size() == 0)
        {
            return std::string("");
        }

        int i;
        std::string text;

        for (i = 0; i < (int)mTextRows.size() - 1; ++i)
        {
            text = text + mTextRows[i] + "\n";
        }

        text = text + mTextRows[i];

        return text;
    }

    void TextBox::fontChanged()
    {
        adjustSize();
    }

    void TextBox::scrollToCaret()
    {
        Widget *par = getParent();
        if (par == NULL)
        {
            return;
        }

        ScrollArea* scrollArea = dynamic_cast<ScrollArea *>(par);
        if (scrollArea != NULL)
        {
            Rectangle scroll;
            scroll.x = getFont()->getWidth(mTextRows[mCaretRow].substr(0, mCaretColumn));
            scroll.y = getFont()->getHeight() * mCaretRow;
            scroll.width = 6;
            scroll.height = getFont()->getHeight() + 2; // add 2 for some extra space
            scrollArea->scrollToRectangle(scroll);
        }
    }

    void TextBox::setEditable(bool editable)
    {
        mEditable = editable;
    }

    bool TextBox::isEditable() const
    {
        return mEditable;
    }

    void TextBox::addRow(const std::string row)
    {
        mTextRows.push_back(row);
        adjustSize();
    }

    bool TextBox::isOpaque()
    {
        return mOpaque;
    }

    void TextBox::setOpaque(bool opaque)
    {
        mOpaque = opaque;
    }
}
