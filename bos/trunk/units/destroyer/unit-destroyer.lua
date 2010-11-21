--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-destroyer.lua	-	Define the destroyer unit.
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

DefineAnimations("animations-destroyer", {
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
    Attack = {"unbreakable begin", "frame 0", "wait 2", 
        "sound bazoo-attack", "attack", "wait 4",
        "sound bazoo-attack", "attack", "wait 4", 
        "wait 70", 
        "unbreakable end", "wait 1", },
    })

DefineIcon({
    Name = "icon-destroyer",
    Size = {46, 38},
    Frame = 0,
    File = "units/destroyer/destroyer_i.png"})

DefineUnitType("unit-destroyer", {
    Name = "Destroyer",
    Image = {"file", "units/destroyer/destroyer.png", "size", {128, 96}},
    Animations = "animations-destroyer",
    Icon = "icon-destroyer",
    EnergyValue = 600,
    MagmaValue = 300,
    RepairHp = 1,
    HitPoints = 40,
    DrawLevel = 25,
    TileSize = {2, 2},
    BoxSize = {96, 96},
    SightRange = 8,
    Armor = 10,
    BasicDamage = 5,
    PiercingDamage = 30,
    Priority = 10,
    AnnoyComputerFactor = 65,
    Points = 15,
    ExplodeWhenKilled = "missile-64x64-explosion",
    Type = "naval",
    AllowTerrainShallowWater = false,
    ComputerReactionRange = 10,
    PersonReactionRange = 10,
    RightMouseAction = "attack",
    SelectableByRectangle = true,
    CanAttack = true,
    CanTargetLand = true,
    CanTargetAir = true,
    CanTargetSea = true,
    NumDirections = 8,
    Missile = "missile-bazoo",
    MaxAttackRange = 6,
})

DefineAllow("unit-destroyer", AllowAll)

DefineButton({
   Pos = 3, Level = 0, Icon = "icon-destroyer", Action = "train-unit",
   Value = "unit-destroyer", Hint = "BUILD ~!Destroyer",
   ForUnit = {"unit-shipyard"}})

DefineCommonButtons({"unit-destroyer"})



