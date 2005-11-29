--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	buttons.lua	-	Define the unit-buttons of the elites race.
--
--	(c) Copyright 2001-2003 by Vladi Belperchinov-Shabanski, Lutz Sammer,
--				François Beerten and Crestez Leonard
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

-- general commands -- almost all units have it -------------------------------

DefineButton({
	Pos = 4, Level = 0, Icon = "icon-attack-ground",
	Action = "unload", Key = "u", Hint = "~!UNLOAD",
	ForUnit = {"unit-apcs"}})
DefineButton({
	Pos = 1, Level = 0, Icon = "icon-move",
	Action = "move", Key = "m", Hint = "~!MOVE",
	ForUnit = {"unit-engineer", "unit-harvester", "unit-assault", "unit-grenadier",
		"unit-bazoo", "unit-medic", "unit-apcs", "unit-buggy", "elites-group"}})

DefineButton({
	Pos = 2, Level = 0, Icon = "icon-stop",
	Action = "stop", Key = "s", Hint = "~!STOP",
	ForUnit = {"unit-engineer", "unit-harvester", "unit-assault", "unit-grenadier",
		"unit-bazoo", "unit-medic", "unit-apcs", "unit-gturret", "elites-group",
                "unit-buggy", "unit-cannon"}})

DefineButton({
	Pos = 3, Level = 0, Icon = "icon-attack",
	Action = "attack", Key = "a", Hint = "~!ATTACK",
	ForUnit = {"unit-assault", "unit-grenadier", "unit-bazoo", "unit-apcs", "unit-gturret",
		"elites-group", "unit-buggy", "unit-cannon"}})

DefineButton({
	Pos = 4, Level = 0, Icon = "icon-patrol",
	Action = "patrol", Key = "p", Hint = "~!PATROL",
	ForUnit = {"unit-assault", "unit-grenadier", "unit-bazoo", "elites-group",
             "unit-buggy"}})

DefineButton({
	Pos = 5, Level = 0, Icon = "icon-stand-ground",
	Action = "stand-ground", Key = "t", Hint = "S~!TAND GROUND",
	ForUnit = {"unit-assault", "unit-grenadier", "unit-bazoo", "unit-apcs",
		"unit-medic", "elites-group", "unit-buggy"}})

-- resource specific actions ---------------------------------------------------

DefineButton({
	Pos = 4, Level = 0, Icon = "icon-repair",
	Action = "repair", Key = "r", Hint = "~!REPAIR BUILDINGS/VEHICLES",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 5, Level = 0, Icon = "icon-harvest",
	Action = "harvest", Key = "h", Hint = "~!MINE TITANIUM",
	ForUnit = {"unit-engineer"}})

-- TODO !!!!!
DefineButton({
	Pos = 5, Level = 0, Icon = "icon-harvest",
	Action = "harvest", Key = "h", Hint = "~!HARVEST CRYSTALS",
	ForUnit = {"unit-harvester"}})

DefineButton({
	Pos = 6, Level = 0, Icon = "icon-return-goods-peasant",
	Action = "return-goods", Key = "g", Hint = "~!GO BACK WITH RESOURCES",
	ForUnit = {"unit-engineer", "unit-harvester"}})

-- build basic/advanced structs -----------------------------------------------

DefineButton({
	Pos = 7, Level = 0, Icon = "icon-build-lvl1", Action = "button",
	Value = 1, Key = "b", Hint = "BUILD ~!BASIC STRUCTURES",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 8, Level = 0, Icon = "icon-build-lvl2", Action = "button",
	Value = 2, Key = "u", Hint = "BUILD ~!UNIT STRUCTURES",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 9, Level = 0, Icon = "icon-build-lvl3", Action = "button",
	Value = 3, Key = "d", Hint = "BUILD ~!DEFENSIVE STRUCTURES",
	ForUnit = {"unit-engineer"}})

-- simple buildings elites -----------------------------------------------------

DefineButton({
        Pos = 1, Level = 1, Icon = "icon-vault", Action = "build",
        Value = "unit-vault", Key = "v", Hint = "BUILD ~!VAULT",
        ForUnit = {"unit-engineer"}})
        
DefineButton({
	Pos = 2, Level = 1, Icon = "icon-gen_b", Action = "build",
	Value = "unit-gen", Key = "g", Hint = "BUILD ~!GENERATOR",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 1, Level = 2, Icon = "icon-camp_b", Action = "build",
	Value = "unit-camp", Key = "c", Hint = "BUILD TRAINING ~!CAMP",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 7, Level = 2, Icon = "icon-dev_b", Action = "build",
	Value = "unit-dev-yard", Key = "d", Hint = "BUILD ~!DEVELOPMENT YARD",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 6, Level = 1, Icon = "icon-rfac_b", Action = "build",
	Value = "unit-rfac", Key = "f", Hint = "BUILD RESEARCH ~!FACILITY",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 2, Level = 2, Icon = "icon-hosp_b", Action = "build",
	Value = "unit-hosp", Key = "h", Hint = "BUILD ~!HOSPITAL",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 3, Level = 2, Icon = "icon-vfac_b", Action = "build",
	Value = "unit-vfac", Key = "v", Hint = "BUILD ~!VEHICLE FACTORY",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 4, Level = 3, Icon = "icon-msilo_b", Action = "build",
	Value = "unit-msilo", Key = "m", Hint = "BUILD ~!MISSILE SILO",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 9, Level = 1, Icon = "icon-cancel", Action = "button",
	Value = 0, Key = "\027", Hint = "~<ESC~> CANCEL",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 9, Level = 2, Icon = "icon-cancel", Action = "button",
	Value = 0, Key = "\027", Hint = "~<ESC~> CANCEL",
	ForUnit = {"unit-engineer"}})

DefineButton({
	Pos = 9, Level = 3, Icon = "icon-cancel", Action = "button",
	Value = 0, Key = "\027", Hint = "~<ESC~> CANCEL",
	ForUnit = {"unit-engineer"}})
-- buildings commands ---------------------------------------------------------

DefineButton({
	Pos = 1, Level = 0, Icon = "icon-engineer",
	Action = "train-unit", Value = "unit-engineer",
	Allowed = "check-no-research", Key = "e", Hint = "TRAIN ~!ENGINEER",
	ForUnit = {"unit-vault"}})

DefineButton({
	Pos = 1, Level = 0, Icon = "icon-assault",
	Action = "train-unit", Value = "unit-assault",
	Key = "a" , Hint = "TRAIN ~!ASSAULT UNIT",
	ForUnit = {"unit-camp"}})

DefineButton({
	Pos = 2, Level = 0, Icon = "icon-grenadier",
	Action = "train-unit", Value = "unit-grenadier",
	Key = "g", Hint = "TRAIN ~!GRENADIER",
	ForUnit = {"unit-camp"}})

DefineButton({
	Pos = 3, Level = 0, Icon = "icon-bazoo",
	Action = "train-unit", Value = "unit-bazoo",
	Key = "b", Hint = "TRAIN ~!BAZOO",
	ForUnit = {"unit-camp"}})

DefineButton({
	Pos = 1, Level = 0, Icon = "icon-apcs",
	Action = "train-unit", Value = "unit-apcs",
	Key = "a", Hint = "BUILD ~!APC SMOLDER",
	ForUnit = {"unit-vfac"}})

DefineButton({
	Pos = 2, Level = 0, Icon = "icon-harvester",
	Action = "train-unit", Value = "unit-harvester",
	Key = "h", Hint = "BUILD MINERAL ~!HARVESTER",
	ForUnit = {"unit-vfac"}})


-- upgrades -------------------------------------------------------------------

DefineButton({
	Pos = 1, Level = 0, Icon = "icon-expl",
	Action = "research", Value = "upgrade-expl",
	Allowed = "check-single-research",
	Key = "e", Hint = "RESEARCH ~!EXPLOSIVES",
	ForUnit = {"unit-rfac"}})

DefineButton({
	Pos = 1, Level = 0, Icon = "icon-expl2",
	Action = "research", Value = "upgrade-expl2",
	Allowed = "check-single-research",
	Key = "e", Hint = "RESEARCH ~!EXPLOSIVES LVL 2",
	ForUnit = {"unit-rfac"}})

DefineButton({
	Pos = 2, Level = 0, Icon = "icon-tdril",
	Action = "research", Value = "upgrade-tdril",
	Allowed = "check-single-research",
	Key = "d", Hint = "RESEARCH TITANIUM ~!DRILL",
	ForUnit = {"unit-rfac"}})

DefineButton({
	Pos = 2, Level = 0, Icon = "icon-ddril",
	Action = "research", Value = "upgrade-ddril",
	Allowed = "check-single-research",
	Key = "d", Hint = "RESEARCH DIAMOND ~!DRILL",
	ForUnit = {"unit-rfac"}})

DefineButton({
	Pos = 2, Level = 0, Icon = "icon-pdril",
	Action = "research", Value = "upgrade-pdril",
	Allowed = "check-single-research",
	Key = "d", Hint = "RESEARCH POWER ~!DRILL",
	ForUnit = {"unit-rfac"}})

DefineButton({
	Pos = 9, Level = 0, Icon = "icon-void",
	Action = "research", Value = "upgrade-void",
	Allowed = "check-single-research",
	Key = "v", Hint = "RESEARCH ALIEN ~!VOID",
	ForUnit = {"unit-rfac"}})

-- town hall upgrades ---------------------------------------------------------

DefineButton({
	Pos = 5, Level = 0, Icon = "icon-harvest",
	Action = "harvest", Value = "upgrade-void",
	Key = "h", Hint = "SET ~!HARVEST/MINE",
	ForUnit = {"unit-vault"}})

DefineButton({
	Pos = 7, Level = 0, Icon = "icon-move",	Action = "move",
	Key = "n", Hint = "SET ~!NEW UNITS TARGET",
	ForUnit = {"unit-vault", "unit-camp"}})

DefineButton({
	Pos = 8, Level = 0, Icon = "icon-stop", Action = "stop",
	Key = "c", Hint = "SET ~!CANCEL UNIT TARGET",
	ForUnit = {"unit-vault", "unit-camp"}})

DefineButton({
	Pos = 9, Level = 0, Icon = "icon-attack", Action = "attack",
	Key = "i", Hint = "UN~!IT ATTACK",
	ForUnit = {"unit-vault", "unit-camp"}})

-- spells ----------------------------------------------------------

DefineButton({
	Pos = 3, Level = 0, Icon = "icon-expl2",
	Action = "cast-spell", Value = "spell-nuke",
	Allowed = "check-true", Key = "n", Hint = "~!NUKE EM",
	ForUnit = {"unit-msilo"}})

