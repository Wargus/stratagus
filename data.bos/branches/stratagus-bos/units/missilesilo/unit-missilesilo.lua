--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	unit-missilesilo.lua	-	Define the missile silo unit
--
--	(c) Copyright 2001 - 2005 by François Beerten, Lutz Sammer and Crestez Leonard
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

DefineIcon({
	Name = "icon-msilo",
	Size = {46, 38},
	Frame = 0,
	File = GetCurrentLuaPath().."/missile_silo_i.png"})

DefineConstruction("construction-msilo", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 0},
		{Percent = 10, File = "main", Frame = 1},
		{Percent = 20, File = "main", Frame = 2},
		{Percent = 30, File = "main", Frame = 3},
		{Percent = 40, File = "main", Frame = 4},
		{Percent = 50, File = "main", Frame = 5},
		{Percent = 60, File = "main", Frame = 6},
		{Percent = 70, File = "main", Frame = 7},
		{Percent = 80, File = "main", Frame = 8},
		{Percent = 90, File = "main", Frame = 9}
	}
})
DefineAnimations("animations-msilo", {
    Still = {"frame 15", "wait 5", "frame 16", "wait 5", "frame 17", "wait 5",
             "frame 18", "wait 5", "frame 19", "wait 5", },
    Train = {"frame 15", "wait 5", "frame 16", },
    Attack = {"unbreakable begin", "frame 11", "wait 4",
        "frame 12", "sound msilo-attack", "wait 2", "attack",
        "frame 13", "wait 2", "frame 14", "wait 10", "frame 15", 
        "unbreakable end", "wait 25", },
    })
DefineUnitType("unit-msilo", {
	Name = "Missile Silo",
	Image = {"file", GetCurrentLuaPath().."/missile_silo.png", "size", {256, 256}},
	Shadow = {"file", GetCurrentLuaPath().."/missile_silo_s.png", "size", {256, 256}},
	Animations = "animations-msilo", Icon = "icon-msilo",
	Costs = {"time", 2000, "titanium", 10000, "crystal", 10000},
	RepairHp = 2, RepairCosts = {"titanium", 2},
	Construction = "construction-msilo",
	Speed = 0, HitPoints = 450, DrawLevel = 25, 
	TileSize = {5, 5}, BoxSize = {168, 168},
	SightRange = 3, Armor = 10, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 20, AnnoyComputerFactor = 45,
	Points = 100, ExplodeWhenKilled = "missile-160x128-explosion",
	Corpse = {"build-dead-msilo", 0}, Type = land,
	MaxMana = 1000, CanCastSpell = {"spell-nuke"},
	Demand = 400, Building = true, BuilderOutside = true,
	VisibleUnderFog = true,
	})
DefineAnimations("animations-dead-msilo", {
    Death = {"unbreakable begin", "wait 1", "frame 1", "wait 2000", 
             "unbreakable end", "wait 1", },
    })
DefineUnitType("build-dead-msilo", {
	Name = "SiloCrater",
	Image = {"file", GetCurrentLuaPath().."/missile_silo.png", "size", {256, 256}},
	Animations = "animations-dead-msilo", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,	TileSize = {5, 5},
	BoxSize = {124, 124}, SightRange = 1, BasicDamage = 0,
	PiercingDamage = 0, Missile = "missile-none", Priority = 0,
	Type = "land" , Building = true, Vanishes = true})

DefineAllow("unit-msilo", "AAAAAAAA")

