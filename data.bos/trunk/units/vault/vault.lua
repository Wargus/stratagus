--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	unit-vault.lua	-	Define the vault unit.
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

DefineAnimations("animations-vault", {
    Still = {"frame 0", "wait 5", "frame 1", "wait 5", "frame 2", "wait 5",
        "frame 3", "wait 5", "frame 4", "wait 5", "frame 5", "wait 5",
        "frame 5", "wait 5", "frame 5", "wait 5", "frame 4", "wait 5",
        "frame 3", "wait 5", "frame 2", "wait 5", "frame 1", "wait 5",
        "frame 0", "wait 5", "frame 0", "wait 5", },
    Death = {"unbreakable begin", "frame 0", "unbreakable end", "wait 3", },
    })

DefineIcon({
        Name = "icon-vault",
        Size = {46, 38},
        Frame = 0,
        File = "units/vault/vault-i.png"})

DefineConstruction("construction-vault", {
        Constructions = {
                {Percent = 0, File = "main", Frame = 6},
                {Percent = 1, File = "main", Frame = 7},
                {Percent = 2, File = "main", Frame = 8},
                {Percent = 3, File = "main", Frame = 9},
                {Percent = 4, File = "main", Frame = 10},
                {Percent = 5, File = "main", Frame = 11},
                {Percent = 6, File = "main", Frame = 12},
                {Percent = 7, File = "main", Frame = 13},
                {Percent = 8, File = "main", Frame = 14},
                {Percent = 9, File = "main", Frame = 15}
        }
})

MakeSound("dev-selected", GetCurrentLuaPath().."/sfx_fort.select.wav")
MakeSound("dev-ready", GetCurrentLuaPath().."/elite.fort.completed.wav")
MakeSound("dev-help", GetCurrentLuaPath().."/elite.fort.underattack.wav")
MakeSound("dev-dead", GetCurrentLuaPath().."/sfx_fort.die.wav")

DefineUnitType("unit-vault", {
	Name = "Vault",
	Image = {"file", "units/vault/vault.png", "size", {160, 220}},
	Animations = "animations-vault", Icon = "icon-vault",
	Costs = {"time", 150, "titanium", 1000, "crystal", 1000},
	RepairHp = 4, RepairCosts = {"titanium", 4}, Construction = "construction-vault",
	Speed = 0, HitPoints = 1800, DrawLevel = 25, TileSize = {5, 4}, BoxSize = {164, 132},
	SightRange = 4, Armor = 30, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 35, AnnoyComputerFactor = 45,
	Points = 200, Supply = 200, ExplodeWhenKilled = "missile-288x288-explosion",
	Corpse = {"build-dead-body3", 0}, Type = "land",
	Building = true, BuilderOutside = true, VisibleUnderFog = true,
	CanStore = {"crystal", "titanium"},
	Sounds = {
		"selected", "dev-selected",
		"ready", "dev-ready",
		"help", "dev-help",
		"dead", "dev-dead"}
	})

DefineAllow("unit-vault", "AAAAAAAAAAAAAAAA")
