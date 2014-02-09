--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-rocksfield.lua	-	Define the rocks-field.
--
--	(c) Copyright 1998 - 2007 by Lutz Sammer, Crestez Leonard, Francois Beerten
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
        Name = "icon-rocks_field",
        Size = {46, 38},
        Frame = 0,
        File = "units/rocksfield/ico_rocks_field.png"})

DefineConstruction("construction-rocksfield", {
        Constructions = {
                {Percent = 0, File = "main", Frame = 0},
        }
})

DefineUnitType("unit-rocksfield", {
    Name = "Rocks Field",
    Image = {"file", "units/rocksfield/rocks_field.png", "size", {128, 128}},
    Shadow = {"file", "units/rocksfield/rocks_field_s.png", "size", {128, 128}, "offset", {0, 0}},
    Animations = "animations-building",
    Icon = "icon-rocks_field",
    MagmaValue = 9000,
    VisibleUnderFog = true,
    Construction = "construction-rocksfield",
    NeutralMinimapColor = {196, 196, 196},
    DrawLevel = 40,
    TileSize = {4, 4},
    BoxSize = {96, 96},
    SightRange = 1,
    HitPoints = 25500,
    Priority = 0,
    Armor = 20,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Corpse = "unit-destroyed-4x4-place",
    ExplodeWhenKilled = "missile-explosion",
    Type = "land",
    Building = true,
    NumDirections = 1,
    CanHarvestFrom = true,
    Neutral = true
})

