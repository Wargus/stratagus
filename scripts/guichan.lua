--      (c) Copyright 2010      by Stratagus team

Load("scripts/widgets.lua")
Load("scripts/menus/options.lua")
Load("scripts/menus/credits.lua")
Load("scripts/menus/games.lua")

-- Default configurations -------
Widget:setGlobalFont(Fonts["stratagus-menu"])

-- Define the different menus ----------

local function BuildProgramStartMenu()
--  war1gus.playlist = { "music/Orc Briefing.ogg" }
--[[
  if not (IsMusicPlaying()) then
    PlayMusic("music/Orc Briefing.ogg")
  end
--]]
  local menu = StratagusMenu()
  local offx = (Video.Width - 640) / 2
  local offy = (Video.Height - 480) / 2

  local currentGame = "";
  
  if (stratagus.preferences.CurrentGame ~= nil) then
    -- Remove .lua
	currentGame = stratagus.preferences.CurrentGame
  end

  menu:addLabel("Stratagus V" .. GetStratagusVersion() .. "  " .. GetStratagusHomepage(), offx + 320, offy + 390 + 18*1)
  menu:addLabel("Copyright (c) 1998-2011 by The Stratagus Project", offx + 320, offy + 390 + 18*4)

  menu:addFullButton("Select ~!Game", "g", offx + 208, offy + 104 + 36*1, function() RunGamesMenu(); menu:stop(1) end)
  local playbutton = menu:addFullButton("~!Play " .. currentGame, "p", offx + 208, offy + 104 + 36*2, function() Load(stratagus.preferences.CurrentGame); menu:stop(0); end)

  menu:addFullButton("Preference ~!Options", "o", offx + 208, offy + 104 + 36*4, function() stratagus.RunPreferencesOptionsMenu(); menu:stop(1) end)
  menu:addFullButton("~!Sound Options", "s", offx + 208, offy + 104 + 36*5, function() RunSoundOptionsMenu(); menu:stop(1) end)
  menu:addFullButton("~!Video Options", "v", offx + 208, offy + 104 + 36*6, function() RunVideoOptionsMenu(); menu:stop(1) end)
  menu:addFullButton("S~!how Credits", "h", offx + 208, offy + 104 + 36*7, RunShowCreditsMenu)
  menu:addFullButton("E~!xit Program", "x", offx + 208, offy + 104 + 36*9, function() menu:stop() end)

  playbutton:setEnabled(stratagus.preferences.CurrentGame ~= nil)

  return menu:run()
end

function stratagus.RunProgramStartMenu()
  local continue = 1

  while continue == 1 do
    continue = BuildProgramStartMenu(menu)
  end
end

LoadGameFile = nil

stratagus.RunProgramStartMenu()
