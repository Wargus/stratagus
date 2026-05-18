local ARGS = ...

Load("scripts/stratagus.lua")
SetTitleScreens({})

local function expect(condition, message)
  if not condition then
    error(message)
  end
end

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

  if ARGS == "server-lobby-callbacks" then
    InitNetwork1()
    local map = "maps/skirmish/multiplayer/(4)central-park.smp.gz"
    Load(map)
    NetworkMapName = map
    NetworkInitServerConnect(4)
    local menu = CreateOnlineLobby(map, 4, true)
    Hosts[0].PlyNr = 2

    menu.option_dedicated_ai_server:setMarked(true)
    menu.option_dedicated_ai_server.callback(menu.option_dedicated_ai_server)
    expect(ServerSetupState.CompOpt[2] == 2, "dedicated callback did not close host player slot")
    expect(ServerSetupState.CompOpt[0] ~= 2, "dedicated callback closed host index instead of player slot")

    menu.option_race:setSelected(2)
    menu.option_race.callback(menu.option_race)
    expect(ServerSetupState.Race[2] == 1, "server race callback did not update host player slot")
    expect(ServerSetupState.Race[0] ~= 1, "server race callback updated host index instead of player slot")

    menu:updateOptions()
    print("PYTEST_WARGUS_SERVER_LOBBY_CALLBACKS_OK")
    ExitNetwork1()
    Exit(0)
    return
  end

  if ARGS == "client-lobby-race-slot" then
    InitNetwork1()
    local map = "maps/skirmish/multiplayer/(4)central-park.smp.gz"
    Load(map)
    NetworkInitClientConnect()
    NetLocalHostsSlot = 3
    Hosts[NetLocalHostsSlot].PlyNr = 2
    LocalSetupState.Race[2] = -1
    LocalSetupState.Race[3] = -1
    local menu = CreateOnlineLobby(map, 4, false)

    menu.option_race:setSelected(2)
    menu.option_race.callback(menu.option_race)
    expect(LocalSetupState.Race[2] == 1, "client race callback did not update assigned player slot")
    expect(LocalSetupState.Race[3] == -1, "client race callback updated host index instead of player slot")

    print("PYTEST_WARGUS_CLIENT_LOBBY_RACE_SLOT_OK")
    ExitNetwork1()
    Exit(0)
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
