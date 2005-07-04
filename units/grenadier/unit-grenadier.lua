--       _________ __                 __
--      /   _____//  |_1____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--			  T H E   W A R   B E G I N S
--	   Stratagus - A free fantasy real time strategy game engine
--
--	unit-grenadier.lua	-	Define the grenadier
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
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id$

DefineIcon({
	Name = "icon-grenadier",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/ico_grenadier.png"})

DefineAnimations("animations-grenadier", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 2", "wait 1", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "wait 1", "frame 5", "move 2", "wait 1", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "wait 1", "frame 20", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", "frame 25", "wait 3", "frame 30", "wait 3", 
        "frame 35", "sound grenadier-attack", "attack", "wait 3", 
        "frame 40", "wait 3", "frame 0", "wait 150", 
        "frame 0", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 30", "wait 5", "frame 35", "wait 5", 
        "frame 40", "wait 5", "frame 45", "unbreakable end", "wait 5", },
    })
DefineUnitType("unit-grenadier", {
	Name = "Grenadier",
	Image = {"file", GetCurrentLuaPath().."/unit_grenadier.png", "size", {64, 64}},
	Shadow  = {"file", GetCurrentLuaPath().."/unit_grenadier_s.png", "size", {64, 64}},
	Animations = "animations-grenadier", Icon = "icon-grenadier",
	Costs = {"time", 40, "titanium", 25, "crystal", 100},
	Speed = 10, HitPoints = 50, DrawLevel = 25,
	TileSize = {1, 1}, BoxSize = {17, 28}, SightRange = 6,
	ComputerReactionRange = 6, PersonReactionRange = 6,
	RightMouseAction = "attack",
	Armor = 2, BasicDamage = 15, PiercingDamage = 15, Missile = "missile-grenadier",
	MaxAttackRange = 5, Priority = 60, Points = 50, Corpse = {"unit-dead-body3", 0},
	Type = "land", CanAttack = true, CanTargetLand = true,
	LandUnit = true, organic = true, Demand = 0, SelectableByRectangle = true,
	Sounds = {
		"selected", "grenadier-selected",
		"acknowledge", "grenadier-acknowledge",
		"ready", "grenadier-ready",
		"help", "grenadier-help",
		"dead", "grenadier-die"}
	})
DefineHumanCorpse("grenadier")

DefineAllow("unit-grenadier", "AAAAAAAAAAAAAAAA")
