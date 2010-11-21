--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-heli.lua	-	Define the heli unit.
--
--	(c) Copyright 2008 by Francois Beerten.
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

DefineAnimations("animations-heli", {
    Still = {"unbreakable begin", "frame 0", "wait 1", "frame 5", "wait 1",
            "frame 10", "wait 1", "unbreakable end", "wait 1"}, 
    Move = {"unbreakable begin", "frame 0", "move 2", "wait 1",
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1",
        "frame 0", "move 2", "wait 1", "frame 5", "move 2", "wait 1",
        "frame 10", "move 2", "wait 1", "frame 0", "move 2", "wait 1",
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1",
        "frame 0", "move 2", "wait 1", "frame 5", "move 2", "wait 1",
        "frame 10", "move 2", "wait 1", "frame 0", "move 2", "wait 1",
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1",
        "frame 0", "move 2", "wait 1", "frame 5", "unbreakable end", "wait 1",},
    Attack = {"unbreakable begin", "frame 25", "wait 1", 
        "frame 20", "sound assault-attack", "attack", "wait 2", 
        "frame 15", "wait 1",
        "frame 10", "wait 1", "frame 5", "wait 1", "frame 0", "wait 1",
        "frame 10", "wait 1", "frame 5", "wait 1", "frame 0", "wait 1",
        "frame 10", "wait 1", "frame 5", "wait 1", "frame 0", "wait 1",
        "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 30", "wait 4", "frame 35", 
        "wait 4", "frame 40", "wait 4",
        "frame 45", "wait 6", "unbreakable end", "wait 2", },
    })

DefineIcon({
	Name = "icon-heli",
	Size = {46, 38},
	Frame = 0,
	File = "units/heli/ico_heli.png"})

DefineUnitType("unit-heli", {
    Name = "Heli",
    Image = {"file", "units/heli/unit_heli.png", "size", {128,128}},
    Shadow = {"file", "units/heli/unit_heli_s.png", "size", {128, 128}, "offset", {5,128}},
    Animations = "animations-heli",
    Icon = "icon-heli",
    EnergyValue = 5500,
    MagmaValue = 2000,
    HitPoints = 40,
    DrawLevel = 125,
    TileSize = {1, 1},
    BoxSize = {64, 64},
    SightRange = 7,
    Armor = 10,
    BasicDamage = 5,
    PiercingDamage = 2,
    Missile = "missile-none",
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
    CanTargetAir = true,
    NumDirections = 8,
    MaxAttackRange = 7,
})

DefineAllow("unit-heli", AllowAll)

DefineButton({
    Pos = 4, Level = 0, Icon = "icon-heli", Action = "train-unit",
    Value = "unit-heli", Hint = "BUILD ~!HELI",
    ForUnit = {"unit-aircraftfactory"}})

DefineCommonButtons({"unit-heli"})



