--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      broke.lua
--      Define the AI that has the following philosophy: 
--      Start broke, i.e. this AI presumes you start with
--      no energy, no magma, and a crew of only a few engineers.
--
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
--      $Id:  $
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

local function InitAiScripts_broke()
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
  -- Build magma pump or power plant first depending on resources
  function()
    state.build_order = GetBuildOrder()
    return AiNeed(state.build_order[1])
  end,
  function()
    LocalDebugPrint("waits for " .. state.build_order[1]);
    return false
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
      LocalDebugPrint("waits for " .. state.build_order[2]);
    end
    return false
  end,
  function()
    if (state.build_order[2] ~= nil) then
      return AiWait(state.build_order[2])
    else
      return false
    end
  end,

  function() return AiNeed("unit-nukepowerplant") end,
  function()
    LocalDebugPrint("waits for nuke powerplant.");
    return false
  end,
  function() return AiWait("unit-nukepowerplant") end,

  function() return AiSet("unit-vault", 1) end,
  function() return AiSet("unit-engineer", 5) end,

  function() 
    if (AiHotSpotExists()) then
      return AiSet("unit-magmapump", 4)
    else
      return false -- do nothing
    end 
  end,
  function() return AiSleep((SyncRand(500)+500)*GameSettings.Difficulty) end, 
  function() return AiSet("unit-nukepowerplant", 5) end,
  function() return AiSleep((SyncRand(500)+500)*GameSettings.Difficulty) end, 

  function() return AiSet("unit-camp", 2) end,

  -- Defense
  function() return AiForce(0, {"unit-assault", 4}) end,
  function() return AiWaitForce(0) end,
  function()
    LocalDebugPrint("force 0 finished.");
    return false
  end,
  function() return AiSet("unit-camp", 2) end,

  -- Attack wave
  function() return AiForce(1, {"unit-grenadier", 6}) end,
  function() return AiWaitForce(1) end,
  function()
    LocalDebugPrint("force 1 (6 grenadiers) finished.");
    return false
  end,
  function() return AiSleep((SyncRand(500)+500)*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(1) end,
  function()
    LocalDebugPrint("force 1 (6 grenadiers) attacking.");
    return false;
  end,

  -- Bigger attack wave
  function() return AiForce(1, {"unit-assault", 8, "unit-grenadier", 2}) end,
  function() return AiWaitForce(1) end,

  function()
    LocalDebugPrint("force 1 (8 assaults, 2 grenadiers) finished.");
    return false;
  end,
  function() return AiAttackWithForce(1) end,

  function() return AiSet("unit-engineer", 8) end,
  function() return AiSet("unit-biggunturret", 1) end,
  function() return AiSet("unit-vfac", 1) end,
  function() return AiSet("unit-gturret", 2) end,
  
  function()
    LocalDebugPrint("waiting for vfac.");
    return false
  end,
  function() return AiWait("unit-vfac") end,

  -- Attack wave
  function()
    LocalDebugPrint("vfac finished.");
    return false
  end,
  function() return AiForce(1, {"unit-assault", SyncRand(5)+1, 
                                "unit-buggy", SyncRand(2)+1}) end,
  
  function()
    LocalDebugPrint("waiting for force 1 (assaults + buggies).");
    return false
  end,
  function() return AiWaitForce(1) end,
  function() return AiSleep((SyncRand(500)+500)*GameSettings.Difficulty) end, 
  function() return AiAttackWithForce(1) end,

  function()
    LocalDebugPrint("attacking with force 1.");
    return false;
  end,
  function() return AiSet("unit-biggunturret", 6) end,
  function() return AiSet("unit-vfac", 2) end,
  function() return AiSet("unit-gturret", 10) end,

  -- Start flying:
  function()
    LocalDebugPrint("more biggunturret, vfac, gturret.");
    return false
  end,
  function() return AiSet("unit-aircraftfactory", 2) end,

  -- ============================================================

  function() 
    LocalDebugPrint("is starting loop.");
    state.loop_start = state.loop_pos;
    return false
  end,

  -- Attack wave (land)
  function() return AiForce(1, {"unit-assault", 4}) end,

    -- Attack wave (air)
  function() return AiForce(2, {"unit-jet", 2}) end,

  -- Attack wave (vehicles)
  function() return AiForce(3, {"unit-rtank", 1,
                                "unit-tank", 3}) end,

  -- Attack wave (mixed land)
  function() return AiForce(4, {"unit-buggy", 2,
                                "unit-artil", 1}) end,

  -- Attack wave (air)
  function() return AiForce(5, {"unit-chopper", 1}) end,

  -- Attack wave (land, slow)
  function() return AiForce(6, {"unit-bazoo", 6}) end,

  -- Defense force 0 is sent to a building under attack
  function() return AiForce(0, {"unit-assault", 4,
                                "unit-bazoo", 3,
                                "unit-grenadier", 3}) end,

  -- wait 10 to 40 s:
  function() return AiSleep((SyncRand(900)+300) * GameSettings.Difficulty) end,

  function()
    local f
    for f=1,6 do
      if (AiCheckForce(f)) then
        LocalDebugPrint("is attacking with force " .. f .. ".");
        -- AiWaitForce(f);
        return AiAttackWithForce(f)
      end
    end
    return AiSleep((SyncRand(100)+40) * GameSettings.Difficulty)
  end,

  function()
    LocalDebugPrint("Reached the end of AI script and will loop");
    state.loop_pos = state.loop_start - 1; -- AiLoop will immediately increment it.
    return false
  end,
}

local function AiBroke()
  LocalDebugPrint("Script position " .. AiState[AiPlayer()].loop_pos);
  return AiLoop(ai_funcs)
end

this_ai_type = {
  Ident = "ai-broke",
  Name = _("Broke"),
  Init = InitAiScripts_broke,
  EachSecond = AiBroke,
}
DefineAiType(this_ai_type)
