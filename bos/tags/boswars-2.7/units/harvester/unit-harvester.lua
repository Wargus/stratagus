--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-harvester.lua	-	Define the harvester unit
--
--	(c) Copyright 2001 - 2010 by Francois Beerten, Lutz Sammer and Crestez Leonard
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

DefineIcon({
	Name = "icon-harvester",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/ico_harv.png"})

DefineAnimations("animations-harvester", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "unbreakable end", "wait 1", },
    Harvest = {"frame 5", "sound harvester-harvest", "wait 6", 
        "frame 10", "wait 3", "frame 15", "wait 3", "frame 20", "wait 3", 
        "frame 25", "sound harvester-harvest", "wait 6", "frame 20", "wait 3", 
        "frame 15", "wait 3", "frame 10", "wait 3", },
    Death = {"unbreakable begin", "frame 0", "wait 5", "frame 0", "wait 5", 
        "frame 0", "wait 5", "frame 0", "unbreakable end", "wait 5", },
        })

MakeSound("harvester-selected", GetCurrentLuaPath().."/harvester_select.wav")
MakeSound("harvester-acknowledge", GetCurrentLuaPath().."/harvester_action.wav")
MakeSound("harvester-ready", GetCurrentLuaPath().."/harvester.completed.wav")
MakeSound("harvester-help", GetCurrentLuaPath().."/harvester.underattack.wav")
MakeSound("harvester-die", GetCurrentLuaPath().."/harvester_die.wav")
MakeSound("harvester-harvest", GetCurrentLuaPath().."/harvester_attack.wav")

DefineUnitType("unit-harvester", {
    Name = "Harvester",
    Image = {"file", GetCurrentLuaPath().."/unit_harv.png", "size", {96, 96}},
    Shadow = {"file", GetCurrentLuaPath().."/unit_harv_s.png", "size", {96, 96}},
    Animations = "animations-harvester",
    Icon = "icon-harvester",
    Flip = true,
    EnergyValue = 3200,
    MagmaValue = 1600,
    MaxEnergyUtilizationRate = 40,
    MaxMagmaUtilizationRate = 20,
    RepairHp = 2,
    ExplodeWhenKilled = "missile-160x128-explosion",
    HitPoints = 200,
    DrawLevel = 25,
    TileSize = {1, 1},
    BoxSize = {63, 63},
    SightRange = 5,
    ComputerReactionRange = 6,
    PersonReactionRange = 4,
    Armor = 25,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    MaxAttackRange = 0,
    Priority = 50,
    Points = 30,
    Type = "land",
    RightMouseAction = "harvest",
    CanAttack = true,
    CanTargetLand = true,
    Coward = true,
    Harvester = true,
    SelectableByRectangle = true,
    Sounds = {
        "selected", "harvester-selected",
        "acknowledge", "harvester-acknowledge",
        "ready", "harvester-ready",
        "help", "harvester-help",
        "dead", "harvester-die"}
})
DefineAllow("unit-harvester", AllowAll)
