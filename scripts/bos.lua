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
--	bos.lua		-	game specific stuff, and wc2 format compatibility
--
--	(c) Copyright 2001-2003 by Crestez Leonard
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

DefineRaceNames(
	"race", {
		"race", 0,
		"name", "elites",
		"display", "Elites",
		"visible"},
	"race", {
		"race", 1,
		"name", "neutral",
		"display", "Neutral"}
)

DefineTilesetWcNames("tileset-desert", "tileset-winter")

DefineConstructionWcNames(
	"construction-none", "construction-land", "construction-wall",
	"construction-land2", "construction-plate", "construction-msilo",
	"construction-gen", "construction-vault", "construction-camp",
	"construction-rfac", "construction-hosp", "construction-vfac", "construction-gturret")

DefineUnitTypeWcNames(
	"unit-assault", "unit-grunt", "unit-engineer", "unit-peon", "unit-ballista",
	"unit-catapult", "unit-knight", "unit-ogre", "unit-archer", "unit-axethrower",
	"unit-mage", "unit-death-knight", "unit-paladin", "unit-dev-yard",
	"unit-dwarves", "unit-goblin-sappers", "unit-engineer",
	"unit-peon", "unit-ranger", "unit-berserker", "unit-female-hero",
	"unit-evil-knight", "unit-flying-angle", "unit-fad-man",
	"unit-white-mage", "unit-beast-cry", "unit-elites-oil-tanker",
	"unit-terras-oil-tanker", "unit-elites-transport", "unit-terras-transport",
	"unit-elites-destroyer", "unit-terras-destroyer", "unit-battleship",
	"unit-ogre-juggernaught", "unit-nothing-22", "unit-fire-breeze",
	"unit-nothing-24", "unit-nothing-25", "unit-elites-submarine",
	"unit-terras-submarine", "unit-balloon", "unit-zeppelin",
	"unit-gryphon-rider", "unit-dragon", "unit-knight-rider", "unit-eye-of-vision",
	"unit-arthor-literios", "unit-quick-blade", "unit-knight-rider", "unit-double-head",
	"unit-wise-man", "unit-ice-bringer", "unit-man-of-light", "unit-sharp-axe",
	"unit-skeleton", "unit-skeleton", "unit-daemon", "unit-critter",
	"unit-gen", "unit-pig-farm", "unit-camp", "unit-terras-barracks",
	"unit-church", "unit-altar-of-storms", "unit-elites-watch-tower",
	"unit-terras-watch-tower", "unit-stables", "unit-ogre-mound",
	"unit-inventor", "unit-alchemist", "unit-gryphon-aviary",
	"unit-dragon-roost", "unit-elites-shipyard", "unit-terras-shipyard",
	"unit-vault", "unit-great-hall", "unit-elven-lumber-mill",
	"unit-troll-lumber-mill", "unit-elites-foundry", "unit-terras-foundry",
	"unit-mage-tower", "unit-temple-of-the-damned", "unit-elites-blacksmith",
	"unit-terras-blacksmith", "unit-elites-refinery", "unit-terras-refinery",
	"unit-elites-oil-platform", "unit-terras-oil-platform", "unit-keep",
	"unit-stronghold", "unit-castle", "unit-fortress", "unit-gold-mine",
	"unit-oil-patch", "unit-elites-start-location", "unit-terras-start-location",
	"unit-elites-guard-tower", "unit-terras-guard-tower", "unit-elites-cannon-tower",
	"unit-terras-cannon-tower", "unit-circle-of-power", "unit-dark-portal",
	"unit-runestone", "unit-elites-wall", "unit-terras-wall", "unit-dead-body",
	"unit-destroyed-1x1-place", "unit-destroyed-2x2-place",
	"unit-destroyed-3x3-place", "unit-destroyed-4x4-place",
	"unit-engineer", "unit-peon", "unit-engineer", "unit-peon", "unit-elites-oil-tanker", "unit-terras-oil-tanker",
	"unit-harvester",
	"unit-crystal-field1",
	"unit-rfac",
	"unit-grenadier",
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
	"unit-bazoo",
	"unit-medic",
	"unit-hosp",
	"unit-vfac",
	"unit-apcs",
	"unit-msilo",
	"unit-gturret",
	"unit-cam",
	"unit-plate1",
	"unit-radar",
	"unit-tree",
	"unit-rock-1",
	"unit-rock-2",
	"unit-rock-3",
	"unit-rock-4",
	"unit-rock-5",
	"unit-rock-6")
	

DefineIconWcNames(
	"icon-assault", "icon-apcs", "icon-grenadier", "icon-bazoo",
	"icon-medic", "icon-engineer", "icon-harvester", "icon-gen",
	"icon-gen_b", "icon-camp", "icon-camp_b", "icon-vault", "icon-dev_b",
	"icon-rfac", "icon-rfac_b", "icon-hosp", "icon-hosp_b", "icon-vfac", "icon-gturret",
	"icon-vfac_b", "icon-msilo", "icon-msilo_b", "icon-plate1", "icon-expl",
	"icon-expl2", "icon-tdril", "icon-ddril", "icon-pdril", "icon-void",
	"icon-heal", "icon-build-lvl1", "icon-build-lvl2", "icon-build-lvl3",
	"icon-dorcoz", "icon-crystal-field", "icon-rocks_field", "icon-cancel",
	"icon-stop", "icon-move", "icon-attack", "icon-patrol", "icon-attack-ground",
	"icon-repair", "icon-harvest", "icon-build-advanced",
	"icon-return-goods-peasant", "icon-return-goods-peon", "icon-stand-ground")

DefineUpgradeWcNames(
	"upgrade-expl", "upgrade-expl2", "upgrade-tdril",
	"upgrade-ddril", "upgrade-pdril", "upgrade-void")

DefineAiWcNames("ai-rush", "ai-passive")

SetColorWaterCycleStart(254)
SetColorWaterCycleEnd(254)
SetColorIconCycleStart(254)
SetColorIconCycleEnd(254)
SetColorBuildingCycleStart(254)
SetColorBuildingCycleEnd(254)
