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

-- Global usefull objects for menus  ----------
dark = Color(38, 38, 78, 130)
clear = Color(200, 200, 120)
black = Color(0, 0, 0)

bckground = CGraphic:New("graphics/screens/menu.png")
bckground:Load()
bckground:Resize(Video.Width, Video.Height)
backgroundWidget = ImageWidget(bckground)

-- Store the widget in the container. This way we keep a refence
-- to the widget until the container gets deleted.
-- TODO: embed this in tolua++
local guichanadd = Container.add
Container.add = function(self, widget, x, y)
  -- ugly hack, should be done in some kind of constructor
  if not self._addedWidgets then
     self._addedWidgets = {}
  end
  self._addedWidgets[widget] = true
  guichanadd(self, widget, x, y)
end

Container.addCentered = function(self, widget, x, y)
  self.add(self, widget, x - widget:getWidth() / 2, y)
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

  function menu:addButton(caption, x, y, callback)
    local b
    b = ButtonWidget(caption)
    b:setActionCallback(callback)
    b:setSize(200, 24)
    b:setBackgroundColor(dark)
    b:setBaseColor(dark)
    self:add(b, x, y)
    return b
  end

  function menu:addListBox(x, y, w, h, list)
    local bq
    bq = ListBoxWidget(w, h)
    bq:setList(list)
    bq:setBaseColor(black)
    bq:setForegroundColor(clear)
    bq:setBackgroundColor(dark)
    bq:setFont(CFont:Get("game"))
    menu:add(bq, x, y)   
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
      if(string.find(f, filter)) then
        mapslist[u] = f
        u = u + 1
      end
    end

    local bq
    bq = menu:addListBox(x, y, w, h, mapslist)
    bq.getSelectedItem = function(self)
        return self.itemslist[self:getSelected() + 1]
    end

    return bq
  end

  function menu:addCheckBox(caption, x, y, callback)
    local b
    b = CheckBox(caption)
    b:setBaseColor(clear)
    b:setForegroundColor(clear)
    b:setBackgroundColor(dark)
    b:setActionCallback(function(s) callback(b, s) end)
    b:setFont(CFont:Get("game"))
    self:add(b, x, y)
    return b
  end

  function menu:addRadioButton(caption, group, x, y, callback)
    local b
    b = RadioButton(caption, group)
    b:setBaseColor(dark)
    b:setForegroundColor(clear)
    b:setBackgroundColor(dark)
    b:setActionCallback(callback)
    self:add(b, x, y)
    return b
  end

  function menu:addDropDown(list, x, y, callback)
    local dd = DropDownWidget()
    dd:setFont(CFont:Get("game"))
    dd:setList(list)
    dd:setActionCallback(function(s) callback(dd, s) end)
    dd:setBaseColor(dark)
    dd:setForegroundColor(clear)
    dd:setBackgroundColor(dark)
    self:add(dd, x, y)
  end

  function menu:writeText(text, x, y)
    local label = Label(text)
    label:setFont(CFont:Get("game"))
    label:setSize(200, 30)
    menu:add(label, x, y)
    return label
  end

  function menu:writeLargeText(text, x, y)
    local label = Label(text)
    label:setFont(CFont:Get("large"))
    label:setSize(200, 30)
    menu:add(label, x, y)
    return label
  end

  function menu:addTextInputField(text, x, y)
    local b = TextField(text)
    b:setActionCallback(function() print("field") end)
    b:setFont(CFont:Get("game"))
    b:setBaseColor(clear)
    b:setForegroundColor(clear)
    b:setBackgroundColor(dark)
    b:setSize(100, 18)
    menu:add(b, x, y)
    return b
  end

  if title then
    local titlelabel = Label(title)
    titlelabel:setFont(CFont:Get("large"))
    titlelabel:adjustSize()
    menu:addCentered(titlelabel, Video.Width / 2, Video.Height/20)
  end

  exitButton = menu:addButton(_("~!Exit"),
        Video.Width / 2 - 100, Video.Height - 100,
        function() menu:stop() end)
  return menu
end

-- Default configurations -------
Widget:setGlobalFont(CFont:Get("large"))


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

function RunMap(map)
   StartMap(map)
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
  local map = "default.smp"

  menu = BosMenu(_("Start Game"))

  menu:writeLargeText(_("Map"), sx, sy*3)
  menu:writeText(_("File:"), sx, sy*3+30)
  maptext = menu:writeText(map, sx+50, sy*3+30)
  menu:writeText(_("Players:"), sx, sy*3+50)
  players = menu:writeText(numplayers, sx+70, sy*3+50)
  menu:writeText(_("Description:"), sx, sy*3+70)
  descr = menu:writeText(description, sx+20, sy*3+90)

  local fow = menu:addCheckBox(_("Fog of war"), sx, sy*3+120, function() end)
  fow:setMarked(true)
  local revealmap = menu:addCheckBox(_("Reveal map"), sx, sy*3+150, function() end)
  
  menu:writeText(_("Difficulty:"), sx, sy*11)
  menu:addDropDown({_("easy"), _("normal"), _("hard")}, sx + 90, sy*11 + 7,
      function(dd) difficulty = (5 - dd:getSelected()*2) end)
  menu:writeText(_("Map richness:"), sx, sy*11+25)
  menu:addDropDown({_("high"), _("normal"), _("low")}, sx + 110, sy*11+25 + 7,
      function(dd) mapresources = (5 - dd:getSelected()*2) end)
  menu:writeText(_("Starting resources:"), sx, sy*11+50)
  menu:addDropDown({_("high"), _("normal"), _("low")}, sx + 140, sy*11+50 + 7,
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
    SetFogOfWar(fow:isMarked())
    if revealmap:isMarked() == true then
       RevealMap()
    end
    RunMap("maps/" .. map)
    PresentMap = OldPresentMap
    menu:stop()
  end
  menu:addButton(_("Start"),  sx * 11,  sy*14, startgamebutton)

  menu:run()
  PresentMap = OldPresentMap
end

function RunReplayMenu(s)
  local menu
  menu = BosMenu(_("Show a Replay"))

  local browser = menu:addBrowser("~logs/", ".log$", 300, 100, 300, 200)

  function startreplaybutton(s)
    print("Starting map -------")
    StartReplay("~logs/" .. browser:getSelectedItem())
    menu:stop()
  end

  menu:addButton(_("~!Start"), 100, 300, startreplaybutton)

  menu:run()
end

function RunCampaignsMenu(s)
  local menu
  local b

  menu = BosMenu(_("List of Campaigns"))

  local browser = menu:addBrowser("campaigns/", "^%a", 300, 100, 300, 200, ListDirsInDirectory)
  function startgamebutton(s)
    print("Starting campaign")
    Load("campaigns/" .. browser:getSelectedItem() .. "/campaign.lua")
    menu:stop()
  end
  menu:addButton(_("Start"), 100, 300, startgamebutton)

  menu:run()
end

function RunLoadGameMenu(s)
  local menu
  local b

  menu = BosMenu(_("Load Game"))
  local browser = menu:addBrowser("~save", ".sav.gz$", 300, 100, 300, 200)
    function startgamebutton(s)
    print("Starting saved game")
    StartSavedGame("~save/" .. browser:getSelectedItem())
    menu:stop()
  end
  menu:addButton(_("Start"), 100, 300, startgamebutton)

  menu:run()
end

function RunEditorMenu(s)
  local menu
  menu = BosMenu(_("Editor"))

  local browser = menu:addBrowser("maps/", "^.*%.smp$", 300, 100, 300, 200)
  function starteditorbutton(s)
    StartEditor("maps/" .. browser:getSelectedItem())
    menu:stop()
  end

  menu:addButton(_("Start Editor"), 100, 300, starteditorbutton)

  menu:run()
end

Load("scripts/menus/network.lua")
Load("scripts/menus/options.lua")
Load("scripts/menus/credits.lua")
Load("scripts/menus/widgetsdemo.lua")

function BuildMainMenu(menu)
  local x = Video.Width / 2 - 100
  local ystep = Video.Height / 20
  menu:addButton(_("~!Start Game"), x, ystep * 4, RunStartGameMenu)
  menu:addButton(_("Start ~!Editor"), x, ystep * 5, RunEditorMenu)
  menu:addButton(_("~!Options"), x, ystep * 6, function() RunOptionsMenu() menu:stop(1) end)
  menu:addButton(_("~!MultiPlayer"), x, ystep * 7, RunMultiPlayerMenu)
  menu:addButton(_("~!Campaigns"), x, ystep * 8, RunCampaignsMenu)
  menu:addButton(_("~!Load Game"), x, ystep * 9, RunLoadGameMenu)
  menu:addButton(_("Show ~!Replay"), x, ystep * 10, RunReplayMenu)
  menu:addButton(_("~!Credits"), x, ystep * 11, RunCreditsMenu)
  menu:addButton(_("~!Widgets Demo"), x, ystep * 12, RunWidgetsMenu)
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



