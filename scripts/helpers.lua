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
--	helpers.lua		-	Stratagus helper functions.
--
--	(c) Copyright 2004 by François Beerten
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

if(not helpersInited) then
  helpersInited = true

  --- 
  -- Ai helper functions. These function are to be used in coroutines based
  -- ai scripts.

  OldAiWait=AiWait
  function AiWait(unit)
    while OldAiWait(unit) do
      coroutine.yield()
    end
  end

  OldAiSleep = AiSleep
  function AiSleep(cycles)
    while OldAiSleep(cycles) do
      coroutine.yield()
    end
  end

  OldAiWaitForce = AiWaitForce
  function AiWaitForce(forceId)
    while OldAiWaitForce(forceId) do
      coroutine.yield()
    end
  end

  OldAiTimedWaitForce = AiTimedWaitForce
  function AiTimedWaitForce(forceId, maxCycles)
    while OldAiTimedWaitForce(forceId, maxCycles) do
      coroutine.yield()
    end
  end
end