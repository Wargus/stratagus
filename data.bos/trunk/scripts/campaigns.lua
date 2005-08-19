--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
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
