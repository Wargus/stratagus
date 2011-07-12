--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      just_attack.lua	- Define a AI that does not build anything, but just
--      attacks with any means available.
--      For now, this only works with assaults, bazoos and grenadiers.
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
--      $Id: just_attack.lua  $
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

function InitAiScripts_just_attack()
  ai_pos      = {1, 1, 1, 1, 1, 1, 1, 1}
  ai_loop_pos = {1, 1, 1, 1, 1, 1, 1, 1}
end


local ai_loop_funcs = {
  function() return AiForce(0, {"unit-assault", 200}) end,
  function() return AiAttackWithForce(0) end,

  function() return AiForce(2, {"unit-grenadier", 200}) end,
  function() return AiSleep((SyncRand(70)+150)*GameSettings.Difficulty) end, 
  function() return AiAttackWithForce(2) end,

  function() return AiForce(1, {"unit-bazoo", 200}) end,
  function() return AiSleep((SyncRand(70)+150)*GameSettings.Difficulty) end, 
  function() return AiAttackWithForce(1) end,

  function()
    -- print("Ai " .. player .. " is looping");
    ai_loop_pos[player] = 0;
    return false
  end,
}


local ai_funcs = {
  --function() AiDebug(false) return false end,

  function() return AiForce(0, {"unit-assault", 200}) end,
  -- function() return AiWaitForce(0) end,
  function() return AiAttackWithForce(0) end,

  -- Attack wave
  function() return AiForce(2, {"unit-grenadier", 200}) end,
  --function() return AiWaitForce(2) end,
  function() return AiSleep((SyncRand(70)+150)*GameSettings.Difficulty) end, 
  function() return AiAttackWithForce(2) end,

  -- Attack wave
  function() return AiForce(1, {"unit-bazoo", 200}) end,
  --function() return AiWaitForce(1) end,
  function() return AiSleep((SyncRand(70)+150)*GameSettings.Difficulty) end,
  function() return AiAttackWithForce(1) end,

  function() return AiSleep((SyncRand(70)+150)*GameSettings.Difficulty) end,
  function() return AiLoop(ai_loop_funcs, ai_loop_pos) end,
}

function AiJustAttack()
    -- print("Just attack " .. AiPlayer() + 1 .. " position ".. ai_pos[AiPlayer() + 1]);
    return AiLoop(ai_funcs, ai_pos)
end

RegisterAi("just_attack", "just_attack", AiJustAttack,
           InitAiScripts_just_attack)

