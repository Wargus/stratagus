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
    Train = {"frame 10", "wait 5", "frame 11", "wait 5", "frame 12", "wait 5",
        "frame 13", "wait 5", "frame 14", "wait 5", "frame 13", "wait 5",
        "frame 12", "wait 5", "frame 11", "wait 5", "frame 10", "wait 30", },
    Still = {"frame 15", "wait 5", "frame 16", "wait 5", "frame 17", "wait 5",
        "frame 18", "wait 5", "frame 19", "wait 5", "frame 18", "wait 5",
        "frame 17", "wait 5", "frame 16", "wait 5", "frame 15", "wait 10", },
    })

DefineIcon({
        Name = "icon-vault",
        Size = {46, 38},
        Frame = 0,
        File = "units/vault/vault-i.png"})

DefineConstruction("construction-vault", {
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

MakeSound("dev-selected", GetCurrentLuaPath().."/sfx_fort.select.wav")
MakeSound("dev-ready", GetCurrentLuaPath().."/elite.fort.completed.wav")
MakeSound("dev-help", GetCurrentLuaPath().."/elite.fort.underattack.wav")
MakeSound("dev-dead", GetCurrentLuaPath().."/sfx_fort.die.wav")

DefineUnitType("unit-vault", {
	Name = "Vault",
	Image = {"file", "units/vault/vault.png", "size", {236, 208}},
	Shadow = {"file", GetCurrentLuaPath().."/vault_s.png", "size", {236, 208}},
	Animations = "animations-vault", Icon = "icon-vault",
	Costs = {"time", 150, "titanium", 1000, "crystal", 1000},
	RepairHp = 4, RepairCosts = {"titanium", 4}, Construction = "construction-vault",
	Speed = 0, HitPoints = 1800, DrawLevel = 25, TileSize = {6, 6}, BoxSize = {196, 164},
	SightRange = 4, Armor = 30, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 35, AnnoyComputerFactor = 45,
	Points = 200, Supply = 200, ExplodeWhenKilled = "missile-288x288-explosion",
	Corpse = {"build-dead-vault", 0}, Type = "land",
	Building = true, BuilderOutside = true, VisibleUnderFog = true,
	CanStore = {"crystal", "titanium"},
	Sounds = {
		"selected", "dev-selected",
		"ready", "dev-ready",
		"help", "dev-help",
		"dead", "dev-dead"}
	})

DefineAnimations("animations-vault2", {
    Death = {"unbreakable begin", "wait 1", "frame 0", "wait 2000", 
        "frame 1", "wait 200", "frame 2", "wait 200",  "unbreakable end", "wait 1", },
    })

DefineUnitType("build-dead-vault", {
	Name = "vaultCrater",
	Image = {"file", GetCurrentLuaPath().."/vault_c.png", "size", {236, 208}},
	Animations = "animations-vault2", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {6, 6}, BoxSize = {196, 164}, SightRange = 1,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})


DefineAllow("unit-vault", "AAAAAAAA")
