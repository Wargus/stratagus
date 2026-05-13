local ARGS = ...

Load("scripts/stratagus.lua")
SetTitleScreens({})

CustomStartup = function()
  print("PYTEST_SAVEGAME_START " .. tostring(ARGS))
  StartSavedGame(ARGS)
  print("PYTEST_SAVEGAME_DONE " .. tostring(GameResult))
  Exit(0)
end
