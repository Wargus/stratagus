--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-wscout.lua	-	Define the water scout unit.
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

DefineAnimations("animations-wscout", {
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
    })

DefineIcon({
	Name = "icon-wscout",
	Size = {46, 38},
	Frame = 0,
	File = "units/wscout/wscout_i.png"})

DefineUnitType("unit-wscout", {
    Name = "Water Scout",
    Image = {"file", "units/wscout/wscout.png", "size", {96, 64}},
    Animations = "animations-wscout",
    Icon = "icon-wscout",
    EnergyValue = 600,
    MagmaValue = 300,
    RepairHp = 1,
    HitPoints = 40,
    DrawLevel = 25,
    TileSize = {2, 2},
    BoxSize = {84, 64},
    SightRange = 8,
    Armor = 10,
    BasicDamage = 5,
    PiercingDamage = 30,
    Priority = 10,
    AnnoyComputerFactor = 65,
    Points = 15,
    ExplodeWhenKilled = "missile-64x64-explosion",
    Type = "naval",
    AllowTerrainShallowWater = true,
    ComputerReactionRange = 10,
    PersonReactionRange = 10,
    RightMouseAction = "move",
    SelectableByRectangle = true,
    CanAttack = false,
    CanTargetLand = true,
    CanTargetAir = true,
    NumDirections = 8,
    MaxAttackRange = 6,
})

DefineAllow("unit-wscout", AllowAll)

DefineButton({
   Pos = 2, Level = 0, Icon = "icon-wscout", Action = "train-unit",
   Value = "unit-wscout", Hint = "BUILD ~!WATER SCOUT",
   ForUnit = {"unit-shipyard"}})

DefineCommonButtons({"unit-wscout"})



