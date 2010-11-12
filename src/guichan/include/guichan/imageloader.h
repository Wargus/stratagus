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

#ifndef GCN_IMAGELOADER_HPP
#define GCN_IMAGELOADER_HPP

#include <string>

#include "guichan/color.h"
#include "guichan/platform.h"

namespace gcn
{
    class Image;

    /**
     * ImageLoaders base class. Contains basic image loading functions every
     * image loader should have. Image loaders should inherit from this
     * class and impements it's functions.
     */
    class GCN_CORE_DECLSPEC ImageLoader
    {
    public:

        /**
         * Destructor.
         */
        virtual ~ImageLoader() { }

        /**
         * Prepares an image for reading. After you have called this function
         * you can retrieve information about it and edit it.
         *
         * @param filename the image file to prepare.
         * @throws Exception when called without having finalized or disposed to
         *                   last image or when unable to load the image.
         */
        virtual void prepare(const std::string& filename) = 0;

        /**
         * This function frees an image.
         *
         * NOTE: There is generally no reason to call this function as
         *       it is called upon by the Image object when destroying an Image.
         *
         * @param image the image to be freed and removed.
         * @throws Exception when image points to NULL.
         */
        virtual void free(Image* image) = 0;

        /**
         * Rreturns a pointer of raw data of an image. The raw data is in 32
         * bit RGBA format. The funcion will not free a prepared image, so
         * finalize or discard should be used afterwards.
         *
         * @return a pointer to the raw image data.
         */
        virtual void* getRawData() = 0;

        /**
         * Finalizes an image meaning it will return the image data. If the
         * image contains pixels with "magic pink" (0xff00ff) they will be
         * treated as transparent pixels.
         *
         * @return a pointer to the image data.
         * @throws Exception when no image has been prepared.
         */
        virtual void* finalize() = 0;

        /**
         * Discards a prepared image.
         *
         * @throws Exception when no image has been prepared.
         */
        virtual void discard() = 0;

        /**
         * Gets the height if the image.
         *
         * @return the height of the image.
         * @throws Exception if no image have been prepared.
         */
        virtual int getHeight() const = 0;

        /**
         * Gets the width of an image.
         *
         * @return the width of the image.
         * @throws Exception if no image have been prepared.
         */
        virtual int getWidth() const = 0;

        /**
         * Gets the color of a pixel at coordinate x and y.
         *
         * @param x the x coordinate.
         * @param y the y coordinate.
         * @return the color of the pixel.
         */
        virtual Color getPixel(int x, int y) = 0;

        /**
         * Puts a pixel with a certain color at coordinate x and y.
         *
         * @param x the x coordinate.
         * @param y the y coordinate.
         * @param color the color of the pixel to put.
         */
        virtual void putPixel(int x, int y, const Color& color) = 0;
    };
}

#endif // end GCN_IMAGELOADER_HPP
