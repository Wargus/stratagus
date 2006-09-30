--            ____
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  )
--        /_____/\____/____/
--
--      Invasion - Battle of Survival
--       A GPL'd futuristic RTS game
--
--      guichan.lua - The main UI lua script.
--
--      (c) Copyright 2005-2006 by François Beerten
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

-- Global useful objects for menus  ----------
dark = Color(38, 38, 78, 130)
clear = Color(200, 200, 120)
black = Color(0, 0, 0)

bckground = CGraphic:New("graphics/screens/menu.png")
bckground:Load()
bckground:Resize(Video.Width, Video.Height)
backgroundWidget = ImageWidget(bckground)

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

  function menu:writeText(text, x, y)
    return self:addLabel(text, x, y, Fonts["game"], false)
  end

  function menu:writeLargeText(text, x, y)
    return self:addLabel(text, x, y, Fonts["large"], false)
  end

  function menu:addButton(caption, hotkey, x, y, callback, size)
    local b = ButtonWidget(caption)
    b:setHotKey(hotkey)
    b:setActionCallback(callback)
    if (size == nil) then size = {200, 24} end
    b:setSize(size[1], size[2])
    b:setBackgroundColor(dark)
    b:setBaseColor(dark)
    self:add(b, x, y)
    return b
  end

  function menu:addSmallButton(caption, hotkey, x, y, callback)
    return self:addButton(caption, hotkey, x, y, callback, {106, 28})
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

  function menu:addBrowser(path, filter, x, y, w, h, lister)
    local mapslist = {}
    local u = 1
    local fileslist
    local i
    local f
    if lister == nil then
       lister = ListFilesInDirectory
    end
    fileslist = lister(path)
    for i,f in fileslist do
      if (string.find(f, filter)) then
        mapslist[u] = f
        u = u + 1
      end
    end

    local bq = self:addListBox(x, y, w, h, mapslist)
    bq.getSelectedItem = function(self)
      if (self:getSelected() < 0) then
        return self.itemslist[1]
      end
      return self.itemslist[self:getSelected() + 1]
    end

    return bq
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
    dd:setBaseColor(dark)
    dd:setForegroundColor(clear)
    dd:setBackgroundColor(dark)
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
  local exitButton
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

  exitButton = menu:addButton(_("E~!xit"), "x",
    Video.Width / 2 - 100, Video.Height - 100,
    function() menu:stop() end)
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

  menu = BosMenu(_("Results"), background)
  menu:writeLargeText(result, sx*6, sy*5)

  menu:writeText(_("Player"), sx*3, sy*7)
  menu:writeText(_("Units"), sx*6, sy*7)
  menu:writeText(_("Buildings"), sx*8, sy*7)
  menu:writeText(_("Kills"), sx*10, sy*7)
  menu:writeText(_("Razings"), sx*12, sy*7)

  for i=0,7 do
    if (GetPlayerData(i, "TotalUnits") > 0 ) then
      menu:writeText(i .. " ".. GetPlayerData(i, "Name"), sx*3, sy*(8+i))
      menu:writeText(GetPlayerData(i, "TotalUnits"), sx*6, sy*(8+i))
      menu:writeText(GetPlayerData(i, "TotalBuildings"), sx*8, sy*(8+i))
      menu:writeText(GetPlayerData(i, "TotalKills"), sx*10, sy*(8+i))
      menu:writeText(GetPlayerData(i, "TotalRazings"), sx*12, sy*(8+i))     
    end
  end

  menu:run()
end

function RunMap(map, objective, fow, revealmap)
  if objective == nil then
    current_objective = default_objective
  else
    current_objective = objective
  end
  loop = true
  while (loop) do
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
  RunResultsMenu(s)
end

difficulty = 5
mapresources = 5
startingresources = 5

function RunStartGameMenu(s)
  local menu
  local maptext
  local descr
  local numplayers = 2
  local players
  local sx = Video.Width / 20
  local sy = Video.Height / 20
  local map = "islandwar.smp"

  menu = BosMenu(_("Start Game"))

  menu:writeLargeText(_("Map"), sx, sy*3)
  menu:writeText(_("File:"), sx, sy*3+30)
  maptext = menu:writeText(map, sx+50, sy*3+30)
  menu:writeText(_("Players:"), sx, sy*3+50)
  players = menu:writeText(numplayers, sx+70, sy*3+50)
  menu:writeText(_("Description:"), sx, sy*3+70)
  descr = menu:writeText(description, sx+20, sy*3+90)

  local fow = menu:addCheckBox(_("Fog of war"), sx, sy*3+120, function() end)
  fow:setMarked(preferences.FogOfWar)
  local revealmap = menu:addCheckBox(_("Reveal map"), sx, sy*3+150, function() end)
  
  menu:writeText(_("Difficulty:"), sx, sy*11)
  menu:addDropDown({_("easy"), _("normal"), _("hard")}, sx + 90, sy*11 + 7,
    function(dd) difficulty = (5 - dd:getSelected()*2) end)
  menu:writeText(_("Map richness:"), sx, sy*11+25)
  menu:addDropDown({_("high"), _("normal"), _("low")}, sx + 110, sy*11+25 + 7,
    function(dd) mapresources = (5 - dd:getSelected()*2) end)
  menu:writeText(_("Starting resources:"), sx, sy*11+50)
  menu:addDropDown({_("high"), _("normal"), _("low")}, sx + 150, sy*11+50 + 7,
    function(dd) startingresources = (5 - dd:getSelected()*2) end)

  local OldPresentMap = PresentMap
  PresentMap = function(description, nplayers, w, h, id)
    print(description)
    numplayers = nplayers
    players:setCaption(""..nplayers)
    descr:setCaption(description)
    OldPresentMap(description, nplayers, w, h, id)
  end
 
  Load("maps/"..map)
  local browser = menu:addBrowser("maps/", "^.*%.smp$",  sx*10, sy*2+20, sx*8, sy*11)
  local function cb(s)
    print(browser:getSelectedItem())
    maptext:setCaption(browser:getSelectedItem())
    Load("maps/" .. browser:getSelectedItem())
    map = browser:getSelectedItem()
  end
  browser:setActionCallback(cb)

  local function startgamebutton(s)
    print("Starting map -------")
    RunMap("maps/" .. map, nil, fow:isMarked(), revealmap:isMarked())
    PresentMap = OldPresentMap
    menu:stop()
  end
  menu:addButton(_("Start"), 0,  sx * 11,  sy*14, startgamebutton)

  menu:run()
  PresentMap = OldPresentMap
end

function RunReplayMenu(s)
  local menu
  menu = BosMenu(_("Show a Replay"))

  local browser = menu:addBrowser("~logs/", ".log$", 300, 100, 300, 200)

  local reveal = menu:addCheckBox(_("Reveal map"), 100, 250, function() end)

  function startreplaybutton(s)
    print("Starting map -------")
    StartReplay("~logs/" .. browser:getSelectedItem(), reveal:isMarked())
    menu:stop()
  end

  menu:addButton(_("~!Start"), "s", 100, 300, startreplaybutton)

  menu:run()
end


function RunLoadGameMenu(s)
  local menu
  local b

  menu = BosMenu(_("Load Game"))
  local browser = menu:addBrowser("~save", ".sav.gz$", 300, 100, 300, 200)
    function startgamebutton(s)
      print("Starting saved game")
      currentCampaign = nil
      loop = true
      while (loop) do
        StartSavedGame("~save/" .. browser:getSelectedItem())
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
  menu:addButton(_("Start"), 0, 100, 300, startgamebutton)

  menu:run()
end

function RunEditorMenu(s)
  local menu
  local sx = Video.Width / 20
  local sy = Video.Height / 20
  local selectedmap = "islandwar.smp"
  local numplayers = 2

  menu = BosMenu(_("Editor"))

  menu:writeLargeText(_("Map"), sx, sy*3)
  menu:writeText(_("File:"), sx, sy*3+30)
  local maptext = menu:writeText(selectedmap .. "                       ", sx+50, sy*3+30)
  menu:writeText(_("Players:"), sx, sy*3+50)
  local players = menu:writeText("             ", sx+70, sy*3+50)
  menu:writeText(_("Description:"), sx, sy*3+70)
  local descr = menu:writeText("                                        ", sx+20, sy*3+90)

  local OldPresentMap = PresentMap
  PresentMap = function(description, nplayers, w, h, id)
    print(description)
    numplayers = nplayers
    players:setCaption(""..nplayers)
    descr:setCaption(description)
    OldPresentMap(description, nplayers, w, h, id)
  end

  Load("maps/"..selectedmap)
  local browser = menu:addBrowser("maps/", "^.*%.smp$", sx*10, sy*2+20, sx*8, sy*11)
  local function selectMap(s)
    print(browser:getSelectedItem())
    maptext:setCaption(browser:getSelectedItem())
    Load("maps/" .. browser:getSelectedItem())
    selectedmap = browser:getSelectedItem()
  end
  browser:setActionCallback(selectMap)

  function starteditorbutton(s)
    UI.MenuButton:SetCallback(function() RunEditorIngameMenu() end)
    HandleCommandKey = HandleEditorIngameCommandKey
    StartEditor(selectedmap)
    UI.MenuButton:SetCallback(function() RunGameMenu() end)
    HandleCommandKey = HandleIngameCommandKey
    menu:stop()
  end
  menu:addButton(_("Start Editor"), 0, sx * 11,  sy*14, starteditorbutton)

  menu:run()
  PresentMap = OldPresentMap
end

Load("scripts/menus/network.lua")
Load("scripts/menus/options.lua")
Load("scripts/menus/credits.lua")
Load("scripts/menus/widgetsdemo.lua")
Load("scripts/menus/ingame/game.lua")
Load("scripts/menus/ingame/editor.lua")
Load("scripts/menus/campaigns.lua")

function BuildMainMenu(menu)
  local x = Video.Width / 2 - 100
  local ystep = Video.Height / 20
  menu:addButton(_("~!Start Game"), "s", x, ystep * 4, RunStartGameMenu)
  menu:addButton(_("Start ~!Editor"), "e", x, ystep * 5, RunEditorMenu)
  menu:addButton(_("~!Options"), "o", x, ystep * 6, function() RunOptionsMenu() menu:stop(1) end)
  menu:addButton(_("~!MultiPlayer"), "m", x, ystep * 7, RunMultiPlayerMenu)
  menu:addButton(_("~!Campaigns"), "c", x, ystep * 8, RunCampaignsMenu)
  menu:addButton(_("~!Load Game"), "l", x, ystep * 9, RunLoadGameMenu)
  menu:addButton(_("Show ~!Replay"), "r", x, ystep * 10, RunReplayMenu)
  menu:addButton(_("Cre~!dits"), "d", x, ystep * 11, RunCreditsMenu)
  menu:addButton("~!Widgets Demo", "w", x, ystep * 12, RunWidgetsMenu)
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


RunMainMenu()

