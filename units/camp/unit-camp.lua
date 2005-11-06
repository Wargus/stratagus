--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	unit-camp.lua	-	Define the training camp
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
	Name = "icon-camp",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/training_camp_i.png"})

DefineConstruction("construction-camp", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 0},
		{Percent = 10, File = "main", Frame = 1},
		{Percent = 20, File = "main", Frame = 2},
		{Percent = 30, File = "main", Frame = 3},
		{Percent = 40, File = "main", Frame = 4},
		{Percent = 50, File = "main", Frame = 6},
		{Percent = 60, File = "main", Frame = 7},
		{Percent = 70, File = "main", Frame = 8},
		{Percent = 80, File = "main", Frame = 9},
		{Percent = 90, File = "main", Frame = 10}
	}
    })

DefineAnimations("animations-camp", {
    Still = {"frame 10", "wait 10", "frame 11", "wait 10", "frame 12", "wait 10",
             "frame 13", "wait 10", "frame 14", "wait 10",},
    Train = {"frame 15", "wait 3", "frame 16", "wait 5", "frame 17", "wait 5",
             "frame 18", "wait 5", "frame 19", "wait 3", },
    })

MakeSound("camp-selected", GetCurrentLuaPath().."/sfx_camp.select.wav")
MakeSound("camp-ready", GetCurrentLuaPath().."/training.camp.completed.wav")
MakeSound("camp-help", GetCurrentLuaPath().."/training.camp.underattack.wav")
MakeSound("camp-dead", GetCurrentLuaPath().."/sfx_camp.die.wav")

DefineUnitType("unit-camp", {
	Name = "Training Camp",
	Image = {"file", GetCurrentLuaPath().."/training_camp.png", "size", {256, 256}},
	Shadow = {"file", GetCurrentLuaPath().."/training_camp_s.png", "size", {256, 256}},
	Animations = "animations-camp", Icon = "icon-camp",
	Costs = {"time", 100, "titanium", 300, "crystal", 150},
	RepairHp = 2, RepairCosts = {"titanium", 2},
	Construction = "construction-camp", Speed = 0, HitPoints = 500,
	DrawLevel = 25, Demand = 125, TileSize = {7, 5}, BoxSize = {220, 156},
	SightRange = 1, Armor = 25, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 30, AnnoyComputerFactor = 35,
	Points = 160, ExplodeWhenKilled = "missile-160x128-explosion",
	Corpse = {"build-dead-camp", 0}, Type = "land",
	Building = true, BuilderOutside = true, VisibleUnderFog = true,
	Sounds = {
		"selected", "camp-selected",
		"ready", "camp-ready",
		"help", "camp-help",
		"dead", "camp-dead"}
	})

DefineAnimations("animations-dead-camp", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 0", "wait 200", "frame 0", "wait 200", "frame 0", "wait 200",
        "frame 0", "wait 200", "frame 0", "wait 1", "unbreakable end", "wait 1", },
    })

DefineUnitType("build-dead-camp", {
	Name = "CampCrater",
	Image = {"file", GetCurrentLuaPath().."/training_camp.png", "size", {256, 256}},
	Animations = "animations-dead-camp", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {7, 5}, BoxSize = {220, 156}, SightRange = 1,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})

DefineAllow("unit-camp", "AAAAAAAAAAAAAAAA")


