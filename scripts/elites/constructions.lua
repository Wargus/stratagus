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
--	constructions.ccl	-	Define the elites constructions.
--
--	(c) Copyright 2001,2003 by Lutz Sammer and Jimmy Salmon
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
--	$Id: constructions.lua,v 1.3 2004/11/02 03:30:09 mr-russ Exp $

DefineConstruction("construction-plate", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 0}
	}
})

DefineConstruction("construction-msilo", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 7},
		{Percent = 10, File = "main", Frame = 7},
		{Percent = 20, File = "main", Frame = 8},
		{Percent = 30, File = "main", Frame = 9},
		{Percent = 40, File = "main", Frame = 10},
		{Percent = 50, File = "main", Frame = 11},
		{Percent = 60, File = "main", Frame = 11},
		{Percent = 70, File = "main", Frame = 12},
		{Percent = 80, File = "main", Frame = 13},
		{Percent = 90, File = "main", Frame = 0}
	}
})

DefineConstruction("construction-gen", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 9},
		{Percent = 10, File = "main", Frame = 10},
		{Percent = 20, File = "main", Frame = 11},
		{Percent = 30, File = "main", Frame = 12},
		{Percent = 40, File = "main", Frame = 13},
		{Percent = 50, File = "main", Frame = 14},
		{Percent = 60, File = "main", Frame = 15},
		{Percent = 70, File = "main", Frame = 16},
		{Percent = 80, File = "main", Frame = 17},
		{Percent = 90, File = "main", Frame = 0}
	}
})

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

DefineConstruction("construction-dev-yard", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 7},
		{Percent = 11, File = "main", Frame = 8},
		{Percent = 22, File = "main", Frame = 9},
		{Percent = 33, File = "main", Frame = 10},
		{Percent = 44, File = "main", Frame = 11},
		{Percent = 55, File = "main", Frame = 12},
		{Percent = 66, File = "main", Frame = 13},
		{Percent = 77, File = "main", Frame = 14},
		{Percent = 88, File = "main", Frame = 14}
	}
})

DefineConstruction("construction-camp", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 5},
		{Percent = 10, File = "main", Frame = 6},
		{Percent = 20, File = "main", Frame = 7},
		{Percent = 30, File = "main", Frame = 8},
		{Percent = 40, File = "main", Frame = 9},
		{Percent = 50, File = "main", Frame = 10},
		{Percent = 60, File = "main", Frame = 11},
		{Percent = 70, File = "main", Frame = 12},
		{Percent = 80, File = "main", Frame = 13},
		{Percent = 90, File = "main", Frame = 14}
	}
})

DefineConstruction("construction-rfac", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 2},
		{Percent = 10, File = "main", Frame = 3},
		{Percent = 20, File = "main", Frame = 4},
		{Percent = 30, File = "main", Frame = 5},
		{Percent = 40, File = "main", Frame = 6},
		{Percent = 50, File = "main", Frame = 7},
		{Percent = 60, File = "main", Frame = 8},
		{Percent = 70, File = "main", Frame = 9},
		{Percent = 80, File = "main", Frame = 10},
		{Percent = 90, File = "main", Frame = 0}
	}
})

DefineConstruction("construction-hosp", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 4},
		{Percent = 10, File = "main", Frame = 5},
		{Percent = 20, File = "main", Frame = 6},
		{Percent = 30, File = "main", Frame = 7},
		{Percent = 40, File = "main", Frame = 8},
		{Percent = 50, File = "main", Frame = 9},
		{Percent = 60, File = "main", Frame = 10},
		{Percent = 70, File = "main", Frame = 11},
		{Percent = 80, File = "main", Frame = 12},
		{Percent = 90, File = "main", Frame = 0}
	}
})

DefineConstruction("construction-vfac", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 20},
		{Percent = 10, File = "main", Frame = 21},
		{Percent = 20, File = "main", Frame = 22},
		{Percent = 30, File = "main", Frame = 23},
		{Percent = 40, File = "main", Frame = 24},
		{Percent = 50, File = "main", Frame = 25},
		{Percent = 60, File = "main", Frame = 26},
		{Percent = 70, File = "main", Frame = 27},
		{Percent = 80, File = "main", Frame = 28},
		{Percent = 90, File = "main", Frame = 0}
	}
})

DefineConstruction("construction-gturret", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 10},
		{Percent = 20, File = "main", Frame = 11},
		{Percent = 40, File = "main", Frame = 12},
		{Percent = 60, File = "main", Frame = 13},
		{Percent = 80, File = "main", Frame = 14}
}
})

