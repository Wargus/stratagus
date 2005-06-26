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
--	stratagus.lua	-	The craft configuration language.
--
--	(c) Copyright 1998-2005 by Crestez Leonard and François Beerten
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
--	$Id$

-- For documentation see stratagus/doc/ccl/ccl.html

print("Battle of Survival default config file loading ...\n")

-------------------------------------------------------------------------------
--	Config-Part
-------------------------------------------------------------------------------

--	Enter your default title screen.

SetTitleScreens(
      {Image="video/stratagus_intro.ogg"},
      {Image="video/bos_intro.ogg"})

--	Enter your menu music.
-- SetMenuMusic("music/title.ogg")

--	Set the game name
SetGameName("bos")

--	set the default map file.
SetDefaultMap="maps/default.smp"

-------------------------------------------------------------------------------
--	Music play list -	Insert your titles here
-------------------------------------------------------------------------------
playlist = {}
musiclist = ListDirectory("music/")
for i,f in musiclist do
  if(string.find(f, ".ogg$") or string.find(f, ".wav$") or string.find(f, ".mp3$")) then 
    print("Added music file:" .. f) 
    playlist[i] = f
  end
end

SetSelectionStyle("corners")
SetShowSightRange(false)
SetShowAttackRange(false)
SetShowReactionRange(false)

SetShowOrders(2)

ManaSprite("general/mana2.png", 0, -1, 31, 4)
HealthSprite("general/health2.png", 0, -4, 31, 4)

ShowHealthDot()
ShowManaDot()
ShowNoFull()

--	Enable fancy building (random mirroring buildings)
SetFancyBuildings(false)

--	Enable show tips at the start of a level
SetShowTips(true)

-------------------------------------------------------------------------------
--	Game modification
-------------------------------------------------------------------------------

--	Enable XP to add more damage to attacks?
SetXPDamage(true)

--	Edit this to enable/disable the training queues.
SetTrainingQueue(true)

--	Always reveal the attacker.
SetRevealAttacker(true)

-------------------------------------------------------------------------------

--  Fighters move by default.
RightButtonMoves();

--	Set the name of the missile to use when clicking (move order)
SetClickMissile("missile-green-cross")

--	Set the name of the missile to use when displaying damage
SetDamageMissile("missile-hit")

--	Disable grabbing the mouse.
SetGrabMouse(false)

--	Enable stopping scrolling on mouse leave.
SetLeaveStops(true)

--	Enable mouse and keyboard scrolling.
SetMouseScroll(true)
SetKeyScroll(true)
SetKeyScrollSpeed(1)
SetMouseScrollSpeed(1)
SetMouseScrollSpeedDefault(4)
SetMouseScrollSpeedControl(15)

SetDoubleClickDelay(300)
SetHoldClickDelay(1000)

--	Enable the display of the command keys in buttons.
SetShowCommandKey(true)

--  Enable fog of war by default.
SetFogOfWar(true)

--	Enable minimap terrain by default.
SetMinimapTerrain(true)

--  Set Fog of War opacity
SetFogOfWarOpacity(128)

-------------------------------------------------------------------------------
--	Define default resources
-------------------------------------------------------------------------------

DefineDefaultResources(0, 2000, 1000, 1000, 1000, 1000, 1000)
DefineDefaultResourcesLow(0, 2000, 1000, 1000, 1000, 1000, 1000)
DefineDefaultResourcesMedium(0, 5000, 2000, 2000, 2000, 2000, 2000)
DefineDefaultResourcesHigh(0, 10000, 5000, 5000, 5000, 5000, 5000)
DefineDefaultIncomes(0, 100, 100, 100, 100, 100, 100)
DefineDefaultActions("stop", "mine", "harvest", "drill", "mine", "mine", "mine")

DefineDefaultResourceNames("time", "titanium", "crystal", "gas", "ore", "stone", "coal")

DefineDefaultResourceAmounts("titanium", 150, "crystal", 150, "gas", 150)

DefinePlayerColorIndex(208, 4)

DefinePlayerColors({
  "red", {{164, 0, 0}, {124, 0, 0}, {92, 4, 0}, {68, 4, 0}},
  "blue", {{12, 72, 204}, {4, 40, 160}, {0, 20, 116}, {0, 4, 76}},
  "green", {{44, 180, 148}, {20, 132, 92}, {4, 84, 44}, {0, 40, 12}},
  "violet", {{152, 72, 176}, {116, 44, 132}, {80, 24, 88}, {44, 8, 44}},
  "orange", {{248, 140, 20}, {200, 96, 16}, {152, 60, 16}, {108, 32, 12}},
  "black", {{40, 40, 60}, {28, 28, 44}, {20, 20, 32}, {12, 12, 20}},
  "white", {{224, 224, 224}, {152, 152, 180}, {84, 84, 128}, {36, 40, 76}},
  "yellow", {{252, 252, 72}, {228, 204, 40}, {204, 160, 16}, {180, 116, 0}},
  "red", {{164, 0, 0}, {124, 0, 0}, {92, 4, 0}, {68, 4, 0}},
  "blue", {{12, 72, 204}, {4, 40, 160}, {0, 20, 116}, {0, 4, 76}},
  "green", {{44, 180, 148}, {20, 132, 92}, {4, 84, 44}, {0, 40, 12}},
  "violet", {{152, 72, 176}, {116, 44, 132}, {80, 24, 88}, {44, 8, 44}},
  "orange", {{248, 140, 20}, {200, 96, 16}, {152, 60, 16}, {108, 32, 12}},
  "black", {{40, 40, 60}, {28, 28, 44}, {20, 20, 32}, {12, 12, 20}},
  "white", {{224, 224, 224}, {152, 152, 180}, {84, 84, 128}, {36, 40, 76}},
  "yellow", {{252, 252, 72}, {228, 204, 40}, {204, 160, 16}, {180, 116, 0}},
})

--	Chittin
SetSpeeds(1)

AStar("fixed-unit-cost", 1000, "moving-unit-cost", 20, "dont-know-unseen-terrain", "unseen-terrain-cost", 2)

--	Maximum number of selectable units
SetMaxSelectable(24)

--	All player unit limit
SetAllPlayersUnitLimit(200)
--	All player building limit
SetAllPlayersBuildingLimit(200)
--	All player total unit limit
SetAllPlayersTotalUnitLimit(400)

-------------------------------------------------------------------------------
--  Default triggers for single player

function SinglePlayerTriggers()
  AddTrigger(
    function() return GetPlayerData(GetThisPlayer(),"TotalNumUnits") == 0 end,
    function() return ActionDefeat() end)

  AddTrigger(
    function() return GetNumOpponents(GetThisPlayer()) == 0 end,
    function() return ActionVictory() end)   
end

-------------------------------------------------------------------------------
--	Tables-Part
-------------------------------------------------------------------------------
SetFogOfWarGraphics("general/fog.png")

Load("preferences1.lua")

--; Uses Stratagus Library path!
Load("scripts/bos.lua")
Load("scripts/icons.lua")
Load("scripts/sound.lua")
Load("scripts/missiles.lua")
Load("scripts/constructions.lua")
Load("scripts/spells.lua")
Load("units/vault/vault.lua")
Load("scripts/units.lua")
Load("scripts/fonts.lua")
Load("scripts/buttons.lua")
Load("scripts/ui.lua")

-- Load extra units
local list
local i
local f
local ff

list = ListDirectory("units/")
for i,f in list do
  if not(string.find(f, "^%.")) then
     subdirlist = ListDirectory("units/" .. f)
     for i, ff in subdirlist do
        if(string.find(ff, "^unit-.*%.lua$")) then
          print("Loading unit: " .. ff)
          Load("units/"..f.."/"..ff)
        end
     end
  end
end

Load("scripts/upgrade.lua")
Load("scripts/ai.lua")
Load("scripts/campaigns.lua")
Load("scripts/credits.lua")
Load("scripts/tips.lua")
Load("scripts/ranks.lua")
Load("scripts/menus.lua")
Load("scripts/cheats.lua")

Load("preferences2.lua")

print("... ready!")
