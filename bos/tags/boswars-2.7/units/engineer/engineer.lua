--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	engineer.lua	-	Define the engineer unit.
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
	Name = "icon-engineer",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/ico_engineer.png"})

DefineAnimations("animations-engineer", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 2", "wait 2", 
        "frame 5", "move 2", "wait 2", "frame 5", "move 2", "wait 2", 
        "frame 5", "move 2", "wait 2", "frame 10", "move 2", "wait 2", 
        "frame 10", "move 2", "wait 2", "frame 10", "move 2", "wait 2", 
        "frame 10", "move 2", "wait 2", "frame 15", "move 2", "wait 2", 
        "frame 15", "move 2", "wait 2", "frame 15", "move 2", "wait 2", 
        "frame 15", "move 2", "wait 2", "frame 20", "move 2", "wait 2", 
        "frame 20", "move 2", "wait 2", "frame 20", "move 2", "wait 2", 
        "frame 20", "move 2", "unbreakable end", "frame 0", "wait 2", },
    Repair = {"unbreakable begin", "frame 25", "wait 8", "frame 30", "wait 2",
        "frame 35", "wait 2", "frame 40", "sound engineer-repair", "wait 8",
        "frame 35", "wait 3", "frame 30", "wait 2", "unbreakable end", "wait 1", },
    Harvest = {"frame 25", "wait 8", "frame 30", "wait 2", 
        "frame 35", "wait 2", "frame 40", "sound engineer-harvest", "wait 8", 
        "frame 35", "wait 3", "frame 30", "wait 3", },
    Death = {"unbreakable begin", "frame 45", "wait 5", "frame 50", "wait 5", 
        "frame 55", "wait 5", "frame 50", "unbreakable end", "wait 5", },
    })

MakeSound("engineer-selected", GetCurrentLuaPath().."/engineer_select.wav")
MakeSound("engineer-acknowledge", GetCurrentLuaPath().."/engineer_action.wav")
MakeSound("engineer-ready", GetCurrentLuaPath().."/engineer.ready.wav")
MakeSound("engineer-help", GetCurrentLuaPath().."/engineer.underattack.wav")
MakeSound("engineer-die", GetCurrentLuaPath().."/engineer_die.wav")
MakeSound("engineer-repair", GetCurrentLuaPath().."/engineer_attack.wav")
MakeSound("engineer-harvest", GetCurrentLuaPath().."/engineer_attack.wav")

DefineUnitType("unit-engineer", {
    Name = "Engineer",
    Image = {"file", GetCurrentLuaPath().."/unit_engineer.png", "size", {56, 56}},
    Shadow = {"file", GetCurrentLuaPath().."/unit_engineer_s.png", "size", {56, 56}},
    Animations = "animations-engineer",
    Icon = "icon-engineer",
    Flip = true,
    EnergyValue = 1600,
    MagmaValue = 800,
    MaxEnergyUtilizationRate = 40,
    MaxMagmaUtilizationRate = 20,
    HitPoints = 30,
    DrawLevel = 25,
    TileSize = {1, 1},
    BoxSize = {17, 28},
    SightRange = 5,
    ComputerReactionRange = 6,
    PersonReactionRange = 4,
    Armor = 1,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    MaxAttackRange = 1,
    Priority = 50,
    Points = 30,
    Corpse = "unit-dead-body2",
    Type = "land",
    RightMouseAction = "harvest",
    RepairRange = 1,
    AutoRepairRange = 5,
    CanTargetLand = true,
    Coward = true,
    Harvester = true,
    Organic = true,
    SelectableByRectangle = true,
    Sounds = {
        "selected", "engineer-selected",
        "acknowledge", "engineer-acknowledge",
        "ready", "engineer-ready",
        "help", "engineer-help",
        "dead", "engineer-die"}
})
DefineHumanCorpse("engineer", {56, 56}, true)

DefineAllow("unit-engineer", AllowAll)
