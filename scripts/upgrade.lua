--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	upgrade.lua	-	Define the upgrades.
--
--	(c) Copyright 2001 - 2006 by Lutz Sammer, Crestez Leonard and Francois Beerten
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


function DefineUpgrade(name, icon, costs)
   u = CUpgrade:New(name)
   u.Icon = Icons[icon]
   for j = 1,table.getn(costs) do
      u.Costs[j - 1] = costs[j]
   end
end

DefineUpgrade("upgrade-expl", "icon-expl",
	{100, 250, 300, 0, 0, 0, 0})
DefineUpgrade("upgrade-expl2", "icon-expl2",
	{150, 350, 400, 0, 0, 0, 0})
DefineUpgrade("upgrade-tdril", "icon-tdril",
	{100, 300, 220, 0, 0, 0, 0})
DefineUpgrade("upgrade-ddril", "icon-ddril",
	{150, 400, 350, 0, 0, 0, 0})
DefineUpgrade("upgrade-pdril", "icon-pdril",
	{200, 600, 450, 0, 0, 0, 0})


DefineModifier("upgrade-expl2",
	{"Level", 1},
	{"piercing-damage", 5},
	{"apply-to", "unit-grenadier"})

DefineModifier("upgrade-tdril",
	{"Level", 1},
	{"armor", 1},
	{"apply-to", "unit-engineer"}, {"apply-to", "unit-harvester"})

DefineModifier("upgrade-ddril",
	{"Level", 1},
	{"armor", 1},
	{"apply-to", "unit-engineer"}, {"apply-to", "unit-harvester"})

DefineModifier("upgrade-pdril",
	{"Level", 1},
	{"armor", 1},
	{"apply-to", "unit-engineer"}, {"apply-to", "unit-harvester"})


DefineAllow("upgrade-expl", "AAAAAAAA")
DefineAllow("upgrade-expl2", "AAAAAAAA")
DefineAllow("upgrade-tdril", "AAAAAAAA")
DefineAllow("upgrade-ddril", "AAAAAAAA")
DefineAllow("upgrade-pdril", "AAAAAAAA")


DefineAllow("unit-dead-body", "AAAAAAAA")
DefineAllow("unit-destroyed-1x1-place", "AAAAAAAA")
DefineAllow("unit-destroyed-2x2-place", "AAAAAAAA")
DefineAllow("unit-destroyed-3x3-place", "AAAAAAAA")
DefineAllow("unit-destroyed-4x4-place", "AAAAAAAA")
