--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-buggy.lua	-	Define the buggy unit.
--
--	(c) Copyright 2004-2005 by gorm.
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

DefineAnimations("animations-dead_buggy", {
    Death = {"unbreakable begin", "frame 30", "wait 50", 
        "frame 35", "sound grenade-impact", "wait 1", "frame 40", "wait 1", 
        "frame 45", "wait 1", "frame 50", "wait 1", "frame 55", "wait 1", 
        "frame 60", "wait 1","frame 65", "wait 2", 
        "unbreakable end", "wait 1", },
    })
 
DefineAnimations("animations-buggy", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin",        "frame  0", "move 2", "wait 1",
        "frame  0", "move 2", "wait 1", "frame  0", "move 2", "wait 1", 
        "frame  0", "move 2", "wait 1", "frame  5", "move 2", "wait 1", 
        "frame  5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame  0", "move 2", "wait 1", 
        "frame  0", "move 2", "wait 1", "frame  0", "move 2", "wait 1", 
        "frame  0", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", "frame 0", "wait 10", 
        "frame 25", "sound assault-attack", "attack", "wait 1", 
        "frame  0", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 30", "unbreakable end", "wait 1", },
    })

DefineIcon({
	Name = "icon-buggy",
	Size = {46, 38},
	Frame = 0,
	File = "units/buggy/ico_buggy.png"})

MakeSound("buggy-selected", "units/buggy/buggy_select.wav")
MakeSound("buggy-acknowledge", "units/buggy/buggy_action.wav")
MakeSound("buggy-ready", "units/buggy/buggy_ready.wav")
MakeSound("buggy-help", "units/buggy/buggy_attacked.wav")
MakeSound("buggy-die", "units/buggy/buggy_die.wav")

DefineUnitType("buggy_destroyed", {
    Name = "DestroyedBuggy",
    Image = {"file", "units/buggy/unit_buggy.png", "size", {64, 64}},
    Shadow = {"file", "units/buggy/unit_buggy_s.png", "size", {64, 64}},
    Animations = "animations-dead_buggy",
    Icon = "icon-cancel",
    HitPoints = 999,
    DrawLevel = 10,
    TileSize = {1, 1},
    BoxSize = {62, 62},
    SightRange = 2,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 0,
    Type = "land",
    Vanishes = true,
    Sounds = {
        "dead", "grenade-impact",
    }
})

DefineUnitType("unit-buggy", {
    Name = "Buggy",
    Image = {"file", "units/buggy/unit_buggy.png", "size", {64, 64}},
    Shadow = {"file", "units/buggy/unit_buggy_s.png", "size", {64, 64}},
    Animations = "animations-buggy",
    Icon = "icon-buggy",
    EnergyValue = 2000,
    MagmaValue = 500,
    RepairHp = 1,
    HitPoints = 40,
    DrawLevel = 25,
    TileSize = {1, 1},
    BoxSize = {62, 62},
    SightRange = 7,
    ComputerReactionRange = 7,
    PersonReactionRange = 7,
    Armor = 3,
    BasicDamage = 5,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 20,
    AnnoyComputerFactor = 45,
    Points = 100,
    ExplodeWhenKilled = "missile-64x64-explosion",
    Corpse = "buggy_destroyed",
    Type = "land",
    MaxAttackRange = 6,
    CanAttack = true,
    CanTargetLand = true,
    RightMouseAction = "attack",
    SelectableByRectangle = true,
    VisibleUnderFog = true,
    Sounds = {
        "selected",    "buggy-selected",
        "acknowledge", "buggy-acknowledge",
        "ready",       "buggy-ready",
        "help",        "buggy-help",
        "dead",        "buggy-die"}
})

DefineAllow("unit-buggy", AllowAll)

DefineButton({
    Pos = 3, Level = 0, Icon = "icon-buggy", Action = "train-unit",
    Value = "unit-buggy", Hint = "BUILD ~!BUGGY",
    ForUnit = {"unit-vfac"}})

