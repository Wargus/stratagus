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
--	(c) Copyright 1998-2004 by Crestez Leonard and François Beerten
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

-- SetTitleScreens(
--		{Image = "video/int_logo_stratagus.avi", Music =  "video/int_logo_stratagus.ogg"},
--		{Image = "video/int_logo_bos.avi", Music =  "video/int_logo_bos.ogg"})

--	Enter your menu music.
-- SetMenuMusic("music/title.ogg")

--  Enable color cyclings.
SetColorCycleAll(false)

--	Set the game name
SetGameName("bos")

--	set the default map file.
SetDefaultMap="puds/default.pud"

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

--	Define shadow-sprite.
--ShadowSprite("missiles/unit_shadow.png",3, 42, 32, 32)
SpellSprite("general/spells1.png", 1, 1, 16, 16)

--	Enable fancy building (random mirroring buildings)
SetFancyBuildings(false)

--	Enable show tips at the start of a level
SetShowTips(true)

-------------------------------------------------------------------------------
--	Game modification
-------------------------------------------------------------------------------

--	Enable XP to add more damage to attacks?
SetXPDamage(true)

--	Edit this to enable/disable extended features???
--	We should remove this junk.
extensions = true

--	Edit this to enable/disable the training queues.
SetTrainingQueue(true)

--	Always reveal the attacker.
SetRevealAttacker(true)

-------------------------------------------------------------------------------

--  Fighters move by default.
RightButtonMoves();

--	Set the name of the missile to use when clicking
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
    function() return IfUnit("this", "==", 0, "all") end,
    function() return ActionDefeat() end)

  AddTrigger(
    function() return IfOpponents("this", "==", 0) end,
    function() return ActionVictory() end)
end

-------------------------------------------------------------------------------
--	Tables-Part
-------------------------------------------------------------------------------
SetFogOfWarGraphics("general/fog.png")
DefineTileset("tileset-desert", "class", "desert", "name", "Desert", "file", "scripts/tilesets/desert.lua")
-- FIXME: todo winter
DefineTileset("tileset-winter", "class", "winter", "name", "Winter (incomplete)", "file", "scripts/tilesets/winter.lua")

Load("preferences1.lua")

--; Uses Stratagus Library path!
Load("scripts/bos.lua")
Load("scripts/icons.lua")
Load("scripts/sound.lua")
Load("scripts/missiles.lua")
Load("scripts/constructions.lua")
Load("scripts/spells.lua")
Load("scripts/units.lua")
Load("scripts/upgrade.lua")
Load("scripts/fonts.lua")
Load("scripts/buttons.lua")
Load("scripts/ui.lua")

-- Load extra units
list = ListDirectory("scripts/elites/")
for i,f in list do
  if(string.find(f, "^unit%-.*%.lua$")) then 
    print("Loading unit: " .. f) 
    Load("scripts/elites/"..f)
  end
end

Load("scripts/ai.lua")
--Load("scripts/campaigns.lua")
Load("scripts/credits.lua")
Load("scripts/tips.lua")
Load("scripts/ranks.lua")
Load("scripts/menus.lua")
Load("scripts/cheats.lua")

Load("preferences2.lua")

print("... ready!")
