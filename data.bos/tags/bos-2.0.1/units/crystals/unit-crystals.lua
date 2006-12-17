--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	unit-crystals.lua - Define the crystals.
--
--	(c) Copyright 1998 - 2005 by Lutz Sammer, Crestez Leonard, François Beerten
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

DefineAnimations("animations-crystal-field1", {
    Still = {"frame 6", "wait 4", "frame 6", "wait 1", },
    })
DefineAnimations("animations-crystal-field2", {
    Still = {"frame 10", "wait 4", "frame 10", "wait 1", },
    })
DefineAnimations("animations-crystal-field3", {
    Still = {"frame 12", "wait 4", "frame 12", "wait 1", },
    })
DefineAnimations("animations-crystal-field4", {
    Still = {"frame 4", "wait 4", "frame 4", "wait 1", },
    })
DefineAnimations("animations-crystal-field5", {
    Still = {"frame 0", "wait 4", "frame 0", "wait 1", },
    })
DefineAnimations("animations-crystal-field6", {
    Still = {"frame 1", "wait 4", "frame 1", "wait 1", },
    })
DefineAnimations("animations-crystal-field7", {
    Still = {"frame 5", "wait 4", "frame 5", "wait 1", },
    })
DefineAnimations("animations-crystal-field8", {
    Still = {"frame 7", "wait 4", "frame 7", "wait 1", },
    })
DefineAnimations("animations-crystal-field9", {
    Still = {"frame 2", "wait 4", "frame 2", "wait 1", },
    })
DefineAnimations("animations-crystal-field10", {
    Still = {"frame 3", "wait 4", "frame 3", "wait 1", },
    })
DefineAnimations("animations-crystal-field11", {
    Still = {"frame 11", "wait 4", "frame 11", "wait 1", },
    })
DefineAnimations("animations-crystal-field12", {
    Still = {"frame 8", "wait 4", "frame 8", "wait 1", },
    })
DefineAnimations("animations-crystal-field13", {
    Still = {"frame 9", "wait 4", "frame 9", "wait 1", },
    })

DefineIcon({
    Name = "icon-crystal-field",
    Size = {46, 38},
    Frame = 0,
    File = "units/crystals/ico_crys.png"})

-- Define all of the crystal fields. Only Ident and animations differ.
for i = 1, 13 do
    DefineUnitType("unit-crystal-field" .. i, {
        Name = "Crystal Field" .. i,
        Image = {"file", "units/crystals/res_crys.png", "size", {32, 32}},
        Shadow = {"file", "units/crystals/res_crys_s.png", "size", {32, 32}, "offset", {5, 5}},
        Animations = "animations-crystal-field" .. i, Icon = "icon-crystal-field",
        HitPoints = 50,	TileSize= {1, 1}, BoxSize = {31, 31},
        NeutralMinimapColor = {81, 200, 234},
        Armor = 999, Missile = "missile-none",
        Priority = 0, Corpse = "unit-destroyed-1x1-place",
        Type = "land", Building = true, VisibleUnderFog = true,
        GivesResource = "crystal", CanHarvest = true})
end
