--       _________ __                 __                               
--      /   _____//  |_____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--			  T H E   W A R   B E G I N S
--	   Stratagus - A free fantasy real time strategy game engine
--
--	editor.lua	-	Editor configuration and functions.
--
--	(c) Copyright 2002-2003 by Lutz Sammer and Jimmy Salmon
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
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id$


--	Set which icons to display
SetEditorSelectIcon("icon-patrol")
SetEditorUnitsIcon("icon-assault")


--
--	editor-unit-types a sorted list of unit-types for the editor.
--	FIXME: this is only a temporary hack, for better sorted units.
--
DefineEditorUnitTypes(
   "unit-vault",
   "unit-plate1",

   "unit-apcs",
   "unit-medic",
   "unit-bazoo",
   "unit-assault",
   "unit-grenadier",
   "unit-engineer",
   "unit-harvester",
   "unit-msilo",
   "unit-dev-yard",
   "unit-gen",
   "unit-camp",
   "unit-rfac",
   "unit-hosp",
   "unit-vfac",
   "unit-gturret",
   "unit-cam",
   "unit-buggy",
   "unit-radar",

   "unit-gold-mine",
   "unit-tree",
   "unit-crystal-field1",
   "unit-crystal-field2",
   "unit-crystal-field3",
   "unit-crystal-field4",
   "unit-crystal-field5",
   "unit-crystal-field6",
   "unit-crystal-field7",
   "unit-crystal-field8",
   "unit-crystal-field9",
   "unit-crystal-field10",
   "unit-crystal-field11",   
   "unit-crystal-field12",
   "unit-crystal-field13",

   "unit-elites-start-location"
)
