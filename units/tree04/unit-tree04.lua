--       _________ __                 __                               
--      /   _____//  |_____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--			  T H E   W A R   B E G I N S
--	   Stratagus - A free fantasy real time strategy game engine
--
--	unit-tree.lua	-	Define the tree unit.
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

DefineAnimations("animations-tree04", {
    Still = {"frame 0", "random-wait 255 5000", "frame 1", "wait 6", 
        "frame 2", "wait 6", "frame 3", "wait 6", "frame 2", "wait 6", 
        "frame 1", "wait 6", },
    Death = {"unbreakable begin", "frame 0", "unbreakable end", "wait 10", },
    })

DefineUnitType("unit-tree04", {
    Name = "tree04",
    Image = {"file", "units/tree04/tree04.png", "size", {128, 128}},
    Offset = {0, -24},
    Shadow = {"file", "units/tree04/tree04_s.png", "size", {128, 128}},
    Animations = "animations-tree04",
    Icon = "icon-tree",
    EnergyValue = 5000,
    Construction = "construction-tree",
    HitPoints = 50,
    DrawLevel = 25,
    TileSize = {1, 1},
    BoxSize = {32, 32},
    NeutralMinimapColor = {73, 159, 9},
    SightRange = 1,
    Armor = 0,
    BasicDamage = 0,
    PiercingDamage = 0,
    Missile = "missile-none",
    Priority = 0,
    AnnoyComputerFactor = 0,
    Points = 10,
    ExplodeWhenKilled = "missile-64x64-explosion",
    Corpse = "unit-destroyed-1x1-place",
    Type = "land",
    Building = true,
    VisibleUnderFog = true,
    NumDirections = 1,
    CanHarvestFrom = true,
    Neutral = true,
    Sounds = {}
})

DefineAllow("unit-tree04", AllowAll)






