--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-apcs.lua	-	Define the apcs
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
	Name = "icon-apcs",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/ico_apcs.png"})

DefineAnimations("animations-apcs", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 0", "move 4", "wait 2", 
        "frame 0", "move 4", "wait 2", "frame 0", "move 4", "wait 2", 
        "frame 0", "move 4", "wait 2", "frame 0", "move 4", "wait 2", 
        "frame 0", "move 4", "wait 2", "frame 0", "move 4", "wait 2", 
        "frame 0", "move 4", "wait 2", "frame 0", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", "frame 5", "sound apcs-attack", "attack", "wait 2", 
        "frame 0", "wait 2", "frame 5", "wait 2", "frame 0", "wait 2", 
        "frame 5", "attack", "wait 2", "frame 0", "wait 2", 
        "frame 0", "wait 2", "frame 0", "wait 2", "frame 0", "wait 2", 
        "frame 0", "wait 2", "frame 0", "wait 2", "frame 0", "wait 2", 
        "frame 0", "wait 2", "frame 0", "wait 2", "frame 0", "wait 2", 
        "frame 0", "wait 2", "frame 0", "wait 2", "frame 0", "wait 2", 
        "frame 0", "wait 2", "frame 0", "unbreakable end", "wait 2", },
    Death = {"unbreakable begin", "frame 0", "wait 5", "frame 0", "wait 5", 
        "frame 0", "wait 5", "frame 0", "unbreakable end", "wait 5", },
    })

MakeSound("apcs-selected", GetCurrentLuaPath().."/smolder_select.wav")
MakeSound("apcs-acknowledge", GetCurrentLuaPath().."/smolder_action.wav")
MakeSound("apcs-ready", GetCurrentLuaPath().."/smolder.completed.wav")
MakeSound("apcs-help", GetCurrentLuaPath().."/smolder.underattack.wav")
MakeSound("apcs-die", GetCurrentLuaPath().."/smolder_die.wav")
MakeSound("apcs-attack", GetCurrentLuaPath().."/smolder_attack.wav")

DefineUnitType("unit-apcs", {
    Name = "APC Smolder",
    Image = {"file", GetCurrentLuaPath().."/unit_apcs.png", "size", {96, 96}},
    Shadow = {"file", GetCurrentLuaPath().."/unit_apcs_s.png", "size", {96, 96}},
    Animations = "animations-apcs",
    Icon = "icon-apcs",
    Flip = true,
    EnergyValue = 4000,
    MagmaValue = 2000,
    HitPoints = 200,
    DrawLevel = 25,
    TileSize = {1, 1},
    BoxSize = {96, 96},
    SightRange = 5,
    ComputerReactionRange = 5,
    PersonReactionRange = 5,
    Armor = 10,
    BasicDamage = 5,
    PiercingDamage = 5,
    Missile = "missile-none",
    MaxAttackRange = 4,
    Priority = 60,
    Points = 50,
    CanTransport = true,
    AttackFromTransporter = true,
    MaxOnBoard = 6,
    Type = "land",
    RepairHp = 2,
    RightMouseAction = "attack",
    ExplodeWhenKilled = "missile-160x128-explosion",
    CanAttack = true,
    CanTargetLand = true,
    CanTargetAir = true,
    SelectableByRectangle = true,
    Sounds = {
        "selected", "apcs-selected",
        "acknowledge", "apcs-acknowledge",
        "ready", "apcs-ready",
        "help", "apcs-help",
        "dead", "apcs-die"}
})

DefineAllow("unit-apcs", AllowAll)
