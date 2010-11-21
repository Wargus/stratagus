--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	shipyard.lua	-	Define the ship yard
--
--	(c) Copyright 2001 - 2010 by Francois Beerten
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
	Name = "icon-shipyard",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/shipyard_i.png"})

DefineConstruction("construction-shipyard", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 1},
		{Percent = 10, File = "main", Frame = 2},
		{Percent = 20, File = "main", Frame = 3},
		{Percent = 30, File = "main", Frame = 4},
		{Percent = 40, File = "main", Frame = 5},
		{Percent = 50, File = "main", Frame = 6},
		{Percent = 60, File = "main", Frame = 7},
		{Percent = 70, File = "main", Frame = 8},
		{Percent = 80, File = "main", Frame = 9},
		{Percent = 90, File = "main", Frame = 10},
		{Percent = 95, File = "main", Frame = 11},
	}
    })

DefineAnimations("animations-shipyard", {
    Still = {"frame 12", "wait 5", "frame 13", "wait 5", "frame 14", "wait 5", "frame 15", 
        "wait 5", "frame 16", "wait 5", "frame 17", "wait 5" },
    Train = {"frame 18", "wait 5", "frame 19", "wait 5", "frame 20", "wait 5", "frame 21", 
        "wait 5", "frame 22", "wait 5", "frame 23", "wait 5"},
    })

DefineUnitType("unit-shipyard", {
    Name = "Shipyard",
    Image = {"file", GetCurrentLuaPath().."/shipyard.png", "size", {256, 256}},
    Shadow = {"file", GetCurrentLuaPath().."/shipyard_s.png", "size", {256, 256}},
    Animations = "animations-shipyard",
    Icon = "icon-shipyard",
    EnergyValue = 6000,
    MagmaValue = 6000,
    RepairHp = 2,
    Construction = "construction-shipyard",
    HitPoints = 550,
    DrawLevel = 25,
    TileSize = {6, 5},
    BoxSize = {192, 160},
    SightRange = 2,
    Armor = 30,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 35,
    AnnoyComputerFactor = 45,
    Points = 200,
    DeathExplosion = largeExplosion,
    Type = "naval",
    BuildingRules = {
        -- Both conditions below must be satisfied.
        {
            -- The shipyard must have at least one land or coast tile
            -- under it; it cannot be built in the open sea.  This
            -- restriction exists for the map editor.  In the game,
            -- engineers wouldn't be able to reach such a building
            -- place anyway.
            "terrain", {CountLand = true, CountCoast = true, Min = 1},

            -- The shipyard must not have more than 8 land tiles under
            -- it.  (The total size is 6*5 = 30 tiles.)  This limit
            -- must be at least 5 so that an engineer with sight range
            -- 5 walking along a coast can explore the whole building
            -- site beforehand without cameras.
         "terrain", {CountLand = true, Max = 8}}},
    AllowTerrainLand = true,
    AllowTerrainCoast = true,
    Building = true,
    VisibleUnderFog = true,
    MaxEnergyUtilizationRate = 60,
    MaxMagmaUtilizationRate = 30,
    CanHarvestFrom = true,
})

DefineAllow("unit-shipyard", AllowAll)

DefineButton({
    Pos = 6, Level = 2, Icon = "icon-shipyard", Action = "build",
    Value = "unit-shipyard", Hint = "BUILD ~!SHIPYARD",
    ForUnit = {"unit-engineer"}})
