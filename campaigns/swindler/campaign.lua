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
--      (c) Copyright 2010 by François Beerten
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

function DisplayEnding()
  t =
   "The End ?\n"..
   "Lehan managed to escape in the mountains.\n"..
   "But the Obersims are searching him.\n\n"..
   "How will this end ? Will the Obersims find Lehan ? "..
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
  DisplayEnding,
}

