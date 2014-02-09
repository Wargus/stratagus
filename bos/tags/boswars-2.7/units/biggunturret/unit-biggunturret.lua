--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-biggunturret.lua	-	Define the big gun turret unit.
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

DefineAnimations("animations-biggunturret", {
    Still = {"frame 5", "wait 1", },
    Attack = {"unbreakable begin", "frame 10", "wait 1", "frame 15", "sound gturret-attack", "attack", "wait 1",
        "frame 20", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 10", "unbreakable end", "wait 50", },
    })

DefineIcon({
	Name = "icon-biggunturret",
	Size = {46, 38},
	Frame = 0,
	File = "units/biggunturret/gturret_i.png"})

DefineIcon({
	Name = "icon-biggunturret_b",
	Size = {46, 38},
	Frame = 0,
	File = "units/biggunturret/gturret_i.png"})

DefineConstruction("construction-biggunturret", {
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

DefineUnitType("unit-biggunturret", {
    Name = "Big gun Turret",
    Image = {"file", "units/biggunturret/gturret.png", "size", {96, 96}},
    Shadow = {"file", "units/biggunturret/gturret_s.png", "size", {96, 96}},
    Animations = "animations-biggunturret",
    Icon = "icon-biggunturret",
    EnergyValue = 8000,
    MagmaValue = 2000,
    RepairHp = 20,
    Construction = "construction-biggunturret",
    HitPoints = 260,
    DrawLevel = 25,
    TileSize = {2, 2},
    BoxSize = {60, 60},
    SightRange = 6,
    ComputerReactionRange = 6,
    PersonReactionRange = 6,
    Armor = 10,
    BasicDamage = 8,
    PiercingDamage = 1,
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
    Corpse = "build-dead-biggunturret",
    Type = "land",
    Building = true,
    VisibleUnderFog = true,
    CanHarvestFrom = true,
})

DefineAnimations("animations-dead-biggunturret", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 1", "wait 200", "frame 2", "wait 200", "frame 2", "wait 1", 
        "unbreakable end", "wait 1", },
    })

DefineUnitType("build-dead-biggunturret", {
    Name = "GturretCrater",
    Image = {"file", "units/gturret/gturret_c.png", "size", {96, 96}},
    Animations = "animations-dead-biggunturret",
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


DefineAllow("unit-biggunturret", AllowAll)

DefineButton({
    Pos = 2, Level = 3, Icon = "icon-biggunturret_b", Action = "build",
    Value = "unit-biggunturret", Hint = "BUILD ~!BIG GUN TURRET",
    ForUnit = {"unit-engineer"}})

-- DefineButton for the 'stop' and 'attack' actions like those
-- defined in buttons.lua.
DefineButton({
    Pos = 2, Level = 0, Icon = "icon-stop",
    Action = "stop", Hint = "~!STOP",
    ForUnit = {"unit-biggunturret"}})


DefineButton({
    Pos = 3, Level = 0, Icon = "icon-attack",
    Action = "attack", Hint = "~!ATTACK",
    ForUnit = {"unit-biggunturret"}})

