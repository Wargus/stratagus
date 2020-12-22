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
//      (c) Copyright 2020 by Alyokhin
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

void CEasedTexture::Init(const uint16_t width, const uint16_t height, const uint8_t numOfSteps)
{
    Width       = width;
    Height      = height;
    TextureSize = Width * Height;

    for (auto &frame : Frames) {
        frame.clear();
        frame.resize(TextureSize);

        const size_t size = frame.size();
        const uint8_t *ptr = frame.data();

        std::fill(frame.begin(), frame.end(), 0xFF);
    }

    Deltas.clear();
    Deltas.resize(TextureSize);
    std::fill(Deltas.begin(), Deltas.end(), 0xFF);

    EasingStepsNum  = numOfSteps;
    CurrentStep     = numOfSteps;
    isWatingForNext = true;
}

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
    isWatingForNext = false;
}

void CEasedTexture::DrawRegion(uint8_t *target, const uint16_t trgWidth, const uint16_t x0, const uint16_t y0, const SDL_Rect &srcRect)
{
    const uint16_t xEnd   = srcRect.x + srcRect.w;
    
    intptr_t trgIndex     = y0 * trgWidth + x0;
    intptr_t textureIndex = srcRect.y * Width + srcRect.x;
   
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

void CEasedTexture::CalcDeltas() 
{ 
    const uint8_t *curr   = Frames[Curr].data();
    const uint8_t *next   = Frames[Next].data();

    size_t index = 0;
    for (int16_t &delta : Deltas)
    {
        delta = (static_cast <int16_t>(next[index]) - curr[index]) / EasingStepsNum;
        index++;
    }
}



/**
**  Init box blurer
**
**  @param textureWidth width of the working texture (input/output texture also)
**  @param textureHeight height of the working texture (input/output texture also)
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
    //Assert (radius >= 0 && numOfIterations > 0);

    Radius          = radius;
    NumOfIterations = numOfIterations;

    if (Radius * NumOfIterations == 0) { return; }

    float D = sqrt((12.0 * radius * radius  / numOfIterations) + 1);
    int dL = floor(D);  
    if (dL % 2 == 0) { dL--; }
    int dU = dL + 2;
				
    float M = (12 * radius * radius - numOfIterations * dL * dL - 4 * numOfIterations * dL - 3 * numOfIterations) / (-4 * dL - 4);
    int m = round(M);

    HalfBoxes.clear();
    HalfBoxes.resize(numOfIterations);
    for (uint8_t i = 0; i < numOfIterations; i++) {
        HalfBoxes[i] = ((float)(i < m ? dL : dU) - 1) / 2;
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
void CBlurer::Blur(uint8_t *texture)
{
    if (Radius * NumOfIterations == 0) { return; }
    
    uint8_t *source = texture;
    uint8_t *target = WorkingTexture.data();

    for (int i = 0; i < HalfBoxes.size(); i++){
        if (i > 0) {
            uint8_t *swap = source;
            source = target;
            target = swap;
        }
        ProceedIteration(source, target, HalfBoxes[i]); 
    }
    if (target != texture) {
        // memcpy(texture, WorkingTexture.data(), WorkingTexture.size() * sizeof(uint8_t));
        std::copy(WorkingTexture.begin(), WorkingTexture.end(), texture);
    }
}

/**
**  Proceed one iteration of box bluring
**
**  @param  source  source texture (which has to be blured)
**  @param  target  target texture (where result will be)
**  @param  radius  blur radius (box size) for current iteration
** /// TODO: sitch to fixed point math
**
*/
void CBlurer::ProceedIteration(uint8_t *source, uint8_t *target, const uint8_t radius)
{
    //memcpy(target, source, WorkingTexture.size() * sizeof(uint8_t));
    std::copy_n(&source[0], WorkingTexture.size(), target);

    uint8_t *swap = source;
    source = target;
    target = swap; 

    float iarr = 1.0 / (radius + radius + 1);
    size_t ti, li, ri;
    uint8_t leftBorder, rightBorder;
    int16_t sum;

    /// Horizontal blur pass
    for (size_t i = 0; i < TextureHeight; i++) {

        ti = i * TextureWidth; 
        li = ti;
        ri = ti + radius;
        leftBorder  = source[ti];
        rightBorder = source[ti + TextureWidth - 1];
        sum         = (radius + 1) * leftBorder;

        for (size_t j = 0; j < radius; j++) { 
            sum += source[ti + j]; 
        }
        for (size_t j = 0; j <= radius; j++) {
            sum += source[ri++] - leftBorder; 
            target[ti++] = round(iarr * sum);
        }
        for (size_t j = radius + 1; j < TextureWidth - radius; j++) {
            sum += source[ri++] - source[li++];
            target[ti++] = round(sum * iarr);
        }
        for (size_t j = TextureWidth - radius; j < TextureWidth; j++) {
            sum += rightBorder - source[li++];   
            target[ti++] = round(iarr * sum);
        }
    }

    swap = source;
    source = target;
    target = swap;  

    /// Vertical blur pass
    for (size_t i = 0; i < TextureWidth; i++) {

        ti = i;
        li = ti;
        ri = ti + radius * TextureWidth;
        leftBorder  = source[ti];
        rightBorder = source[ti + TextureWidth * (TextureHeight - 1)];
        sum         = (radius + 1) * leftBorder;

        for (size_t j = 0; j < radius; j++) {
            sum += source[ti + j * TextureWidth];
        }
        for (size_t j = 0; j <= radius ; j++) { 
            sum += source[ri] - leftBorder;
            target[ti] = round(iarr * sum);
            ri += TextureWidth;
            ti += TextureWidth;
        }
        for (size_t j = radius + 1; j < TextureHeight - radius; j++) { 
            sum += source[ri] - source[li];
            target[ti] = round(iarr * sum);
            li += TextureWidth;
            ri += TextureWidth;
            ti += TextureWidth;
        }
        for (size_t j = TextureHeight - radius; j < TextureHeight; j++) { 
            sum += rightBorder - source[li];
            target[ti] = round(iarr * sum);
            li += TextureWidth;
            ti += TextureWidth;
        }
    }

}


//@}
