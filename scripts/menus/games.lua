

local function CreateGameMenu()
  local menu = StratagusMenu()
  local offx = (Video.Width - 640) / 2
  local offy = 0

  local gameBrowser = menu:addBrowser("games", ".lua$", offx + 320, offy + 11, 200, 200)
  gameBrowser:setActionCallback(function(browser) stratagus.preferences.CurrentGame = browser.path .. browser:getSelectedItem()  end)

  menu:addFullButton("~!OK", "o", offx + 320 - (224 / 2), offy + 288 - 40, function() SavePreferences(); menu:stop() end)
  return menu:run()
end


local function GamesMenu()
  local menu = StratagusMenu()
  local offx = (Video.Width - 640) / 2
  local offy = 0
  
  menu:addLabel("Games", offx + 320, offy + 11)

  
  
  menu:addFullButton("~!New", "n", offx + 320 - (224 / 2), offy + 104 + 36*4, CreateGameMenu)
  
  menu:addFullButton("~!OK", "o", offx + 320 - (224 / 2), offy + 288 - 40, function() menu:stop() end)
  return menu:run()
end

function RunGamesMenu()
  local continue = 1
  while (continue == 1) do
--    continue = GamesMenu()
continue = CreateGameMenu()
  end
end



