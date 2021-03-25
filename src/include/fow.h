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
#include "player.h"
#include "video.h"

//@{


/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/
enum class FogOfWarTypes { cLegacy, cEnhanced, cNumOfTypes };  /// Types of the fog of war

class CFogOfWar
{
public:
    enum VisionType   { cUnseen  = 0, cExplored = 0b001, cVisible = 0b010 };
    enum States       { cFirstEntry = 0, cGenerateFog, cGenerateTexture, cBlurTexture, cReady };
    enum UpscaleTypes { cSimple = 0, cBilinear };
    
    void Init();
    void Clean();
    bool SetType(const FogOfWarTypes fowType);

    FogOfWarTypes GetType()   const { return Settings.FOW_Type; }
    CColor   GetFogColor()    const { return Settings.FogColor; }
    uint32_t GetFogColorSDL() const { return Settings.FogColorSDL; }

    void ShowVisionFor(const CPlayer &player) { VisionFor.insert(player.Index); }
    void HideVisionFor(const CPlayer &player) { VisionFor.erase(player.Index); }

    void SetFogColor(uint8_t r, uint8_t g, uint8_t b);
    void SetFogColor(CColor color);

    void EnableBilinearUpscale(const bool enable);
    void InitBlurer(const float radius1, const float radius2, const uint16_t numOfIterations);

    void Update(bool doAtOnce = false);
    void GetFogForViewport(const CViewport &viewport, SDL_Surface *const vpSurface);

private:
    void GenerateFog();
    void FogUpscale4x4();

    uint8_t DeterminePattern(const size_t index, const uint8_t visFlag) const;
    void FillUpscaledRec(uint32_t *texture, const uint16_t textureWidth, size_t index, 
                         const uint8_t patternVisible, const uint8_t patternExplored) const;

    void UpscaleBilinear(const uint8_t *const src, const SDL_Rect &srcRect, const int16_t srcWidth,
                         SDL_Surface *const trgSurface, const SDL_Rect &trgRect) const;

    void UpscaleSimple(const uint8_t *src, const SDL_Rect &srcRect, const int16_t srcWidth,
                       SDL_Surface *const trgSurface, const SDL_Rect &trgRect) const;
    
public:

private:
    struct FogOfWarSettings 
	{
		FogOfWarTypes FOW_Type         {FogOfWarTypes::cEnhanced}; /// Type of fog of war - legacy or enhanced(smooth)
        uint8_t       NumOfEasingSteps {8};                        /// Number of the texture easing steps
        float         BlurRadius[2]    {2.0, 1.5};                 /// Radiuses or standard deviation
        uint8_t       BlurIterations   {3};                        /// Number of blur iterations
        uint8_t       UpscaleType      {UpscaleTypes::cSimple};    /// Rendering zoom type
        CColor        FogColor         {0, 0, 0, 0};               /// Fog of war color
        uint32_t      FogColorSDL      {0};                        /// Fog of war color in the SDL format
	} Settings;  /// Fog of war settings

    uint8_t State { States::cFirstEntry };    /// State of the fog of war calculation process
    
    std::set<uint8_t>    VisionFor;           /// Visibilty through the fog is generated for this players
                                              /// ThisPlayer and his allies in normal games
                                              /// Any set of players for observers and in the replays
    std::vector<uint8_t> VisTable;            /// vision table for whole map + 1 tile around (for simplification of upscale algorithm purposes)
    intptr_t             VisTable_Index0 {0}; /// index in the vision table for [0:0] tile
    size_t               VisTableWidth   {0}; /// width of the vision table
    CEasedTexture        FogTexture;          /// Upscaled fog texture (alpha-channel values only) for whole map 
                                              /// + 1 tile around (for simplification of upscale algorithm purposes).
    std::vector<uint8_t> RenderedFog;         /// Back buffer for bilinear upscaling in to viewports
    CBlurer              Blurer;              /// Blurer for fog of war texture

    /// Table with patterns to generate fog of war texture from vision table
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    static constexpr uint32_t UpscaleTable[16][4] { {0x7F7F7F7F, 0x7F7F7F7F, 0x7F7F7F7F, 0x7F7F7F7F},   // 0 00:00
                                                    {0x7F7F7F7F, 0x7F7F7F7F, 0x3F7F7F7F, 0x003F7F7F},   // 1 00:01
                                                    {0x7F7F7F7F, 0x7F7F7F7F, 0x7F7F7F3F, 0x7F7F3F00},   // 2 00:10
                                                    {0x7F7F7F7F, 0x7F7F7F7F, 0x3F3F3F3F, 0x00000000},   // 3 00:11
                                                    {0x003F7F7F, 0x3F7F7F7F, 0x7F7F7F7F, 0x7F7F7F7F},   // 4 01:00
                                                    {0x003F7F7F, 0x003F7F7F, 0x003F7F7F, 0x003F7F7F},   // 5 01:01
                                                    {0x003F7F7F, 0x3F7F7F7F, 0x7F7F7F3F, 0x7F7F3F00},   // 6 01:10
                                                    {0x00003F7F, 0x0000003F, 0x00000000, 0x00000000},   // 7 01:11
                                                    {0x7F7F3F00, 0x7F7F7F3F, 0x7F7F7F7F, 0x7F7F7F7F},   // 8 10:00
                                                    {0x7F7F3F00, 0x7F7F7F3F, 0x3F7F7F7F, 0x003F7F7F},   // 9 10:01
                                                    {0x7F7F3F00, 0x7F7F3F00, 0x7F7F3F00, 0x7F7F3F00},   // A 10:10
                                                    {0x7F3F0000, 0x3F000000, 0x00000000, 0x00000000},   // B 10:11
                                                    {0x00000000, 0x3F3F3F3F, 0x7F7F7F7F, 0x7F7F7F7F},   // C 11:00
                                                    {0x00000000, 0x00000000, 0x0000003F, 0x00003F7F},   // D 11:01
                                                    {0x00000000, 0x00000000, 0x3F000000, 0x7F3F0000},   // E 11:10
                                                    {0x00000000, 0x00000000, 0x00000000, 0x00000000} }; // F 11:11

#else // big endian
    static constexpr uint32_t UpscaleTable[16][4] { {0x7F7F7F7F, 0x7F7F7F7F, 0x7F7F7F7F, 0x7F7F7F7F},   // 0 00:00
                                                    {0x7F7F7F7F, 0x7F7F7F7F, 0x7F7F7F3F, 0x7F7F3F00},   // 1 00:01
                                                    {0x7F7F7F7F, 0x7F7F7F7F, 0x3F7F7F7F, 0x003F7F7F},   // 2 00:10
                                                    {0x7F7F7F7F, 0x7F7F7F7F, 0x00000000, 0x00000000},   // 3 00:11
                                                    {0x7F7F3F00, 0x7F7F7F3F, 0x7F7F7F7F, 0x7F7F7F7F},   // 4 01:00
                                                    {0x7F7F0000, 0x7F7F0000, 0x7F7F0000, 0x7F7F0000},   // 5 01:01
                                                    {0x7F7F3F00, 0x7F7F7F3F, 0x3F7F7F7F, 0x003F7F7F},   // 6 01:10
                                                    {0x7F3F0000, 0x3F000000, 0x00000000, 0x00000000},   // 7 01:11
                                                    {0x003F7F7F, 0x3F7F7F7F, 0x7F7F7F7F, 0x7F7F7F7F},   // 8 10:00
                                                    {0x003F7F7F, 0x3F7F7F7F, 0x7F7F7F3F, 0x7F7F3F00},   // 9 10:01
                                                    {0x00007F7F, 0x00007F7F, 0x00007F7F, 0x00007F7F},   // A 10:10
                                                    {0x00003F7F, 0x0000003F, 0x00000000, 0x00000000},   // B 10:11
                                                    {0x00000000, 0x00000000, 0x7F7F7F7F, 0x7F7F7F7F},   // C 11:00
                                                    {0x00000000, 0x00000000, 0x3F000000, 0x7F3F0000},   // D 11:01
                                                    {0x00000000, 0x00000000, 0x0000003F, 0x00003F7F},   // E 11:10
                                                    {0x00000000, 0x00000000, 0x00000000, 0x00000000} }; // F 11:11
#endif
};

extern CFogOfWar FogOfWar;

#endif // !__FOW_H__
