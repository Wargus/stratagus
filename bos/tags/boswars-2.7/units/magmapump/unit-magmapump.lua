--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-magmapump.lua	-	Define the magmapump
--
--	(c) Copyright 2001-2007 by Fran�is Beerten, Lutz Sammer and Crestez Leonard
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
	Name = "icon-magmapump",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/magmapump_i.png"})

DefineConstruction("construction-magmapump", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 32},
		{Percent = 10, File = "main", Frame = 33},
		{Percent = 30, File = "main", Frame = 34},
		{Percent = 40, File = "main", Frame = 35},
		{Percent = 50, File = "main", Frame = 36},
		{Percent = 70, File = "main", Frame = 37},
		{Percent = 80, File = "main", Frame = 38}
	}
    })

DefineAnimations("animations-magmapump", {
    Still = {"frame 0", "wait 2", "frame 1", "wait 2", "frame 2", "wait 2", 
        "frame 3", "wait 2", "frame 4", "wait 2", "frame 5", "wait 2", 
        "frame 6", "wait 2", "frame 7", "wait 2", "frame 8", "wait 2",
        "frame 9", "wait 2", "frame 10", "wait 2", "frame 11", "wait 2",
        "frame 12", "wait 2", "frame 13", "wait 2", "frame 14", "wait 2",
        "frame 15", "wait 2", "frame 16", "wait 2", "frame 17", "wait 2",
        "frame 18", "wait 2", "frame 19", "wait 2", "frame 20", "wait 2",
        "frame 21", "wait 2", "frame 22", "wait 2", "frame 23", "wait 2",
        "frame 24", "wait 2", "frame 25", "wait 2", "frame 26", "wait 2",
        "frame 27", "wait 2", "wait 2", },
    })

MakeSound("magmapump-selected", GetCurrentLuaPath().."/magmapump.select.wav")
MakeSound("magmapump-ready", GetCurrentLuaPath().."/magmapump.completed.wav")
MakeSound("magmapump-help", GetCurrentLuaPath().."/magmapump.underattack.wav")
MakeSound("magmapump-dead", GetCurrentLuaPath().."/sfx_pplnt.die.wav")

DefineUnitType("unit-magmapump", {
    Name = "Magma Pump",
    Image = {"file", GetCurrentLuaPath().."/magmapump.png", "size", {64, 64}},
    Shadow = {"file", GetCurrentLuaPath().."/magmapump_s.png", "size", {64, 64}},
    Animations = "animations-magmapump",
    Icon = "icon-magmapump",
    EnergyValue = 800,
    MagmaValue = 150,
    RepairHp = 2,
    Construction = "construction-magmapump",
    HitPoints = 250,
    DrawLevel = 25,
    TileSize = {2, 2},
    BoxSize = {60, 60},
    SightRange = 1,
    Armor = 10,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 20,
    AnnoyComputerFactor = 45,
    Points = 100,
    ExplodeWhenKilled = "missile-160x128-explosion",
    Corpse = "build-dead-magmapump",
    Type = "land",
    Building = true,
    VisibleUnderFog = true,
    MagmaProductionRate = 12,
    BuildingRules = {{"ontop",
    {Type = "unit-hotspot"}},{"ontop",
    {Type = "unit-weakhotspot"}}},
    CanHarvestFrom = true,
    Sounds = {
        "selected", "magmapump-selected",
        "ready", "magmapump-ready",
        "help", "magmapump-help",
        "dead", "magmapump-dead"}
})

DefineAnimations("animations-dead-magmapump", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 1", "wait 200", "frame 2", "wait 200", "frame 2", "wait 1", 
        "unbreakable end", "wait 1", },
    })

DefineUnitType("build-dead-magmapump", {
    Name = "Pump Crater",
    Image = {"file", GetCurrentLuaPath().."/magmapump_c.png", "size", {64, 64}},
    Animations = "animations-dead-magmapump",
    Icon = "icon-cancel",
    HitPoints = 999,
    DrawLevel = 10,
    TileSize = {2, 2},
    BoxSize = {220, 156},
    SightRange = 1,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 0,
    Type = "land",
    Building = true,
    Vanishes = true
})



DefineAllow("unit-magmapump", AllowAll)

DefineButton({
    Pos = 2, Level = 1, Icon = "icon-magmapump_b", Action = "build",
    Value = "unit-magmapump", Hint = "BUILD ~!MAGMA PUMP",
    ForUnit = {"unit-engineer"}})

