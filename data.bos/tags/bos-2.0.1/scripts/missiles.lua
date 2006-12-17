--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	missiles.lua	-	Define the used missiles.
--
--	(c) Copyright 1998-2004 by Lutz Sammer, Fabrice Rossi and Crestez Leonard
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

-------------------------------------------------------------------------------Y
--	Define missiles
-------------------------------------------------------------------------------Y

DefineMissileType("missile-nuke", {
	File = "elites/missile/nuke.png",
	Size = {128, 128}, Frames = 5, NumDirections = 8,
	ImpactSound = "dev-dead", DrawLevel = 300,
	Class = "missile-class-point-to-point", Sleep = 1, Speed = 64, Range = 1,
	ImpactMissile = "missile-288x288-explosion"})

DefineMissileType("missile-grenadier", {
	File =  "elites/missile/grenade.png",
	Size = {32, 32}, Frames = 5, NumDirections = 8,
	ImpactSound = "grenade-impact", DrawLevel = 50,
	Class = "missile-class-parabolic", Sleep = 1, Speed = 16, Range = 2,
	ImpactMissile = "missile-64x64-explosion"})

DefineMissileType("missile-bazoo", {
	File = "elites/missile/rocket.png",
	Size = {32, 32}, Frames = 5, NumDirections = 8,
	ImpactSound = "rocket-impact", DrawLevel = 50,
	Class = "missile-class-point-to-point", Sleep = 1, Speed = 16, Range = 2,
	ImpactMissile = "missile-64x64-explosion"})

DefineMissileType("missile-64x64-explosion", {
	File = "missiles/expl_64x64x1.png",
	Size = {64, 64}, Frames = 7, NumDirections = 1, DrawLevel = 50,
	Class = "missile-class-stay", Sleep = 1, Speed = 16, Range = 1})

DefineMissileType("missile-160x128-explosion", {
	File = "missiles/expl_160x128x1.png",
	Size = {160, 128}, Frames = 20, NumDirections = 1, DrawLevel = 50,
	Class = "missile-class-stay", Sleep = 1, Speed = 16, Range = 1})

DefineMissileType("missile-288x288-explosion", {
	File = "missiles/expl_288x288x1.png",
	Size = {288, 288}, Frames = 20, NumDirections = 1, DrawLevel = 300,
	Class = "missile-class-stay", Sleep = 1, Speed = 16, Range = 15})


DefineMissileType("missile-small-fire", {
	File = "missiles/small_fire.png",
	Size = {32, 48}, Frames = 6, NumDirections = 1, DrawLevel = 45,
	Class = "missile-class-fire", Sleep = 8, Speed = 16, Range = 1})

DefineMissileType("missile-big-fire", {
	File = "missiles/big_fire.png",
	Size = {48, 48}, Frames = 10, NumDirections = 1, DrawLevel = 45,
	Class = "missile-class-fire", Sleep = 8, Speed = 16, Range = 1})

DefineMissileType("missile-explosion", {
	File = "missiles/explosion.png",
	Size = {64, 64}, Frames = 20, NumDirections = 1, DrawLevel = 50,
	Class = "missile-class-stay", Sleep = 1, Speed = 16, Range = 1})

DefineMissileType("missile-green-cross", {
	File = "missiles/green_cross.png",
	Size = {32, 32}, Frames = 4, NumDirections = 1, DrawLevel = 150,
	Class = "missile-class-cycle-once", Sleep = 1, Speed = 16, Range = 1})

DefineMissileType("missile-none", {
	Size = {32, 32}, DrawLevel = 50,
	Class = "missile-class-none", Sleep = 1, Speed = 16, Range = 1})

DefineMissileType("missile-hit", {
	Size = {15, 15}, DrawLevel = 150,
	Class = "missile-class-hit", Sleep = 1, Speed = 1, Range = 16})

DefineBurningBuilding(
	{"percent", 0, "missile", "missile-big-fire"}, 
	{"percent", 50, "missile", "missile-small-fire"},
	{"percent", 75}) -- no missile
