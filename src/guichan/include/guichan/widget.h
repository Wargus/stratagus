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

#ifndef GCN_WIDGET_HPP
#define GCN_WIDGET_HPP

#include <list>
#include <string>

#include "guichan/actionlistener.h"
#include "guichan/color.h"
#include "guichan/defaultfont.h"
#include "guichan/font.h"
#include "guichan/graphics.h"
#include "guichan/keyinput.h"
#include "guichan/keylistener.h"
#include "guichan/mouseinput.h"
#include "guichan/mouselistener.h"
#include "guichan/rectangle.h"
#include "guichan/platform.h"

namespace gcn
{
    /*
     * Exists for the widget class to compile.
     */
    class FocusHandler;

    /*
     * Exists for the widget class to compile.
     */
    class BasicContainer;

    /**
     * Widget base class. Contains basic widget functions every widget should
     * have. Widgets should inherit from this class and implements it's
     * functions.
     *
     * NOTE: Functions begining with underscore "_" should not
     *       be overloaded unless you know what you are doing.
     */
    class GCN_CORE_DECLSPEC Widget
    {
    public:
        /**
         * Constructor. Resets member variables. Noteable, a widget is not
         * focusable as default, therefore, Widgets that are supposed to be
         * focusable should overide this default in their own constructor.
         */
        Widget();

        /**
         * Default destructor.
         */
        virtual ~Widget();

        /**
         * Draws the Widget. It is called by the parent widget when it is time
         * for the Widget to draw itself. The graphics object is set up so
         * that all drawing is relative to the Widget, i.e coordinate (0,0) is
         * the top-left corner of the Widget. It is not possible to draw
         * outside of a Widgets dimension.
         *
         * @param graphics a Graphics object to draw with.
         */
        virtual void draw(Graphics* graphics) = 0;

        /**
         * Draws a the Widget border. A border is drawn around a Widget.
         * The width and height of the border is therefore the Widgets
         * height+2*bordersize. Think of a painting that has a certain size,
         * the border surrounds the painting.
         *
         * @param graphics a Graphics object to draw with.
         */
        virtual void drawBorder(Graphics*) { }

        /**
         * Called for all Widgets in the gui each time Gui::logic is called.
         * You can do logic stuff here like playing an animation.
         *
         * @see Gui
         */
        virtual void logic() { }

        /**
         * Gets the Widget parent container.
         *
         * @return the Widget parent container. Returns NULL if the Widget
         *         has no parent.
         */
        virtual BasicContainer* getParent() const;

        /**
         * Sets the width of the Widget in pixels.
         *
         * @param width the Widget width in pixels.
         */
        virtual void setWidth(int width);

        /**
         * Gets the width of the Widget in pixels.
         *
         * @return the Widget with in pixels.
         */
        virtual int getWidth() const;

        /**
         * Sets the height of the Widget in pixels.
         *
         * @param height the Widget height in pixels.
         */
        virtual void setHeight(int height);

        /**
         * Gets the height of the Widget in pixels.
         *
         * @return the Widget height in pixels.
         */
        virtual int getHeight() const;

        /**
         * Sets the size of the Widget.
         *
         * @param width the width.
         * @param height the height.
         */
        virtual void setSize(int width, int height);

        /**
         * Set the Widget x coordinate. It is relateive to it's parent.
         *
         * @param x the Widget x coordinate.
         */
        virtual void setX(int x);

        /**
         * Gets the Widget x coordinate. It is relative to it's parent.
         *
         * @return the Widget x coordinate.
         */
        virtual int getX() const;

        /**
         * Set the Widget y coordinate. It is relative to it's parent.
         *
         * @param y the Widget y coordinate.
         */
        virtual void setY(int y);

        /**
         * Gets the Widget y coordinate. It is relative to it's parent.
         *
         * @return the Widget y coordinate.
         */
        virtual int getY() const;

        /**
         * Sets the Widget position. It is relative to it's parent.
         *
         * @param x the Widget x coordinate.
         * @param y the Widgets y coordinate.
         */
        virtual void setPosition(int x, int y);

        /**
         * Sets the dimension of the Widget. It is relative to it's parent.
         *
         * @param dimension the Widget dimension.
         */
        virtual void setDimension(const Rectangle& dimension);

        /**
         * Sets the size of the border, or the width if you so like. The size
         * is the number of pixels that the border extends outside the Widget.
         * Border size = 0 means no border.
         *
         * @param borderSize the size of the border.
         * @see drawBorder
        */
        virtual void setBorderSize(unsigned int borderSize);

        /**
         * Gets the size of the border, or the width if you so like. The size
         * is the number of pixels that the border extends outside the Widget.
         * Border size = 0 means no border.
         *
         * @return the size of the border.
         * @see drawBorder
         */
        virtual unsigned int getBorderSize() const;

        /**
         * Gets the dimension of the Widget. It is relative to it's parent.
         *
         * @return the widget dimension.
         */
        virtual const Rectangle& getDimension() const;

        /**
         * Sets a Widgets focusability.
         *
         * @param focusable true if the Widget should be focusable.
         */
        virtual void setFocusable(bool focusable);

        /**
         * Checks whether the Widget is focusable.
         *
         * @return true if the widget is focusable.
         */
        virtual bool isFocusable() const;

        /**
         * Checks if the Widget has focus.
         *
         * @return true if the widget currently has focus.
         */
        virtual bool hasFocus() const;

        /**
         * Sets the Widget to be disabled or enabled. A disabled
         * Widget will never recieve mouse or key input.
         *
         * @param enabled true if Widget is enabled.
         */
        virtual void setEnabled(bool enabled);

        /**
         * Checks if a Widget is disabled or not.
         *
         * @return true if the Widget should be enabled.
         */
        virtual bool isEnabled() const;

        /**
         * Called if the Widget loses focus.
         */
        virtual void lostFocus() { setDirty(true); };

        /**
         * Called if the Widget recieves focus.
         */
        virtual void gotFocus() { setDirty(true); };

        /**
         * Checks if the Widget has the mouse.
         *
         * @return true if the Widget has the mouse.
         */
        virtual bool hasMouse() const;

        /**
         * Sets the Widget to be visible.
         *
         * @param visible true if the Widget should be visiable.
         */
        virtual void setVisible(bool visible);

        /**
         * Checks if the Widget is visible.
         *
         * @return true if the Widget is visible.
         */
        virtual bool isVisible() const;

        /**
         * Sets the base color. The base color is the background
         * color for many Widgets like the Button and Contianer Widgets.
         *
         * @param color the baseground Color.
         */
        virtual void setBaseColor(const Color& color);

        /**
         * Gets the base color.
         *
         * @return the foreground Color.
         */
        virtual const Color& getBaseColor() const;

        /**
         * Sets the foreground color.
         *
         * @param color the foreground Color.
         */
        virtual void setForegroundColor(const Color& color);

        /**
         * Gets the foreground color.
         *
         * @return the foreground Color.
         */
        virtual const Color& getForegroundColor() const;

        /**
         * Sets the background color.
         *
         * @param color the background Color.
         */
        virtual void setBackgroundColor(const Color& color);

        /**
         * Gets the background color.
         *
         * @return the background Color.
         */
        virtual const Color& getBackgroundColor() const;

        /**
         * Sets the disabled color.
         *
         * @param color the disabled Color.
         */
        virtual void setDisabledColor(const Color& color);

        /**
         * Gets the disabled color.
         *
         * @return the disabled Color.
         */
        virtual const Color& getDisabledColor() const;

        /**
         * Called when a Widget recieves a MouseInput.
         *
         * WARNING: This function is used internally to handle all mouse
         *          messages. Don't call or overload it unless you know what
         *          you are doing.
         *
         * @param mouseInput the MouseInput message.
         */
        virtual void _mouseInputMessage(const MouseInput& mouseInput);

        /**
         * Called when a Widget recieves a KeyInput.
         *
         * WARNING: This function is used internally to handle all key
         *          messages. Don't call or overload it unless you know what
         *          you are doing.
         *
         * @param keyInput the KeyInput message.
         */
        virtual bool _keyInputMessage(const KeyInput& keyInput);

        /**
         * Called when a Widget's hot key is pressed
         */
        virtual void hotKeyPress() {}

        /**
         * Called when a Widget's hot key is released
         */
        virtual void hotKeyRelease() {}

        /**
         * Called when the mouse enters the Widget area.
         *
         * WARNING: This function is used internally to handle mouse in
         *          messages. Don't call or overload this function unless
         *          you know what you are doing.
         */
        virtual void _mouseInMessage();

        /**
         * Called when the mouse leaves the Widget area.
         *
         * WARNING: This function is used internally be to handle mouse
         *          out messages. Don't call or overload this function
         *          unless you know what you are doing.
         */
        virtual void _mouseOutMessage();

        /**
         * Requests focus for the Widget. A Widget will only recieve focus
         * if it is focusable.
         */
        virtual void requestFocus();

        /**
         * Requests a move to the top in the parent Widget.
         */
        virtual void requestMoveToTop();

        /**
         * Requests a move to the bottom in the parent Widget.
         */
        virtual void requestMoveToBottom();

        /**
         * Sets the FocusHandler to be used.
         *
         * WARNING: This function is used internally and should not
         *          be called or overloaded unless you know what you
         *          are doing.
         *
         * @param focusHandler the FocusHandler to use.
         */
        virtual void _setFocusHandler(FocusHandler* focusHandler);

        /**
         * Gets the FocusHandler used.
         *
         * WARNING: This function is used internally and should not
         *          be called or overloaded unless you know what you
         *          are doing.
         *
         * @return the FocusHandler used.
         */
        virtual FocusHandler* _getFocusHandler();

        /**
         * Adds an ActionListener to the Widget. When an action is triggered
         * by the Widget, the action function in all the Widget's
         * ActionListeners will be called.
         *
         * @param actionListener the ActionListener to add.
         */
        virtual void addActionListener(ActionListener* actionListener);

        /**
         * Removes an added ActionListener from the Widget.
         *
         * @param actionListener the ActionListener to remove.
         */
        virtual    void removeActionListener(ActionListener* actionListener);

        /**
         * Adds a MouseListener to the Widget. When a mouse message is
         * recieved, it will be sent to the Widget's MouseListeners.
         *
         * @param mouseListener the MouseListener to add.
         */
        virtual    void addMouseListener(MouseListener* mouseListener);

        /**
         * Removes an added MouseListener from the Widget.
         *
         * @param mouseListener the MouseListener to remove.
         */
        virtual    void removeMouseListener(MouseListener* mouseListener);

        /**
         * Adds a KeyListener to the Widget. When a key message is recieved,
         * it will be sent to the Widget's KeyListeners.
         *
         * @param keyListener the KeyListener to add.
         */
        virtual    void addKeyListener(KeyListener* keyListener);

        /**
         * Removes an added KeyListener from the Widget.
         *
         * @param keyListener the KeyListener to remove.
         */
        virtual    void removeKeyListener(KeyListener* keyListener);

        /**
         * Sets the event identifier of the Widget. The event identifier is
         * used to be able to identify which Widget generated an action when
         * an action has occured.
         *
         * NOTE: An event identifier should not be used to identify a certain
         *       Widget but rather a certain event in your application. Several
         *       Widgets can have the same event identifer.
         *
         * @param eventId the event identifier.
         */
        virtual    void setEventId(const std::string& eventId);

        /**
         * Gets the event identifier.
         *
         * @return the event identifier.
         */
        virtual const std::string& getEventId() const;

        /**
         * Gets the absolute position on the screen for the Widget.
         *
         * @param x absolute x coordinate will be stored in this parameter.
         * @param y absolute y coordinate will be stored in this parameter.
         */
        virtual    void getAbsolutePosition(int& x, int& y) const;

        /**
         * Sets the parent of the Widget. A parent must be a BasicContainer.
         *
         * WARNING: This function is used internally and should not
         *          be called or overloaded unless you know what you
         *          are doing.
         *
         * @param parent the parent BasicContainer..
         */
        virtual    void _setParent(BasicContainer* parent);

        /**
         * Gets the font used. If no font has been set, the global font will
         * be returned instead. If no global font has been set, the default
         * font will be returend.
         * ugly default.
         *
         * @return the used Font.
         */
        Font *getFont() const;

        /**
         * Sets the global font to be used by default for all Widgets.
         *
         * @param font the global Font.
         */
        static void setGlobalFont(Font* font);

        /**
         * Sets the font. If font is NULL, the global font will be used.
         *
         * @param font the Font.
         */
        virtual void setFont(Font* font);

        /**
         * Called when the font has changed. If the change is global,
         * this function will only be called if the Widget don't have a
         * font already set.
         */
        virtual void fontChanged() { }

        /**
         * Get the hot key
         */
        virtual int getHotKey() const { return mHotKey; }

        /**
         * Set the hot key
         */
        virtual void setHotKey(const int key);
        virtual void setHotKey(const char *key);

        /**
         * Checks whether a Widget exists or not, that is if it still exists
         * an instance of the object.
         *
         * @param widget the Widget to check.
         */
        static bool widgetExists(const Widget* widget);

        /**
         * Check if tab in is enabled. Tab in means that you can set focus
         * to this Widget by pressing the tab button. If tab in is disabled
         * then the FocusHandler will skip this widget and focus the next
         * in its focus order.
         *
         * @return true if tab in is enabled.
         */
        virtual bool isTabInEnabled() const;

        /**
         * Sets tab in enabled. Tab in means that you can set focus
         * to this Widget by pressing the tab button. If tab in is disabled
         * then the FocusHandler will skip this widget and focus the next
         * in its focus order.
         *
         * @param enabled true if tab in should be enabled.
         */
        virtual void setTabInEnabled(bool enabled);

        /**
         * Checks if tab out is enabled. Tab out means that you can lose
         * focus to this Widget by pressing the tab button. If tab out is
         * disabled then the FocusHandler ignores tabbing and focus will
         * stay with this Widget.
         *
         * @return true if tab out is enabled.
         */
        virtual bool isTabOutEnabled() const;

        /**
         * Sets tab out enabled. Tab out means that you can lose
         * focus to this Widget by pressing the tab button. If tab out is
         * disabled then the FocusHandler ignores tabbing and focus will
         * stay with this Widget.
         *
         * @param enabled true if tab out should be enabled.
         */
        virtual void setTabOutEnabled(bool enabled);

        /**
         * Checks if the Widget is dragged. Dragged means that the mouse
         * button has been pressed down over the Widget and the mouse has
         * been moved.
         *
         * @return true if the widget is dragged.
         */
        virtual bool isDragged() const;

        /**
         * Requests modal focus. When a widget has modal focus, only that
         * Widget and it's children may recieve input. If some other Widget
         * already has modal focus, an exception will be thrown.
         *
         * @throws Exception if another Widget already has modal focus.
         */
        virtual void requestModalFocus();

        /**
         * Releases modal focus. Modal focus will only be released if the
         * Widget has the modal focus.
         */
        virtual void releaseModalFocus();

        /**
         * Checks if the Widget or it's parent has modal focus.
         */
        virtual bool hasModalFocus() const;

        virtual void setDirty(bool dirty);
        virtual bool getDirty() const;


    protected:
        /**
         * Generates an action to the Widget's ActionListeners.
         */
        void generateAction();

        typedef std::list<MouseListener*> MouseListenerList;
        MouseListenerList mMouseListeners;
        typedef MouseListenerList::iterator MouseListenerIterator;

        typedef std::list<KeyListener*> KeyListenerList;
        KeyListenerList mKeyListeners;
        typedef KeyListenerList::iterator KeyListenerIterator;

        typedef std::list<ActionListener*> ActionListenerList;
        ActionListenerList mActionListeners;
        typedef ActionListenerList::iterator ActionListenerIterator;

        Color mForegroundColor;
        Color mBackgroundColor;
        Color mBaseColor;
        Color mDisabledColor;
        FocusHandler* mFocusHandler;
        BasicContainer* mParent;
        Rectangle mDimension;
        unsigned int mBorderSize;
        std::string mEventId;
        int mClickTimeStamp;
        int mClickCount;
        int mClickButton;
        bool mHasMouse;
        bool mFocusable;
        bool mVisible;
        bool mTabIn;
        bool mTabOut;
        bool mEnabled;

        Font* mCurrentFont;
        int mHotKey;
        static DefaultFont mDefaultFont;
        static Font* mGlobalFont;
        static std::list<Widget*> mWidgets;
        bool mDirty;
    };
}

#endif // end GCN_WIDGET_HPP

/*
 * yakslem  - "I have a really great idea! Why not have an
 *             interesting and funny quote or story at the
 *             end of every source file."
 * finalman - "Yeah - good idea! .... do you know any funny
 *             quotes?"
 * yakslem  - "Well.. I guess not. I just thought it would be
 *             fun to tell funny quotes."
 * finalman - "That's not a very funny quote. But i believe
 *             pointless quotes are funny in their own pointless
 *             way."
 * yakslem  - "...eh...ok..."
 */
