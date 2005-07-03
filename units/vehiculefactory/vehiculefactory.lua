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
--	vehiculefactory.lua	-	Define the vehiculefactory
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

DefineIcon({
	Name = "icon-vfac",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/vehicle_factory_i.png"})

DefineConstruction("construction-vfac", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 20},
		{Percent = 10, File = "main", Frame = 21},
		{Percent = 20, File = "main", Frame = 22},
		{Percent = 30, File = "main", Frame = 23},
		{Percent = 40, File = "main", Frame = 24},
		{Percent = 50, File = "main", Frame = 25},
		{Percent = 60, File = "main", Frame = 26},
		{Percent = 70, File = "main", Frame = 27},
		{Percent = 80, File = "main", Frame = 28},
		{Percent = 90, File = "main", Frame = 0}
	}
    })
DefineAnimations("animations-vfac", {
    Still = {"frame 0", "wait 3", "frame 1", "wait 3", "frame 2", "wait 3", 
        "frame 3", "wait 3", "frame 4", "wait 3", "frame 5", "wait 3", 
        "frame 6", "wait 3", "frame 7", "wait 3", "frame 8", "wait 3", },
    })
DefineUnitType("unit-vfac", {
	Name = "Vehicle Factory",
	Image = {"file", GetCurrentLuaPath().."/vehicle_factory.png", "size", {224, 160}},
	Shadow = {"file", GetCurrentLuaPath().."/vehicle_factory_s.png", "size", {224, 160}},
	Animations = "animations-vfac", Icon = "icon-vfac",
	Costs = {"time", 200, "titanium", 750, "crystal", 100},
	RepairHp = 2, RepairCosts = {"titanium", 2}, Construction = "construction-vfac",
	Speed = 0, HitPoints = 550, DrawLevel = 25, TileSize = {7, 5},
	BoxSize = {220, 156}, SightRange = 2, Armor = 30, BasicDamage = 0,
	PiercingDamage = 0, Missile = "missile-none", Priority = 35,
	AnnoyComputerFactor = 45, Demand = 400, Points = 200,
	ExplodeWhenKilled = "missile-160x128-explosion", Corpse = {"build-dead-body6", 0},
	Type = "land",  Building = true, BuilderOutside = true, VisibleUnderFog = true,
	Sounds = {
		"selected", "vfac-selected",
		"ready", "vfac-ready",
		"help", "vfac-help",
		"dead", "vfac-dead"}
	})
DefineAnimations("animations-elitebuild6", {
    Death = {"unbreakable begin", "wait 1", "frame 30", "wait 2000", 
        "frame 30", "wait 200", "frame 30", "wait 200", "frame 31", "wait 200",
        "frame 31", "wait 200", "frame 31", "wait 1", "unbreakable end", "wait 1", },
    })
DefineUnitType("build-dead-body6", {
	Name = "FactoryCrater",
	Image = {"file", GetCurrentLuaPath().."/vehicle_factory.png", "size", {224, 160}},
	Animations = "animations-elitebuild6", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10, TileSize = {7, 5},
	BoxSize = {220, 156}, SightRange = 1, BasicDamage = 0,
	PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})
