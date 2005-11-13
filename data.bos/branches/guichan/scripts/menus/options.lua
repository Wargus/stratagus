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
--      (c) Copyright 2005 by François Beerten
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

function RunSoundOptionsMenu(s)
  local menu
  local b
  local offx = (Video.Width - 352) / 2
  local offy = (Video.Height - 352) / 2

  menu = BosMenu("Sound Options")

  b = Label("Effects Volume")
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 16, offy + 36 * 1)

  soundslider = Slider(0, 255)
  soundslider:setValue(GetSoundVolume())
  soundslider:setActionCallback(function() SetSoundVolume(soundslider:getValue()) end)
  soundslider:setWidth(198)
  soundslider:setHeight(18)
  soundslider:setBaseColor(dark)
  soundslider:setForegroundColor(clear)
  soundslider:setBackgroundColor(clear)
  menu:add(soundslider, offx + 32, offy + 36 * 1.5)

  b = Label("min")
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 44, offy + 36 * 2 + 6)
  
  b = Label("max")
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 218, offy + 36 * 2 + 6)

  b = menu:addCheckBox("Enabled", offx + 240, offy + 36 * 1.5,
    function() print("checkbox1") end)
  b:setFont(CFont:Get("large"))
  b:adjustSize();

  b = Label("Music Volume")
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

  b = Label("min")
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 44, offy + 36 * 4 + 6)
  
  b = Label("max")
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 218, offy + 36 * 4 + 6)

  b = menu:addCheckBox("Enabled", offx + 240, offy + 36 * 3.5,
    function() print("checkbox2") end)
  b:setFont(CFont:Get("large"))
  b:adjustSize();

  b = Label("CD Volume")
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:add(b, offx + 16, offy + 36 * 5)

  b = Slider(0, 1)
  b:setActionCallback(function() print("slider") end)
  b:setWidth(198)
  b:setHeight(18)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(clear)
  menu:add(b, offx + 32, offy + 36 * 5.5)

  b = Label("min")
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 44, offy + 36 * 6 + 6)
  
  b = Label("max")
  b:setFont(CFont:Get("game"))
  b:adjustSize();
  menu:addCentered(b, offx + 218, offy + 36 * 6 + 6)

  b = menu:addCheckBox("Enabled", offx + 240, offy + 36 * 5.5,
    function() print("checkbox3") end)
  b:setFont(CFont:Get("large"))
  b:adjustSize();

  menu:addButton("~!OK", offx + 176 - (200 / 2), offy + 352 - 11 - 27,
    function() end)

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
  b = menu:addRadioButton("1024 x 800", "video", offx, offy + 36 * 3.5,
    function() SetVideoSize(1024, 800) menu:stop(1) end)
  if Video.Width == 1024 then
    b:setMarked(true)
  end
  b = menu:addRadioButton("1600 x 1200", "video", offx, offy + 36 * 4.5,
    function() SetVideoSize(1600, 1200) menu:stop(1) end)
  if Video.Width == 1600 then
    b:setMarked(true)
  end

  fullscreen = menu:addCheckBox("Fullscreen", offx, offy + 36 * 5.5,
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
    menu = BosMenu("Video Options")
    BuildVideoOptionsMenu(menu)
    continue = menu:run()
  end 
end

function BuildOptionsMenu(menu)
  local x = Video.Width / 2 - 100
  menu:addButton("~!Sound", x, 140, RunSoundOptionsMenu)
  menu:addButton("~!Video", x, 180, function() RunVideoOptionsMenu() menu:stop(1) end)
end

function RunOptionsMenu(s)
  local menu
  local continue = 1

  while continue == 1 do
    menu = BosMenu("Options") 
    BuildOptionsMenu(menu)
    continue = menu:run()
  end
end

