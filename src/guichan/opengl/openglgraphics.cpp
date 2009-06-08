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
 * Olof Naess�n a.k.a jansem/yakslem                _asww7!uY`>  )\a//
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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __amigaos4__
#include <mgl/gl.h>
#define glVertex3i glVertex3f
#else
#include <GL/gl.h>
#endif

#include <string>

#include "guichan/opengl/openglgraphics.h"
#include "guichan/exception.h"


namespace gcn
{
    OpenGLGraphics::OpenGLGraphics()
    {
        setTargetPlane(640, 480);
        mAlpha = false;
    }

    OpenGLGraphics::OpenGLGraphics(int width, int height)
    {
        setTargetPlane(width, height);
    }

    OpenGLGraphics::~OpenGLGraphics()
    {

    }

    void OpenGLGraphics::_beginDraw()
    {
        glPushAttrib(
            GL_COLOR_BUFFER_BIT |
            GL_CURRENT_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_ENABLE_BIT |
            GL_FOG_BIT |
            GL_LIGHTING_BIT |
            GL_LINE_BIT |
            GL_POINT_BIT |
            GL_POLYGON_BIT |
            GL_SCISSOR_BIT |
            GL_STENCIL_BUFFER_BIT    |
            GL_TEXTURE_BIT |
            GL_TRANSFORM_BIT
            );

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glLoadIdentity();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        glOrtho(0.0, (double)mWidth, (double)mHeight, 0.0, -1.0, 1.0);

        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        glEnable(GL_SCISSOR_TEST);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        pushClipArea(Rectangle(0, 0, mWidth, mHeight));
    }

    void OpenGLGraphics::_endDraw()
    {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glMatrixMode(GL_TEXTURE);
        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glPopAttrib();

        popClipArea();
    }

    bool OpenGLGraphics::pushClipArea(Rectangle area)
    {
        bool result = Graphics::pushClipArea(area);

        glScissor(mClipStack.top().x,
                  mHeight - mClipStack.top().y - mClipStack.top().height,
                  mClipStack.top().width,
                  mClipStack.top().height);

        return result;
    }

    void OpenGLGraphics::popClipArea()
    {
        Graphics::popClipArea();

        if (mClipStack.empty())
        {
            return;
        }

        glScissor(mClipStack.top().x,
                  mHeight - mClipStack.top().y - mClipStack.top().height,
                  mClipStack.top().width,
                  mClipStack.top().height);
    }

    void OpenGLGraphics::setTargetPlane(int width, int height)
    {
        mWidth = width;
        mHeight = height;
    }

    void OpenGLGraphics::drawImage(const Image* image, int srcX, int srcY,
                                   int dstX, int dstY, int width,
                                   int height)
    {
        dstX += mClipStack.top().xOffset;
        dstY += mClipStack.top().yOffset;

        // The following code finds the real width and height of the texture.
        // OpenGL only supports texture sizes that are powers of two
        int realImageWidth = 1;
        int realImageHeight = 1;
        while (realImageWidth < image->getWidth())
        {
            realImageWidth *= 2;
        }
        while (realImageHeight < image->getHeight())
        {
            realImageHeight *= 2;
        }

        // Find OpenGL texture coordinates
        float texX1 = srcX / (float)realImageWidth;
        float texY1 = srcY / (float)realImageHeight;
        float texX2 = (srcX+width) / (float)realImageWidth;
        float texY2 = (srcY+height) / (float)realImageHeight;

        // Please dont look too closely at the next line, it is not pretty.
        // It uses the image data as a pointer to a GLuint
        glBindTexture(GL_TEXTURE_2D, *((GLuint *)(image->_getData())));

        glEnable(GL_TEXTURE_2D);

        // Check if blending already is enabled
        if (!mAlpha)
        {
            glEnable(GL_BLEND);
        }

        // Draw a textured quad -- the image
        glBegin(GL_QUADS);
        glTexCoord2f(texX1, texY1);
        glVertex3i(dstX, dstY, 0);

        glTexCoord2f(texX1, texY2);
        glVertex3i(dstX, dstY + height, 0);

        glTexCoord2f(texX2, texY2);
        glVertex3i(dstX + width, dstY + height, 0);

        glTexCoord2f(texX2, texY1);
        glVertex3i(dstX + width, dstY, 0);
        glEnd();

        glDisable(GL_TEXTURE_2D);

        // Don't disable blending if the color has alpha
        if (!mAlpha)
        {
            glDisable(GL_BLEND);
        }
    }

    void OpenGLGraphics::drawPoint(int x, int y)
    {
        x += mClipStack.top().xOffset;
        y += mClipStack.top().yOffset;

        glBegin(GL_POINTS);
        glVertex3i(x, y, 0);
        glEnd();
    }

    void OpenGLGraphics::drawLine(int x1, int y1, int x2, int y2)
    {
        x1 += mClipStack.top().xOffset;
        y1 += mClipStack.top().yOffset;
        x2 += mClipStack.top().xOffset;
        y2 += mClipStack.top().yOffset;

        glBegin(GL_LINES);
        glVertex3f(x1+0.5f, y1+0.5f, 0);
        glVertex3f(x2+0.5f, y2+0.5f, 0);
        glEnd();

        glBegin(GL_POINTS);
        glVertex3f(x2+0.5f, y2+0.5f, 0);
        glEnd();
    }

    void OpenGLGraphics::drawRectangle(const Rectangle& rectangle)
    {
        glBegin(GL_LINE_LOOP);
        glVertex3f(rectangle.x + mClipStack.top().xOffset + 0.5f,
                   rectangle.y + mClipStack.top().yOffset + 0.5f, 0);
        glVertex3f(rectangle.x + rectangle.width - 0.5f + mClipStack.top().xOffset,
                   rectangle.y + mClipStack.top().yOffset + 0.5f, 0);
        glVertex3f(rectangle.x + rectangle.width - 0.5f + mClipStack.top().xOffset,
                   rectangle.y + rectangle.height + mClipStack.top().yOffset - 0.5f, 0);
        glVertex3f(rectangle.x + mClipStack.top().xOffset + 0.5f,
                   rectangle.y + rectangle.height + mClipStack.top().yOffset - 0.5f, 0);
        glEnd();
    }

    void OpenGLGraphics::fillRectangle(const Rectangle& rectangle)
    {
        glBegin(GL_QUADS);
        glVertex3i(rectangle.x + mClipStack.top().xOffset,
                   rectangle.y + mClipStack.top().yOffset, 0);
        glVertex3i(rectangle.x + rectangle.width + mClipStack.top().xOffset,
                   rectangle.y + mClipStack.top().yOffset, 0);
        glVertex3i(rectangle.x + rectangle.width + mClipStack.top().xOffset,
                   rectangle.y + rectangle.height + mClipStack.top().yOffset, 0);
        glVertex3i(rectangle.x + mClipStack.top().xOffset,
                   rectangle.y + rectangle.height + mClipStack.top().yOffset, 0);
        glEnd();
    }

    void OpenGLGraphics::setColor(const Color& color)
    {
        mColor = color;
        glColor4f(color.r/255.0f,
                  color.g/255.0f,
                  color.b/255.0f,
                  color.a/255.0f);

        mAlpha = color.a != 255;

        if (mAlpha)
        {
            glEnable(GL_BLEND);
        }
    }

    const Color& OpenGLGraphics::getColor()
    {
        return mColor;
    }
}
