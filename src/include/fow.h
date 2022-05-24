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
/**@name fow.h - The fog of war headerfile. */
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

#ifndef __FOW_H__
#define __FOW_H__

#include <cstdint>
#include <vector>
#include "fow_utils.h"
#include "map.h"
#include "player.h"
#include "settings.h"
#include "video.h"

//@{


/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/
enum class FogOfWarTypes { cTiled, cEnhanced, cTiledLegacy, cNumOfTypes };  /// Types of the fog of war
class CFogOfWar
{
public:
    CFogOfWar()
    {
        SetOpacityLevels(this->Settings.ExploredOpacity, this->Settings.RevealedOpacity, this->Settings.UnseenOpacity);
    }

    enum VisionType   { cUnseen  = 0, cExplored = 0b001, cVisible = 0b010 };
    enum States       { cFirstEntry = 0, cGenerateFog, cGenerateTexture, cBlurTexture, cReady };
    enum UpscaleTypes { cSimple = 0, cBilinear };

    static void SetTiledFogGraphic(const std::string &fogGraphicFile);

    void Init();
    void Clean(const bool isHardClean = false);
    bool SetType(const FogOfWarTypes fowType);
    void SetOpacityLevels(const uint8_t explored, const uint8_t revealed, const uint8_t unseen);

    FogOfWarTypes GetType()       const { return Settings.Type; }
    
    CColor   GetFogColor()        const { return Settings.FogColor; }
    uint32_t GetFogColorSDL()     const { return Settings.FogColorSDL; }
    uint8_t  GetExploredOpacity() const { return Settings.ExploredOpacity; }
    uint8_t  GetRevealedOpacity() const { return Settings.RevealedOpacity; }
    uint8_t  GetUnseenOpacity()   const { return Settings.UnseenOpacity; }

    void ShowVisionFor(const CPlayer &player) { VisionFor.insert(player.Index); }
    void HideVisionFor(const CPlayer &player) { VisionFor.erase(player.Index); }

    void SetFogColor(const uint8_t r, const uint8_t g, const uint8_t b);
    void SetFogColor(const CColor color);

    void EnableBilinearUpscale(const bool enable);
    bool IsBilinearUpscaleEnabled() const { return Settings.UpscaleType == UpscaleTypes::cBilinear; }
    void InitBlurer(const float radius1, const float radius2, const uint16_t numOfIterations);

    void Update(bool doAtOnce = false);
    void Draw(CViewport &viewport);
    
    uint8_t GetVisibilityForTile(const Vec2i tilePos) const;

private:
    void InitEnhanced();
    void DrawEnhanced(CViewport &viewport);

    void GenerateUpscaleTables(uint32_t (*table)[4], const uint8_t alphaFrom, const uint8_t alphaTo);

    void GenerateFog();
    void FogUpscale4x4();

    uint8_t DeterminePattern(const size_t index, const uint8_t visFlag) const;
    void FillUpscaledRec(uint32_t *texture, const uint16_t textureWidth, size_t index, 
                         const uint8_t patternVisible, const uint8_t patternExplored) const;

    void UpscaleBilinear(const uint8_t *const src, const SDL_Rect &srcRect, const int16_t srcWidth,
                         SDL_Surface *const trgSurface, const SDL_Rect &trgRect) const;

    void UpscaleSimple(const uint8_t *src, const SDL_Rect &srcRect, const int16_t srcWidth,
                       SDL_Surface *const trgSurface, const SDL_Rect &trgRect) const;

    void InitTiled();
    void CleanTiled(const bool isHardClean = false);

    void DrawFullShroudOfFog(const int16_t x, const int16_t y, const uint8_t alpha, 
                             SDL_Surface *const vpFogSurface);
    void GetFogTile(const size_t visIndex, const size_t mapIndex, const size_t mapIndexBase, 
                         int *fogTile, int *blackFogTile) const;
    bool IsMapFieldExplored(const size_t index) const { return (VisTable[index] != 0); }
    bool IsMapFieldVisible(const size_t index)  const { return (VisTable[index]  > 1); }
    void DrawFogTile(const size_t visIndex, const size_t mapIndex, const size_t mapIndexBase, 
                          const int16_t dx, const int16_t dy, SDL_Surface *const vpFogSurface);
    void DrawTiled(CViewport &viewport);
    void DrawTiledLegacy(CViewport &viewport);
    
public:

private:
    struct FogOfWarSettings 
	{
		FogOfWarTypes Type             {FogOfWarTypes::cEnhanced}; /// Type of fog of war - tiled or enhanced(smooth)
        uint8_t       NumOfEasingSteps {8};                        /// Number of the texture easing steps
        float         BlurRadius[2]    {2.0, 1.5};                 /// Radiuses or standard deviation
        uint8_t       BlurIterations   {3};                        /// Number of blur iterations
        uint8_t       UpscaleType      {UpscaleTypes::cSimple};    /// Rendering zoom type
        CColor        FogColor         {0, 0, 0, 0};               /// Fog of war color
        uint32_t      FogColorSDL      {0};                        /// Fog of war color in the SDL format
        uint8_t       ExploredOpacity  {0x7F};
        uint8_t       RevealedOpacity  {0xBE};
        uint8_t       UnseenOpacity    {0xFE};
	} Settings;  /// Fog of war settings

    uint8_t State { States::cFirstEntry };    /// State of the fog of war calculation process
    
    std::set<uint8_t>   VisionFor;  /// Visibilty through the fog is generated for this players
                                    /// ThisPlayer and his allies in normal games
                                    /// Any set of players for observers and in the replays

    static CGraphic *TiledFogSrc;           /// Graphic for tiled fog of war
    CGraphic *TiledAlphaFog {nullptr};      /// Working set of graphic for tiled fog of war with alpha channel
    SDL_Surface *TileOfFogOnly {nullptr};   /// Tile contains only fog. Used for legacy rendering of tiled fog
    
    /**
    **  Mapping for fog of war tiles.
    */
    const int TiledFogTable[16] = {0, 11, 10, 2,  13, 6, 14, 3,  12, 15, 4, 1,  8, 9, 7, 0};

    std::vector<uint8_t> VisTable;            /// vision table for whole map + 1 tile around (for simplification of upscale algorithm purposes)
    size_t               VisTable_Index0 {0}; /// index in the vision table for [0:0] map tile
    size_t               VisTableWidth   {0}; /// width of the vision table
    CEasedTexture        FogTexture;          /// Upscaled fog texture (alpha-channel values only) for whole map 
                                              /// + 1 tile to the left and up (for simplification of upscale algorithm purposes).
    std::vector<uint8_t> RenderedFog;         /// Back buffer for bilinear upscaling in to viewports
    CBlurer              Blurer;              /// Blurer for fog of war texture

    /// Tables with patterns to generate fog of war texture from vision table
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    const uint32_t UpscaleTable_4x4[16][4] { {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},   // 0 00:00
                                             {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x007FFFFF},   // 1 00:01
                                             {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFF7F, 0xFFFF7F00},   // 2 00:10
                                             {0xFFFFFFFF, 0xFFFFFFFF, 0x7F7F7F7F, 0x00000000},   // 3 00:11
                                             {0x007FFFFF, 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},   // 4 01:00
                                             {0x007FFFFF, 0x007FFFFF, 0x007FFFFF, 0x007FFFFF},   // 5 01:01
                                             {0x007FFFFF, 0x7FFFFFFF, 0xFFFFFF7F, 0xFFFF7F00},   // 6 01:10
                                             {0x00007FFF, 0x0000007F, 0x00000000, 0x00000000},   // 7 01:11
                                             {0xFFFF7F00, 0xFFFFFF7F, 0xFFFFFFFF, 0xFFFFFFFF},   // 8 10:00
                                             {0xFFFF7F00, 0xFFFFFF7F, 0x7FFFFFFF, 0x007FFFFF},   // 9 10:01
                                             {0xFFFF7F00, 0xFFFF7F00, 0xFFFF7F00, 0xFFFF7F00},   // A 10:10
                                             {0xFF7F0000, 0x7F000000, 0x00000000, 0x00000000},   // B 10:11
                                             {0x00000000, 0x7F7F7F7F, 0xFFFFFFFF, 0xFFFFFFFF},   // C 11:00
                                             {0x00000000, 0x00000000, 0x0000007F, 0x00007FFF},   // D 11:01
                                             {0x00000000, 0x00000000, 0x7F000000, 0xFF7F0000},   // E 11:10
                                             {0x00000000, 0x00000000, 0x00000000, 0x00000000} }; // F 11:11

#else // big endian
    const uint32_t UpscaleTable_4x4[16][4] { {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},   // 0 00:00
                                             {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFF7F, 0xFFFF7F00},   // 1 00:01
                                             {0xFFFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, 0x007FFFFF},   // 2 00:10
                                             {0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000},   // 3 00:11
                                             {0xFFFF7F00, 0xFFFFFF7F, 0xFFFFFFFF, 0xFFFFFFFF},   // 4 01:00
                                             {0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000},   // 5 01:01
                                             {0xFFFF7F00, 0xFFFFFF7F, 0x7FFFFFFF, 0x007FFFFF},   // 6 01:10
                                             {0xFF7F0000, 0x7F000000, 0x00000000, 0x00000000},   // 7 01:11
                                             {0x007FFFFF, 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},   // 8 10:00
                                             {0x007FFFFF, 0x7FFFFFFF, 0xFFFFFF7F, 0xFFFF7F00},   // 9 10:01
                                             {0x0000FFFF, 0x0000FFFF, 0x0000FFFF, 0x0000FFFF},   // A 10:10
                                             {0x00007FFF, 0x0000007F, 0x00000000, 0x00000000},   // B 10:11
                                             {0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF},   // C 11:00
                                             {0x00000000, 0x00000000, 0x7F000000, 0xFF7F0000},   // D 11:01
                                             {0x00000000, 0x00000000, 0x0000007F, 0x00007FFF},   // E 11:10
                                             {0x00000000, 0x00000000, 0x00000000, 0x00000000} }; // F 11:11
#endif

    uint32_t UpscaleTableVisible[16][4]  = {}; /// It will be generated from UpscaleTable_4x4
    uint32_t UpscaleTableExplored[16][4] = {}; /// It will be generated from UpscaleTable_4x4
    uint32_t UpscaleTableRevealed[16][4] = {}; /// It will be generated from UpscaleTable_4x4
    const uint32_t (*CurrUpscaleTableExplored)[4] = UpscaleTableVisible;
};

extern CFogOfWar *FogOfWar;


/**
**  Determine upscale patterns (index in the upscale table) for Visible and Explored layers
**
**  @param  index       tile in the vision table 
**  @param  visFlag     layer to determine pattern for
**
*/
inline uint8_t CFogOfWar::DeterminePattern(const size_t index, const uint8_t visFlag) const
{
    Assert(visFlag == VisionType::cVisible || visFlag == (VisionType::cExplored | VisionType::cVisible));

    uint8_t n1, n2, n3, n4;
    size_t offset = index;

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
inline void CFogOfWar::FillUpscaledRec(uint32_t *texture, const uint16_t textureWidth, size_t index, 
                                        const uint8_t patternVisible, const uint8_t patternExplored) const
{
    for (uint8_t scan_line = 0; scan_line < 4; scan_line++) {
        texture[index] = UpscaleTableVisible[patternVisible][scan_line] + CurrUpscaleTableExplored[patternExplored][scan_line];
        index += textureWidth;
    }
}

inline uint8_t CFogOfWar::GetVisibilityForTile(const Vec2i tilePos) const
{
    return VisTable[VisTable_Index0 + tilePos.x + VisTableWidth * tilePos.y];
}
#endif // !__FOW_H__
