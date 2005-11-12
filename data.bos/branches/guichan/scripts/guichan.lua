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
dark = Color(8, 8, 38, 130)
clear = Color(200, 200, 120)
black = Color(0, 0, 0)

bckground = CGraphic:New("graphics/screens/menu.png")
bckground:Load()
bckground:Resize(Video.Width, Video.Height)
backgroundWidget = ImageWidget(bckground)

normalImage = CGraphic:New("graphics/button.png", 200, 24)
pressedImage = CGraphic:New("graphics/pressed.png", 200, 24)
normalImage:Load() -- FIXME remove when immediatly loaded
pressedImage:Load() -- idem

cbUncheckedNormalImage = CGraphic:New("general/checkbox-unchecked-normal.png")
cbUncheckedPressedImage = CGraphic:New("general/checkbox-unchecked-pressed.png")
cbCheckedNormalImage = CGraphic:New("general/checkbox-checked-normal.png")
cbCheckedPressedImage = CGraphic:New("general/checkbox-checked-pressed.png")
cbUncheckedNormalImage:Load()
cbUncheckedPressedImage:Load()
cbCheckedNormalImage:Load()
cbCheckedPressedImage:Load()

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

function BosMenu()
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
    local fileslist = ListDirectory(path)
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
    b = ImageCheckBox(caption)
    b:setUncheckedNormalImage(cbUncheckedNormalImage)
    b:setUncheckedPressedImage(cbUncheckedPressedImage)
    b:setCheckedNormalImage(cbCheckedNormalImage)
    b:setCheckedPressedImage(cbCheckedPressedImage)
    b:setActionCallback(callback)
    self:add(b, x, y)
    return b
  end

  exitButton = menu:addButton("~!Exit", 
        Video.Width / 2 - 100, Video.Height - 100, 
        function() menu:stop() end)
  return menu
end

-- Default configurations -------
Widget:setGlobalFont(CFont:Get("large"))


-- Define the different menus ----------

function RunSubMenu(s)
  local menu
  menu = BosMenu()
  menu:run()
end

function RunStartGameMenu(s)
  local menu
  menu = BosMenu()

  local browser = menu:addBrowser("maps/", "^.*%.smp$")
  function startgamebutton(s)
    print("Starting map -------")
    StartMap("maps/" .. browser:getSelectedItem())
    menu:stop()
  end
  menu:addButton("Start", 100, 350, startgamebutton)

  menu:run()
end


function RunReplayMenu(s)
  local menu
  menu = BosMenu()

  function startreplaybutton(s)
    print("Starting map -------")
    StartReplay("/home/feb/.stratagus-2.2/bos/logs/log_of_stratagus_0.log")
    menu:stop()
  end

  menu:addButton("~!Start", 100, 300, startreplaybutton)

  menu:run()
end

function RunWidgetsMenu(s)
  local menu
  local b
  menu = BosMenu()

  b = Label("a label, sir.")
  b:setFont(CFont:Get("game"))
  menu:add(b, 20, 10)

  b = RadioButton("Platoon", "dumgroup", true)
  b:setActionCallback(function() print("one") end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 20, 50)
  b = RadioButton("Army", "dumgroup")
  b:setActionCallback(function() print("two") end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 150, 50)

  b = TextField("text widget")
  b:setActionCallback(function() print("field") end)
  b:setFont(CFont:Get("game"))
  b:setBaseColor(clear)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 20, 100)

  b = Slider(0, 1)
  b:setActionCallback(function() print("slider") end)
  menu:add(b, 20, 150)
  b:setWidth(60)
  b:setHeight(20)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(clear)

  local ic = CGraphic:New("units/assault/ico_assault.png")
  ic:Load()
  b = ImageWidget(ic)
  menu:add(b, 20, 250)

  b = DropDownWidget()
  b:setFont(CFont:Get("game"))
  b:setList({"line1", "line2"})
  b:setActionCallback(function(s) print("dropdown ".. b:getSelected()) end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 20, 350)

  b = ImageCheckBox("ImageCheckBox")
  local cb = CGraphic:New("general/checkbox-unchecked-normal.png")
  cb:Load()
  b:setUncheckedNormalImage(cb)
  cb = CGraphic:New("general/checkbox-unchecked-pressed.png")
  cb:Load()
  b:setUncheckedPressedImage(cb)
  cb = CGraphic:New("general/checkbox-checked-normal.png")
  cb:Load()
  b:setCheckedNormalImage(cb)
  cb = CGraphic:New("general/checkbox-checked-pressed.png")
  cb:Load()
  b:setCheckedPressedImage(cb)
  menu:add(b, 20, 400)

  win = Windows("Test", 200, 200)
  win:setBaseColor(dark)
  win:setForegroundColor(dark)
  win:setBackgroundColor(dark)
  menu:add(win, 450, 40)
  win2 = Windows("", 50, 50)
  win:add(win2, 0, 0)

  b = ImageButton("SubMenu", normalImage, pressedImage)
  b:setActionCallback(RunSubMenu)
  menu:add(b, 300, 250)

  menu:run()
end

function RunSoundOptionsMenu(s)
  local menu
  local b
  local offx = (Video.Width - 352) / 2
  local offy = (Video.Height - 352) / 2

  menu = BosMenu()

  b = Label("Sound Options")
  b:setFont(CFont:Get("large"))
  menu:add(b, offx + 176, offy + 11)

  b = Label("Effects Volume")
  b:setFont(CFont:Get("game"))
  menu:add(b, offx + 16, offy + 36 * 1)

  soundslider = Slider(0, 255)
  soundslider:setValue(GetSoundVolume())
  soundslider:setActionCallback(function() SetSoundVolume(soundslider:getValue()) end)
  soundslider:setWidth(198)
  soundslider:setHeight(18)
  soundslider:setBaseColor(dark)
  soundslider:setForegroundColor(clear)
  soundslider:setBackgroundColor(clear)
  menu:add(soundslider, offx + 32, offy + 36 * 1.5)

  b = Label("min")
  b:setFont(CFont:Get("game"))
  menu:add(b, offx + 44, offy + 36 * 2 + 6)
  
  b = Label("max")
  b:setFont(CFont:Get("game"))
  menu:add(b, offx + 218, offy + 36 * 2 + 6)

  b = menu:addCheckBox("Enabled", offx + 240, offy + 36 * 1.5,
    function() print("checkbox1") end)
  b:setFont(CFont:Get("large"))

  b = Label("Music Volume")
  b:setFont(CFont:Get("game"))
  menu:add(b, offx + 16, offy + 36 * 3)

  local musicslider = Slider(0, 255)
  musicslider:setValue(GetMusicVolume())
  musicslider:setActionCallback(function() SetMusicVolume(musicslider:getValue()) end)
  musicslider:setWidth(198)
  musicslider:setHeight(18)
  musicslider:setBaseColor(dark)
  musicslider:setForegroundColor(clear)
  musicslider:setBackgroundColor(clear)
  menu:add(musicslider, offx + 32, offy + 36 * 3.5)

  b = Label("min")
  b:setFont(CFont:Get("game"))
  menu:add(b, offx + 44, offy + 36 * 4 + 6)
  
  b = Label("max")
  b:setFont(CFont:Get("game"))
  menu:add(b, offx + 218, offy + 36 * 4 + 6)

  b = menu:addCheckBox("Enabled", offx + 240, offy + 36 * 3.5,
    function() print("checkbox2") end)
  b:setFont(CFont:Get("large"))

  b = Label("CD Volume")
  b:setFont(CFont:Get("game"))
  menu:add(b, offx + 16, offy + 36 * 5)

  b = Slider(0, 1)
  b:setActionCallback(function() print("slider") end)
  b:setWidth(198)
  b:setHeight(18)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(clear)
  menu:add(b, offx + 32, offy + 36 * 5.5)

  b = Label("min")
  b:setFont(CFont:Get("game"))
  menu:add(b, offx + 44, offy + 36 * 6 + 6)
  
  b = Label("max")
  b:setFont(CFont:Get("game"))
  menu:add(b, offx + 218, offy + 36 * 6 + 6)

  b = menu:addCheckBox("Enabled", offx + 240, offy + 36 * 5.5,
    function() print("checkbox3") end)
  b:setFont(CFont:Get("large"))

  menu:addButton("~!OK", offx + 176 - (106 / 2), offy + 352 - 11 - 27,
    function() end)

  menu:run()
end

function RunCampaignsMenu(s)
  local menu
  local b

  menu = BosMenu()

  b = Label("Campaigns")
  b:setFont(CFont:Get("large"))
  menu:add(b, 176, 11)

  local browser = menu:addBrowser("campaigns/", "^%a")
  function startgamebutton(s)
    print("Starting campaign")
    Load("campaigns/" .. browser:getSelectedItem() .. "/campaign.lua")
    menu:stop()
  end
  menu:addButton("Start", 100, 300, startgamebutton)

  menu:run()
end

function RunLoadGameMenu(s)
  local menu
  local b

  menu = BosMenu()

  b = Label("Load Game")
  b:setFont(CFont:Get("large"))
  menu:add(b, 176, 11)

  menu:addButton("~!OK", 176 - (106 / 2), 352 - 11 - 27, function() end)

  menu:run()
end

function RunOptionsMenu(s)
  local menu
  menu = BosMenu()

  menu:addButton("~!Sound", 300, 140, RunSoundOptionsMenu)

  menu:run()
end

function RunEditorMenu(s)
  local menu
  menu = BosMenu()

  local browser = menu:addBrowser("maps/", "^.*%.smp$")
  function starteditorbutton(s)
    print("Starting map -------")
    StartEditor("test.smp")
    menu:stop()
  end

  menu:addButton("Start Editor", 100, 300, starteditorbutton)

  menu:run()
end

function RunMainMenu(s)
  local b
  local menu = BosMenu() 

  menu:addButton("~!Start Game", 300, 140, RunStartGameMenu)
  menu:addButton("~!Widgets Demo", 300, 180, RunWidgetsMenu)
  menu:addButton("Start ~!Editor", 300, 220, RunEditorMenu)
  menu:addButton("~!Options", 300, 260, RunOptionsMenu)
  menu:addButton("~!MultiPlayer", 300, 300, RunOptionsMenu)
  menu:addButton("~!Campaigns", 300, 340, RunCampaignsMenu)
  menu:addButton("~!Load Game", 300, 380, RunLoadGameMenu)
  menu:addButton("Show ~!Replay", 300, 420, RunReplayMenu)
  menu:addButton("~!Credits", 300, 460, RunSubMenu)

  menu:run()
end


RunMainMenu()



