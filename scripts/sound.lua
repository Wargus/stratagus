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
--	sound.lua	-	Define the used sounds.
--
--	(c) Copyright 1999-2004 by Fabrice Rossi, Lutz Sammer and Crestez Leonard
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

------------------------------------------------------------------------------
--	Music part

------------------------------------------------------------------------------
--	MusicStopped() is called if the current music is finished.
--
--		This is a random music player demo
--

function MusicStopped()
  if (table.getn(playlist) ~= 0) then
    PlayMusic(playlist[math.random(table.getn(playlist))])
  end
end

-- No cd music
SetCdMode("off")

------------------------------------------------------------------------------
--	Define sounds later used
--

sound_click = MakeSound("click", "ui/click.wav")

------------------------------------------------------------------------------
--	ELITES
--

-- UNITS
MakeSound("grenadier-selected", "elites/units/grenadier_select.wav")
MakeSound("grenadier-acknowledge", "elites/units/grenadier_action.wav")
MakeSound("grenadier-ready", "elites/computer_voice/grenadier.ready.wav")
MakeSound("grenadier-help", "elites/computer_voice/grenadier.underattack.wav")
MakeSound("grenadier-die", "elites/units/grenadier_die.wav")
MakeSound("grenadier-attack", "elites/units/grenadier_attack.wav")

MakeSound("engineer-selected", "elites/units/engineer_select.wav")
MakeSound("engineer-acknowledge", "elites/units/engineer_action.wav")
MakeSound("engineer-ready", "elites/computer_voice/engineer.ready.wav")
MakeSound("engineer-help", "elites/computer_voice/engineer.underattack.wav")
MakeSound("engineer-die", "elites/units/engineer_die.wav")
MakeSound("engineer-repair", "elites/units/engineer_attack.wav")
MakeSound("engineer-harvest", "elites/units/engineer_attack.wav")

MakeSound("harvester-selected", "elites/units/harvester_select.wav")
MakeSound("harvester-acknowledge", "elites/units/harvester_action.wav")
MakeSound("harvester-ready", "elites/computer_voice/harvester.completed.wav")
MakeSound("harvester-help", "elites/computer_voice/harvester.underattack.wav")
MakeSound("harvester-die", "elites/units/harvester_die.wav")
MakeSound("harvester-harvest", "elites/units/harvester_attack.wav")

MakeSound("apcs-selected", "elites/units/smolder_select.wav")
MakeSound("apcs-acknowledge", "elites/units/smolder_action.wav")
MakeSound("apcs-ready", "elites/computer_voice/smolder.completed.wav")
MakeSound("apcs-help", "elites/computer_voice/smolder.underattack.wav")
MakeSound("apcs-die", "elites/units/smolder_die.wav")
MakeSound("apcs-attack", "elites/units/smolder_attack.wav")

-- BUILDINGS
MakeSound("gen-selected", "elites/buildings/sfx_pplnt.select.wav")
MakeSound("gen-ready", "elites/computer_voice/power.plant.completed.wav")
MakeSound("gen-help", "elites/computer_voice/power.plant.underattack.wav")
MakeSound("gen-dead", "elites/buildings/sfx_pplnt.die.wav")

MakeSound("camp-selected", "elites/buildings/sfx_camp.select.wav")
MakeSound("camp-ready", "elites/computer_voice/training.camp.completed.wav")
MakeSound("camp-help", "elites/computer_voice/training.camp.underattack.wav")
MakeSound("camp-dead", "elites/buildings/sfx_camp.die.wav")

MakeSound("dev-selected", "elites/buildings/sfx_fort.select.wav")
MakeSound("dev-ready", "elites/computer_voice/elite.fort.completed.wav")
MakeSound("dev-help", "elites/computer_voice/elite.fort.underattack.wav")
MakeSound("dev-dead", "elites/buildings/sfx_fort.die.wav")

MakeSound("rfac-selected", "elites/buildings/sfx_rfac.select.wav")
MakeSound("rfac-ready", "elites/computer_voice/research.facility.completed.wav")
MakeSound("rfac-help", "elites/computer_voice/research.facility.underattack.wav")
MakeSound("rfac-dead", "elites/buildings/sfx_rfac.die.wav")

MakeSound("hosp-selected", "elites/buildings/sfx_hosp.select.wav")
MakeSound("hosp-ready", "elites/computer_voice/hospital.completed.wav")
MakeSound("hosp-help", "elites/computer_voice/hospital.underattack.wav")
MakeSound("hosp-dead", "elites/buildings/sfx_hosp.die.wav")

MakeSound("vfac-selected", "elites/buildings/sfx_vfac.select.wav")
MakeSound("vfac-ready", "elites/computer_voice/vehicle.factory.completed.wav")
MakeSound("vfac-help", "elites/computer_voice/vehicle.factory.underattack.wav")
MakeSound("vfac-dead", "elites/buildings/sfx_vfac.die.wav")

MakeSound("grenade-impact", "elites/units/grenadier_g_hit.wav")
MakeSound("rocket-impact", "elites/units/booza_g_hit.wav")



------------------------------------------------------------------------------
--	TERRAS
--

-- UNITS

-- BUILDINGS





------------------------------------------------------------------------------
--	Define sounds used by game
--

DefineGameSounds(
  "placement-error", sound_click,
  "placement-success", sound_click,
  "click", sound_click)

sound_click = MakeSound("statsthump", "ui/click.wav")
