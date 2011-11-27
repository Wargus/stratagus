--     ____                _       __
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  )
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/
--
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	spacious.lua	
--      Define a AI that needs not too many resources, for areas with lots of space.
--      It attacks with fast equipment, also through the air.
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
--	$Id: spacious.lua  $
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

local function InitAiScripts_spacious()
  AiState[AiPlayer()] = {
    loop_pos = 1,
    loop_start = nil,
  }
end

local ai_funcs = {
  -- If we loose an engineer, we want to keep the number of engineers on 3:
  function() return AiSet("unit-engineer", 3) end,

  -- Build magma pump and power plant
  function() return AiNeed("unit-magmapump") end,
  function() return AiSleep((SyncRand(300)+10) * GameSettings.Difficulty) end,
  function() return AiNeed("unit-powerplant") end,
  function() return AiSleep((SyncRand(300)+20) * GameSettings.Difficulty) end,
  -- and build a camp, so that we can attack a.s.a.p.:
  function() return AiNeed("unit-camp") end,

  function() return AiWait("unit-magmapump") end,
  function() return AiWait("unit-powerplant") end,
  function() return AiWait("unit-camp") end,

  -- First contact - Very Small Attack wave
  function()
    LocalDebugPrint("now has the basic resources to start building an attack force.");
    return AiForce(1, {"unit-assault", 2})
  end,
  -- In the meanwhile, the engineers can build a second magnapump:
  function() return AiSet("unit-magmapump", 2) end,
  -- wait a bit, then build a nukepowerplant:
  function() return AiSleep((SyncRand(300)+30) * GameSettings.Difficulty) end,
  function() return AiNeed("unit-nukepowerplant") end,
  -- now wait for the first attackwave to be ready:
  function() return AiWaitForce(1) end,
  -- We wait a bit to make it easier for beginners.
  -- And we make it random, so that when there are multiple opponents,
  -- they do not all come at the same time.
  function() return AiSleep((SyncRand(700)+300) * GameSettings.Difficulty) end,
  function() 
    LocalDebugPrint("is attacking with force 1a.");
    return AiAttackWithForce(1)
  end,

  -- Get more resources and a second camp:
  function() return AiSet("unit-camp", 2) end,
  function() return AiSet("unit-powerplant", 2) end,
  function() return AiSet("unit-magmapump", 3) end,

  function() return AiForce(1, {"unit-assault", 3}) end,
  function() return AiWaitForce(1) end,
  function()
    LocalDebugPrint("is attacking with force 1b.");
    return AiAttackWithForce(1)
  end,

  function() return AiSet("unit-nukepowerplant", 2) end,

  -- force 0 is sent to a building under attack
  function() return AiForce(0, {"unit-assault", 4}) end,

  -- We need a few more engineers, so that they are spread out better:
  function() return AiSet("unit-engineer", 6) end,
  function() return AiNeed("unit-vfac") end,
  function() return AiSet("unit-gturret", 2) end,

  -- Small Attack wave
  function() return AiForce(1, {"unit-assault", 4}) end,
  function() return AiWaitForce(1) end,
  function() return AiSet("unit-magmapump", 5) end,

  function() return AiSleep((SyncRand(600)+400) * GameSettings.Difficulty) end,
  function()
    LocalDebugPrint("is attacking with force 1c.");
    return AiAttackWithForce(1)
  end,

  -- Attack wave
  function() return AiSleep((SyncRand(600)+400) * GameSettings.Difficulty) end,
  function() return AiForce(4, {"unit-buggy", 2, "unit-rtank", 2}) end,
  function() return AiWaitForce(4) end,
  function() return AiSleep((SyncRand(600)+400) * GameSettings.Difficulty) end,
  function()
    LocalDebugPrint("is attacking with force 1d.");
    return AiAttackWithForce(4)
  end,

  function() return AiSet("unit-nukepowerplant", 3) end,
  function() return AiSet("unit-magmapump", 7) end,
  function() return AiNeed("unit-biggunturret") end,
  function() return AiSleep((SyncRand(600)+400) * GameSettings.Difficulty) end,

  -- Defense
  function() return AiSet("unit-nukepowerplant", 4) end,
  function() return AiNeed("unit-harvester") end,
  function() return AiForce(0, {"unit-assault", 4,
                                "unit-bazoo", 2,
                                "unit-grenadier", 2}) end,

  function() return AiSet("unit-magmapump", 9) end,

  -- Build some more defense and start flying:
  function() return AiNeed("unit-aircraftfactory") end,
  function() return AiSet("unit-gturret", 3) end,
  function() return AiSet("unit-biggunturret", 2) end,
  function() return AiSet("unit-vfac", 2) end,
  -- We want to fly, so let's wait for an aircraftfactory to be built:
  function() return AiWait("unit-aircraftfactory") end,
  function() return AiSet("unit-aircraftfactory", 2) end,

  -- ============================================================

  function() 
    LocalDebugPrint("is starting loop.");
    state.loop_start = state.loop_pos;
    return false
  end,

  -- Attack wave (air)
  function() return AiForce(2, {"unit-jet", 2}) end,


  -- Defense force 0 is sent to a building under attack
  function() return AiForce(0, {"unit-assault", 4,
                                "unit-bazoo", 3,
                                "unit-grenadier", 3}) end,

  -- Attack wave (air)
  function() return AiForce(5, {"unit-chopper", 1}) end,

  -- Attack wave (land)
  function() return AiForce(1, {"unit-assault", 4}) end,

  -- Attack wave (vehicles)
  function() return AiForce(3, {"unit-rtank", 1,
                                "unit-tank", 3}) end,

  -- Attack wave (mixed land)
  function() return AiForce(4, {"unit-buggy", 2,
                                "unit-rtank", 2}) end,

  -- Attack wave (land)
  function() return AiForce(6, {"unit-bazoo", 6}) end,

  -- wait 10 to 40 s:
  function() return AiSleep((SyncRand(900)+300) * GameSettings.Difficulty) end,

  function()
    local f
    for f=1,6 do
      if (AiCheckForce(f)) then
        LocalDebugPrint("is attacking with force " .. f .. ".");
        AiWaitForce(f);
        return AiAttackWithForce(f)
      end
    end
    return AiSleep((SyncRand(100)+40) * GameSettings.Difficulty)
  end,

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

local function AiSpacious()
  LocalDebugPrint("Script position " .. AiState[AiPlayer()].loop_pos);
  return AiLoop(ai_funcs)
end

this_ai_type = {
  Ident = "ai-spacious",
  Name = _("Spacious"),
  Init = InitAiScripts_spacious,
  EachSecond = AiSpacious,
}
DefineAiType(this_ai_type)
