--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	upgrade.lua	-	Define the dependencies and upgrades.
--
--	(c) Copyright 2001 - 2004 by Lutz Sammer and Crestez Leonard
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

--   Stuff for the different races
Load("scripts/elites/upgrade.lua")

DefineAllow("unit-gold-mine", "AAAAAAAAAAAAAAAA")

DefineAllow("unit-dead-body", "AAAAAAAAAAAAAAAA")
DefineAllow("unit-destroyed-1x1-place", "AAAAAAAAAAAAAAAA")
DefineAllow("unit-destroyed-2x2-place", "AAAAAAAAAAAAAAAA")
DefineAllow("unit-destroyed-3x3-place", "AAAAAAAAAAAAAAAA")
DefineAllow("unit-destroyed-4x4-place", "AAAAAAAAAAAAAAAA")
