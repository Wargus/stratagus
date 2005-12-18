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
--      (c) Copyright 2005 by François Beerten
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

function BosMenu(title)
  local menu
  local exitButton

  menu = MenuScreen()

  menu:add(backgroundWidget, 0, 0)

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

  function menu:addBrowser(path, filter)
    local mapslist = {}
    local u = 1
    local fileslist = ListFilesInDirectory(path)
    local i
    local f
    for i,f in fileslist do
      if(string.find(f, filter)) then
        print("Added item:" .. f .. "--" )
        mapslist[u] = f
        u = u + 1
      end
    end

    local bq
    bq = ListBoxWidget(300, 200)
    bq:setList(mapslist)
    bq:setBaseColor(black)
    bq:setForegroundColor(clear)
    bq:setBackgroundColor(dark)
    bq:setFont(CFont:Get("game"))
    menu:add(bq, 300, 100)
    
    bq.itemslist = mapslist
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
    b:setActionCallback(callback)
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

difficulty = 5
mapresources = 5
startingresources = 5

function RunStartGameMenu(s)
  local menu
  menu = BosMenu(_("Start Game"))

  menu:writeText(_("Players:"), 20, 80)
  players = menu:writeText(_("No map"), 80, 80)
  menu:writeText(_("Description:"), 20, 120)
  descr = menu:writeText(_("No map"),40, 160)

  local fow = menu:addCheckBox(_("Fog of war"), 25, 200, function() end)
  fow:setMarked(true)
  local revealmap = menu:addCheckBox(_("Reveal map"), 25, 230, function() end)
  
  menu:writeText(_("Difficulty:"), 20, Video.Height*11/20)
  menu:addDropDown({_("easy"), _("normal"), _("hard")}, 120, Video.Height*11/20 + 7,
      function(dd) difficulty = (5 - dd:getSelected()*2) end)
  menu:writeText(_("Map richness:"), 20, Video.Height*12/20)
  menu:addDropDown({_("high"), _("normal"), _("low")}, 140, Video.Height*12/20 + 7,
      function(dd) mapresources = (5 - dd:getSelected()*2) end)
  menu:writeText(_("Starting resources:"), 20, Video.Height*13/20)
  menu:addDropDown({_("high"), _("normal"), _("low")}, 170, Video.Height*13/20 + 7,
      function(dd) startingresources = (5 - dd:getSelected()*2) end)


  local OldPresentMap = PresentMap
  PresentMap = function(description, nplayers, w, h, id)
      print(description)
      players:setCaption(""..nplayers)
      descr:setCaption(description)
      OldPresentMap(description, nplayers, w, h, id)
  end
 
  local browser = menu:addBrowser("maps/", "^.*%.smp$")
  local function cb(s)
    print(browser:getSelectedItem())
    Load("maps/" .. browser:getSelectedItem())
  end
  browser:setActionCallback(cb)

  local function startgamebutton(s)
    print("Starting map -------")
    SetFogOfWar(fow:isMarked())
    if revealmap:isMarked() == true then
       RevealMap()
    end
    StartMap("maps/" .. browser:getSelectedItem())
    menu:stop()
  end
  menu:addButton(_("Start"), 20, Video.Height - 100, startgamebutton)

  menu:run()
  PresentMap = OldPresentMap
end


function RunReplayMenu(s)
  local menu
  menu = BosMenu(_("Show a Replay"))

  local browser = menu:addBrowser("~logs/", ".log$")

  function startreplaybutton(s)
    print("Starting map -------")
    StartReplay("~logs/" .. browser:getSelectedItem())
    menu:stop()
  end

  menu:addButton(_("~!Start"), 100, 300, startreplaybutton)

  menu:run()
end

function RunWidgetsMenu(s)
  local menu
  local b
  menu = BosMenu()

  local normalImage = CGraphic:New("graphics/button.png", 200, 24)
  local pressedImage = CGraphic:New("graphics/pressed.png", 200, 24)
  normalImage:Load() -- FIXME remove when immediatly loaded
  pressedImage:Load() -- idem

  b = Label(_("Translucent widgets"))
  b:setFont(CFont:Get("large"))
  b:adjustSize();
  menu:add(b, 20, 10)

  menu:addButton(_("SubMenu"), 30, 50, RunSubMenu)

  b = TextField(_("text input"))
  b:setActionCallback(function() print("field") end)
  b:setFont(CFont:Get("game"))
  b:setBaseColor(clear)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 20, 100)

  b = Slider(0, 1)
  b:setActionCallback(function() print("slider") end)
  menu:add(b, 20, 140)
  b:setWidth(60)
  b:setHeight(20)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(clear)

  b = RadioButton(_("Platoon"), "dumgroup", true)
  b:setActionCallback(function() print("one") end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 20, 180)
  b = RadioButton(_("Army"), "dumgroup")
  b:setActionCallback(function() print("two") end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 150, 180)

  menu:addCheckBox(_("CheckBox"), 20, 210, function(s) print("checked ?") end)

  local ic = CGraphic:New("units/assault/ico_assault.png")
  ic:Load()
  b = ImageWidget(ic)
  menu:add(b, 20, 250)

  local sb = StatBoxWidget(200, 20)
  sb.caption = "progress"
  sb.percent = 45
  menu:add(sb, 20, 300)
  sb:setBackgroundColor(dark)

  b = DropDownWidget()
  b:setFont(CFont:Get("game"))
  b:setList({_("line1"), _("line2")})
  b:setActionCallback(function(s) print("dropdown ".. b:getSelected()) end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 20, 350)

  win = Windows(_("Test"), 70, 70)
  win:setBaseColor(dark)
  win:setForegroundColor(dark)
  win:setBackgroundColor(dark)
  menu:add(win, 40, 450)
  win2 = Windows("", 50, 50)
  win:add(win2, 0, 0)

  local sw = ScrollingWidget(200, 50)
  menu:add(sw, 20, 380)
  sw:setBackgroundColor(dark)
  sw:setActionCallback(function() sw:restart() end)
  for i,f in {"Jarod", "was", "here", " ", ":)"} do
    sw:add(Label(f), 0, 20 * i + 50)
  end
  

  b = Label(_("Image based widgets"))
  b:setFont(CFont:Get("large"))
  b:adjustSize();
  menu:add(b, 330, 10)

  b = ImageButton(_("SubMenu"))
  b:setNormalImage(normalImage)
  b:setPressedImage(pressedImage)
  b:setActionCallback(RunSubMenu)
  menu:add(b, 330, 50)

  b = ImageCheckBox(_("ImageCheckBox"))
  local cb = CGraphic:New("ui/widgets/checkbox-unchecked-normal.png")
  cb:Load()
  b:setUncheckedNormalImage(cb)
  cb = CGraphic:New("ui/widgets/checkbox-unchecked-pressed.png")
  cb:Load()
  b:setUncheckedPressedImage(cb)
  cb = CGraphic:New("ui/widgets/checkbox-checked-normal.png")
  cb:Load()
  b:setCheckedNormalImage(cb)
  cb = CGraphic:New("ui/widgets/checkbox-checked-pressed.png")
  cb:Load()
  b:setCheckedPressedImage(cb)
  menu:add(b, 330, 210)

  bs = ImageSlider(0,1)
  local wimg = CGraphic:New("ui/widgets/undef_square.png")
  wimg:Load()
  bs:setMarkerImage(wimg)
  local wimg2 = CGraphic:New("ui/widgets/scrollhorizontal_default.png")
  wimg2:Load()
  bs:setBackgroundImage(wimg2)
  bs:setWidth(100)
  bs:setHeight(20)
  menu:add(bs, 330, 140)



  menu:run()
end


function RunCampaignsMenu(s)
  local menu
  local b

  menu = BosMenu(_("List of Campaigns"))

  local browser = menu:addBrowser("campaigns/", "^%a")
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
  local browser = menu:addBrowser("~save", ".sav.gz$")
    function startgamebutton(s)
    print("Starting saved game")
    StartSavedGame("~save/" .. browser:getSelectedItem())
    menu:stop()
  end
  menu:addButton(_("Start"), 100, 300, startgamebutton)

  menu:run()
end


function RunCreditsMenu(s)
  local menu
  local b

  local credits
  credits = {
     _("Graphics:"),
     "  Tina Petersen Jensen",
     "  Soeren Soendergaard Jensen",
     "  TimberDragon",
     "  Frank Loeffler",
     "  Francois Beerten",
     "",
     _("Scripting:"),
     "  Francois Beerten",
     "  Lois Taulelle",
     "",
     _("Maps and campaigns:"),
     "  Lois Taulelle",
     "  Francois Beerten",
     "",
     _("Sound:"),
     "  Tina Petersen",
     "  Brian Pedersen",
     " ",
     _("powered by STRATAGUS"),
     _("Stratagus Programmers:"),
     "  Andreas 'Ari' Arens",
     "  Lutz 'Johns' Sammer",
     "  Edgar 'Froese' Toernig",
     "  Jimmy Salmon",
     "  Nehal Mistry",
     "  Russell Smith",
     "  Francois Beerten",
     "  Joris Dauphin",
     "  Mark Pazolli",
     "  Valery Shchedrin",
     "  Iftikhar Rathore",
     "  Charles K Hardin",
     "  Fabrice Rossi",
     "  DigiCat",
     "  Josh Cogliati",
     "  Patrick Mullen",
     "  Vladi Belperchinov-Shabanski",
     "  Cris Daniluk",
     "  Patrice Fortier",
     "  FT Rathore",
     "  Trent Piepho",
     "  Jon Gabrielson",
     "  Lukas Hejtmanek",
     "  Steinar Hamre",
     "  Ian Farmer",
     "  Sebastian Drews",
     "  Jarek Sobieszek",
     "  Anthony Towns",
     "  Stefan Dirsch",
     "  Al Koskelin",
     "  George J. Carrette",
     "  Dirk 'Guardian' Richartz",
     "  Michael O'Reilly",
     "  Dan Hensley",
     "  Sean McMillian",
     "  Mike Earl",
     "  Ian Turner",
     "  David Slimp",
     "  Iuri Fiedoruk",
     "  Luke Mauldin",
     "  Nathan Adams",
     "  Stephan Rasenberger",
     "  Dave Reed",
     "  Josef Spillner",
     "  James Dessart",
     "  Jan Uerpmann",
     "  Aaron Berger",
     "  Latimerius",
     "  Antonis Chaniotis",
     "  Samuel Hays",
     "  David Martinez Moreno",
     "  Flavio Silvestrow",
     "  Daniel Burrows",
     "  Dave Turner",
     "  Ben Hines",
     "  Kachalov Anton",
     _("Patches"),
     "  Martin Renold",
     "  Martin Hajduch",
     "  Jeff Binder",
     "  Ludovic",
     "  Juan Pablo",
     "  Phil Hannent",
     "  Alexander MacLean",
     "",
     _("Stratagus Media Project Graphics"),
      -- land construction site-summer-01.png; big_fire.png; explosion.png; green_cross.png; winter, big fire
     "  Paolo D'Inca", 
     "  Rick Elliot", -- cursor arrows
     "  Chris Hopp", -- small_fire.png
     "",
     "",
     _("The Bos and the Stratagus Team thanks all the people who have contributed"),
     _("patches, bug reports, ideas.")
  }

  menu = BosMenu(_("Battle of Survival Credits"))

  local sw = ScrollingWidget(400, Video.Height * 12 / 20)
  menu:add(sw, Video.Width / 2 - 200, Video.Height / 20 * 3)
  sw:setBackgroundColor(dark)
  sw:setActionCallback(function() sw:restart() end)
  for i,f in credits do
    sw:add(Label(f), 50, 20 * i + 50)
  end

  menu:run()
end

function RunMultiPlayerMenu(s)
  local menu
  local b

  menu = BosMenu(_("MultiPlayer"))
  menu:writeText(_("Coming soon ..."), Video.Width/2 - 100, Video.Height/3)

  menu:run()
end

function RunEditorMenu(s)
  local menu
  menu = BosMenu(_("Editor"))

  local browser = menu:addBrowser("maps/", "^.*%.smp$")
  function starteditorbutton(s)
    print("Starting map -------")
    StartEditor("test.smp")
    menu:stop()
  end

  menu:addButton(_("Start Editor"), 100, 300, starteditorbutton)

  menu:run()
end

Load("scripts/menus/options.lua")

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



