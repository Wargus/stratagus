--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-artil.lua	-	Define the artil unit.
--
--	(c) Copyright 2005-2010 by Francois Beerten.
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

MakeSound("artil-ready", GetCurrentLuaPath().."/artil.ready.wav")
MakeSound("artil-help", GetCurrentLuaPath().."/artil.underattack.wav")

DefineUnitType("unit-artil", {
    Name = "Artil",
    Image = {"file", "units/artil/unit_artil.png", "size", {160, 160}},
    Offset = {0, -10},
    Shadow = {"file", "units/artil/unit_artil_s.png", "size", {160, 160}},
    Animations = "animations-artil",
    Icon = "icon-artil",
    EnergyValue = 10000,
    MagmaValue = 10000,
    RepairHp = 1,
    HitPoints = 250,
    DrawLevel = 25,
    TileSize = {1, 1},
    BoxSize = {64, 64},
    SightRange = 5,
    Armor = 25,
    BasicDamage = 10,
    PiercingDamage = 50,
    Missile = "missile-bazoo",
    Priority = 20,
    AnnoyComputerFactor = 65,
    Points = 25,
    ExplodeWhenKilled = "missile-64x64-explosion",
    Type = "land",
    ComputerReactionRange = 10,
    PersonReactionRange = 10,
    RightMouseAction = "attack",
    SelectableByRectangle = true,
    CanAttack = true,
    CanTargetLand = true,
    CanTargetSea = true,
    NumDirections = 8,
    MaxAttackRange = 8,
    Sounds = {
        "ready", "artil-ready",
        "help", "artil-help"
        }
})

DefineAllow("unit-artil", AllowAll)

DefineCommonButtons({"unit-artil"})

DefineButton({
    Pos = 6, Level = 0, Icon = "icon-artil", Action = "train-unit",
    Value = "unit-artil", Hint = "BUILD ARTI~!L",
    ForUnit = {"unit-vfac"}})


