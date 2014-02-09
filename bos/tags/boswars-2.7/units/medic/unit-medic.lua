--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      unit-medic.lua - Define the used elites unit-types.
--
--      (c) Copyright 2001-2007 by Francois Beerten, Lutz Sammer, and
--                                 Crestez Leonard
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


DefineAnimations("animations-medic", {
  Still = {"frame 0", "wait 1", },
  Move = {"unbreakable begin", "frame 5", "move 2", "wait 2",
    "frame 5", "move 2", "wait 2", "frame 5", "move 2", "wait 2",
    "frame 5", "move 2", "wait 2", "frame 10", "move 2", "wait 2",
    "frame 10", "move 2", "wait 2", "frame 10", "move 2", "wait 2",
    "frame 10", "move 2", "wait 2", "frame 15", "move 2", "wait 2",
    "frame 15", "move 2", "wait 2", "frame 15", "move 2", "wait 2",
    "frame 15", "move 2", "wait 2", "frame 20", "move 2", "wait 2",
    "frame 20", "move 2", "wait 2", "frame 20", "move 2", "wait 2",
    "frame 20", "move 2", "unbreakable end", "wait 2", },
  Attack = {"unbreakable begin", "frame 25", "wait 4",
    "frame 30", "sound medic-attack", "attack", "wait 4",
    "frame 0", "unbreakable end", "wait 1", },
  Death = {"unbreakable begin", "frame 40", "wait 2", "frame 45",
    "wait 2", "frame 50", "wait 2", "frame 55", "unbreakable end", "wait 2", },
})

DefineIcon({
  Name = "icon-medic",
  Size = {46, 38},
  Frame = 0,
  File = "units/medic/ico_medic.png"})
DefineIcon({
  Name = "icon-heal",
  Size = {46, 38},
  Frame = 0,
  File = "units/medic/ico_heal.png"})

MakeSound("medic-selected", "units/medic/medic_select.wav")
MakeSound("medic-acknowledge", "units/medic/medic_action.wav")
MakeSound("medic-ready", "units/medic/medic.ready.wav")
MakeSound("medic-help", "units/medic/medic.underattack.wav")
MakeSound("medic-die", "units/medic/medic_die.wav")
MakeSound("medic-attack", "units/medic/medic_attack.wav")

DefineMissileType("missile-heal", {
  File = "units/medic/heal.png",
  Size = {32, 32}, Frames = 5, NumDirections = 1, DrawLevel = 50,
  Class = "missile-class-stay", Sleep = 2, Speed = 32, Range = 15})

DefineSpell("spell-healing",
  "showname", "Bandage",
  "manacost", 3,
  "range",  1,
  "target", "unit",
  "action", {
    {"spawn-missile", "missile", "missile-heal",
     "start-point", {"base", "target"}},
    {"adjust-vitals", "hit-points", 1}
  },
  "condition", {
    "Organic", "only",
    "Building", "false",
    "self", "false",
    "HitPoints", {MaxValuePercent = 100}
  },
  "sound-when-cast", "medic-attack",
  "autocast", {
    "range", 6,
    "condition", {
      "alliance", "only",
      "HitPoints", {MaxValuePercent = 90}
    }
  }
)

DefineUnitType("unit-medic", {
  Name = "Medic",
  Image = {"file", "units/medic/unit_medic.png", "size", {64, 64}},
  Shadow = {"file", "units/medic/unit_medic_s.png", "size", {64, 64}},
  Animations = "animations-medic",
  Icon = "icon-medic",
  Flip = true,
  EnergyValue = 2500,
  MagmaValue = 2000,
  HitPoints = 25,
  MaxMana = 100,
  CanCastSpell = {"spell-healing"},
  DrawLevel = 25,
  TileSize = {1, 1},
  BoxSize = {17, 28},
  SightRange = 3,
  ComputerReactionRange = 6,
  PersonReactionRange = 6,
  Armor = 1,
  BasicDamage = 0,
  PiercingDamage = 0,
  Missile = "missile-none",
  MaxAttackRange = 1,
  Priority = 60,
  Points = 50,
  Corpse = "unit-dead-body5",
  Type = "land",
  CanTargetLand = true,
  Organic = true,
  SelectableByRectangle = true,
  CanCastSpell = {"spell-healing"},
  AutoCastActive = {"spell-healing"},
  Sounds = {
    "selected", "medic-selected",
    "acknowledge", "medic-acknowledge",
    "ready", "medic-ready",
    "help", "medic-help",
    "dead", "medic-die"}
})

DefineHumanCorpse("medic", nil, true)

DefineAllow("unit-medic", AllowAll)

DefineButton({
  Pos = 1, Level = 0, Icon = "icon-medic",
  Action = "train-unit", Value = "unit-medic",
  Hint = "TRAIN ~!MEDIC",
  ForUnit = {"unit-hosp"}})

DefineButton({
  Pos = 3, Level = 0, Icon = "icon-heal",
  Action = "cast-spell", Value = "spell-healing",
  Allowed = "check-true", Hint = "~!BANDAGE",
  ForUnit = {"unit-medic"}})
