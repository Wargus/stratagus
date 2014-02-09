--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-gturret.lua	-	Define the gun turret unit.
--
--	(c) Copyright 2004-2008 by François Beerten.
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

DefineAnimations("animations-gturret", {
    Still = {"frame 5", "wait 1", },
    Attack = {"unbreakable begin", "frame 10", "wait 1", "frame 15", "sound gturret-attack", "attack", "wait 1",
        "frame 20", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 10", "unbreakable end", "wait 50", },
    })

DefineIcon({
	Name = "icon-gturret",
	Size = {46, 38},
	Frame = 0,
	File = "units/gturret/gturret_i.png"})

DefineIcon({
	Name = "icon-gturret_b",
	Size = {46, 38},
	Frame = 0,
	File = "units/gturret/gturret_i.png"})

DefineConstruction("construction-gturret", {
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

DefineUnitType("unit-gturret", {
    Name = "Gun Turret",
    Image = {"file", "units/gturret/gturret.png", "size", {96, 96}},
    Shadow = {"file", "units/gturret/gturret_s.png", "size", {96, 96}},
    Animations = "animations-gturret",
    Icon = "icon-gturret",
    EnergyValue = 4000,
    MagmaValue = 500,
    RepairHp = 2,
    Construction = "construction-gturret",
    HitPoints = 160,
    DrawLevel = 25,
    TileSize = {2, 2},
    BoxSize = {60, 60},
    SightRange = 6,
    ComputerReactionRange = 6,
    PersonReactionRange = 6,
    Armor = 10,
    BasicDamage = 4,
    PiercingDamage = 0,
    MaxAttackRange = 6,
    Missile = "missile-none",
    Priority = 20,
    AnnoyComputerFactor = 45,
    Points = 100,
    ExplodeWhenKilled = "missile-160x128-explosion",
    RightMouseAction = "attack",
    CanAttack = true,
    CanTargetLand = true,
    CanTargetAir = true,
    CanTargetSea = true,
    NumDirections = 8,
    Corpse = "build-dead-gturret",
    Type = "land",
    Building = true,
    VisibleUnderFog = true,
    CanHarvestFrom = true,
    Sounds = {
        "selected", "gturret-selected",
        "ready", "gturret-ready",
        "help", "gturret-help"
        }
})

MakeSound("gturret-selected", GetCurrentLuaPath().."/gturret_select.wav")
MakeSound("gturret-attack", GetCurrentLuaPath().."/gturret_attack.wav")
MakeSound("gturret-ready", GetCurrentLuaPath().."/gturret.completed.wav")
MakeSound("gturret-help", GetCurrentLuaPath().."/gturret.underattack.wav")

DefineAnimations("animations-dead-gturret", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 1", "wait 200", "frame 2", "wait 200", "frame 2", "wait 1", 
        "unbreakable end", "wait 1", },
    })

DefineUnitType("build-dead-gturret", {
    Name = "GturretCrater",
    Image = {"file", GetCurrentLuaPath().."/gturret_c.png", "size", {96, 96}},
    Animations = "animations-dead-gturret",
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


DefineAllow("unit-gturret", AllowAll)

DefineButton({
    Pos = 1, Level = 3, Icon = "icon-gturret_b", Action = "build",
    Value = "unit-gturret", Hint = "BUILD ~!GUN TURRET",
    ForUnit = {"unit-engineer"}})
-- DefineButton for the 'stop' and 'attack' actions are defined in buttons.lua.


