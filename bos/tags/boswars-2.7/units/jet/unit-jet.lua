--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-jet.lua	-	Define the jet unit.
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

DefineAnimations("animations-jet", {
    Still = {"frame 0", "wait 1", },
    Attack = {"unbreakable begin", 
        "frame 0", "wait 1", "attack", "sound assault-attack",
        "frame 0", "wait 1", "frame 0","wait 15",
	"unbreakable end", "wait 1", },
    Move = {"unbreakable begin", 
        "frame 5", "move 5", "wait 1", "frame 5", "move 4", "wait 1",
        "frame 5", "move 5", "wait 1", "frame 5", "move 4", "wait 1",
        "frame 5", "move 4", "wait 1", "frame 5", "move 5", "wait 1",
        "frame 5", "move 5", "wait 1",
	"unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 0", "wait 15","unbreakable end", "wait 1",},
    })

DefineIcon({
	Name = "icon-jet",
	Size = {46, 38},
	Frame = 0,
	File = "units/jet/ico_jet.png"})

DefineMissileType("missile-jet", {
	File = "units/jet/missile.png",
	Size = {96, 96}, Frames = 5, NumDirections = 8,
	ImpactSound = "rocket-impact", DrawLevel = 150,
	Class = "missile-class-point-to-point", Sleep = 1, Speed = 16, Range = 16})

MakeSound("jet-ready", GetCurrentLuaPath().."/jet.ready.wav")
MakeSound("jet-help", GetCurrentLuaPath().."/jet.underattack.wav")

DefineUnitType("unit-jet", {
    Name = "Jet fighter",
    Image = {"file", "units/jet/unit_jet.png", "size", {128, 128}},
    Shadow = {"file", "units/jet/unit_jet_s.png", "size", {128, 128},
                  "offset", {5, 128}},
    Animations = "animations-jet",
    Icon = "icon-jet",
    EnergyValue = 8000,
    MagmaValue = 3000,
    HitPoints = 50,
    DrawLevel = 125,
    TileSize = {3, 3},
    BoxSize = {96, 96},
    SightRange = 7,
    Armor = 20,
    BasicDamage = 5,
    PiercingDamage = 30,
    Missile = "missile-jet",
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
    CanTargetSea = true,
    NumDirections = 8,
    MaxAttackRange = 5,
    Sounds = {
        "selected", "grenadier-selected",
        "acknowledge", "grenadier-acknowledge",
        "ready", "jet-ready",
        "help", "jet-help"
        }
})

DefineAllow("unit-jet", AllowAll)


DefineButton({
    Pos = 1, Level = 0, Icon = "icon-jet", Action = "train-unit",
    Value = "unit-jet", Hint = "BUILD ~!JET FIGHTER",
    ForUnit = {"unit-aircraftfactory"}})

DefineCommonButtons({"unit-jet"})



