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
--	unit-missilesilo.lua	-	Define the missile silo unit
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
	Name = "icon-msilo",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/missile_silo_i.png"})

DefineConstruction("construction-msilo", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 7},
		{Percent = 10, File = "main", Frame = 7},
		{Percent = 20, File = "main", Frame = 8},
		{Percent = 30, File = "main", Frame = 9},
		{Percent = 40, File = "main", Frame = 10},
		{Percent = 50, File = "main", Frame = 11},
		{Percent = 60, File = "main", Frame = 11},
		{Percent = 70, File = "main", Frame = 12},
		{Percent = 80, File = "main", Frame = 13},
		{Percent = 90, File = "main", Frame = 0}
	}
})
DefineAnimations("animations-msilo", {
    Still = {"frame 0", "wait 1", },
    Attack = {"unbreakable begin", "frame 1", "wait 1", 
        "frame 2", "sound msilo-attack", "attack", "wait 1", 
        "frame 3", "wait 1", "frame 4", "wait 1", "frame 5", "wait 25", 
        "frame 6", "sound msilo-attack", "attack", "wait 25", 
        "frame 6", "unbreakable end", "wait 1", },
    })
DefineUnitType("unit-msilo", {
	Name = "Missile Silo",
	Image = {"file", GetCurrentLuaPath().."/missile_silo.png", "size", {128, 128}},
	Shadow = {"file", GetCurrentLuaPath().."/missile_silo_s.png", "size", {128, 128}},
	Animations = "animations-msilo", Icon = "icon-msilo",
	Costs = {"time", 2000, "titanium", 10000, "crystal", 10000},
	RepairHp = 2, RepairCosts = {"titanium", 2},
	Construction = "construction-msilo",
	Speed = 0, HitPoints = 450, DrawLevel = 25, 
	TileSize = {4, 4}, BoxSize = {124, 124},
	SightRange = 1, Armor = 10, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 20, AnnoyComputerFactor = 45,
	Points = 100, ExplodeWhenKilled = "missile-160x128-explosion",
	Corpse = {"build-dead-body4", 0}, Type = land,
	MaxMana = 1000, CanCastSpell = {"spell-nuke"},
	Demand = 400, Building = true, BuilderOutside = true,
	VisibleUnderFog = true,
	Sounds = {
		"selected", "gen-selected",
		"ready", "gen-ready",
		"help", "gen-help",
		"dead", "gen-dead"}
	})
DefineUnitType("build-dead-body7", {
	Name = "SiloCrater",
	Image = {"file", GetCurrentLuaPath().."/missile_silo.png", "size", {128, 128}},
	Animations = "animations-elitebuild7", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,	TileSize = {4, 4},
	BoxSize = {124, 124}, SightRange = 1, BasicDamage = 0,
	PiercingDamage = 0, Missile = "missile-none", Priority = 0,
	Type = "land" , Building = true, Vanishes = true})
