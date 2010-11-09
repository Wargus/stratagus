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

-- The list of registered AIs in BOS
-- Every AI has an entry name: {internal_name, name, fun, initfun}
-- See at RegisterAi() for a description what these are
local AiList = {}

function GetAiList()
  return AiList
end

-- Function to register an AI to BOS
-- Parameters:
-- internal_name : Internal name of the Ai (without leading "ai-"
-- name          : Name of the AI the Player sees and which gets translated
-- fun           : main AI function
-- initfun       : initialization function, can be obmitted
function RegisterAi(internal_name, name, fun, initfun)
  DefineAi("ai-" .. internal_name, "ai-" .. internal_name, fun)
  AiList[name] = {internal_name, name, fun, initfun}
end

-- This call examines the buttons that were defined with DefineButton
-- and figures out which types of units can build/train/repair others.
-- We don't currently have any "unit-equiv" information to provide here.
DefineAiHelper()

-- Execute all AI init scripts
function InitAiScripts()
  for key,value in next,AiList do
    -- check if this AI actually has an init script
    if (value[4] ~= nil) then
      value[4]()
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

