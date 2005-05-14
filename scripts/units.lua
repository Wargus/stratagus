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
--	units.lua	-	Define the used unit-types.
--
--	(c) Copyright 1998 - 2004 by Lutz Sammer and Crestez Leonard
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

-- Load the animations for the units.
Load("scripts/anim.lua")

-- Define all of the crystal fields. Only Ident and animations differ.
for i = 1, 13 do
	DefineUnitType("unit-crystal-field" .. i, {
		Name = "Crystal Field" .. i,
		Files = {"default", "nature/res/res_crys.png"}, Size = {32, 32},
		Shadow = {"file", "nature/res/res_crys_s.png", "size", {32, 32}, "offset", {5, 5}},
		Animations = "animations-crystal-field" .. i, Icon = "icon-crystal-field",
		HitPoints = 50,	TileSize= {1, 1}, BoxSize = {31, 31},
		NeutralMinimapColor = {81, 200, 234},
		Armor = 999, Missile = "missile-none",
		Priority = 0, Corpse = {"unit-destroyed-1x1-place", 0},
		Type = "land", Building = true, VisibleUnderFog = true,
		GivesResource = "crystal", CanHarvest = true})
end

DefineUnitType("unit-gold-mine", {
	Name = "Titanium Field",
	Files = {"default", "nature/res/rocks_field.png"}, Size = {128, 128},
	Shadow = {"file", "nature/res/rocks_field_s.png", "size", {128, 128}, "offset", {0, 0}},
	Animations = "animations-building", Icon = "icon-rocks_field",
	Costs = {"time", 150}, VisibleUnderFog = true,
	Construction = "construction-land2",
	NeutralMinimapColor = {196, 196, 196},
	DrawLevel = 40, TileSize = {4, 4}, BoxSize = {96, 96},
	SightRange = 1, Speed = 0, HitPoints = 25500, Priority = 0,
	Armor = 20, BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Corpse = {"unit-destroyed-4x4-place", 0}, ExplodeWhenKilled = "missile-explosion",
	Type = "land", Building = true, GivesResource = "titanium", CanHarvest = true,
	Sounds = {
		"selected", "gold-mine-selected",
		"acknowledge", "gold-mine-acknowledge",
		"ready", "gold-mine-ready",
		"help", "gold-mine-help",
		"dead", "building destroyed"}})

DefineUnitType("unit-dead-body", {
	Name= "Dead Body",
	Files = {"default", "neutral/units/corpses.png"}, Size = {72, 72},
	Animations = "animations-dead-body", Icon = "icon-cancel",
	Speed = 0, HitPoints = 255, DrawLevel = 30, Priority = 0,
	TileSize = {1, 1}, BoxSize = {31, 31}, SightRange = 1,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Type = "land", Vanishes = true})

DefineUnitType("unit-destroyed-1x1-place", { Name = "Destroyed 1x1 Place",
	Files = {
		"default", "tilesets/desert/neutral/buildings/small_destroyed_site.png",},
	Size = {32, 32},
	Animations = "animations-destroyed-place", Icon = "icon-cancel",
	Speed = 0, HitPoints = 255, DrawLevel = 10,
	TileSize = {1, 1}, BoxSize = {31, 31}, SightRange = 2,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land",
	Building = true, VisibleUnderFog = true, Vanishes = true})

DefineUnitType("unit-destroyed-2x2-place", { Name = "Destroyed 2x2 Place",
	Files = {"default", "tilesets/desert/neutral/buildings/destroyed_site.png"},
	Size = {64, 64},
	Animations = "animations-destroyed-place", Icon = "icon-cancel",
	Speed = 0, HitPoints = 255, DrawLevel = 10,
	TileSize = {2, 2}, BoxSize = {63, 63}, SightRange = 2,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land",
	Building = true, VisibleUnderFog = true, Vanishes = true})

DefineUnitType("unit-destroyed-3x3-place", { Name = "Destroyed 3x3 Place",
	Files = {"default", "tilesets/desert/neutral/buildings/destroyed_site.png"},
	Size = {64, 64},
	Animations = "animations-destroyed-place", Icon = "icon-cancel",
	Speed = 0, HitPoints = 255, DrawLevel = 10,
	TileSize = {3, 3}, BoxSize = {95, 95}, SightRange = 3,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land",
	Building = true, VisibleUnderFog = true, Vanishes = true})

DefineUnitType("unit-destroyed-4x4-place", { Name = "Destroyed 4x4 Place",
	Files = {"default", "tilesets/desert/neutral/buildings/destroyed_site.png"},
	Size = {64, 64},
	Animations = "animations-destroyed-place", Icon = "icon-cancel",
	Speed = 0, HitPoints = 255, DrawLevel = 10,
	TileSize = {4, 4}, BoxSize = {127, 127}, SightRange = 3,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land",
	Building = true, VisibleUnderFog = true, Vanishes = true})

DefineUnitType("unit-revealer", {
	Name = "Dummy unit",
	Size = {0, 0},
	Animations = "animations-building", Icon = "icon-cancel",
	Speed = 0, HitPoints = 0,
	TileSize = {1, 1}, BoxSize = {1, 1},
	SightRange = 12,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, DecayRate = 1, Type = "land",
	Building = true, Revealer = true, DetectCloak = true})

-- Needed for stratagus otherwise it crashes
DefineUnitType("unit-human-wall", {
	Name = "Wall",
	Files = {"tileset-desert", "tilesets/desert/neutral/buildings/wall.png"},
	Size = {32, 32}, Costs = {"time", 30},
	Animations = "animations-building", Icon = "icon-cancel",
	Construction = "construction-wall",
	Speed = 0, HitPoints = 40, DrawLevel = 39,
	TileSize = {1, 1}, BoxSize = {31, 31}, SightRange = 1,
	Armor = 20, BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, AnnoyComputerFactor = 45, Points = 1,
	Corpse = {"unit-destroyed-1x1-place", 0},
	ExplodeWhenKilled = "missile-explosion",
	Type = "land", Building = true})

-- Needed to avoid a stratagus crash
DefineUnitType("unit-orc-wall", {
	Name = "Wall", 
	Files = {"tileset-desert", "tilesets/desert/neutral/buildings/wall.png"},
	Size = {32, 32}, Costs = {"time", 30},
	Animations = "animations-building", Icon = "icon-cancel",
	Construction = "construction-wall",
	Speed = 0, HitPoints = 40, DrawLevel = 39,
	TileSize = {1, 1}, BoxSize = {31, 31}, SightRange = 1,
	Armor = 20, BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, AnnoyComputerFactor = 45, Points = 1,
	Corpse = {"unit-destroyed-1x1-place", 0},
	ExplodeWhenKilled = "missile-explosion",
	Type = "land", Building = true})

-- Load the different races
Load("scripts/minefield.lua")
Load("scripts/elites/units.lua")



