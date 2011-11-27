--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      editor.lua - In-game menus.
--
--      (c) Copyright 2006-2010 by Francois Beerten and Jimmy Salmon
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

function ErrorEditorMenu(errmsg)
  local menu = BosGameMenu()
  menu:setBaseColor(darkNoAlpha)
  menu:setDrawMenusUnder(true)

  menu:addLabel(_("Error"), menu:getWidth() / 2, 11)

  local l = MultiLineLabel(errmsg)
  l:setFont(Fonts["large"])
  l:setAlignment(MultiLineLabel.CENTER)
  l:setVerticalAlignment(MultiLineLabel.CENTER)
  l:setLineWidth(240)
  l:setWidth(240)
  l:setHeight(200)
  l:setBackgroundColor(dark)
  menu:add(l, menu:getWidth() / 2 - l:getWidth() / 2, 50)

  menu:addSmallButton(_("~!OK"), menu:getWidth() / 2 - 50, 248,
    function() menu:stop() end)

  menu:run(false)
end

function HandleEditorIngameCommandKey(key, ctrl, alt, shift)
  if ((key == "m" and alt) or key == "f10") then
    RunEditorIngameMenu()
  elseif ((key == "s" and alt) or key == "f11") then
    RunEditorSaveMenu()
  elseif (key == "f5") then
    RunEditorMapPropertiesMenu()
  elseif (key == "f6") then
    RunEditorPlayerPropertiesMenu()
  elseif (key == "t" and not (ctrl or alt)) then
    Editor.ShowTerrainFlags = not Editor.ShowTerrainFlags
  elseif (key == "t" and (ctrl and shift)) then
    Editor:TileSelectedPatch()
  else
    return false
  end
  return true
end


function RunEditorIngameMenu(s)
  local menu = BosGameMenu()
  local showTerrain

  menu:setSize(256, 324)
  menu:addLabel(_("Editor Menu"), 128, 11)
  menu:addButton(_("Save (~<F11~>)"), 16, 40,
    function() RunEditorSaveMenu() end)
  menu:addButton(_("Map Properties (~<F5~>)"), 16, 40 + (36 * 1),
    function() RunEditorMapPropertiesMenu() end)
  menu:addButton(_("Player Properties (~<F6~>)"), 16, 40 + (36 * 2),
    function() RunEditorPlayerPropertiesMenu() end)
  showTerrain = menu:addCheckBox(_("Show terrain attributes"), 16, 40 + (36 * 3),
    function() Editor.ShowTerrainFlags = showTerrain:isMarked() end)
  showTerrain:setFont(Fonts["large"])
  showTerrain:setMarked(Editor.ShowTerrainFlags)

  menu:addButton(_("~!Exit to menu"), 16, 40 + (36 * 5),
    function() Editor.Running = EditorNotRunning; menu:stop() end)
  menu:addButton(_("Return to the Editor (~<Esc~>)"), 16, 284,
    function() menu:stop() end)

  menu:run(false)
end

function RunEditorSaveMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Save Map"), 128, 11)

  local t = menu:addTextInputField("new.map", 16, 40, 224)

  local browser = menu:addMapBrowser("maps/", 16, 70, 224, 166)
  local function cb(s)
    t:setText(browser:getSelectedItem())
  end
  browser:setActionCallback(cb)

  menu:addSmallButton(_("~!Save"), 16, 248,
    -- FIXME: use a confirm menu if the file exists already
    function()
      local mapname = browser.path .. t:getText()
      EditorSaveMap(mapname)
      UI.StatusLine:Set("Saved map to: " .. mapname)
      menu:stop()
    end)

  menu:addSmallButton(_("Cancel (~<Esc~>)"), 16 + 12 + 106, 248,
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

  menu:addSmallButton(_("~!OK"), (288 - (106 * 2)) / 4, 256 - 11 - 27,
    function()
      Map.Info.Description = desc:getText()
      menu:stop()
    end
  )

  menu:addSmallButton(_("Cancel (~<Esc~>)"), (288 - (288 - (106 * 2)) / 4) - 106, 256 - 11 - 27,
    function()
      menu:stop()
    end
  )

  menu:run(false)
end

function RunEditorPlayerPropertiesMenu()
  local menu = BosGameMenu()
  menu:setSize(500, 310)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)

  menu:addLabel(_("Player Properties"), 500 / 2, 11)

  local LastEditablePlayer = 7

  for i=0,LastEditablePlayer do
    local l = Label(tostring(i))
    l:setFont(Fonts["game"])
    l:adjustSize()
    menu:add(l, 12, 40 + (22 * (i + 1)))
  end

  local l = Label(_("Type"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 50, 40 + (22 * 0))

  local types = {_("Person"), _("Computer"), _("Rescue (Passive)"),
    _("Rescue (Active)"), _("Neutral"), _("Nobody")}
  local pt =  { 5, 4, 5, 1, 0, 2, 3 }; pt[0] = 5
  local pt2 = { 4, 6, 7, 2, 3 }; pt2[0] = 5
  local typeWidgets = {}
  for i=0,LastEditablePlayer do
    local d = menu:addDropDown(types, 50, 40 + (22 * (i + 1)),
      function() end)
    d:getListBox():setWidth(150)
    d:setWidth(150)
    d:setSelected(pt[Map.Info.PlayerType[i]])
    typeWidgets[i] = d
  end

  local l = Label(_("AI"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 210, 40 + (22 * 0))

  local types = {}		-- key >= 1; value = .Name
  local itypes = {}		-- key >= 0; value = .Ident
  local ainametonum = {}	-- key = .Ident; value >= 0
  local i = 1
  for ai_ident,ai_type in pairs(AiTypes) do
    types[i] = ai_type.Name
    ainametonum[ai_ident] = i - 1
    itypes[i - 1] = ai_ident
    i = i + 1
  end

  -- If any players have AI types that this version does not support,
  -- then add them to the list, so that the editor will at least
  -- display them and save them back as they were.
  for i=0,LastEditablePlayer do
    local ident = Players[i].AiName
    -- Check ainametonum[ident] rather than AiTypes[ident]
    -- so we add each ident only once even if multiple players have it.
    if ainametonum[ident] == nil then
      local name = ident .. _(" (unsupported)")
      types[#types + 1] = name
      itypes[#itypes + 1] = ident
      ainametonum[ident] = #itypes
    end
  end

  local aiWidgets = {}
  for i=0,LastEditablePlayer do
    local d = menu:addDropDown(types, 210, 40 + (22 * (i + 1)),
      function(dd) end)
    d:setSelected(ainametonum[Players[i].AiName])
    d:getListBox():setWidth(120)
    d:setWidth(80)
    aiWidgets[i] = d
  end

  local l = Label(_("Energy"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 340, 40 + (22 * 0))

  local energyWidgets = {}
  for i=0,LastEditablePlayer do
    local d = menu:addTextInputField("" .. Players[i].EnergyStored,
      340, 40 + (22 * (i + 1)), 60)
    energyWidgets[i] = d
  end

  local l = Label(_("Magma"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 410, 40 + (22 * 0))

  local magmaWidgets = {}
  for i=0,LastEditablePlayer do
    local d = menu:addTextInputField("" .. Players[i].MagmaStored,
      410, 40 + (22 * (i + 1)), 60)
    magmaWidgets[i] = d
  end

  menu:addSmallButton(_("~!OK"), (500 - (106 * 2)) / 4, 270,
    function()
      -- Check for at least 1 Person player type
      local foundPlayer = false
      for i=0,LastEditablePlayer do
        if (pt2[typeWidgets[i]:getSelected()] == 5) then
          foundPlayer = true
          break
        end
      end

      if (foundPlayer == false) then
        ErrorEditorMenu(_("Must have at least one Type set to Person"))
        return
      end

      for i=0,LastEditablePlayer do
        Map.Info.PlayerType[i] = pt2[typeWidgets[i]:getSelected()]
        Players[i].AiName = itypes[aiWidgets[i]:getSelected()]
        Players[i].EnergyStored = tonumber(energyWidgets[i]:getText())
        Players[i].MagmaStored = tonumber(magmaWidgets[i]:getText())
      end
      menu:stop()
    end)

  menu:addSmallButton(_("Cancel (~<Esc~>)"), (500 - (500 - (106 * 2)) / 4) - 106, 270,
    function() menu:stop() end)

  menu:run(false)
end



