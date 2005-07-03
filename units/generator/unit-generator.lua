--       _________ __                 __
--      /   _____//  |_1____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--			  T H E   W A R   B E G I N S
--	   Stratagus - A free fantasy real time strategy game engine
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
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id$

DefineAnimations("animations-gen", {
    Still = {"frame 0", "wait 2", "frame 1", "wait 2", "frame 2", "wait 2", 
        "frame 3", "wait 2", "frame 4", "wait 2", "frame 5", "wait 2", 
        "frame 6", "wait 2", "frame 7", "wait 2", "frame 8", "wait 2", },
    })
DefineIcon({
	Name = "icon-gen",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/generator_i.png"})

DefineUnitType("build-dead-body2", {
	Name = "GeneratorCrater",
	Image = {"file", GetCurrentLuaPath().."/generator.png", "size", {64, 64}},
	Animations = "animations-elitebuild2", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {2, 2}, BoxSize = {60, 60}, SightRange = 1,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})
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
	Corpse = {"build-dead-body2", 0}, Type = "land",
	Building = true, BuilderOutside = true,
	VisibleUnderFog = true,
	Sounds = {
		"selected", "gen-selected",
		"ready", "gen-ready",
		"help", "gen-help",
		"dead", "gen-dead"}
	})
