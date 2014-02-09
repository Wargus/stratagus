--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-plant.lua	-	Define the plant units.
--
--	(c) Copyright 2007 by Francois Beerten.
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
--	$Id: unit-tree.lua 8780 2007-04-27 20:53:31Z feb $

DefineAnimations("animations-static", {
    Still = {"frame 0", "wait 1", },
    Death = {"unbreakable begin", "frame 0", "unbreakable end", "wait 1", },
    })


DefineConstruction("construction-static", {
	Constructions = {
		{Percent = 0, File = "main", Frame = 0},
	}
})

function DefinePlantType(internalname, name, image_size,
                         tilesize, offsets)
   DefineIcon({
	Name = "icon-"..internalname,
	Size = {46, 38},
	Frame = 0,
	File = "units/plants/"..internalname.."_icon.png"})
   DefineUnitType("unit-"..internalname, {
       Name = name,
       Image = {"file", "units/plants/"..internalname..".png", "size", image_size},
       Offset = offsets,
       Shadow = {"file", "units/plants/"..internalname.."_shadow.png", "size", image_size},
       Animations = "animations-static",
       Icon = "icon-"..internalname,
       Construction = "construction-static",
       EnergyValue = 3000,
       HitPoints = 50,
       DrawLevel = 25,
       TileSize = tilesize,
       BoxSize = {32*tilesize[1], 32*tilesize[2]},
       NeutralMinimapColor = {73, 159, 9},
       SightRange = 1,
       Armor = 0,
       BasicDamage = 0,
       PiercingDamage = 0,
       Missile = "missile-none",
       Priority = 0,
       AnnoyComputerFactor = 0,
       Points = 10,
       ExplodeWhenKilled = "missile-64x64-explosion",
       Type = "land",
       Building = true,
       VisibleUnderFog = true,
       NumDirections = 1,
       CanHarvestFrom = true,
       Sounds = {},
       Neutral = true
   })
   DefineAllow("unit-"..internalname, AllowAll)
end

DefinePlantType("pitcher", 
                "Pitcher",
                {128, 128},
    		{3, 3},
                {0, -16})
DefinePlantType("rafflesia", 
                "Rafflesia",
                {128, 128},
    		{3, 2},
                {0, -26})







