--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-bomber.lua	-	Define the bomber unit.
--
--	(c) Copyright 2005-2007 by Francois Beerten.
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

DefineAnimations("animations-bomber", {
    Still = {"frame 0", "wait 1", },
    Attack = {"unbreakable begin", 
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

MakeSound("bomber-ready", GetCurrentLuaPath().."/bomber.ready.wav")
MakeSound("bomber-help", GetCurrentLuaPath().."/bomber.underattack.wav")

DefineUnitType("unit-bomber", {
    Name = "Bomber",
    Image = {"file", "units/bomber/unit_bomber.png", "size", {128, 128}},
    Shadow = {"file", "units/bomber/unit_bomber_s.png", "size", {128, 128}, "offset", {5, 128}},
    Animations = "animations-bomber",
    Icon = "icon-bomber",
    EnergyValue = 10000,
    MagmaValue = 7000,
    HitPoints = 50,
    DrawLevel = 125,
    TileSize = {2, 2},
    BoxSize = {64, 64},
    SightRange = 7,
    Armor = 20,
    BasicDamage = 50,
    PiercingDamage = 30,
    Missile = "missile-bomber",
    Priority = 20,
    AnnoyComputerFactor = 65,
    Points = 15,
    ExplodeWhenKilled = "missile-64x64-explosion",
    Type = "fly",
    ComputerReactionRange = 10,
    PersonReactionRange = 10,
    RightMouseAction = "attack",
    SelectableByRectangle = true,
    CanAttack = true,
    CanTargetLand = true,
    CanTargetSea = true,
    NumDirections = 8,
    MaxAttackRange = 1,
    Sounds = {
        "selected", "grenadier-selected",
        "acknowledge", "grenadier-acknowledge",
        "ready", "bomber-ready",
        "help", "bomber-help"
        }
})

DefineAllow("unit-bomber", AllowAll)

DefineButton({
    Pos = 2, Level = 0, Icon = "icon-bomber", Action = "train-unit",
    Value = "unit-bomber", Hint = "BUILD ~!BOMBER",
    ForUnit = {"unit-aircraftfactory"}})

DefineCommonButtons({"unit-bomber"})



