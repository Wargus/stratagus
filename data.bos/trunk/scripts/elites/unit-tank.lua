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
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id$

DefineAnimations("animations-tank", 
   "still", {{3, 0, 1, 0}},
   "move", {
           {0, 2, 1, 0}, {0, 2, 1, 0}, {0, 2, 1, 0}, {0, 2, 1, 0},
           {0, 2, 1, 0}, {0, 2, 1, 0}, {0, 2, 1, 0}, {0, 2, 1, 0},
           {0, 2, 1, 0}, {0, 2, 1, 0}, {0, 2, 1, 0}, {0, 2, 1, 0},
           {0, 2, 1, 0}, {0, 2, 1, 0}, {0, 2, 1, 0}, {3, 2, 1, 0},},
   "attack", {
           {2, 0, 50, 0}, {12, 0, 2, 5}, {4, 0, 4, 10}, {3, 0, 100, 0}},
   "die", {
           {0, 0, 4, 15}, {0, 0, 2, 20}, {0, 0, 2, 25}, {0, 0, 2, 30},
           {3, 0, 2, 35}})

DefineIcon({
	Name = "icon-tank",
	Size = {48, 39},
	Frame = 0,
	File = "elites/units/ico_tank.png"})

DefineUnitType("unit-tank", {
        Name = "Tank",
        Files = {"tileset-desert", "elites/units/unit_tank.png"}, Size = {96, 96},
        Shadow = {"file", "elites/units/unit_tank_s.png", "size", {96, 96}},
        Animations = "animations-tank", Icon = "icon-tank",
        Flip = false,
        Costs = {"time", 120, "titanium", 170, "crystal", 300},
        RepairHp = 1, RepairCosts = {"crystal", 6},
        Speed = 10, HitPoints = 50, DrawLevel = 25, TileSize  = {1, 1}, BoxSize = {64, 64},
        SightRange = 5, Armor = 20, BasicDamage = 10, PiercingDamage = 0,
        Missile = "missile-bazoo", Priority = 20, AnnoyComputerFactor = 65,
        Points = 10, Supply = 0, ExplodeWhenKilled = "missile-64x64-explosion",
        Type = "land",
	ComputerReactionRange = 10, PersonReactionRange = 10,
	RightMouseAction = "attack",
	LandUnit = true, SelectableByRectangle = true, 
	Demand = 0, CanAttack = true, CanTargetLand = true,
	NumDirections = 8, MaxAttackRange = 8,
        Sounds = {}
        })

DefineAllow("unit-tank", "AAAAAAAAAAAAAAAA")
DefineDependency("unit-tank", {"unit-vfac"})

DefineButton({
        Pos = 4, Level = 0, Icon = "icon-tank", Action = "train-unit",
        Value = "unit-tank", Key = "t", Hint = "BUILD ~!TANK",
        ForUnit = {"unit-vfac"}})

DefineCommonButtons({"unit-tank"})




