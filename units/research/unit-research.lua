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
--	unit-research.lua	-	Define the research facility
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
	Name = "icon-rfac",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/research_facility_i.png"})

DefineAnimations("animations-rfac", {
    Still = {"frame 0", "wait 3", },
    })
DefineUnitType("unit-rfac", {
	Name = "Research Facility",
	Image = {"file", GetCurrentLuaPath().."/research_facility.png", "size", {128, 128}},
	Shadow = {"file", GetCurrentLuaPath().."/research_facility_s.png", "size", {128, 128}},
	Animations = "animations-rfac", Icon = "icon-rfac",
	Costs = {"time", 125, "titanium", 300, "crystal", 300},
	RepairHp = 2, RepairCosts = {"titanium", 2}, Construction = "construction-rfac",
	Speed = 0, HitPoints = 350, DrawLevel = 25, TileSize = {4, 4}, BoxSize = {124, 124},
	SightRange = 1, Armor = 30, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 35, AnnoyComputerFactor = 45,
	Demand = 300, Points = 200, ExplodeWhenKilled = "missile-160x128-explosion",
	Corpse = {"build-dead-body4", 0}, Type = "land",
	Building = true, BuilderOutside = true, VisibleUnderFog = true,
	Sounds = {
		"selected", "rfac-selected",
		"ready", "rfac-ready",
		"help", "rfac-help",
		"dead", "rfac-dead"}
	})
DefineAnimations("animations-elitebuild4", {
    Death = {"unbreakable begin", "wait 1", "frame 12", "wait 2000", 
        "frame 12", "wait 200", "frame 12", "wait 200", "frame 13", "wait 200",
        "frame 13", "wait 200", "frame 13", "wait 1", "unbreakable end", "wait 1", },
    })
DefineUnitType("build-dead-body4", {
	Name = "RfacCrater",
	Image = {"file", GetCurrentLuaPath().."/research_facility.png", "size", {128, 128}},
	Animations = "animations-elitebuild4", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {4, 4}, BoxSize = {124, 124}, SightRange = 1,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})
