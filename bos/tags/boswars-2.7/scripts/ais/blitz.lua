--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	blitz.lua - Define the blitz AI.
--
--	(c) Copyright 2000-2007 by Lutz Sammer, Jimmy Salmon
--                  and Tejay Penfold
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
--	$Id: blitz.lua 626 2007-10-14 18:10:03Z timberdragon $
--

-- What we registered in AiTypes.
local this_ai_type

-- Same as AiState[AiPlayer()].  Valid only during AiLoop.
local state

local function AiLoop(funcs)
  state = AiState[AiPlayer()]
  while (true) do
    local ret = funcs[state.loop_pos]()
    if (ret) then
      break
    end
    state.loop_pos = state.loop_pos + 1
  end
  return true
end

local function LocalDebugPrint(text)
  -- DebugPrint(this_ai_type.Ident .. " player " .. AiPlayer() .. " " .. text)
end

local function InitAiScripts_blitz()
  AiState[AiPlayer()] = {
    loop_pos = 1,
    loop_start = nil,
    build_order = nil,
  }
end

local function GetBuildOrder()
  local order = {}

  if (not AiHotSpotExists()) then
    order[1] = "unit-powerplant"
    order[2] = nil
  elseif (Players[AiPlayer()].MagmaStored < 300) then
    order[1] = "unit-magmapump"
    order[2] = "unit-powerplant"
  else
    order[1] = "unit-powerplant"
    order[2] = "unit-magmapump"
  end

  return order
end

local ai_funcs = {
  function() AiDebug(false) return false end,

  -- Build magma pump or power plant first depending on resources
  function()
    state.build_order = GetBuildOrder()
    return AiNeed(state.build_order[1])
  end,
  function()
    return AiWait(state.build_order[1])
  end,
  function()
    if (state.build_order[2] ~= nil) then
      return AiNeed(state.build_order[2])
    else
      return false
    end
  end,
  function()
    if (state.build_order[2] ~= nil) then
      return AiWait(state.build_order[2])
    else
      return false
    end
  end,

  function() return AiSet("unit-engineer", 3) end,
  function() return AiNeed("unit-vault") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiWait("unit-vault") end,

  function() return AiNeed("unit-camp") end,
  function() return AiNeed("unit-nukepowerplant") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiNeed("unit-gturret") end,
  function() return AiNeed("unit-gturret") end,
  function() return AiNeed("unit-gturret") end,
  function() return AiNeed("unit-vfac") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-powerplant") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-powerplant") end,
  function() return AiNeed("unit-vault") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-powerplant") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiNeed("unit-gturret") end,
  function() return AiWait("unit-gturret") end,
  function() return AiSet("unit-engineer", 4) end,

  -- Defense
  function() return AiForce(0, {"unit-assault", 20}) end,
  function() return AiWaitForce(0) end, 

  function() return AiNeed("unit-powerplant") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-camp") end,
  function() return AiNeed("unit-gturret") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiNeed("unit-powerplant") end,
  function() return AiNeed("unit-gturret") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiNeed("unit-powerplant") end,
  function() return AiNeed("unit-vault") end,
  function() return AiNeed("unit-nukepowerplant") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiNeed("unit-gturret") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiNeed("unit-engineer") end,
  function() return AiSleep(150*GameSettings.Difficulty) end,

  -- Attack wave
  function() return AiForce(1, {"unit-assault", 5}) end,
  function() return AiWaitForce(1) end,
  function() return AiSleep(50*GameSettings.Difficulty) end, 
  function() return AiAttackWithForce(1) end,

  -- Bigger attack wave
  function() return AiNeed("unit-magmapump") end,
  function() return AiForce(1, {"unit-assault", 40, "unit-grenadier", 10, "unit-tank", 5}) end,
  function() return AiWaitForce(1) end, 
  function() return AiAttackWithForce(1) end,
  
    -- Bigger attack wave
  function() return AiNeed("unit-magmapump") end,
  function() return AiForce(2, {"unit-assault", 60, "unit-grenadier", 20, "unit-tank", 10}) end,
  function() return AiWaitForce(2) end, 
  function() return AiAttackWithForce(2) end,

  -- ============================================================

  function() 
    LocalDebugPrint("is starting loop.");
    state.loop_start = state.loop_pos;
    return false
  end,

  function() return AiForce(1, {"unit-assault", 20, 
                                "unit-grenadier", 8, 
                                "unit-bazoo", 8}) end,
  function() return AiWaitForce(1) end,  -- wait until attack party is completed
  function() return AiSleep(50*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(1) end,

  function()
    LocalDebugPrint("Reached the end of AI script and will loop");
    state.loop_pos = state.loop_start - 1; -- AiLoop will immediately increment it.
    return false
  end,
}

local function AiBlitz()
  LocalDebugPrint("Script position " .. AiState[AiPlayer()].loop_pos);
  return AiLoop(ai_funcs)
end

this_ai_type = {
  Ident = "ai-blitz",
  Name = _("Blitz"),
  Init = InitAiScripts_blitz,
  EachSecond = AiBlitz,
}
DefineAiType(this_ai_type)
