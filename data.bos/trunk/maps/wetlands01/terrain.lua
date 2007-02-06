--            ____
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  )
--        /_____/\____/____/
--
--  Invasion - Battle of Survival
--   A GPL'd futuristic RTS game
--
--  wetlands/terrain.lua - Define image based terrain
--
--      (c) Copyright 2006 by Loïs Taulelle.
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
-- $Id: terrain.lua 253 2005-11-12 13:48:40Z feb $


DefineImageTilemodels(GetCurrentLuaPath().."/wetlands01.png", 128, 128)

Load(GetCurrentLuaPath().."/access-unpassable.lua")

RepeatMap(128, 128, 128, 128)
