--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-antharus.lua	-	Define the antharus unit.
--
--	(c) Copyright 2005-2007 by Francois Beerten.
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

DefineAnimations("animations-antharus", {
    Still = {"frame 0","wait 10",},
    Death = {"unbreakable begin", "frame 0", "unbreakable end", "wait 10", },
    })

DefineIcon({
	Name = "icon-antharus",
	Size = {46, 38},
	Frame = 0,
	File = "units/antharus/icon.png"})

DefineConstruction("construction-antharus", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 0},
	}
})

DefineUnitType("unit-antharus", {
    Name = "Antharus",
    Image = {"file", "units/antharus/antharus.png", "size", {132, 100}},
    Animations = "animations-antharus",
    Icon = "icon-antharus",
    Construction = "construction-antharus",
    HitPoints = 50,
    DrawLevel = 25,
    EnergyValue = 5000,
    TileSize = {2, 2},
    BoxSize = {64, 64},
    SightRange = 1,
    Armor = 0,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 0,
    AnnoyComputerFactor = 0,
    Points = 10,
    ExplodeWhenKilled = "missile-64x64-explosion",
    Corpse = "unit-destroyed-2x2-place",
    Type = "land",
    Building = true,
    VisibleUnderFog = false,
    CanHarvestFrom = true,
    NumDirections = 1,
    Sounds = {},
    Neutral = true
})

DefineAllow("unit-antharus", ForbidAll)






