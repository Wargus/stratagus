--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-grenadier.lua	-	Define the grenadier
--
--	(c) Copyright 2001 - 2007 by Francois Beerten, Lutz Sammer and Crestez Leonard
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

DefineIcon({
	Name = "icon-grenadier",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/ico_grenadier.png"})

DefineAnimations("animations-grenadier", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 2", "wait 1", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "wait 1", "frame 5", "move 2", "wait 1", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "wait 1", "frame 20", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", "frame 25", "wait 3", "frame 30", "wait 3", 
        "frame 35", "sound grenadier-attack", "attack", "wait 3", 
        "frame 40", "wait 3", "frame 0", "wait 150", 
        "frame 0", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 30", "wait 5", "frame 35", "wait 5", 
        "frame 40", "wait 5", "frame 45", "unbreakable end", "wait 5", },
    })

MakeSound("grenadier-selected", GetCurrentLuaPath().."/grenadier_select.wav")
MakeSound("grenadier-acknowledge", GetCurrentLuaPath().."/grenadier_action.wav")
MakeSound("grenadier-ready", GetCurrentLuaPath().."/grenadier.ready.wav")
MakeSound("grenadier-help", GetCurrentLuaPath().."/grenadier.underattack.wav")
MakeSound("grenadier-die", GetCurrentLuaPath().."/grenadier_die.wav")
MakeSound("grenadier-attack", GetCurrentLuaPath().."/grenadier_attack.wav")

DefineUnitType("unit-grenadier", {
    Name = "Grenadier",
    Image = {"file", GetCurrentLuaPath().."/unit_grenadier.png", "size", {64, 64}},
    Shadow = {"file", GetCurrentLuaPath().."/unit_grenadier_s.png", "size", {64, 64}},
    Animations = "animations-grenadier",
    Icon = "icon-grenadier",
    Flip = true,
    EnergyValue = 1600,
    MagmaValue = 1350,
    HitPoints = 50,
    DrawLevel = 25,
    TileSize = {1, 1},
    BoxSize = {17, 28},
    SightRange = 6,
    ComputerReactionRange = 6,
    PersonReactionRange = 6,
    RightMouseAction = "attack",
    Armor = 2,
    BasicDamage = 15,
    PiercingDamage = 15,
    Missile = "missile-grenadier",
    MaxAttackRange = 5,
    Priority = 60,
    Points = 50,
    Corpse = "unit-dead-body3",
    Type = "land",
    CanAttack = true,
    CanTargetLand = true,
    Organic = true,
    SelectableByRectangle = true,
    Sounds = {
        "selected", "grenadier-selected",
        "acknowledge", "grenadier-acknowledge",
        "ready", "grenadier-ready",
        "help", "grenadier-help",
        "dead", "grenadier-die"}
})
DefineHumanCorpse("grenadier", nil, true)

DefineAllow("unit-grenadier", AllowAll)
