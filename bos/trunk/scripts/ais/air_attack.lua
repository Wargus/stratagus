--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      air_attack.lua	- Define a AI that needs many resources
--      and attacks through the air.
--
--	(c) Copyright 2010 by Michiel van der Wulp
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
--      $Id: air_attack.lua  $
--

local player
local ai_pos

local function AiLoop(loop_funcs, loop_pos)
  local ret

  player = AiPlayer() + 1
  while (true) do
    ret = loop_funcs[loop_pos[player]]()
    if (ret) then
        break
    end
    loop_pos[player] = loop_pos[player] + 1
  end
  return true
end

function InitAiScripts_air_attack()
  ai_pos      = {1, 1, 1, 1, 1, 1, 1, 1}
end

local ai_funcs = {
  function() AiDebug(false) return false end,

  function() return AiNeed("unit-nukepowerplant") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiWait("unit-nukepowerplant") end,
  function() return AiWait("unit-magmapump") end,

  function() return AiSet("unit-engineer", 5) end,

  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-magmapump") end,
  -- but we do not wait for these ...

  function() return AiForce(0, {"unit-bazoo", 4}) end,
  function() return AiWaitForce(0) end,
  function() return AiAttackWithForce(0) end,

  function() return AiNeed("unit-cannon") end,
  function() return AiSleep((SyncRand(600)+400) * GameSettings.Difficulty) end,
  
  function() return AiNeed("unit-radar") end,
  function() return AiWait("unit-radar") end,
  function() return AiSleep((SyncRand(100)+100) * GameSettings.Difficulty) end,
  function() return AiWait("unit-cannon") end,

  -- Defense
  function() return AiNeed("unit-aircraftfactory") end,
  function() return AiWait("unit-jet") end,
  function() return AiSleep((SyncRand(100)+100)*GameSettings.Difficulty) end,

  -- Attack wave
  function() return AiForce(2, {"unit-jet", 2}) end,
  function() return AiWaitForce(2) end,
  function() return AiSleep((SyncRand(70)+50)*GameSettings.Difficulty) end, 
  function() return AiAttackWithForce(2) end,

  function() return AiNeed("unit-powerplant") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiSleep((SyncRand(60)+40)*GameSettings.Difficulty) end, 
}

function AiAttackDefense()
    return AiLoop(ai_funcs, ai_pos)
end

RegisterAi("air_attack", "air_attack", AiAttackDefense, 
           InitAiScripts_air_attack)

