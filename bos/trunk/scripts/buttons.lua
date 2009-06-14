--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	buttons.lua	-	Define the general unit-buttons.
--
--	(c) Copyright 2001-2008 by Vladi Belperchinov-Shabanski, Lutz Sammer,
--				Francois Beerten and Crestez Leonard
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

-- general cancel button ------------------------------------------------------

DefineButton( { Pos = 9, Level = 9, Icon = "icon-cancel",
  Action = "cancel",
  Hint = "~<ESC~> CANCEL",
  ForUnit = {"*"} } )

DefineButton( { Pos = 9, Level = 0, Icon = "icon-cancel",
  Action = "cancel-train-unit",
  Hint = "~<ESC~> CANCEL UNIT TRAINING",
  ForUnit = {"*"} } )

DefineButton( { Pos = 9, Level = 0, Icon = "icon-cancel",
  Action = "cancel-build",
  Hint = "~<ESC~> CANCEL CONSTRUCTION",
  ForUnit = {"cancel-build"} } )

-- general commands -- almost all units have it -------------------------------

DefineButton({
	Pos = 6, Level = 0, Icon = "icon-attack-ground",
	Action = "unload", Hint = "~!UNLOAD",
	ForUnit = {"unit-apcs"}})
DefineButton({
	Pos = 1, Level = 0, Icon = "icon-move",
	Action = "move", Hint = "~!MOVE",
	ForUnit = {"unit-engineer", "unit-harvester", "unit-assault",
             "unit-grenadier", "unit-bazoo", "unit-medic",
             "unit-apcs", "unit-buggy",
             "elites-group"}})

DefineButton({
	Pos = 2, Level = 0, Icon = "icon-stop",
	Action = "stop", Hint = "~!STOP",
	ForUnit = {"unit-engineer", "unit-harvester", "unit-assault",
             "unit-grenadier", "unit-bazoo", "unit-medic",
             "unit-apcs", "unit-buggy",
             "unit-gturret", "unit-cannon",
             "elites-group"}})

DefineButton({
	Pos = 3, Level = 0, Icon = "icon-attack",
	Action = "attack", Hint = "~!ATTACK",
	ForUnit = {"unit-assault", "unit-grenadier", "unit-bazoo",
             "unit-apcs", "unit-buggy",
             "unit-gturret", "unit-cannon",
             "elites-group",}})

DefineButton({
	Pos = 4, Level = 0, Icon = "icon-patrol",
	Action = "patrol", Hint = "~!PATROL",
	ForUnit = {"unit-assault", "unit-grenadier", "unit-bazoo",
             "unit-apcs", "unit-buggy", "unit-engineer",
             "elites-group",}})

DefineButton({
	Pos = 5, Level = 0, Icon = "icon-stand-ground",
	Action = "stand-ground", Hint = "S~!TAND GROUND",
	ForUnit = {"unit-assault", "unit-grenadier", "unit-bazoo",
             "unit-apcs", "unit-buggy",
             "unit-medic", "elites-group",}})

-- resource specific actions ---------------------------------------------------

DefineButton({
	Pos = 6, Level = 0, Icon = "icon-repair",
	Action = "repair", Hint = "~!REPAIR BUILDINGS/VEHICLES",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 5, Level = 0, Icon = "icon-harvest",
	Action = "harvest", Hint = "~!HARVEST RESOURCE",
	ForUnit = {"unit-engineer"}})

-- TODO !!!!!
DefineButton({
	Pos = 5, Level = 0, Icon = "icon-harvest",
	Action = "harvest", Hint = "~!HARVEST RESOURCE",
	ForUnit = {"unit-harvester"}})

-- build basic/advanced structs -----------------------------------------------

DefineButton({
	Pos = 7, Level = 0, Icon = "icon-build-lvl1", Action = "button",
	Value = 1, Hint = "BUILD ~!BASIC STRUCTURES",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 8, Level = 0, Icon = "icon-build-lvl2", Action = "button",
	Value = 2, Hint = "BUILD ~!UNIT STRUCTURES",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 9, Level = 0, Icon = "icon-build-lvl3", Action = "button",
	Value = 3, Hint = "BUILD ~!DEFENSIVE STRUCTURES",
	ForUnit = {"unit-engineer"}})

-- simple buildings elites -----------------------------------------------------

DefineButton({
        Pos = 1, Level = 1, Icon = "icon-vault", Action = "build",
        Value = "unit-vault", Hint = "BUILD ~!VAULT",
        ForUnit = {"unit-engineer"}})
        
DefineButton({
	Pos = 9, Level = 1, Icon = "icon-cancel", Action = "button",
	Value = 0, Hint = "~<ESC~> CANCEL",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 9, Level = 2, Icon = "icon-cancel", Action = "button",
	Value = 0, Hint = "~<ESC~> CANCEL",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 9, Level = 3, Icon = "icon-cancel", Action = "button",
	Value = 0, Hint = "~<ESC~> CANCEL",
	ForUnit = {"unit-engineer"}})

-- buildings commands ---------------------------------------------------------

DefineButton({
	Pos = 1, Level = 0, Icon = "icon-engineer",
	Action = "train-unit", Value = "unit-engineer",
	Hint = "TRAIN ~!ENGINEER",
	ForUnit = {"unit-vault"}})

DefineButton({
	Pos = 5, Level = 0, Icon = "icon-engineer",
	Action = "train-unit", Value = "unit-engineer",
	Hint = "TRAIN ~!ENGINEER",
	ForUnit = {"unit-camp"}})

DefineButton({
	Pos = 1, Level = 0, Icon = "icon-assault",
	Action = "train-unit", Value = "unit-assault",
	Hint = "TRAIN ~!ASSAULT UNIT",
	ForUnit = {"unit-camp"}})

DefineButton({
	Pos = 2, Level = 0, Icon = "icon-grenadier",
	Action = "train-unit", Value = "unit-grenadier",
	Hint = "TRAIN ~!GRENADIER",
	ForUnit = {"unit-camp"}})

DefineButton({
	Pos = 3, Level = 0, Icon = "icon-bazoo",
	Action = "train-unit", Value = "unit-bazoo",
	Hint = "TRAIN ~!BAZOO",
	ForUnit = {"unit-camp"}})

DefineButton({
	Pos = 1, Level = 0, Icon = "icon-apcs",
	Action = "train-unit", Value = "unit-apcs",
	Hint = "BUILD ~!APC SMOLDER",
	ForUnit = {"unit-vfac"}})

DefineButton({
	Pos = 2, Level = 0, Icon = "icon-harvester",
	Action = "train-unit", Value = "unit-harvester",
	Hint = "BUILD MINERAL ~!HARVESTER",
	ForUnit = {"unit-vfac"}})


-- vault actions ---------------------------------------------------------

DefineButton({
	Pos = 5, Level = 0, Icon = "icon-harvest",
	Action = "harvest",
	Hint = "SET ~!HARVEST/MINE",
	ForUnit = {"unit-vault"}})

DefineButton({
	Pos = 7, Level = 0, Icon = "icon-move",	Action = "move",
	Hint = "SET ~!NEW UNITS TARGET",
	ForUnit = {"unit-vault", "unit-camp"}})

DefineButton({
	Pos = 8, Level = 0, Icon = "icon-stop", Action = "stop",
	Hint = "SET ~!CANCEL UNIT TARGET",
	ForUnit = {"unit-vault", "unit-camp"}})

DefineButton({
	Pos = 9, Level = 0, Icon = "icon-attack", Action = "attack",
	Hint = "UN~!IT ATTACK",
	ForUnit = {"unit-vault", "unit-camp"}})

-- spells ----------------------------------------------------------

DefineButton({
	Pos = 3, Level = 0, Icon = "icon-attack",
	Action = "cast-spell", Value = "spell-nuke",
	Allowed = "check-true", Hint = "~!NUKE EM",
	ForUnit = {"unit-msilo"}})

