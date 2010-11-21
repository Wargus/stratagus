--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-aircraftfactory.lua	-	Define the aircraft factory
--
--	(c) Copyright 2001 - 2007 by Loïs Taulelle, Francois Beerten, Lutz Sammer and Crestez Leonard
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
--	$Id: unit-developmentyard.lua 8920 2007-06-23 21:28:48Z jim4 $

DefineIcon({
	Name = "icon-dev",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath() .. "/aircraftfactory_i.png"})

DefineConstruction("construction-aircraftfactory", {
        Constructions = {
                {Percent = 0, File = "main", Frame = 0},
                {Percent = 11, File = "main", Frame = 1},
                {Percent = 22, File = "main", Frame = 2},
                {Percent = 33, File = "main", Frame = 3},
                {Percent = 44, File = "main", Frame = 4},
                {Percent = 55, File = "main", Frame = 5},
                {Percent = 66, File = "main", Frame = 6},
                {Percent = 77, File = "main", Frame = 7},
                {Percent = 88, File = "main", Frame = 8}
       }
   })
DefineAnimations("animations-aircraftfactory", {
    Still = {"frame 10", "wait 60", "frame 11", "wait 60", "frame 12", "wait 60", 
        "frame 13", "wait 60", "frame 14", "wait 60" },
    Train = {"frame 15", "wait 3", "frame 16", "wait 3", "frame 17", "wait 3",
         "frame 18", "wait 30", }
})

-- TODO make new sounds for aircraftfactory
--MakeSound("aircraftfactory-selected", GetCurrentLuaPath().."/sfx_fort.select.wav")
--MakeSound("aircraftfactory-dead", GetCurrentLuaPath().."/sfx_fort.die.wav")
MakeSound("aircraftfactory-ready", GetCurrentLuaPath().."/aircraftfactory.completed.wav")
MakeSound("aircraftfactory-help", GetCurrentLuaPath().."/aircraftfactory.underattack.wav")

DefineUnitType("unit-aircraftfactory", {
    Name = "Aircraft Factory",
    Image = {"file", GetCurrentLuaPath() .. "/aircraftfactory.png", "size", {256, 256}},
    Shadow = {"file", GetCurrentLuaPath().."/aircraftfactory_s.png", "size",
 {256, 256}},
    Animations = "animations-aircraftfactory",
    Icon = "icon-dev",
    EnergyValue = 12000,
    MagmaValue = 6000,
    RepairHp = 4,
    Construction = "construction-aircraftfactory",
    HitPoints = 500,
    DrawLevel = 25,
    TileSize = {7, 6},
    BoxSize = {224, 196},
    SightRange = 4,
    Armor = 30,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 35,
    AnnoyComputerFactor = 45,
    Points = 200,
    ExplodeWhenKilled = "missile-288x288-explosion",
    Corpse = "build-dead-aircraftfactory",
    Type = "land",
    VisibleUnderFog = true,
    Building = true,
    MaxEnergyUtilizationRate = 60,
    MaxMagmaUtilizationRate = 30,
    CanHarvestFrom = true,
    Sounds = {
--        "selected", "aircraftfactory-selected",
--        "dead", "aircraftfactory-dead",
        "ready", "aircraftfactory-ready",
        "help", "aircraftfactory-help"}
})

DefineAnimations("animations-dead-aircraftfactory", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 1", "wait 200", "frame 2", "wait 200", "frame 2", "wait 1", 
        "unbreakable end", "wait 1", },
    }) 
DefineUnitType("build-dead-aircraftfactory", {
    Name = "aircraftfactoryCrater",
    Image = {"file", GetCurrentLuaPath().."/aircraftfactory_c.png", "size", {256, 256}},
    Animations = "animations-dead-aircraftfactory",
    Icon = "icon-cancel",
    HitPoints = 999,
    DrawLevel = 10,
    TileSize = {7, 6},
    BoxSize = {220, 192},
    SightRange = 1,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 0,
    Type = "land",
    Building = true,
    Vanishes = true,
    Sounds = {}
})

DefineAllow("unit-aircraftfactory", AllowAll)

DefineButton({
    Pos = 7, Level = 2, Icon = "icon-dev_b", Action = "build",
    Value = "unit-aircraftfactory", Hint = "BUILD ~!AIRCRAFT FACTORY",
    ForUnit = {"unit-engineer"}})

