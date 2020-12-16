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
//      (c) Copyright 2020 Alyokhin
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
#include "player.h"
#include "video.h"

//@{




class CViewport;

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CFogOfWar
{
public:
    enum VisionType { cUnseen  = 0, cExplored = 0b001, cVisible = 0b010, cCached      = 0b100};

    ~CFogOfWar()
    {
        Clean();
    }

    void Refresh(const CViewport &viewport, const CPlayer &thisPlayer);
    
    static void InitCache();
    static void CleanCache();
    static void ResetCache();

private:
    void Clean();
    void AdjustToViewport(const CViewport &viewport);
    void Render(const uint8_t *alphaTexture, const CViewport &viewport);
    void GenerateFog(const CViewport &viewport, const CPlayer &thisPlayer);
    void UpscaleFog(uint8_t *alphaTexture, const CViewport &viewport);
    uint8_t DeterminePattern(intptr_t index, const uint8_t visFlag);
    void FillUpscaledRec(uint32_t *texture, const int textureWidth, intptr_t index, const uint8_t patternVisible, 
                                                                                    const uint8_t patternExplored);
    
public:

private:
    /// cached vision table. Tiles filled only once even if it present in the several viewports
    static std::vector<uint8_t> VisTableCache; 
    static intptr_t VisCache_Index0; /// index in the cached vision table for [0:0] tile
    static size_t   VisCacheWidth;   /// width of the cached vision table
    
    std::vector<uint8_t> FogTexture; // Upscaled fog texture
    SDL_Surface         *WorkSurface {nullptr};
    SDL_Surface         *FogSurface {nullptr};

    uint16_t RenderWidth      {0}; // In pixels
    uint16_t RenderHeight     {0}; // In pixels
    uint16_t FogTextureWidth  {0};
    uint16_t FogTextureHeight {0};


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

#endif // !__FOW_H__
