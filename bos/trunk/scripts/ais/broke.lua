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

local player

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

function InitAiScripts_broke()
  ai_pos      = {1, 1, 1, 1, 1, 1, 1, 1}
  ai_loop_pos = {1, 1, 1, 1, 1, 1, 1, 1}
  hotspotexists = nil
end

local ai_loop_funcs = {
  -- function() print("Looping !"); return false end,

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
        -- print("Ai " .. player .. " is attacking with force " .. f .. ".");
        -- AiWaitForce(f);
        return AiAttackWithForce(f)
      end
    end
    return AiSleep((SyncRand(100)+40) * GameSettings.Difficulty)
  end,


  function() ai_loop_pos[player] = 0; return false end,
}

local function HotSpotExists()
  if (hotspotexists == nil) then
    local hotspot = UnitTypeByIdent("unit-hotspot")
    local whotspot = UnitTypeByIdent("unit-weakhotspot")
    local count = Players[PlayerNumNeutral].UnitTypesCount[hotspot.Slot]
    local wcount = Players[PlayerNumNeutral].UnitTypesCount[whotspot.Slot]
    -- print ("This map has " .. count .. " hotspots and " .. wcount .. " weak hotspots.")
    hotspotexists = ((count + wcount) ~= 0)
  end
  return hotspotexists
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

  if (not HotSpotExists()) then
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
    local order = GetBuildOrder()
    return AiNeed(order[1])
  end,
  function()
    -- local order = GetBuildOrder()
    -- print("Player" .. AiPlayer() .. " waits for " .. order[1]);
    return false
  end,
  function()
    local order = GetBuildOrder()
    return AiWait(order[1])
  end,
  function()
    local order = GetBuildOrder()
    if (order[2] ~= nil) then
      return AiNeed(order[2])
    else
      return false
    end
  end,
  function()
    local order = GetBuildOrder()
    if (order[2] ~= nil) then    
      -- print("Player" .. AiPlayer() .. " waits for " .. order[2]);
    end
    return false
  end,
  function()
    local order = GetBuildOrder()
    if (order[2] ~= nil) then
      return AiWait(order[2])
    else
      return false
    end
  end,

  function() return AiNeed("unit-nukepowerplant") end,
  function()
    -- print("Player" .. AiPlayer() .. " waits for nuke powerplant.");
    return false
  end,
  function() return AiWait("unit-nukepowerplant") end,

  function() return AiSet("unit-vault", 1) end,
  function() return AiSet("unit-engineer", 5) end,

  function() 
    if (HotSpotExists()) then
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
    -- print("Player" .. AiPlayer() .. " force 0 finished.");
    return false
  end,
  function() return AiSet("unit-camp", 2) end,

  -- Attack wave
  function() return AiForce(1, {"unit-grenadier", 6}) end,
  function() return AiWaitForce(1) end,
  function()
    -- print("Player" .. AiPlayer() .. " force 1 (6 grenadiers) finished.");
    return false
  end,
  function() return AiSleep((SyncRand(500)+500)*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(1) end,
  function()
    -- print("Player" .. AiPlayer() .. " force 1 (6 grenadiers) attacking.");
    return false;
  end,

  -- Bigger attack wave
  function() return AiForce(1, {"unit-assault", 8, "unit-grenadier", 2}) end,
  function() return AiWaitForce(1) end,

  function()
    -- print("Player" .. AiPlayer() .. " force 1 (8 assaults, 2 grenadiers) finished.");
    return false;
  end,
  function() return AiAttackWithForce(1) end,

  function() return AiSet("unit-engineer", 8) end,
  function() return AiSet("unit-biggunturret", 1) end,
  function() return AiSet("unit-vfac", 1) end,
  function() return AiSet("unit-gturret", 2) end,
  
  function()
    -- print("Player" .. AiPlayer() .. " waiting for vfac.");
    return false
  end,
  function() return AiWait("unit-vfac") end,

  -- Attack wave
  function()
    -- print("Player" .. AiPlayer() .. " vfac finished.");
    return false
  end,
  function() return AiForce(1, {"unit-assault", SyncRand(5)+1, 
                                "unit-buggy", SyncRand(2)+1}) end,
  
  function()
    -- print("Player" .. AiPlayer() .. " waiting for force 1 (assaults + buggies).");
    return false
  end,
  function() return AiWaitForce(1) end,
  function() return AiSleep((SyncRand(500)+500)*GameSettings.Difficulty) end, 
  function() return AiAttackWithForce(1) end,

  function()
    -- print("Player" .. AiPlayer() .. " attacking with force 1.");
    return false;
  end,
  function() return AiSet("unit-biggunturret", 6) end,
  function() return AiSet("unit-vfac", 2) end,
  function() return AiSet("unit-gturret", 10) end,

  -- Start flying:
  function()
    -- print("Player" .. AiPlayer() .. " more biggunturret, vfac, gturret.");
    return false
  end,
  function() return AiSet("unit-aircraftfactory", 2) end,

  function() return AiLoop(ai_loop_funcs, ai_loop_pos) end,
}

function AiBroke()
    -- print(AiPlayer() .. " position ".. ai_pos[AiPlayer() + 1]);
    return AiLoop(ai_funcs, ai_pos)
end

DefineAiType({
	Ident = "ai-broke",
	Name = _("Broke"),
	Init = InitAiScripts_broke,
	EachSecond = AiBroke })
