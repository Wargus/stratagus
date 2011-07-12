--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      naval.lua	- Define a AI that defends itself, and
--      attacks over sea.
--
--      (c) Copyright 2010 by Michiel van der Wulp
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
--      $Id: naval.lua  $
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

function InitAiScripts_naval()
  ai_pos      = {1, 1, 1, 1, 1, 1, 1, 1}
  ai_loop_pos = {1, 1, 1, 1, 1, 1, 1, 1}
end

local ai_loop_funcs = {
  function() return AiForce(0, {"unit-destroyer", 3}) end,
  function() return AiWaitForce(0) end,
  function() return AiAttackWithForce(0) end,

  -- Always attack as soon as the 3 destroyers are finished:
  --function()
  --  if (AiCheckForce(0)) then
  --    AiWaitForce(0);
  --    return AiAttackWithForce(0)
  --  end
  --  return true
  --end,

  function()
    -- print("Ai " .. player .. " is looping"); -- debug only
    ai_loop_pos[player] = 0;
    return false
  end,
}


local ai_funcs = {
  -- Use all 4 engineers to help building a magnapump:
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-magmapump") end,
  function() return AiNeed("unit-magmapump") end,
  -- The above statement is repeated 4 times:
  -- Here the engineers cooperate on building one magmapump.
  -- Using this technique in other cases fails, since 
  -- sometimes the engineers build more than one unit!
  -- This also does not work with the AiSet() function.
  function() return AiWait("unit-magmapump") end,
  function() -- debug only
    print("Naval " .. AiPlayer() + 1 .. " Magnapump finished.");
    return false
  end,

  function() return AiNeed("unit-nukepowerplant") end,
  -- Do not start building anything else until we have the energy:
  function() return AiWait("unit-nukepowerplant") end,
  function() -- debug only
    print("Naval " .. AiPlayer() + 1 .. " NukePowerplant finished.");
    return false
  end,

  -- there is not enough real estate for a vault
  -- function() return AiNeed("unit-vault") end,
  -- function() return AiWait("unit-vault") end,
  -- function() -- debug only
  --   print("Naval " .. AiPlayer() + 1 .. " Vault finished.");
  --   return false
  -- end,

  function() return AiNeed("unit-shipyard") end,
  function() return AiNeed("unit-shipyard") end,

  function() return AiNeed("unit-gturret") end,

  function() return AiWait("unit-shipyard") end,
  -- function() -- debug only
  --  print("Naval " .. AiPlayer() + 1 .. " Shipyard finished.");
  --  return false
  -- end,

  function() return AiForce(0, {"unit-destroyer", 2}) end,
  function() return AiWaitForce(0) end,
  function() return AiAttackWithForce(0) end,
  -- function() -- debug only
  --  print(AiPlayer() + 1 .. " Attack force of 2 destroyers is ready - attacking.");
  --  return false
  --end,

  function() return AiSet("unit-magmapump",2) end,
  function() return AiSet("unit-nukepowerplant", 1) end,
  function() return AiSet("unit-engineer", 4) end,
  function() return AiSet("unit-gturret", 4) end,

  function() return AiLoop(ai_loop_funcs, ai_loop_pos) end,
}

function AiNaval()
    -- print("Naval " .. AiPlayer() + 1 .. " position ".. ai_pos[AiPlayer() + 1]); -- debug only
    return AiLoop(ai_funcs, ai_pos)
end

RegisterAi("naval", "naval", AiNaval,
           InitAiScripts_naval)

