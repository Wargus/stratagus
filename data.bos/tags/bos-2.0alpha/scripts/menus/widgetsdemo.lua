--            ____
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  )
--        /_____/\____/____/
--
--      Invasion - Battle of Survival
--       A GPL'd futuristic RTS game
--
--      widgetsdemo.lua - Demonstration of the UI widgets available.
--
--      (c) Copyright 2005-2006 by Francois Beerten
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
--      $Id: guichan.lua 304 2005-12-18 13:13:15Z feb $

function RunWidgetsMenu(s)
  local menu
  local b
  menu = BosMenu()

  b = Label("Translucent widgets")
  b:setFont(CFont:Get("large"))
  b:adjustSize();
  menu:add(b, 20, 10)

  menu:addButton("SubMenu", 0, 30, 50, RunSubMenu)

  b = TextField("text input")
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

  b = RadioButton("Platoon", "dumgroup", true)
  b:setActionCallback(function() print("one") end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 20, 180)
  b = RadioButton("Army", "dumgroup")
  b:setActionCallback(function() print("two") end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 150, 180)

  menu:addCheckBox("CheckBox", 20, 210, function(s) print("checked ?") end)

  local ic = CGraphic:New("units/assault/ico_assault.png")
  ic:Load()
  b = ImageWidget(ic)
  menu:add(b, 20, 250)

  local sb = StatBoxWidget(200, 20)
  sb:setCaption("progress")
  sb:setPercent(45)
  menu:add(sb, 20, 300)
  sb:setBackgroundColor(dark)

  b = DropDownWidget()
  b:setFont(CFont:Get("game"))
  b:setList({"line1", "line2"})
  b:setActionCallback(function(s) print("dropdown ".. b:getSelected()) end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 20, 350)

  win = Windows("Test", 70, 70)
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
  
  
  x = MultiLineLabel("a bc def ghij klmnop qrstuvw wwwwwwwwwwwwwwwwwwwwwwwwwwwwww test\na t\na c b")
  x:setFont(Fonts["large"])
  x:setAlignment(MultiLineLabel.CENTER)
  x:setVerticalAlignment(MultiLineLabel.CENTER)
  x:setLineWidth(100)
  x:adjustSize()
  x:setBorderSize(1)
  x:setHeight(200)
  menu:add(x, 330, 270)

  menu:run()
end







