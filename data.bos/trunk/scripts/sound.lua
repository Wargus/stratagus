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

-- BUILDINGS
MakeSound("gen-selected", "elites/buildings/sfx_pplnt.select.wav")
MakeSound("gen-ready", "elites/computer_voice/power.plant.completed.wav")
MakeSound("gen-help", "elites/computer_voice/power.plant.underattack.wav")
MakeSound("gen-dead", "elites/buildings/sfx_pplnt.die.wav")

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

MakeSound("grenade-impact", "sounds/grenadier_g_hit.wav")
MakeSound("rocket-impact", "sounds/bazoo_g_hit.wav")



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
