--       _________ __                 __                               
--      /   _____//  |_____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--			  T H E   W A R   B E G I N S
--	   Stratagus - A free fantasy real time strategy game engine
--
--	unit-buggy.lua	-	Define the buggy unit.
--
--	(c) Copyright 2004-2005 by gorm.
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

DefineAnimations("animations-dead_buggy", {
    Death = {"unbreakable begin", "frame 30", "wait 50", 
        "frame 35", "sound grenade-impact", "wait 1", "frame 40", "wait 1", 
        "frame 45", "wait 1", "frame 50", "wait 1", "frame 55", "wait 1", 
        "frame 60", "wait 1","frame 65", "wait 2", 
        "unbreakable end", "wait 1", },
    })
 
DefineAnimations("animations-buggy", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 0", "move 2", "wait 1",
        "frame 0", "move 2", "wait 1", "frame 0", "move 2", "wait 1", 
        "frame 0", "move 2", "wait 1", "frame 5", "move 2", "wait 1", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame 0", "move 2", "wait 1", 
        "frame 0", "move 2", "wait 1", "frame 0", "move 2", "wait 1", 
        "frame 0", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", "frame 0", "wait 10", 
        "frame 25", "sound assault-attack", "attack", "wait 1", 
        "frame 0", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 30", "unbreakable end", "wait 1", },
    })

DefineIcon({
	Name = "icon-buggy",
	Size = {46, 38},
	Frame = 0,
	File = "units/buggy/ico_buggy.png"})

MakeSound("buggy-selected", "units/buggy/buggy_select.wav")
MakeSound("buggy-acknowledge", "units/buggy/buggy_action.wav")
MakeSound("buggy-ready", "units/buggy/buggy_ready.wav")
MakeSound("buggy-help", "units/buggy/buggy_attacked.wav")
MakeSound("buggy-die", "units/buggy/buggy_die.wav")

DefineUnitType("buggy_destroyed", {
	Name = "DestroyedBuggy",
	Image = {"file", "units/buggy/unit_buggy.png", "size", {64, 64}},
	Shadow = {"file", "units/buggy/unit_buggy_s.png", "size", {64, 64}},
	Animations = "animations-dead_buggy", Icon = "icon-cancel",
	Flip = false,
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {1, 1}, BoxSize = {62, 62}, SightRange = 2,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Vanishes = true,
	Sounds = {
		"dead", "grenade-impact",
	}
})

DefineUnitType("unit-buggy", {
	Name = "Buggy",
	Image = {"file", "units/buggy/unit_buggy.png", "size", {64, 64}},
	Shadow = {"file", "units/buggy/unit_buggy_s.png", "size", {64, 64}},
	Animations = "animations-buggy", Icon = "icon-buggy",
	Flip = false,
	Costs = {"time", 100, "titanium", 100, "crystal", 200},
	RepairHp = 1, RepairCosts = {"crystal", 3},
	Speed = 20, HitPoints = 40, DrawLevel = 25, TileSize  = {1, 1},
	BoxSize = {62, 62},
	SightRange = 7, ComputerReactionRange = 7, PersonReactionRange = 7,
	Armor = 3 , BasicDamage = 5, PiercingDamage = 0,
	Missile = "missile-none", Priority = 20, AnnoyComputerFactor = 45,
	Points = 100, Supply = 0, ExplodeWhenKilled = "missile-64x64-explosion",
	Corpse = {"buggy_destroyed", 0}, Type = "land",
	MaxAttackRange = 6, CanAttack = true, CanTargetLand = true,
	RightMouseAction = "attack",
	LandUnit = true, SelectableByRectangle = true,
	VisibleUnderFog = true,
	Sounds = {
		"selected",    "buggy-selected",
		"acknowledge", "buggy-acknowledge",
		"ready",       "buggy-ready",
		"help",        "buggy-help",
		"dead",        "buggy-die"}
})

DefineAllow("unit-buggy", "AAAAAAAAAAAAAAAA")

DefineButton({
	Pos = 3, Level = 0, Icon = "icon-buggy", Action = "train-unit",
	Value = "unit-buggy", Key = "b", Hint = "BUILD ~!BUGGY",
	ForUnit = {"unit-vfac"}})

