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

#ifndef GCN_CONTAINER_HPP
#define GCN_CONTAINER_HPP

#include <list>

#include "guichan/basiccontainer.h"
#include "guichan/graphics.h"
#include "guichan/platform.h"

namespace gcn
{
    /**
     * A container able to contain other Widgets. It is in other words a
     * Widget that holds other Widgets. A Widgets position in the container is
     * always relativ to the Container itself, not the screen. Using a
     * Container as the top Widget in Gui is the only way to use more then one
     * Widget in your Gui.
     */
    class GCN_CORE_DECLSPEC Container: public BasicContainer
    {
    public:

        /**
         * Constructor. A container is opauqe as default.
         *
         * @see setOpaque, isOpaque
         */
        Container();

        /**
         * Destructor.
         */
        virtual ~Container();

        /**
         * Sets whether the background should be drawn or not. If the
         * Container is not opaque it will be completely transparent.
         *
         * NOTE: This is not the same as to set visibility. A nonvisible
         *       Container will not draw it's content.
         *
         * @param opaque true if the Container should be opaque.
         * @see isOpaque
         */
        virtual void setOpaque(bool opaque);

        /**
         * Checks if the Container is opaque.
         *
         * @return true if the Container is opaque.
         * @see setOpaque
         */
        virtual bool isOpaque() const;

        /**
         * Adds a Widget to the Container.
         *
         * @param widget the Widget to add.
         * @see remove
         */
        virtual void add(Widget* widget);

        /**
         * Adds a Widget to the container and also specifices it's postion
         *
         * @param widget the Widget to add.
         * @param x the x coordinat for the Widget in the Container.
         * @param y the y coordinat for the Widget in the Container.
         * @see remove
         */
        virtual void add(Widget* widget, int x, int y);

        /**
         * Removes a Widget from the Container.
         *
         * @param widget the Widget to remove.
         * @throws Exception when the Widget has not been added to the
         *                   Container.
         * @see add, clear
         */
        virtual void remove(Widget* widget);

        /**
         * Clears the Container of all widgets.
         *
         * @see add, remove
         */
        virtual void clear();


        // Inherited from Widget

        virtual void draw(Graphics* graphics);

        virtual void drawBorder(Graphics* graphics);

        virtual void logic();

        virtual void _setFocusHandler(FocusHandler* focusHandler);

        virtual void _mouseInputMessage(const MouseInput &mouseInput);

        virtual void _mouseOutMessage();


        // Inherited from BasicContainer

        virtual void moveToTop(Widget* widget);

        virtual void moveToBottom(Widget* widget);

        virtual void _announceDeath(Widget *widget);

        virtual void getDrawSize(int& width, int& height, Widget* widget);

        virtual void setDirty(bool dirty);
        virtual bool getDirty() const;

    protected:

        /**
         * Draws all children of the Container. The Widgets will bedrawn in
         * the order the Widgets were added to the Container.
         *
         * @param graphics the Graphics object to draw with.
         */
        virtual void drawChildren(Graphics* graphics);

        /**
         * Calls the logic function for all children of Container. The Widgets
         * logic function will be called in the order the Widgets were added
         * to the Container.
         */
        virtual void logicChildren();

        Widget* mWidgetWithMouse;
        typedef std::list<Widget*> WidgetList;
        typedef WidgetList::iterator WidgetIterator;
        typedef WidgetList::const_iterator WidgetConstIterator;
        WidgetList mWidgets;
        bool mOpaque;
    };
}

#endif // end GCN_CONTAINER_HPP
