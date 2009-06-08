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
#else
#include <GL/gl.h>
#endif

#include "guichan/opengl/openglimageloader.h"
#include "guichan/exception.h"

namespace gcn
{

    OpenGLImageLoader::OpenGLImageLoader()
    {
        mImageLoader = 0;
    }

    OpenGLImageLoader::OpenGLImageLoader(ImageLoader* imageLoader)
    {
        mImageLoader = imageLoader;
    }

    void OpenGLImageLoader::setHostImageLoader(ImageLoader* imageLoader)
    {
        mImageLoader = imageLoader;
    }

    void OpenGLImageLoader::prepare(const std::string& filename)
    {
        if(mImageLoader == NULL)
        {
            throw GCN_EXCEPTION("No host ImageLoader present.");
        }

        mImageLoader->prepare(filename);
    }

    void* OpenGLImageLoader::getRawData()
    {
        if(mImageLoader == NULL)
        {
            throw GCN_EXCEPTION("No host ImageLoader present.");
        }

        return mImageLoader->getRawData();
    }

    void* OpenGLImageLoader::finalize()
    {
        if(mImageLoader == NULL)
        {
            throw GCN_EXCEPTION("No host ImageLoader present.");
        }

        unsigned int *rawData = (unsigned int *)mImageLoader->getRawData();
        int width = mImageLoader->getWidth();
        int height = mImageLoader->getHeight();
        int realWidth = 1, realHeight = 1;

        while(realWidth < width)
        {
            realWidth *= 2;
        }

        while(realHeight < height)
        {
            realHeight *= 2;
        }

        unsigned int *realData = new unsigned int[realWidth*realHeight];
        int x, y;

        for (y = 0; y < realHeight; y++)
        {
            for (x = 0; x < realWidth; x++)
            {
                if (x < width && y < height)
                {
                    if (rawData[x+y*width] == 0xffff00ff)
                    {
                        realData[x+y*realWidth] = 0x00000000;
                    }
                    else
                    {
                        realData[x+y*realWidth] = rawData[x+y*width];
                    }
                }
                else
                {
                    realData[x+y*realWidth] = 0;
                }
            }
        }

        GLuint *texture = new GLuint[1];
        glGenTextures(1, texture);
        glBindTexture(GL_TEXTURE_2D, *texture);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     4,
                     realWidth,
                     realHeight,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     realData);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

        delete[] realData;
        mImageLoader->discard();

        GLenum error = glGetError();
        if (error)
        {
            std::string errmsg;
            switch (error)
            {
              case GL_INVALID_ENUM:
                  errmsg = "GL_INVALID_ENUM";
                  break;

              case GL_INVALID_VALUE:
                  errmsg = "GL_INVALID_VALUE";
                  break;

              case GL_INVALID_OPERATION:
                  errmsg = "GL_INVALID_OPERATION";
                  break;

              case GL_STACK_OVERFLOW:
                  errmsg = "GL_STACK_OVERFLOW";
                  break;

              case GL_STACK_UNDERFLOW:
                  errmsg = "GL_STACK_UNDERFLOW";
                  break;

              case GL_OUT_OF_MEMORY:
                  errmsg = "GL_OUT_OF_MEMORY";
                  break;
            }

            throw GCN_EXCEPTION(std::string("glGetError said: ") + errmsg);
        }

        return (void *)texture;
    }

    void OpenGLImageLoader::discard()
    {
        if(mImageLoader == NULL)
        {
            throw GCN_EXCEPTION("No host ImageLoader present.");
        }

        mImageLoader->discard();
    }

    void OpenGLImageLoader::free(Image* image)
    {
        glDeleteTextures(1, (GLuint *)image->_getData());

        delete[] (GLuint *)(image->_getData());
    }

    int OpenGLImageLoader::getWidth() const
    {
        if(mImageLoader == NULL)
        {
            throw GCN_EXCEPTION("No host ImageLoader present.");
        }

        return mImageLoader->getWidth();
    }

    int OpenGLImageLoader::getHeight() const
    {
        if(mImageLoader == NULL)
        {
            throw GCN_EXCEPTION("No host ImageLoader present.");
        }

        return mImageLoader->getHeight();
    }

    Color OpenGLImageLoader::getPixel(int x, int y)
    {
        if(mImageLoader == NULL)
        {
            throw GCN_EXCEPTION("No host ImageLoader present.");
        }

        return mImageLoader->getPixel(x, y);
    }

    void OpenGLImageLoader::putPixel(int x, int y, const Color& color)
    {
        if(mImageLoader == NULL)
        {
            throw GCN_EXCEPTION("No host ImageLoader present.");
        }

        mImageLoader->putPixel(x, y, color);
    }
}
