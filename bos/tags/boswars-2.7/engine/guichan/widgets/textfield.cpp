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

#include <limits>
#include "guichan/keyinput.h"
#include "guichan/mouseinput.h"
#include "guichan/widgets/textfield.h"
#include "guichan/exception.h"
#include "util.h"

namespace gcn
{
    TextField::TextField()
    {
        mCaretPosition = 0;
        mXScroll = 0;
        mSelectStart = 0;
        mMaxLengthBytes = std::numeric_limits<int>::max();

        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        adjustHeight();
        setBorderSize(1);    
    }
  
    TextField::TextField(const std::string& text)
    {
        mCaretPosition = 0;
        mXScroll = 0;
        mSelectStart = 0;
        mMaxLengthBytes = std::numeric_limits<int>::max();

        mText = text;
        // std::string::max_size() might exceed std::numeric_limits<int>::max().
        truncateToMaxLength();
        adjustSize();
        setBorderSize(1);
        
        setFocusable(true);    
  
        addMouseListener(this);
        addKeyListener(this);    
    }

    void TextField::setText(const std::string& text)
    {
        mText = text;
        truncateToMaxLength();
        mSelectStart = mCaretPosition;
        setDirty(true);
    }
  
    void TextField::draw(Graphics* graphics)
    {
        Font *font;
        int x, y;
        Color faceColor = getBackgroundColor();
        graphics->setColor(faceColor);
        graphics->fillRectangle(Rectangle(0, 0, getWidth(), getHeight()));
    
        if (hasFocus())
        {      
            drawCaret(graphics, getFont()->getWidth(mText.substr(0, mCaretPosition)) - mXScroll);
        }
    
        graphics->setColor(getForegroundColor());
        font = getFont();
        graphics->setFont(font);

        x = 1 - mXScroll;
        y = 1;

        if (mSelectStart != mCaretPosition)
        {
            unsigned int first;
            unsigned int len;
            int selX;
            int selW;
            std::string tmpStr;

            getTextSelectionPositions(&first, &len);

            tmpStr = std::string(mText.substr(0, first));
            selX = font->getWidth(tmpStr);

            tmpStr = std::string(mText.substr(first, len));
            selW = font->getWidth(tmpStr);

            graphics->setColor(Color(127, 127, 127));
            graphics->fillRectangle(Rectangle(x + selX, y, selW, font->getHeight()));
        }

        graphics->drawText(mText, x, y);    
    }

    void TextField::drawBorder(Graphics* graphics)
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
    
    void TextField::drawCaret(Graphics* graphics, int x)
    {
        graphics->setColor(getForegroundColor());
        graphics->drawLine(x, getHeight() - 2, x, 1);    
    }
  
    void TextField::mousePress(int x, int y, int button)
    {
        if (hasMouse() && button == MouseInput::LEFT)
        {
            mCaretPosition = getFont()->getStringIndexAt(mText, x + mXScroll);
            mSelectStart = mCaretPosition;
            fixScroll();
        }
        else if (hasMouse() && button == MouseInput::MIDDLE)
        {
            std::string str;
            if (GetClipboard(str) >= 0) {
                // GetClipboard ensures that the string does not
                // contain control characters.
                insertAtCaret(str);
                fixScroll();
            }
        }
    }

    void TextField::mouseMotion(int x, int y)
    {
        if (isDragged() && mClickButton == MouseInput::LEFT)
        {
            mCaretPosition = getFont()->getStringIndexAt(mText, x + mXScroll);
            setDirty(true);
        }
    }

    bool TextField::keyPress(const Key& key)
    {
        bool ret = false;
        unsigned int selFirst;
        unsigned int selLen;

        getTextSelectionPositions(&selFirst, &selLen);

        if (key.getValue() == Key::LEFT)
        {
            if (mCaretPosition > 0) {
                int newpos = UTF8GetPrev(mText, mCaretPosition);
                if (newpos < 0) {
                    throw GCN_EXCEPTION("Invalid UTF8.");
                }
                mCaretPosition = newpos;
            }
            if (!key.isShiftPressed()) {
                mSelectStart = mCaretPosition;
            }

            ret = true;
        }

        else if (key.getValue() == Key::RIGHT)
        {
            if (mCaretPosition < (int)mText.size()) {
                int newpos = UTF8GetNext(mText, mCaretPosition);
                if (newpos > (int)mText.size()) {
                    throw GCN_EXCEPTION("Invalid UTF8.");
                }
                mCaretPosition = newpos;
            }
            if (!key.isShiftPressed()) {
                mSelectStart = mCaretPosition;
            }

            ret = true;
        }

        else if (key.getValue() == Key::DELETE )
        {
            if (selLen > 0) {
                mText.erase(selFirst, selLen);
                mCaretPosition = selFirst;
                mSelectStart = selFirst;
            } else if (mCaretPosition < (int)mText.size()) {
                int newpos = UTF8GetNext(mText, mCaretPosition);
                if (newpos > (int)mText.size()) {
                    throw GCN_EXCEPTION("Invalid UTF8.");
                }
                mText.erase(mCaretPosition, newpos - mCaretPosition);
            }

            ret = true;
        }

        else if (key.getValue() == Key::BACKSPACE || key.getValue() == 'h' - 'a' + 1)
        {
            if (selLen > 0) {
                mText.erase(selFirst, selLen);
                mCaretPosition = selFirst;
                mSelectStart = selFirst;
            } else if (mCaretPosition > 0) {
                int newpos = UTF8GetPrev(mText, mCaretPosition);
                if (newpos < 0) {
                    throw GCN_EXCEPTION("Invalid UTF8.");
                }
                mText.erase(newpos, mCaretPosition - newpos);
                mCaretPosition = newpos;
                mSelectStart = newpos;
            }

            ret = true;
        }

        else if (key.getValue() == Key::ENTER)
        {
            generateAction();
            ret = true;
        }

        else if (key.getValue() == Key::HOME || key.getValue() == 'a' - 'a' + 1) // ctrl-a
        {
            mCaretPosition = 0;
            if (!key.isShiftPressed()) {
                mSelectStart = mCaretPosition;
            }

            ret = true;
        }    

        else if (key.getValue() == Key::END || key.getValue() == 'e' - 'a' + 1)  //ctrl-e
        {
            mCaretPosition = mText.size();
            if (!key.isShiftPressed()) {
                mSelectStart = mCaretPosition;
            }

            ret = true;
        }    

        else if (key.getValue() == 'u' - 'a' + 1) // ctrl-u
        {
            setText("");
            ret = true;
        }

        else if (key.getValue() == 'v' - 'a' + 1) // ctrl-v
        {
            std::string str;

            if (GetClipboard(str) >= 0) {
                // GetClipboard ensures that the string does not
                // contain control characters.
                insertAtCaret(str);
            }

            // Even if GetClipboard failed, we did recognize the key
            // and the caller should not treat it as a hot key.
            ret = true;
        }

        else if (key.isCharacter())
        {
            insertAtCaret(key.toString());
            ret = true;
        }

        fixScroll();
        setDirty(true);
        return ret;
    }

    void TextField::adjustSize()
    {
        setWidth(getFont()->getWidth(mText) + 4);
        adjustHeight();

        fixScroll();    
    }
  
    void TextField::adjustHeight()
    {
        setHeight(getFont()->getHeight() + 2);    
    }

    void TextField::fixScroll()
    {
        if (hasFocus())
        {
            int caretX = getFont()->getWidth(mText.substr(0, mCaretPosition));

            if (caretX - mXScroll > getWidth() - 4)
            {
                mXScroll = caretX - getWidth() + 4;
            }
            else if (caretX - mXScroll < getFont()->getWidth(" "))
            {
                mXScroll = caretX - getFont()->getWidth(" ");
        
                if (mXScroll < 0)
                {
                    mXScroll = 0;
                }
            }
        }
    }

    void TextField::setCaretPosition(unsigned int position)
    {
        if (position > mText.size())
        {
            mCaretPosition = mText.size();
        }
        else
        {    
            mCaretPosition = position;
        }

        fixScroll();    
    }

    unsigned int TextField::getCaretPosition() const
    {
        return mCaretPosition;    
    }

    void TextField::getTextSelectionPositions(unsigned int* first, unsigned int* len)
    {
        if (mCaretPosition < mSelectStart)
        {
            *first = mCaretPosition;
            *len = mSelectStart - mCaretPosition;
        }
        else
        {
            *first = mSelectStart;
            *len = mCaretPosition - mSelectStart;
        }
    }

    const std::string& TextField::getText() const
    {
        return mText;    
    }
  
    void TextField::fontChanged()
    {
        fixScroll();
    }

    int TextField::getMaxLengthBytes() const
    {
        return mMaxLengthBytes;
    }

    void TextField::setMaxLengthBytes(int maxLengthBytes)
    {
        if (maxLengthBytes < 0)
        {
            maxLengthBytes = 0;
        }

        mMaxLengthBytes = maxLengthBytes;
        truncateToMaxLength();
    }

    void TextField::truncateToMaxLength()
    {
        bool changedSomething = false;

        // Because we never let mMaxLengthBytes become negative,
        // the following static_cast cannot wrap around.
        if (mText.size() > static_cast<unsigned int>(mMaxLengthBytes))
        {
            int newLength = UTF8LastCharacterBoundary(mText, mMaxLengthBytes);
            mText.resize(newLength);
            changedSomething = true;
        }

        // Because of mMaxLengthBytes, the following static_casts
        // cannot overflow.
        if (mCaretPosition > static_cast<int>(mText.size()))
        {
            mCaretPosition = mText.size();
            changedSomething = true;
        }
        if (mSelectStart > static_cast<int>(mText.size()))
        {
            mSelectStart = mText.size();
            changedSomething = true;
        }

        if (changedSomething)
        {
            fixScroll();
            setDirty(true);
        }
    }

    int TextField::UTF8LastCharacterBoundary(const std::string &str,
                                             int maxBytes)
    {
        if (maxBytes < 0)
        {
            // A bug in the caller.
            maxBytes = 0;
        }

        // The following static_cast cannot wrap around because
        // maxBytes cannot be negative at this point.
        if (str.size() < static_cast<unsigned int>(maxBytes))
        {
            // The following static_cast cannot overflow because
            // str.size() is less than int maxBytes.
            maxBytes = static_cast<int>(str.size());
        }

        if (maxBytes == 0)
        {
            // There is no space for any character.
            return 0;
        }

        // The following would be easy to implement as:
        //   return UTF8GetPrev(str, maxBytes + 1);
        // but maxBytes + 1 might overflow, so don't do that.

        // Find the previous character boundary.
        int prev = UTF8GetPrev(str, maxBytes);
        // Assert(prev < maxBytes);

        // If maxBytes already is at a character
        // boundary, then that character will fit.
        int next = UTF8GetNext(str, prev);
        if (next == maxBytes)
        {
            return maxBytes;
        }
        else
        {
            return prev;
        }
    }

    void TextField::insertAtCaret(const std::string& str)
    {
        int bytesFree = mMaxLengthBytes - mText.size();
        if (bytesFree < 0)
        {
            bytesFree = 0;
        }

        int bytesToCopy;
        if (str.size() > static_cast<unsigned int>(bytesFree))
        {
            bytesToCopy = UTF8LastCharacterBoundary(str, bytesFree);
        }
        else
        {
            bytesToCopy = static_cast<int>(str.size());
        }

        unsigned int selFirst;
        unsigned int selLen;
        getTextSelectionPositions(&selFirst, &selLen);
        if (selLen > 0) {
            mText.erase(selFirst, selLen);
            mCaretPosition = selFirst;
            mSelectStart = selFirst;
        }

        mText.insert(mCaretPosition, str, 0, bytesToCopy);
        mCaretPosition += bytesToCopy;
        mSelectStart = mCaretPosition;
        setDirty(true);
    }
}
