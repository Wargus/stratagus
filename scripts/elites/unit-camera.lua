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

DefineNewAnimations("animations-cam", {
    Still = {"frame 0", "wait 2", "frame 1", "wait 2", "frame 2", "wait 2",
        "frame 3", "wait 2", "frame 4", "wait 2", "frame 5", "wait 2", 
        "frame 6", "wait 2", "frame 7", "wait 2", "frame 8", "wait 2", 
        "frame 9", "wait 2", "frame 10", "wait 2", "frame 11", "wait 2", 
        "frame 12", "wait 2", "frame 13", "wait 2", "frame 14", "wait 2", 
        "frame 15", "wait 2", },
    Death = {"unbreakable begin", "frame 0", "unbreakable end", "wait 1", },
    })


DefineNewAnimations("animations-dead_cam", {
    Death = {"unbreakable begin", "frame 0", "wait 2000",
        "frame 1", "unbreakable end", "wait 200", },
    })

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
        NewAnimations = "animations-cam", Icon = "icon-cam",
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
        NewAnimations = "animations-dead_cam", Icon = "icon-cancel",
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






