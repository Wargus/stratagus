--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      cheats.lua - Cheats
--
--      (c) Copyright 2001-2007 by Lutz Sammer and Jimmy Salmon
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


function HandleCheats(str)
  local resources = { "energy", "magma" }

  if (str == "rich") then
    ThisPlayer.EnergyStored = ThisPlayer.EnergyStorageCapacity
    ThisPlayer.MagmaStored = ThisPlayer.MagmaStorageCapacity
    AddMessage("Jackpot !")
    Cheater = true

  elseif (str == "poor") then
    ThisPlayer.EnergyStored = 0
    ThisPlayer.MagmaStored = 0
    AddMessage("Bankruptcy !")
    Cheater = true

  elseif (str == "reveal") then
    RevealMap()
    Cheater = true

  elseif (str == "fow on") then
    SetFogOfWar(true)
    Cheater = true

  elseif (str == "fow off") then
    SetFogOfWar(false)
    Cheater = true
 
  elseif (str == "see all") then
    SetFogOfWar(false)
    RevealMap()
    Cheater = true

  elseif (str == "fast debug") then
    SetSpeedBuild(10)
    SetSpeedTrain(10)
    AddMessage("FAST DEBUG SPEED")
    Cheater = true

  elseif (str == "normal debug") then
    SetSpeedBuild(1)
    SetSpeedTrain(1)
    AddMessage("NORMAL DEBUG SPEED")
    Cheater = true

  elseif (str == "speed cheat") then
    if (GetSpeedBuild() ~= 1) then
      SetSpeedBuild(1)
      SetSpeedTrain(1)
      AddMessage("NO SPEED!")
    else
      SetSpeedBuild(10)
      SetSpeedTrain(10)
      ThisPlayer.EnergyStored = ThisPlayer.EnergyStored + 32000
      ThisPlayer.MagmaStored = ThisPlayer.MagmaStored + 32000
      AddMessage("SPEED!")
    end
    Cheater = true

  elseif (str == "victory") then
    StopGame(GameVictory)
    Cheater = true

  elseif (str == "defeat") then
    StopGame(GameDefeat)
    Cheater = true

  elseif (str == "draw") then
    StopGame(GameDraw)
    Cheater = true

  elseif (str == "godcheat") then
    if (GetGodMode()) then
      SetGodMode(false)
      AddMessage("God Mode OFF")
    else
      SetGodMode(true)
      AddMessage("God Mode ON")
    end
    Cheater = true

  elseif (str == "fill mana") then
    for i = 0,ThisPlayer.TotalNumUnits-1 do
      SetUnitVariable(ThisPlayer.Units[i].Slot, "Mana", 999999)
    end
    Cheater = true

  elseif (string.sub(str, 1, 6) == "gimme ") then
    local arr = {}
    for w in string.gmatch(str,"[%w%p]+") do
      table.insert(arr, w)
    end

    local unittype = arr[2]
    local count = tonumber(arr[3])
    if (count == nil or count < 1) then
      count = 1
    elseif (count > 9) then
      count = 9
    end

    local tilex = UI.MouseViewport:Viewport2MapX(CursorX)
    local tiley = UI.MouseViewport:Viewport2MapY(CursorY)

    for i = 1, count do
      CreateUnit(unittype, "this", {tilex, tiley})
    end

    AddMessage("Cheater!")
    Cheater = true

  else
    return false
  end
  return true
end
