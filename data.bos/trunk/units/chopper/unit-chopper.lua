--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	unit-chopper.lua	-	Define the chopper unit.
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

DefineAnimations("animations-chopper", {
    Still = {"frame 0", "wait 10", "frame 0", "wait 10", "frame 0", "wait 600",
            "frame 0", "wait 10", "frame 0", "wait 300", 
            "rotate 1", "wait 10",},
    Move = {"unbreakable begin", "frame 15", "move 2", "wait 0",
        "frame 10", "move 2", "wait 0", "frame 10", "move 2", "wait 0",
        "frame 15", "move 2", "wait 0", "frame 15", "move 2", "wait 0",
        "frame 10", "move 2", "wait 0", "frame 10", "move 2", "wait 0",
        "frame 15", "move 2", "wait 0", "frame 15", "move 2", "wait 0",
        "frame 10", "move 2", "wait 0", "frame 10", "move 2", "wait 0",
        "frame 15", "move 2", "wait 0", "frame 15", "move 2", "wait 0",
        "frame 10", "move 2", "wait 0", "frame 10", "move 2", "wait 0",
        "frame 15", "move 2", "unbreakable end", "wait 0", },
    Attack = {"unbreakable begin", "frame 0", "wait 4", 
        "frame 5", "sound bazoo-attack", "attack", "wait 1", 
        "frame 0", "wait 2", 
        "frame 5", "attack", "wait 1", 
        "frame 0", "unbreakable end", "wait 60", },
    Death = {"unbreakable begin", "frame 10", "wait 5", "frame 15", 
        "wait 5", "frame 20", "unbreakable end", "wait 20", },
    })

DefineIcon({
	Name = "icon-chopper",
	Size = {46, 38},
	Frame = 0,
	File = "units/chopper/ico_chopper.png"})

DefineUnitType("unit-chopper", {
	Name = "Chopper",
	Image = {"file", "units/chopper/unit_chopper.png", "size", {128,128}},
	Shadow = {"file", "units/chopper/unit_chopper_s.png", "size", {128, 128}, "offset", {5,128}}, 
	Animations = "animations-chopper", Icon = "icon-chopper",
	Flip = false,
	Costs = {"time", 100, "titanium", 100, "crystal", 150},
	RepairHp = 1, RepairCosts = {"crystal", 6},
	Speed = 40, HitPoints = 50, DrawLevel = 125, TileSize  = {1, 1}, BoxSize = {64, 64},
	SightRange = 7, Armor = 20, BasicDamage = 5, PiercingDamage = 30,
	Missile = "missile-bazoo", Priority = 20, AnnoyComputerFactor = 65,
	Points = 15, Supply = 0, ExplodeWhenKilled = "missile-64x64-explosion",
	Type = "fly",
	ComputerReactionRange = 10, PersonReactionRange = 10,
	RightMouseAction = "attack",
	AirUnit = true, SelectableByRectangle = true, 
	Demand = 0, CanAttack = true, CanTargetLand = true,
	NumDirections = 8, MaxAttackRange = 7,
	Sounds = {
		"selected", "grenadier-selected",
		"acknowledge", "grenadier-acknowledge"
	}
})

DefineAllow("unit-chopper", "AAAAAAAAAAAAAAAA")

DefineButton({
	Pos = 7, Level = 0, Icon = "icon-chopper", Action = "train-unit",
	Value = "unit-chopper", Key = "c", Hint = "BUILD ~!Chopper",
	ForUnit = {"unit-vfac"}})

DefineCommonButtons({"unit-chopper"})



