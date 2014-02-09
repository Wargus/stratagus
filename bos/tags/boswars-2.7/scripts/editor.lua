--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	editor.lua	-	Editor configuration and functions.
--
--	(c) Copyright 2002-2008 by Lutz Sammer, Francois Beerten and Jimmy Salmon
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


Load("scripts/uilayout.lua")

HandleCommandKey = HandleEditorIngameCommandKey

DefineIcon({
  Name = "icon-editor-patch",
  Size = {46, 38},
  Frame = 0,
  File = "graphics/ui/editor_patch.png"})

-- Set which icons to display
SetEditorSelectIcon("icon-patrol")
SetEditorUnitsIcon("icon-assault")
SetEditorPatchIcon("icon-editor-patch")

-- Start location unit
DefineIcon({
  Name = "icon-start-location",
  Size = {32, 32},
  Frame = 0,
  File = "ui/x_startpoint.png"})
DefineUnitType("unit-start-location", {
  Name = "Start Location",
  Image = {"file", "ui/x_startpoint.png", "size", {32, 32}},
  Animations = "animations-building", Icon = "icon-start-location",
  HitPoints = 0, DrawLevel = 0, TileSize = {1, 1},
  BoxSize = {31, 31}, SightRange = 0, BasicDamage = 0, PiercingDamage = 0,
  Missile = "missile-none", Priority = 0, Type = "land", NumDirections = 1,
 })
SetEditorStartUnit("unit-start-location")

-- editor-unit-types a sorted list of unit-types for the editor.
-- FIXME: this is only a temporary hack, for better sorted units.
local editor_types = {
  "unit-vault",
  "unit-hotspot",
  "unit-weakhotspot",

  "unit-apcs",
  "unit-medic",
  "unit-assault",
  "unit-bazoo",
  "unit-grenadier",
  "unit-engineer",
  "unit-harvester",
  "unit-msilo",
  "unit-aircraftfactory",
  "unit-magmapump",
  "unit-camp",
  "unit-powerplant",
  "unit-nukepowerplant",
  "unit-hosp",
  "unit-vfac",
  "unit-gturret",
  "unit-biggunturret",
  "unit-cannon",
  "unit-cam",
  "unit-radar",
  "unit-buggy",
  "unit-rtank",
  "unit-tank",
  "unit-artil",
  "unit-chopper",
  "unit-bomber",
  "unit-jet",
  "unit-heli",
  
  "unit-shipyard",
  "unit-wscout",
  "unit-destroyer",

  "unit-tree",
  "unit-tree02",
  "unit-tree03",
  "unit-tree04",
  "unit-rocksfield",
  "unit-rock-1",
  "unit-rock-2",
  "unit-rock-3",
  "unit-rock-4",
  "unit-rock-5",
  "unit-rock-6",
  "unit-morel-1",
  "unit-morel-2",
  "unit-morel-3",
  "unit-morel-4",
  "unit-morel-5",
  "unit-morel-6",
  "unit-morel-7",
  "unit-antharus",
  "unit-pitcher",
  "unit-rafflesia",
}

Editor.UnitTypes:clear()
for key,value in ipairs(editor_types) do
  Editor.UnitTypes:push_back(value)
end

