--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--  campaign.lua  -  Define the Elite campaign 1.
--
--  (c) Copyright 2005-2006 by Lois Taulelle and François Beerten
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
--=============================================================================
--  Define the campaign
--

function CreateMapStep(map)
   function RunCampaignMap()
     Load(map) -- Needed to force the load of the presentation
     RunMap(map) 
   end
   return RunCampaignMap
end

currentCampaign = "campaigns/elites/campaign.lua"

local steps = {
  CreateMapStep("campaigns/elites/level01.smp"),
  CreateMapStep("campaigns/elites/level02.smp"),
  CreateMapStep("campaigns/elites/level03.smp"),
  CreateMapStep("campaigns/elites/level04.smp"),
  CreateMapStep("campaigns/elites/level05.smp"),
  CreateMapStep("campaigns/elites/level06.smp"),
  CreateMapStep("campaigns/elites/level07.smp"),
  CreateMapStep("campaigns/elites/level08.smp"),
  CreateMapStep("campaigns/elites/level09.smp"),
  CreateMapStep("campaigns/elites/level10.smp")}

if position == nil then
  position = 1
else
  -- We just finished a loaded game.
  if GameResult == GameVictory then
    position = position + 1
  elseif GameResult == GameDefeat then
    position = position
  else 
    return
  end
end

function RunCampaign()
  while position < 10 do
    steps[position]()
    if GameResult == GameVictory then
       position = position + 1
    elseif GameResult == GameDefeat then
       position = position
    else 
      return
    end
  end
end
RunCampaign()

currentCampaign = nil

