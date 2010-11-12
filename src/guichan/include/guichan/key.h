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

#ifndef GCN_KEY_HPP
#define GCN_KEY_HPP

#include <string>

#include "guichan/platform.h"

namespace gcn
{
    /**
     * Represents a key or a character.
     */
    class GCN_CORE_DECLSPEC Key
    {
    public:
        /**
         * Constructor.
         */
        Key();

        /**
         * Constructor.
         *
         * @param value the ascii or enum value for the key.
         */
        Key(int value);

        /**
         * Checks whether a key is a character.
         *
         * @return true if the key is a letter, number or whitespace.
         */
        bool isCharacter() const;

        /**
         * Checks whether a key is a number.
         *
         * @return true if the key is a number (0-9).
         */
        bool isNumber() const;

        /**
         * Checks whether a key is a letter.
         *
         * @return true if the key is a letter (a-z,A-Z).
         */
        bool isLetter() const;

        /**
         * Checks whether shift is pressed.
         *
         * @return true if shift was pressed at the same time as the key.
         */
        bool isShiftPressed() const;

        /**
         * Sets the shift pressed flag.
         *
         * @param pressed the shift flag value.
         */
        void setShiftPressed(bool pressed);

        /**
         * Checks whether control is pressed.
         * @return true if control was pressed at the same time as the key.
         */
        bool isControlPressed() const;

        /**
         * Sets the control pressed flag.
         *
         * @param pressed the control flag value.
         */
        void setControlPressed(bool pressed);

        /**
         * Checks whether alt is pressed.
         *
         * @return true if alt was pressed at the same time as the key.
         */
        bool isAltPressed() const;

        /**
         * Sets the alt pressed flag.
         *
         * @param pressed the alt flag value.
         */
        void setAltPressed(bool pressed);

        /**
         * Checks whether meta is pressed.
         *
         * @return true if meta was pressed at the same time as the key.
         */
        bool isMetaPressed() const;

        /**
         * Sets the meta pressed flag.
         *
         * @param pressed the meta flag value.
         */
        void setMetaPressed(bool pressed);

        /**
         * Checks whether the key was pressed at the numeric pad.
         *
         * @return true if key pressed at the numeric pad.
         */
        bool isNumericPad() const;

        /**
         * Sets the numeric pad flag.
         *
         * @param numpad the numeric pad flag value.
         */
        void setNumericPad(bool numpad);

        /**
         * Gets the value of the key. If an ascii value exists it will be
         * returned. Otherwise an enum value will be returned.
         *
         * @return the value of the key.
         */
        int getValue() const;

        /**
         * Sets the value of the key. An ascii value or an enum value.
         *
         * @param value the key value.
         */
        void setValue(int value);

        /**
         * Convert to a UTF8 string
         */
        std::string toString() const;

        /**
         * An enum with key values.
         */
        enum
        {
            K_SPACE              = ' ',
            K_TAB                = '\t',
            K_ENTER              = '\n',
            K_LEFT_ALT           = 1000,
            K_RIGHT_ALT,
            K_LEFT_SHIFT,
            K_RIGHT_SHIFT,
            K_LEFT_CONTROL,
            K_RIGHT_CONTROL,
            K_LEFT_META,
            K_RIGHT_META,
            K_LEFT_SUPER,
            K_RIGHT_SUPER,
            K_INSERT,
            K_HOME,
            K_PAGE_UP,
            K_DELETE,
            K_END,
            K_PAGE_DOWN,
            K_ESCAPE,
            K_CAPS_LOCK,
            K_BACKSPACE,
            K_F1,
            K_F2,
            K_F3,
            K_F4,
            K_F5,
            K_F6,
            K_F7,
            K_F8,
            K_F9,
            K_F10,
            K_F11,
            K_F12,
            K_F13,
            K_F14,
            K_F15,
            K_PRINT_SCREEN,
            K_SCROLL_LOCK,
            K_PAUSE,
            K_NUM_LOCK,
            K_ALT_GR,
            K_LEFT,
            K_RIGHT,
            K_UP,
            K_DOWN
        };

    protected:
        int mValue;
        bool mShiftPressed;
        bool mControlPressed;
        bool mAltPressed;
        bool mMetaPressed;
        bool mNumericPad;
    };
}

#endif // end GCN_KEY_HPP
