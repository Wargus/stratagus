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
--	unit-dorcoz.lua	- Define the dorcoz unit.
--
--	(c) Copyright 2003 - 2005 by François Beerten
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

DefineAnimations("animations-dorcoz", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 4", "wait 2",
        "frame 10", "move 4", "wait 2", "frame 15", "move 4", "wait 2",
        "frame 20", "move 4", "wait 2", "frame 5", "move 4", "wait 2",
        "frame 10", "move 4", "wait 2", "frame 15", "move 4", "wait 2",
        "frame 20", "move 4", "unbreakable end", "wait 2", },
    Attack = {"unbreakable begin",
        "frame 25", "sound dorcoz-attack", "attack", "wait 1",
        "frame 0", "unbreakable end", "wait 49", },
    Death = {"unbreakable begin", "frame 30", "wait 5", "frame 35", "wait 5",
        "frame 40", "wait 5", "frame 45", "unbreakable end", "wait 5", },
    })

DefineIcon({
        Name = "icon-dorcoz",
        Size = {46, 38},
        Frame = 0,
        File = "units/dorcoz/ico_dorcoz.png"})

DefineMissileType("missile-dorcoz", {
        File = "units/dorcoz/mis_plasma_sml.png",
        Size = {32, 32}, Frames = 5, NumDirections = 5,
        ImpactSound = "fireball hit", DrawLevel = 50,
        Class = "missile-class-point-to-point", Sleep = 1, Speed = 32, Range = 1})

DefineUnitType("unit-dorcoz", {
        Name = "Dorcoz",
        Image = {"file", "units/dorcoz/unit_dorcoz.png", "size", {64, 64}},
        Animations = "animations-dorcoz", Icon = "icon-dorcoz",
        Costs = {"time", 100, "titanium", 50, "crystal", 300}, Speed = 10,
        HitPoints = 100, DrawLevel = 40, TileSize = {1, 1}, BoxSize = {31, 31},
        SightRange = 7, ComputerReactionRange = 7, PersonReactionRange = 7,
        Armor = 2, BasicDamage = 6, PiercingDamage = 3, Missile = "missile-dorcoz",
        MaxAttackRange = 7, Priority = 60, Points = 50, Demand = 1,
        Corpse = {"unit-dead-body", 0}, Type = "land", RightMouseAction = "attack",
        CanAttack = true, CanTargetLand = true, LandUnit = true, organic = true,
        SelectableByRectangle = true, DetectCloak = true,
        RightMouseAction = "attack",
        Sounds = {
                "selected", "dorcoz-selected",
                "acknowledge", "dorcoz-acknowledge",
                "ready", "dorcoz-ready",
                "help", "basic terras voices help 1",
                "dead", "basic terras voices dead"}
        })

DefineAllow("unit-dorcoz", "AAAAAAAAAAAAAAAA")

DefineButton({
        Pos = 4, Level = 0, Icon = "icon-dorcoz",
        Action = "train-unit", Value = "unit-dorcoz",
        Key = "d", Hint = "TRAIN ~!DORCOZ",
        ForUnit = {"unit-camp"}})

