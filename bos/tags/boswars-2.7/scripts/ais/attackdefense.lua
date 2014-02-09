--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      attackdefense.lua	- Define a hard to beat AI 
--      that needs many resources and enough space,
--      takes care of a good defense,
--      and attacks with everything available.
--
--      (c) Copyright 2009-2010 by Michiel van der Wulp
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
--      $Id: attackdefense.lua  $
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

local function InitAiScripts_attackdefense()
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
  -- force 0 is sent to a building under attack
  function() return AiForce(0, {"unit-assault", 2}) end,
  function() return AiWaitForce(0) end,

  function() return AiNeed("unit-powerplant") end,
  function() return AiNeed("unit-magmapump") end,

  -- First contact - Small Attack wave
  function() return AiForce(1, {"unit-assault", 4}) end,
  function() return AiWaitForce(1) end,
  function() return AiNeed("unit-magmapump") end,
  -- Since this 1st force is very small, it comes very soon.
  -- Hence, we wait a bit longer to make it easier for beginners
  -- And we make it random, so that when there are multiple opponents,
  -- they do not all come at the same time
  function() return AiSleep((SyncRand(1000)+400) * GameSettings.Difficulty) end,
  function() return AiAttackWithForce(1) end,

  -- Defense
  function() return AiNeed("unit-vfac") end,
  function() return AiNeed("unit-gturret") end,
  function() return AiSleep((SyncRand(200)+100)*GameSettings.Difficulty) end,

  -- Attack wave
  function() return AiForce(2, {"unit-assault", 5, "unit-rtank", 3}) end,
  function() return AiWaitForce(2) end,
  function() return AiSleep((SyncRand(120)+50)*GameSettings.Difficulty) end, 
  function() return AiAttackWithForce(2) end,

  function() return AiNeed("unit-powerplant") end,
  function() return AiWait("unit-powerplant") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiWait("unit-magmapump") end,
  function() return AiNeed("unit-biggunturret") end,
  function() return AiSleep((SyncRand(100)+40)*GameSettings.Difficulty) end, 

  -- Defense
  function() return AiNeed("unit-nukepowerplant") end,
  function() return AiNeed("unit-harvester") end,
  function() return AiForce(0, {"unit-assault", 2,
                                "unit-bazoo", 1,
                                "unit-buggy", 1}) end,
  function() return AiWaitForce(0) end,
  function() return AiSleep((SyncRand(100)+40)*GameSettings.Difficulty) end,

  -- Bigger attack wave
  function() return AiNeed("unit-magmapump") end,
  function() return AiForce(3, {"unit-assault", 3,
                                "unit-grenadier", 3,
                                "unit-tank", 2,
                                "unit-rtank", 1}) end,
  function() return AiWaitForce(3) end,
  function() return AiSleep((SyncRand(30)+50)*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(3) end,

  -- Defense
  function() return AiSet("unit-engineer", 6) end,
  function() return AiSet("unit-gturret", 4) end,
  function() return AiSet("unit-camp", 2) end,
  function() return AiSet("unit-biggunturret", 2) end,
  function() return AiSet("unit-magmapump", 5) end,
  function() return AiSet("unit-vfac", 2) end,
  function() return AiNeed("unit-aircraftfactory") end,
  -- function() return AiWait("unit-aircraftfactory") end,

  -- ============================================================

  -- Attack wave
  function() 
    LocalDebugPrint("is starting loop.");
    state.loop_start = state.loop_pos;
    LocalDebugPrint("is making 6 bazoos.");
    return AiForce(4, {"unit-bazoo", 6}) 
  end,
  function() return AiWaitForce(4) end,
  function() return AiSleep((SyncRand(100)+40)*GameSettings.Difficulty) end,
  function()
    LocalDebugPrint("is attacking with force 4 (6 bazoos).");
    return AiAttackWithForce(4)
  end,

  function() return AiForce(4, {"unit-grenadier", 7}) end,
  function() return AiWaitForce(4) end,
  function() return AiSleep((SyncRand(100)+40)*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(4) end,

  function() return AiForce(4, {"unit-tank", 3}) end,
  function() return AiWaitForce(4) end,
  function() return AiSleep((SyncRand(100)+40)*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(4) end,

  function() return AiForce(4, {"unit-jet", 2}) end,
  function() return AiWaitForce(4) end,
  function() return AiSleep((SyncRand(100)+40)*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(4) end,

  function() return AiForce(4, {"unit-rtank", 2}) end,
  function() return AiWaitForce(4) end,
  function() return AiSleep((SyncRand(100)+40)*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(4) end,

  -- Defense (resources and static guns)
  function() return AiSet("unit-nukepowerplant", 2) end,
  function() return AiSet("unit-gturret", 5) end,
  function() return AiSet("unit-vfac", 2) end,
  function() return AiSet("unit-camp", 2) end,
  function() return AiSet("unit-engineer", 8) end,
  function() return AiSet("unit-magmapump",7) end,
  function() return AiSet("unit-biggunturret", 3) end,

  -- Defense force 0 is sent to a building under attack
  function() return AiForce(0, {"unit-assault", 3,
                                "unit-bazoo", 2,
                                "unit-buggy", 2,
                                "unit-jet", 1,
                                "unit-tank", 1}) end,

  -- Attack wave (for buildings with weak defense)
  function() return AiForce(5, {"unit-assault", 4,
                                "unit-bazoo", 3,
                                "unit-rtank", 1,
                                "unit-jet", 1,
                                "unit-bomber", 1,
                                "unit-tank", 2}) end,
  function() return AiWaitForce(5) end,
  function() return AiSleep((SyncRand(100)+40)*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(5) end,

  -- Attack wave (slow units)
  function() return AiForce(6, {"unit-bazoo", 12,
                                "unit-artil", 2,
                                "unit-chopper", 2}) end,
  function() return AiWaitForce(6) end,
  function() return AiSleep((SyncRand(100)+40)*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(6) end,

  -- Attack wave (fast units)
  function() return AiForce(7, {"unit-grenadier", 6,
                                "unit-buggy", 2,
                                "unit-heli", 1,
                                "unit-jet", 1}) end,
  function() return AiWaitForce(7) end,
  function() return AiSleep((SyncRand(100)+40)*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(7) end,

  -- Defense
  function() return AiSet("unit-biggunturret", 6) end,
  -- Let's not build too many cannons, since the 
  -- AI places them mostly in useless locations:
  function() return AiSet("unit-cannon", 1) end,
  -- A radar is needed for the cannon to see its targets:
  function() return AiSet("unit-radar", 1) end,

  function()
    LocalDebugPrint("Reached the end of AI script and will loop");
    state.loop_pos = state.loop_start - 1; -- AiLoop will immediately increment it.
    return false
  end,
}

local function AiAttackDefense()
  LocalDebugPrint("Script position " .. AiState[AiPlayer()].loop_pos);
  return AiLoop(ai_funcs)
end

this_ai_type = {
  Ident = "ai-attackdefense",
  Name = _("Attack/Defense"),
  Init = InitAiScripts_attackdefense,
  EachSecond = AiAttackDefense,
}
DefineAiType(this_ai_type)
