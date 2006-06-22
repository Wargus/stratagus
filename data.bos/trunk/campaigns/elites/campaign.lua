--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--      Invasion - Battle of Survival                  
--       A GPL'd futuristic RTS game
--
--      campaign.lua  -  Define the Elite campaign 1.
--
--      (c) Copyright 2005-2006 by Lois Taulelle and François Beerten
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
--=============================================================================
--  Define the campaign
--

local briefingtext1 = {
  "You're the last remaining free leader. ",
  "Defend your small outpost in this crucial battle. ",
  "Your first enemy, Imperial general Szarin, ",
  "knows that victory will spell the end of ",
  "yours. You must lead your war-weary troops ", 
  "and stop him at all costs."
}

campaign_steps = {
  CreateMapStep("campaigns/elites/level01.smp", "Kill them all !", briefingtext1),
  CreateMapStep("campaigns/elites/level02.smp"),
  CreateMapStep("campaigns/elites/level03.smp"),
  CreateMapStep("campaigns/elites/level04.smp"),
  CreateMapStep("campaigns/elites/level05.smp"),
  CreateMapStep("campaigns/elites/level06.smp"),
  CreateMapStep("campaigns/elites/level07.smp"),
  CreateMapStep("campaigns/elites/level08.smp"),
  CreateMapStep("campaigns/elites/level09.smp"),
  CreateMapStep("campaigns/elites/level10.smp")}

