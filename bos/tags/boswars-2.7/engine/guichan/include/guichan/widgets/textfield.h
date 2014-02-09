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

#ifndef GCN_TEXTFIELD_HPP
#define GCN_TEXTFIELD_HPP

#include "guichan/platform.h"
#include "guichan/widget.h"

#include <string>

namespace gcn
{
    /**
     * A text field in which you can write or display a line of text.
     */
    class GCN_CORE_DECLSPEC TextField:
        public Widget,
        public MouseListener,
        public KeyListener
    {
    public:
        /**
         * Default constructor.
         */
        TextField();

        /**
         * Constructor. Initializes the textfield with a given string.
         *
         * @param text the initial text.
         */
        TextField(const std::string& text);

        /**
         * Sets the text.
         *
         * @param text the new text in the TextField.
         */
        virtual void setText(const std::string& text);

        /**
         * Gets the text.
         *
         * @return the text of the TextField.
         */
        virtual const std::string& getText() const;

        /**
         * Sets the maximum number of bytes that the user can type in
         * the TextField.  Because TextField uses the UTF-8 encoding,
         * each character may require several bytes.
         *
         * If the text in the field is already too long, this function
         * truncates it immediately.
         *
         * @param maxLengthBytes The new maximum length, in bytes.
         * Must not be negative.
         *
         * In addition to this UTF-8 byte limit, we could also
         * implement limits on the number of code points, the number
         * of UTF-16 code units, the number of grapheme clusters, or
         * the width of the string in a given font.  However, there
         * haven't been any use cases for those so far.
         *
         * @seealso #getMaxLengthBytes
         */
        void setMaxLengthBytes(int maxLengthBytes);

        /**
         * Gets the maximum number of bytes that the user can type in
         * the TextField.
         *
         * @return The current maximum length, in bytes.
         *
         * @seealso #setMaxLengthBytes
         */
        int getMaxLengthBytes() const;

        /**
         * Draws the caret (the little marker in the text that shows where the
         * letters you type will appear). Easily overloaded if you want to
         * change the style of the caret.
         *
         * @param graphics the Graphics object to draw with.
         * @param x the caret's x-position.
         */
        virtual void drawCaret(Graphics* graphics, int x);

        /**
         * Adjusts the size of the TextField to fit the font size. The
         * constructor taking a string uses this function to initialize the
         * size of the TextField.
         */
        virtual void adjustSize();

        /**
         * Adjusts the height of the text field to fit the font size. The
         * height of the TextField is initialized with this function by the
         * constructors.
         */
        virtual void adjustHeight();

        /**
         * Sets the caret position.
         *
         * @param position the caret position.
         */
        virtual void setCaretPosition(unsigned int position);

        /**
         * Gets the caret position.
         *
         * @return the caret position.
         */
        virtual unsigned int getCaretPosition() const;

		/// Gets the currently selected text
		virtual void getTextSelectionPositions(unsigned int* first, unsigned int* len);

        // Inherited from Widget

        virtual void fontChanged();

        virtual void draw(Graphics* graphics);

        virtual void drawBorder(Graphics* graphics);


        // Inherited from MouseListener

        virtual void mousePress(int x, int y, int button);

		virtual void mouseMotion(int x, int y);

        // Inherited from KeyListener

        virtual bool keyPress(const Key& key);

    protected:
        /**
         * Scrolls the text horizontally so that the caret shows if needed.
         */
        void fixScroll();

        /**
         * Truncates #mText to #mMaxLengthBytes, and #mCaretPosition
         * and #mSelectStart to the length of #mText.  Marks the
         * widget dirty if it changes anything.
         */
        void truncateToMaxLength();

        /**
         * Inserts a string at the caret, replacing the selection if any.
         *
         * This function also marks the widget dirty.  However, it
         * does not call #fixScroll because that is somewhat
         * expensive and should not be called redundantly.  Therefore,
         * the caller must do that.
         *
         * @param text The text to insert.
         */
        void insertAtCaret(const std::string& str);

        /**
         * Finds the last UTF-8 character boundary in the first
         * @a maxBytes bytes of @a str.
         *
         * @param str UTF-8 string.
         *
         * @param maxBytes How many bytes to consider at the beginning
         * of @a str.  Must not be negative.
         *
         * @return The position of the last character boundary.  Always
         * <= @a maxBytes.
         */
        static int UTF8LastCharacterBoundary(const std::string &str, int maxBytes);

        std::string mText;
        int mCaretPosition;
        int mXScroll;
		int mSelectStart;

        /**
         * The maximum length of #mText, in bytes.  Might be zero but
         * never negative.
         */
        int mMaxLengthBytes;
    };
}

#endif // end GCN_TEXTFIELD_HPP
