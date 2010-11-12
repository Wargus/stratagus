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
#ifdef USE_SDL_IMAGE
#include <SDL/SDL_image.h>

#include "guichan/sdl/sdlimageloader.h"
#include "guichan/exception.h"
#include "guichan/sdl/sdlpixel.h"

namespace gcn
{

    SDLImageLoader::SDLImageLoader()
    {
        mCurrentImage = NULL;
    }

    void SDLImageLoader::prepare(const std::string& filename)
    {
        if (mCurrentImage != NULL)
        {
            throw GCN_EXCEPTION("Function called before finalizing or discarding last loaded image.");
        }

        SDL_Surface* tmp = IMG_Load(filename.c_str());

        if (tmp == NULL)
        {
            throw GCN_EXCEPTION(std::string("Unable to load image file: ")+filename);
        }

        Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif

        mCurrentImage = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 32,
                                             rmask, gmask, bmask, amask);

        if (mCurrentImage == NULL)
        {
            throw GCN_EXCEPTION(std::string("Not enough memory to load: ")+filename);
        }

        SDL_Surface* tmp2 = SDL_ConvertSurface(tmp, mCurrentImage->format, SDL_SWSURFACE);
        SDL_FreeSurface(tmp);
        SDL_FreeSurface(mCurrentImage);

        mCurrentImage = tmp2;
    }

    void* SDLImageLoader::getRawData()
    {
        return mCurrentImage->pixels;
    }

    void* SDLImageLoader::finalize()
    {
        if (mCurrentImage == NULL)
        {
            throw GCN_EXCEPTION("No image prepared.");
        }

        int i;
        bool hasPink = false;
        bool hasAlpha = false;

        for (i = 0; i < mCurrentImage->w * mCurrentImage->h; ++i)
        {
            if (((unsigned int*)mCurrentImage->pixels)[i] == SDL_MapRGB(mCurrentImage->format,255,0,255))
            {
                hasPink = true;
                break;
            }
        }

        for (i = 0; i < mCurrentImage->w * mCurrentImage->h; ++i)
        {
            Uint8 r, g, b, a;

            SDL_GetRGBA(((unsigned int*)mCurrentImage->pixels)[i], mCurrentImage->format,
                        &r, &g, &b, &a);

            if (a != 255)
            {
                hasAlpha = true;
                break;
            }
        }

        // Don't convert 32bpp images with alpha, it will destroy the
        // alpha channel.
        SDL_Surface *temp;
        if (hasAlpha)
        {
            temp = mCurrentImage;
            mCurrentImage = NULL;
        }
        else
        {
            temp = SDL_DisplayFormat(mCurrentImage);
            SDL_FreeSurface(mCurrentImage);
            mCurrentImage = NULL;
        }

        if (hasPink)
        {
            SDL_SetColorKey(temp, SDL_SRCCOLORKEY,
                            SDL_MapRGB(temp->format,255,0,255));
        }
        if (hasAlpha)
        {
            SDL_SetAlpha(temp, SDL_SRCALPHA, 255);
        }

        return temp;
    }

    void SDLImageLoader::discard()
    {
        if (mCurrentImage == NULL)
        {
            throw GCN_EXCEPTION("No image prepared.");
        }

        SDL_FreeSurface(mCurrentImage);

        mCurrentImage = NULL;
    }

    void SDLImageLoader::free(Image* image)
    {
        if (image->_getData() == NULL)
        {
            throw GCN_EXCEPTION("Image data points to null.");
        }

        SDL_FreeSurface((SDL_Surface*)image->_getData());
    }

    int SDLImageLoader::getWidth() const
    {
        if (mCurrentImage == NULL)
        {
            throw GCN_EXCEPTION("No image prepared.");
        }

        return mCurrentImage->w;
    }

    int SDLImageLoader::getHeight() const
    {
        if (mCurrentImage == NULL)
        {
            throw GCN_EXCEPTION("No image prepared.");
        }

        return mCurrentImage->h;
    }

    Color SDLImageLoader::getPixel(int x, int y)
    {
        if (mCurrentImage == NULL)
        {
            throw GCN_EXCEPTION("No image prepared.");
        }

        if (x < 0 || y < 0 || x >= mCurrentImage->w || y >= mCurrentImage->h)
        {
            throw GCN_EXCEPTION("x and y out of image bound.");
        }

        return SDLgetPixel(mCurrentImage, x, y);
    }

    void SDLImageLoader::putPixel(int x, int y, const Color& color)
    {
        if (mCurrentImage == NULL)
        {
            throw GCN_EXCEPTION("No image prepared.");
        }

        if (x < 0 || y < 0 || x >= mCurrentImage->w || y >= mCurrentImage->h)
        {
            throw GCN_EXCEPTION("x and y out of image bound.");
        }

        SDLputPixel(mCurrentImage, x, y, color);
    }
}
#endif
