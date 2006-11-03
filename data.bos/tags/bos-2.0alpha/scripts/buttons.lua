--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	buttons.lua	-	Define the general unit-buttons.
--
--	(c) Copyright 2001 by Vladi Belperchinov-Shabanski, Lutz Sammer and
--							Crestez Leonard
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

-- Load the buttons of all races

Load("scripts/elites/buttons.lua")

-- general cancel button ------------------------------------------------------

DefineButton( { Pos = 9, Level = 9, Icon = "icon-cancel",
  Action = "cancel",
  Key = "\027", Hint = "~<ESC~> CANCEL",
  ForUnit = {"*"} } )

DefineButton( { Pos = 9, Level = 0, Icon = "icon-cancel",
  Action = "cancel-upgrade",
  Key = "\027", Hint = "~<ESC~> CANCEL UPGRADE",
  ForUnit = {"cancel-upgrade"} } )

DefineButton( { Pos = 9, Level = 0, Icon = "icon-cancel",
  Action = "cancel-train-unit",
  Key = "\027", Hint = "~<ESC~> CANCEL UNIT TRAINING",
  ForUnit = {"*"} } )

DefineButton( { Pos = 9, Level = 0, Icon = "icon-cancel",
  Action = "cancel-build",
  Key = "\027", Hint = "~<ESC~> CANCEL CONSTRUCTION",
  ForUnit = {"cancel-build"} } )
