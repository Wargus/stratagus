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

  menu:addLabel(_("Editor Menu"), 128, 11)
  menu:addButton(_("Save (~<F11~>)"), "f11", 16, 40,
    function() RunEditorSaveMenu() end)
  menu:addButton(_("Map Properties (~<F5~>)"), "f5", 16, 40 + (36 * 1),
    function() RunEditorMapPropertiesMenu() end)
  menu:addButton(_("Player Properties (~<F6~>)"), "f6", 16, 40 + (36 * 2),
    function() RunEditorPlayerPropertiesMenu() end)

  menu:addButton(_("~!Exit to menu"), "e", 16, 40 + (36 * 4),
    function() Editor.Running = EditorNotRunning end)
  menu:addButton(_("Return to the Editor (~<Esc~>)"), "escape", 16, 248,
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

  menu:addSmallButton(_("Save"), 0, 16, 248,
    -- FIXME: use a confirm menu if the file exists already
    function()
      print(t:getText())
      EditorSaveMap("maps/"..t:getText())
      UI.StatusLine:Set("Saved map to: " .. t:getText())
      menu:stop()
    end)

  menu:addSmallButton(_("Cancel"), 0, 16 + 12 + 106, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunEditorMapPropertiesMenu()
  local menu = BosGameMenu()
  menu:setSize(288, 256)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)

  menu:addLabel(_("Map Properties"), 288 / 2, 11)

  local l = Label(_("Map Description:"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, (288 - 260) / 2, 11 + 36)

  local desc = menu:addTextInputField(Map.Info.Description,
    (288 - 260) / 2, 11 + 36 + 22, 260)

  local l = Label(_("Size:"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, (288 - 260) / 2, 11 + (36 * 2) + 22)

  local sizeLabel = Label("" .. Map.Info.MapWidth .. " x " .. Map.Info.MapHeight)
  sizeLabel:setFont(Fonts["game"])
  menu:add(sizeLabel, 288 - ((288 - 260) / 2) - 152, 11 + (36 * 2) + 22)

  menu:addSmallButton(_("~!OK"), "o", (288 - (106 * 2)) / 4, 256 - 11 - 27,
    function()
      Map.Info.Description = desc:getText()
      menu:stop()
    end
  )

  menu:addSmallButton(_("~!Cancel"), "c", (288 - (288 - (106 * 2)) / 4) - 106, 256 - 11 - 27,
    function()
      menu:stop()
    end
  )

  menu:run(false)
end

function RunEditorPlayerPropertiesMenu()
  local menu = BosGameMenu()
  menu:setSize(640, 480)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)

  menu:addLabel(_("Player Properties"), 640 / 2, 11)

  menu:addSmallButton(_("OK"), 0, 455, 440, function() menu:stop() end)

  for i=0,15 do
    local l = Label(tostring(i))
    l:setFont(Fonts["game"])
    l:adjustSize()
    menu:add(l, 12, 40 + (22 * (i + 1)))
  end

  local l = Label(_("Race"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 40, 40 + (22 * 0))

  local l = Label(_("Type"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 130, 40 + (22 * 0))

  local types = {_("Person"), _("Computer"), _("Rescue (Passive)"),
    _("Rescue (Active)"), _("Neutral"), _("Nobody")}
  for i=0,14 do
    local d = menu:addDropDown(types, 130, 40 + (22 * (i + 1)),
      function() end)
    d:getListBox():setWidth(150)
    d:setWidth(150)
  end

  local l = Label(_("AI"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 290, 40 + (22 * 0))

  local l = Label(_("Titanium"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 420, 40 + (22 * 0))

  local l = Label(_("Crystal"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 490, 40 + (22 * 0))

  local l = Label(_("Quality"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 560, 40 + (22 * 0))

  menu:run(false)
end

