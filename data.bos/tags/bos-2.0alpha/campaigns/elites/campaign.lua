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

local briefingtext01 = 
   "You're the last remaining free leader. "..
   "Defend your small outpost in this crucial battle. "..
   "Your first enemy, Imperial general Szarin, "..
   "knows that victory will spell the end of "..
   "yours. You must lead your war-weary troops ".. 
   "and stop him at all costs. "
local briefingtext02 =
   "Your scouts report that a small force of mercenary is encamped in the " ..
   "Green Valley. " ..
   "Mercenaries no doubt sees this conflict as an opportunity to expand their " ..
   "military influence. Not to worry, though: Mercenarie's troops are neutral " ..
   "and in fact their presence offers you a strategic advantage. If you should " ..
   "so, choose to first capture the mercenary vault, you will have the resources " ..
   "to complete your quest easily. "

campaign_steps = {
  CreateMapStep("campaigns/elites/level01.smp",
      "Build your base. Drive the enemy out. Kill all enemy units", 
      briefingtext01),
  CreateMapStep("campaigns/elites/level02.smp", 
      "Gain an ally and drive the enemy out. Kill all enemy units", 
      briefingtext02),
  CreateMapStep("campaigns/elites/level03.smp", _("Kill them all !"), 
      "Level 3"),
  CreateMapStep("campaigns/elites/level04.smp", _("Kill them all !"), 
      "Level 4"),
  CreateMapStep("campaigns/elites/level05.smp", _("Kill them all !"), 
      "Level 5"),
  CreateMapStep("campaigns/elites/level06.smp", _("Kill them all !"), 
      "Level 6"),
  CreateMapStep("campaigns/elites/level07.smp", _("Kill them all !"), 
      "Level 7"),
  CreateMapStep("campaigns/elites/level08.smp", _("Kill them all !"), 
      "Level 8"),
  CreateMapStep("campaigns/elites/level09.smp", _("Kill them all !"), 
      "Level 9"),
  CreateMapStep("campaigns/elites/level10.smp", _("Kill them all !"), 
      "Level 10")
}

