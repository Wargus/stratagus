--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-nukeplant.lua	-	Define the nukeplant
--
--	(c) Copyright 2001 - 2007 by François Beerten, Lutz Sammer and Crestez Leonard
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
--	$Id: unit-nukeplant.lua 287 2005-11-24 21:55:35Z feb $

DefineIcon({
	Name = "icon-nukepowerplant",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/nukeplant_i.png"})

DefineConstruction("construction-nukepowerplant", {
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

DefineAnimations("animations-nukepowerplant", {
    Still = {"frame 10", "wait 20", "frame 11", "wait 20", "frame 12", "wait 20", 
        "frame 13", "wait 20", "frame 14", "wait 20", "wait 20", },
    })

MakeSound("nuke-selected", GetCurrentLuaPath().."/sfx_pplnt.select.wav")
MakeSound("nuke-ready", GetCurrentLuaPath().."/power.plant.completed.wav")
MakeSound("nuke-help", GetCurrentLuaPath().."/power.plant.underattack.wav")
MakeSound("nuke-dead", GetCurrentLuaPath().."/sfx_pplnt.die.wav")

DefineUnitType("unit-nukepowerplant", {
    Name = "Nuclear Power Plant",
    Image = {"file", GetCurrentLuaPath().."/nukeplant.png", "size", {256, 256}},
    Shadow = {"file", GetCurrentLuaPath().."/nukeplant_s.png", "size", {256, 256}},
    Animations = "animations-nukepowerplant",
    Icon = "icon-nukepowerplant",
    EnergyValue = 1600,
    MagmaValue = 270,
    EnergyProductionRate = 46,
    RepairHp = 2,
    Construction = "construction-nukepowerplant",
    HitPoints = 350,
    DrawLevel = 25,
    TileSize = {7, 5},
    BoxSize = {224, 160},
    SightRange = 3,
    Armor = 15,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 50,
    AnnoyComputerFactor = 55,
    Points = 200,
    DeathExplosion = nuclearExplosion,
    Corpse = "build-dead-nukepowerplant",
    Type = "land",
    Building = true,
    VisibleUnderFog = true,
    CanHarvestFrom = true,
    Sounds = {
        "selected", "nuke-selected",
        "ready", "nuke-ready",
        "help", "nuke-help",
        "dead", "nuke-dead"}
})

DefineAnimations("animations-dead-nukepowerplant", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 1", "wait 200", "frame 2", "wait 200", "frame 2", "wait 1", 
        "unbreakable end", "wait 1", },
    })

DefineUnitType("build-dead-nukepowerplant", {
    Name = "NuclearplantCrater",
    Image = {"file", GetCurrentLuaPath().."/nukeplant_c.png", "size", {256, 256}},
    Animations = "animations-dead-nukepowerplant",
    Icon = "icon-cancel",
    HitPoints = 999,
    DrawLevel = 10,
    TileSize = {7, 5},
    BoxSize = {224, 160},
    SightRange = 1,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 0,
    Type = "land",
    Building = true,
    Vanishes = true
})


DefineAllow("unit-nukepowerplant", AllowAll)

DefineButton({
    Pos = 7, Level = 1, Icon = "icon-nukepowerplant", Action = "build",
    Value = "unit-nukepowerplant",
    Hint = "BUILD ~!Nuclear Power Plant",
    ForUnit = {"unit-engineer"}})
