--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
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
--      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
--
--	$Id$

DefineAnimations("animations-cam", {
    Still = {"frame 0", "wait 2", "frame 1", "wait 2", "frame 2", "wait 2",
        "frame 3", "wait 2", "frame 4", "wait 2", "frame 5", "wait 2", 
        "frame 6", "wait 2", "frame 7", "wait 2", "frame 8", "wait 2", 
        "frame 9", "wait 2", "frame 10", "wait 2", "frame 11", "wait 2", 
        "frame 12", "wait 2", "frame 13", "wait 2", "frame 14", "wait 2", 
        "frame 15", "wait 2", },
    Death = {"unbreakable begin", "frame 0", "unbreakable end", "wait 1", },
    })


DefineAnimations("animations-dead_cam", {
    Death = {"unbreakable begin", "frame 0", "wait 2000",
        "frame 1", "unbreakable end", "wait 200", },
    })

DefineIcon({
	Name = "icon-cam",
	Size = {46, 38},
	Frame = 0,
	File = "units/camera/camera_i.png"})

DefineIcon({
	Name = "icon-cam_b",
	Size = {46, 38},
	Frame = 0,
	File = "units/camera/camera_i.png"})

MakeSound("camera-selected", "units/camera/sfx_camera.select.wav")

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
	Image = {"file", "units/camera/camera.png", "size", {32, 64}},
	Offset = {0, -16},
	Shadow = {"file", "units/camera/camera_s.png", "size", {64, 64}, "offset", {16,0}},
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
	Image = {"file", "units/camera/camera_c.png", "size", {32, 64}},
	Animations = "animations-dead_cam", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {1, 1}, BoxSize = {28, 28}, SightRange = 1,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
})

DefineAllow("unit-cam", "AAAAAAAAAAAAAAAA")

DefineButton({
	Pos = 5, Level = 1, Icon = "icon-cam_b", Action = "build",
	Value = "unit-cam", Key = "c", Hint = "BUILD ~!CAMERA",
	ForUnit = {"unit-engineer"}})



