--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--      cheats.lua - Cheats
--
--      (c) Copyright 2001-2006 by Lutz Sammer and Jimmy Salmon
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
--      $Id$

speedcheat = false
godcheat = false

function HandleCheats(str)
  local resources = { "titanium", "crystal" }

  if (str == "rich") then
    SetPlayerData(GetThisPlayer(), "Resources", "titanium",
      GetPlayerData(GetThisPlayer(), "Resources", "titanium") + 12000)
    SetPlayerData(GetThisPlayer(), "Resources", "crystal",
      GetPlayerData(GetThisPlayer(), "Resources", "crystal") + 5000)
    AddMessage("!!! :)")

  elseif (str == "reveal") then
    RevealMap()

  elseif (str == "fow on") then
    SetFogOfWar(true)

  elseif (str == "fow off") then
    SetFogOfWar(false)
 
  elseif (str == "see all") then
    SetFogOfWar(false)
    RevealMap()

  elseif (str == "fast debug") then
    for i = 1,table.getn(resources) do
      SetSpeedResourcesHarvest(resources[i], 10)
      SetSpeedResourcesReturn(resources[i], 10)
    end
    SetSpeedBuild(10)
    SetSpeedTrain(10)
    SetSpeedUpgrade(10)
    SetSpeedResearch(10)
    AddMessage("FAST DEBUG SPEED")

  elseif (str == "normal debug") then
    for i = 1,table.getn(resources) do
      SetSpeedResourcesHarvest(resources[i], 1)
      SetSpeedResourcesReturn(resources[i], 1)
    end
    SetSpeedBuild(1)
    SetSpeedTrain(1)
    SetSpeedUpgrade(1)
    SetSpeedResearch(1)
    AddMessage("NORMAL DEBUG SPEED")

  elseif (str == "speed cheat") then
    if (speedcheat) then
      speedcheat = false
      for i = 1,table.getn(resources) do
        SetSpeedResourcesHarvest(resources[i], 1)
        SetSpeedResourcesReturn(resources[i], 1)
      end
      SetSpeedBuild(1)
      SetSpeedTrain(1)
      SetSpeedUpgrade(1)
      SetSpeedResearch(1)
      AddMessage("NO SO!")
    else
      speedcheat = true
      for i = 1,table.getn(resources) do
        SetSpeedResourcesHarvest(resources[i], 10)
        SetSpeedResourcesReturn(resources[i], 10)
      end
      SetSpeedBuild(10)
      SetSpeedTrain(10)
      SetSpeedUpgrade(10)
      SetSpeedResearch(10)
      for i = 1,table.getn(resources) do
        SetPlayerData(GetThisPlayer(), "Resources", resources[i],
          GetPlayerData(GetThisPlayer(), "Resources", resources[i]) + 32000)
      end
      AddMessage("SO!")
    end

  elseif (str == "victory") then
    StopGame(GameVictory)

  elseif (str == "defeat") then
    StopGame(GameDefeat)

  elseif (str == "draw") then
    StopGame(GameDraw)

  elseif (str == "godcheat") then
    if (godcheat) then
      godcheat = false
      SetGodMode(false)
      AddMessage("God Mode OFF")
    else
      godcheat = true
      SetGodMode(true)
      AddMessage("God Mode ON")
    end

  elseif (str == "fill mana") then
    for i = 0,ThisPlayer.TotalNumUnits-1 do
      SetUnitVariable(ThisPlayer.Units[i].Slot, "Mana", 255)
    end

  else
    return false
  end
  return true
end
