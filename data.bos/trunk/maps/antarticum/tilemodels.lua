--       _________ __                 __                               
--      /   _____//  |_____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--  T H E   W A R   B E G I N S
--   Stratagus - A free fantasy real time strategy game engine
--
--	tilemodels.lua - Define tilemodels
--
--      (c) Copyright 2005 by François Beerten.
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
-- $Id$

local til
til = {}
for i = 0, 255 do
    til[i+1] = i
end

DefineTileset("atileset",
 "class", "desert",
 "name",  "atileset",
 "image", "maps/antarticum/texture.png",
 -- Slots descriptions
 "slots", {
  "solid", { "light-grass", "land",
    til}
 }
)

SelectTileset("atileset")
