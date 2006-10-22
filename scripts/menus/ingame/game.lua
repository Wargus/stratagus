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
--      (c) Copyright 2006 by Jimmy Salmon and Francois Beerten
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

  AddMenuHelpers(menu)

  -- FIXME: not a good solution
  -- default size is 200,24 but we want 224,28 so we override these functions
  menu.addButtonOrig = menu.addButton
  function menu:addButton(caption, hotkey, x, y, callback, size)
    return self:addButtonOrig(caption, hotkey, x, y, callback, {224, 28})
  end
  function menu:addSmallButton(caption, hotkey, x, y, callback)
    return self:addButtonOrig(caption, hotkey, x, y, callback, {100, 24})
  end

  return menu
end


function RunGameMenu(s)
  local menu = BosGameMenu()

  menu:addLabel(_("Game Menu"), 128, 11)
  local b = menu:addButton(_("Save (~<F11~>)"), "f11", 16, 40,
    function() RunSaveMenu() end)
  if (IsReplayGame() or IsNetworkGame()) then
    b:setEnabled(false)
  end
  menu:addButton(_("Options (~<F5~>)"), "f5", 16, 40 + (36 * 1),
    function() RunGameOptionsMenu() end)
  menu:addButton(_("Help (~<F1~>)"), "f1", 16, 40 + (36 * 2),
    function() RunHelpMenu() end)
  menu:addButton(_("~!Objectives"), "o", 16, 40 + (36 * 3),
    function() RunObjectivesMenu() end)
  menu:addButton(_("~!End Game"), "e", 16, 40 + (36 * 4),
    function() RunEndGameMenu() end)
  menu:addButton(_("Return to Game (~<Esc~>)"), "escape", 16, 248,
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

  menu:addSmallButton(_("Save"), 0, 16, 248,
    -- FIXME: use a confirm menu if the file exists already
    function()
      local name = t:getText()
      -- strip .gz
      if (string.find(name, ".gz$") ~= nil) then
        name = string.sub(name, 1, string.len(name) - 3)
      end
      -- append .sav
      if (string.find(name, ".sav$") == nil) then
        name = name .. ".sav"
      end
      -- replace invalid chars with underscore
      local t = {"\\", "/", ":", "*", "?", "\"", "<", ">", "|"}
      table.foreachi(t, function(k,v) name = string.gsub(name, v, "_") end)

      SaveGame(name)
      UI.StatusLine:Set("Saved game to: " .. name)
      menu:stop()
    end)

  menu:addSmallButton(_("Cancel"), 0, 16 + 12 + 106, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunGameSoundOptionsMenu()
  local menu = BosGameMenu()
 
  menu:addLabel(_("Sound Options"), 128, 11)
  AddSoundOptions(menu, 0, 0, 128 - 224/2, 280)

  menu:run(false)
end

function RunGameOptionsMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Game Options"), 128, 11)
  menu:addButton(_("Sound (~<F7~>)"), "f7", 16, 40 + (36 * 0),
    function() RunGameSoundOptionsMenu() end)
  menu:addButton(_("Game (~<F9~>)"), "f9", 16, 40 + (36 * 1),
    function() RunPreferencesMenu() end)
  menu:addButton(_("~!Diplomacy"), "d", 16, 40 + (36 * 2),
    function() RunDiplomacyMenu() end)
  menu:addButton(_("Previous (~<Esc~>)"), "escape", 128 - (224 / 2), 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunPreferencesMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Game Options"), 128, 11)

  local fog = {}
  fog = menu:addCheckBox(_("Fog of War Enabled"), 16, 36 * 1,
    function() SetFogOfWar(fog:isMarked()) end)
  fog:setMarked(GetFogOfWar())
  if (IsReplayGame() or IsNetworkGame()) then
    fog:setEnabled(false)
  end

  local ckey = {}
  ckey = menu:addCheckBox(_("Show command key"), 16, 36 * 2,
    function() UI.ButtonPanel.ShowCommandKey = ckey:isMarked() end)
  ckey:setMarked(UI.ButtonPanel.ShowCommandKey)

  local l = Label(_("Game Speed"))
  l:setFont(Fonts["game"])
  l:adjustSize()
  menu:add(l, 16, 36 * 3)

  local gamespeed = {}
  gamespeed = menu:addSlider(15, 75, 198, 18, 32, 36 * 3.5,
    function() SetGameSpeed(gamespeed:getValue()) end)
  gamespeed:setValue(GetGameSpeed())

  l = Label(_("slow"))
  l:setFont(Fonts["small"])
  l:adjustSize()
  menu:add(l, 32, (36 * 4) + 6)
  l = Label(_("fast"))
  l:setFont(Fonts["small"])
  l:adjustSize()
  menu:add(l, 230 - l:getWidth(), (36 * 4) + 6)

  menu:addSmallButton(_("~!OK"), "o", 128 - (106 / 2), 245,
    function()
      preferences.FogOfWar = GetFogOfWar()
      preferences.ShowCommandKey = UI.ButtonPanel.ShowCommandKey
      preferences.GameSpeed = GetGameSpeed()
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

      if (IsReplayGame() or ThisPlayer:IsTeamed(Players[i])) then
        alliedcb:setEnabled(false)
        enemycb:setEnabled(false)
        sharedvisioncb:setEnabled(false)
      end
    end
  end

  menu:addSmallButton(_("~!OK"), "o", 75, 384 - 40,
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
        if (sharedvision[j]:isMarked()) then
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
  menu:addSmallButton(_("~!Cancel"), "c", 195, 384 - 40, function() menu:stop() end)

  menu:run(false)
end

function RunEndGameMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("End Game"), 128, 11)
  local b = menu:addButton(_("~!Restart Game"), "r", 16, 40 + (36 * 0),
    function() RunRestartConfirmMenu() end)
  if (IsNetworkGame()) then
    b:setEnabled(false)
  end
  menu:addButton(_("~!Surrender"), "s", 16, 40 + (36 * 1),
    function() RunSurrenderConfirmMenu() end)
  menu:addButton(_("~!Quit to Menu"), "q", 16, 40 + (36 * 2),
    function() RunQuitToMenuConfirmMenu() end)
  menu:addButton(_("E~!xit Program"), "x", 16, 40 + (36 * 3),
    function() RunExitConfirmMenu() end)
  menu:addButton(_("Previous (~<Esc~>)"), "escape", 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunRestartConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to restart"), 128, 11 + (24 * 1))
  menu:addLabel(_("the game?"), 128, 11 + (24 * 2))
  menu:addButton(_("~!Restart Game"), "r", 16, 11 + (24 * 3) + 29,
    function() StopGame(GameRestart); menu:stopAll() end)
  menu:addButton(_("Cancel (~<Esc~>)"), "escape", 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunSurrenderConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to surrender"), 128, 11 + (24 * 1))
  menu:addLabel(_("to your enemies?"), 128, 11 + (24 * 2))
  menu:addButton(_("~!Surrender"), "s", 16, 11 + (24 * 3) + 29,
    function() StopGame(GameDefeat); menu:stopAll() end)
  menu:addButton(_("Cancel (~<Esc~>)"), "escape", 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunQuitToMenuConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to quit to"), 128, 11 + (24 * 1))
  menu:addLabel(_("the main menu?"), 128, 11 + (24 * 2))
  menu:addButton(_("~!Quit to Menu"), "q", 16, 11 + (24 * 3) + 29,
    function() StopGame(GameQuitToMenu); menu:stopAll() end)
  menu:addButton(_("Cancel (~<Esc~>)"), "escape", 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunExitConfirmMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Are you sure you"), 128, 11)
  menu:addLabel(_("want to exit"), 128, 11 + (24 * 1))
  menu:addLabel(_("Stratagus?"), 128, 11 + (24 * 2))
  menu:addButton(_("E~!xit Program"), "x", 16, 11 + (24 * 3) + 29,
    function() Exit(0) end)
  menu:addButton(_("Cancel (~<Esc~>)"), "escape", 16, 248,
    function() menu:stop() end)

  menu:run(false)
end

function RunHelpMenu()
  local menu = BosGameMenu()

  menu:addLabel(_("Help Menu"), 128, 11)
  menu:addButton(_("Keystroke ~!Help"), "h", 16, 40 + (36 * 0),
    function() RunKeystrokeHelpMenu() end)
  menu:addButton(_("Stratagus ~!Tips"), "t", 16, 40 + (36 * 1),
    function() RunTipsMenu() end)
  menu:addButton(_("Previous (~<Esc~>)"), "escape", 128 - (224 / 2), 248,
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
  {"Alt-R", "- restart game"},
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
  menu:addButton(_("Previous (~<Esc~>)"), "escape", (352 / 2) - (224 / 2), 352 - 40,
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
  end

  l:updateCaption()

  local showtips = {}
  showtips = menu:addCheckBox(_("Show tips at startup"), 14, 256 - 75,
    function()
      preferences.ShowTips = showtips:isMarked()
      SavePreferences()
    end)
  showtips:setMarked(preferences.ShowTips)
  menu:addSmallButton(_("~!OK"), "o", 14, 256 - 40,
    function() l:nextTip(); menu:stop() end)
  menu:addSmallButton(_("~!Previous Tip"), "p", 14 + 106 + 11, 256 - 40,
    function() l:prevTip(); l:updateCaption() end)
  menu:addSmallButton(_("~!Next Tip"), "n", 14 + 106 + 11 + 106 + 11, 256 - 40,
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

  menu:addButton(_("~!OK"), "o", 80, 256 - 40,
    function() menu:stop() end)

  menu:run(false)
end


