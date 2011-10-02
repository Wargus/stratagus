--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      network.lua - The multiplayer UI.
--
--      (c) Copyright 2005-2010 by François Beerten
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
--      $Id: guichan.lua 305 2005-12-18 13:36:42Z feb $

-- TODO: 
--  * lua cleanup
--  * abort (exit)
--  * network errors
--  * load and store LocalPlayerName from/in preferences2.lua

function bool2int(boolvalue)
  if boolvalue == true then
    return 1
  else
    return 0
  end
end

function int2bool(int)
  if int == 0 then
    return false
  else
    return true
  end
end

function ErrorMenu(errmsg)
  local menu

  menu = BosMenu(_("Error"))

  local l = MultiLineLabel(errmsg)
  l:setFont(Fonts["large"])
  l:setAlignment(MultiLineLabel.CENTER)
  l:setVerticalAlignment(MultiLineLabel.CENTER)
  l:setLineWidth(340)
  l:setWidth(340)
  l:setHeight(200)
  l:setBackgroundColor(dark)
  menu:add(l, Video.Width / 2 - 170, Video.Height / 2 - 100)

  menu:addButton(_("~!OK"), Video.Width / 2 - 100, Video.Height - 100,
    function() menu:stop() end)

  menu:run()
end

function addPlayersList(menu, numplayers)
  local i
  local players_name = {}
  local players_state = {}
  local sx = Video.Width / 20
  local sy = Video.Height / 20
  local numplayers_text

  menu:writeLargeText(_("Players"), sx * 11, sy*3)
  for i=1,8 do
    players_name[i] = menu:writeText(_("Player")..i, sx * 11, sy*4 + i*18)
    players_name[i]:setWidth(80)
    players_state[i] = menu:writeText(_("Preparing"), sx * 11 + 85, sy*4 + i*18)
  end
  numplayers_text = menu:writeText(_("Open slots: ") .. numplayers - 1, sx *11, sy*4 + 162)

  local function updatePlayers()
    local connected_players = 0
    local ready_players = 0
    players_state[1]:setCaption(_("Creator"))
    players_name[1]:setCaption(Hosts[0].PlyName)
    for i=2,8 do
      if Hosts[i-1].PlyName == "" then
        players_name[i]:setCaption("")
        players_state[i]:setCaption("")
      else
        connected_players = connected_players + 1
        if ServerSetupState.Ready[i-1] == 1 then
          ready_players = ready_players + 1
          players_state[i]:setCaption(_("Ready"))    
        else
          players_state[i]:setCaption(_("Preparing"))
        end
        players_name[i]:setCaption(Hosts[i-1].PlyName)
     end
    end
    numplayers_text:setCaption(_("Open slots : ") .. numplayers - 1 - connected_players)
    numplayers_text:adjustSize()
    return (connected_players > 0 and ready_players == connected_players)
  end

  return updatePlayers
end


local joincounter = 0

function RunJoiningMapMenu(s)
  local menu
  local listener  
  local sx = Video.Width / 20
  local sy = Video.Height / 20
  local numplayers = 3
  local state

  menu = BosMenu(_("Joining game: Map"))

  menu:writeLargeText(_("Map"), sx, sy*3)
  menu:writeText(_("File:"), sx, sy*3+30)
  maptext = menu:writeText(NetworkMapName, sx+50, sy*3+30)
  maptext:setWidth(sx * 9 - 50 - 20)
  menu:writeText(_("Players:"), sx, sy*3+50)
  players = menu:writeText(numplayers, sx+70, sy*3+50)
  menu:writeText(_("Description:"), sx, sy*3+70)
  descr = menu:writeText(_("Unknown map"), sx+20, sy*3+90)
  descr:setWidth(sx * 9 - 20 - 20)

  local fow = menu:addCheckBox(_("Fog of war"), sx, sy*3+120, function() end)
  fow:setMarked(true)
  ServerSetupState.FogOfWar = 1
  fow:setEnabled(false)
  local revealmap = menu:addCheckBox(_("Reveal map"), sx, sy*3+150, function() end)
  revealmap:setEnabled(false)
  
  menu:writeText(_("Difficulty:"), sx, sy*11)
  local difficulty = menu:addDropDown({_("easy"), _("normal"), _("hard")}, sx + 150, sy*11,
    function(dd) end)
  difficulty:setEnabled(false)
  menu:writeText(_("Starting resources:"), sx, sy*11+25)
  local resources = menu:addDropDown({_("high"), _("normal"), _("low")}, sx + 150, sy*11+25,
    function(dd) end)
  resources:setEnabled(false)
  menu:writeText(_("Game type:"), sx, sy*11+50)
  local gametype = menu:addDropDown({_("Map Default"), _("Melee"), _("Free For All"), _("Top vs Bottom"), _("Left vs Right"), _("Man vs Machine")}, sx + 150, sy*11+50,
    function(dd) end)
  gametype:setEnabled(false)
  gametype:setSize(140, gametype:getHeight())

  local OldPresentMap = PresentMap
  PresentMap = function(description, nplayers, w, h, id)
    players:setCaption(""..nplayers)
    descr:setCaption(description)
    numplayers = nplayers
    OldPresentMap(description, nplayers, w, h, id)
  end

  -- Security: The map name is checked by the stratagus engine.
  Load(NetworkMapName)
  local function readycb(dd)
     LocalSetupState.Ready[NetLocalHostsSlot] = bool2int(dd:isMarked())
  end
  menu:addCheckBox(_("~!Ready"), sx*11,  sy*14, readycb)

  menu:addButton(_("Cancel (~<Esc~>)"), sx * 10 - 100, Video.Height - 100,
                 function() 
                 NetworkDetachFromServer() menu:stop() end
  )

  local updatePlayersList = addPlayersList(menu, numplayers)

  joincounter = 0
  local function listen()
    NetworkProcessClientRequest()
    fow:setMarked(int2bool(ServerSetupState.FogOfWar))
    GameSettings.NoFogOfWar = not int2bool(ServerSetupState.FogOfWar)
    revealmap:setMarked(int2bool(ServerSetupState.RevealMap))
    GameSettings.RevealMap = ServerSetupState.RevealMap
    difficulty:setSelected((5 - ServerSetupState.Difficulty) / 2)
    GameSettings.Difficulty = ServerSetupState.Difficulty
    resources:setSelected((5 - ServerSetupState.ResourcesOption) / 2)
    GameSettings.Resources = ServerSetupState.ResourcesOption
    if (ServerSetupState.GameTypeOption == 255) then
      gametype:setSelected(0)
    else
      gametype:setSelected(ServerSetupState.GameTypeOption + 1)
    end
    GameSettings.GameType = ServerSetupState.GameTypeOption
    updatePlayersList()
    state = GetNetworkState()
    -- FIXME: don't use numbers
    if (state == 15) then -- ccs_started, server started the game
      ThisPlayer = Players[1]
      joincounter = joincounter + 1
      if (joincounter == 30) then
        SetFogOfWar(fow:isMarked())
        if revealmap:isMarked() == true then
          RevealMap()
        end
        NetworkGamePrepareGameSettings()
        AllowAllUnits()
        RunMap(NetworkMapName)
        PresentMap = OldPresentMap
        menu:stop()
      end
    elseif (state == 10) then -- ccs_unreachable
      ErrorMenu(_("Cannot reach server"))
      menu:stop(1)
    end
  end
  listener = LuaActionListener(listen)
  menu:addLogicCallback(listener)
  menu:run()
end

function RunJoiningGameMenu(s)
  local menu
  local x = Video.Width/2 - 100
  local listener
  local state
  local percent = 0

  menu = BosMenu(_("Joining game"))

  local sb = StatBoxWidget(300, 30)
  sb:setCaption(_("Connecting ..."))
  sb:setPercent(0)
  menu:add(sb, x-50, Video.Height/2)
  sb:setBackgroundColor(dark)

  menu:addButton(_("Cancel (~<Esc~>)"), x, Video.Height - 100,
                 function() menu:stop() end)

  local function checkconnection() 
    NetworkProcessClientRequest()
    percent = percent + 100 / (24 * GetGameSpeed()) -- 24 seconds * fps
    sb:setPercent(percent)
    state = GetNetworkState()
    -- FIXME: do not use numbers
    if (state == 3) then -- ccs_mapinfo
      -- got ICMMap => load map
      RunJoiningMapMenu()
      menu:stop()
    elseif (state == 4) then -- ccs_badmap
      ErrorMenu(_("Map not available"))
      menu:stop(1)
    elseif (state == 10) then -- ccs_unreachable
      ErrorMenu(_("Cannot reach server"))
      menu:stop(1)
    elseif (state == 12) then -- ccs_nofreeslots
      ErrorMenu(_("Server is full"))
      menu:stop(1)
    elseif (state == 13) then -- ccs_serverquits
      ErrorMenu(_("Server gone"))
      menu:stop(1)
    elseif (state == 16) then -- ccs_incompatibleengine
      ErrorMenu(_("Incompatible engine version"))
      menu:stop(1)
    elseif (state == 17) then -- ccs_incompatiblenetwork
      ErrorMenu(_("Incompatible network version"))
      menu:stop(1)
    end
  end
  listener = LuaActionListener(checkconnection)
  menu:addLogicCallback(listener)
  menu:run()
end

function RunJoinIpMenu()
  local menu
  local server
  local x = Video.Width/2 - 100

  menu = BosMenu(_("Enter Server address"))
  menu:writeText(_("IP or server name :"), x, Video.Height*8/20)
  server = menu:addTextInputField("localhost", x + 60, Video.Height*9/20 + 4, 130)
  menu:addButton(_("~!Join Game"), x,  Video.Height*10/20, 
    function(s) 
      -- FIXME: allow port ("localhost:1234")
      if (NetworkSetupServerAddress(server:getText()) ~= 0) then
        ErrorMenu(_("Invalid server name"))
        return
      end
      NetworkInitClientConnect() 
      if (RunJoiningGameMenu() ~= 0) then
        -- connect failed, don't leave this menu
        return
      end
      menu:stop() 
    end
  )
  menu:addButton(_("Cancel (~<Esc~>)"), x, Video.Height - 100,
                 function() menu:stop() end)
  menu:run()
end

function RunServerMultiGameMenu(map, description, numplayers)
  local menu
  local sx = Video.Width / 20
  local sy = Video.Height / 20
  local startgame
  local d

  menu = BosMenu(_("Create MultiPlayer game"))

  menu:writeLargeText(_("Map"), sx, sy*3)
  menu:writeText(_("File:"), sx, sy*3+30)
  maptext = menu:writeText(map, sx+50, sy*3+30)
  maptext:setWidth(sx * 9 - 50 - 20)
  menu:writeText(_("Players:"), sx, sy*3+50)
  players = menu:writeText(numplayers, sx+70, sy*3+50)
  menu:writeText(_("Description:"), sx, sy*3+70)
  descr = menu:writeText(description, sx+20, sy*3+90)
  descr:setWidth(sx * 9 - 20 - 20)

  local function fowCb(dd)
    ServerSetupState.FogOfWar = bool2int(dd:isMarked()) 
    NetworkServerResyncClients()
    GameSettings.NoFogOfWar = not dd:isMarked()
  end
  local fow = menu:addCheckBox(_("Fog of war"), sx, sy*3+120, fowCb)
  fow:setMarked(true)
  local function revealMapCb(dd)
    ServerSetupState.RevealMap = bool2int(dd:isMarked()) 
    NetworkServerResyncClients()
    GameSettings.RevealMap = bool2int(dd:isMarked())
  end
  local revealmap = menu:addCheckBox(_("Reveal map"), sx, sy*3+150, revealMapCb)
  
  menu:writeText(_("Difficulty:"), sx, sy*11)
  d = menu:addDropDown({_("easy"), _("normal"), _("hard")}, sx + 150, sy*11,
    function(dd)
      GameSettings.Difficulty = 5 - dd:getSelected()*2
      ServerSetupState.Difficulty = GameSettings.Difficulty
      NetworkServerResyncClients()
    end)
  d:setSelected(1)
  menu:writeText(_("Starting resources:"), sx, sy*11+25)
  d = menu:addDropDown({_("high"), _("normal"), _("low")}, sx + 150, sy*11+25,
    function(dd)
      GameSettings.Resources = 5 - dd:getSelected()*2
      ServerSetupState.ResourcesOption = GameSettings.Resources
      NetworkServerResyncClients()
    end)
  d:setSelected(1)
  menu:writeText(_("Game type:"), sx, sy*11+50)
  d = menu:addDropDown({_("Map Default"), _("Melee"), _("Free For All"), _("Top vs Bottom"), _("Left vs Right"), _("Man vs Machine")}, sx + 150, sy*11+50,
    function(dd)
      GameSettings.GameType = dd:getSelected() - 1
      ServerSetupState.GameTypeOption = GameSettings.GameType
      NetworkServerResyncClients()
    end)
  d:setSelected(0)
  d:setSize(140, d:getHeight())


  local updatePlayers = addPlayersList(menu, numplayers)

  NetworkMapName = map
  NetworkInitServerConnect(numplayers)
  ServerSetupState.FogOfWar = 1
  ServerSetupState.Difficulty = 3
  ServerSetupState.ResourcesOption = 3
  ServerSetupState.GameTypeOption = SettingsGameTypeMapDefault

  menu:addButton(_("Cancel (~<Esc~>)"), Video.Width / 2 - 250, Video.Height - 100,
                 function() menu:stop() end)

  startgame = menu:addButton(_("~!Start Game"), 
    Video.Width / 2 + 50, 
    Video.Height - 100,
    function(s)    
      SetFogOfWar(fow:isMarked())
      if revealmap:isMarked() == true then
        RevealMap()
      end
      NetworkServerStartGame() 
      NetworkGamePrepareGameSettings()
      AllowAllUnits()
      RunMap(map)
      menu:stop()
    end
  )
  startgame:setVisible(false)
  local waitingtext = menu:writeText(_("Waiting for players"), sx*11, sy*14)
  local function updateStartButton(ready) 
    startgame:setVisible(ready)
    waitingtext:setVisible(not ready)
  end

  local listener = LuaActionListener(function(s) updateStartButton(updatePlayers()) end)
  menu:addLogicCallback(listener)
  menu:run()
end

function RunCreateMultiGameMenu(s)
  local menu
  local map = _("No Map")
  local description = _("No map")
  local mapfile = "maps/islandwar.map"
  local numplayers = 3
  local sx = Video.Width / 20
  local sy = Video.Height / 20

  menu = BosMenu(_("Create MultiPlayer game"))

  menu:writeText(_("File:"), sx, sy*3+30)
  maptext = menu:writeText(mapfile, sx+50, sy*3+30)
  maptext:setWidth(sx * 9 - 50 - 20)
  menu:writeText(_("Players:"), sx, sy*3+50)
  players = menu:writeText(numplayers, sx+70, sy*3+50)
  menu:writeText(_("Size:"), sx, sy*3+70)
  mapsize = menu:writeText("       ", sx+70, sy*3+70)
  menu:writeText(_("Description:"), sx, sy*3+90)
  descr = menu:writeText(description, sx+20, sy*3+110)
  descr:setWidth(sx * 9 - 20 - 20)

  local OldPresentMap = PresentMap
  PresentMap = function(desc, nplayers, w, h, id)
    numplayers = nplayers
    players:setCaption(""..numplayers)
    players:adjustSize()
	mapsize:setCaption(""..h.."x"..w)
    description = desc
    descr:setCaption(description)
    OldPresentMap(description, nplayers, w, h, id)
  end

  Load(mapfile .. '/presentation.smp')
  local browser = menu:addMapBrowser("maps/", sx*10, sy*2+20, sx*8, sy*11, mapfile)
  local function cb(s)
    mapfile = browser:getSelectedMap()
    Load(mapfile)
    maptext:setCaption(browser:getSelectedItem())
  end
  browser:setActionCallback(cb)
  
  menu:addButton(_("Cancel (~<Esc~>)"), Video.Width / 2 - 250, Video.Height - 100,
                 function() menu:stop(1) end)
  menu:addButton(_("Create ~!Game"), Video.Width / 2 + 50, Video.Height - 100,
    function(s)    
      RunServerMultiGameMenu(browser:getSelectedMap(), description, numplayers)
      menu:stop()
    end
  )
  menu:run()
  PresentMap = OldPresentMap
end

function RunMultiPlayerMenu(s)
  local menu
  local b
  local x = Video.Width/2 - 100
  local nick

  menu = BosMenu(_("MultiPlayer"))

  menu:writeText(_("Nickname :"), x, Video.Height*8/20)
  nick = menu:addTextInputField(GetLocalPlayerName(), x + 90, Video.Height*8/20 + 4)
  nick:setMaxLengthBytes(NetPlayerNameSize - 1)

  ResetMapOptions()
  InitNetwork1()
  menu:addButton(_("~!Join Game"), x, Video.Height*11/20, 
    function(s)
      if nick:getText() ~= GetLocalPlayerName() then
        SetLocalPlayerName(nick:getText())
        preferences.PlayerName = nick:getText()
        SavePreferences()
      end
      RunJoinIpMenu()
      menu:stop(1)
    end)
  menu:addButton(_("Create ~!Game"), x, Video.Height*12/20, 
    function(s)
      if nick:getText() ~= GetLocalPlayerName() then
        SetLocalPlayerName(nick:getText())
        preferences.PlayerName = nick:getText()
        SavePreferences()
      end
      RunCreateMultiGameMenu()
      menu:stop(1)
    end)

  menu:addButton(_("Cancel (~<Esc~>)"), Video.Width / 2 - 100, Video.Height - 100,
                 function() menu:stop() end)
  menu:run()
  ExitNetwork1()
end

