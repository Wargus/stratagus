--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	stratagus.lua	-	The craft configuration language.
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
--      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
--
--	$Id$

DefineBoolFlags("organic")
DefineVariables("Speed")
DefineVariables("Level")

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
    -- FIXME: bazoo-attack isn't defined yet
	"sound-when-cast", "bazoo-attack",
	"autocast", {"range", 128}
)
