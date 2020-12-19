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

#include "fow.h"
#include "map.h"
#include "player.h"
#include "tile.h"
#include "viewport.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/
float   CBlurer::Radius {1.5};
uint8_t CBlurer::NumOfIterations {2};

std::vector<uint8_t> CBlurer::BoxRadius; 


CFogOfWar::FogOfWarSettings CFogOfWar::Settings;

std::vector<uint8_t> CFogOfWar::VisTableCache;

intptr_t CFogOfWar::VisCache_Index0 = 0;
size_t   CFogOfWar::VisCacheWidth   = 0;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/
void CFogOfWar::Init()
{
    /// +1 to the top & left and +1 to the bottom & right for 4x scale algorithm purposes, 
    /// +1 to the bottom & right because of UI.MapArea.ScrollPadding
    /// Extra tiles will always be VisionType::cNone. 
    CFogOfWar::VisCacheWidth = Map.Info.MapHeight + 3;    
    CFogOfWar::VisCache_Index0 = CFogOfWar::VisCacheWidth + 1;

    size_t tableSize = CFogOfWar::VisCacheWidth * (Map.Info.MapHeight + 3);
    VisTableCache.clear();
    VisTableCache.resize(tableSize);
    ResetCache();

    CBlurer::Init();
}

void CFogOfWar::CleanCache()
{
    VisTableCache.clear();
    CFogOfWar::VisCacheWidth = 0;
    CFogOfWar::VisCache_Index0 = 0;
    
}

void CFogOfWar::ResetCache()
{
    std::fill(VisTableCache.begin(), VisTableCache.end(), 0);
}

/** 
** Select which type of Fog of War to use
** 
** @param fow_type	type to set
** @return true if success, false for wrong fow_type 
*/
bool CFogOfWar::SetType(const FogOfWarTypes fow_type)
{
	if (fow_type < FogOfWarTypes::NumOfTypes) {
		CFogOfWar::Settings.FOW_Type = fow_type;
		return true;
	} else {
		return false;
	}
}

/** 
** Returns used type of Fog of War 
** 
** @return current Fog of War type
*/
FogOfWarTypes CFogOfWar::GetType()
{
	return CFogOfWar::Settings.FOW_Type;
}

/**
**  Adjust viewport
**
**  @param viewport     viewport to adjust fog of war for
**  
*/
void CFogOfWar::AdjustToViewport(const CViewport &viewport)
{
    /// TODO: Add support for maps which smaller than viewport
    Assert(viewport.MapWidth <= Map.Info.MapWidth && viewport.MapHeight <= Map.Info.MapHeight);

    Clean();
    
    /// +1 to the top & left and +1 to the bottom & right for 4x scale algorithm purposes, 
    /// +1 to the bottom & right because of UI.MapArea.ScrollPadding
    uint16_t mapAreaWidth  = viewport.MapWidth + 3;
    uint16_t mapAreaHeight = viewport.MapHeight + 3;
    
    FogTextureWidth  = mapAreaWidth  * 4;
    FogTextureHeight = mapAreaHeight * 4;

    FogTexture.resize(FogTextureWidth * FogTextureHeight);
    std::fill(FogTexture.begin(), FogTexture.end(), 0xFF);

    Blurer.Setup(FogTextureWidth, FogTextureHeight);

    RenderWidth  = viewport.BottomRightPos.x - viewport.TopLeftPos.x + 1;
    RenderHeight = viewport.BottomRightPos.y - viewport.TopLeftPos.y + 1;
    
    WorkSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, FogTextureWidth, FogTextureHeight, 
                                                      32, RMASK, GMASK, BMASK, AMASK);
    SDL_SetSurfaceBlendMode(WorkSurface, SDL_BLENDMODE_NONE);

    FogSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, mapAreaWidth * PixelTileSize.x, 
                                                     mapAreaHeight * PixelTileSize.y, 
                                                     32, RMASK, GMASK, BMASK, AMASK);
    SDL_SetSurfaceBlendMode(FogSurface, SDL_BLENDMODE_BLEND);
    SDL_FillRect(FogSurface, NULL, SDL_MapRGBA(FogSurface->format, 0, 0, 0, 0));
}

void CFogOfWar::Clean()
{
    FogTexture.clear();
    FogTextureWidth = 0;
    FogTextureHeight = 0;
   
    SDL_FreeSurface(WorkSurface); /// It is safe to pass NULL to this function.
    WorkSurface = nullptr;
    SDL_FreeSurface(FogSurface); /// It is safe to pass NULL to this function.
    FogSurface = nullptr;
   
    Blurer.Clean();
}

/**
**  Generate and render fog of war for certain viewport.
**
**  @param viewport     viewport to refresh fog of war for
**  @param thisPlayer   player to refresh fog of war for
*/
void CFogOfWar::Refresh(const CViewport &viewport, const CPlayer &thisPlayer)
{
    // flags must redraw or not
	if (ReplayRevealMap) {
		return;
	}
    if ((viewport.BottomRightPos.x - viewport.TopLeftPos.x + 1) != RenderWidth
        || (viewport.BottomRightPos.y - viewport.TopLeftPos.y + 1) != RenderHeight ) {
  
        AdjustToViewport(viewport);
    }

    GenerateFog(viewport, thisPlayer);
    Render(FogTexture.data(), viewport);
}

/**
**  Render fog of war texture in certain viewport.
**
**  @param alphaTexture texture with alpha mask to render 
**  @param viewport     viewport to render fog of war texture in
*/
 void CFogOfWar::Render(const uint8_t *alphaTexture, const CViewport &viewport)
{
    
    /// Clear fog surface
    SDL_FillRect(FogSurface, NULL, SDL_MapRGBA(FogSurface->format, 0, 0, 0, 0));
    
    /// convert fog texture into surface to be able to scale it with SDL_BlitScaled
    intptr_t renderIndex = 0;
    for (uint16_t y = 0; y < WorkSurface->h; y++) {
        for (uint16_t x = 0; x < WorkSurface->w; x++) {
            reinterpret_cast<uint32_t*>(WorkSurface->pixels)[renderIndex] = SDL_MapRGBA(WorkSurface->format, 0, 0, 0, (alphaTexture[renderIndex])); 
            renderIndex++;
        }
    }
    SDL_BlitScaled(WorkSurface, NULL, FogSurface, NULL);

    SDL_Rect screenRect;
    screenRect.x = viewport.TopLeftPos.x;
    screenRect.y = viewport.TopLeftPos.y;
    screenRect.w = RenderWidth;
    screenRect.h = RenderHeight;
    
    SDL_Rect fogRect;
    fogRect.x = viewport.Offset.x + PixelTileSize.x / 2;
    fogRect.y = viewport.Offset.y + PixelTileSize.y / 2;
    fogRect.w = screenRect.w;
    fogRect.h = screenRect.h;
	SDL_BlitSurface(FogSurface, &fogRect, TheScreen, &screenRect);
}

/**
**  Generate fog of war texture for certain viewport.
**
**  @param viewport     viewport to generate fog of war for
**  @param thisPlayer   player to generate fog of war for
*/
void CFogOfWar::GenerateFog(const CViewport &viewport, const CPlayer &thisPlayer)
{
    // Update for visibility all tile in viewport 
    /// +1 to the top & left and +1 to the bottom & right for 4x scale algorithm purposes, 
    /// +1 to the bottom & right because of UI.MapArea.ScrollPadding
    int beginCol    = std::max(viewport.MapPos.x - 1, 0);
    int beginRow    = std::max(viewport.MapPos.y - 1, 0);
    int rightEdge   = std::min<int>(viewport.MapPos.x + viewport.MapWidth + 2, Map.Info.MapWidth);
    int bottomEdge  = std::min<int>(viewport.MapPos.y + viewport.MapHeight + 2, Map.Info.MapHeight);

    intptr_t cacheIndex = CFogOfWar::VisCache_Index0 + beginRow * CFogOfWar::VisCacheWidth;
    intptr_t mapIndex = beginRow * Map.Info.MapWidth;
    for (int row = beginRow ; row < bottomEdge; row++) {
		for (int col = beginCol; col < rightEdge; col++) {
            /// FIXME: to speedup this part, maybe we have to use Map.Field(mapIndex + col)->playerInfo.Visible[thisPlayer.index] instead
            /// this must be much faster
            if (!(VisTableCache[cacheIndex + col] & VisionType::cCached)) {
                VisTableCache[cacheIndex + col] = Map.Field(mapIndex + col)->playerInfo.TeamVisibilityState(thisPlayer);
                VisTableCache[cacheIndex + col] |= VisionType::cCached;
            }
		}
		cacheIndex += CFogOfWar::VisCacheWidth;
        mapIndex += Map.Info.MapWidth;
	}

    /// Set the fog texture fully opaque
    std::fill(FogTexture.begin(), FogTexture.end(), 0xFF);

    UpscaleFog(FogTexture.data(), viewport);
    /// TODO: Blur fog texture
    Blurer.Blur(FogTexture.data());
}


/**
**  4x4 upscale for generated fog of war texture
**
**  @param 
**  @param 
*/
void CFogOfWar::UpscaleFog(uint8_t *alphaTexture, const CViewport &viewport)
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
    uint32_t    *fogTexture    = reinterpret_cast<uint32_t*>(alphaTexture);
    const size_t textureHeight = FogTextureHeight / 4;
    const size_t textureWidth  = FogTextureWidth / 4;
    intptr_t     textureIndex  = 0;
  
    /// in fact it's viewport.MapPos.y -1 & viewport.MapPos.x -1 because of VisTableCache starts from [-1:-1]
    intptr_t  visIndex = viewport.MapPos.y * VisCacheWidth + viewport.MapPos.x;
        
    for (int row = 0; row < textureHeight; row++) {
        for (int col = 0; col < textureWidth; col++) {
            /// Fill the 4x4 scaled tile
            FillUpscaledRec(fogTexture, textureWidth, textureIndex + col, 
                            DeterminePattern(visIndex + col, VisionType::cVisible), 
                            DeterminePattern(visIndex + col, VisionType::cVisible | VisionType::cExplored));
        }
        visIndex     += CFogOfWar::VisCacheWidth;
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
uint8_t CFogOfWar::DeterminePattern(intptr_t index, uint8_t visFlag)
{
    Assert(visFlag == VisionType::cVisible || visFlag == (VisionType::cExplored | VisionType::cVisible));

    uint8_t n1, n2, n3, n4;
    n1 = (visFlag & VisTableCache[index]);
    n2 = (visFlag & VisTableCache[index + 1]);
    index += CFogOfWar::VisCacheWidth;
    n3 = (visFlag & VisTableCache[index]);
    n4 = (visFlag & VisTableCache[index + 1]);
    
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
**  Determine radiuses (box sizes) for box blur iterations
**
**  @param radius Radius or standard deviation
**  @param numOfIterations Number of boxes
*/
void CBlurer::Init()
{
    CBlurer::Init(CBlurer::Radius, CBlurer::NumOfIterations);
}

void CBlurer::Init(const float radius, const uint8_t numOfIterations)
{
  
    float D = sqrt((12.0 * radius * radius  / numOfIterations) + 1);
    int dL = floor(D);  
    if (dL % 2 == 0) dL--;
    int dU = dL + 2;
				
    float M = (12 * radius * radius - numOfIterations * dL * dL - 4 * numOfIterations * dL - 3 * numOfIterations) / (-4 * dL - 4);
    int m = round(M);

    CBlurer::BoxRadius.clear();
    CBlurer::BoxRadius.resize(numOfIterations);
    for(uint8_t i = 0; i < numOfIterations; i++) {
        float val = ((float)(i < m ? dL : dU) - 1) / 2;
        CBlurer::BoxRadius[i] = val;
    }

    CBlurer::Radius         = radius;
    CBlurer::NumOfIterations = numOfIterations;
}

/**
**  Setup blurer
**
**  @param textureWidth width of the working texture (input/output texture also)
**  @param textureHeight height of the working texture (input/output texture also)
**  @param radius Radius or standard deviation
**  @param numOfIterations Number of boxes
*/
void CBlurer::Setup(const uint16_t textureWidth, const uint16_t textureHeight)
{
    TextureWidth = textureWidth;
    TextureHeight = textureHeight;
    WorkingTexture.clear();
    WorkingTexture.resize(TextureWidth * TextureHeight);
}

/**
**  Clean blurer
**
*/
void CBlurer::Clean()
{
    WorkingTexture.clear();
    TextureWidth = 0;
    TextureHeight = 0;
}


void CBlurer::Blur(uint8_t *texture)
{
    uint8_t *source = texture;
    uint8_t *target = WorkingTexture.data();

    for (int i = 0; i < BoxRadius.size(); i++){
        if (i > 0) {
            uint8_t *swap = source;
            source = target;
            target = swap;
        }
        ProceedIteration(source, target, BoxRadius[i]); 
    }
    if (target != texture) {
        memcpy(texture, WorkingTexture.data(), WorkingTexture.size() * sizeof(uint8_t));    
    }
 }

void CBlurer::ProceedIteration(uint8_t *source, uint8_t *target, const uint8_t radius)
{
    memcpy(target, source, WorkingTexture.size() * sizeof(uint8_t));
    
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
        for (size_t j = radius + 1; j < TextureWidth - radius; j++) 
        {
            sum += source[ri++] - source[li++];
            target[ti++] = round(sum * iarr);
        }
        for (size_t j = TextureWidth - radius; j < TextureWidth; j++)
        {
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
        for (size_t j = 0; j <= radius ; j++)
        { 
            sum += source[ri] - leftBorder;
            target[ti] = round(iarr * sum);
            ri += TextureWidth;
            ti += TextureWidth;
        }
        for (size_t j = radius + 1; j < TextureHeight - radius; j++)
        { 
            sum += source[ri] - source[li];
            target[ti] = round(iarr * sum);
            li += TextureWidth;
            ri += TextureWidth;
            ti += TextureWidth;
        }
        for (size_t j = TextureHeight - radius; j < TextureHeight; j++)
        { 
            sum += rightBorder - source[li];
            target[ti] = round(iarr * sum);
            li += TextureWidth;
            ti += TextureWidth;
        }
    }

}

//@}
