--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	unit-tank.lua	-	Define the tank unit.
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

DefineAnimations("animations-tank", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 0", "move 2", "wait 1", 
        "frame 0", "move 2", "wait 1", "frame 0", "move 2", "wait 1", 
        "frame 0", "move 2", "wait 1", "frame 0", "move 2", "wait 1", 
        "frame 0", "move 2", "wait 1", "frame 0", "move 2", "wait 1", 
        "frame 0", "move 2", "wait 1", "frame 0", "move 2", "wait 1", 
        "frame 0", "move 2", "wait 1", "frame 0", "move 2", "wait 1", 
        "frame 0", "move 2", "wait 1", "frame 0", "move 2", "wait 1", 
        "frame 0", "move 2", "wait 1", "frame 0", "move 2", "wait 1", 
        "frame 0", "move 2", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", "frame 0", "wait 10", 
        "frame 5", "sound bazoo-attack", "attack", "wait 2", 
        "frame 10", "wait 4", "frame 0", "unbreakable end", "wait 50", },
    Death = {"unbreakable begin", "frame 15", "wait 4", 
        "frame 20", "wait 2", "frame 25", "wait 2", "frame 30", "wait 2", 
        "frame 35", "unbreakable end", "wait 2", },
    })

DefineIcon({
	Name = "icon-tank",
	Size = {48, 39},
	Frame = 0,
	File = "units/tank/ico_tank.png"})

DefineUnitType("unit-tank", {
	Name = "Tank",
	Image = {"file", "units/tank/unit_tank.png", "size", {96, 96}},
	Shadow = {"file", "units/tank/unit_tank_s.png", "size", {96, 96}},
	Animations = "animations-tank", Icon = "icon-tank",
	Flip = false,
	Costs = {"time", 150, "titanium", 200, "crystal", 100},
	RepairHp = 1, RepairCosts = {"crystal", 6},
	Speed = 12, HitPoints = 200, DrawLevel = 25, TileSize  = {1, 1}, BoxSize = {64, 64},
	SightRange = 6, Armor = 20, BasicDamage = 10, PiercingDamage = 40,
	Missile = "missile-bazoo", Priority = 20, AnnoyComputerFactor = 65,
	Points = 15, Supply = 0, ExplodeWhenKilled = "missile-64x64-explosion",
	Type = "land",
	ComputerReactionRange = 10, PersonReactionRange = 10,
	RightMouseAction = "attack",
	LandUnit = true, SelectableByRectangle = true, 
	Demand = 0, CanAttack = true, CanTargetLand = true, CanTargetAir = true,
	NumDirections = 8, MaxAttackRange = 6,
	Sounds = {
		"selected", "assault-selected",
		"acknowledge", "assault-acknowledge"
	}
})

DefineAllow("unit-tank", "AAAAAAAA")
DefineDependency("unit-tank", {"unit-vfac"})

DefineButton({
	Pos = 4, Level = 0, Icon = "icon-tank", Action = "train-unit",
	Value = "unit-tank", Key = "t", Hint = "BUILD ~!TANK",
	ForUnit = {"unit-vfac"}})

DefineCommonButtons({"unit-tank"})




