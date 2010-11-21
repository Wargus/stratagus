--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-rtank.lua	-	Define the rocket tank unit.
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

DefineAnimations("animations-rtank", {
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
    Attack = {"unbreakable begin", "frame 0", "wait 4", 
        "frame 5", "sound bazoo-attack", "attack", "wait 1", 
        "frame 0", "wait 2", 
        "frame 5", "attack", "wait 1", 
        "frame 0", "wait 59", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 10", "wait 5", "frame 15", "wait 5", "frame 20", "unbreakable end", "wait 20", },
    })

DefineIcon({
	Name = "icon-rtank",
	Size = {46, 38},
	Frame = 0,
	File = "units/rtank/ico_rtank.png"})

MakeSound("rtank-ready", GetCurrentLuaPath().."/rtank.ready.wav")
MakeSound("rtank-help", GetCurrentLuaPath().."/rtank.underattack.wav")

DefineUnitType("unit-rtank", {
    Name = "Rocket Tank",
    Image = {"file", "units/rtank/unit_rtank.png", "size", {96, 96}},
    Shadow = {"file", "units/rtank/unit_rtank_s.png", "size", {96, 96}},
    Animations = "animations-rtank",
    Icon = "icon-rtank",
    EnergyValue = 6000,
    MagmaValue = 3000,
    RepairHp = 1,
    HitPoints = 50,
    DrawLevel = 25,
    TileSize = {1, 1},
    BoxSize = {64, 64},
    SightRange = 7,
    Armor = 20,
    BasicDamage = 5,
    PiercingDamage = 30,
    Missile = "missile-bazoo",
    Priority = 20,
    AnnoyComputerFactor = 65,
    Points = 15,
    ExplodeWhenKilled = "missile-64x64-explosion",
    Type = "land",
    ComputerReactionRange = 10,
    PersonReactionRange = 10,
    RightMouseAction = "attack",
    SelectableByRectangle = true,
    CanAttack = true,
    CanTargetLand = true,
    CanTargetAir = true,
    CanTargetSea = true,
    NumDirections = 8,
    MaxAttackRange = 6,
    Sounds = {
        "selected", "grenadier-selected",
        "acknowledge", "grenadier-acknowledge",
        "ready", "rtank-ready",
        "help", "rtank-help"
        }
})

DefineAllow("unit-rtank", AllowAll)

DefineButton({
    Pos = 5, Level = 0, Icon = "icon-rtank", Action = "train-unit",
    Value = "unit-rtank", Hint = "BUILD ~!ROCKET TANK",
    ForUnit = {"unit-vfac"}})

DefineCommonButtons({"unit-rtank"})



