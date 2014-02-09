--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-cannon.lua	-	Define the cannon unit.
--
--	(c) Copyright 2004-2010 by Francois Beerten.
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

DefineAnimations("animations-cannon", {
    Still = {"frame 5", "wait 1", },
    Attack = {"unbreakable begin", "frame 5", "wait 5", 
              "frame 5", "sound bazoo-attack", "attack", "wait 1",
              "frame 10", "wait 15",
              "frame 5", "wait 170", "frame 5", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 10", "unbreakable end", "wait 50", },
    })

DefineIcon({
	Name = "icon-cannon",
	Size = {46, 38},
	Frame = 0,
	File = "units/cannon/cannon_i.png"})

DefineIcon({
	Name = "icon-cannon_b",
	Size = {46, 38},
	Frame = 0,
	File = "units/cannon/cannon_i.png"})

DefineConstruction("construction-cannon", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 0},
		{Percent = 10, File = "main", Frame = 0},
		{Percent = 20, File = "main", Frame = 1},
		{Percent = 30, File = "main", Frame = 2},
		{Percent = 40, File = "main", Frame = 3},
		{Percent = 50, File = "main", Frame = 4},
		{Percent = 60, File = "main", Frame = 5},
		{Percent = 70, File = "main", Frame = 6},
		{Percent = 80, File = "main", Frame = 7},
		{Percent = 90, File = "main", Frame = 7}
	}
})

DefineMissileType("missile-cannon", {
	File = "units/cannon/weapon.png",
	Size = {128, 128}, Frames = 6, NumDirections = 1,
	ImpactSound = "rocket-impact", DrawLevel = 150,
	Class = "missile-class-point-to-point", Sleep = 1, 
	Speed = 20, Range = 3})

MakeSound("cannon-ready", GetCurrentLuaPath().."/cannon.completed.wav")
MakeSound("cannon-help", GetCurrentLuaPath().."/cannon.underattack.wav")
MakeSound("cannon-selected", GetCurrentLuaPath().."/cannon_selected.wav")

DefineUnitType("unit-cannon", {
    Name = "Cannon",
    Image = {"file", "units/cannon/cannon.png", "size", {128, 128},},
    Offset = {0, -20},
    Shadow = {"file", "units/cannon/cannon_s.png", "size", {128, 128}},
    Animations = "animations-cannon",
    Icon = "icon-cannon",
    EnergyValue = 17000,
    MagmaValue = 8500,
    RepairHp = 2,
    Construction = "construction-cannon",
    HitPoints = 160,
    DrawLevel = 25,
    TileSize = {2, 2},
    BoxSize = {64, 58},
    SightRange = 6,
    ComputerReactionRange = 16,
    PersonReactionRange = 6,
    Armor = 10,
    BasicDamage = 40,
    PiercingDamage = 16,
    MaxAttackRange = 27,
    Missile = "missile-cannon",
    Priority = 20,
    AnnoyComputerFactor = 45,
    Points = 100,
    ExplodeWhenKilled = "missile-160x128-explosion",
    RightMouseAction = "attack",
    CanAttack = true,
    CanTargetLand = true,
    CanTargetAir = false,
    CanTargetSea = true,
    NumDirections = 8,
    Corpse = "build-dead-cannon",
    Type = "land",
    Building = true,
    VisibleUnderFog = true,
    CanHarvestFrom = true,
    Sounds = {
        "selected", "cannon-selected",
        "help", "cannon-help",
        "ready", "cannon-ready"
        }
})

DefineAnimations("animations-dead-cannon", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 1", "wait 2000", "frame 2", "wait 2000", "frame 2", "wait 1", 
        "unbreakable end", "wait 1", },
    })

DefineUnitType("build-dead-cannon", {
    Name = "CannonCrater",
    Image = {"file", GetCurrentLuaPath().."/cannon_c.png", "size", {128, 128}},
    Animations = "animations-dead-cannon",
    Icon = "icon-cancel",
    HitPoints = 999,
    DrawLevel = 10,
    TileSize = {2, 2},
    BoxSize = {220, 156},
    SightRange = 1,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 0,
    Type = "land",
    Building = true,
    Vanishes = true
})


DefineAllow("unit-cannon", AllowAll)

DefineButton({
    Pos = 3, Level = 3, Icon = "icon-cannon_b", Action = "build",
    Value = "unit-cannon", Hint = "BUILD ~!CANNON",
    ForUnit = {"unit-engineer"}})
-- DefineButton for the 'stop' and 'attack' actions are defined in buttons.lua.


