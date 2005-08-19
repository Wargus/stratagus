--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
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
--      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

-- MISSILE
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
