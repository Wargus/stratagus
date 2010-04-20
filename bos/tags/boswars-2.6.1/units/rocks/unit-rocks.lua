--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-rocks.lua	-	Define the rocks units.
--
--	(c) Copyright 2005 - 2007 by Lo� Taulelle.
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
--	$Id: unit-rocks.lua 85 2005-04-23 23:03:50Z jim4 $

-- Define, Animation and Construction for all rocks

DefineAnimations("animations-rocks", {
	Still = {"frame 0", "wait 1", },
})

DefineConstruction("construction-rocks", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 0},
	}
})

-- rock-1

DefineUnitType("unit-rock-1", {
    Name = "rock-1",
    Image = {"file", "units/rocks/rock_1.png", "size", {96, 96}},
    Shadow = {"file", "units/rocks/rock_1s.png", "size", {96, 96}},
    Animations = "animations-rocks",
    Icon = "icon-rock-1",
    Construction = "construction-rocks",
    MagmaValue = 5000,
    HitPoints = 3000,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {48, 48},
    Armor = 999,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    NumDirections = 1,
    CanHarvestFrom = true,
    Neutral = true,
    Sounds = {}
})

DefineIcon({
    Name = "icon-rock-1",
    Size = {46, 38},
    Frame = 0,
    File = "units/rocks/rock_1i.png"})

DefineAllow("unit-rock-1", AllowAll)

-- rock-2

DefineUnitType("unit-rock-2", {
    Name = "rock-2",
    Image = {"file", "units/rocks/rock_2.png", "size", {96, 96}},
    Shadow = {"file", "units/rocks/rock_2s.png", "size", {96, 96}},
    Animations = "animations-rocks",
    Icon = "icon-rock-2",
    Construction = "construction-rocks",
    MagmaValue = 4000,
    HitPoints = 2500,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {48, 48},
    Armor = 999,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    NumDirections = 1,
    CanHarvestFrom = true,
    Neutral = true,
    Sounds = {}
})

DefineIcon({
    Name = "icon-rock-2",
    Size = {46, 38},
    Frame = 0,
    File = "units/rocks/rock_2i.png"})

DefineAllow("unit-rock-2", AllowAll)

-- rock-3

DefineUnitType("unit-rock-3", {
    Name = "rock-3",
    Image = {"file", "units/rocks/rock_3.png", "size", {96, 96}},
    Shadow = {"file", "units/rocks/rock_3s.png", "size", {96, 96}},
    Animations = "animations-rocks",
    Icon = "icon-rock-3",
    Construction = "construction-rocks",
    MagmaValue = 3000,
    HitPoints = 2500,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {48, 48},
    Armor = 999,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    NumDirections = 1,
    CanHarvestFrom = true,
    Neutral = true,
    Sounds = {}
})

DefineIcon({
    Name = "icon-rock-3",
    Size = {46, 38},
    Frame = 0,
    File = "units/rocks/rock_3i.png"})

DefineAllow("unit-rock-3", AllowAll)

-- rock-4

DefineUnitType("unit-rock-4", {
    Name = "rock-4",
    Image = {"file", "units/rocks/rock_4.png", "size", {96, 96}},
    Shadow = {"file", "units/rocks/rock_4s.png", "size", {96, 96}},
    Animations = "animations-rocks",
    Icon = "icon-rock-4",
    Construction = "construction-rocks",
    MagmaValue = 2000,
    HitPoints = 2500,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {48, 48},
    Armor = 999,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    NumDirections = 1,
    CanHarvestFrom = true,
    Neutral = true,
    Sounds = {}
})

DefineIcon({
    Name = "icon-rock-4",
    Size = {46, 38},
    Frame = 0,
    File = "units/rocks/rock_4i.png"})

DefineAllow("unit-rock-4", AllowAll)

-- rock-5

DefineUnitType("unit-rock-5", {
    Name = "rock-5",
    Image = {"file", "units/rocks/rock_5.png", "size", {96, 96}},
    Shadow = {"file", "units/rocks/rock_5s.png", "size", {96, 96}},
    Animations = "animations-rocks",
    Icon = "icon-rock-5",
    Construction = "construction-rocks",
    MagmaValue = 1000,
    HitPoints = 1500,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {32, 32},
    Armor = 999,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    NumDirections = 1,
    CanHarvestFrom = true,
    Neutral = true,
    Sounds = {}
})

DefineIcon({
    Name = "icon-rock-5",
    Size = {46, 38},
    Frame = 0,
    File = "units/rocks/rock_5i.png"})

DefineAllow("unit-rock-5", AllowAll)

-- rock-6

DefineUnitType("unit-rock-6", {
    Name = "rock-6",
    Image = {"file", "units/rocks/rock_6.png", "size", {96, 96}},
    Shadow = {"file", "units/rocks/rock_6s.png", "size", {96, 96}},
    Animations = "animations-rocks",
    Icon = "icon-rock-6",
    Construction = "construction-rocks",
    MagmaValue = 1200,
    HitPoints = 1500,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {32, 32},
    Armor = 999,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    NumDirections = 1,
    CanHarvestFrom = true,
    Neutral = true,
    Sounds = {}
})

DefineIcon({
    Name = "icon-rock-6",
    Size = {46, 38},
    Frame = 0,
    File = "units/rocks/rock_6i.png"})

DefineAllow("unit-rock-6", AllowAll)


