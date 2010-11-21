--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-hospital.lua	-	Define the hospital building
--
--	(c) Copyright 2001 - 2005 by Fran�is Beerten, Lutz Sammer and Crestez Leonard
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
	Name = "icon-hosp",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/hospital_i.png"})

DefineConstruction("construction-hosp", {
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

DefineAnimations("animations-hosp", {
    Still = {"frame 10", "wait 2", "frame 10", "wait 2", "frame 10", "wait 2", 
        "frame 10", "wait 2", },
    })

MakeSound("hosp-selected", GetCurrentLuaPath().."/sfx_hosp.select.wav")
MakeSound("hosp-ready", GetCurrentLuaPath().."/hospital.completed.wav")
MakeSound("hosp-help", GetCurrentLuaPath().."/hospital.underattack.wav")
MakeSound("hosp-dead", GetCurrentLuaPath().."/sfx_hosp.die.wav")

DefineUnitType("unit-hosp", {
    Name = "Hospital",
    Image = {"file", GetCurrentLuaPath().."/hospital.png", "size", {256, 256}},
    Shadow = {"file", GetCurrentLuaPath().."/hospital_s.png", "size", {256, 256}},
    Animations = "animations-hosp",
    Icon = "icon-hosp",
    EnergyValue = 9000,
    MagmaValue = 1500,
    RepairHp = 2,
    Construction = "construction-hosp",
    HitPoints = 350,
    DrawLevel = 25,
    TileSize = {6, 5},
    BoxSize = {192, 160},
    SightRange = 2,
    Armor = 30,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 35,
    AnnoyComputerFactor = 45,
    Points = 200,
    ExplodeWhenKilled = "missile-160x128-explosion",
    Corpse = "build-dead-hosp",
    Type = "land",
    Building = true,
    VisibleUnderFog = true,
    MaxEnergyUtilizationRate = 20,
    MaxMagmaUtilizationRate = 10,
    CanHarvestFrom = true,
    Sounds = {
        "selected", "hosp-selected",
        "ready", "hosp-ready",
        "help", "hosp-help",
        "dead", "hosp-dead"}
})

DefineAnimations("animations-dead-hosp", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 1", "wait 200", "frame 2", "wait 200", "unbreakable end", "wait 1", },
    })

DefineUnitType("build-dead-hosp", {
    Name = "HospCrater",
    Image = {"file", GetCurrentLuaPath().."/hospital_c.png", "size", {256, 256}},
    Animations = "animations-dead-hosp",
    Icon = "icon-cancel",
    HitPoints = 999,
    DrawLevel = 10,
    TileSize = {6, 5},
    BoxSize = {124, 92},
    SightRange = 1,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 0,
    Type = "land",
    Building = true,
    Vanishes = true
})

DefineAllow("unit-hosp", AllowAll)

DefineButton({
    Pos = 2, Level = 2, Icon = "icon-hosp_b", Action = "build",
    Value = "unit-hosp", Hint = "BUILD ~!HOSPITAL",
    ForUnit = {"unit-engineer"}})
