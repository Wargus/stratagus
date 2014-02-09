--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
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
--	Define sounds used by game
--

DefineGameSounds(
  "placement-error", sound_click,
  "placement-success", sound_click,
  "click", sound_click,
  "chat-message", MakeSound("", "ui/chatmessage.wav"))

sound_click = MakeSound("statsthump", "ui/click.wav")

