--       _________ __                 __                               
--      /   _____//  |_____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--  T H E   W A R   B E G I N S
--   Stratagus - A free fantasy real time strategy game engine
--
--	terrain.lua - Define image based terrain
--
--      (c) Copyright 2005 by François Beerten.
--
--      This program is free software; you can redistribute it and/or modify
--      it under the terms of the GNU General Public License as published by
--      the Free Software Foundation; either version 2 of the License, or
--      (at your option) any later version.
--  
--      This program is distributed in the hope that it will be useful,
--      but WITHOUT ANY WARRANTY; without even the implied warranty of
--      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--      GNU General Public License for more details.
--  
--      You should have received a copy of the GNU General Public License
--      along with this program; if not, write to the Free Software
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
-- $Id: terrain.lua 191 2005-07-31 16:20:38Z feb $


DefineImageTilemodels(GetCurrentLuaPath().."/terrain.png", 16, 111)

Load("maps/patches/access-unpassable.lua")

function DefinePatch(r, w, h)
  local f = function(x, y)
      local i
      local j
      for i = 0, h-1 do
        for j = 0, w-1 do
            SetTile((r+i)*16+j, x+j, y+i)
        end
      end
  end
  return f
end

plain4 = DefinePatch(0, 4, 4)
greenMountain = DefinePatch(13, 16, 16)
rockyMountainL = DefinePatch(13+16, 16, 16)
rockyMountainR = DefinePatch(13+32, 16, 16)
pikeMountain = DefinePatch(13+48, 16, 16)
lake = DefinePatch(13+64, 16, 16)
plain16 = DefinePatch(92, 16, 16)


rockyMountainL(0,0)
plain16(16,0)
rockyMountainR(32, 0)
plain16(48,0)
plain16(0,16)
lake(16, 16)
greenMountain(32,16)
plain16(48, 16)
plain16(0,32)
greenMountain(16, 32)
lake(32,32)
plain16(48, 32)
pikeMountain(0, 48)
plain16(16, 48)
plain16(32,48)
greenMountain(48, 48)
greenMountain(18, 48)


SetTerrainEditable(false)