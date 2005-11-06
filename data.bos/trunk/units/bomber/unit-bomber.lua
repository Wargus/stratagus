--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival
--   A GPL'd futuristic RTS game
--
--	unit-bomber.lua	-	Define the bomber unit.
--
--	(c) Copyright 2005 by François Beerten.
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

DefineAnimations("animations-bomber", {
    Still = {"frame 0", "wait 1", },
    Attack= {"unbreakable begin", 
        "frame 0", "wait 5", "attack",
        "frame 0", "wait 1", "frame 0","wait 35",
	"unbreakable end", "wait 1", },
    Move = {"unbreakable begin", 
        "frame 5", "move 3", "wait 1", "frame 5", "move 3", "wait 1", 
	"frame 5", "move 3", "wait 1", "frame 5", "move 3", "wait 1",
        "frame 5", "move 3", "wait 1", "frame 5", "move 3", "wait 1", 
	"frame 5", "move 3", "wait 1", "frame 5", "move 3", "wait 1",
        "frame 5", "move 3", "wait 1", "frame 5", "move 3", "wait 1", 
        "frame 5", "move 2", "wait 1",
	"unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 0", "wait 15","unbreakable end", "wait 1",},
    })

DefineIcon({
	Name = "icon-bomber",
	Size = {46, 38},
	Frame = 0,
	File = "units/bomber/ico_bomber.png"})

DefineMissileType("missile-bomber", {
	File = "units/bomber/missile.png",
	Size = {32, 32}, Frames = 5, NumDirections = 8,
	ImpactSound = "rocket-impact", DrawLevel = 150,
	Class = "missile-class-point-to-point", Sleep = 1, Speed = 16, Range = 16})

DefineUnitType("unit-bomber", {
	Name = "bomber Construction",
	Image = {"file", "units/bomber/unit_bomber.png", "size", {128, 128}},
	Shadow = {"file", "units/bomber/unit_bomber_s.png", "size", {128, 128}, "offset", {5, 128}},
	Animations = "animations-bomber", Icon = "icon-bomber",
	Flip = false,
	Costs = {"time", 100, "titanium", 100, "crystal", 150},
	RepairHp = 1, RepairCosts = {"crystal", 6},
	Speed = 20, HitPoints = 50, DrawLevel = 125, TileSize  = {2, 2}, BoxSize = {64, 64},
	SightRange = 7, Armor = 20, BasicDamage = 50, PiercingDamage = 30,
	Missile = "missile-bomber", Priority = 20, AnnoyComputerFactor = 65,
	Points = 15, Supply = 0, ExplodeWhenKilled = "missile-64x64-explosion",
	Type = "fly",
	ComputerReactionRange = 10, PersonReactionRange = 10,
	RightMouseAction = "attack",
	AirUnit = true, SelectableByRectangle = true, 
	Demand = 0, CanAttack = true, CanTargetLand = true,
	NumDirections = 8, MaxAttackRange = 1,
	Sounds = {
		"selected", "grenadier-selected",
		"acknowledge", "grenadier-acknowledge"
	}
})

DefineAllow("unit-bomber", "AAAAAAAAAAAAAAAA")

DefineButton({
	Pos = 2, Level = 0, Icon = "icon-bomber", Action = "train-unit",
	Value = "unit-bomber", Key = "r", Hint = "BUILD ~!bomber",
	ForUnit = {"unit-dev-yard"}})

DefineCommonButtons({"unit-bomber"})



