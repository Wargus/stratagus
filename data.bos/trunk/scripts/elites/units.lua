--       _________ __                 __
--      /   _____//  |_1____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--			  T H E   W A R   B E G I N S
--	   Stratagus - A free fantasy real time strategy game engine
--
--	units.ccl	-	Define the used elites unit-types.
--
--	(c) Copyright 2001 - 2004 by Lutz Sammer and Crestez Leonard
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
--	$Id: units.lua,v 1.19 2004/11/02 03:30:09 mr-russ Exp $

--=============================================================================
--	Define unit-types.
--
--	NOTE: Save can generate this table.
--
DefineUnitType("unit-plate1", {
	Name = "1x1 Ground Plate",
	Files = {"tileset-desert", "elites/buildings/plate.png"}, Size = {32, 32},
	Animations = "animations-plate", Icon = "icon-plate1",
	Costs = {"time", 10, "titanium", 150, "crystal", 0},
	RepairHp = 1, RepairCosts = {"titanium", 1},
	Construction = "construction-plate",
	Speed = 0, HitPoints = 100, DrawLevel = 1, TileSize = {1, 1}, BoxSize = {36, 36},
	SightRange = 0, Armor = 20, BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 20, AnnoyComputerFactor = 45, VisibleUnderFog = true,
	Points = 15, Type = "land", Building = true, BuilderOutside = true,
	Sounds = {
		"selected", "gen-selected",
		"ready", "gen-ready",
		"help", "gen-help",
		"dead", "gen-dead"}
	})

DefineUnitType("unit-apcs", {
	Name = "APC Smolder",
	Files = {"tileset-desert", "elites/units/unit_apcs.png"}, Size = {96, 96},
	Shadow = {"file", "elites/units/unit_apcs_s.png", "size", {96, 96}},
	Animations = "animations-apcs", Icon = "icon-apcs",
	Costs = {"time", 100, "titanium", 200, "crystal", 50},
	Speed = 15, HitPoints = 200, DrawLevel = 25, TileSize = {1, 1},
	BoxSize = {96, 96}, SightRange = 5,
	ComputerReactionRange = 5, PersonReactionRange = 5,
	Armor = 10, BasicDamage = 5, PiercingDamage = 5, Missile = "missile-none",
	MaxAttackRange = 4, Priority = 60, Points = 50,
	CanTransport = {"organic", "only"}, AttackFromTransporter = true, MaxOnBoard = 6,
	Type = "land", RepairHp = 2, RepairCosts = {titanium, 2},
	RightMouseAction = "attack",
	ExplodeWhenKilled = "missile-160x128-explosion",
	LandUnit = true, Demand = 0, CanAttack = true, CanTargetLand = true,
	SelectableByRectangle = true,
	Sounds = {
		"selected", "apcs-selected",
		"acknowledge", "apcs-acknowledge",
		"ready", "apcs-ready",
		"help", "apcs-help",
		"dead", "apcs-die",
		"attack", "apcs-attack"}
	})

DefineUnitType("unit-medic", {
	Name = "Medic",
	Files = {"tileset-desert", "elites/units/unit_medic.png"}, Size = {64, 64},
	Shadow = {"file", "elites/units/unit_medic_s.png", "size", {64, 64}},
	Animations = "animations-medic", Icon = "icon-medic",
	Costs = {"time", 50, "titanium", 25, "crystal", 150},
	Speed = 15, HitPoints = 100, MaxMana = 100, CanCastSpell = {SpellHealing},
	DrawLevel = 25, TileSize = {1, 1}, BoxSize = {17, 28},
	SightRange = 3, ComputerReactionRange = 6, PersonReactionRange = 6,
	Armor = 1, BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	MaxAttackRange = 1, Priority = 60, Points = 50,
	Corpse = {"unit-dead-body5", 0},
	Type = "land", CanTargetLand = true,
	LandUnit = true, Demand = 0, organic = true,
	SelectableByRectangle = true,
	CanCastSpell = {"spell-healing"},
	AutoCastActive = {"spell-healing"},
	Sounds = {
		"selected", "medic-selected",
		"acknowledge", "medic-acknowledge",
		"ready", "medic-ready",
		"help", "medic-help",
		"dead", "medic-die",
		"attack", "medic-attack"}
	})

DefineUnitType("unit-bazoo", {
	Name = "Bazoo",
	Files = {"tileset-desert", "elites/units/unit_bazoo.png"}, Size = {64, 64},
	Shadow = {"file", "elites/units/unit_bazoo_s.png", "size", {64, 64}},
	Animations = "animations-bazoo", Icon = "icon-bazoo",
	Costs = {"time", 40, "titanium", 50, "crystal", 100},
	Speed = 8, HitPoints = 50, DrawLevel = 25,
	TileSize = {1, 1}, BoxSize = {17, 28},
	SightRange = 7, ComputerReactionRange = 6, PersonReactionRange = 6,
	Armor = 2, BasicDamage = 5, PiercingDamage = 15, Missile = "missile-bazoo",
	MaxAttackRange = 6, Priority = 60, Points = 50,
	Corpse = {"unit-dead-body4", 0},
	Type = "land", Demand = 0, 
	RightMouseAction = "attack",
	CanAttack = true, CanTargetLand = true, CanTargetAir = true,
	LandUnit = true, organic = true, SelectableByRectangle = true,
	Sounds = {
		"selected", "bazoo-selected",
		"acknowledge", "bazoo-acknowledge",
		"ready", "bazoo-ready",
		"help", "bazoo-help",
		"dead", "bazoo-die",
		"attack", "bazoo-attack"}
	})

DefineUnitType("unit-assault", {
	Name = "Assault Unit",
	Files = {"tileset-desert", "elites/units/unit_assault.png"}, Size = {64, 64},
	Shadow = {"file", "elites/units/unit_assault_s.png", "size", {64, 64}},
	Animations = "animations-assault", Icon = "icon-assault",
	Costs = {"time", 30, "titanium", 25, "crystal", 50},
	Speed = 10, HitPoints = 30, DrawLevel = 25,
	TileSize = {1, 1}, BoxSize = {17, 28},
	RightMouseAction = "attack",
	SightRange = 6, ComputerReactionRange = 6, PersonReactionRange = 6,
	Armor = 3, BasicDamage = 4, PiercingDamage = 0, Missile = "missile-none",
	MaxAttackRange = 5, Priority = 60, Points = 50,
	Corpse = {"unit-dead-body1", 0}, Type = "land", Demand = 0,
	CanAttack = true, CanTargetLand = true,
	LandUnit = true, organic = true, SelectableByRectangle = true,
	Sounds = {
		"selected", "assault-selected",
		"acknowledge", "assault-acknowledge",
		"ready", "assault-ready",
		"help", "assault-help",
		"dead", "assault-die",
		"attack", "assault-attack"}
	})

DefineUnitType("unit-grenadier", {
	Name = "Grenadier",
	Files = {"tileset-desert", "elites/units/unit_grenadier.png"}, Size = {64, 64},
	Shadow  = {"file", "elites/units/unit_grenadier_s.png", "size", {64, 64}},
	Animations = "animations-grenadier", Icon = "icon-grenadier",
	Costs = {"time", 40, "titanium", 25, "crystal", 100},
	Speed = 10, HitPoints = 50, DrawLevel = 25,
	TileSize = {1, 1}, BoxSize = {17, 28}, SightRange = 6,
	ComputerReactionRange = 6, PersonReactionRange = 6,
	RightMouseAction = "attack",
	Armor = 2, BasicDamage = 15, PiercingDamage = 15, Missile = "missile-grenadier",
	MaxAttackRange = 5, Priority = 60, Points = 50, Corpse = {"unit-dead-body3", 0},
	Type = "land", CanAttack = true, CanTargetLand = true,
	LandUnit = true, organic = true, Demand = 0, SelectableByRectangle = true,
	Sounds = {
		"selected", "grenadier-selected",
		"acknowledge", "grenadier-acknowledge",
		"ready", "grenadier-ready",
		"help", "grenadier-help",
		"dead", "grenadier-die",
		"attack", "grenadier-attack"}
	})

DefineUnitType("unit-engineer", {
	Name = "Engineer",
	Files = {"tileset-desert", "elites/units/unit_engineer.png"}, Size = {64, 64},
	Shadow = {"file", "elites/units/unit_engineer_s.png", "size", {64, 64}},
	DrawLevel = 19, Animations = "animations-engineer", Icon = "icon-engineer",
	Costs = {"time", 50, "titanium", 50, "crystal", 100},
	Speed = 8, HitPoints = 30, DrawLevel = 25,
	TileSize = {1, 1}, BoxSize = {17, 28},
	SightRange = 5, ComputerReactionRange = 6, PersonReactionRange = 4,
	Armor = 1, BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	MaxAttackRange = 1, Priority = 50, Points = 30, Corpse = {"unit-dead-body2", 0},
	Type = "land", Demand = 0, RightMouseAction = "harvest", RepairRange = 1,
	CanTargetLand = true, LandUnit = true, Coward = true,
        CanCastSpell={"spell-minefield"},
	CanGatherResources = {{
		"file-when-loaded", "elites/units/unit_engineer.png",
		"resource-id", "titanium",
		"resource-capacity", 50,
		"wait-at-resource", 7,
		"wait-at-depot", 1,
		"resource-step", 1,
		"harvest-from-outside"}, 
		{"file-when-loaded", "elites/units/unit_engineer.png",
                "resource-id", "crystal",
                "resource-capacity", 50,
                "wait-at-resource", 7,
                "wait-at-depot", 1,
                "resource-step", 1,
                "harvest-from-outside"}},
	organic = true, SelectableByRectangle = true,
	Sounds = {
		"selected", "engineer-selected",
		"acknowledge", "engineer-acknowledge",
		"ready", "engineer-ready",
		"repair", "engineer-repair",
		"harvest", "titanium", "engineer-harvest",
		"help", "engineer-help",
		"dead", "engineer-die"}
	})

DefineUnitType("unit-harvester", {
	Name = "Harvester",
	Files = {"tileset-desert", "elites/units/unit_harv.png"}, Size = {96, 96},
	Shadow = {"file", "elites/units/unit_harv_s.png", "size", {96, 96}},
	DrawLevel = 25, Animations = "animations-harvester", Icon = "icon-harvester",
	Costs = {'time', 75, 'titanium', 250, 'crystal', 100},
	RepairHp = 2, RepairCosts = {"titanium", 2},
	ExplodeWhenKilled = "missile-160x128-explosion",
	Speed = 10, HitPoints = 200, DrawLevel = 40, TileSize = {1, 1}, BoxSize = {63, 63},
	SightRange = 5, ComputerReactionRange = 6, PersonReactionRange = 4,
	Armor = 25, BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	MaxAttackRange = 0, Priority = 50, Points = 30, Type = "land",
	Demand = 50, RightMouseAction = "harvest", CanAttack = true, CanTargetLand = true,
	LandUnit = true, Coward = true,
	CanGatherResources = {{
                "file-when-loaded", "elites/units/unit_harv.png",
                "resource-id", "titanium",
                "resource-capacity", 100,
                "wait-at-resource", 2,
                "wait-at-depot", 1,
                "resource-step", 2,
                "harvest-from-outside"}, 
		{"file-when-loaded", "elites/units/unit_harv.png",
		"resource-id", "crystal",
		"resource-capacity", 100,
		"wait-at-resource", 2,
		"wait-at-depot", 1,
		"resource-step", 2,
		"harvest-from-outside"}},
	SelectableByRectangle = true,
	Sounds = {
		"selected", "harvester-selected",
		"acknowledge", "harvester-acknowledge",
		"ready", "harvester-ready",
		"harvest", "crystal", "harvester-harvest",
		"harvest", "titanium", "harvester-harvest",
		"help", "harvester-help",
		"dead", "harvester-die"}
	})

DefineUnitType("unit-dorcoz", {
	Name = "Dorcoz",
	Files = {"tileset-desert", "elites/units/unit_dorcoz.png"}, Size = {64, 64},
	Animations = "animations-dorcoz", Icon = "icon-dorcoz",
	Costs = {"time", 100, "titanium", 50, "crystal", 300}, Speed = 10,
	HitPoints = 100, DrawLevel = 40, TileSize = {1, 1}, BoxSize = {31, 31},
	SightRange = 7, ComputerReactionRange = 7, PersonReactionRange = 7,
	Armor = 2, BasicDamage = 6, PiercingDamage = 3, Missile = "missile-dorcoz",
	MaxAttackRange = 7, Priority = 60, Points = 50, Demand = 1,
	Corpse = {"unit-dead-body", 0}, Type = "land", RightMouseAction = "attack",
	CanAttack = true, CanTargetLand = true, LandUnit = true, organic = true,
	SelectableByRectangle = true, DetectCloak = true,
	RightMouseAction = "attack",
	Sounds = {
		"selected", "dorcoz-selected",
		"acknowledge", "dorcoz-acknowledge",
		"ready", "dorcoz-ready",
		"help", "basic terras voices help 1",
		"dead", "basic terras voices dead",
		"attack", "dorcoz-attack"}
	})

DefineUnitType("unit-msilo", {
	Name = "Missile Silo",
	Files = {"tileset-desert", "elites/build/missile_silo.png"}, Size = {128, 128},
	Shadow = {"file", "elites/build/missile_silo_s.png", "size", {128, 128}},
	Animations = "animations-msilo", Icon = "icon-msilo",
	Costs = {"time", 2000, "titanium", 10000, "crystal", 10000},
	RepairHp = 2, RepairCosts = {"titanium", 2},
	Construction = "construction-msilo",
	Speed = 0, HitPoints = 450, DrawLevel = 25, 
	TileSize = {4, 4}, BoxSize = {124, 124},
	SightRange = 1, Armor = 10, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 20, AnnoyComputerFactor = 45,
	Points = 100, ExplodeWhenKilled = "missile-160x128-explosion",
	Corpse = {"build-dead-body4", 0}, Type = land,
	MaxMana = 1000, CanCastSpell = {"spell-nuke"},
	Demand = 400, Building = true, BuilderOutside = true,
	VisibleUnderFog = true,
	Sounds = {
		"selected", "gen-selected",
		"ready", "gen-ready",
		"help", "gen-help",
		"dead", "gen-dead"}
	})

DefineUnitType("unit-gen", {
	Name = "Generator",
	Files = {"tileset-desert", "elites/build/generator.png"}, Size = {64, 64},
	Shadow = {"file", "elites/build/generator_s.png", "size", {64, 64}},
	Animations = "animations-gen", Icon = "icon-gen",
	Costs = {"time", 75, "titanium", 250, "crystal", 50},
	RepairHp = 2, RepairCosts = {"titanium", 2}, Construction = "construction-gen",
	Speed = 0, HitPoints = 250, DrawLevel = 25, TileSize  = {2, 2}, BoxSize = {60, 60},
	SightRange = 1, Armor = 10 , BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 20, AnnoyComputerFactor = 45,
	Points = 100, Supply = 125, ExplodeWhenKilled = "missile-160x128-explosion",
	Corpse = {"build-dead-body2", 0}, Type = "land",
	--[[MustBuildOnTop = "unit-plate1", --]] Building = true, BuilderOutside = true,
	VisibleUnderFog = true,
	Sounds = {
		"selected", "gen-selected",
		"ready", "gen-ready",
		"help", "gen-help",
		"dead", "gen-dead"}
	})

DefineUnitType("unit-camp", {
	Name = "Training Camp",
	Files = {"tileset-desert", "elites/build/training_camp.png"}, Size = {160, 128},
	Shadow = {"file", "elites/build/training_camp_s.png", "size", {160, 128}},
	Animations = "animations-camp", Icon = "icon-camp",
	Costs = {"time", 100, "titanium", 300, "crystal", 150},
	RepairHp = 2, RepairCosts = {"titanium", 2},
	Construction = "construction-camp", Speed = 0, HitPoints = 500,
	DrawLevel = 25, Demand = 125, TileSize = {5, 4}, BoxSize = {156, 124},
	SightRange = 1, Armor = 25, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 30, AnnoyComputerFactor = 35,
	Points = 160, ExplodeWhenKilled = "missile-160x128-explosion",
	Corpse = {"build-dead-body3", 0}, Type = "land",
	Building = true, BuilderOutside = true, VisibleUnderFog = true,
	Sounds = {
		"selected", "camp-selected",
		"ready", "camp-ready",
		"help", "camp-help",
		"dead", "camp-dead"}
	})

DefineUnitType("unit-vault", {
	Name = "Vault",
	Files = {"tileset-desert", "elites/buildings/vault.png"}, Size = {160, 220},
	Animations = "animations-vault", Icon = "icon-vault",
	Costs = {"time", 150, "titanium", 1000, "crystal", 1000},
	RepairHp = 4, RepairCosts = {"titanium", 4}, Construction = "construction-vault",
	Speed = 0, HitPoints = 1800, DrawLevel = 25, TileSize = {5, 4}, BoxSize = {164, 132},
	SightRange = 4, Armor = 30, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 35, AnnoyComputerFactor = 45,
	Points = 200, Supply = 200, ExplodeWhenKilled = "missile-288x288-explosion",
	Corpse = {"build-dead-body3", 0}, Type = "land",
	Building = true, BuilderOutside = true, VisibleUnderFog = true,
	CanStore = {"crystal", "titanium"},
	Sounds = {
		"selected", "dev-selected",
		"ready", "dev-ready",
		"help", "dev-help",
		"dead", "dev-dead"}
	})

DefineUnitType("unit-dev-yard", {
	Name = "Development yard",
	Files = {"tileset-desert", "elites/build/development_yard.png"}, Size = {224, 196},
	Animations = "animations-dev-yard", Icon = "icon-dev",
	Costs = {"time", 150, "titanium", 300, "crystal", 300},
	RepairHp = 4, RepairCosts = {"titanium", 4}, Construction = "construction-dev-yard",
	Speed = 0, HitPoints = 1800, DrawLevel = 25, TileSize = {7, 6}, BoxSize = {224, 196},
	SightRange = 4, Armor = 30, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 35, AnnoyComputerFactor = 45,
	Points = 200, Supply = 200, ExplodeWhenKilled = "missile-288x288-explosion",
	Corpse = {"build-dead-body1", 0}, Type = "land",
	VisibleUnderFog = true,	Building = true, BuilderOutside = true,
	Sounds = {
		"selected", "dev-selected",
		"ready", "dev-ready",
		"help", "dev-help",
		"dead", "dev-dead"}
	})

DefineUnitType("unit-rfac", {
	Name = "Research Facility",
	Files = {"tileset-desert", "elites/build/research_facility.png"}, Size = {128, 128},
	Shadow = {"file", "elites/build/research_facility_s.png", "size", {128, 128}},
	Animations = "animations-rfac", Icon = "icon-rfac",
	Costs = {"time", 125, "titanium", 300, "crystal", 300},
	RepairHp = 2, RepairCosts = {"titanium", 2}, Construction = "construction-rfac",
	Speed = 0, HitPoints = 350, DrawLevel = 25, TileSize = {4, 4}, BoxSize = {124, 124},
	SightRange = 1, Armor = 30, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 35, AnnoyComputerFactor = 45,
	Demand = 300, Points = 200, ExplodeWhenKilled = "missile-160x128-explosion",
	Corpse = {"build-dead-body4", 0}, Type = "land",
	Building = true, BuilderOutside = true, VisibleUnderFog = true,
	Sounds = {
		"selected", "rfac-selected",
		"ready", "rfac-ready",
		"help", "rfac-help",
		"dead", "rfac-dead"}
	})

DefineUnitType("unit-hosp", {
	Name = "Hospital",
	Files = {"tileset-desert", "elites/build/hospital.png"}, Size = {128, 96},
	Shadow = {"file", "elites/build/hospital_s.png", "size", {128, 96}},
	Animations = "animations-hosp", Icon = "icon-hosp",
	Costs = {"time", 125, "titanium", 500, "crystal", 100},
	RepairHp = 2, RepairCosts = {"titanium", 2}, Construction = "construction-hosp",
	Speed = 0, HitPoints = 350, DrawLevel = 25, TileSize = {4, 3},
	BoxSize = {124, 92}, SightRange = 2, Armor = 30, BasicDamage = 0,
	PiercingDamage = 0, Missile = "missile-none", Priority = 35,
	AnnoyComputerFactor = 45, Demand = 200, Points = 200,
	ExplodeWhenKilled = "missile-160x128-explosion", Corpse = {"build-dead-body5", 0},
	Type = "land", Building = true, BuilderOutside = true, VisibleUnderFog = true,
	Sounds = {
		"selected", "hosp-selected",
		"ready", "hosp-ready",
		"help", "hosp-help",
		"dead", "hosp-dead"}
	})

DefineUnitType("unit-vfac", {
	Name = "Vehicle Factory",
	Files = {"tileset-desert", "elites/build/vehicle_factory.png"}, Size = {224, 160},
	Shadow = {"file", "elites/build/vehicle_factory_s.png", "size", {224, 160}},
	Animations = "animations-vfac", Icon = "icon-vfac",
	Costs = {"time", 200, "titanium", 750, "crystal", 100},
	RepairHp = 2, RepairCosts = {"titanium", 2}, Construction = "construction-vfac",
	Speed = 0, HitPoints = 550, DrawLevel = 25, TileSize = {7, 5},
	BoxSize = {220, 156}, SightRange = 2, Armor = 30, BasicDamage = 0,
	PiercingDamage = 0, Missile = "missile-none", Priority = 35,
	AnnoyComputerFactor = 45, Demand = 400, Points = 200,
	ExplodeWhenKilled = "missile-160x128-explosion", Corpse = {"build-dead-body6", 0},
	Type = "land",  Building = true, BuilderOutside = true, VisibleUnderFog = true,
	Sounds = {
		"selected", "vfac-selected",
		"ready", "vfac-ready",
		"help", "vfac-help",
		"dead", "vfac-dead"}
	})

CorpseTable = {"assault", "engineer", "grenadier", "bazoo", "medic"}
for i = 1, table.getn(CorpseTable) do
	DefineUnitType("unit-dead-body" .. i, {
		Name = CorpseTable[i] .. "body",
		Files = {"tileset-desert", "elites/units/unit_" .. CorpseTable[i] .. "_c.png"},
		Size = {64, 64},
		Animations = "animations-elitecorpse1", Icon = "icon-cancel",
		Speed = 0, HitPoints = 999, DrawLevel = 10, TileSize = {1, 1},
		BoxSize = {31, 31}, SightRange = 1, BasicDamage = 0,
		PiercingDamage = 0, Missile = "missile-none",
		Priority = 0, Type = "land", Vanishes = true})
end

DefineUnitType("build-dead-body1", {
	Name = "DevelopmentyardCrater",
	Files = {"tileset-desert", "elites/build/development_yard.png"}, Size = {224, 196},
	Animations = "animations-elitebuild1", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {7, 6}, BoxSize = {220, 192},
	SightRange = 1, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 0, Type = "land",
	Building = true, Vanishes = true, Sounds = {}})

DefineUnitType("build-dead-body2", {
	Name = "GeneratorCrater",
	Files = {"tileset-desert", "elites/build/generator.png"}, Size = {64, 64},
	Animations = "animations-elitebuild2", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {2, 2}, BoxSize = {60, 60}, SightRange = 1,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})

DefineUnitType("build-dead-body3", {
	Name = "CampCrater",
	Files = {"tileset-desert", "elites/build/training_camp.png"}, Size = {160, 128},
	Animations = "animations-elitebuild3", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {5, 4}, BoxSize = {156, 124}, SightRange = 1,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})

DefineUnitType("build-dead-body4", {
	Name = "RfacCrater",
	Files = {"tileset-desert", "elites/build/research_facility.png"}, Size = {128, 128},
	Animations = "animations-elitebuild4", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,
	TileSize = {4, 4}, BoxSize = {124, 124}, SightRange = 1,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})

DefineUnitType("build-dead-body5", {
	Name = "HospCrater",
	Files = {"tileset-desert", "elites/build/hospital.png"}, Size = {128, 96},
	Animations = "animations-elitebuild5", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10, TileSize = {4, 3},
	BoxSize = {124, 92}, SightRange = 1, BasicDamage = 0,
	PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})

DefineUnitType("build-dead-body6", {
	Name = "FactoryCrater",
	Files = {"tileset-desert", "elites/build/vehicle_factory.png"}, Size = {224, 160},
	Animations = "animations-elitebuild6", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10, TileSize = {7, 5},
	BoxSize = {220, 156}, SightRange = 1, BasicDamage = 0,
	PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Building = true, Vanishes = true
	})

DefineUnitType("build-dead-body7", {
	Name = "SiloCrater",
	Files = {"tileset-desert", "elites/build/missile_silo.png"}, Size = {128, 128},
	Animations = "animations-elitebuild7", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10,	TileSize = {4, 4},
	BoxSize = {124, 124}, SightRange = 1, BasicDamage = 0,
	PiercingDamage = 0, Missile = "missile-none", Priority = 0,
	Type = "land" , Building = true, Vanishes = true})

DefineUnitType("unit-elites-start-location", {
	Name = "Start Location",
	Files = {"tileset-desert", "elites/x_startpoint.png"}, Size = {32, 32},
	Animations = "animations-building", Icon = "icon-cancel",
	Speed = 0, HitPoints = 0, DrawLevel = 0, TileSize = {1, 1},
	BoxSize = {31, 31}, SightRange = 0, BasicDamage = 0, PiercingDamage = 0,
	Missile = "missile-none", Priority = 0, Demand = 0, Type = "land",
	Sounds = {
		"selected", "elites-start-location-selected",
		"acknowledge", "elites-start-location-acknowledge",
		"ready", "elites-start-location-ready",
		"help", "basic elites voices help 2",
		"dead", "building destroyed",
		"attack", "elites-start-location-attack"}
	})

