--            ____
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  )
--        /_____/\____/____/
--
--      Invasion - Battle of Survival
--       A GPL'd futuristic RTS game
--
--      game.lua - In-game menus.
--
--      (c) Copyright 2006 by Jimmy Salmon
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

function BosGameMenu()
  local menu = MenuScreen()
  menu:setSize(256, 288)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)
  menu:setBorderSize(1)
  menu:setOpaque(true)
  menu:setBaseColor(dark)

  function menu:addLabel(text, x, y, font)
    local label = Label(text)
    if (font == nil) then font = Fonts["large"] end
    label:setFont(font)
    label:adjustSize()
    self:add(label, x - label:getWidth() / 2, y)
    return label
  end

  function menu:addSmallButton(caption, x, y, callback)
    local b = ButtonWidget(caption)
    b:setActionCallback(callback)
    b:setSize(106, 28)
    b:setBackgroundColor(dark)
    b:setBaseColor(dark)
    menu:add(b, x, y)
    return b
  end

  function menu:addButton(caption, x, y, callback)
    local b = ButtonWidget(caption)
    b:setActionCallback(callback)
    b:setSize(224, 28)
    b:setBackgroundColor(dark)
    b:setBaseColor(dark)
    menu:add(b, x, y)
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
      if(string.find(f, filter)) then
        mapslist[u] = f
        u = u + 1
      end
    end

    local bq = self:addListBox(x, y, w, h, mapslist)
    bq.getSelectedItem = function(self)
      if self:getSelected() < 0 then
         return self.itemslist[1]
      end
      return self.itemslist[self:getSelected() + 1]
    end

    return bq
  end

  function menu:addTextInputField(text, x, y, w)
    local b = TextField(text)
    b:setActionCallback(function() print("field") end)
    b:setFont(Fonts["game"])
    b:setBaseColor(clear)
    b:setForegroundColor(clear)
    b:setBackgroundColor(dark)
    b:setSize(w, 18)
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

  return menu
end


function RunGameMenu(s)
  local menu = BosGameMenu()

  menu:addLabel(_("Game Menu"), 128, 11)
  -- FIXME: disable for multiplayer or replay
  menu:addSmallButton(_("Save (~<F11~>)"), 16, 40,
    function() RunSaveMenu() end)
  -- FIXME: disable for multiplayer
  menu:addSmallButton(_("Load (~<F12~>)"), 16 + 12 + 106, 40,
    function() RunLoadMenu() end)
  menu:addButton(_("Options (~<F5~>)"), 16, 40 + (36 * 1),
    function() RunGameOptionsMenu() end)
  menu:addButton(_("Help (~<F1~>)"), 16, 40 + (36 * 2),
    function() RunHelpMenu() end)
  menu:addButton(_("~!Objectives"), 16, 40 + (36 * 3),
    function() RunObjectivesMenu() end)
  menu:addButton(_("~!End Scenario"), 16, 40 + (36 * 4),
    function() RunEndScenarioMenu() end)
  menu:addButton(_("Return to Game (~<Esc~>)"), 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunSaveMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Save Game"), 128, 11)

  local t = menu:addTextInputField("game.sav", 16, 40, 224)

  local browser = menu:addBrowser("~save", ".sav.gz$", 16, 70, 224, 166)
  local function cb(s)
    t:setText(browser:getSelectedItem())
  end
  browser:setActionCallback(cb)

  menu:addSmallButton(_("Save"), 16, 248,
    -- FIXME: use a confirm menu if the file exists already
    function()
      SaveGame(t:getText())
      UI.StatusLine:Set("Saved game to: " .. t:getText())
      menu:stop()
    end)

  menu:addSmallButton(_("Cancel"), 16 + 12 + 106, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunLoadMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Load Game"), 128, 11)

  local browser = menu:addBrowser("~save", ".sav.gz$", 16, 40, 224, 200)
  local function cb(s)
    print(browser:getSelectedItem())
  end
  browser:setActionCallback(cb)

  menu:addSmallButton(_("Load"), 16, 248,
    function() StartSavedGame("~save/"..browser:getSelectedItem()) end)

  menu:addSmallButton(_("Cancel"), 16 + 12 + 106, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunGameOptionsMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Game Options"), 128, 11)
  menu:addButton(_("Sound (~<F7~>)"), 16, 40 + (36 * 0),
    function() RunGameSoundOptionsMenu() end)
  menu:addButton(_("Speeds (~<F8~>)"), 16, 40 + (36 * 1),
    function() RunGameSpeedOptionsMenu() end)
  menu:addButton(_("Preferences (~<F9~>)"), 16, 40 + (36 * 2),
    function() RunPreferencesMenu() end)
  menu:addButton(_("~!Diplomacy"), 16, 40 + (36 * 3),
    function() RunDiplomacyMenu() end)
  menu:addButton(_("Previous (~<Esc~>)"), 128 - (224 / 2), 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunGameSoundOptionsMenu()
  --FIXME:
end

function RunGameSpeedOptionsMenu()
  local menu = BosGameMenu()
  local l

  menu:addLabel(_("Speed Settings"), 128, 11)

  l = Label(_("Game Speed"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 16, 36 * 1)

  local gamespeed = {}
  gamespeed = menu:addSlider(15, 75, 198, 18, 32, 36 * 1.5,
    function() SetGameSpeed(gamespeed:getValue()) end)
  gamespeed:setValue(GetGameSpeed())

  l = Label(_("slow"))
  l:setFont(Fonts["small"])
  l:adjustSize()
  menu:add(l, 34, (36 * 2) + 6)
  l = Label(_("fast"))
  l:setFont(Fonts["small"])
  l:adjustSize()
  menu:add(l, 230, (36 * 2) + 6)

  menu:addSmallButton(_("~!OK"), 128 - (106 / 2), 248,
    function()
      preferences.GameSpeed = GetGameSpeed()
      SavePreferences()
      menu:stop()
    end)

  menu:run(false)
end

function RunPreferencesMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Preferences"), 128, 11)
  local fog = {}
  -- FIXME: disable checkbox for multiplayer or replays
  fog = menu:addCheckBox(_("Fog of War Enabled"), 16, 36 * 1,
    function() SetFogOfWar(fog:isMarked()) end)
  fog:setMarked(GetFogOfWar())
  local ckey = {}
  ckey = menu:addCheckBox(_("Show command key"), 16, 36 * 2,
    function() UI.ButtonPanel.ShowCommandKey = ckey:isMarked() end)
  ckey:setMarked(UI.ButtonPanel.ShowCommandKey)
  menu:addSmallButton(_("~!OK"), 128 - (106 / 2), 245,
    function()
      preferences.FogOfWar = GetFogOfWar()
      preferences.ShowCommandKey = UI.ButtonPanel.ShowCommandKey
      SavePreferences()
      menu:stop()
    end)

  menu:run(false)
end

function RunDiplomacyMenu()
  local menu = BosGameMenu()
  menu:setSize(384, 384)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)

  menu:addLabel(_("Diplomacy"), 192, 11)

  menu:addLabel(_("Allied"), 136, 30, Fonts["game"])
  menu:addLabel(_("Enemy"), 208, 30, Fonts["game"])
  menu:addLabel(_("Shared Vision"), 310, 30, Fonts["game"])

  local allied = {}
  local enemy = {}
  local sharedvision = {}
  local j = 0

  for i=0,14 do
    if (Players[i].Type ~= PlayerNobody and ThisPlayer.Index ~= i) then
      j = j + 1

      local l = Label(Players[i].Name)
      l:setFont(Fonts["game"])
      l:adjustSize()
      menu:add(l, 16, (21 * j) + 27)

      -- FIXME: disable checkboxes in replays or if on the same team

      local alliedcb = {}
      local enemycb = {}
      local sharedvisioncb = {}

      alliedcb = menu:addCheckBox("", 126, (21 * j) + 24,
        function()
          if (alliedcb:isMarked() and enemycb:isMarked()) then
            enemycb:setMarked(false)
          end
        end)
      alliedcb:setMarked(ThisPlayer:IsAllied(Players[i]))
      allied[j] = alliedcb
      allied[j].index = i

      enemycb = menu:addCheckBox("", 198, (21 * j) + 24,
        function()
          if (alliedcb:isMarked() and enemycb:isMarked()) then
            alliedcb:setMarked(false)
          end
        end)
      enemycb:setMarked(ThisPlayer:IsEnemy(Players[i]))
      enemy[j] = enemycb

      sharedvisioncb = menu:addCheckBox("", 300, (21 * j) + 24,
        function() end)
      sharedvisioncb:setMarked(ThisPlayer:IsSharedVision(Players[i]))
      sharedvision[j] = sharedvisioncb
    end
  end

  menu:addSmallButton(_("~!OK"), 75, 384 - 40,
    function()
      for j=1,table.getn(allied) do
        local i = allied[j].index

        -- allies
        if (allied[j]:isMarked() and enemy[j]:isMarked() == false) then
          if (ThisPlayer:IsAllied(Players[i]) == false or
             ThisPlayer:IsEnemy(Players[i])) then
            SetDiplomacy(ThisPlayer.Index, "allied", i)
          end
        end

        -- enemies
        if (allied[j]:isMarked() == false and enemy[j]:isMarked()) then
          if (ThisPlayer:IsAllied(Players[i]) or
             ThisPlayer:IsEnemy(Players[i]) == false) then
            SetDiplomacy(ThisPlayer.Index, "enemy", i)
          end
        end

        -- neutral
        if (allied[j]:isMarked() == false and enemy[j]:isMarked() == false) then
          if (ThisPlayer:IsAllied(Players[i]) or
             ThisPlayer:IsEnemy(Players[i])) then
            SetDiplomacy(ThisPlayer.Index, "neutral", i)
          end
        end

        -- crazy
        if (allied[j]:isMarked() and enemy[j]:isMarked()) then
          if (ThisPlayer:IsAllied(Players[i]) == false or
             ThisPlayer:IsEnemy(Players[i]) == false) then
            SetDiplomacy(ThisPlayer.Index, "crazy", i)
          end
        end

        -- shared vision
        if (sharedvision[i]:isMarked()) then
          if (ThisPlayer:IsSharedVision(Players[i]) == false) then
            SetSharedVision(ThisPlayer.Index, true, i)
          end
        else
          if (ThisPlayer:IsSharedVision(Players[i])) then
            SetSharedVision(ThisPlayer.Index, false, i)
          end
        end
      end
      menu:stop()
    end)
  menu:addSmallButton(_("~!Cancel"), 195, 384 - 40, function() menu:stop() end)

  menu:run(false)
end

function RunEndScenarioMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("End Scenario"), 128, 11)
  -- FIXME: disable for multiplayer
  menu:addButton(_("~!Restart Scenario"), 16, 40 + (36 * 0),
    function() RunRestartConfirmMenu() end)
  menu:addButton(_("~!Surrender"), 16, 40 + (36 * 1),
    function() RunSurrenderConfirmMenu() end)
  menu:addButton(_("~!Quit to Menu"), 16, 40 + (36 * 2),
    function() RunQuitToMenuConfirmMenu() end)
  menu:addButton(_("E~!xit Program"), 16, 40 + (36 * 3),
    function() RunExitConfirmMenu() end)
  menu:addButton(_("Previous (~<Esc~>)"), 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunRestartConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to restart"), 128, 11 + (24 * 1))
  menu:addLabel(_("the scenario?"), 128, 11 + (24 * 2))
  menu:addButton(_("~!Restart Scenario"), 16, 11 + (24 * 3) + 29,
    function() StopGame(GameRestart); menu:stopAll() end)
  menu:addButton(_("Cancel (~<Esc~>)"), 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunSurrenderConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to surrender"), 128, 11 + (24 * 1))
  menu:addLabel(_("to your enemies?"), 128, 11 + (24 * 2))
  menu:addButton(_("~!Surrender"), 16, 11 + (24 * 3) + 29,
    function() StopGame(GameDefeat); menu:stopAll() end)
  menu:addButton(_("Cancel (~<Esc~>)"), 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunQuitToMenuConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to quit to"), 128, 11 + (24 * 1))
  menu:addLabel(_("the main menu?"), 128, 11 + (24 * 2))
  menu:addButton(_("~!Quit to Menu"), 16, 11 + (24 * 3) + 29,
    function() StopGame(GameQuitToMenu); menu:stopAll() end)
  menu:addButton(_("Cancel (~<Esc~>)"), 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunExitConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to exit"), 128, 11 + (24 * 1))
  menu:addLabel(_("Stratagus?"), 128, 11 + (24 * 2))
  menu:addButton(_("E~!xit Program"), 16, 11 + (24 * 3) + 29,
    function() Exit(0) end)
  menu:addButton(_("Cancel (~<Esc~>)"), 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunHelpMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Help Menu"), 128, 11)
  menu:addButton(_("Keystroke ~!Help"), 16, 40 + (36 * 0),
    function() RunKeystrokeHelpMenu() end)
  menu:addButton(_("Stratagus ~!Tips"), 16, 40 + (36 * 1),
    function() RunTipsMenu() end)
  menu:addButton(_("Previous (~<Esc~>)"), 128 - (224 / 2), 248,
    function() menu:stop() end)

  menu:run(false)
end

local keystrokes = {
  {"Alt-F", "- toggle full screen"},
  {"Alt-G", "- toggle grab mouse"},
  {"Ctrl-S", "- mute sound"},
  {"Ctrl-M", "- mute music"},
  {"+", "- increase game speed"},
  {"-", "- decrease game speed"},
  {"Ctrl-P", "- pause game"},
  {"PAUSE", "- pause game"},
  {"PRINT", "- make screen shot"},
  {"Alt-H", "- help menu"},
  {"Alt-R", "- restart scenario"},
  {"Alt-Q", "- quit to main menu"},
  {"Alt-X", "- quit game"},
  {"Alt-B", "- toggle expand map"},
  {"Alt-M", "- game menu"},
  {"ENTER", "- write a message"},
  {"SPACE", "- goto last event"},
  {"TAB", "- hide/unhide terrain"},
  {"Ctrl-T", "- track unit"},
  {"Alt-I", "- find idle peon"},
  {"Alt-C", "- center on selected unit"},
  {"Alt-V", "- next view port"},
  {"Ctrl-V", "- previous view port"},
  {"^", "- select nothing"},
  {"#", "- select group"},
  {"##", "- center on group"},
  {"Ctrl-#", "- define group"},
  {"Shift-#", "- add to group"},
  {"Alt-#", "- add to alternate group"},
  {"F2-F4", "- recall map position"},
  {"Shift F2-F4", "- save map postition"},
  {"F5", "- game options"},
  {"F7", "- sound options"},
  {"F8", "- speed options"},
  {"F9", "- preferences"},
  {"F10", "- game menu"},
  {"F11", "- save game"},
  {"F12", "- load game"},
}

function RunKeystrokeHelpMenu()
  local menu = BosGameMenu()
  menu:setSize(352, 352)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)

  local c = Container()
  c:setOpaque(false)

  for i=1,table.getn(keystrokes) do
    local l = Label(keystrokes[i][1])
    l:setFont(Fonts["game"])
    l:adjustSize()
    c:add(l, 0, 20 * (i - 1))
    local l = Label(keystrokes[i][2])
    l:setFont(Fonts["game"])
    l:adjustSize()
    c:add(l, 80, 20 * (i - 1))
  end

  local s = ScrollArea()
  c:setSize(320 - s:getScrollbarWidth(), 20 * table.getn(keystrokes))
  s:setBaseColor(dark)
  s:setBackgroundColor(dark)
  s:setForegroundColor(clear)
  s:setSize(320, 216)
  s:setContent(c)
  menu:add(s, 16, 60)

  menu:addLabel(_("Keystroke Help Menu"), 352 / 2, 11)
  menu:addButton(_("Previous (~<Esc~>)"), (352 / 2) - (224 / 2), 352 - 40,
    function() menu:stop() end)

  menu:run(false)
end

local tips = {
  "You can select all of your currently visible units of the same type by holding down the CTRL key and selecting a unit or by \"double clicking\" on a unit.",
  "The more engineers you have collecting resources, the faster your economy will grow.",

  "Use your engineers to repair damaged buildings.",
  "Explore your surroundings early in the game.",


  "Keep all engineers working. Use ALT-I to find idle engineers.",
  "You can make units automatically cast spells by selecting a unit, holding down CTRL and clicking on the spell icon.  CTRL click again to turn off.",

  -- Shift tips
  "You can give an unit an order which is executed after it finishes the current work, if you hold the SHIFT key.",
  "You can give way points, if you press the SHIFT key, while you click right for the move command.",
  "You can order an engineer to build one building after the other, if you hold the SHIFT key, while you place the building.",
  "You can build many of the same building, if you hold the ALT and SHIFT keys while you place the buildings.",

  "Use CTRL-V or ALT-V to cycle through the viewport configuration, you can than monitor your base and lead an attack.",

  "Know a useful tip?  Then add it here!",
}

function RunTipsMenu()
  local menu = BosGameMenu()
  menu:setSize(384, 256)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)

  menu:addLabel(_("Stratagus Tips"), 192, 11)

  local l = MultiLineLabel()
  l:setFont(Fonts["game"])
  l:setSize(356, 144)
  l:setLineWidth(356)
  menu:add(l, 14, 36)
  function l:prevTip()
    preferences.TipNumber = preferences.TipNumber - 1
    if (preferences.TipNumber < 1) then
      preferences.TipNumber = table.getn(tips)
    end
    SavePreferences()
  end
  function l:nextTip()
    preferences.TipNumber = preferences.TipNumber + 1
    if (preferences.TipNumber > table.getn(tips)) then
      preferences.TipNumber = 1
    end
    SavePreferences()
  end
  function l:updateCaption()
    self:setCaption(tips[preferences.TipNumber])
  end
  if (preferences.TipNumber == 0) then
    l:nextTip()
  else
    l:updateCaption()
  end

  local showtips = {}
  showtips = menu:addCheckBox(_("Show tips at startup"), 14, 256 - 75,
    function()
      preferences.ShowTips = showtips:isMarked()
      SavePreferences()
    end)
  showtips:setMarked(preferences.ShowTips)
  menu:addSmallButton(_("~!OK"), 14, 256 - 40,
    function() l:nextTip(); menu:stop() end)
  menu:addSmallButton(_("~!Previous Tip"), 14 + 106 + 11, 256 - 40,
    function() l:prevTip(); l:updateCaption() end)
  menu:addSmallButton(_("~!Next Tip"), 14 + 106 + 11 + 106 + 11, 256 - 40,
    function() l:nextTip(); l:updateCaption() end)

  menu:run(false)
end

function RunObjectivesMenu()
  local menu = BosGameMenu()
  menu:setSize(384, 256)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)

  menu:addLabel(_("Objectives"), 192, 11)

  local l = MultiLineLabel()
  l:setFont(Fonts["game"])
  l:setSize(356, 144)
  l:setLineWidth(356)
  menu:add(l, 14, 36)

  l:setCaption(current_objective)

  menu:addButton(_("~!OK"), 80, 256 - 40,
    function() menu:stop() end)

  menu:run(false)
end

function RunVictoryMenu()
  local menu = BosGameMenu()
  menu:setSize(288, 128)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)

  menu:addLabel(_("Congratulations!"), 144, 11)
  menu:addLabel(_("You are victorious!"), 144, 32)
  menu:addButton(_("~!Victory"), 32, 54,
    function() menu:stop() end)
  -- FIXME: check if log is disabled
  menu:addButton(_("Save ~!Replay"), 32, 90,
    function() RunSaveReplayMenu() end)

  menu:run(false)
end

function RunDefeatMenu()
  local menu = BosGameMenu()
  menu:setSize(288, 128)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)

  menu:addLabel(_("You have failed to"), 144, 11)
  menu:addLabel(_("achieve victory!"), 144, 32)
  menu:addButton(_("~!OK"), 32, 56,
    function() menu:stop() end)
  -- FIXME: check if log is disabled
  menu:addButton(_("Save ~!Replay"), 32, 90,
    function() RunSaveReplayMenu() end)

  menu:run(false)
end

function RunSaveReplayMenu()
  local menu = BosGameMenu()
  menu:setSize(288, 128)
  menu:setPosition((Video.Width - menu:getWidth()) / 2,
    (Video.Height - menu:getHeight()) / 2)

  menu:addLabel(_("Save Replay"), 144, 11)
  menu:addTextInputField("", 14, 40, 260)
  menu:addSmallButton(_("~!OK"), 14, 80,
    function() menu:stop() end)
  menu:addSmallButton(_("Cancel (~<Esc~>)"), 162, 80,
    function() menu:stop() end)

  menu:run(false)
end

