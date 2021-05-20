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
#include <omp.h>

#include "stratagus.h"

#include "fow.h"
#include "map.h"
#include "player.h"
#include "tile.h"
#include "ui.h"
#include "viewport.h"

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
/// Calculate values of upscale table for explored/unexplored tiles
void CFogOfWar::GenerateUpscaleTables(uint32_t (*table)[4], const uint8_t alphaFrom, const uint8_t alphaTo)
{
    for (auto i = 0; i < 16; i++) {
        for (auto j = 0; j < 4; j++) {
            table[i][j] = 0;
            for (auto pos = 0; pos < 4; pos ++) {
                uint32_t initValue {0};
                switch ((UpscaleTable_4x4[i][j] >> (8 * pos)) & 0xFF) {
                    case 0xFF: // full
                        initValue = alphaTo - alphaFrom;
                    break;
                    case 0x7F: // half
                        initValue = (alphaTo - alphaFrom) / 2;
                    break;
                    default:   // zero
                        initValue = 0;
                }
                table[i][j] |= initValue << (pos * 8);
            }
        }
    }

}
void CFogOfWar::Init()
{
    /// +1 to the top & left and +1 to the bottom & right for 4x scale algorithm purposes, 
    /// Extra tiles will always be VisionType::cUnseen. 
                   VisTableWidth  = Map.Info.MapWidth  + 2;
    const uint16_t visTableHeight = Map.Info.MapHeight + 2;
    const size_t   tableSize      = VisTableWidth * visTableHeight;
    VisTable.clear();
    VisTable.resize(tableSize);
    std::fill(VisTable.begin(), VisTable.end(), VisionType::cUnseen);
    
    VisTable_Index0 = VisTableWidth + 1;

    /// +1 to the top & left for 4x scale algorithm purposes, 
    const uint16_t fogTextureWidth  = (Map.Info.MapWidth  + 1) * 4;
    const uint16_t fogTextureHeight = (Map.Info.MapHeight + 1) * 4;

    FogTexture.Init(fogTextureWidth, fogTextureHeight, Settings.NumOfEasingSteps);
    
    RenderedFog.clear();
    RenderedFog.resize(Map.Info.MapWidth * Map.Info.MapHeight * 16);
    std::fill(RenderedFog.begin(), RenderedFog.end(), 0xFF);

    Blurer.Init(fogTextureWidth, fogTextureHeight, Settings.BlurRadius[Settings.UpscaleType], Settings.BlurIterations);

    SetFogColor(Settings.FogColor);
    
    /// TODO: Add fog initialization for replays and observer players
    ShowVisionFor(*ThisPlayer);

    this->State = cFirstEntry;
}

void CFogOfWar::SetFogColor(uint8_t r, uint8_t g, uint8_t b)
{
    SetFogColor(CColor(r, g, b));
}

void CFogOfWar::SetFogColor(CColor color)
{
    Settings.FogColor = color;
    Settings.FogColorSDL = (color.R << RSHIFT) | (color.G << GSHIFT) | (color.B << BSHIFT);
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
** @param fowType	type to set
** @return true if success, false for wrong fow_type 
*/
bool CFogOfWar::SetType(const FogOfWarTypes fowType)
{
	if (fowType != Settings.FOW_Type && fowType < FogOfWarTypes::cNumOfTypes) {
        if (Map.isInitialized()) {
            switch (fowType) {
                case FogOfWarTypes::cLegacy:
                    this->Clean();
                    Map.InitLegacyFogOfWar();
                    break;
                case FogOfWarTypes::cEnhanced:
                    Map.CleanLegacyFogOfWar();
                    this->Init();
                    break;
                default:
                    break;
            }
        }
    	Settings.FOW_Type = fowType;
		return true;
	} else {
		return false;
	}
}

/** 
** Set fog of war opacity (alpha chanel values) for different levels of visibility
** 
** @param explored  alpha channel value for explored tiles
** @param revealed  alpha channel value for revealed tiles (when the map revealed)
** @param unseen    alpha channel value for unseen tiles
** 
*/
void CFogOfWar::SetOpacityLevels(const uint8_t explored, const uint8_t revealed, const uint8_t unseen)
{
    this->Settings.ExploredOpacity = explored;
    this->Settings.RevealedOpacity = revealed;
    this->Settings.UnseenOpacity   = unseen;
    GenerateUpscaleTables(UpscaleTableVisible, 0, explored);
    GenerateUpscaleTables(UpscaleTableExplored, explored, unseen);
    GenerateUpscaleTables(UpscaleTableRevealed, explored, revealed);
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
    Settings.UpscaleType = enable ? UpscaleTypes::cBilinear : UpscaleTypes::cSimple;
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

/** 
** Generate fog of war:
** fill map-sized table with values of visiblty for current player/players 
** TODO: Add posibility to select players (actual for replays or observers)
**
** 
*/
void CFogOfWar::GenerateFog()
{
    /// FIXME: Maybe to move this set into the CFogOfWar and recalt with every change of shared vision
    std::set<uint8_t> playersToRenderView;
    for (const uint8_t player : VisionFor) {
        playersToRenderView.insert(player);
        for (const uint8_t playersSharedVision : Players[player].GetSharedVision()) {
            playersToRenderView.insert(playersSharedVision);
        }
    }
    CurrUpscaleTableExplored = GameSettings.RevealMap ? UpscaleTableRevealed
                                                      : UpscaleTableExplored;

    const uint8_t visibleThreshold = Map.NoFogOfWar ? 1 : 2;
    
    #pragma omp parallel
    {
        const uint16_t thisThread   = omp_get_thread_num();
        const uint16_t numOfThreads = omp_get_num_threads();
        
        const uint16_t lBound = (thisThread    ) * Map.Info.MapHeight / numOfThreads;
        const uint16_t uBound = (thisThread + 1) * Map.Info.MapHeight / numOfThreads;

        for (uint16_t row = lBound; row < uBound; row++) {

            const size_t visIndex = VisTable_Index0 + row * VisTableWidth;
            const size_t mapIndex = size_t(row) * Map.Info.MapHeight;

            for (uint16_t col = 0; col < Map.Info.MapWidth; col++) {

                uint8_t &visCell = VisTable[visIndex + col];
                visCell = 0; /// Clear it before check for players
                const CMapField *mapField = Map.Field(mapIndex + col);
                for (const uint8_t player : playersToRenderView) {
                    visCell = std::max<uint8_t>(visCell, mapField->playerInfo.Visible[player]); // mapField->playerInfo.TeamVisibilityState(Players[player]));
                    if (visCell >= visibleThreshold) {
                        visCell = 2;
                        break;
                    }
                }
            }
        }
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
    if (Settings.FOW_Type == FogOfWarTypes::cLegacy) {
        return;
    }

    FogTexture.Ease();

    if (Settings.NumOfEasingSteps < States::cReady) doAtOnce = true;

    if (doAtOnce || this->State == States::cFirstEntry) {
        /// TODO: Add posibility to generate fog for different players (for replays purposes)
        GenerateFog();
        FogUpscale4x4();
        Blurer.Blur(FogTexture.GetNext());
        FogTexture.PushNext(doAtOnce);
        this->State = States::cGenerateFog;
    } else {
        switch (this->State) {
            case States::cGenerateFog:
                /// TODO: Add posibility to generate fog for different players (for replays purposes)
                GenerateFog();
                this->State++;
                break;

            case States::cGenerateTexture:
                FogUpscale4x4();
                this->State++;
                break;

            case States::cBlurTexture:
                Blurer.Blur(FogTexture.GetNext());
                this->State++;
                break;
                
            case States::cReady:
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
**  Generate fog of war texture for certain viewport.
**
**  @param viewport     viewport to generate fog of war texture for
**  @param vpFogSurface surface where to put the generated texture
*/
void CFogOfWar::GetFogForViewport(const CViewport &viewport, SDL_Surface *const vpFogSurface)
{

    SDL_Rect srcRect;
    srcRect.x = (viewport.MapPos.x + 1) * 4 - 2;  /// '+1' because of 1 tile frame around the texture 
    srcRect.y = (viewport.MapPos.y + 1) * 4 - 2;  /// '-2' is a half-tile compensation
    srcRect.w = viewport.MapWidth  * 4;
    srcRect.h = viewport.MapHeight * 4;
    
    const uint16_t x0 = viewport.MapPos.x * 4;
    const uint16_t y0 = viewport.MapPos.y * 4;

    FogTexture.DrawRegion(RenderedFog.data(), Map.Info.MapWidth * 4, x0, y0, srcRect);

    /// TOTO: This part may be replaced by GPU shaders. 
    /// In that case vpFogSurface shall be filled up with FogTexture.DrawRegion()

    srcRect.x = x0;
    srcRect.y = y0;
    
    SDL_Rect trgRect;
    trgRect.x = 0;
    trgRect.y = 0;
    trgRect.w = viewport.MapWidth  * PixelTileSize.x;
    trgRect.h = viewport.MapHeight * PixelTileSize.y;
   
    switch (this->Settings.UpscaleType) {
        case cBilinear:
            UpscaleBilinear(RenderedFog.data(), srcRect, Map.Info.MapWidth * 4, vpFogSurface, trgRect);
            break;
        case cSimple:
        default:
            UpscaleSimple(RenderedFog.data(), srcRect, Map.Info.MapWidth * 4, vpFogSurface, trgRect);
            break;
    }
}

/**
**  4x4 upscale generated fog of war texture
**
*/
void CFogOfWar::FogUpscale4x4()
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
    uint32_t *const fogTexture = (uint32_t*)FogTexture.GetNext();
    
    /// Fog texture width and height in 32bit chunks
    const uint16_t textureWidth  = FogTexture.GetWidth()  / 4;
    const uint16_t textureHeight = FogTexture.GetHeight() / 4;
    const uint16_t nextRowOffset = textureWidth * 4;

    #pragma omp parallel
    {

        const uint16_t thisThread   = omp_get_thread_num();
        const uint16_t numOfThreads = omp_get_num_threads();
        
        const uint16_t lBound = (thisThread    ) * textureHeight / numOfThreads;
        const uint16_t uBound = (thisThread + 1) * textureHeight / numOfThreads;

        /// in fact it's viewport.MapPos.y -1 & viewport.MapPos.x -1 because of VisTable starts from [-1:-1]
        size_t visIndex      = lBound * VisTableWidth;
        size_t textureIndex  = lBound * nextRowOffset;
        
        for (uint16_t row = lBound; row < uBound; row++) {
            for (uint16_t col = 0; col < textureWidth; col++) {
                /// Fill the 4x4 scaled tile
                FillUpscaledRec(fogTexture, textureWidth, textureIndex + col, 
                                DeterminePattern(visIndex + col, VisionType::cVisible), 
                                DeterminePattern(visIndex + col, VisionType::cVisible | VisionType::cExplored));
            }
            visIndex     += VisTableWidth;
            textureIndex += nextRowOffset;
        }
    } // pragma omp parallel
}

/**
** Bilinear zoom Fog Of War texture into SDL surface
** 
**  @param src          Image src.
**  @param srcRect      Rectangle in the src image to render
**  @param srcWidth     Image width
**  @param trgSurface   Where to render
**  @param trgRect      Scale src rectangle to this rectangle
**
*/
void CFogOfWar::UpscaleBilinear(const uint8_t *const src, const SDL_Rect &srcRect, const int16_t srcWidth,
                                SDL_Surface *const trgSurface, const SDL_Rect &trgRect) const
{
    constexpr int32_t fixedOne = 65536;

    uint32_t *const target = (uint32_t*)trgSurface->pixels;
    const uint16_t AShift = trgSurface->format->Ashift;
    
    /// FIXME: '-1' shouldn't be here, but without it the resulting fog has a shift to the left and upward
    const int32_t xRatio = (int32_t(srcRect.w - 1) << 16) / trgRect.w;
    const int32_t yRatio = (int32_t(srcRect.h - 1) << 16) / trgRect.h;
    
    #pragma omp parallel
    {    
        const uint16_t thisThread   = omp_get_thread_num();
        const uint16_t numOfThreads = omp_get_num_threads();
        
        const uint16_t lBound = (thisThread    ) * trgRect.h / numOfThreads; 
        const uint16_t uBound = (thisThread + 1) * trgRect.h / numOfThreads; 


        size_t  trgIndex = size_t(trgRect.y + lBound) * trgSurface->w + trgRect.x;
        int64_t y        = ((int32_t)srcRect.y << 16) + lBound * yRatio;

        for (uint16_t yTrg = lBound; yTrg < uBound; yTrg++) {

            const int32_t ySrc          = int32_t(y >> 16);
            const int64_t yDiff         = y - (ySrc << 16);
            const int64_t one_min_yDiff = fixedOne - yDiff;
            const size_t  yIndex        = ySrc * srcWidth;
                  int64_t x             = int32_t(srcRect.x) << 16;

            for (uint16_t xTrg = 0; xTrg < trgRect.w; xTrg++) {

                const int32_t xSrc          = int32_t(x >> 16);
                const int64_t xDiff         = x - (xSrc << 16);
                const int64_t one_min_xDiff = fixedOne - xDiff;
                const size_t  srcIndex      = yIndex + xSrc;

                const uint8_t A = src[srcIndex];
                const uint8_t B = src[srcIndex + 1];
                const uint8_t C = src[srcIndex + srcWidth];
                const uint8_t D = src[srcIndex + srcWidth + 1];

                const uint32_t alpha = ((  A * one_min_xDiff * one_min_yDiff
                                         + B * xDiff * one_min_yDiff
                                         + C * yDiff * one_min_xDiff
                                         + D * xDiff * yDiff ) >> 32 );

                target[trgIndex + xTrg] = (alpha << AShift) | Settings.FogColorSDL;
                x += xRatio;
            }
            y += yRatio;
            trgIndex += trgSurface->w;
        }
    } /// pragma omp parallel
}

/**
** Simple zoom Fog Of War texture into SDL surface
** 
**  @param src          Image src.
**  @param srcRect      Rectangle in the src image to render
**  @param srcWidth     Image width
**  @param trgSurface   Where to render
**  @param trgRect      Scale src rectangle to this rectangle
**
*/
void CFogOfWar::UpscaleSimple(const uint8_t *src, const SDL_Rect &srcRect, const int16_t srcWidth,
                              SDL_Surface *const trgSurface, const SDL_Rect &trgRect) const
{
    const uint16_t surfaceAShift = trgSurface->format->Ashift;

    const uint8_t texelWidth  = PixelTileSize.x / 4;
    const uint8_t texelHeight = PixelTileSize.y / 4;
    
    uint32_t *const target =(uint32_t*)trgSurface->pixels;

    #pragma omp parallel
    {    
        const uint16_t thisThread   = omp_get_thread_num();
        const uint16_t numOfThreads = omp_get_num_threads();
        
        const uint16_t lBound = (thisThread    ) * srcRect.h / numOfThreads; 
        const uint16_t uBound = (thisThread + 1) * srcRect.h / numOfThreads; 

        size_t srcIndex = size_t(srcRect.y + lBound) * srcWidth + srcRect.x;
        size_t trgIndex = size_t(trgRect.y + lBound * texelHeight) * trgSurface->w + trgRect.x;

        for (uint16_t ySrc = lBound; ySrc < uBound; ySrc++) {
            for (uint16_t xSrc = 0; xSrc < srcRect.w; xSrc++) {
    
                const uint32_t texelValue = (uint32_t(src[srcIndex + xSrc]) << surfaceAShift) 
                                            | Settings.FogColorSDL;
                std::fill_n(&target[trgIndex + xSrc * texelWidth], texelWidth, texelValue);
            }
            for (uint8_t texelRow = 1; texelRow < texelHeight; texelRow++) {
                std::copy_n(&target[trgIndex], trgRect.w, &target[trgIndex + texelRow * trgSurface->w]);
            }
            srcIndex += srcWidth;
            trgIndex += trgSurface->w * texelHeight;
        }
    } /// pragma omp parallel
}
//@}
