--            ____
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  )
--        /_____/\____/____/
--
--      Invasion - Battle of Survival
--       A GPL'd futuristic RTS game
--
--      campaigns.lua - The main UI lua script.
--
--      (c) Copyright 2006 by François Beerten
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


function RunBriefingMenu(objectivestext, briefingtext)
  local menu
  local b

  if objectivestext == nil then
    current_objective = default_objective
    return
  end

  current_objective = objectivestext
  objectivestext = {_("Objectives :"), objectivestext}

  menu = BosMenu(_("Briefing level 1 The river"))

  local sw = ScrollingWidget(400, Video.Height * 12 / 20)
  menu:add(sw, Video.Width / 2 - 200, Video.Height / 20 * 3)
  sw:setBackgroundColor(dark)
  sw:setActionCallback(function() sw:restart() end)
  local lastpos = 50
  for i,f in briefingtext do
    sw:add(Label(f), 20, lastpos)
    lastpos = lastpos + 20
  end
  lastpos = lastpos +  20
  for j,f in objectivestext do
    sw:add(Label(f), 20, lastpos)
    lastpos = lastpos + 20
  end

  menu:run()
end


function CreateMapStep(map, objectivestext, briefingtext)
   function RunCampaignMap()
     RunBriefingMenu(objectivestext, briefingtext)
     Load(map) -- Needed to force the load of the presentation
     RunMap(map, objectivestext) 
   end
   return RunCampaignMap
end


function RunCampaign(campaign)
  Load(campaign)
  currentCampaign = campaign
  if position == nil then
    position = 1
  end
  while position < 10 do
    campaign_steps[position]()
    if GameResult == GameVictory then
       position = position + 1
    elseif GameResult == GameDefeat then
       position = position
    else
      currentCampaign = nil
      return
    end
  end
  currentCampaign = nil
end


function RunCampaignsMenu(s)
  local menu
  local b

  menu = BosMenu(_("List of Campaigns"))

  local browser = menu:addBrowser("campaigns/", "^%a", 300, 100, 300, 200, ListDirsInDirectory)
  function startgamebutton(s)
    print("Starting campaign")
    RunCampaign("campaigns/" .. browser:getSelectedItem() .. "/campaign.lua")
    menu:stop()
  end
  menu:addButton(_("Start"), 100, 300, startgamebutton)

  menu:run()
end

