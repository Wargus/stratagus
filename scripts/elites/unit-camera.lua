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
--	camera.lua	-	Define the camera unit.
--
--	(c) Copyright 2004 by gorm.
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
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id$

DefineAnimations("animations-cam",
   "still", {
        {0, 0, 2, 0}, {0, 0, 2, 1}, {0, 0, 2, 2}, {0, 0, 2, 3},
        {0, 0, 2, 4}, {0, 0, 2, 5}, {0, 0, 2, 6}, {0, 0, 2, 7},
        {0, 0, 2, 8}, {0, 0, 2, 9}, {0, 0, 2,10}, {0, 0, 2,11},
        {0, 0, 2,12}, {0, 0, 2,13}, {0, 0, 2,14}, {3, 0, 2,15}},
   "die", {
        {0, 0,100, 0}, {0, 0,100, 1}, {3, 0, 1, 1}})

DefineIcon({
	Name = "icon-cam",
	Size = {46, 38},
	Frame = 0,
	File = "elites/build/camera_i.png"})

DefineIcon({
	Name = "icon-cam_b",
	Size = {46, 38},
	Frame = 0,
	File = "elites/build/camera_b.png"})

MakeSound("camera-selected", "elites/buildings/sfx_camera.select.wav")

DefineConstruction("construction-cam", {
        Constructions = {
                {Percent = 0, File = "main", Frame = 16},
                {Percent = 20, File = "main", Frame = 17},
                {Percent = 30, File = "main", Frame = 18},
                {Percent = 40, File = "main", Frame = 19},
                {Percent = 50, File = "main", Frame = 20},
                {Percent = 60, File = "main", Frame = 21},
                {Percent = 80, File = "main", Frame = 22},
                {Percent = 90, File = "main", Frame = 23}
        }
})

DefineUnitType("unit-cam", {
        Name = "Camera",
        Files = {"tileset-desert", "elites/build/camera.png"}, Size = {32, 64},
        Offset = {0, -16},
        Shadow = {"file", "elites/build/camera_s.png", "size", {64, 64}, "offset", {16,0}},
        Animations = "animations-cam", Icon = "icon-cam",
        Costs = {"time", 20, "titanium", 7, "crystal", 35},
        RepairHp = 1, RepairCosts = {"crystal", 3}, Construction = "construction-cam",
        Speed = 0, HitPoints = 5, DrawLevel = 25, TileSize  = {1, 1}, BoxSize = {28, 28},
        SightRange = 15, Armor = 0 , BasicDamage = 0, PiercingDamage = 0,
        Missile = "missile-none", Priority = 20, AnnoyComputerFactor = 65,
        Points = 10, Supply = 0, ExplodeWhenKilled = "missile-64x64-explosion",
        Corpse = {"camera_destroyed", 0}, Type = "land",
	Building = true, BuilderOutside = true,
        VisibleUnderFog = true,
        Sounds = {"selected", "camera-selected"}
        })

DefineUnitType("camera_destroyed", {
        Name = "CameraCrater",
        Files = {"tileset-desert", "elites/build/camera_c.png"}, Size = {32, 64},
        Animations = "animations-cam", Icon = "icon-cancel",
        Speed = 0, HitPoints = 999, DrawLevel = 10,
        TileSize = {1, 1}, BoxSize = {28, 28}, SightRange = 1,
        BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
        Priority = 0, Type = "land", Building = true, Vanishes = true
        })

DefineAllow("unit-cam", "AAAAAAAAAAAAAAAA")
DefineDependency("unit-cam", {"unit-vault"})

DefineButton({
        Pos = 5, Level = 1, Icon = "icon-cam_b", Action = "build",
        Value = "unit-cam", Key = "c", Hint = "BUILD ~!CAMERA",
        ForUnit = {"unit-engineer"}})






