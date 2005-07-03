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
--	unit-developmentyard.lua	-	Define the development yard
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

DefineAnimations("animations-dev-yard", {
    Still = {"frame 1", "wait 20", "frame 2", "wait 20", "frame 3", "wait 20", 
        "frame 4", "wait 50", "frame 5", "wait 20", "frame 6", "wait 50", },
    })

DefineIcon({
	Name = "icon-dev",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath() .. "/development_yard_i.png"})

DefineUnitType("build-dead-body1", {
	Name = "DevelopmentyardCrater",
	Image = {"file", GetCurrentLuaPath().."/development_yard.png", "size", {224, 196}},
	Animations = "animations-elitebuild1", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {7, 6}, BoxSize = {220, 192},
	SightRange = 1, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 0, Type = "land",
	Building = true, Vanishes = true, Sounds = {}})
DefineUnitType("unit-dev-yard", {
	Name = "Development yard",
	Image = {"file", GetCurrentLuaPath() .. "/development_yard.png", "size", {224, 196}},
	Animations = "animations-dev-yard", Icon = "icon-dev",
	Costs = {"time", 150, "titanium", 300, "crystal", 300},
	RepairHp = 4, RepairCosts = {"titanium", 4}, Construction = "construction-dev-yard",
	Speed = 0, HitPoints = 1800, DrawLevel = 25, TileSize = {7, 6}, BoxSize = {224, 196},
	SightRange = 4, Armor = 30, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 35, AnnoyComputerFactor = 45,
	Points = 200, Supply = 200, ExplodeWhenKilled = "missile-288x288-explosion",
	Corpse = {"build-dead-body1", 0}, Type = "land",
	VisibleUnderFog = true,	Building = true, BuilderOutside = true,
	Sounds = {
		"selected", "dev-selected",
		"ready", "dev-ready",
		"help", "dev-help",
		"dead", "dev-dead"}
	})
