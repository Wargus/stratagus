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
--	unit-harvester.lua	-	Define the harvester unit
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
	Name = "icon-harvester",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/ico_harv.png"})

DefineAnimations("animations-harvester", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "unbreakable end", "wait 1", },
    Harvest_crystal = {"frame 5", "sound harvester-harvest", "wait 6", 
        "frame 10", "wait 3", "frame 15", "wait 3", "frame 20", "wait 3", 
        "frame 25", "sound harvester-harvest", "wait 6", "frame 20", "wait 3", 
        "frame 15", "wait 3", "frame 10", "wait 3", },
    Harvest_titanium = {"frame 5", "sound harvester-harvest", "wait 6", 
        "frame 10", "wait 3", "frame 15", "wait 3", "frame 20", "wait 3", 
        "frame 25", "sound harvester-harvest", "wait 6", "frame 20", "wait 3", 
        "frame 15", "wait 3", "frame 10", "wait 3", },
    Death = {"unbreakable begin", "frame 0", "wait 5", "frame 0", "wait 5", 
        "frame 0", "wait 5", "frame 0", "unbreakable end", "wait 5", },
        })

MakeSound("harvester-selected", GetCurrentLuaPath().."/harvester_select.wav")
MakeSound("harvester-acknowledge", GetCurrentLuaPath().."/harvester_action.wav")
MakeSound("harvester-ready", GetCurrentLuaPath().."/harvester.completed.wav")
MakeSound("harvester-help", GetCurrentLuaPath().."/harvester.underattack.wav")
MakeSound("harvester-die", GetCurrentLuaPath().."/harvester_die.wav")
MakeSound("harvester-harvest", GetCurrentLuaPath().."/harvester_attack.wav")

DefineUnitType("unit-harvester", {
	Name = "Harvester",
	Image = {"file", GetCurrentLuaPath().."/unit_harv.png", "size", {96, 96}},
	Shadow = {"file", GetCurrentLuaPath().."/unit_harv_s.png", "size", {96, 96}},
	DrawLevel = 25, Animations = "animations-harvester", Icon = "icon-harvester",
	Costs = {'time', 75, 'titanium', 250, 'crystal', 100},
	RepairHp = 2, RepairCosts = {"titanium", 2},
	ExplodeWhenKilled = "missile-160x128-explosion",
	Speed = 10, HitPoints = 200, DrawLevel = 40, TileSize = {1, 1}, BoxSize = {63, 63},
	SightRange = 5, ComputerReactionRange = 6, PersonReactionRange = 4,
	Armor = 25, BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	MaxAttackRange = 0, Priority = 50, Points = 30, Type = "land",
	Demand = 50, RightMouseAction = "harvest", CanAttack = true, CanTargetLand = true,
	LandUnit = true, Coward = true,
	CanGatherResources = {{
		"file-when-loaded", GetCurrentLuaPath().."/unit_harv.png",
		"resource-id", "titanium",
		"resource-capacity", 100,
		"wait-at-resource", 2,
		"wait-at-depot", 1,
		"resource-step", 2,
		"harvest-from-outside"}, 
		{"file-when-loaded", GetCurrentLuaPath().."/unit_harv.png",
		"resource-id", "crystal",
		"resource-capacity", 100,
		"wait-at-resource", 2,
		"wait-at-depot", 1,
		"resource-step", 2,
		"harvest-from-outside"}},
	SelectableByRectangle = true,
	Sounds = {
		"selected", "harvester-selected",
		"acknowledge", "harvester-acknowledge",
		"ready", "harvester-ready",
		"harvest", "crystal", "harvester-harvest",
		"harvest", "titanium", "harvester-harvest",
		"help", "harvester-help",
		"dead", "harvester-die"}
	})
DefineAllow("unit-harvester", "AAAAAAAAAAAAAAAA")
