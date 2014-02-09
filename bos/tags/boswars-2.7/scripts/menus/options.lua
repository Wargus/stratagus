--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      options.lua - The option menus
--
--      (c) Copyright 2005-2010 by Francois Beerten
--
--      This program is free software; you can redistribute it and/or modify
--      it under the terms of the GNU General Public License as published by
--      the Free Software Foundation; only version 2 of the License.
--
--      This program is distributed in the hope that it will be useful,
--      but WITHOUT ANY WARRANTY; without even the implied warranty of
--      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--      GNU General Public License for more details.
--
--      You should have received a copy of the GNU General Public License
--      along with this program; if not, write to the Free Software
--      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
--      02111-1307, USA.
--


function RunSpeedOptionsMenu(s)
  local menu
  local b
  local offx = (Video.Width - 256) / 2
  local offy = (Video.Height - 288) / 2
  local speed

  menu = BosMenu(_("Speed Options"))

  b = Label(_("Game Speed"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 16, offy + 36 * 1)

  local gamespeedslider = Slider(15, 75)
  gamespeedslider:setValue(GetGameSpeed())
  gamespeedslider:setActionCallback(function() SetGameSpeed(gamespeedslider:getValue()) end)
  gamespeedslider:setWidth(198)
  gamespeedslider:setHeight(18)
  gamespeedslider:setBaseColor(dark)
  gamespeedslider:setForegroundColor(clear)
  gamespeedslider:setBackgroundColor(clear)
  menu:add(gamespeedslider, offx + 32, offy + 36 * 1.5)

  b = Label(_("slow"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 32, offy + 36 * 2 + 6)
  
  b = Label(_("fast"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 230 - b:getWidth(), offy + 36 * 2 + 6)

  menu:addButton(_("~!OK"), offx + 128 - (200 / 2), offy + 245,
    function()
      preferences.GameSpeed = GetGameSpeed()
      SavePreferences()
      menu:stop()
    end)

  menu:run()
end

function AddSoundOptions(menu, offx, offy, centerx, bottom)
  local b

  b = Label(_("Effects Volume"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 16, offy + 36 * 1)

  -- FIXME: disable if effects turned off
  local soundslider = Slider(0, 255)
  soundslider:setValue(GetEffectsVolume())
  soundslider:setActionCallback(function() SetEffectsVolume(soundslider:getValue()) end)
  soundslider:setWidth(198)
  soundslider:setHeight(18)
  soundslider:setBaseColor(dark)
  soundslider:setForegroundColor(clear)
  soundslider:setBackgroundColor(clear)
  menu:add(soundslider, offx + 32, offy + 36 * 1.5)

  b = Label(_("min"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 32, offy + 36 * 2 + 6)
  
  b = Label(_("max"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 230 - b:getWidth(), offy + 36 * 2 + 6)

  local effectscheckbox = {}
  effectscheckbox = menu:addCheckBox(_("Enabled"), offx + 32, offy + 36 * 3,
    function() SetEffectsEnabled(effectscheckbox:isMarked()) end)
  effectscheckbox:setMarked(IsEffectsEnabled())
  effectscheckbox:adjustSize();

  b = Label(_("Music Volume"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 16, offy + 36 * 4)

  -- FIXME: disable if music turned off
  local musicslider = Slider(0, 255)
  musicslider:setValue(GetMusicVolume())
  musicslider:setActionCallback(function() SetMusicVolume(musicslider:getValue()) end)
  musicslider:setWidth(198)
  musicslider:setHeight(18)
  musicslider:setBaseColor(dark)
  musicslider:setForegroundColor(clear)
  musicslider:setBackgroundColor(clear)
  menu:add(musicslider, offx + 32, offy + 36 * 4.5)

  b = Label(_("min"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 32, offy + 36 * 5 + 6)
  
  b = Label(_("max"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 230 - b:getWidth(), offy + 36 * 5 + 6)

  local musiccheckbox = {}
  musiccheckbox = menu:addCheckBox(_("Enabled"), offx + 32, offy + 36 * 6,
    function() SetMusicEnabled(musiccheckbox:isMarked()) end)
  musiccheckbox:setMarked(IsMusicEnabled())
  musiccheckbox:adjustSize();

  b = menu:addButton(_("~!OK"), centerx, bottom - 11 - 27,
    function()
      preferences.EffectsVolume = GetEffectsVolume()
      preferences.EffectsEnabled = IsEffectsEnabled()
      preferences.MusicVolume = GetMusicVolume()
      preferences.MusicEnabled = IsMusicEnabled()
      SavePreferences()
      menu:stop()
    end)
end

function RunSoundOptionsMenu(s)
  local menu
  local offx = (Video.Width - 260) / 2
  local offy = (Video.Height - 352) / 2

  menu = BosMenu(_("Sound Options"))

  AddSoundOptions(menu, offx, offy, offx + 130 - 200/2, offy + 352)

  menu:run()
end

function SetVideoSize(width, height)
  if (Video:ResizeScreen(width, height) == false) then
    return
  end
  bckground:Resize(Video.Width, Video.Height)
  backgroundWidget = ImageWidget(bckground)
  Load("scripts/ui.lua")
  preferences.VideoWidth = Video.Width
  preferences.VideoHeight = Video.Height
  SavePreferences()
end

function BuildVideoOptionsMenu(menu)
  local b
  local x1 = (Video.Width / 3) - 100
  local x2 = (Video.Width / 3) * 2 - 100
  local offy = (Video.Height - 388) / 2

  local modes = {
     -- 4:3, 5:4 aspect ratio modes
     {  640,  480 },		-- 4:3
     {  800,  600 },		-- 4:3
     { 1024,  768 },		-- 4:3
     { 1280, 1024 },		-- 5:4
     { 1600, 1200 },		-- 4:3
     -- 16:9, 8:5 (widescreen) aspect ratio modes
     { 1024,  600 },		-- 5.12:3 = 15.36:9
     { 1280,  720 }, 		-- 16:9
     { 1366,  768 },		-- 16:9 inexact
     { 1440,  900 },		-- 8:5
     { 1680, 1050 },		-- 8:5
     { 1920, 1080 },		-- 16:9
     { 1920, 1200 }		-- 8:5
  }

  local modes2 = math.ceil (#modes / 2);
  local offybot = offy + 36 * modes2;

  for i, size in ipairs (modes) do
     local sx, sy = size[1], size[2];
     local t = sx .. " x " .. sy;
     local f = function ()
                  SetVideoSize (sx, sy);
                  menu:stop (1);
               end;
     local x;
     if (i <= modes2) then x = x1; else x = x2; end;
     local y1 = offy + 36 * (i + .5);
     if (i <= modes2) then y = y1; else y = y1 - 36 * modes2; end;
     local b = menu:addRadioButton (t, "video", x, y, f);

     if (Video.Width == sx and Video.Height == sy) then
        b:setMarked (true);
     end
  end

  fullScreen = menu:addCheckBox(_("Fullscreen"), x1, offybot + 36 * 1.5,
    function()
      ToggleFullScreen()
      preferences.VideoFullScreen = Video.FullScreen
      SavePreferences()
    end)
  fullScreen:setMarked(Video.FullScreen)

  useopengl = menu:addCheckBox(_("Use OpenGL (restart required)"), x1, offybot + 36 * 2.5,
    function()
      preferences.UseOpenGL = useopengl:isMarked()
      SavePreferences()
    end)
  useopengl:setMarked(UseOpenGL)
end

function RunVideoOptionsMenu(s)
  local menu 
  local continue = 1

  while continue == 1 do
    menu = BosMenu(_("Video Options"))
    BuildVideoOptionsMenu(menu)
    menu:addButton(_("~!OK"),
      Video.Width / 2 - 100, 
      Video.Height - 100, 
      function() menu:stop() end)
    continue = menu:run()
  end 
end

function RunLanguageOptionsMenu(s)
  local menu 
  local b
  local grid = {{}, {}, {}, {}, {}, {}}
  local offy = (Video.Height - 300) / 2
  local i = 1

  menu = BosMenu(_("Language Selection"))
  local function AddLanguage(language, po, enginepo, bospo)
    if enginepo == nil then
      enginepo = "languages/" .. po .. ".po"
      bospo = "languages/bos-" .. po .. ".po"
    end
    local function SetLanguage()
      SetTranslationsFiles(enginepo, bospo) 
      preferences.StratagusTranslation = StratagusTranslation
      preferences.GameTranslation = GameTranslation
      SavePreferences()
    end      
    local rb = menu:addRadioButton(language, "lang", 
                                   0, offy + i * 36, SetLanguage)
    grid[i][#grid[i] + 1] = rb
    i = i + 1
    if i > #grid then
       i = 1
    end
    if StratagusTranslation == enginepo then
      rb:setMarked(true)
    end
    return rb
  end
     
  b = AddLanguage("English", "en", 
        "languages/engine.pot", "languages/bos.pot")
  if StratagusTranslation == "" then
     b:setMarked(true)
  end
  AddLanguage("Français", "fr")
  AddLanguage("Suomi", "fi")
  AddLanguage("Deutsch", "de")
  AddLanguage("Polski", "pl")
  AddLanguage("Dansk", "da")
  AddLanguage("Türkçe", "tr")
  AddLanguage("Español", "es")
  AddLanguage("Czech", "cs")
  AddLanguage("Português (Br)", "pt-br")
  AddLanguage("Português (Pt)", "pt-pt")
  AddLanguage("Nederlands", "nl")
  AddLanguage("Svenska", "sv")
  AddLanguage("Italiano", "it")
  AddLanguage("Magyar", "hu")
  AddLanguage("Lietuviškai", "lt")
  menu:adjustColumnWidths(grid)

  menu:addButton(_("~!OK"), Video.Width / 2 - 100, Video.Height - 100,
    function() menu:stop() end)
  menu:run()
end

function BuildOptionsMenu(menu)
  local x = Video.Width / 2 - 100
  menu:addButton(_("~!Sound"), x, 140, RunSoundOptionsMenu)
  menu:addButton(_("~!Video"), x, 180, function() RunVideoOptionsMenu() menu:stop(1) end)
  menu:addButton(_("S~!peed"), x, 220, RunSpeedOptionsMenu)
  menu:addButton(_("~!Language"), x, 260, function() RunLanguageOptionsMenu() menu:stop(1) end)

  menu:addButton(_("Main Menu (~<Esc~>)"), x, Video.Height - 100, function() menu:stop() end)
end

function RunOptionsMenu(s)
  local menu
  local continue = 1

  while continue == 1 do
    menu = BosMenu(_("Options")) 
    BuildOptionsMenu(menu)
    continue = menu:run()
  end
end

