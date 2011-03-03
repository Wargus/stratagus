--       _________ __                 __                               
--      /   _____//  |_____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--                        T H E   W A R   B E G I N S
--         Stratagus - A free fantasy real time strategy game engine
--
--      stratagus.lua - The craft configuration language.
--
--      (c) Copyright 1998-2003 by Lutz Sammer
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
--      $Id$

-- For documentation see stratagus/doc/scripts/scripts.html
print("Stratagus default config file loading ...\n")

SetGameName("stratagus")
-------------------------------------------------------------------------------
--  Config-Part
-------------------------------------------------------------------------------

--  Edit the next sections to get your look and feel.
--  Note, some of those values are overridden by user preferences,
--  see ~/.stratagus/preferences1.scripts
--  and ~/.stratagus/gamename/preferences2.scripts

--  Enter your default title screen.
--[[
SetTitleScreens(
  {Image = "ui/logo.png",
   Music = "sounds/logo.wav",
   Timeout = 3},
)
--]]

--  Enter your menu music.
--SetMenuMusic("music/default.mod")-------------------------------------------------------------------------------
--  Tables-Part
-------------------------------------------------------------------------------

if (stratagus == nil) then
  stratagus = {}
end

Load("preferences.lua")

if (stratagus.preferences == nil) then
  stratagus.preferences = {
    VideoWidth = 800,
    VideoHeight = 600,
    VideoFullScreen = true,
    PlayerName = "Player",
    ShowCommandKey = true,
    GroupKeys = "0123456789`",
    EffectsEnabled = true,
    EffectsVolume = 128,
    MusicEnabled = true,
    MusicVolume = 128,
    StratagusTranslation = "",
    GameTranslation = "",
    GrabMouse = false,
    UseOpenGL = false,
    MaxOpenGLTexture = 0,
	GameSpeed = 30
  }
end

SetUseOpenGL(stratagus.preferences.UseOpenGL)
SetVideoResolution(stratagus.preferences.VideoWidth, stratagus.preferences.VideoHeight)
SetVideoFullScreen(stratagus.preferences.VideoFullScreen)
SetMaxOpenGLTexture(stratagus.preferences.MaxOpenGLTexture)
SetLocalPlayerName(stratagus.preferences.PlayerName)
UI.ButtonPanel.ShowCommandKey = stratagus.preferences.ShowCommandKey
SetGroupKeys(stratagus.preferences.GroupKeys)
SetEffectsEnabled(stratagus.preferences.EffectsEnabled)
SetEffectsVolume(stratagus.preferences.EffectsVolume)
SetMusicEnabled(stratagus.preferences.MusicEnabled)
SetMusicVolume(stratagus.preferences.MusicVolume)
SetTranslationsFiles(stratagus.preferences.StratagusTranslation, stratagus.preferences.GameTranslation)
SetGrabMouse(stratagus.preferences.GrabMouse)
SetGameSpeed(stratagus.preferences.GameSpeed)

SavePreferences()

Load("scripts/fonts.lua")

DefineCursor({
  Name = "cursor-point",
  Race = "any",
  File = "cursors/arrow.png",
  HotSpot = {0, 0},
  Size = {14, 23}})


Widget:setGlobalFont(Fonts["stratagus-menu"])

--	MusicStopped is called if the current music is finished.
--		This is a random music player demo
function MusicStopped()
--  if (table.getn(wargus.playlist) ~= 0) then
--    PlayMusic(wargus.playlist[math.random(table.getn(wargus.playlist))])
--  end
end
  
  
print("... ready!\n")
