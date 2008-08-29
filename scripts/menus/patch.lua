
function RunNewPatchMenu()
  local menu
  local x = Video.Width / 2 - 100
  local y = Video.Height * 4 / 10
  local name = ""
  local image = ""
  local width = 1
  local height = 1

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

  menu:addButton(_("Create ~!Patch"), x, Video.Height - 140,
    function()
      name = nameInput:getText()
      image = imageInput:getText()
      width = tonumber(widthInput:getText())
      height = tonumber(heightInput:getText())

      local flags = {}
      for i=1,width * height do
        table.insert(flags, 3)
      end

      patchType(name, image, width, height, flags)
      StartPatchEditor(name)
      menu:stop()
    end)
  menu:addButton(_("~!Cancel"), x, Video.Height - 100,
    function() menu:stop() end)

  menu:run()
end


function RunLoadPatchMenu()
  local menu
  local x = Video.Width / 2 - 100
  local y = 260
  local name = ""
  local typeNames
  local names = {}

  menu = BosMenu(_("Patch Editor"))

  typeNames = Map.PatchManager:getPatchTypeNames()
  -- Convert vector to lua table
  for i = 0, typeNames:size() - 1 do
   table.insert(names, typeNames[i])
  end

  menu:addLabel(_("Name:"), Video.Width / 2 - 30, y)
  local nameDropDown = menu:addDropDown(names,
    Video.Width / 2, y,
    function() end)
  nameDropDown:setSize(130, nameDropDown:getHeight())

  y = y + 40

  menu:addButton(_("Load ~!Patch"), x, Video.Height - 140,
    function()
      name = names[nameDropDown:getSelected() + 1]

      StartPatchEditor(name)
      menu:stop()
    end)
  menu:addButton(_("~!Cancel"), x, Video.Height - 100,
    function() menu:stop() end)

  menu:run()
end


function RunPatchEditorMenu()
  local menu
  local x = Video.Width / 2 - 100

  menu = BosMenu(_("Patch Editor"))

  menu:addButton(_("Create ~!New Patch"), x, 260,
    function() RunNewPatchMenu(); menu:stop() end)
  menu:addButton(_("~!Load Patch"), x, 300,
    function() RunLoadPatchMenu(); menu:stop() end)

  menu:addButton(_("~!Cancel"), x, Video.Height - 100,
    function() menu:stop() end)

  menu:run()
end
