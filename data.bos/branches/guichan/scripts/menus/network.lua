--            ____
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  )
--        /_____/\____/____/
--
--      Invasion - Battle of Survival
--       A GPL'd futuristic RTS game
--
--      network.lua - The multiplayer UI.
--
--      (c) Copyright 2005-2006 by François Beerten
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


function addPlayersList(menu)
  local i
  local players_name = {}
  local players_state = {}
  for i=1,8 do
     players_name[i] = menu:writeText("Player"..i, 300, 280 + i*18)
     players_state[i] = menu:writeText("Preparing", 380, 280 + i*18)
  end

  local function updatePlayers()
    players_state[1]:setCaption("Creator")
    players_name[1]:setCaption(Hosts[0].PlyName)
    for i=2,8 do
      if Hosts[i-1].PlyName == "" then
         players_name[i]:setCaption("")
         players_state[i]:setCaption("")
      else
         if ServerSetupState.Ready[i-1] == 1 then
           players_state[i]:setCaption("Ready")    
         else
           players_state[i]:setCaption("Preparing")
         end
         players_name[i]:setCaption(Hosts[i-1].PlyName)
     end
    end
  end

  return updatePlayers
end


joincounter = 0

function RunJoiningMapMenu(s)
  local menu
  local server
  local x = Video.Width/2 - 100
  local listener

  menu = BosMenu(_("Joining game: Map"))

  menu:writeText(_("Players:"), 20, 80)
  players = menu:writeText(_("No map"), 80, 80)
  menu:writeText(_("Description:"), 20, 120)
  descr = menu:writeText(_("No map"),40, 160)

  local fow = menu:addCheckBox(_("Fog of war"), 25, 200, function() end)
  fow:setMarked(true)
  ServerSetupState.FogOfWar = 1
  local revealmap = menu:addCheckBox(_("Reveal map"), 25, 230, function() end)
  
  menu:writeText(_("Difficulty:"), 20, Video.Height*11/20)
  menu:addDropDown({_("easy"), _("normal"), _("hard")}, 120, Video.Height*11/20 + 7,
      function(dd) difficulty = (5 - dd:getSelected()*2) end)
  menu:writeText(_("Map richness:"), 20, Video.Height*12/20)
  menu:addDropDown({_("high"), _("normal"), _("low")}, 140, Video.Height*12/20 + 7,
      function(dd) mapresources = (5 - dd:getSelected()*2) end)
  menu:writeText(_("Starting resources:"), 20, Video.Height*13/20)
  menu:addDropDown({_("high"), _("normal"), _("low")}, 170, Video.Height*13/20 + 7,
      function(dd) startingresources = (5 - dd:getSelected()*2) end)

  local OldPresentMap = PresentMap
  PresentMap = function(description, nplayers, w, h, id)
      print(description)
      players:setCaption(""..nplayers)
      descr:setCaption(description)
      OldPresentMap(description, nplayers, w, h, id)
  end

  menu:writeText("Map: " .. NetworkMapName, 40, 40)
  -- Security: The map name is checked by the stratagus engine.
  Load(NetworkMapName)
  local function readycb(dd)
     if dd:isMarked() == true then 
        LocalSetupState.Ready[NetLocalHostsSlot] = 1 
     else 
        LocalSetupState.Ready[NetLocalHostsSlot] = 0 
     end 
  end
  menu:addCheckBox(_("~!Ready"), x,  Video.Height*15/20, readycb)

  local updatePlayersList = addPlayersList(menu)

  joincounter = 0
  local function listen()
     NetworkProcessClientRequest()
     if ServerSetupState.FogOfWar == 1 then
        fow:setMarked(true)
     else
        fow:setMarked(false)
     end
     if ServerSetupState.RevealMap == 1 then
        revealmap:setMarked(true)
     else
        revealmap:setMarked(false)
     end
     updatePlayersList()
     if GetNetworkState() == 15  then
        SetThisPlayer(1)
        joincounter = joincounter + 1
        if joincounter == 30 then
          SetFogOfWar(fow:isMarked())
          if revealmap:isMarked() == true then
             RevealMap()
          end
          NetworkGamePrepareGameSettings()
          StartMap(NetworkMapName)
          PresentMap = OldPresentMap
          menu:stop()
        end
     end
  end
  listener = LuaActionListener(listen)
  menu:addLogicCallback(listener)
  menu:run()
end

function RunJoiningGameMenu(s)
  local menu
  local server
  local x = Video.Width/2 - 100
  local listener

  menu = BosMenu(_("Joining game"))

  local sb = StatBoxWidget(300, 30)
  sb.caption = "Connecting ..."
  sb.percent = 0
  menu:add(sb, x-50, Video.Height/2)
  sb:setBackgroundColor(dark)

  local function checkconnection() 
      NetworkProcessClientRequest()
      sb.percent = sb.percent + 1 
      if GetNetworkState() == 3 then
           -- got ICMMap => load map
           RunJoiningMapMenu()
           menu:stop()
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
  server = menu:addTextInputField("localhost", x + 90, Video.Height*9/20 + 4)
  menu:addButton(_("~!Join Game"), x,  Video.Height*10/20, 
     function(s) 
       NetworkSetupServerAddress(server:getText()) 
       NetworkInitClientConnect() 
       RunJoiningGameMenu()
       menu:stop() 
     end
  )
  menu:run()
end

function RunServerMultiGameMenu(map, description)
  local menu
  local x = Video.Width/2 - 100

  menu = BosMenu(_("Create MultiPlayer game"))

  menu:writeText(_("Players:"), 20, 80)
  players = menu:writeText(map, 80, 80)
  menu:writeText(_("Description:"), 20, 120)
  descr = menu:writeText(description,40, 160)

  local function fowCb(dd)
      if dd:isMarked() == true then 
         ServerSetupState.FogOfWar = 1 
      else 
         ServerSetupState.FogOfWar = 0 
      end
      NetworkServerResyncClients()
  end
  local fow = menu:addCheckBox(_("Fog of war"), 25, 200, fowCb)
  fow:setMarked(true)
  local function revealMapCb(dd)
      if dd:isMarked() then 
         ServerSetupState.RevealMap = 1 
      else 
         ServerSetupState.RevealMap = 0 
      end
      NetworkServerResyncClients()
  end
  local revealmap = menu:addCheckBox(_("Reveal map"), 25, 230, revealMapCb)
  
  menu:writeText(_("Difficulty:"), 20, Video.Height*11/20)
  menu:addDropDown({_("easy"), _("normal"), _("hard")}, 120, Video.Height*11/20 + 7,
      function(dd) difficulty = (5 - dd:getSelected()*2) end)
  menu:writeText(_("Map richness:"), 20, Video.Height*12/20)
  menu:addDropDown({_("high"), _("normal"), _("low")}, 140, Video.Height*12/20 + 7,
      function(dd) mapresources = (5 - dd:getSelected()*2) end)
  menu:writeText(_("Starting resources:"), 20, Video.Height*13/20)
  menu:addDropDown({_("high"), _("normal"), _("low")}, 170, Video.Height*13/20 + 7,
      function(dd) startingresources = (5 - dd:getSelected()*2) end)

  local updatePlayers = addPlayersList(menu)

  NetworkMapName = map
  NetworkInitServerConnect();
  ServerSetupState.FogOfWar = 1
  menu:addButton(_("~!StartGameWhenReady"), x,  Video.Height*16/20, 
     function(s)    
        SetFogOfWar(fow:isMarked())
        if revealmap:isMarked() == true then
          RevealMap()
        end
        NetworkServerStartGame() 
        NetworkGamePrepareGameSettings()
	StartMap(map)
        menu:stop()
     end
  )
  
  local listener = LuaActionListener(updatePlayers)
  menu:addLogicCallback(listener)
  menu:run()
end

function RunCreateMultiGameMenu(s)
  local menu
  local x = Video.Width/2 - 100
  local map = _("No Map")
  local description = _("No map")
  local mapfile = "maps/default.smp"

  menu = BosMenu(_("Create MultiPlayer game"))

  menu:writeText(_("Players:"), 20, 80)
  players = menu:writeText(map, 80, 80)
  menu:writeText(_("Description:"), 20, 120)
  descr = menu:writeText(description, 40, 160)

  local OldPresentMap = PresentMap
  PresentMap = function(description, nplayers, w, h, id)
      print(description)
      players:setCaption(""..nplayers)
      descr:setCaption(description)
      OldPresentMap(description, nplayers, w, h, id)
  end

  Load(mapfile)
  maptext = menu:writeText(NetworkMapName, 20, 20)

  local browser = menu:addBrowser("maps/", "^.*%.smp$", 300, 100, 300, 200)
  local function cb(s)
    mapfile = "maps/" .. browser:getSelectedItem()
    print(browser:getSelectedItem())
    Load(mapfile)
    maptext:setCaption(mapfile)
  end
  browser:setActionCallback(cb)
  
  menu:addButton(_("~!Create Game"), x,  Video.Height*16/20, 
     function(s)    
	RunServerMultiGameMenu(mapfile, description)
        menu:stop()
     end
  )
  menu:run()
end

function RunMultiPlayerMenu(s)
  local menu
  local b
  local x = Video.Width/2 - 100
  local nick

  menu = BosMenu(_("MultiPlayer"))

  menu:writeText(_("Nickname :"), x, Video.Height*8/20)
  nick = menu:addTextInputField("Unknown", x + 90, Video.Height*8/20 + 4)

  InitNetwork1()
  menu:addButton(_("~!Join Game"), x,  Video.Height*11/20, 
      function(s) SetLocalPlayerName(nick:getText()) RunJoinIpMenu() menu:stop(1) end)
  menu:addButton(_("~!Create Game"), x,  Video.Height*12/20, 
      function(s) SetLocalPlayerName(nick:getText()) RunCreateMultiGameMenu() menu:stop(1) end)

  menu:run()
  ExitNetwork1();
end

