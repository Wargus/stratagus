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

  menu = BosMenu()

  b = Label("Sound Options")
  b:setFont(CFont:Get("large"))
  b:adjustSize();
  menu:addCentered(b, offx + 176, offy + 11)

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


function RunOptionsMenu(s)
  local menu
  menu = BosMenu()

  menu:addButton("~!Sound", 300, 140, RunSoundOptionsMenu)

  menu:run()
end

