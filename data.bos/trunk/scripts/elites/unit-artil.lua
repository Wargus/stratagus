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
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id$

DefineAnimations("animations-artil", 
   "still", {{3, 0, 1, 0}},
   "move", {
	   {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
	   {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
	   {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
	   {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
	   {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
	   {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
	   {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
           {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {3, 1, 1, 0},},
   "attack", {
           {2, 0, 20, 0}, {12, 0, 2, 5}, {0, 0, 10, 10}, {0, 0, 10, 15}, 
           {0, 0, 20, 20}, {3, 0, 20, 0}},
   "die", {
           {0, 0, 5, 30}})

DefineIcon({
	Name = "icon-artil",
	Size = {46, 38},
	Frame = 0,
	File = "elites/units/ico_artil.png"})

DefineUnitType("unit-artil", {
        Name = "Artil",
        Files = {"default", "elites/units/unit_artil.png"}, Size = {96, 96},
        Shadow = {"file", "elites/units/unit_artil_s.png", "size", {96, 96}},
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

DefineAllow("unit-artil", "AAAAAAAAAAAAAAAA")
DefineDependency("unit-artil", {"unit-vfac"})

DefineCommonButtons({"unit-artil"})

DefineButton({
        Pos = 6, Level = 0, Icon = "icon-artil", Action = "train-unit",
        Value = "unit-artil", Key = "l", Hint = "BUILD ARTI~!L",
        ForUnit = {"unit-vfac"}})


