--       _________ __                 __                               
--      /   _____//  |_____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--                        T H E   W A R   B E G I N S
--         Stratagus - A free fantasy real time strategy game engine
--
--      campaigns.lua - List all available campaigns.
--
--      (c) Copyright 2005 by François Beerten
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
--      $Id:  $
--=============================================================================
-- List all available campaigns automatically and declare them to the engine

local list
local i
local f
local ff

list = ListDirectory("campaigns/")
for i,f in list do
  if not(string.find(f, "^%.")) then
     local subdirlist = ListDirectory("campaigns/" .. f)
     for i, ff in subdirlist do
        if(string.find(ff, "^campaign.*%.lua$")) then
          print("Found a campaign: " .. ff)
          DefineCampaign(f, "name", f, "file", "campaigns/"..f.."/"..ff)
        end
     end
  end
end
