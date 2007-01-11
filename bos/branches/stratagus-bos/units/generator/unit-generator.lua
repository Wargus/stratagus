--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	unit-generator.lua	-	Define the generator
--
--	(c) Copyright 2001 - 2005 by François Beerten, Lutz Sammer and Crestez Leonard
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
--	$Id$

DefineIcon({
	Name = "icon-gen",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/generator_i.png"})

DefineConstruction("construction-gen", {
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

DefineAnimations("animations-gen", {
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

MakeSound("gen-selected", GetCurrentLuaPath().."/sfx_pplnt.select.wav")
MakeSound("gen-ready", GetCurrentLuaPath().."/power.plant.completed.wav")
MakeSound("gen-help", GetCurrentLuaPath().."/power.plant.underattack.wav")
MakeSound("gen-dead", GetCurrentLuaPath().."/sfx_pplnt.die.wav")

DefineUnitType("unit-gen", {
	Name = "Generator",
	Image = {"file", GetCurrentLuaPath().."/generator.png", "size", {64, 64}},
	Shadow = {"file", GetCurrentLuaPath().."/generator_s.png", "size", {64, 64}},
	Animations = "animations-gen", Icon = "icon-gen",
	Costs = {"time", 75, "titanium", 250, "crystal", 50},
	RepairHp = 2, RepairCosts = {"titanium", 2}, Construction = "construction-gen",
	Speed = 0, HitPoints = 250, DrawLevel = 25, TileSize  = {2, 2}, BoxSize = {60, 60},
	SightRange = 1, Armor = 10 , BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 20, AnnoyComputerFactor = 45,
	Points = 100, Supply = 125, ExplodeWhenKilled = "missile-160x128-explosion",
	Corpse = {"build-dead-gen", 0}, Type = "land",
	Building = true, BuilderOutside = true,
	VisibleUnderFog = true,
	Sounds = {
		"selected", "gen-selected",
		"ready", "gen-ready",
		"help", "gen-help",
		"dead", "gen-dead"}
	})

DefineAnimations("animations-dead-gen", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 1", "wait 200", "frame 2", "wait 200", "frame 2", "wait 1", 
        "unbreakable end", "wait 1", },
    })

DefineUnitType("build-dead-gen", {
	Name = "GenCrater",
	Image = {"file", GetCurrentLuaPath().."/generator_c.png", "size", {64, 64}},
	Animations = "animations-dead-gen", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {2, 2}, BoxSize = {220, 156}, SightRange = 1,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})



DefineAllow("unit-gen", "AAAAAAAA")

DefineButton({
	Pos = 2, Level = 1, Icon = "icon-gen_b", Action = "build",
	Value = "unit-gen", Key = "g", Hint = "BUILD ~!GENERATOR",
	ForUnit = {"unit-engineer"}})

