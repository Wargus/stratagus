--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	engineer.lua	-	Define the engineer unit.
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
	Name = "icon-engineer",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/ico_engineer.png"})

DefineAnimations("animations-engineer", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 2", "wait 2", 
        "frame 5", "move 2", "wait 2", "frame 5", "move 2", "wait 2", 
        "frame 5", "move 2", "wait 2", "frame 10", "move 2", "wait 2", 
        "frame 10", "move 2", "wait 2", "frame 10", "move 2", "wait 2", 
        "frame 10", "move 2", "wait 2", "frame 15", "move 2", "wait 2", 
        "frame 15", "move 2", "wait 2", "frame 15", "move 2", "wait 2", 
        "frame 15", "move 2", "wait 2", "frame 20", "move 2", "wait 2", 
        "frame 20", "move 2", "wait 2", "frame 20", "move 2", "wait 2", 
        "frame 20", "move 2", "unbreakable end", "wait 2", },
    Repair = {"unbreakable begin", "frame 25", "wait 8", "frame 30", "wait 2",
        "frame 35", "wait 2", "frame 40", "sound engineer-repair", "wait 8",
        "frame 35", "wait 3", "frame 30", "wait 2", "unbreakable end", "wait 1", },
    Build = {"frame 25", "wait 8", "frame 30", "wait 2", "frame 35", "wait 2", 
        "frame 40", "sound engineer-repair", "wait 8", "frame 35", "wait 3", 
        "frame 30", "wait 3", },
    Harvest_titanium = {"frame 25", "wait 8", "frame 30", "wait 2", 
        "frame 35", "wait 2", "frame 40", "sound engineer-harvest", "wait 8", 
        "frame 35", "wait 3", "frame 30", "wait 3", },
    Harvest_crystal = {"frame 25", "wait 8", "frame 30", "wait 2", 
        "frame 35", "wait 2", "frame 40", "sound engineer-harvest", "wait 8", 
        "frame 35", "wait 3", "frame 30", "wait 3", },
    Death = {"unbreakable begin", "frame 45", "wait 5", "frame 50", "wait 5", 
        "frame 55", "wait 5", "frame 50", "unbreakable end", "wait 5", },
    })

MakeSound("engineer-selected", GetCurrentLuaPath().."/engineer_select.wav")
MakeSound("engineer-acknowledge", GetCurrentLuaPath().."/engineer_action.wav")
MakeSound("engineer-ready", GetCurrentLuaPath().."/engineer.ready.wav")
MakeSound("engineer-help", GetCurrentLuaPath().."/engineer.underattack.wav")
MakeSound("engineer-die", GetCurrentLuaPath().."/engineer_die.wav")
MakeSound("engineer-repair", GetCurrentLuaPath().."/engineer_attack.wav")
MakeSound("engineer-harvest", GetCurrentLuaPath().."/engineer_attack.wav")

DefineUnitType("unit-engineer", {
	Name = "Engineer",
	Image = {"file", GetCurrentLuaPath().."/unit_engineer.png", "size", {64, 64}},
	Shadow = {"file", GetCurrentLuaPath().."/unit_engineer_s.png", "size", {64, 64}},
	DrawLevel = 19, Animations = "animations-engineer", Icon = "icon-engineer",
	Costs = {"time", 50, "titanium", 50, "crystal", 100},
	Speed = 8, HitPoints = 30, DrawLevel = 25,
	TileSize = {1, 1}, BoxSize = {17, 28},
	SightRange = 5, ComputerReactionRange = 6, PersonReactionRange = 4,
	Armor = 1, BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	MaxAttackRange = 1, Priority = 50, Points = 30, Corpse = {"unit-dead-body2", 0},
	Type = "land", Demand = 0, RightMouseAction = "harvest", RepairRange = 1,
	CanTargetLand = true, Coward = true,
	CanGatherResources = {{
		"file-when-loaded", GetCurrentLuaPath().."/unit_engineer.png",
		"resource-id", "titanium",
		"resource-capacity", 50,
		"wait-at-resource", 7,
		"wait-at-depot", 1,
		"resource-step", 1,
		"harvest-from-outside"}, 
		{"file-when-loaded", GetCurrentLuaPath().."/unit_engineer.png",
			"resource-id", "crystal",
			"resource-capacity", 50,
			"wait-at-resource", 7,
			"wait-at-depot", 1,
			"resource-step", 1,
			"harvest-from-outside"}},
	organic = true, SelectableByRectangle = true,
	Sounds = {
		"selected", "engineer-selected",
		"acknowledge", "engineer-acknowledge",
		"ready", "engineer-ready",
		"repair", "engineer-repair",
		"harvest", "crystal", "engineer-harvest",
		"harvest", "titanium", "engineer-harvest",
		"help", "engineer-help",
		"dead", "engineer-die"}
	})
DefineHumanCorpse("engineer")

DefineAllow("unit-engineer", "AAAAAAAAAAAAAAAA")
