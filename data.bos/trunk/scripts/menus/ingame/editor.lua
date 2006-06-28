--            ____
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  )
--        /_____/\____/____/
--
--      Invasion - Battle of Survival
--       A GPL'd futuristic RTS game
--
--      editor.lua - In-game menus.
--
--      (c) Copyright 2006 by Francois Beerten and Jimmy Salmon
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
--      $Id: game.lua 453 2006-06-26 18:36:19Z feb $

function HandleEditorIngameCommandKey(key, ctrl, alt, shift)
  if ((key == "m" and alt) or key == "f10") then
    RunEditorIngameMenu()
  elseif ((key == "s" and alt) or key == "f11") then
    RunEditorSaveMenu()
  else
    return false
  end
  return true
end


function RunEditorIngameMenu(s)
  local menu = BosGameMenu()

  menu:addLabel(_("Game Menu"), 128, 11)
  menu:addButton(_("Save (~<F11~>)"), 16, 40,
    function() RunEditorSaveMenu() end)
  menu:addButton(_("Map Properties (~<F5~>)"), 16, 40 + (36 * 1),
    function() end)
  menu:addButton(_("Player Properties (~<F6~>)"), 16, 40 + (36 * 2),
    function() end)

  menu:addButton(_("~!Exit to menu"), 16, 40 + (36 * 4),
    function() end)
  menu:addButton(_("Return to Game (~<Esc~>)"), 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunEditorSaveMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Save Map"), 128, 11)

  local t = menu:addTextInputField("map.smp", 16, 40, 224)

  local browser = menu:addBrowser("maps/", ".smp$", 16, 70, 224, 166)
  local function cb(s)
    t:setText(browser:getSelectedItem())
  end
  browser:setActionCallback(cb)

  menu:addSmallButton(_("Save"), 16, 248,
    -- FIXME: use a confirm menu if the file exists already
    function()
      print(t:getText())
      EditorSaveMap("maps/"..t:getText())
      UI.StatusLine:Set("Saved map to: " .. t:getText())
      menu:stop()
    end)

  menu:addSmallButton(_("Cancel"), 16 + 12 + 106, 248,
    function() menu:stop() end)

  menu:run(false)
end


