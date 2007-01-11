--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	unit-artil.lua	-	Define the artil unit.
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

DefineAnimations("animations-artil", {
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
    Attack = {"unbreakable begin", "frame 0", "wait 10",
        "frame 0", "sound bazoo-attack", "attack", "wait 1",
        "frame 5", "wait 10",
        "frame 0", "wait 25", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 10", "wait 10", "unbreakable end", 
        "wait 1",},
    })


DefineIcon({
	Name = "icon-artil",
	Size = {46, 38},
	Frame = 0,
	File = "units/artil/ico_artil.png"})

DefineUnitType("unit-artil", {
	Name = "Artil",
	Image = {"file", "units/artil/unit_artil.png", "size", {160, 160}},
        Offset = {0, -10},
	Shadow = {"file", "units/artil/unit_artil_s.png", "size", {160, 160}},
	Animations = "animations-artil", Icon = "icon-artil",
	Flip = false,
	Costs = {"time", 200, "titanium", 300, "crystal", 300},
	RepairHp = 1, RepairCosts = {"crystal", 6},
	Speed = 10, HitPoints = 250, DrawLevel = 25, TileSize  = {1, 1}, BoxSize = {64, 64},
	SightRange = 5, Armor = 25, BasicDamage = 10, PiercingDamage = 50,
	Missile = "missile-bazoo", Priority = 20, AnnoyComputerFactor = 65,
	Points = 25, Supply = 0, ExplodeWhenKilled = "missile-64x64-explosion",
	Type = "land",
	ComputerReactionRange = 10, PersonReactionRange = 10,
	RightMouseAction = "attack",
	LandUnit = true, SelectableByRectangle = true, 
	Demand = 50, CanAttack = true, CanTargetLand = true,
	NumDirections = 8, MaxAttackRange = 8,
	Sounds = {}
})

DefineAllow("unit-artil", "AAAAAAAA")
DefineDependency("unit-artil", {"unit-vfac"})

DefineCommonButtons({"unit-artil"})

DefineButton({
	Pos = 6, Level = 0, Icon = "icon-artil", Action = "train-unit",
	Value = "unit-artil", Key = "l", Hint = "BUILD ARTI~!L",
	ForUnit = {"unit-vfac"}})


