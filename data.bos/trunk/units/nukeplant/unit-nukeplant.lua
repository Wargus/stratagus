--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	unit-nukeplant.lua	-	Define the nukeplant
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
--	$Id: unit-nukeplant.lua 287 2005-11-24 21:55:35Z feb $

DefineIcon({
	Name = "icon-nuke",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/nukeplant_i.png"})

DefineConstruction("construction-nuke", {
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

DefineAnimations("animations-nuke", {
    Still = {"frame 10", "wait 20", "frame 11", "wait 20", "frame 12", "wait 20", 
        "frame 13", "wait 20", "frame 14", "wait 20", "wait 20", },
    })

MakeSound("nuke-selected", GetCurrentLuaPath().."/sfx_pplnt.select.wav")
MakeSound("nuke-ready", GetCurrentLuaPath().."/power.plant.completed.wav")
MakeSound("nuke-help", GetCurrentLuaPath().."/power.plant.underattack.wav")
MakeSound("nuke-dead", GetCurrentLuaPath().."/sfx_pplnt.die.wav")

DefineUnitType("unit-nuke", {
	Name = "nukeplant",
	Image = {"file", GetCurrentLuaPath().."/nukeplant.png", "size", {256, 256}},
	Shadow = {"file", GetCurrentLuaPath().."/nukeplant_s.png", "size", {256, 256}},
	Animations = "animations-nuke", Icon = "icon-nuke",
	Costs = {"time", 500, "titanium", 3000, "crystal", 750},
	RepairHp = 2, RepairCosts = {"titanium", 5}, Construction = "construction-nuke",
	Speed = 0, HitPoints = 350, DrawLevel = 25, TileSize  = {7, 5}, BoxSize = {224, 160},
	SightRange = 3, Armor = 15 , BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 50, AnnoyComputerFactor = 55,
	Points = 200, Supply = 2500, ExplodeWhenKilled = "missile-160x128-explosion",
	Corpse = "build-dead-nuke", Type = "land",
	Building = true, BuilderOutside = true,
	VisibleUnderFog = true,
	Sounds = {
		"selected", "nuke-selected",
		"ready", "nuke-ready",
		"help", "nuke-help",
		"dead", "nuke-dead"}
	})

DefineAnimations("animations-nukebuild", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 1", "wait 200", "frame 2", "wait 200", "frame 2", "wait 1", 
        "unbreakable end", "wait 1", },
    })

DefineUnitType("build-dead-nuke", {
	Name = "NuclearplantCrater",
	Image = {"file", GetCurrentLuaPath().."/nukeplant_c.png", "size", {256, 256}},
	Animations = "animations-nukebuild", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10, TileSize = {7, 5},
	BoxSize = {224, 160}, SightRange = 1, BasicDamage = 0,
	PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})


DefineAllow("unit-nuke", "AAAAAAAAAAAAAAAA")

DefineButton({
	Pos = 7, Level = 1, Icon = "icon-nuke", Action = "build",
	Value = "unit-nuke", Key = "n", Hint = "BUILD ~!Nuclear Power Plant",
	ForUnit = {"unit-engineer"}})
