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
--	unit-rocksfield.lua	-	Define the rocks-field.
--
--	(c) Copyright 1998 - 2005 by Lutz Sammer, Crestez Leonard, François Beerten
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
        Name = "icon-rocks_field",
        Size = {46, 38},
        Frame = 0,
        File = "units/rocksfield/ico_rocks_field.png"})

-- Note: the name of the unit is hardcoded as unit-gold-mine in the editor.
DefineUnitType("unit-gold-mine", {
	Name = "Titanium Field",
	Image = {"file", "units/rocksfield/rocks_field.png", "size", {128, 128}},
	Shadow = {"file", "units/rocksfield/rocks_field_s.png", "size", {128, 128}, "offset", {0, 0}},
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

