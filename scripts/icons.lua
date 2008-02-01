--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	icons.lua	-	Define the icons.
--
--	(c) Copyright 2003-2008 by Jimmy Salmon and Crestez Leonard.
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

function DefineIcon(arg)
	local icon = CIcon:New(arg.Name)
	icon.G = CGraphic:New(arg.File, arg.Size[1], arg.Size[2])
	icon.Frame = arg.Frame
end

-- Build buttons
DefineIcon({
	Name = "icon-magmapump_b",
	Size = {46, 38},
	Frame = 0,
	File = "units/magmapump/magmapump_i.png"})

DefineIcon({
	Name = "icon-camp_b",
	Size = {46, 38},
	Frame = 0,
	File = "units/camp/training_camp_i.png"})

DefineIcon({
	Name = "icon-dev_b",
	Size = {46, 38},
	Frame = 0,
	File = "units/aircraftfactory/aircraftfactory_i.png"})

DefineIcon({
	Name = "icon-powerplant_b",
	Size = {46, 38},
	Frame = 0,
	File = "units/powerplant/powerplant_i.png"})

DefineIcon({
	Name = "icon-hosp_b",
	Size = {46, 38},
	Frame = 0,
	File = "units/hospital/hospital_i.png"})

DefineIcon({
	Name = "icon-vfac_b",
	Size = {46, 38},
	Frame = 0,
	File = "units/vehiclefactory/vehicle_factory_i.png"})

DefineIcon({
	Name = "icon-msilo_b",
	Size = {46, 38},
	Frame = 0,
	File = "units/missilesilo/missile_silo_i.png"})

DefineIcon({
	Name = "icon-build-lvl1",
	Size = {46, 38},
	Frame = 0,
	File = "units/engineer/engineer_icons.png"})

DefineIcon({
	Name = "icon-build-lvl2",
	Size = {46, 38},
	Frame = 1,
	File = "units/engineer/engineer_icons.png"})

DefineIcon({
	Name = "icon-build-lvl3",
	Size = {46, 38},
	Frame = 2,
	File = "units/engineer/engineer_icons.png"})

--
--  MISC
--  

DefineIcon({
	Name = "icon-cancel",
	Size = {46, 38},
	Frame = 0,
	File = "general/commands.png"})

DefineIcon({
	Name = "icon-stop",
	Size = {46, 38},
	Frame = 1,
	File = "general/commands.png"})

DefineIcon({
	Name = "icon-move",
	Size = {46, 38},
	Frame = 2,
	File = "general/commands.png"})

DefineIcon({
	Name = "icon-attack",
	Size = {46, 38},
	Frame = 3,
	File = "general/commands.png"})

DefineIcon({
	Name = "icon-patrol",
	Size = {46, 38},
	Frame = 4,
	File = "general/commands.png"})

DefineIcon({
	Name = "icon-attack-ground",
	Size = {46, 38},
	Frame = 5,
	File = "general/commands.png"})


DefineIcon({
	Name = "icon-repair",
	Size = {46, 38},
	Frame = 6,
	File = "general/commands.png"})

DefineIcon({
	Name = "icon-harvest",
	Size = {46, 38},
	Frame = 8,
	File = "general/commands.png"})

DefineIcon({
	Name = "icon-build-advanced",
	Size = {46, 38},
	Frame = 13,
	File = "general/commands.png"})

DefineIcon({
	Name = "icon-stand-ground",
	Size = {46, 38},
	Frame = 10,
	File = "general/commands.png"})

