--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      guichan.lua - The main UI lua script.
--
--      (c) Copyright 2005-2008 by François Beerten
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

-- Global useful objects for menus  ----------
dark = Color(38, 38, 78, 128)
darkNoAlpha = Color(38, 38, 78)
clear = Color(200, 200, 128)
black = Color(0, 0, 0)
disabled = Color(112, 112, 112, 128)

bckground = CGraphic:New("graphics/screens/menu.png")
bckground:Load()
bckground:Resize(Video.Width, Video.Height)
backgroundWidget = ImageWidget(bckground)

local SavedGame = false

function FilterList(originallist, pattern)
  local filteredlist = {}
  local k, v
  for k, v in ipairs(originallist) do
    if (string.find(v, pattern)) then
      table.insert(filteredlist, v)
    end
  end
  return filteredlist  
end

function CreateFilteringLister(pattern, lister)
  function ListFilteredItems(path)
     return FilterList(lister(path), pattern)
  end
  return ListFilteredItems
end


function AddMenuHelpers(menu)
  function menu:addCentered(widget, x, y)
    self:add(widget, x - widget:getWidth() / 2, y)
  end

  function menu:addLabel(text, x, y, font, center)
    local label = Label(text)
    if (font == nil) then font = Fonts["large"] end
    label:setFont(font)
    label:adjustSize()
    if (center == nil or center == true) then -- center text by default
      x = x - label:getWidth() / 2
    end
    self:add(label, x, y)

    return label
  end
  function menu:addMultiLineLabel(text, x, y, font, center, width)
    local label = MultiLineLabel(text)
    if (font == nil) then font = Fonts["large"] end
    label:setFont(font)
    if (width == nil) then width = 240 end
    label:setLineWidth(width)
    label:adjustSize()
    if (center == nil or center == true) then -- center text by default
      x = x - label:getWidth() / 2
    end
    self:add(label, x, y)

    return label
  end


  function menu:writeText(text, x, y)
    return self:addLabel(text, x, y, Fonts["game"], false)
  end

  function menu:writeLargeText(text, x, y)
    return self:addLabel(text, x, y, Fonts["large"], false)
  end

  function menu:addButton(caption, x, y, callback, size)
    local b = ButtonWidget(caption)
    b:setActionCallback(callback)
    if (size == nil) then size = {200, 24} end
    b:setSize(size[1], size[2])
    b:setBackgroundColor(dark)
    b:setBaseColor(dark)
    b:setDisabledColor(disabled)
    self:add(b, x, y)
    return b
  end

  function menu:addSmallButton(caption, x, y, callback)
    return self:addButton(caption, x, y, callback, {106, 28})
  end

  function menu:addSlider(min, max, w, h, x, y, callback)
    local b = Slider(min, max)
    b:setBaseColor(dark)
    b:setForegroundColor(clear)
    b:setBackgroundColor(clear)
    b:setSize(w, h)
    b:setActionCallback(function(s) callback(b, s) end)
    self:add(b, x, y)
    return b
  end

  function menu:addListBox(x, y, w, h, list)
    local bq = ListBoxWidget(w, h)
    bq:setList(list)
    bq:setBaseColor(black)
    bq:setForegroundColor(clear)
    bq:setBackgroundColor(dark)
    bq:setFont(Fonts["game"])
    self:add(bq, x, y)   
    bq.itemslist = list
    return bq
  end

  function menu:addBrowser(path, lister, x, y, w, h, default)
    local bq = self:addListBox(x, y, w, h, {})
    bq.origpath = path
    bq.actioncb = nil

    local function updateList()
      bq.itemslist = lister(bq.path)
      if (bq.path ~= bq.origpath) then
        table.insert(bq.itemslist, 1, "../")
      end
      bq:setList(bq.itemslist)
    end

    -- Change to the default directory and select the default file
    if (default == nil) then
      bq.path = path
      updateList()
    else
      local i
      for i = string.len(default) - 1, 1, -1 do
        if (string.sub(default, i, i) == "/") then
          bq.path = string.sub(default, 1, i)
          updateList()

          local f = string.sub(default, i + 1)
          for i = 1, table.getn(bq.itemslist) do
            if (bq.itemslist[i] == f) then
              bq:setSelected(i - 1)
            end
          end
          break
        end
      end
    end

    function bq:getSelectedItem()
      if (self:getSelected() < 0) then
        return self.itemslist[1]
      end
      return self.itemslist[self:getSelected() + 1]
    end

    -- If a directory was clicked change dirs
    -- Otherwise call the user's callback
    local function cb(s)
      local f = bq:getSelectedItem()
      if (f == "../") then
        local i
        for i = string.len(bq.path) - 1, 1, -1 do
          if (string.sub(bq.path, i, i) == "/") then
            bq.path = string.sub(bq.path, 1, i)
            updateList()
            break
          end
        end
      elseif (string.sub(f, string.len(f)) == '/') then
        bq.path = bq.path .. f
        updateList()
      else
        if (bq.actioncb ~= nil) then
          bq:actioncb(s)
        end
      end
    end
    bq:setActionCallback(cb)

    bq.oldSetActionCallback = bq.setActionCallback
    function bq:setActionCallback(cb)
      bq.actioncb = cb
    end

    return bq
  end

  function menu:addMapBrowser(path, x, y, w, h, default)
    local function listFilesAndDirs(path)
      local dirlist = {}
      local filelist = {}
      local dirs = ListDirsInDirectory(path)
      -- Create table of dirs and files
      for i,f in ipairs(dirs) do
        if (string.find(f, "^%w.*%.map$")) then
          table.insert(filelist, f)
        elseif (string.find(f, "^%a")) then
          table.insert(dirlist, f .. "/")
        end
      end
      -- Append files after dirs
      for i,f in ipairs(filelist) do
        table.insert(dirlist, f)
      end
      return dirlist
    end
    local browser = self:addBrowser(path, listFilesAndDirs, x, y, w, h, default)
    local function getMap(browser)
      return browser.path .. browser:getSelectedItem() .. "/presentation.smp"
    end
    browser.getSelectedMap = getMap
    return browser
  end

  function menu:addCheckBox(caption, x, y, callback)
    local b = CheckBox(caption)
    b:setBaseColor(clear)
    b:setForegroundColor(clear)
    b:setBackgroundColor(dark)
    b:setActionCallback(function(s) callback(b, s) end)
    b:setFont(Fonts["game"])
    self:add(b, x, y)
    return b
  end

  function menu:addRadioButton(caption, group, x, y, callback)
    local b = RadioButton(caption, group)
    b:setBaseColor(dark)
    b:setForegroundColor(clear)
    b:setBackgroundColor(dark)
    b:setActionCallback(callback)
    self:add(b, x, y)
    return b
  end

  function menu:addDropDown(list, x, y, callback)
    local dd = DropDownWidget()
    dd:setFont(Fonts["game"])
    dd:setList(list)
    dd:setActionCallback(function(s) callback(dd, s) end)
    dd:setBaseColor(darkNoAlpha)
    dd:setForegroundColor(clear)
    dd:setBackgroundColor(darkNoAlpha)
    self:add(dd, x, y)
    return dd
  end

  function menu:addTextInputField(text, x, y, w)
    local b = TextField(text)
    b:setActionCallback(function() end) --FIXME: remove this?
    b:setFont(Fonts["game"])
    b:setBaseColor(clear)
    b:setForegroundColor(clear)
    b:setBackgroundColor(dark)
    if (w == nil) then w = 100 end
    b:setSize(w, 18)
    self:add(b, x, y)
    return b
  end
end

function BosMenu(title, background)
  local menu
  local bg
  local bgg

  menu = MenuScreen()

  if background == nil then
    bg = backgroundWidget
  else
    bgg = CGraphic:New(background)
    bgg:Load()
    bgg:Resize(Video.Width, Video.Height)
    bg = ImageWidget(bgg)
  end
  menu:add(bg, 0, 0)

  AddMenuHelpers(menu)

  if title then
    menu:addLabel(title, Video.Width / 2, Video.Height / 20, Fonts["large"])
  end

  return menu
end

-- Default configurations -------
Widget:setGlobalFont(Fonts["large"])


-- Define the different menus ----------

function RunSubMenu(s)
  local menu
  menu = BosMenu(_("Empty sub menu"))
  menu:run()
end

function RunResultsMenu()
  local menu
  local background = "graphics/screens/menu.png"
  local sx = Video.Width / 20
  local sy = Video.Height / 20
  local result

  if GameResult == GameVictory then
    result = _("Victory !")
    background = "graphics/screens/victory.png"
  elseif GameResult == GameDraw then
    result = _("Draw !")
  elseif GameResult == GameDefeat then
    result = _("Defeat !")
    background = "graphics/screens/defeat.png"
  else 
    return
  end

  if (Cheater) then
    result = result .. " - " .. _("Cheater")
  end

  menu = BosMenu(_("Results"), background)
  menu:writeLargeText(result, sx*6, sy*5)

  menu:writeText(_("Player"), sx*3, sy*7)
  menu:writeText(_("Units"), sx*6, sy*7)
  menu:writeText(_("Buildings"), sx*8, sy*7)
  menu:writeText(_("Kills"), sx*10, sy*7)
  menu:writeText(_("Razings"), sx*12, sy*7)

  for i=0,7 do
    if (Players[i].TotalUnits > 0) then
      menu:writeText(i .. " ".. Players[i].Name, sx*3, sy*(8+i))
      menu:writeText(Players[i].TotalUnits, sx*6, sy*(8+i))
      menu:writeText(Players[i].TotalBuildings, sx*8, sy*(8+i))
      menu:writeText(Players[i].TotalKills, sx*10, sy*(8+i))
      menu:writeText(Players[i].TotalRazings, sx*12, sy*(8+i))     
    end
  end

  menu:addButton(_("~!Continue"), Video.Width / 2 - 100, Video.Height - 100,
                 function() menu:stop() end)

  menu:run()
end

function RunMap(map, objective, fow, revealmap)
  if objective == nil then
    SetObjectives(default_objective)
  else
    SetObjectives(objective)
  end
  loop = true
  while (loop) do
    InitGameVariables()
    if fow ~= nil then
      SetFogOfWar(fow)
    end
    if revealmap == true then
       RevealMap()
    end
    StartMap(map)
    if GameResult ~= GameRestart then
      loop = false
    end
  end
  RunResultsMenu()
end

current_objectives = ""
function ClearObjectives()
  current_objectives = ""
end

function AddObjective(objective)
  if (current_objectives ~= "") then
    current_objectives = current_objective .. "\n" .. objective
  else
    current_objectives = objective
  end
end

function SetObjectives(objectives)
  current_objectives = objectives
end

function GetObjectives()
  return current_objectives
end


-- Callback right before the game starts
function GameStarting()
  local i
  local factor

  if (not SavedGame) then
    for i=0,7 do
      -- resources can be 1,3,5 - low,medium,high
      if (GameSettings.Resources == 1) then
        factor = 0.5
      elseif (GameSettings.Resources == 3) then
        factor = 1
      else
        factor = 10
      end
      Players[i].MagmaStored = Players[i].MagmaStored * factor
      Players[i].EnergyStored = Players[i].EnergyStored * factor
    end
  end

  Cheater = false

  -- FIXME: get the version from somewhere else
  UI.StatusLine:Set("Bos Wars V" .. "2.5.0" ..
    ", (c) 1998-2008 by the Bos Wars and Stratagus Project.")
end


function ResetMapOptions()
  GameSettings.Resources = 3
  GameSettings.Difficulty = 3
  GameSettings.GameType = SettingsGameTypeMapDefault
  GameSettings.NoFogOfWar = false
  GameSettings.RevealMap = 0
end


function RunStartGameMenu(s)
  local menu
  local maptext
  local descr
  local numplayers = 2
  local players
  local sx = Video.Width / 20
  local sy = Video.Height / 20
  local selectedmap = "islandwar.map"
  local d

  menu = BosMenu(_("Start Game"))

  menu:writeLargeText(_("Map"), sx, sy*3)
  menu:writeText(_("File:"), sx, sy*3+30)
  local maptext = menu:writeText(selectedmap, sx+50, sy*3+30)
  maptext:setWidth(sx * 9 - 50 - 20)
  menu:writeText(_("Size:"), sx, sy*3+50)
  local mapsize = menu:writeText("       ", sx+50, sy*3+50)
  mapsize:setWidth(sx * 9 - 50 - 20)
  menu:writeText(_("Players:"), sx, sy*3+70)
  local players = menu:writeText("             ", sx+70, sy*3+70)
  menu:writeText(_("Description:"), sx, sy*3+90)
  local descr = menu:writeText("No map", sx+20, sy*3+110)
  descr:setWidth(sx * 9 - 20 - 20)

  local fow = menu:addCheckBox(_("Fog of war"), sx, sy*3+140,
    function(f) GameSettings.NoFogOfWar = not f:isMarked() end)
  fow:setMarked(preferences.FogOfWar)
  local revealmap = menu:addCheckBox(_("Reveal map"), sx, sy*3+160,
    function(f) GameSettings.RevealMap = bool2int(f:isMarked()) end)
  
  ResetMapOptions()
  menu:writeText(_("Difficulty:"), sx, sy*11)
  d = menu:addDropDown({_("easy"), _("normal"), _("hard")}, sx + 150, sy*11,
    function(dd) GameSettings.Difficulty = (5 - dd:getSelected()*2) end)
  d:setSelected(1)
  menu:writeText(_("Starting resources:"), sx, sy*11+25)
  d = menu:addDropDown({_("high"), _("normal"), _("low")}, sx + 150, sy*11+25,
    function(dd) GameSettings.Resources = (5 - dd:getSelected()*2) end)
  d:setSelected(1)
  menu:writeText(_("Game type:"), sx, sy*11+50)
  d = menu:addDropDown({_("Map Default"), _("Melee"), _("Free For All"), _("Top vs Bottom"), _("Left vs Right"), _("Man vs Machine")}, sx + 150, sy*11+50,
    function(dd) GameSettings.GameType = dd:getSelected() - 1 end)
  d:setSelected(0)
  d:setSize(140, d:getHeight())

  local OldPresentMap = PresentMap
  PresentMap = function(description, nplayers, w, h, id)
    numplayers = nplayers
    players:setCaption(""..nplayers)
    descr:setCaption(description)
	mapsize:setCaption(""..w.."x"..h)
    OldPresentMap(description, nplayers, w, h, id)
  end
 
  Load("maps/"..selectedmap..'/presentation.smp')
  local browser = menu:addMapBrowser("maps/", sx*10, sy*2+20, sx*8, sy*11,
                                     "maps/"..selectedmap)
  local function cb(s)
    maptext:setCaption(browser:getSelectedItem())
    Load(browser:getSelectedMap())
    selectedmap = browser:getSelectedItem()
  end
  browser:setActionCallback(cb)

  AllowAllUnits()
  local function startgamebutton(s)
    print("Starting map -------")
    RunMap(browser:getSelectedMap(), nil, fow:isMarked(),
           revealmap:isMarked())
    PresentMap = OldPresentMap
    menu:stop()
  end
  menu:addButton(_("~!Main Menu"), Video.Width / 2 - 250, Video.Height - 100,
                 function() menu:stop() end)
  menu:addButton(_("~!Start"), Video.Width / 2 + 50 ,  Video.Height - 100,
                 startgamebutton)

  menu:run()
  PresentMap = OldPresentMap
end

function RunReplayMenu(s)
  local menu
  menu = BosMenu(_("Show a Replay"))

  -- By default allow all units.
  -- Current implementation relies on the hypothesis that the map setup will
  -- configure which units are allowed.
  -- Stratagus should store complete starting conditions in the log.
  AllowAllUnits()

  local lister = CreateFilteringLister(".log$",  ListFilesInDirectory)
  local browser = menu:addBrowser("~logs/", lister, 300, 100, 300, 200)
  local reveal = menu:addCheckBox(_("Reveal map"), 100, 250, function() end)

  function startreplaybutton(s)
    print("Starting map -------")
    ResetMapOptions()
    InitGameVariables()
    StartReplay("~logs/" .. browser:getSelectedItem(), reveal:isMarked())
    RunResultsMenu()
    menu:stop()
  end

  menu:addButton(_("~!Main Menu"), Video.Width / 2 - 250, Video.Height - 100,
                 function() menu:stop() end)
  menu:addButton(_("~!Start"), Video.Width / 2 + 50 ,  Video.Height - 100,
                 startreplaybutton)

  menu:run()
end


function RunLoadGameMenu(s)
  local menu
  local b

  menu = BosMenu(_("Load Game"))
  local lister = CreateFilteringLister(".sav.gz$",  ListFilesInDirectory)
  local browser = menu:addBrowser("~save", lister, 
                                 Video.Width / 2 - 150, 100, 300, 200)
  function startgamebutton(s)
      print("Starting saved game")
      currentCampaign = nil
      loop = true
      while (loop) do
        InitGameVariables()
        SavedGame = true
        StartSavedGame("~save/" .. browser:getSelectedItem())
        SavedGame = false
        if (GameResult ~= GameRestart) then
          loop = false
        end
      end
      RunResultsMenu()
      if currentCampaign ~= nil then
         if GameResult == GameVictory then
            position = position + 1
         elseif GameResult == GameDefeat then
            position = position
         else
            currentCampaign = nil
            return
         end
         RunCampaign(currentCampaign)
      end
    menu:stop()
  end
  menu:addButton(_("~!Main Menu"), Video.Width / 2 - 250, Video.Height - 100,
                 function() menu:stop() end)
  menu:addButton(_("~!Start"), Video.Width / 2 + 50 ,  Video.Height - 100,
                 startgamebutton)

  DisallowAllUnits()
  menu:run()
end

function RunEditorMenu(s)
  local menu
  local x = Video.Width / 2 - 100

  menu = BosMenu(_("Editor"))

  menu:addButton(_("Create ~!New Map"), x, 220,
    function() RunEditorNewMenu(); menu:stop() end)
  menu:addButton(_("~!Load Map"), x, 260,
    function() RunEditorLoadMenu(); menu:stop() end)

  menu:addButton(_("~!Patch Editor"), x, 320,
    function() RunPatchEditorMenu(); menu:stop() end)

  menu:addButton(_("~!Cancel"), x, Video.Height - 100,
    function() menu:stop() end)

  menu:run()
end

function RunEditorNewMenu()
  local menu
  local sy = Video.Height / 20
  local xsize
  local ysize
  local defaultSize = {64, 64}

  menu = BosMenu(_("Editor"))

  function starteditorbutton(s)
    local n = tonumber(xsize:getText())
    if (n == nil) then n = 64 end
    Map.Info.MapWidth = n
    local n = tonumber(ysize:getText())
    if (n == nil) then n = 64 end
    Map.Info.MapHeight = n

    StartEditor(nil)
    Load("scripts/uilayout.lua")
    HandleCommandKey = HandleIngameCommandKey
    menu:stop()
  end

  local l = Label(_("Size:"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, Video.Width / 2 - 50, 8 * sy)

  xsize = menu:addTextInputField(tostring(defaultSize[1]),
    Video.Width / 2 + 10, 8 * sy, 40)

  local l = Label("x")
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, Video.Width / 2 + 57, 8 * sy)

  ysize = menu:addTextInputField(tostring(defaultSize[2]),
    Video.Width / 2 + 10 + 60, 8 * sy, 40)

  menu:addButton(_("~!Main Menu"), Video.Width / 2 - 250, Video.Height - 100,
    function() menu:stop() end)
  menu:addButton(_("Start ~!Editor"), Video.Width / 2 + 50, Video.Height - 100,
    starteditorbutton)

  menu:run()
end

function RunEditorLoadMenu()
  local menu
  local sx = Video.Width / 20
  local sy = Video.Height / 20
  local selectedmap = "islandwar.map"
  local numplayers = 2

  menu = BosMenu(_("Editor"))

  menu:writeLargeText(_("Map"), sx, sy*3)
  menu:writeText(_("File:"), sx, sy*3+30)
  local maptext = menu:writeText(selectedmap .. "                       ", sx+50, sy*3+30)
  menu:writeText(_("Players:"), sx, sy*3+50)
  local players = menu:writeText("             ", sx+70, sy*3+50)
  menu:writeText(_("Size:"), sx, sy*3+70)
  local mapsize = menu:writeText("       ", sx+70, sy*3+70)
  menu:writeText(_("Description:"), sx, sy*3+90)
  local descr = menu:writeText("                                        ", sx+20, sy*3+110)

  local OldPresentMap = PresentMap
  PresentMap = function(description, nplayers, w, h, id)
    numplayers = nplayers
    players:setCaption(""..nplayers)
	mapsize:setCaption(""..h.."x"..w)
    descr:setCaption(description)
    OldPresentMap(description, nplayers, w, h, id)
  end

  Load("maps/"..selectedmap..'/presentation.smp')
  local browser = menu:addMapBrowser("maps/", sx*10, sy*2+20, sx*8, sy*11, 
                                     "maps/"..selectedmap)
  local function selectMap(s)
    maptext:setCaption(browser:getSelectedItem())
    Load(browser:getSelectedMap())
    selectedmap = browser:getSelectedItem()
  end
  browser:setActionCallback(selectMap)

  function starteditorbutton(s)
    StartEditor(browser:getSelectedMap())
    Load("scripts/uilayout.lua")
    HandleCommandKey = HandleIngameCommandKey
    menu:stop()
  end
  menu:addButton(_("~!Main Menu"), Video.Width / 2 - 250, Video.Height - 100,
                 function() menu:stop() end)
  menu:addButton(_("Start ~!Editor"), Video.Width / 2 + 50, Video.Height -100,
                 starteditorbutton)

  menu:run()
  PresentMap = OldPresentMap
end

Load("scripts/menus/network.lua")
Load("scripts/menus/options.lua")
Load("scripts/menus/credits.lua")
Load("scripts/menus/ingame/game.lua")
Load("scripts/menus/ingame/editor.lua")
Load("scripts/menus/patch.lua")
Load("scripts/menus/campaigns.lua")

function BuildMainMenu(menu)
  local x = Video.Width / 3
  local ystep = Video.Height / 10
  local x1 = x - 100
  local x2 = 2*x - 100

  menu:addButton(_("~!Start Game"), x1, ystep * 2, RunStartGameMenu)
  menu:addButton(_("~!Load Game"), x1, ystep * 3, RunLoadGameMenu)
  menu:addButton(_("~!Campaigns"), x2, ystep * 2, RunCampaignsMenu)
  menu:addButton(_("Show ~!Replay"), x2, ystep * 3, RunReplayMenu)

  menu:addButton(_("~!MultiPlayer"), x1, ystep * 5, RunMultiPlayerMenu)
  menu:addButton(_("~!Options"), x2, ystep * 5, function() RunOptionsMenu() menu:stop(1) end)

  menu:addButton(_("Cre~!dits"), x1, ystep * 6, RunCreditsMenu)
  menu:addButton(_("Start ~!Editor"), x2, ystep * 6, RunEditorMenu)

  menu:addButton(_("E~!xit"), Video.Width / 2 - 100, Video.Height - 100,
                 function() menu:stop() end)

  if false then 
     menu:addButton("~!Widgets Demo", x2, ystep * 7, RunWidgetsMenu)
  end
end

function RunMainMenu(s)
  local menu
  local continue = 1

  while continue == 1 do
    menu = BosMenu() 
    BuildMainMenu(menu)
    continue = menu:run()
  end
end

function RunMapFromCommandLine()
  ResetMapOptions()
  AllowAllUnits()
  if (CliMapName ~= "") then
    RunMap(CliMapName, nil, true, false)
  end
end

RunMapFromCommandLine()
RunMainMenu()

