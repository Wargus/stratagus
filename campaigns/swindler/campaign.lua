--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      campaign.lua  -  Define the Swindler campaign.
--
--      (c) Copyright 2010-2013 by François Beerten
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


local briefingtext01 = 
 "Chapter One\n"..
 "Lahen, a failing and broken inventor, managed to sell his "..
 "rubbish solar panels to the Obersims at an enormous price. "..
 "After having paid, the Obersims discovered how much they had been cheated. "..
 "The proud Obersims could not allow this and cried for revenge:\n"..
 "   Lahen wanted ! Dead!\n"..
 "\n"..
 "Lahen knows of the danger and now tries to escape in the "..
 "mountains."

 
local briefingtext02 = 
 "Chapter Two\n"..
 "Lahen escaped in the mountains and arrives in a small valley.\n"..
 "There he founds some abandoned resources and makes a few friends. "..
 "He builds a small army to defend himself since there is a small outpost of the Obersims very near..."
 
 local briefingtext03 = 
 "Chapter Three\n"..
 "Lahen beat the small group of Obersims, with some friends.\n"..
 "But they are all scared of the main force of the Obersims, so they decide ".. 
 " to run. Now they have to cross a desert. To increase their chances, they decide to split. "..
 "Someone thinks there are oasis in the desert with resources - who knows he is right?\n"..
 "From now on, it is everybody fighting for his own..."
 
 local briefingtext04 = 
 "Chapter Four\n"..
 "Lahen survived the fights with the tribes in the desert, and even gained many resources.\n"..
 "At the other side of the desert, he has to pass some lowlands, close to the sea. "..
 "This is the only way to pass - but it is Tubbies territory. "..
 "The Tubbies want him dead, since he massacred their allies, one of the desert tribes. "..
 "And they have a strong army..."
 
  local briefingtext05 = 
 "Chapter Five\n"..
 "The Tubbies are decimated and will leave you alone for now.\n"..
 "You reached the waterside. You better build some defenses, since "..
 "there are some places where you can cross the water, and nobody knows "..
 "who lives across this rough terrain...\n"..
 "Or maybe you should plan a surprise attack?"

 
 
function DisplayEnding()
  t =
   "The End ?\n"..
   "Lahen managed to escape so far.\n"..
   "But the Obersims are still searching him.\n"..
   "And the Tubbies are now decimated, but remain dangerous.\n\n"..
   "How will this end ? Will the Obersims find Lahen ? "..
   "How will the Tubbies react ?\n"..
   "To find out and to play the rest of the campaign, "..
   "help us finish it. There is a big need for new maps."
  obj = {_("Go to boswars.org and contact us to help on the campaign.")}
  RunBriefingMenu(obj, t, nil, "campaigns/swindler/swindler.png")
end

campaign_steps = {
  CreateMapStep("campaigns/swindler/level01.smp",
      {"Get to the upper mountains without being seen."}, 
      briefingtext01,
      nil,
      "campaigns/swindler/swindler.png"),
  CreateMapStep("campaigns/swindler/level02.smp",
      {"Find some abandoned resources in a small valley, build some troops and defend yourself."}, 
      briefingtext02,
      nil,
      "campaigns/swindler/swindler.png"),
  CreateMapStep("campaigns/swindler/level03.smp",
      {"Split up, find resources, dig in and defend your self."}, 
      briefingtext03,
      nil,
      "campaigns/swindler/swindler.png"),
  CreateMapStep("campaigns/swindler/level04.smp",
      {"Train many soldiers, go east, attack the Tubbies in their trenches, you have to pass or die."}, 
      briefingtext04,
      nil,
      "campaigns/swindler/swindler.png"),
  CreateMapStep("campaigns/swindler/level05.smp",
      {"Take your time to build an attack force and then go east, cross the water."}, 
      briefingtext05,
      nil,
      "campaigns/swindler/swindler.png"),
  DisplayEnding,
}

