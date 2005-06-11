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
--	unit-gturret.lua	-	Define the gun turret unit.
--
--	(c) Copyright 2004 by François Beerten.
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

DefineAnimations("animations-gturret", {
    Still = {"frame 4", "wait 1", },
    Attack = {"unbreakable begin", "frame 0", "wait 1", "frame 5", "wait 1",
        "frame 0", "wait 1", "frame 5", "wait 1", "frame 0", "wait 1", 
        "frame 5", "sound gturret-attack", "attack", "wait 1", 
        "frame 0", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 10", "unbreakable end", "wait 50", },
    })

DefineIcon({
	Name = "icon-gturret",
	Size = {46, 38},
	Frame = 0,
	File = "elites/build/gturret_i.png"})

DefineIcon({
	Name = "icon-gturret_b",
	Size = {46, 38},
	Frame = 10,
	File = "elites/units/engineer_icons.png"})

DefineConstruction("construction-gturret", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 10},
		{Percent = 20, File = "main", Frame = 11},
		{Percent = 40, File = "main", Frame = 12},
		{Percent = 60, File = "main", Frame = 13},
		{Percent = 80, File = "main", Frame = 14}
	}
})

DefineUnitType("unit-gturret", {
	Name = "Gun Turret",
	Image = {"file", "elites/build/gturret.png", "size", {64, 64}},
	Shadow = {"file", "elites/build/generator_s.png", "size", {64, 64}},
	Animations = "animations-gturret", Icon = "icon-gturret",
	Costs = {"time", 130, "titanium", 250, "crystal", 50},
	RepairHp = 2, RepairCosts = {"titanium", 2}, Construction = "construction-gturret",
	Speed = 0, HitPoints = 160, DrawLevel = 25, TileSize  = {2, 2}, BoxSize = {60, 60},
	SightRange = 6, ComputerReactionRange = 6, PersonReactionRange = 6, Armor = 10,
	BasicDamage = 4, PiercingDamage = 0, MaxAttackRange = 6, Missile = "missile-none",
	Priority = 20, AnnoyComputerFactor = 45, Points = 100,
	ExplodeWhenKilled = "missile-160x128-explosion", RightMouseAction = "attack",
	CanAttack = true, CanTargetLand = true, NumDirections = 8,
	Corpse = {"build-dead-body2", 0}, Type = "land",
	--[[MustBuildOnTop = "unit-plate1", --]] Building = true, BuilderOutside = true,
	VisibleUnderFog = true,
	Sounds = {
		"selected", "gturret-selected",
		"ready", "gen-ready",
		"help", "gen-help",
		"dead", "gen-dead"}
})

MakeSound("gturret-selected", "elites/units/gturret_select.wav")
MakeSound("gturret-attack", "elites/units/gturret_attack.wav")

DefineAllow("unit-gturret", "AAAAAAAAAAAAAAAA")
DefineDependency("unit-gturret", {"unit-vault"})

DefineButton({
	Pos = 2, Level = 3, Icon = "icon-gturret_b", Action = "build",
	Value = "unit-gturret", Key = "g", Hint = "BUILD ~!GUN TURRET",
	ForUnit = {"unit-engineer"}})
-- DefineButton for the 'stop' and 'attack' actions are defined in buttons.lua.


