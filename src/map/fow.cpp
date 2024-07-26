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
//      (c) Copyright 1999-2021 by Lutz Sammer, Vladi Shabanski,
//		Russell Smith, Jimmy Salmon, Pali Rohár, Andrettin and Alyokhin
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

#include "fow.h"

#include "../video/intern_video.h"
#include "map.h"
#include "player.h"
#include "stratagus.h"
#include "tile.h"
#include "ui.h"
#include "viewport.h"

#include <algorithm>

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/
/// FIXME: Maybe move it into CMap
std::unique_ptr<CFogOfWar> FogOfWar; /// Fog of war itself
std::shared_ptr<CGraphic> CFogOfWar::TiledFogSrc; // Graphic tiles set for tiled fog

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/
void CFogOfWar::SetTiledFogGraphic(const fs::path &fogGraphicFile)
{
	CFogOfWar::TiledFogSrc = CGraphic::New(fogGraphicFile.string(), PixelTileSize.x, PixelTileSize.y);
}

/// Calculate values of upscale table for explored/unexplored tiles
void CFogOfWar::GenerateUpscaleTables(uint32_t (&table)[16][4], const uint8_t alphaFrom, const uint8_t alphaTo)
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
             this->VisTableWidth  = Map.Info.MapWidth  + 2;
    const uint16_t visTableHeight = Map.Info.MapHeight + 2;
    const size_t   tableSize      = VisTableWidth * visTableHeight;
    VisTable.clear();
    VisTable.resize(tableSize);
    ranges::fill(VisTable, VisionType::cUnseen);

    VisTable_Index0 = VisTableWidth + 1;

    switch (Settings.Type) {
        case FogOfWarTypes::cTiled:
        case FogOfWarTypes::cTiledLegacy:

            InitTiled();
            break;

        case FogOfWarTypes::cEnhanced:

            InitEnhanced();
            break;

        default:
            break;
    }

    /// TODO: Add fog initialization for replays and observer players
    VisionFor.clear();
    ShowVisionFor(*ThisPlayer);

    this->State = cFirstEntry;
}

/**
**  Initialize the tiled fog of war.
**  Build tables, setup functions.
*/
void CFogOfWar::InitTiled()
{
	if (TiledAlphaFog || TileOfFogOnly) {
		this->Clean();
	}
    CFogOfWar::TiledFogSrc->Load();

    if (Settings.Type == FogOfWarTypes::cTiledLegacy) {
        TileOfFogOnly = SDL_CreateRGBSurface(SDL_SWSURFACE, PixelTileSize.x, PixelTileSize.y,
                                             32, RMASK, GMASK, BMASK, AMASK);
        SDL_FillRect(TileOfFogOnly, nullptr, Settings.FogColorSDL | uint32_t(Settings.ExploredOpacity) << ASHIFT);
    }

    SDL_Surface * const newFogSurface = SDL_ConvertSurfaceFormat(CFogOfWar::TiledFogSrc->getSurface(),
                                                                 SDL_MasksToPixelFormatEnum(32, RMASK, GMASK, BMASK, AMASK), 0);
    TiledAlphaFog = CGraphic::New("");
    TiledAlphaFog->setSurface(newFogSurface);
    TiledAlphaFog->Width = PixelTileSize.x;
    TiledAlphaFog->Height = PixelTileSize.y;
    TiledAlphaFog->NumFrames = 16;
    TiledAlphaFog->GenFramesMap();
    SDL_SetSurfaceBlendMode(TiledAlphaFog->getSurface(), SDL_BLENDMODE_BLEND);
}

/**
**  Initialize the enhanced fog of war.
**  Build tables, setup functions.
*/
void CFogOfWar::InitEnhanced()
{

    /// +1 to the top & left for 4x scale algorithm purposes,
    const uint16_t fogTextureWidth  = (Map.Info.MapWidth  + 1) * 4;
    const uint16_t fogTextureHeight = (Map.Info.MapHeight + 1) * 4;

    FogTexture.Init(fogTextureWidth, fogTextureHeight, Settings.NumOfEasingSteps);

    RenderedFog.clear();
    RenderedFog.resize(Map.Info.MapWidth * Map.Info.MapHeight * 16);
    ranges::fill(RenderedFog, 0xFF);

    Blurer.Init(fogTextureWidth, fogTextureHeight, Settings.BlurRadius[Settings.UpscaleType], Settings.BlurIterations);

    SetFogColor(Settings.FogColor);
}

void CFogOfWar::SetFogColor(const uint8_t r, const uint8_t g, const uint8_t b)
{
    SetFogColor(CColor(r, g, b));
}

void CFogOfWar::SetFogColor(const CColor color)
{
    Settings.FogColor = color;
    Settings.FogColorSDL = (color.R << RSHIFT) | (color.G << GSHIFT) | (color.B << BSHIFT);
}

void CFogOfWar::SetEasingSteps(const uint8_t num)
{
    Settings.NumOfEasingSteps = num;
    FogTexture.SetNumOfSteps(num);
}

void CFogOfWar::Clean(const bool isHardClean /*= false*/)
{
    if(isHardClean) {
        VisionFor.clear();
    }

    VisTable.clear();
    VisTableWidth   = 0;
    VisTable_Index0 = 0;

    switch (Settings.Type) {
        case FogOfWarTypes::cTiled:
        case FogOfWarTypes::cTiledLegacy:

            CleanTiled(isHardClean);
            break;

        case FogOfWarTypes::cEnhanced:
            if (isHardClean) {
                CleanTiled(isHardClean);
            }
            FogTexture.Clean();
            RenderedFog.clear();
            Blurer.Clean();
            break;

        default:
            break;
    }
}

/**
** Select which type of Fog of War to use
**
** @param fowType	type to set
** @return true if success, false for wrong fow_type
*/
bool CFogOfWar::SetType(const FogOfWarTypes fowType)
{
	if (fowType != Settings.Type && fowType < FogOfWarTypes::cNumOfTypes) {
        if (Map.isInitialized()) {
            this->Clean();
            Settings.Type = fowType;
            this->Init();
        } else {
            Settings.Type = fowType;
        }
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
    Settings.ExploredOpacity = explored;
    Settings.RevealedOpacity = revealed;
    Settings.UnseenOpacity   = unseen;
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
**
*/
void CFogOfWar::GenerateFog()
{
    /// FIXME: Maybe to update this with every change of shared vision
    std::set<uint8_t> playersToRenderView;
    for (const uint8_t player : VisionFor) {
        playersToRenderView.insert(player);
        for (const uint8_t playersSharedVision : Players[player].GetSharedVision()) {
            playersToRenderView.insert(playersSharedVision);
        }
    }
    CurrUpscaleTableExplored = GameSettings.RevealMap != MapRevealModes::cHidden ? UpscaleTableRevealed : UpscaleTableExplored;

    const uint8_t visibleThreshold = Map.NoFogOfWar ? 1 : 2;

    #pragma omp parallel
    {
        const uint16_t thisThread   = omp_get_thread_num();
        const uint16_t numOfThreads = omp_get_num_threads();
        const uint16_t lBound = (thisThread    ) * Map.Info.MapHeight / numOfThreads;
        const uint16_t uBound = (thisThread + 1) * Map.Info.MapHeight / numOfThreads;

        for (uint16_t row = lBound; row < uBound; row++) {

            const size_t visIndex = VisTable_Index0 + row * VisTableWidth;
            const size_t mapIndex = size_t(row) * Map.Info.MapWidth;

            for (uint16_t col = 0; col < Map.Info.MapWidth; col++) {

                uint8_t &visCell = VisTable[visIndex + col];
                visCell = 0; /// Clear it before check for players
                const CMapField *mapField = Map.Field(mapIndex + col);
                for (const uint8_t player : playersToRenderView) {
                    visCell = std::max<uint8_t>(visCell, mapField->playerInfo.Visible[player]);
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
    if (Settings.Type == FogOfWarTypes::cTiled || Settings.Type == FogOfWarTypes::cTiledLegacy) {
        if (doAtOnce || this->State == States::cFirstEntry){
            GenerateFog();
            this->State = States::cGenerateFog;
        } else {
            switch (this->State) {
                case States::cReady:
                    this->State = cGenerateFog;
                    break;

                case States::cGenerateFog:
                    GenerateFog();
					[[fallthrough]];
                default:
                    this->State++;
                    break;
            }
        }
        return;
    }

    /// FogOfWarTypes::cEnhanced
    FogTexture.Ease();

    if (Settings.NumOfEasingSteps < States::cReady) doAtOnce = true;

    if (doAtOnce || this->State == States::cFirstEntry) {
        GenerateFog();
        FogUpscale4x4();
        Blurer.Blur(FogTexture.GetNext());
        FogTexture.PushNext(doAtOnce);
        this->State = States::cGenerateFog;
    } else {
        switch (this->State) {
            case States::cGenerateFog:
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
void CFogOfWar::Draw(CViewport &viewport)
{
    if (Settings.Type == FogOfWarTypes::cTiledLegacy) {
        DrawTiledLegacy(viewport);
    } else {
        SDL_FillRect(viewport.GetFogSurface(), nullptr, 0x00);

        if (Settings.Type == FogOfWarTypes::cTiled) {
            DrawTiled(viewport);
        } else if (Settings.Type == FogOfWarTypes::cEnhanced) {
            DrawEnhanced(viewport);
        }
    }
}


/**
**  Draw enhanced fog of war texture into certain viewport's surface.
**
**  @param viewport     viewport to generate fog of war texture for
**  @param vpFogSurface surface where to put the generated texture
*/
void CFogOfWar::DrawEnhanced(CViewport &viewport)
{
    SDL_Rect srcRect;
    srcRect.x = (viewport.MapPos.x + 1) * 4 - 2;  /// '+1' because of 1 tile frame around the texture
    srcRect.y = (viewport.MapPos.y + 1) * 4 - 2;  /// '-2' is a half-tile compensation
    srcRect.w = viewport.MapWidth  * 4;
    srcRect.h = viewport.MapHeight * 4;

    const uint16_t x0 = viewport.MapPos.x * 4;
    const uint16_t y0 = viewport.MapPos.y * 4;

    FogTexture.DrawRegion(RenderedFog.data(), Map.Info.MapWidth * 4, x0, y0, srcRect);

    /// TODO: This part might be replaced by GPU shaders.
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
            UpscaleBilinear(RenderedFog.data(), srcRect, Map.Info.MapWidth * 4, viewport.GetFogSurface(), trgRect);
            break;
        case cSimple:
        default:
            UpscaleSimple(RenderedFog.data(), srcRect, Map.Info.MapWidth * 4, viewport.GetFogSurface(), trgRect);
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

/**
**  Draw only fog of war
**
**  @param x  X position into video memory
**  @param y  Y position into video memory
*/
void CFogOfWar::DrawFullShroudOfFog(int16_t x, int16_t y, const uint8_t alpha, SDL_Surface *const vpFogSurface)
{
	int oldx;
	int oldy;
	SDL_Rect srect;
	SDL_Rect drect;

	srect.x = 0;
	srect.y = 0;
	srect.w = PixelTileSize.x;
	srect.h = PixelTileSize.y;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	drect.x = x;
	drect.y = y;

    if (vpFogSurface == TheScreen) { /// FogOfWarTypes::cTiledLegacy
        SDL_BlitSurface(TileOfFogOnly, &srect, TheScreen, &drect);
    } else {
        const uint32_t fogColor = GetFogColorSDL() | (uint32_t(alpha) << ASHIFT);
        size_t index = drect.y * vpFogSurface->w + drect.x;
        uint32_t *const dst = reinterpret_cast<uint32_t*>(vpFogSurface->pixels);
        for (uint16_t row = 0; row < srect.h; row++) {
            std::fill_n(&dst[index], srect.w, fogColor);
            index += vpFogSurface->w;
        }
    }
}

void CFogOfWar::GetFogTile(const size_t visIndex, const  size_t mapIndex, const size_t mapIndexBase,
                           int *fogTile, int *blackFogTile) const
{
	int w = Map.Info.MapWidth;
	int fogTileIndex = 0;
	int blackFogTileIndex = 0;
	int x = mapIndex - mapIndexBase;

	if (ReplayRevealMap) {
		*fogTile = 0;
		*blackFogTile = 0;
		return;
	}

	//
	//  Which Tile to draw for fog
	//
	// Investigate tiles around current tile
	// 1 2 3
	// 4 * 5
	// 6 7 8

	//    2  3 1
	//   10 ** 5
	//    8 12 4

    const size_t visIndexBase = (visIndex - x);
	if (mapIndexBase) {
		const size_t index = visIndexBase - VisTableWidth;
		if (mapIndex != mapIndexBase) {
			if (!IsMapFieldExplored(x - 1 + index)) {
				blackFogTileIndex |= 2;
				fogTileIndex |= 2;
			} else if (!IsMapFieldVisible(x - 1 + index)) {
				fogTileIndex |= 2;
			}
		}
		if (!IsMapFieldExplored(x + index)) {
			blackFogTileIndex |= 3;
			fogTileIndex |= 3;
		} else if (!IsMapFieldVisible(x + index)) {
			fogTileIndex |= 3;
		}
		if (mapIndex != mapIndexBase + w - 1) {
			if (!IsMapFieldExplored(x + 1 + index)) {
				blackFogTileIndex |= 1;
				fogTileIndex |= 1;

			} else if (!IsMapFieldVisible(x + 1 + index)) {
				fogTileIndex |= 1;
			}
		}
	}

	if (mapIndex != mapIndexBase) {
		const size_t index = visIndexBase;
		if (!IsMapFieldExplored(x - 1 + index)) {
			blackFogTileIndex |= 10;
			fogTileIndex |= 10;
		} else if (!IsMapFieldVisible(x - 1 + index)) {
			fogTileIndex |= 10;
		}
	}
	if (mapIndex != mapIndexBase + w - 1) {
		const size_t index = visIndexBase;
		if (!IsMapFieldExplored(x + 1 + index)) {
			blackFogTileIndex |= 5;
			fogTileIndex |= 5;
		} else if (!IsMapFieldVisible(x + 1 + index)) {
			fogTileIndex |= 5;
		}
	}

	if (mapIndexBase + w < static_cast<unsigned int>(Map.Info.MapHeight) * w) {
		const size_t index = visIndexBase + VisTableWidth;
		if (mapIndex != mapIndexBase) {
			if (!IsMapFieldExplored(x - 1 + index)) {
				blackFogTileIndex |= 8;
				fogTileIndex |= 8;
			} else if (!IsMapFieldVisible(x - 1 + index)) {
				fogTileIndex |= 8;
			}
		}
		if (!IsMapFieldExplored(x + index)) {
			blackFogTileIndex |= 12;
			fogTileIndex |= 12;
		} else if (!IsMapFieldVisible(x + index)) {
			fogTileIndex |= 12;
		}
		if (mapIndex != mapIndexBase + w - 1) {
			if (!IsMapFieldExplored(x + 1 + index)) {
				blackFogTileIndex |= 4;
				fogTileIndex |= 4;
			} else if (!IsMapFieldVisible(x + 1 + index)) {
				fogTileIndex |= 4;
			}
		}
	}

	*fogTile = this->TiledFogTable[fogTileIndex];
	*blackFogTile = this->TiledFogTable[blackFogTileIndex];
}

/**
**  Draw fog of war tile.
**
**  @param visIndex  Offset in fields to current tile in VisTable
**  @param mapIndex  Offset in fields to current tile on the map
**  @param mapIndexBase  Start of the current row on the map
**  @param dx  X position into fog surface
**  @param dy  Y position into fog surface
**  @param vpFogSurface surface to draw fog
*/
void CFogOfWar::DrawFogTile(const size_t visIndex, const size_t mapIndex, const size_t mapIndexBase,
                            const int16_t dx, const int16_t dy, SDL_Surface *const vpFogSurface)
{
	int fogTile = 0;
	int blackFogTile = 0;

	GetFogTile(visIndex, mapIndex, mapIndexBase, &fogTile, &blackFogTile);

    if (vpFogSurface != TheScreen) {
        if (IsMapFieldVisible(visIndex) || ReplayRevealMap) {
            if (fogTile && fogTile != blackFogTile) {
                TiledAlphaFog->DrawFrameClipCustomMod(fogTile, dx, dy, PixelModifier::CopyWithSrcAlphaKey,
                                                                       GetExploredOpacity(),
                                                                       vpFogSurface);
            }
        } else {
            DrawFullShroudOfFog(dx, dy, FogOfWar->GetExploredOpacity(), vpFogSurface);
        }
        if (blackFogTile) {
            TiledAlphaFog->DrawFrameClipCustomMod(blackFogTile, dx, dy, PixelModifier::CopyWithSrcAlphaKey,
                                                                        GameSettings.RevealMap != MapRevealModes::cHidden ? GetRevealedOpacity()
                                                                                               : GetUnseenOpacity(),
                                                                        vpFogSurface);
        }
    } else { /// legacy draw tiled fog into TheScreen surface (for slow machines)
        if (IsMapFieldVisible(visIndex) || ReplayRevealMap) {
            if (fogTile && fogTile != blackFogTile) {
                TiledAlphaFog->DrawFrameClipTrans(fogTile, dx, dy, GetExploredOpacity());
            }
        } else {
            DrawFullShroudOfFog(dx, dy, GetExploredOpacity(), TheScreen);
        }
        if (blackFogTile) {
            TiledFogSrc->DrawFrameClip(blackFogTile, dx, dy);
        }
    }
}
/**
**  Draw tiled fog of war texture into certain viewport's surface.
**
**  @param viewport     viewport to generate fog of war texture for
*/
void CFogOfWar::DrawTiled(CViewport &viewport)
{
    /// Save current clipping
    PushClipping();

    // Set clipping to FogSurface coordinates
	SDL_Rect fogSurfaceClipRect {viewport.Offset.x,
							 	 viewport.Offset.y,
							 	 viewport.BottomRightPos.x - viewport.TopLeftPos.x + 1,
							 	 viewport.BottomRightPos.y - viewport.TopLeftPos.y + 1};
    SetClipping(fogSurfaceClipRect.x,
				fogSurfaceClipRect.y,
				fogSurfaceClipRect.x + fogSurfaceClipRect.w,
				fogSurfaceClipRect.y + fogSurfaceClipRect.h);

	const int ex = fogSurfaceClipRect.x + fogSurfaceClipRect.w;
	const int ey = fogSurfaceClipRect.y + fogSurfaceClipRect.h;

    #pragma omp parallel
    {
        const uint16_t thisThread   = omp_get_thread_num();
        const uint16_t numOfThreads = omp_get_num_threads();

        uint16_t lBound = thisThread * fogSurfaceClipRect.h / numOfThreads;
        lBound -= lBound % PixelTileSize.y;
        uint16_t uBound = ey;
        if (thisThread != numOfThreads - 1) {
            uBound = (thisThread + 1) * fogSurfaceClipRect.h / numOfThreads;
            uBound -= uBound % PixelTileSize.y;
        }

        size_t mapIndexBase = (viewport.MapPos.y + lBound / PixelTileSize.y) * Map.Info.MapWidth;
        size_t visIndexBase = (viewport.MapPos.y + lBound / PixelTileSize.y) * VisTableWidth + VisTable_Index0;

        int dy = lBound;
        while (dy < uBound) {
            size_t mapIndex = viewport.MapPos.x + mapIndexBase;
            size_t visIndex = viewport.MapPos.x + visIndexBase;

            int dx = 0;
            while (dx < ex) {
                if (VisTable[visIndex]) {
                    DrawFogTile(visIndex, mapIndex, mapIndexBase, dx, dy, viewport.GetFogSurface());
                } else {
                    DrawFullShroudOfFog(dx, dy, GameSettings.RevealMap != MapRevealModes::cHidden ? GetRevealedOpacity()
                                                                       : GetUnseenOpacity(),
                                                viewport.GetFogSurface());
                }
                mapIndex++;
                visIndex++;
                dx += PixelTileSize.x;
            }
            mapIndexBase += Map.Info.MapWidth;
            visIndexBase += VisTableWidth;
            dy += PixelTileSize.y;
        }
    } /// pragma omp parallel

	// Restore Clipping to Viewport coordinates
	PopClipping();

}
/**
**  Legacy draw tiled fog of war texture into TheScreen surface.
**
**  @param viewport     viewport to generate fog of war texture for
*/
void CFogOfWar::DrawTiledLegacy(CViewport &viewport)
{
	const int ex = viewport.BottomRightPos.x;
	const int ey = viewport.BottomRightPos.y;

    size_t mapIndexBase = viewport.MapPos.y * Map.Info.MapWidth;
    size_t visIndexBase = viewport.MapPos.y * VisTableWidth + VisTable_Index0;
    int dy = viewport.TopLeftPos.y - viewport.Offset.y;

    while (dy < ey) {
        size_t mapIndex = viewport.MapPos.x + mapIndexBase;
        size_t visIndex = viewport.MapPos.x + visIndexBase;

        int dx = viewport.TopLeftPos.x - viewport.Offset.x;

        while (dx < ex) {
            if (VisTable[visIndex]) {
                DrawFogTile(visIndex, mapIndex, mapIndexBase, dx, dy, TheScreen);
            } else {
                Video.FillRectangleClip(Settings.FogColorSDL, dx, dy, PixelTileSize.x, PixelTileSize.y);
            }
            mapIndex++;
            visIndex++;
            dx += PixelTileSize.x;
        }
        mapIndexBase += Map.Info.MapWidth;
        visIndexBase += VisTableWidth;
        dy += PixelTileSize.y;
    }
}
/**
**  Cleanup the fog of war.
*/
void CFogOfWar::CleanTiled(const bool isHardClean /*= false*/)
{
	if (isHardClean) {
		CFogOfWar::TiledFogSrc = nullptr;
	}
	if (TileOfFogOnly) {
		SDL_FreeSurface(TileOfFogOnly);
		TileOfFogOnly = nullptr;
	}
	TiledAlphaFog = nullptr;
}
//@}
