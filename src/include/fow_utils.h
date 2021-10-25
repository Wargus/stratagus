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
/**@name fow_utils.h - The utilities for fog of war headerfile. */
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

#ifndef __FOW_UTILS_H__
#define __FOW_UTILS_H__

#include <cstdint>
#include <vector>
#include "SDL.h"


//@{


class CViewport;


/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/
class CEasedTexture
{
public:
    void Init(const uint16_t width, const uint16_t height, const uint8_t numOfSteps);
    void Clean();

    void SetNumOfSteps(const uint8_t num);
    void PushNext(const bool forcedShowNext = false);
    void DrawRegion(uint8_t *target, const uint16_t trgWidth, const uint16_t x0, const uint16_t y0, const SDL_Rect &srcRect);
    uint8_t GetPixel(const uint16_t x, const uint16_t y);

    bool isFullyEased() const { return CurrentStep == EasingStepsNum ? true : false; }
    void Ease()               { if (CurrentStep < EasingStepsNum) CurrentStep++; }

    uint8_t *GetCurrent()      { return Frames[Prev].data(); }
    uint8_t *GetNext()         { return Frames[Next].data(); }
    uint16_t GetWidth()  const { return Width;  }
    uint16_t GetHeight() const { return Height; }

private:
    void CalcDeltas();
    void SwapFrames() { const uint8_t swap = Prev; Prev = Curr; Curr = Next; Next = swap; }

private:
    uint8_t  CurrentStep    {0};
    uint8_t  EasingStepsNum {0}; 
    uint16_t Width          {0};
    uint16_t Height         {0};
    size_t   TextureSize    {0}; // Width * Height

    std::vector<uint8_t> Frames[3];
    uint8_t              Prev  {0};
    uint8_t              Curr  {1};
    uint8_t              Next  {2};

    std::vector<int16_t> Deltas;
};

/// Class for box blur algorithm. Used to blur 4x4 upscaled FOW texture.
class CBlurer
{
public:
    void Init(const uint16_t textureWidth, const uint16_t textureHeight, const float radius, const int numOfIterations);
    void PrecalcParameters(const float radius, const int numOfIterations);
    
    void Clean();
    void Blur(uint8_t *const texture);
private:
    void ProceedIteration(uint8_t *source, uint8_t *target, const uint8_t radius);

private:
    float   Radius          {2}; /// From 1 to 3 is optimal. With 3 result is very smooth, 
                                 /// but it opens about 1/2 extra tiles around SightRange circle
    uint8_t NumOfIterations {3}; /// 2-3 is optimal, with higher values result enhancing not so radicaly

    std::vector<uint8_t> HalfBoxes; /// Radiuses (box sizes) for box blur iterations
    std::vector<uint8_t> WorkingTexture;  /// Back buffer
    uint16_t TextureWidth  {0};
    uint16_t TextureHeight {0};
};

/// returns pixel value for current frame
inline uint8_t CEasedTexture::GetPixel(const uint16_t x, const uint16_t y)
{
    const size_t textureIndex = y * Width + x;
    
    if (CurrentStep == 0 || CurrentStep == EasingStepsNum) {
        const uint8_t currFrame = CurrentStep ? Curr : Prev;
        const uint8_t *curr     = Frames[currFrame].data();
       
        return curr[textureIndex];
    
    } else {
        const uint8_t *prev   = Frames[Prev].data();
        const int16_t *deltas = Deltas.data();

        return prev[textureIndex] + deltas[textureIndex] * CurrentStep;
    
    }
}

#endif // !__FOW_UTILS_H__
