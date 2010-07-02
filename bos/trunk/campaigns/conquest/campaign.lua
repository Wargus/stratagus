--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      campaign.lua  -  Define the Conquest campaign.
--
--      (c) Copyright 2010 by Jimmy Salmon
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
  CreateMapStep("campaigns/conquest/01/presentation.smp",
    "Establish Outpost 31",
    "Construction has begun on Outpost 31 near the border of the sinister Talius empire.\n\n" ..
    "This new outpost will serve as a crucial lookout point for monitoring enemy troop movements and gathering intelligence.\n\n" ..
    "Because of its strategic importance, you have been assigned to oversee its completion.\n\n" ..
    "You are to lead a small group of soldiers to the outpost and get it fully operational.\n\n" ..
    "Stay alert, there could be enemy forces in the vicinity.\n",
    nil,
    "campaigns/conquest/conquest.png"),
}

