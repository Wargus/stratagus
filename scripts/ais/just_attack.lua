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

local function InitAiScripts_just_attack()
  AiState[AiPlayer()] = {
    loop_pos = 1,
    loop_start = nil,
  }
end

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

  -- ============================================================

  function() 
    LocalDebugPrint("is starting loop.");
    state.loop_start = state.loop_pos;
    return false
  end,

  function() return AiForce(0, {"unit-assault", 200}) end,
  function() return AiAttackWithForce(0) end,

  function() return AiForce(2, {"unit-grenadier", 200}) end,
  function() return AiSleep((SyncRand(70)+150)*GameSettings.Difficulty) end, 
  function() return AiAttackWithForce(2) end,

  function() return AiForce(1, {"unit-bazoo", 200}) end,
  function() return AiSleep((SyncRand(70)+150)*GameSettings.Difficulty) end, 
  function() return AiAttackWithForce(1) end,

  function()
    LocalDebugPrint("Reached the end of AI script and will loop");
    state.loop_pos = state.loop_start - 1; -- AiLoop will immediately increment it.
    return false
  end,
}

local function AiJustAttack()
  LocalDebugPrint("Script position " .. AiState[AiPlayer()].loop_pos);
  return AiLoop(ai_funcs)
end

this_ai_type = {
  Ident = "ai-just_attack",
  Name = _("Just attack"),
  Init = InitAiScripts_just_attack,
  EachSecond = AiJustAttack,
}
DefineAiType(this_ai_type)
