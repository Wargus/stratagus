--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	ai.lua		-	Define the AI.
--
--	(c) Copyright 2000-2009 by Lutz Sammer, Frank Loeffler
--      (c) Copyright 2011 by Michiel van der Wulp
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
--

-- Table of registered AI types in BOS.  See HTML documentation for details.
AiTypes = {}

-- AI state of each player.  See HTML documentation for details.
-- Initialize to nil so we'll notice if ClearAiState is not called
-- when it should.
AiState = nil

-- This call examines the buttons that were defined with DefineButton
-- and figures out which types of units can build/train/repair others.
-- We don't currently have any "unit-equiv" information to provide here.
DefineAiHelper()

-- Clear the AI state of each player.  This is called before starting
-- a new game, and before loading a saved game.  This does not call AI
-- initialization functions; the engine will do that after it has
-- selected the AI type for each player.
function ClearAiState()
  AiState = {cache = {}}
end

-- Check if there is a hot spot on the map.
--
-- Returns:
--   True if there is at least one hot spot (weak or not) on the map,
--   False if there are no hot spots.
--
-- AiHotSpotExists detects even hot spots that are not visible to the
-- current AI player.  This is not really a cheat because, if the AI
-- could not access this information directly, then maps without hot
-- spots would presumably select AI types that don't try to build
-- magma pumps.
--
-- AiHotSpotExists does not care whether the hot spots already have
-- magma pumps on them.
function AiHotSpotExists()
  local cache = AiState.cache
  if (cache.hotspotexists == nil) then
    local hotspot = UnitTypeByIdent("unit-hotspot")
    local whotspot = UnitTypeByIdent("unit-weakhotspot")
    local count = Players[PlayerNumNeutral].UnitTypesCount[hotspot.Slot]
    local wcount = Players[PlayerNumNeutral].UnitTypesCount[whotspot.Slot]
    -- DebugPrint("This map has " .. count .. " hotspots and " .. wcount .. " weak hotspots.")
    cache.hotspotexists = ((count + wcount) ~= 0)
  end
  return cache.hotspotexists
end

-- Find and load all Ais
local list
local i
local f
list = ListFilesInDirectory("scripts/ais/")

for i,f in ipairs(list) do
  if(string.find(f, "^.*%.lua$")) then
    DebugPrint("Loading AI: " .. f)
    Load("scripts/ais/" .. f)
  end
end
