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
--	minefield.lua	-	Define the minefields.
--
--	(c) Copyright 2004 by François Beerten
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


DefineMissileType("missile-minefield",
  { File = "minefield.png", Size = {32, 32}, Frames = 1, NumDirections = 1,
  DrawLevel = 20, Class = "missile-class-land-mine", Sleep = 5, Speed = 16, 
  Range = 10,  ImpactMissile = "missile-explosion", CanHitOwner = true } )


DefineSpell("spell-minefield",
        "showname", "minefield",
        "manacost", 0,
        "range", 6,
        "target", "position",
        "action", {
                {"spawn-missile", "ttl", -1, "damage", 500, "missile", "missile-minefield",
                 "start-point", {"base", "target", "add-x", 0, "add-y", 0},
                 "end-point",   {"base", "target", "add-x", 0, "add-y", 0}}
        }
)

DefineButton({
        Pos = 1, Level = 3, Icon = "icon-camp_b", Action = "cast-spell",
        Value = "spell-minefield",  Allowed = "check-true", Key = "f", Hint = "BUILD MINEFIELD",
        ForUnit = {"unit-engineer"}})







--DefineButton({
--        Pos = 8, Level = 0, Icon = "icon-camp_b", Action = "cast-spell",
--        Value = "spell-minefield", Key = "f", Hint = "BUILD MINEFIELD",
--        ForUnit = {"unit-engineer"}})




