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

  if ARGS == "server-lobby-callbacks" then
    InitNetwork1()
    local map = "maps/RushHour(4).smp"
    Load(map)
    NetworkMapName = map
    NetworkInitServerConnect(4)
    local menu = CreateOnlineLobby(map, 4, true)

    menu.option_dedicated_ai_server:setMarked(true)
    menu.option_dedicated_ai_server.callback(menu.option_dedicated_ai_server)
    expect(ServerSetupState.CompOpt[0] == 2, "dedicated callback did not close host slot")

    menu.option_race:setSelected(2)
    menu.option_race.callback(menu.option_race)
    expect(ServerSetupState.Race[0] == 1, "server race callback did not update host player slot")

    menu:updateOptions()
    print("PYTEST_WAR1GUS_SERVER_LOBBY_CALLBACKS_OK")
    ExitNetwork1()
    Exit(0)
    return
  end

  if ARGS == "client-lobby-race-slot" then
    InitNetwork1()
    local map = "maps/RushHour(4).smp"
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

    print("PYTEST_WAR1GUS_CLIENT_LOBBY_RACE_SLOT_OK")
    ExitNetwork1()
    Exit(0)
    return
  end

  error("Unknown war1gus_driver mode: " .. tostring(ARGS))
end
