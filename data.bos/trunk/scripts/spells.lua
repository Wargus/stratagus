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
--	stratagus.ccl	-	The craft configuration language.
--
--	(c) Copyright 1998-2004 by Crestez Leonard
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
--	$Id: spells.lua,v 1.6 2004/10/26 17:59:18 feb Exp $

DefineBoolFlags("organic")
DefineVariables("Speed")

DefineSpell("spell-healing",
	"showname", "Bandage",
	"manacost", 3,
	"range",  1,
	"target", "unit",
	"action", {	{"spawn-missile", "missile", "missile-heal", "start-point", {"base", "target"} }, 
				{"adjust-vitals", "hit-points", 1}},
	"condition", {
		"organic", "only",
		"Building", "false",
		"self", "false",
		"max-hp-percent", 100},
	"sound-when-cast", "medic-attack",
	"autocast", {
		"range", 6,
		"condition", {"alliance", "only", "max-hp-percent", 90 }}
)

DefineSpell("spell-nuke",
	"showname", "Nuclear Attack",
	"manacost", 300,
	"range", "infinite",
	"target", "position",
	"action", {
		{"spawn-missile", "missile", "missile-nuke",
			"ttl", 800,
			"damage", 0,
			"delay", 0,
			"start-point", {"base", "target", "add-x", 0, "add-y", -300},
			"end-point", {"base", "target", "add-x", 0, "add-y", 32}},
		{"demolish" ,
			"range", 4,
			"damage", 250}},
	"sound-when-cast", "bazoo-attack",
	"autocast", {"range", 128}
)
