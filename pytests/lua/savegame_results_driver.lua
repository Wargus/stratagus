local ARGS = ...

Load("scripts/stratagus.lua")
SetTitleScreens({})

if LoadBuffer("scripts/lists/maps/For the Motherland") ~= nil then
  local previous_run_single_player_game_menu = RunSinglePlayerGameMenu
  local previous_war_menu = WarMenu
  local previous_clean_custom_game_lua = CleanCustomGame_Lua
  local dummy_menu = {}
  dummy_menu.__index = function()
    return function()
      return setmetatable({}, dummy_menu)
    end
  end
  RunSinglePlayerGameMenu = function()
    print("PYTEST_SAVEGAME_MOTHERLAND_MENU_SUPPRESSED")
    io.stdout:flush()
  end
  local previous_get_map_info = GetMapInfo
  GetMapInfo = function()
    mapinfo = mapinfo or {description = "", w = 0, h = 0}
  end
  WarMenu = function()
    return setmetatable({}, dummy_menu)
  end
  Load("scripts/lists/maps/For the Motherland")
  RunSinglePlayerGameMenu = previous_run_single_player_game_menu
  CleanCustomGame_Lua = previous_clean_custom_game_lua
  GetMapInfo = previous_get_map_info
  WarMenu = previous_war_menu
end

local PytestLoad = Load
Load = function(path, ...)
  if string.find(path, "_c%.sms$") then
    local script = LoadBuffer(path)
    if script ~= nil then
      script = string.gsub(script, "Briefing%(%s*title,%s*objectives,%s*.-%)%s*", "")
    end
    if script ~= nil then
      print("PYTEST_SAVEGAME_BRIEFING_STRIPPED " .. tostring(path))
      io.stdout:flush()
      assert(loadstring(script, path))()
      return
    end
    print("PYTEST_SAVEGAME_BRIEFING_SUPPRESSING " .. tostring(path))
    io.stdout:flush()
  end
  return PytestLoad(path, ...)
end

Briefing = function()
  print("PYTEST_SAVEGAME_BRIEFING_SUPPRESSED")
  io.stdout:flush()
end

CustomStartup = function()
  print("PYTEST_SAVEGAME_START " .. tostring(ARGS))
  io.stdout:flush()
  StartSavedGame(ARGS)
  print("PYTEST_SAVEGAME_RESULTS_START " .. tostring(GameResult))
  io.stdout:flush()
  RunResultsMenu()
  print("PYTEST_SAVEGAME_RESULTS_DONE " .. tostring(GameResult))
  io.stdout:flush()
  Exit(0)
end
