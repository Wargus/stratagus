--            ____
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  )
--        /_____/\____/____/
--
--      Invasion - Battle of Survival
--       A GPL'd futuristic RTS game
--
--      game.lua - In-game menus.
--
--      (c) Copyright 2006 by Jimmy Salmon
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

function BosGameMenu()
  menu = MenuScreen()
  menu:setSize(256, 288)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)
  menu:setBorderSize(1)

  function menu:addLabel(text, x, y)
    local label = Label(text)
    label:setFont(CFont:Get("large"))
    label:adjustSize()
    self:add(label, x - label:getWidth() / 2, y)
  end

  function menu:addSmallButton(caption, x, y, callback)
    local b
    b = ButtonWidget(caption)
    b:setActionCallback(callback)
    b:setSize(106, 28)
    b:setBackgroundColor(dark)
    b:setBaseColor(dark)
    menu:add(b, x, y)
  end

  function menu:addButton(caption, x, y, callback)
    local b
    b = ButtonWidget(caption)
    b:setActionCallback(callback)
    b:setSize(224, 28)
    b:setBackgroundColor(dark)
    b:setBaseColor(dark)
    menu:add(b, x, y)
  end

  return menu
end

function RunGameMenu(s)
  local menu = BosGameMenu()

  menu:addLabel(_("Game Menu"), 128, 11)
  menu:addSmallButton(_("Save (~<F11~>)"), 16, 40,
    function() print "save"; RunTestMenu() end)
  menu:addSmallButton(_("Load (~<F12~>)"), 16 + 12 + 106, 40,
    function() print "load" end)
  menu:addButton(_("Options (~<F5~>)"), 16, 40 + (36 * 1),
    function() print "save" end)
  menu:addButton(_("Help (~<F1~>)"), 16, 40 + (36 * 2),
    function() print "help" end)
  menu:addButton(_("~!Objectives"), 16, 40 + (36 * 3),
    function() print "objectives" end)
  menu:addButton(_("~!End Scenario"), 16, 40 + (36 * 4),
    function() RunEndScenarioMenu() end)
  menu:addButton(_("Return to Game (~<Esc~>)"), 16, 248,
    function() menu:stop(1) end)

  menu:run(false)
end

function RunEndScenarioMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("End Scenario"), 128, 11)
  menu:addButton(_("~!Restart Scenario"), 16, 40 + (36 * 0),
    function() end)
  menu:addButton(_("~!Surrender"), 16, 40 + (36 * 1),
    function() end)
  menu:addButton(_("~!Quit to Menu"), 16, 40 + (36 * 2),
    function() end)
  menu:addButton(_("Previous (~<Esc~>)"), 16, 248,
    function() menu:stop(1) end)

  menu:run(false)
end

