//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name fow_utils.cpp - The utilities for fog of war . */
//
//      (c) Copyright 2020-2021 by Alyokhin
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <algorithm>

#include "stratagus.h"
#include "fow_utils.h"


/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/
/**
**  Init eased texture
**
**  @param width width of the texture
**  @param height height of the texture
**  @param numOfSteps number easing steps
**
*/
void CEasedTexture::Init(const uint16_t width, const uint16_t height, const uint8_t numOfSteps)
{
    Width       = width;
    Height      = height;
    TextureSize = Width * Height;

    for (auto &frame : Frames) {
        frame.clear();
        frame.resize(TextureSize);

        ranges::fill(frame, 0xFF);
    }

    Deltas.clear();
    Deltas.resize(TextureSize);
    ranges::fill(Deltas, 0xFF);

    EasingStepsNum  = numOfSteps;
    CurrentStep     = numOfSteps;
}

/**
**  Clean the texture
**
*/
void CEasedTexture::Clean()
{
    for (auto &frame : Frames) {
        frame.clear();
    }
    Deltas.clear();

    Width           = 0;
    Height          = 0;
    EasingStepsNum  = 0;
    CurrentStep     = 0;
}

/**
**  Change number of easing steps
**
**  @param width width of the texture
**  @param height height of the texture
**  @param numOfSteps number of the texture easing steps
**
*/
void CEasedTexture::SetNumOfSteps (const uint8_t num)
{
    EasingStepsNum  = num;
    CurrentStep     = EasingStepsNum;
}

/**
**  Switch easing to the new frame of the texture
**
**  @param forcedShowNext cmd to immediately show next frame without easing
**
*/
void CEasedTexture::PushNext(const bool forcedShowNext /*= false*/)
{
    CalcDeltas();
    SwapFrames();
    CurrentStep = forcedShowNext ? EasingStepsNum : 0;
}

/**
**  Draw a rectangle of the texture into the target
**
**  @param target   target where draw texture to
**  @param trgWidth target width in pixels
**  @param x0       left coordinate in the target (where to draw)
**  @param y0       upper coordinate in the target (where to draw)
**  @param srcRect  rectangle of the texture to draw
**
*/
void CEasedTexture::DrawRegion(uint8_t *target, const uint16_t trgWidth, const uint16_t x0, const uint16_t y0, const SDL_Rect &srcRect)
{
    size_t trgIndex     = y0 * trgWidth + x0;
    size_t textureIndex = srcRect.y * Width + srcRect.x;

    if (CurrentStep == 0 || CurrentStep == EasingStepsNum) {
        const uint8_t currFrame = CurrentStep ? Curr : Prev;
        const uint8_t *curr     = Frames[currFrame].data();

        for (uint16_t y = 0; y < srcRect.h; y++) {
            std::copy_n(&curr[textureIndex], srcRect.w, &target[trgIndex]);
            textureIndex += Width;
            trgIndex     += trgWidth;
        }
    } else {
        const uint8_t *prev   = Frames[Prev].data();
        const int16_t *deltas = Deltas.data();
        for (uint16_t y = 0; y < srcRect.h; y++) {
            for (uint16_t x = 0; x < srcRect.w; x++) {
                target[trgIndex + x] = prev[textureIndex + x] + deltas[textureIndex + x] * CurrentStep;
            }
            textureIndex += Width;
            trgIndex     += trgWidth;
        }
    }
}

/**
**  Calculate deltas between next and current frames
**
*/
void CEasedTexture::CalcDeltas()
{
    const uint8_t *curr   = Frames[Curr].data();
    const uint8_t *next   = Frames[Next].data();

    #pragma omp parallel
    {
        const uint16_t thisThread   = omp_get_thread_num();
        const uint16_t numOfThreads = omp_get_num_threads();

        const size_t lBound = TextureSize * (thisThread    ) / numOfThreads;
        const size_t uBound = TextureSize * (thisThread + 1) / numOfThreads;

        for (size_t index = lBound; index < uBound; index++) {
            Deltas[index] = (int16_t(next[index]) - curr[index]) / EasingStepsNum;
        }
    }
}

/**
**  Init box blurer
**
**  @param textureWidth width of the working texture (input/output texture also)
**  @param textureHeight height of the working texture (input/output texture also)
**  @param radius Radius or standard deviation
**  @param numOfIterations c
**
*/
void CBlurer::Init(const uint16_t textureWidth, const uint16_t textureHeight, const float radius, const int numOfIterations)
{
    PrecalcParameters(radius, numOfIterations);

    TextureWidth  = textureWidth;
    TextureHeight = textureHeight;
    WorkingTexture.clear();
    WorkingTexture.resize(TextureWidth * TextureHeight);
}

/**
**  Init box blurer parameters (determine radiuses (box sizes) for box blur iterations)
**  This used to set new parameters for blur algorithm
**
**  @param radius Radius or standard deviation
**  @param numOfIterations Number of boxes
*/
void CBlurer::PrecalcParameters(const float radius, const int numOfIterations)
{
    Radius          = radius;
    NumOfIterations = numOfIterations;

    if (Radius * NumOfIterations == 0) {
        return;
    }

    const float D = sqrt((12.0 * radius * radius  / numOfIterations) + 1);
    uint8_t dL = floor(D);
    if (dL % 2 == 0) { dL--; }
    const uint8_t dU = dL + 2;

    const float M = (12.0 * radius * radius - numOfIterations * dL * dL - 4 * numOfIterations * dL - 3 * numOfIterations) / (-4 * dL - 4);
    const uint8_t m = round(M);

    HalfBoxes.clear();
    HalfBoxes.resize(numOfIterations);
    for (uint8_t i = 0; i < numOfIterations; i++) {
        HalfBoxes[i] = ((i < m ? dL : dU) - 1) / 2;
    }
}

/**
**  Clean blurer
**
*/
void CBlurer::Clean()
{
    HalfBoxes.clear();
    WorkingTexture.clear();
    TextureWidth  = 0;
    TextureHeight = 0;
}


/**
** Blur a texture (optimized for 1 chanel (alpha) textures)
**
** @param  texture texture to blur (uint8_t)
**
*/
void CBlurer::Blur(uint8_t *const texture)
{
    if (Radius * NumOfIterations == 0) { return; }

    uint8_t *source = texture;
    uint8_t *target = WorkingTexture.data();

    for (uint8_t i = 0; i < HalfBoxes.size(); i++) {
        if (i > 0) {
            uint8_t *const swap = source;
            source = target;
            target = swap;
        }
        ProceedIteration(source, target, HalfBoxes[i]);
    }
    if (target != texture) {
        std::copy(WorkingTexture.begin(), WorkingTexture.end(), texture);
    }
}

/**
**  Proceed one iteration of box bluring
**
**  @param  source  source texture (which has to be blured)
**  @param  target  target texture (where result will be)
**  @param  radius  blur radius (box size) for current iteration
**
*/
void CBlurer::ProceedIteration(uint8_t *source, uint8_t *target, const uint8_t radius)
{
    constexpr uint32_t fixedOneHalf = 32768; // 0.5

    std::copy_n(&source[0], WorkingTexture.size(), target);

    uint8_t *swap = source;
    source = target;
    target = swap;

    /// *fixed point math
    const uint32_t iarr = (1 << 16) / (2 * radius + 1);

    /// Horizontal blur pass
    #pragma omp parallel
    {
        const uint16_t thisThread   = omp_get_thread_num();
        const uint16_t numOfThreads = omp_get_num_threads();

        const uint16_t lBound = TextureHeight * (thisThread    ) / numOfThreads;
        const uint16_t uBound = TextureHeight * (thisThread + 1) / numOfThreads;

        for (uint16_t i = lBound; i < uBound; i++) {

            size_t ti = size_t(i) * TextureWidth;
            size_t li = ti;
            size_t ri = ti + radius;

            const uint8_t leftBorder  = source[ti];
            const uint8_t rightBorder = source[ti + TextureWidth - 1];
                  int16_t sum         = int16_t(radius + 1) * leftBorder;

            for (uint16_t j = 0; j < radius; j++) {
                sum += source[ti + j];
            }
            for (uint16_t j = 0; j <= radius; j++) {
                sum += source[ri++] - leftBorder;
                target[ti++] = (iarr * sum + fixedOneHalf) >> 16;
            }
            for (uint16_t j = radius + 1; j < TextureWidth - radius; j++) {
                sum += source[ri++] - source[li++];
                target[ti++] = (iarr * sum + fixedOneHalf) >> 16;
            }
            for (uint16_t j = TextureWidth - radius; j < TextureWidth; j++) {
                sum += rightBorder - source[li++];
                target[ti++] = (iarr * sum + fixedOneHalf) >> 16;
            }
        }
    } // pragma omp parallel

    swap = source;
    source = target;
    target = swap;

    /// Vertical blur pass
    #pragma omp parallel
    {
        const uint16_t thisThread   = omp_get_thread_num();
        const uint16_t numOfThreads = omp_get_num_threads();

        const uint16_t lBound = TextureWidth * (thisThread    ) / numOfThreads;
        const uint16_t uBound = TextureWidth * (thisThread + 1) / numOfThreads;

        for (uint16_t i = lBound; i < uBound; i++) {

            size_t ti = i;
            size_t li = ti;
            size_t ri = ti + radius * TextureWidth;

            const uint8_t leftBorder  = source[ti];
            const uint8_t rightBorder = source[ti + TextureWidth * (TextureHeight - 1)];
                  int16_t sum         = int16_t(radius + 1) * leftBorder;

            for (uint16_t j = 0; j < radius; j++) {
                sum += source[ti + j * TextureWidth];
            }
            for (uint16_t j = 0; j <= radius ; j++) {
                sum += source[ri] - leftBorder;
                target[ti] = (iarr * sum + fixedOneHalf) >> 16;
                ri += TextureWidth;
                ti += TextureWidth;
            }
            for (uint16_t j = radius + 1; j < TextureHeight - radius; j++) {
                sum += source[ri] - source[li];
                target[ti] = (iarr * sum + fixedOneHalf) >> 16;
                li += TextureWidth;
                ri += TextureWidth;
                ti += TextureWidth;
            }
            for (uint16_t j = TextureHeight - radius; j < TextureHeight; j++) {
                sum += rightBorder - source[li];
                target[ti] = (iarr * sum + fixedOneHalf) >> 16;
                li += TextureWidth;
                ti += TextureWidth;
            }
        }
    } // pragma omp parallel
}


//@}
