--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--      Invasion - Battle of Survival                  
--       A GPL'd futuristic RTS game
--
--      campaign.lua  -  Tutorial campaign.
--
--      (c) Copyright 2005-2006 by François Beerten
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


campaign_steps = {
  CreateMapStep("maps/tutorial.smp",
      "Move your units to the vault in the center of the map",
      "Order from the Central Command:\n\n"..
      "TODO.\n"..
      "-- End of transmission 1. Commander GK --"),
  CreateMapStep("maps/tutorial.smp",
      "Train 2 engineers and build 1 power plant",
      "News from the Front:\n\n"..
      "We finally arrived at our camp.\n"..
      "Despite of the recent attacks, the vault is still "..
      "in a good state. "..
      "With the vault, we will train 2 extra "..
      "engineers to reinforce our group. "..
      "We also need a power plant in order to"..
      "to allow us to build more units ".. 
      "for our base.\n"..
      "-- End of transmission 1. Sergeant TJ --"),
  CreateMapStep("maps/tutorial.smp", 
      "",
      "News from the Front:\n"..
      ". "..
      " "..
      " "..
      " ".. 
      "-- End of transmission. Sergeant TJ --"),

}

