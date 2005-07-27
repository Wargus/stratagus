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
--	upgrade.ccl	-	Define the elites dependencies and upgrades.
--
--	(c) Copyright 2001-2004 by Lutz Sammer and Crestez Leonard
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

--	- upgrades

DefineUpgrade("upgrade-expl", "icon", "icon-expl",
	"costs", {100, 250, 300, 0, 0, 0, 0})
DefineUpgrade("upgrade-expl2", "icon", "icon-expl2",
	"costs", {150, 350, 400, 0, 0, 0, 0})
DefineUpgrade("upgrade-tdril", "icon", "icon-tdril",
	"costs", {100, 250, 220, 0, 0, 0, 0})
DefineUpgrade("upgrade-ddril", "icon", "icon-ddril",
	"costs", {150, 350, 350, 0, 0, 0, 0})
DefineUpgrade("upgrade-pdril", "icon", "icon-pdril",
	"costs", {200, 450, 450, 0, 0, 0, 0})
DefineUpgrade("upgrade-void", "icon", "icon-void",
	"costs", {2000, 0, 0, 0, 0, 0, 0})


DefineModifier("upgrade-expl2",
	{"Level", 1},
	{"piercing-damage", 5},
	{"apply-to", "unit-grenadier"})

DefineModifier("upgrade-tdril",
	{"Level", 1},
--	{"harvesting", 10}
	{"apply-to", "unit-engineer"}, {"apply-to", "unit-harvester"})

DefineModifier("upgrade-ddril",
	{"Level", 1},
--	{"harvesting", 20}
	{"apply-to", "unit-engineer"}, {"apply-to", "unit-harvester"})

DefineModifier("upgrade-pdril",
	{"Level", 1},
--	{"harvesting", 30}
	{"apply-to", "unit-engineer"}, {"apply-to", "unit-harvester"})

DefineModifier("upgrade-void",
	{"Level",1},
	{"piercing-damage", 10},
	{"apply-to", "unit-bazoo"}, {"apply-to", "unit-assault"})

DefineAllow("upgrade-expl", "AAAAAAAAAAAAAAAA")
DefineAllow("upgrade-expl2", "AAAAAAAAAAAAAAAA")
DefineAllow("upgrade-tdril", "AAAAAAAAAAAAAAAA")
DefineAllow("upgrade-ddril", "AAAAAAAAAAAAAAAA")
DefineAllow("upgrade-pdril", "AAAAAAAAAAAAAAAA")
DefineAllow("upgrade-void", "AAAAAAAAAAAAAAAA")

-- DefineDependency("upgrade-void", {"upgrade-expl2", "upgrade-pdril"})
DefineDependency("unit-bazoo", {"upgrade-expl2"})
DefineDependency("unit-grenadier", {"upgrade-expl"})
-- DefineDependency("unit-expl2", {"upgrade-expl"})
-- DefineDependency("unit-ddril", {"upgrade-tdril"})
-- DefineDependency("unit-pdril", {"upgrade-ddril"})

DefineDependency("unit-hosp", {"unit-vault", "unit-camp"})
DefineDependency("unit-vfac", {"unit-vault", "unit-rfac"})
DefineDependency("unit-msilo", {"unit-vault", "unit-rfac", "unit-dev-yard"})

