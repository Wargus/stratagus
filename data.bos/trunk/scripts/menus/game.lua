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
  local menu = MenuScreen()
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
    local b = ButtonWidget(caption)
    b:setActionCallback(callback)
    b:setSize(106, 28)
    b:setBackgroundColor(dark)
    b:setBaseColor(dark)
    menu:add(b, x, y)
  end

  function menu:addButton(caption, x, y, callback)
    local b = ButtonWidget(caption)
    b:setActionCallback(callback)
    b:setSize(224, 28)
    b:setBackgroundColor(dark)
    b:setBaseColor(dark)
    menu:add(b, x, y)
  end

  function menu:addCheckBox(caption, x, y, callback)
    local b = CheckBox(caption)
    b:setBaseColor(clear)
    b:setForegroundColor(clear)
    b:setBackgroundColor(dark)
    b:setActionCallback(function(s) callback(b, s) end)
    b:setFont(CFont:Get("game"))
    self:add(b, x, y)
    return b
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
    function() RunOptionsMenu() end)
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

function RunOptionsMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Game Options"), 128, 11)
  menu:addButton(_("Sound (~<F7~>)"), 16, 40 + (36 * 0),
    function() end)
  menu:addButton(_("Speeds (~<F8~>)"), 16, 40 + (36 * 1),
    function() end)
  menu:addButton(_("Preferences (~<F9~>)"), 16, 40 + (36 * 2),
    function() RunPreferencesMenu() end)
  menu:addButton(_("~!Diplomacy"), 16, 40 + (36 * 3),
    function() end)
  menu:addButton(_("Previous (~<Esc~>)"), 128 - (224 / 2), 248,
    function() menu:stop(1) end)

  menu:run(false)
end

function RunPreferencesMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Preferences"), 128, 11)
  menu:addCheckBox(_("Fog of War Enabled"), 16, 36 * 1,
    function() end)
  menu:addCheckBox(_("Show command key"), 16, 36 * 2,
    function() end)
  menu:addSmallButton(_("~!OK"), 128 - (106 / 2), 245,
    function() menu:stop(1) end)

  menu:run(false)
end

function RunEndScenarioMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("End Scenario"), 128, 11)
  menu:addButton(_("~!Restart Scenario"), 16, 40 + (36 * 0),
    function() RunRestartConfirmMenu() end)
  menu:addButton(_("~!Surrender"), 16, 40 + (36 * 1),
    function() RunSurrenderConfirmMenu() end)
  menu:addButton(_("~!Quit to Menu"), 16, 40 + (36 * 2),
    function() RunQuitToMenuConfirmMenu() end)
  menu:addButton(_("E~!xit Program"), 16, 40 + (36 * 3),
    function() RunExitConfirmMenu() end)
  menu:addButton(_("Previous (~<Esc~>)"), 16, 248,
    function() menu:stop(1) end)

  menu:run(false)
end

function RunRestartConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to restart"), 128, 11 + (24 * 1))
  menu:addLabel(_("the scenario?"), 128, 11 + (24 * 2))
  menu:addButton(_("~!Restart Scenario"), 16, 11 + (24 * 3) + 29,
    function() end)
  menu:addButton(_("Cancel (~<Esc~>)"), 16, 248,
    function() menu:stop(1) end)

  menu:run(false)
end

function RunSurrenderConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to surrender"), 128, 11 + (24 * 1))
  menu:addLabel(_("to your enemies?"), 128, 11 + (24 * 2))
  menu:addButton(_("~!Surrender"), 16, 11 + (24 * 3) + 29,
    function() end)
  menu:addButton(_("Cancel (~<Esc~>)"), 16, 248,
    function() menu:stop(1) end)

  menu:run(false)
end

function RunQuitToMenuConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to quit to"), 128, 11 + (24 * 1))
  menu:addLabel(_("the main menu?"), 128, 11 + (24 * 2))
  menu:addButton(_("~!Surrender"), 16, 11 + (24 * 3) + 29,
    function() end)
  menu:addButton(_("Cancel (~<Esc~>)"), 16, 248,
    function() menu:stop(1) end)

  menu:run(false)
end

function RunExitConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to exit"), 128, 11 + (24 * 1))
  menu:addLabel(_("Stratagus?"), 128, 11 + (24 * 2))
  menu:addButton(_("E~!xit Program"), 16, 11 + (24 * 3) + 29,
    function() end)
  menu:addButton(_("Cancel (~<Esc~>)"), 16, 248,
    function() menu:stop(1) end)

  menu:run(false)
end

