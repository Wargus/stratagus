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
  local buttonsY = Video.Height - 100
  local labelX
  local inputX
  local y
  local smallX = Video.Width / 20
  local smallY = Video.Height / 20
  local name = ""
  local image = ""
  local width = 1
  local height = 1
  local returnToMainMenu = false

  local function listDirsAndGraphics(path)
    local list = {}
    for pos,name in ipairs(ListDirsInDirectory(path)) do
      table.insert(list, name .. "/")
    end
    for pos,name in ipairs(ListFilesInDirectory(path)) do
      if string.find(name, "%.png$") then
        table.insert(list, name)
      end
    end
    return list
  end

  menu = BosMenu(_("Create New Patch"))

  labelX = smallX
  inputX = labelX + 30
  y = 2 * smallY + 20
  local browser = menu:addBrowser("patches/", listDirsAndGraphics,
                                  smallX * 10, y,
                                  smallX * 8, buttonsY - 2 * smallY - y)
  menu:addLabel(_("Name:"), labelX, y)
  local nameInput = menu:addTextInputField(name,
    inputX, y, smallX * 9 - inputX)

  y = y + 30

  menu:addLabel(_("Image:"), labelX, y)
  local imageInput = menu:addTextInputField(image,
    inputX, y, smallX * 9 - inputX)

  y = y + 30

  menu:addLabel(_("Size:"), labelX, y)
  local widthInput = menu:addTextInputField(tostring(width),
    inputX, y, 34)
  menu:addLabel(_("x"), inputX + 44, y)
  local heightInput = menu:addTextInputField(tostring(height),
    inputX + 54, y, 34)

  y = y + 30

  menu:addButton(_("Cancel (~<Esc~>)"), Video.Width / 2 - 250, buttonsY,
    function() menu:stop() end)
  menu:addButton(_("Create ~!Patch"), Video.Width / 2 + 50, buttonsY,
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

  local function browserCallback(event)
    local imageDirAndFile = browser.path .. browser:getSelectedItem()
    imageInput:setText(imageDirAndFile)

    local patchName = string.gsub(imageDirAndFile, "^patches/", "", 1)
    patchName = string.gsub(patchName, "%.png$", "", 1)
    patchName = string.gsub(patchName, "/", "-")
    nameInput:setText(patchName)

    local ok,width,height = Map.PatchManager:computePatchSize(imageDirAndFile)
    if ok then
      widthInput:setText(tostring(width))
      heightInput:setText(tostring(height))
    else
      widthInput:setText("")
      heightInput:setText("")
    end
  end
  browser:setActionCallback(browserCallback)

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

  menu = BosMenu(_("Select Patch to Edit"))

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
