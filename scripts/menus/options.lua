--            ____
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  )
--        /_____/\____/____/
--
--      Invasion - Battle of Survival
--       A GPL'd futuristic RTS game
--
--      options.lua - The option menus
--
--      (c) Copyright 2005-2006 by Francois Beerten
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
--      $Id$


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

  local gamespeedslider = Slider(50, 250)
  gamespeedslider:setValue(0) --FIXME
  gamespeedslider:setActionCallback(function() fixme(gamespeedslider:getValue()) end)
  gamespeedslider:setWidth(198)
  gamespeedslider:setHeight(18)
  gamespeedslider:setBaseColor(dark)
  gamespeedslider:setForegroundColor(clear)
  gamespeedslider:setBackgroundColor(clear)
  menu:add(gamespeedslider, offx + 32, offy + 36 * 1.5)

  b = Label(_("slow"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 34, offy + 36 * 2 + 6)
  
  b = Label(_("fast"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 230, offy + 36 * 2 + 6)

  b = Label(_("Mouse Scroll"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 16, offy + 36 * 3)

  local mousescrollslider = Slider(0, 10)
  if (GetMouseScroll()) then speed = GetMouseScrollSpeed() else speed = 0 end
  mousescrollslider:setValue(10 - speed)
  mousescrollslider:setActionCallback(function() SetMouseScrollSpeed(10 - mousescrollslider:getValue()) end)
  mousescrollslider:setWidth(198)
  mousescrollslider:setHeight(18)
  mousescrollslider:setBaseColor(dark)
  mousescrollslider:setForegroundColor(clear)
  mousescrollslider:setBackgroundColor(clear)
  menu:add(mousescrollslider, offx + 32, offy + 36 * 3.5)

  b = Label(_("off"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 34, offy + 36 * 4 + 6)
  
  b = Label(_("fast"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 230, offy + 36 * 4 + 6)

  b = Label(_("Keyboard Scroll"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 16, offy + 36 * 5)

  local keyboardscrollslider = Slider(0, 10)
  if (GetKeyScroll()) then speed = GetKeyScrollSpeed() else speed = 0 end
  keyboardscrollslider:setValue(10 - speed)
  keyboardscrollslider:setActionCallback(function() SetKeyScrollSpeed(10 - keyboardscrollslider:getValue()) end)
  keyboardscrollslider:setWidth(198)
  keyboardscrollslider:setHeight(18)
  keyboardscrollslider:setBaseColor(dark)
  keyboardscrollslider:setForegroundColor(clear)
  keyboardscrollslider:setBackgroundColor(clear)
  menu:add(keyboardscrollslider, offx + 32, offy + 36 * 5.5)

  b = Label(_("off"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 34, offy + 36 * 6 + 6)
  
  b = Label(_("fast"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 230, offy + 36 * 6 + 6)

  menu:addButton(_("~!OK"), offx + 128 - (200 / 2), offy + 245,
    function() SavePreferences(); menu:stop() end)

  menu:run()
end

function RunSoundOptionsMenu(s)
  local menu
  local b
  local offx = (Video.Width - 352) / 2
  local offy = (Video.Height - 352) / 2

  menu = BosMenu(_("Sound Options"))

  b = Label(_("Effects Volume"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 16, offy + 36 * 1)

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
  menu:addCentered(b, offx + 44, offy + 36 * 2 + 6)
  
  b = Label(_("max"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 218, offy + 36 * 2 + 6)

  local effectscheckbox = {}
  effectscheckbox = menu:addCheckBox(_("Enabled"), offx + 240, offy + 36 * 1.5,
    function() SetEffectsEnabled(effectscheckbox:isMarked()) end)
  effectscheckbox:setFont(CFont:Get("large"))
  effectscheckbox:setMarked(IsEffectsEnabled())
  effectscheckbox:adjustSize();

  b = Label(_("Music Volume"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 16, offy + 36 * 3)

  local musicslider = Slider(0, 255)
  musicslider:setValue(GetMusicVolume())
  musicslider:setActionCallback(function() SetMusicVolume(musicslider:getValue()) end)
  musicslider:setWidth(198)
  musicslider:setHeight(18)
  musicslider:setBaseColor(dark)
  musicslider:setForegroundColor(clear)
  musicslider:setBackgroundColor(clear)
  menu:add(musicslider, offx + 32, offy + 36 * 3.5)

  b = Label(_("min"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 44, offy + 36 * 4 + 6)
  
  b = Label(_("max"))
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 218, offy + 36 * 4 + 6)

  local musiccheckbox = {}
  musiccheckbox = menu:addCheckBox(_("Enabled"), offx + 240, offy + 36 * 3.5,
    function() SetMusicEnabled(musiccheckbox:isMarked()) end)
  musiccheckbox:setFont(CFont:Get("large"))
  musiccheckbox:setMarked(IsMusicEnabled())
  musiccheckbox:adjustSize();

  menu:addButton(_("~!OK"), offx + 176 - (200 / 2), offy + 352 - 11 - 27,
    function() SavePreferences(); menu:stop() end)

  menu:run()
end

function SetVideoSize(width, height)
  Video:ResizeScreen(width, height)
  bckground:Resize(Video.Width, Video.Height)
  backgroundWidget = ImageWidget(bckground)
end

function BuildVideoOptionsMenu(menu)
  local b
  local offx = (Video.Width - 352) / 2 + 100
  local offy = (Video.Height - 352) / 2

  b = menu:addRadioButton("640 x 480", "video", offx, offy + 36 * 1.5,
    function() SetVideoSize(640, 480) menu:stop(1) end)
  if Video.Width == 640 then
    b:setMarked(true)
  end
  b = menu:addRadioButton("800 x 600", "video", offx, offy + 36 * 2.5,
    function() SetVideoSize(800, 600) menu:stop(1) end)
  if Video.Width == 800 then
    b:setMarked(true)
  end
  b = menu:addRadioButton("1024 x 768", "video", offx, offy + 36 * 3.5,
    function() SetVideoSize(1024, 768) menu:stop(1) end)
  if Video.Width == 1024 then
    b:setMarked(true)
  end
  b = menu:addRadioButton("1600 x 1200", "video", offx, offy + 36 * 4.5,
    function() SetVideoSize(1600, 1200) menu:stop(1) end)
  if Video.Width == 1600 then
    b:setMarked(true)
  end

  fullscreen = menu:addCheckBox(_("Fullscreen"), offx, offy + 36 * 5.5,
    function() ToggleFullScreen() end)
  b = Video.FullScreen 
  if b == true then 
    fullScreen:setMarked(true)
  end
end

function RunVideoOptionsMenu(s)
  local menu 
  local continue = 1

  while continue == 1 do
    menu = BosMenu(_("Video Options"))
    BuildVideoOptionsMenu(menu)
    continue = menu:run()
  end 
end

function RunLanguageOptionsMenu(s)
  local menu 
  local b
  local offx = (Video.Width - 352) / 2 + 100
  local offy = (Video.Height - 352) / 2

  menu = BosMenu(_("Language Selection"))
  b = menu:addRadioButton("English", "lang", offx, offy + 36 * 1.5,
    function() LoadPO("languages/en.po") end)
  b:setMarked(true)
  b = menu:addRadioButton("Français", "lang", offx, offy + 36 * 2.5,
    function() LoadPO("languages/fr.po") LoadPO("languages/bos-fr.po") end)
  b = menu:addRadioButton("Suomi", "lang", offx, offy + 36 * 3.5,
    function() LoadPO("languages/fi.po") LoadPO("languages/bos-fi.po") end)
  b = menu:addRadioButton("Polski", "lang", offx, offy + 36 * 4.5,
    function() LoadPO("languages/pl.po") end)

  menu:run()
end

function BuildOptionsMenu(menu)
  local x = Video.Width / 2 - 100
  menu:addButton(_("Sound"), x, 140, RunSoundOptionsMenu)
  menu:addButton(_("Video"), x, 180, function() RunVideoOptionsMenu() menu:stop(1) end)
  menu:addButton(_("Speed"), x, 220, RunSpeedOptionsMenu)
  menu:addButton(_("Language"), x, 260, function() RunLanguageOptionsMenu() menu:stop(1) end)
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

