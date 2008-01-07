--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	maps.lua - Define map helper functions.
--
--	(c) Copyright 2005-2007 by Francois Beerten
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
--      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
--
--	$Id$
--

function DefineImageTilemodels(terrain, imgx, imgy, fullTerrainImage)
   local til = {}

   fullTerrainImage = fullTerrainImage or false

   for i = 0, imgx * imgy - 1 do
      til[i + 1] = i
   end

   DefineTileset(
     "name",  "Image terrain",
     "image", terrain,
     "imagemap", fullTerrainImage,
     -- Slots descriptions
     "slots", {
       "solid", { "light-grass", "land",
      til}
     }
   )
end

function RepeatMap(mapx, mapy, imgx, imgy)
  -- Tile map
  for y = 0, mapy - 1 do
    for x = 0, mapx - 1 do
      SetTile(math.mod(x, imgx) + math.mod(y, imgy) * imgx, x, y)
    end
  end

  -- The terrain of image based maps shouldnt be editable by the stratagus 
  -- builtin editor and the editor shouldn't try to write the tiles map
  Editor.TerrainEditable = false
end



function DefineImageTerrain(terrain, mapx, mapy, imgx, imgy)
   fullTerrainImage = imgx == mapx and imgy == mapy
   DefineImageTilemodels(terrain, imgx, imgy, fullTerrainImage)
   RepeatMap(mapx, mapy, imgx, imgy)
end

