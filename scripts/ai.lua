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

-- This call examines the buttons that were defined with DefineButton
-- and figures out which types of units can build/train/repair others.
-- We don't currently have any "unit-equiv" information to provide here.
DefineAiHelper()

-- Execute all AI init scripts
-- FIXME: needs to be done per player; probably move to C++
function InitAiScripts()
  for ident,ai_type in next,AiTypes do
    -- check if this AI actually has an init script
    if (ai_type.Init ~= nil) then
      ai_type.Init()
    end
  end
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
