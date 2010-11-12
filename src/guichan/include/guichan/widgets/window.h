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

#ifndef GCN_WINDOW_HPP
#define GCN_WINDOW_HPP

#include <string>

#include "guichan/platform.h"
#include "guichan/basiccontainer.h"

namespace gcn
{
    /**
     * A movable window which can conatin another Widget.
     */
    class GCN_CORE_DECLSPEC Window : public BasicContainer,
                                     public MouseListener
    {
    public:
        /**
         * Constructor.
         */
        Window();

        /**
         * Constructor.
         *
         * @param caption the Window caption.
         */
        Window(const std::string& caption);

        /**
         * Constructor.
         *
         * @param content the content Widget.
         * @param caption the Window caption.
         */
        Window(Widget* content, const std::string& caption = "");

        /**
         * Destructor.
         */
        virtual ~Window();

        /**
         * Sets the Window caption.
         *
         * @param caption the Window caption.
         */
        virtual void setCaption(const std::string& caption);

        /**
         * Gets the Window caption.
         *
         * @return the Window caption.
         */
        virtual const std::string& getCaption() const;

        /**
         * Sets the alignment for the caption.
         *
         * @param alignment Graphics::LEFT, Graphics::CENTER or Graphics::RIGHT.
         */
        virtual void setAlignment(unsigned int alignment);

        /**
         * Gets the alignment for the caption.
         *
         * @return alignment of caption.
         */
        virtual unsigned int getAlignment() const;

        /**
         * Sets the content Widget.
         *
         * @param widget the contant Widget.
         */
        virtual void setContent(Widget* widget);

        /**
         * Gets the content Widget.
         *
         * @return the contant Widget.
         */
        virtual Widget* getContent() const;

        /**
         * Sets the padding of the window which is the distance between the
         * window border and the content.
         *
         * @param padding the padding value.
         */
        virtual void setPadding(unsigned int padding);

        /**
         * Gets the padding.
         *
         * @return the padding value.
         */
        virtual unsigned int getPadding() const;

        /**
         * Sets the title bar height.
         *
         * @param height the title height value.
         */
        virtual void setTitleBarHeight(unsigned int height);

        /**
         * Gets the title bar height.
         *
         * @return the title bar height.
         */
        virtual unsigned int getTitleBarHeight();

        /**
         * Sets the Window to be moveble.
         *
         * @param movable true or false.
         */
        virtual void setMovable(bool movable);

        /**
         * Check if the window is movable.
         *
         * @return true or false.
         */
        virtual bool isMovable() const;

        /**
         * Resizes the window to fit the content.
         */
        virtual void resizeToContent();

        /**
         * Sets the Window to be opaque. If it's not opaque, the content area
         * will not be filled with a color.
         *
         * @param opaque true or false.
         */
        virtual void setOpaque(bool opaque);

        /**
         * Checks if the Window is opaque.
         *
         * @return true or false.
         */
        virtual bool isOpaque();

        /**
         * Draws the content of the Window. This functions uses the
         * getContentDimension to determin where to draw the content.
         *
         * @param graphics a Graphics object to draw with.
         */
        virtual void drawContent(Graphics* graphics);


        // Inherited from BasicContainer

        virtual void moveToTop(Widget* widget);

        virtual void moveToBottom(Widget* widget);

        virtual void getDrawSize(int& width, int& height, Widget* widget);

        virtual void _announceDeath(Widget *widget);


        // Inherited from Widget

        virtual void draw(Graphics* graphics);

        virtual void drawBorder(Graphics* graphics);

        virtual void logic();

        virtual void _mouseInputMessage(const MouseInput &mouseInput);

        virtual void _mouseOutMessage();

        virtual void _setFocusHandler(FocusHandler* focusHandler);


        // Inherited from MouseListener

        virtual void mousePress(int x, int y, int button);

        virtual void mouseRelease(int x, int y, int button);

        virtual void mouseMotion(int x, int y);

        virtual void setDirty(bool dirty);
        virtual bool getDirty() const;

    protected:
        /**
         * Moves the content to the top left corner of the window,
         * uses getContentDimension to get the offset
         */
        virtual void repositionContent();

        /**
         * Gets the area in the window that the content occupies.
         */
        virtual Rectangle getContentDimension();

        std::string mCaption;
        unsigned int mAlignment;
        Widget* mContent;
        unsigned int mPadding;
        unsigned int mTitleBarHeight;
        bool mMouseDrag;
        int mMouseXOffset;
        int mMouseYOffset;
        bool mMovable;
        bool mOpaque;
    };
}

#endif // end GCN_WINDOW_HPP
