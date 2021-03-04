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
/**@name fow.cpp - The fog of war. */
//
//      (c) Copyright 2021 by Alyokhin
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

#include "fow.h"
#include "map.h"
#include "player.h"
#include "tile.h"
#include "ui.h"
#include "viewport.h"
#include <omp.h>
/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/
CFogOfWar FogOfWar; /// Fog of war itself

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/
void CFogOfWar::Init()
{
    /// +1 to the top & left and +1 to the bottom & right for 4x scale algorithm purposes, 
    /// Extra tiles will always be VisionType::cNone. 
    VisTableWidth   = Map.Info.MapWidth + 2;    
    VisTable_Index0 = VisTableWidth + 1;

    const size_t tableSize = VisTableWidth * (Map.Info.MapHeight + 2);
    VisTable.clear();
    VisTable.resize(tableSize);
    std::fill(VisTable.begin(), VisTable.end(), VisionType::cUnseen);

    /// +1 to the top & left and +1 to the bottom & right for 4x scale algorithm purposes, 
    const uint16_t fogTextureWidth  = (Map.Info.MapWidth  + 2) * 4;
    const uint16_t fogTextureHeight = (Map.Info.MapHeight + 2) * 4;

    FogTexture.Init(fogTextureWidth, fogTextureHeight, Settings.NumOfEasingSteps);
    
    RenderedFog.clear();
    RenderedFog.resize(Map.Info.MapWidth * Map.Info.MapHeight * 16);
    std::fill(RenderedFog.begin(), RenderedFog.end(), 0xFF);

    Blurer.Init(fogTextureWidth, fogTextureHeight, Settings.BlurRadius[Settings.UpscaleType], Settings.BlurIterations);

    this->State = cFirstEntry;
}

void CFogOfWar::Clean()
{
    VisTable.clear();
    VisTableWidth   = 0;
    VisTable_Index0 = 0;

    FogTexture.Clean();
    RenderedFog.clear();

    Blurer.Clean();
}

/** 
** Select which type of Fog of War to use
** 
** @param fow_type	type to set
** @return true if success, false for wrong fow_type 
*/
bool CFogOfWar::SetType(const FogOfWarTypes fow_type)
{
	if (fow_type < FogOfWarTypes::cNumOfTypes) {
		Settings.FOW_Type = fow_type;
		return true;
	} else {
		return false;
	}
}

/** 
** Enable or disable bilinear upscale for the final fog texture rendering
** 
** @param enable  cmd to enable/disable
** 
*/
void CFogOfWar::EnableBilinearUpscale(const bool enable) 
{
    const uint8_t prev = Settings.UpscaleType;
    Settings.UpscaleType = enable ? cBilinear : cSimple;
    if (prev != Settings.UpscaleType) {
        Blurer.PrecalcParameters(Settings.BlurRadius[Settings.UpscaleType], Settings.BlurIterations);
    }
}

void CFogOfWar::InitBlurer(const float radius1, const float radius2, const uint16_t numOfIterations)
{
    Settings.BlurRadius[cSimple]   = radius1;
    Settings.BlurRadius[cBilinear] = radius2;
    Settings.BlurIterations        = numOfIterations;
    Blurer.PrecalcParameters(Settings.BlurRadius[Settings.UpscaleType], numOfIterations);
}

void CFogOfWar::GenerateFog(const CPlayer &thisPlayer)
{
    intptr_t visIndex = VisTable_Index0;
    intptr_t mapIndex = 0;
    for (uint16_t row = 0 ; row < Map.Info.MapHeight; row++) {
		for (uint16_t col = 0; col < Map.Info.MapWidth; col++) {
            /// FIXME: to speedup this part, maybe we have to use Map.Field(mapIndex + col)->playerInfo.Visible[thisPlayer.index] instead
            /// this must be much faster
            VisTable[visIndex + col] = Map.Field(mapIndex + col)->playerInfo.TeamVisibilityState(thisPlayer);
		}
		visIndex += VisTableWidth;
        mapIndex += Map.Info.MapWidth;
	}
}

/**
**  Proceed fog of war state update
**
**  @param doAtOnce     command to calculate fog of war in single sycle
**
*/
void CFogOfWar::Update(bool doAtOnce /*= false*/)
{
    
    FogTexture.Ease();

    if (Settings.NumOfEasingSteps < States::cReady) doAtOnce = true;

    if (doAtOnce || this->State == cFirstEntry) {
        /// TODO: Add posibility to generate fog for different players (for replays purposes)
        GenerateFog(*ThisPlayer);
        GenerateFogTexture();
        Blurer.Blur(FogTexture.GetNext());
        FogTexture.PushNext(doAtOnce);
        this->State = cGenerateFog;
    } else {
        switch (this->State) {
            case cGenerateFog:
                /// TODO: Add posibility to generate fog for different players (for replays purposes)
                GenerateFog(*ThisPlayer);
                this->State++;
                break;

            case cGenerateTexture:
                GenerateFogTexture();
                this->State++;
                break;

            case cBlurTexture:
                Blurer.Blur(FogTexture.GetNext());
                this->State++;
                break;
                
            case cReady:
                if (FogTexture.isFullyEased()) {
                    FogTexture.PushNext();
                    this->State = cGenerateFog;
                }
                break;
            default:
                break;
        }
    }
}

/**
**  Render fog of war texture in certain viewport.
**
**  @param alphaTexture texture with alpha mask to render 
**  @param viewport     viewport to render fog of war texture in
*/
void CFogOfWar::RenderToViewPort(const CViewport &viewport, SDL_Surface *const vpSurface)
{

    SDL_Rect srcRect;
    srcRect.x = (viewport.MapPos.x + 1) * 4 - 2;  /// '+1' because of 1 tile frame around the texture 
    srcRect.y = (viewport.MapPos.y + 1) * 4 - 2;  /// '-2' is a half-tile compensation
    srcRect.w = viewport.MapWidth  * 4;
    srcRect.h = viewport.MapHeight * 4;
    
    const uint16_t x0 = viewport.MapPos.x * 4;
    const uint16_t y0 = viewport.MapPos.y * 4;

    FogTexture.DrawRegion(RenderedFog.data(), Map.Info.MapWidth * 4, x0, y0, srcRect);

    srcRect.x = viewport.MapPos.x  * 4;
    srcRect.y = viewport.MapPos.y  * 4;
    
    SDL_Rect trgRect;
    trgRect.x = 0;
    trgRect.y = 0;
    trgRect.w = viewport.MapWidth  * PixelTileSize.x;
    trgRect.h = viewport.MapHeight * PixelTileSize.y;
    
    RenderToSurface(RenderedFog.data(), srcRect, Map.Info.MapWidth * 4, vpSurface, trgRect);
}

/**
**  4x4 upscale generated fog of war texture
**
*/
void CFogOfWar::GenerateFogTexture()
{
    /*
    **  For all fields from VisTable in the given rectangle to calculate two patterns - Visible and Exlored.
    **  
    **  [1][2] checks neighbours (#2,#3,#4) for tile #1 to calculate upscale patterns.
    **  [3][4] 
    **
    **  There is only 16 patterns.
    **  According to these patterns fill the 4x4 sized alpha texture 
    **  with sum of UpscaleTable values (one for Visible and another for Explored)
    ** 
    **  VisTable     FogTexture
    **              [x][*][0][0]   where X - 2 or 1 - Visible or Exlored
    **   [X][0] --\ [*][0][0][0]         x - 1/2 transparency
    **   [0][0] --/ [0][0][0][0]         * - 1/4 transperency
    **              [0][0][0][0]         0 - full opacity
    */

    /// Because we work with 4x4 scaled map tiles here, the textureIndex is in 32bits chunks (byte * 4)
    uint32_t    *fogTexture    = reinterpret_cast<uint32_t*>(FogTexture.GetNext());
    const size_t textureHeight = FogTexture.GetHeight() / 4;
    const size_t textureWidth  = FogTexture.GetWidth()  / 4;
    intptr_t     textureIndex  = 0;
  
    /// in fact it's viewport.MapPos.y -1 & viewport.MapPos.x -1 because of VisTable starts from [-1:-1]
    intptr_t  visIndex = 0;
        
    for (int row = 0; row < textureHeight; row++) {
        for (int col = 0; col < textureWidth; col++) {
            /// Fill the 4x4 scaled tile
            FillUpscaledRec(fogTexture, textureWidth, textureIndex + col, 
                            DeterminePattern(visIndex + col, VisionType::cVisible), 
                            DeterminePattern(visIndex + col, VisionType::cVisible | VisionType::cExplored));
        }
        visIndex     += VisTableWidth;
        textureIndex += textureWidth * 4;
    }
}

/**
**  Determine upscale patterns (index in the upscale table) for Visible and Explored layers
**
**  @param  index       tile in the vision table 
**  @param  visFlag     layer to determine pattern for
**
*/
uint8_t CFogOfWar::DeterminePattern(const intptr_t index, const uint8_t visFlag)
{
    Assert(visFlag == VisionType::cVisible || visFlag == (VisionType::cExplored | VisionType::cVisible));

    uint8_t n1, n2, n3, n4;
    intptr_t offset = index;

    n1 = (visFlag & VisTable[offset]);
    n2 = (visFlag & VisTable[offset + 1]);
    offset += VisTableWidth;
    n3 = (visFlag & VisTable[offset]);
    n4 = (visFlag & VisTable[offset + 1]);
    
    n1 >>= n1 - VisionType::cExplored;
    n2 >>= n2 - VisionType::cExplored;
    n3 >>= n3 - VisionType::cExplored;
    n4 >>= n4 - VisionType::cExplored;
    
    return ( (n1 << 3) | (n2 << 2) | (n3 << 1) | n4 );
}


/**
**  Fill 4x4 sized tile in the fog texture according to the patterns
**
**  @param  texture         pointer to the texture to fill
**  @param  textureWidth    width of the texture
**  @param  index           index of the tile to fill
**  @param  patternVisible  index int the upscale table for Visible layer
**  @param  patternExplored index int the upscale table for Explored layer
**
*/
void CFogOfWar::FillUpscaledRec(uint32_t *texture, const int textureWidth, intptr_t index, 
                                const uint8_t patternVisible, const uint8_t patternExplored)
{
    for (int scan_line = 0; scan_line < 4; scan_line++) {
        texture[index] = UpscaleTable[patternVisible][scan_line] + UpscaleTable[patternExplored][scan_line];
        index += textureWidth;
    }
}

/**
** Zoom and render Fog Of War texture into SDL surface
** 
**  @param src          Image src.
**  @param srcRect      Rectangle in the src image to render
**  @param srcWidth     Image width
**  @param trgSurface   Where to render
**  @param trgRect      Scale src rectangle to this rectangle
**
*/
void CFogOfWar::RenderToSurface(const uint8_t *src, const SDL_Rect &srcRect, const int16_t srcWidth,
                                SDL_Surface *const trgSurface, const SDL_Rect &trgRect) 
{
    Assert(trgRect.w != 0 && trgRect.h != 0);
    Assert(trgSurface->format->BitsPerPixel == 32);

    switch (this->Settings.UpscaleType) {
        case cBilinear:
            UpscaleBilinear(src, srcRect, srcWidth, trgSurface, trgRect);
            break;
        case cSimple:
        default:
            UpscaleSimple(src, srcRect, srcWidth, trgSurface, trgRect);
            break;
    }
}


/**
** Bilinear zoom and render Fog Of War texture into SDL surface
** 
**  @param src          Image src.
**  @param srcRect      Rectangle in the src image to render
**  @param srcWidth     Image width
**  @param trgSurface   Where to render
**  @param trgRect      Scale src rectangle to this rectangle
**
*/
void CFogOfWar::UpscaleBilinear(const uint8_t *const src, const SDL_Rect &srcRect, const int16_t srcWidth,
                                SDL_Surface *const trgSurface, const SDL_Rect &trgRect) 
{
    const uint16_t surfaceAShift = trgSurface->format->Ashift;
    uint32_t *const target = static_cast<uint32_t *>(trgSurface->pixels);
    
    const int32_t xRatio = static_cast<int32_t>(((srcRect.w - 1) << 16) / trgRect.w);
    const int32_t yRatio = static_cast<int32_t>(((srcRect.h - 1) << 16) / trgRect.h);
    
#pragma omp parallel
{    

    const int thisThread = omp_get_thread_num();
    const int numOfThreads = omp_get_num_threads();
    
    const int this_iBegin = (thisThread    ) * trgRect.h / numOfThreads;
    const int this_iEnd   = (thisThread + 1) * trgRect.h / numOfThreads;

    intptr_t trgIndex     = (trgRect.y + this_iBegin) * trgSurface->w + trgRect.x;
    int64_t y             = (srcRect.y << 16) + this_iBegin * yRatio;

    for (size_t i = this_iBegin ; i < this_iEnd; i++) {

        const int32_t ySrc          = static_cast<int32_t> (y >> 16);
        const int64_t yDiff         = y - (ySrc << 16);
        const int64_t one_min_yDiff = 65536 - yDiff;
        const size_t  yIndex        = ySrc * srcWidth;
              int64_t x             = srcRect.x << 16;

        for (size_t j = 0 ; j < trgRect.w; j++) {

            const int32_t xSrc          = static_cast<int32_t> (x >> 16);
            const int64_t xDiff         = x - (xSrc << 16);
            const int64_t one_min_xDiff = 65536 - xDiff;
            const size_t  srcIndex      = yIndex + xSrc;

            const uint8_t A = src[srcIndex];
            const uint8_t B = src[srcIndex + 1];
            const uint8_t C = src[srcIndex + srcWidth];
            const uint8_t D = src[srcIndex + srcWidth + 1];

            const uint32_t alpha = ((  A * one_min_xDiff * one_min_yDiff
                                     + B * xDiff * one_min_yDiff
                                     + C * yDiff * one_min_xDiff
                                     + D * xDiff * yDiff ) >> 32 );

            target[trgIndex + j] = alpha << surfaceAShift; //SDL_MapRGBA(trgSurface->format, 0, 0, 0, alpha);
          
            x += xRatio;
        }
       y += yRatio;
       trgIndex += trgSurface->w;
    }
} /// pragma omp parallel
}

/**
** Simple zoom and render Fog Of War texture into SDL surface
** 
**  @param src          Image src.
**  @param srcRect      Rectangle in the src image to render
**  @param srcWidth     Image width
**  @param trgSurface   Where to render
**  @param trgRect      Scale src rectangle to this rectangle
**
*/
void CFogOfWar::UpscaleSimple(const uint8_t *src, const SDL_Rect &srcRect, const int16_t srcWidth,
                              SDL_Surface *const trgSurface, const SDL_Rect &trgRect) 
{
    const uint16_t surfaceAShift = trgSurface->format->Ashift;

    const uint8_t texelWidth  = PixelTileSize.x / 4;
    const uint8_t texelHeight = PixelTileSize.y / 4;
    
    uint32_t *const target = static_cast<uint32_t *>(trgSurface->pixels);
    intptr_t srcIndex = srcRect.x + srcRect.y * srcWidth;
    intptr_t trgIndex = trgRect.x + trgRect.y * trgSurface->w;

    for (uint16_t ySrc = 0; ySrc < srcRect.h; ySrc++) {
        for (uint16_t xSrc = 0; xSrc < srcRect.w; xSrc++) {
 
            const uint32_t texelValue = static_cast<uint32_t>(src[srcIndex + xSrc]) << surfaceAShift;
            std::fill_n(&target[trgIndex + xSrc * texelWidth], texelWidth, texelValue);
        }
        for (uint8_t texelRow = 1; texelRow < texelHeight; texelRow++) {
            std::copy_n(&target[trgIndex], trgRect.w, &target[trgIndex + texelRow * trgSurface->w]);
        }
        srcIndex += srcWidth;
        trgIndex += trgSurface->w * texelHeight;
    }
}

//@}
