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
--      (c) Copyright 2005-2006 by Francois Beerten.
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


DefineImageTilemodels(GetCurrentLuaPath().."/terrain.png", 16, 16*7)

Load("maps/tutorial/access-unpassable.lua")

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

function DefinePatchlet(r, w, h)
  local f = function(x, y)
      local i
      local j
      for i = 0, h-1 do
        for j = 0, w-1 do
            SetTile(r+i*16+j, x+j, y+i)
        end
      end
  end
  return f
end


greenMountain = DefinePatch(0, 16, 16)
rockyMountainL = DefinePatch(16, 16, 16)
rockyMountainR = DefinePatch(32, 16, 16)
pikeMountain = DefinePatch(48, 16, 16)
lake = DefinePatch(64, 16, 16)
plain16 = DefinePatch(80, 16, 16)
dirtyPlain = DefinePatch(96, 8, 8)
dirtyPlain2 = DefinePatchlet(96*16+8, 8, 8)
smallWetPlain = DefinePatch(104, 8, 8)
dirtyPlain3 = DefinePatchlet(104*16+8, 8, 8)

plain16(0,0)
smallWetPlain(0, 0)
smallWetPlain(8, 1)
smallWetPlain(5, 9)

plain16(16,0)
plain16(32,0)
plain16(48, 0)

plain16(0,16)
lake(16, 16)
greenMountain(32,16)
plain16(48, 16)

plain16(0,32)
plain16(16, 32)
smallWetPlain(24, 32)
dirtyPlain3(16, 40)
lake(32,32)
plain16(48, 32)

plain16(0, 48)
dirtyPlain3(0, 56)
dirtyPlain2(8, 56)
dirtyPlain(0, 48)
plain16(16, 48)

plain16(32,48)
plain16(48, 48)



Editor.TerrainEditable = false
