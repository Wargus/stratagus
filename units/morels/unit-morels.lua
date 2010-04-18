--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-morels.lua	-	Define the morels units.
--
--	(c) Copyright 2005-2007 by Lo�s Taulelle, Graphics by TimberDragon.
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
--	$Id: unit-morels.lua 85 2005-04-23 23:03:50Z jim4 $

-- Define, Animation and Construction for all morels

DefineAnimations("animations-morels", {
	Still = {"frame 0", "wait 1", },
})

DefineConstruction("construction-morels", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 0},
	}
})

-- morel-1

DefineUnitType("unit-morel-1", {
    Name = "morel-1",
    Image = {"file", "units/morels/morel_1.png", "size", {64, 128}},
    Shadow = {"file", "units/morels/morel_1s.png", "size", {64, 128}},
    Offset = {0, -20},
    Animations = "animations-morels",
    Icon = "icon-morel-1",
    Construction = "construction-morels",
    HitPoints = 50,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {48, 48},
    Armor = 10,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    EnergyValue = 5000,
    CanHarvestFrom = true,
    Neutral = true,
    NumDirections = 1,
    Sounds = {}
})

DefineIcon({
    Name = "icon-morel-1",
    Size = {46, 38},
    Frame = 0,
    File = "units/morels/morel_1i.png"})

DefineAllow("unit-morel-1", AllowAll)

-- morel-2

DefineUnitType("unit-morel-2", {
    Name = "morel-2",
    Image = {"file", "units/morels/morel_2.png", "size", {64, 128}},
    Shadow = {"file", "units/morels/morel_2s.png", "size", {64, 128}},
    Offset = {0, -22},
    Animations = "animations-morels",
    Icon = "icon-morel-2",
    Construction = "construction-morels",
    HitPoints = 50,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {48, 48},
    Armor = 10,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    EnergyValue = 5000,
    CanHarvestFrom = true,
    Neutral = true,
    NumDirections = 1,
    Sounds = {}
})

DefineIcon({
    Name = "icon-morel-2",
    Size = {46, 38},
    Frame = 0,
    File = "units/morels/morel_2i.png"})

DefineAllow("unit-morel-2", AllowAll)

-- morel-3

DefineUnitType("unit-morel-3", {
    Name = "morel-3",
    Image = {"file", "units/morels/morel_3.png", "size", {64, 128}},
    Shadow = {"file", "units/morels/morel_3s.png", "size", {64, 128}},
    Offset = {0, -22},
    Animations = "animations-morels",
    Icon = "icon-morel-3",
    Construction = "construction-morels",
    HitPoints = 50,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {48, 48},
    Armor = 10,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    EnergyValue = 5000,
    CanHarvestFrom = true,
    Neutral = true,
    NumDirections = 1,
    Sounds = {}
})

DefineIcon({
    Name = "icon-morel-3",
    Size = {46, 38},
    Frame = 0,
    File = "units/morels/morel_3i.png"})

DefineAllow("unit-morel-3", AllowAll)

-- morel-4

DefineUnitType("unit-morel-4", {
    Name = "morel-4",
    Image = {"file", "units/morels/morel_4.png", "size", {64, 128}},
    Shadow = {"file", "units/morels/morel_4s.png", "size", {64, 128}},
    Offset = {0, -19},
    Animations = "animations-morels",
    Icon = "icon-morel-4",
    Construction = "construction-morels",
    HitPoints = 50,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {48, 48},
    Armor = 10,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    EnergyValue = 5000,
    CanHarvestFrom = true,
    Neutral = true,
    NumDirections = 1,
    Sounds = {}
})

DefineIcon({
    Name = "icon-morel-4",
    Size = {46, 38},
    Frame = 0,
    File = "units/morels/morel_4i.png"})

DefineAllow("unit-morel-4", AllowAll)

-- morel-5

DefineUnitType("unit-morel-5", {
    Name = "morel-5",
    Image = {"file", "units/morels/morel_5.png", "size", {64, 128}},
    Shadow = {"file", "units/morels/morel_5s.png", "size", {64, 128}},
    Offset = {0, -16},
    Animations = "animations-morels",
    Icon = "icon-morel-5",
    Construction = "construction-morels",
    HitPoints = 50,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {32, 32},
    Armor = 10,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    EnergyValue = 5000,
    CanHarvestFrom = true,
    Neutral = true,
    NumDirections = 1,
    Sounds = {}
})

DefineIcon({
    Name = "icon-morel-5",
    Size = {46, 38},
    Frame = 0,
    File = "units/morels/morel_5i.png"})

DefineAllow("unit-morel-5", AllowAll)

-- morel-6

DefineUnitType("unit-morel-6", {
    Name = "morel-6",
    Image = {"file", "units/morels/morel_6.png", "size", {64, 128}},
    Shadow = {"file", "units/morels/morel_6s.png", "size", {64, 128}},
    Offset = {0, -23},
    Animations = "animations-morels",
    Icon = "icon-morel-6",
    Construction = "construction-morels",
    HitPoints = 50,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {32, 32},
    Armor = 10,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    EnergyValue = 5000,
    CanHarvestFrom = true,
    Neutral = true,
    NumDirections = 1,
    Sounds = {}
})

DefineIcon({
    Name = "icon-morel-6",
    Size = {46, 38},
    Frame = 0,
    File = "units/morels/morel_6i.png"})

DefineAllow("unit-morel-6", AllowAll)

-- morel-7

DefineUnitType("unit-morel-7", {
    Name = "morel-7",
    Image = {"file", "units/morels/morel_7.png", "size", {64, 128}},
    Shadow = {"file", "units/morels/morel_7s.png", "size", {64, 128}},
    Offset = {0, -24},
    Animations = "animations-morels",
    Icon = "icon-morel-7",
    Construction = "construction-morels",
    HitPoints = 50,
    DrawLevel = 25,
    VisibleUnderFog = true,
    NeutralMinimapColor = {196, 196, 196},
    TileSize = {1, 1},
    BoxSize = {32, 32},
    Armor = 10,
    Missile = "missile-none",
    Priority = 0,
    ExplodeWhenKilled = "missile-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    EnergyValue = 5000,
    CanHarvestFrom = true,
    Neutral = true,
    NumDirections = 1,
    Sounds = {}
})

DefineIcon({
    Name = "icon-morel-7",
    Size = {46, 38},
    Frame = 0,
    File = "units/morels/morel_7i.png"})

DefineAllow("unit-morel-7", AllowAll)

