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
  local resources = { "energy", "magma" }

  if (str == "rich") then
    ThisPlayer.EnergyStored = ThisPlayer.EnergyStorageCapacity
    ThisPlayer.MagmaStored = ThisPlayer.MagmaStorageCapacity
    AddMessage("Jackpot !")

  elseif (str == "poor") then
    ThisPlayer.EnergyStored = 0
    ThisPlayer.MagmaStored = 0
    AddMessage("Bankruptcy !")

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
    SetSpeedBuild(10)
    SetSpeedTrain(10)
    AddMessage("FAST DEBUG SPEED")

  elseif (str == "normal debug") then
    SetSpeedBuild(1)
    SetSpeedTrain(1)
    AddMessage("NORMAL DEBUG SPEED")

  elseif (str == "speed cheat") then
    if (speedcheat) then
      speedcheat = false
      SetSpeedBuild(1)
      SetSpeedTrain(1)
      AddMessage("NO SO!")
    else
      speedcheat = true
      SetSpeedBuild(10)
      SetSpeedTrain(10)
      ThisPlayer.EnergyStored = ThisPlayer.EnergyStored + 32000
      ThisPlayer.MagmaStored = ThisPlayer.MagmaStored + 32000
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

  elseif (str:sub(1, 6) == "gimme ") then
    local arr = {}
    for w in str:gmatch("[%w%p]+") do
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

  else
    return false
  end
  return true
end
