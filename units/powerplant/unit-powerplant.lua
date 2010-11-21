--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-research.lua	-	Define the research facility
--
--	(c) Copyright 2001 - 2008 by Francois Beerten, Lutz Sammer and Crestez Leonard
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
	Name = "icon-powerplant",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/powerplant_i.png"})

DefineConstruction("construction-powerplant", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 0},
		{Percent = 10, File = "main", Frame = 1},
		{Percent = 20, File = "main", Frame = 2},
		{Percent = 30, File = "main", Frame = 3},
		{Percent = 40, File = "main", Frame = 4},
		{Percent = 50, File = "main", Frame = 5},
		{Percent = 60, File = "main", Frame = 6},
		{Percent = 70, File = "main", Frame = 7},
		{Percent = 80, File = "main", Frame = 8},
		{Percent = 90, File = "main", Frame = 9}
	}
    })

DefineAnimations("animations-powerplant", {
    Still = {"frame 10", "wait 3", "frame 11", "wait 3", "frame 12", "wait 3",
            "frame 13", "wait 3", "frame 14", "wait 3", "frame 13", "wait 3",
            "frame 12", "wait 3", "frame 11", "wait 3", "frame 10", "wait 3", },
    Train = {"frame 15", "wait 3", "frame 16", "wait 3", "frame 17", "wait 3",
             "frame 18", "wait 3", "frame 19", "wait 3", "frame 18", "wait 3", 
             "frame 17", "wait 3", "frame 16", "wait 3", "frame 15", "wait 3", },
    })

MakeSound("powerplant-selected", GetCurrentLuaPath().."/sfx_pplnt.select.wav")
MakeSound("powerplant-ready", GetCurrentLuaPath().."/power.plant.completed.wav")
MakeSound("powerplant-help", GetCurrentLuaPath().."/power.plant.underattack.wav")
MakeSound("powerplant-dead", GetCurrentLuaPath().."/sfx_pplnt.die.wav")

DefineUnitType("unit-powerplant", {
    Name = "Power Plant",
    Image = {"file", GetCurrentLuaPath().."/powerplant.png", "size", {128, 128}},
    Shadow = {"file", GetCurrentLuaPath().."/powerplant_s.png", "size", {128, 128}},
    Animations = "animations-powerplant",
    Icon = "icon-powerplant",
    EnergyValue = 900,
    MagmaValue = 150,
    RepairHp = 2,
    Construction = "construction-powerplant",
    HitPoints = 350,
    DrawLevel = 25,
    TileSize = {4, 4},
    BoxSize = {124, 124},
    SightRange = 1,
    Armor = 30,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 35,
    AnnoyComputerFactor = 45,
    Points = 200,
    DeathExplosion = largeExplosion,
    Corpse = "build-dead-powerplant",
    Type = "land",
    Building = true,
    VisibleUnderFog = true,
    EnergyProductionRate = 20,
    CanHarvestFrom = true,
    Sounds = {
        "selected", "powerplant-selected",
        "ready", "powerplant-ready",
        "help", "powerplant-help",
        "dead", "powerplant-dead"}
})

DefineAnimations("animations-dead-powerplant", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 1", "wait 200", "frame 2", "wait 200",  "unbreakable end", "wait 1", },
    })

DefineUnitType("build-dead-powerplant", {
    Name = "PowerPlantCrater",
    Image = {"file", GetCurrentLuaPath().."/powerplant_c.png", "size", {128, 128}},
    Animations = "animations-dead-powerplant",
    Icon = "icon-cancel",
    HitPoints = 999,
    DrawLevel = 10,
    TileSize = {4, 4},
    BoxSize = {124, 124},
    SightRange = 1,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 0,
    Type = "land",
    Building = true,
    Vanishes = true
})

DefineAllow("unit-powerplant", AllowAll)

DefineButton({
    Pos = 6, Level = 1, Icon = "icon-powerplant_b", Action = "build",
    Value = "unit-powerplant", Hint = "BUILD ~!POWER PLANT",
    ForUnit = {"unit-engineer"}})


