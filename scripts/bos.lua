--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	bos.lua		-	game specific stuff, and wc2 format compatibility
--
--	(c) Copyright 2001-2003 by Crestez Leonard
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

DefineRaceNames(
	"race", {
		"name", "elites",
		"display", "Elites",
		"visible"},
	"race", {
		"name", "neutral",
		"display", "Neutral"}
	)

function DefineIcon(arg)
    icon = CIcon:New(arg.Name)
    icon.G = CGraphic:New(arg.File, arg.Size[1], arg.Size[2])
    icon.Frame = arg.Frame
end

