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
--	unit-rtank.lua	-	Define the rocket tank unit.
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
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id$

DefineNewAnimations("animations-rtank", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 4", "wait 3", 
        "frame 10", "move 4", "wait 3", "frame 5", "move 4", "wait 3", 
        "frame 10", "move 4", "wait 3", "frame 5", "move 4", "wait 3", 
        "frame 10", "move 4", "wait 3", "frame 5", "move 4", "wait 3", 
        "frame 10", "move 4", "unbreakable end", "wait 3", },
    Attack = {"unbreakable begin", "frame 0", "wait 4", 
        "frame 20", "sound bazoo-attack", "attack", "wait 1", 
        "frame 20", "sound bazoo-attack", "attack", "wait 10", 
        "frame 0", "unbreakable end", "wait 60", },
    Death = {"unbreakable begin", "frame 30", "unbreakable end", "wait 2", },
    })

DefineIcon({
	Name = "icon-rtank",
	Size = {46, 38},
	Frame = 0,
	File = "elites/units/ico_rtank.png"})

DefineUnitType("unit-rtank", {
        Name = "Rocket Tank",
        Files = {"default", "elites/units/unit_rtank.png"}, Size = {64, 64},
        Shadow = {"file", "elites/units/unit_rtank_s.png", "size", {64, 64}},
        NewAnimations = "animations-rtank", Icon = "icon-rtank",
        Flip = false,
        Costs = {"time", 100, "titanium", 100, "crystal", 150},
        RepairHp = 1, RepairCosts = {"crystal", 6},
        Speed = 20, HitPoints = 50, DrawLevel = 25, TileSize  = {1, 1}, BoxSize = {64, 64},
        SightRange = 7, Armor = 20, BasicDamage = 5, PiercingDamage = 30,
        Missile = "missile-bazoo", Priority = 20, AnnoyComputerFactor = 65,
        Points = 15, Supply = 0, ExplodeWhenKilled = "missile-64x64-explosion",
        Type = "land",
	ComputerReactionRange = 10, PersonReactionRange = 10,
	RightMouseAction = "attack",
	LandUnit = true, SelectableByRectangle = true, 
	Demand = 0, CanAttack = true, CanTargetLand = true,
	NumDirections = 8, MaxAttackRange = 7,
        Sounds = {}
        })

DefineAllow("unit-rtank", "AAAAAAAAAAAAAAAA")
DefineDependency("unit-rtank", {"unit-vfac"})

DefineButton({
        Pos = 5, Level = 0, Icon = "icon-rtank", Action = "train-unit",
        Value = "unit-rtank", Key = "r", Hint = "BUILD ~!ROCKET TANK",
        ForUnit = {"unit-vfac"}})

DefineCommonButtons({"unit-rtank"})



