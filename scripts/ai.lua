--       _________ __                 __                               
--      /   _____//  |_____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--			  T H E   W A R   B E G I N S
--	   Stratagus - A free fantasy real time strategy game engine
--
--	ai.ccl		-	Define the AI.
--
--	(c) Copyright 2000-2002 by Lutz Sammer
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
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id: ai.lua,v 1.9 2004/11/02 03:30:09 mr-russ Exp $
--

DefineAiHelper(
  --
  -- Unit can build which buildings.
  --
  {"build", "unit-engineer",
   "unit-msilo", "unit-dev-yard", "unit-gen", "unit-camp",
   "unit-rfac", "unit-hosp", "unit-vfac", "unit-vault", "unit-gturret", "unit-plate1"},
  --
  -- Building can train which units.
  --
  {"train", "unit-vault", "unit-engineer"},
  {"train", "unit-camp", "unit-assault", "unit-bazoo", "unit-grenadier"},
  {"train", "unit-hosp", "unit-medic", "unit-dorcoz"},
  {"train", "unit-vfac", "unit-apcs", "unit-harvester"},
  --
  -- Building can research which spells or upgrades.
  --
  {"research", "unit-rfac", "upgrade-expl", "upgrade-expl2",
  "upgrade-tdril", "upgrade-pdril", "upgrade-ddril"},
  --
  -- Unit can repair which units.
  --
  {"repair", "unit-engineer",
   "unit-msilo", "unit-dev-yard", "unit-gen", "unit-camp", "unit-apcs",
   "unit-rfac", "unit-hosp", "unit-vfac", "unit-vault", "unit-plate1"},
  --
  -- Reduce unit limits.
  --
  {"unit-limit", "unit-gen", "food"})

local player

function AiLoop(loop_funcs, loop_pos)
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

ai_pos = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
ai_loop_pos = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}

local ai_loop_funcs = {
  function() print("Looping !"); return false end,
  function() return AiForce(1, {"unit-assault", 20}) end,
  function() return AiForce(2, {"unit-grenadier", 8}) end,
  function() return AiForce(3, {"unit-bazoo", 8}) end,
  function() return AiWaitForce(2) end,
  function() return AiWaitForce(3) end,  -- wait until attack party is completed
  function() return AiSleep(200) end,
  function() return AiAttackWithForce(1) end,
  function() return AiAttackWithForce(2) end,
  function() return AiAttackWithForce(3) end,
  function() ai_loop_pos[player] = 0; return false end,
}

local ai_funcs = {
  function() AiDebug(false) return false end,
  function() return AiSleep(AiGetSleepCycles()) end,
  function() return AiNeed("unit-vault") end,
  function() return AiSet("unit-engineer", 10) end,
  function() return AiWait("unit-vault") end,

  function() return AiNeed("unit-camp") end,
  function() return AiWait("unit-camp") end,
  function() return AiForce(0, {"unit-assault", 10}) end,
  function() return AiWaitForce(0) end, 
  function() return AiNeed("unit-camp") end,
  function() return AiSleep(500) end,
  function() return AiNeed("unit-camp") end,
  
  function() return AiForce(1, {"unit-assault", 10}) end,
  function() return AiWaitForce(1) end,
  function() return AiSleep(200) end, 
  function() return AiAttackWithForce(1) end,

  function() return AiForce(0, {"unit-assault", 20}) end,
  function() return AiNeed("unit-rfac") end,
  function() return AiResearch("upgrade-expl") end,
  function() return AiForce(1, {"unit-assault", 20, "unit-grenadier", 8}) end,
  function() return AiWaitForce(1) end, 
  function() return AiAttackWithForce(1) end,

  function() return AiResearch("upgrade-expl2") end,
  function() return AiLoop(ai_loop_funcs, ai_loop_pos) end,
}

function AiRush()
--    print(AiPlayer() .. " position ".. ai_pos[AiPlayer() + 1]);
    return AiLoop(ai_funcs, ai_pos)
end

DefineAi("ai-rush", "*", "ai-rush", AiRush)

function AiPassive()
end

DefineAi("ai-passive", "*", "ai-passive", AiPassive)

