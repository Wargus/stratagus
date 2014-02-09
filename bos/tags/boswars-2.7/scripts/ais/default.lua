--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	default.lua	- Define the AI.
--
--	(c) Copyright 2009 by Jimmy Salmon
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
--	$Id: rush.lua 626 2006-11-25 18:10:03Z feb $
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

local function InitAiScripts_default()
  AiState[AiPlayer()] = {
    loop_pos = 1,
    loop_start = nil,
    build_order = nil,
  }
end

local function CheckPower()
  local nuke = UnitTypeByIdent("unit-nukepowerplant")
  local count = Players[AiPlayer()].UnitTypesCount[nuke.Slot]
  if (count ~= 0) then
    return nil
  end
  return "unit-powerplant"
end

local function GetBuildOrder()
  local order = {}

  if (not AiHotSpotExists()) then
    order[1] = CheckPower()
    order[2] = nil
  elseif (Players[AiPlayer()].MagmaStored < 300) then
    order[1] = "unit-magmapump"
    order[2] = CheckPower()
  else
    order[1] = CheckPower()
    order[2] = "unit-magmapump"
  end

  return order
end

local ai_funcs = {
  function() AiDebug(false) return false end,

  -- Build magma pump or power plant first depending on resources
  function()
    state.build_order = GetBuildOrder()
    if (state.build_order[1] ~= nil) then
      return AiNeed(state.build_order[1])
    else
      return false
    end
  end,
  function()
    if (state.build_order[1] ~= nil) then
      return AiWait(state.build_order[1])
    else
      return false
    end
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

  function() return AiSet("unit-engineer", 2) end,
  function() return AiNeed("unit-vault") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiWait("unit-vault") end,

  function() return AiNeed("unit-camp") end,
  function() return AiWait("unit-camp") end,
  function() return AiSet("unit-engineer", 3) end,

  -- Defense
  function() return AiForce(0, {"unit-assault", 2}) end,
  function() return AiWaitForce(0) end, 

  function() return AiNeed("unit-powerplant") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-vfac") end,
  function() return AiNeed("unit-gturret") end,
  function() return AiSleep(150*GameSettings.Difficulty) end,

  -- Attack wave
  function() return AiForce(1, {"unit-assault", 5, "unit-rtank", 3}) end,
  function() return AiWaitForce(1) end,
  function() return AiSleep(50*GameSettings.Difficulty) end, 
  function() return AiAttackWithForce(1) end,

  function() return AiNeed("unit-powerplant") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-biggunturret") end,
  function() return AiSleep(50*GameSettings.Difficulty) end, 

  -- Bigger attack wave
  function() return AiNeed("unit-magmapump") end,
  function() return AiForce(1, {"unit-assault", 3,
                                "unit-grenadier", 3,
                                "unit-tank", 3,
                                "unit-rtank", 3}) end,
  function() return AiWaitForce(1) end, 
  function() return AiAttackWithForce(1) end,

  -- ============================================================

  function() 
    LocalDebugPrint("is starting loop.");
    state.loop_start = state.loop_pos;
    return false
  end,

  function() return AiForce(1, {"unit-bazoo", 10, 
                                "unit-grenadier", 10, 
                                "unit-tank", 4,
                                "unit-rtank", 4}) end,
  function() return AiWaitForce(1) end,
  function() return AiSleep(50*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(1) end,

  function()
    LocalDebugPrint("Reached the end of AI script and will loop");
    state.loop_pos = state.loop_start - 1; -- AiLoop will immediately increment it.
    return false
  end,
}

local function AiDefault()
  LocalDebugPrint("Script position " .. AiState[AiPlayer()].loop_pos);
  return AiLoop(ai_funcs)
end

this_ai_type = {
  Ident = "ai-default",
  Name = _("Default"),
  Init = InitAiScripts_default,
  EachSecond = AiDefault,
}
DefineAiType(this_ai_type)
