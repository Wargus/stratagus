--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      (c) Copyright 2009-2010 by Jimmy Salmon
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

function HandlePatchEditorIngameCommandKey(key, ctrl, alt, shift)
  return false
end

function RunNewPatchMenu()
  local menu
  local y = Video.Height * 4 / 10
  local name = ""
  local image = ""
  local width = 1
  local height = 1
  local returnToMainMenu = false

  menu = BosMenu(_("Patch Editor"))

  menu:addLabel(_("Name:"), Video.Width / 2 - 80, y)
  local nameInput = menu:addTextInputField(name,
    Video.Width / 2 - 50, y, 100)

  y = y + 30

  menu:addLabel(_("Image:"), Video.Width / 2 - 80, y)
  local imageInput = menu:addTextInputField(image,
    Video.Width / 2 - 50, y)

  y = y + 30

  menu:addLabel(_("Size:"), Video.Width / 2 - 80, y)
  local widthInput = menu:addTextInputField(tostring(width),
    Video.Width / 2 - 50, y, 24)
  menu:addLabel(_("x"), Video.Width / 2 + 34 - 50, y)
  local heightInput = menu:addTextInputField(tostring(height),
    Video.Width / 2 + 44 - 50, y, 24)

  y = y + 30

  menu:addButton(_("Cancel (~<Esc~>)"), Video.Width / 2 - 250, Video.Height - 100,
    function() menu:stop() end)
  menu:addButton(_("Create ~!Patch"), Video.Width / 2 + 50, Video.Height - 100,
    function()
      name = nameInput:getText()
      image = imageInput:getText()
      width = tonumber(widthInput:getText())
      height = tonumber(heightInput:getText())

      local flags = {}
      for i=1,width * height do
        table.insert(flags, 0x13)
      end

      patchType(name, image, width, height, flags)
      HandleCommandKey = HandlePatchEditorIngameCommandKey
      StartPatchEditor(name)
      HandleCommandKey = HandleIngameCommandKey
      menu:stop()
      returnToMainMenu = true
    end)

  menu:run()
  return returnToMainMenu
end


function RunLoadPatchMenu()
  local menu
  local y = 260
  local name = ""
  local typeNames
  local names = {}
  local returnToMainMenu = false

  menu = BosMenu(_("Patch Editor"))

  typeNames = Map.PatchManager:getPatchTypeNames()
  -- Convert vector to lua table
  for i = 0, typeNames:size() - 1 do
   table.insert(names, typeNames[i])
  end

  menu:addLabel(_("Name:"), Video.Width / 2 - 80, y)
  local nameDropDown = menu:addDropDown(names,
    Video.Width / 2 - 50, y,
    function() end)
  nameDropDown:setSize(210, nameDropDown:getHeight())

  y = y + 40

  menu:addButton(_("Cancel (~<Esc~>)"), Video.Width / 2 - 250, Video.Height - 100,
    function() menu:stop() end)
  menu:addButton(_("Load ~!Patch"), Video.Width / 2 + 50, Video.Height - 100,
    function()
      name = names[nameDropDown:getSelected() + 1]
      HandleCommandKey = HandlePatchEditorIngameCommandKey
      StartPatchEditor(name)
      HandleCommandKey = HandleIngameCommandKey
      menu:stop()
      returnToMainMenu = true
    end)

  menu:run()
  return returnToMainMenu
end
