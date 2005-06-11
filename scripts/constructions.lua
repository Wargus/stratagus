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
--	constructions.lua	-    Define the constructions.
--
--	(c) Copyright 2001-2004 by Lutz Sammer, Jimmy Salmon and Crestez Leonard
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

Load("scripts/elites/constructions.lua")

DefineConstruction("construction-none", {
	Files = {
		File = "neutral/buildings/land_construction_site.png",
		Size = {64, 64}},
	Constructions = {{
		Percent = 0,
		File = "construction",
		Frame = 0},{
		Percent = 25,
		File = "construction",
		Frame = 1},{
		Percent = 50,
		File = "main",
		Frame = 1}}
})

DefineConstruction("construction-land", {
	Files= {
		File = "neutral/buildings/land_construction_site.png",
		Size = {64, 64}},
	Constructions = {{
		Percent = 0,
		File = "construction",
		Frame = 0},{
		Percent = 25,
		File = "construction",
		Frame = 1},{
		Percent = 50,
		File = "main",
		Frame = 1}}
})

DefineConstruction("construction-land2", {
	Files = {
		File = "neutral/buildings/land_construction_site.png",
		Size = {64, 64}},
	Constructions = {{
		Percent = 0,
		File = "construction",
		Frame = 0}, {
		Percent = 25,
		File = "construction",
		Frame = 1}}
})

DefineConstruction("construction-wall", {
	Files = {
		File = "tilesets/desert/neutral/buildings/wall_construction_site.png",
		Size = {32, 32}},
	Constructions = {{
		Percent = 0,
		File = "construction",
		Frame = 0}, {
		Percent = 25,
		File = "construction",
		Frame = 1}, {
		Percent = 50,
		File = "main",
		Frame = 1}}
})
