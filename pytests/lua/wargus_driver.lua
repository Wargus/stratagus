local ARGS = ...

Load("scripts/stratagus.lua")
SetTitleScreens({})

CustomStartup = function()
  InitGameSettings()

  if ARGS == "join-ip-menu" then
    InitNetwork1()
    local background = WarMenu(nil, nil)
    background:run(false)
    RunJoinIpMenu()
    ExitNetwork1()
    return
  end

  if ARGS == "multiplayer-help" then
    print("wargus_driver multiplayer-help loaded")
    return
  end

  if ARGS == "lifecycle-cleanup-reload-stress" then
    local map = "maps/test/(2)pytest-lifecycle-cleanup.smp"
    for i = 1, 8 do
      print("PYTEST_LIFECYCLE_RELOAD_START " .. i)
      InitGameSettings()
      GameSettings.Presets[0].Type = PlayerPerson
      GameSettings.Presets[0].Race = 0
      GameSettings.Presets[0].PlayerColor = 0
      GameSettings.Presets[1].Type = PlayerComputer
      GameSettings.Presets[1].Race = 1
      GameSettings.Presets[1].PlayerColor = 1
      GameSettings.Presets[1].AIScript = "ai-passive"
      GameSettings.Difficulty = 1
      GameSettings.GameType = SettingsGameTypeMapDefault
      GameSettings.NetGameType = 1
      Objectives = DefaultObjectives
      InitGameVariables()
      ActionVictory = OldActionVictory
      StartMap(map, true)
      print("PYTEST_LIFECYCLE_RELOAD_FINISH " .. i)
    end
    print("PYTEST_LIFECYCLE_RELOAD_DONE")
    Exit(0)
  end

  error("Unknown wargus_driver mode: " .. tostring(ARGS))
end
