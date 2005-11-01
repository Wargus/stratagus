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

function BosMenu()
   local menu
   local exitButton

   menu = MenuScreen()

   menu:add(backgroundWidget, 0, 0)

   exitButton = ImageButton("Exit", normalImage, pressedImage)
   exitButton:setActionCallback(function() menu:stop() end)
   menu:add(exitButton, Video.Width / 2 - 100, Video.Height - 100)

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

  local mapslist
  mapslist = {}
  local u
  u = 1
  local fileslist = ListDirectory("maps/")
  for i,f in fileslist do
    if(string.find(f, "^C.*%.smp$")) then
      print("Added smp file:" .. f .. "--" )
      mapslist[u] = f
      u = u + 1
    end
  end
  print(mapslist)
  print(mapslist[1])

  local bq
  bq = ListBoxWidget()
  bq:setList(mapslist)
  bq:setBaseColor(black)
  bq:setForegroundColor(clear)
  bq:setBackgroundColor(dark)
  bq:setFont(CFont:Get("game"))
  menu:add(bq, 300, 100)

  function startgamebutton(s)
    print("Starting map -------")
    StartMap("maps/" .. mapslist[bq:getSelected() + 1])
  end

  local bb
  bb = ImageButton("Start", normalImage, pressedImage)
  bb:setActionCallback(startgamebutton)
  menu:add(bb, 100, 300)

  menu:run()
end

function RunWidgetsMenu(s)
  local menu
  menu = BosMenu()

  b = Label("a label, sir.")
  b:setFont(CFont:Get("game"))
  menu:add(b, 20, 10)

  br = RadioButton("Platoon", "dumgroup", true)
  br:setActionCallback(function() print("one") end)
  br:setBaseColor(dark)
  br:setForegroundColor(clear)
  br:setBackgroundColor(dark)
  menu:add(br, 20, 50)
  be = RadioButton("Army", "dumgroup")
  be:setActionCallback(function() print("two") end)
  be:setBaseColor(dark)
  be:setForegroundColor(clear)
  be:setBackgroundColor(dark)
  menu:add(be, 100, 50)

  bz = TextField("text widget")
  bz:setActionCallback(function() print("field") end)
  bz:setFont(CFont:Get("game"))
  bz:setBaseColor(clear)
  bz:setForegroundColor(clear)
  bz:setBackgroundColor(dark)
  menu:add(bz, 20, 100)

  bf = Slider(0, 1)
  bf:setActionCallback(function() print("slider") end)
  menu:add(bf, 20, 150)
  bf:setWidth(60)
  bf:setHeight(20)
  bf:setBaseColor(dark)
  bf:setForegroundColor(clear)
  bf:setBackgroundColor(clear)


  ik = CGraphic:New("units/assault/ico_assault.png")
  ik:Load()
  bs = ImageWidget(ik)
  menu:add(bs, 20, 250)

  bw = DropDownWidget()
  bw:setFont(CFont:Get("game"))
  bw:setList({"line1", "line2"})
  bw:setActionCallback(function(s) print("dropdown ".. bw:getSelected()) end)
  bw:setBaseColor(dark)
  bw:setForegroundColor(clear)
  bw:setBackgroundColor(dark)
  menu:add(bw, 20, 350)

  win = Windows("Test", 200, 200)
  win:setBaseColor(dark)
  win:setForegroundColor(dark)
  win:setBackgroundColor(dark)
  menu:add(win, 450, 40)
  win2 = Windows("", 50, 50)
  win:add(win2, 0, 0)

  bv = ImageButton("Exit", normalImage, pressedImage)
  bv:setActionCallback(function() menu:stop() end)
  menu:add(bv, 400, 300)

  menu:run()
end

function RunMainMenu(s)
  menu = BosMenu() 

  sb = ImageButton("SubMenu", normalImage, pressedImage)
  sb:setActionCallback(RunSubMenu)
  menu:add(sb, 300, 250)

  wb = ImageButton("WidgetsMenu", normalImage, pressedImage)
  wb:setActionCallback(RunWidgetsMenu)
  menu:add(wb, 300, 200)

  sg = ImageButton("Start Game", normalImage, pressedImage)
  sg:setActionCallback(RunStartGameMenu)
  menu:add(sg, 300, 150)

  menu:run()
end


RunMainMenu()



