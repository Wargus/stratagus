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
  menu = BosMenu(_("Briefing"))
  local text = briefingtext .. 
      "\n\n" ..
      _("Objectives :") ..
      "         \n" ..
      objectivestext
  local t= MultiLineLabel(text)
  t:setFont(Fonts["large"])
  t:setAlignment(MultiLineLabel.LEFT)
  t:setVerticalAlignment(MultiLineLabel.CENTER)
  t:setLineWidth(400)
  t:adjustSize()
  t:setHeight(Video.Height * 12 / 20)
  t:setBackgroundColor(dark)
  menu:add(t, Video.Width / 2 - 200, Video.Height / 20 * 3)

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
  while position <= table.getn(campaign_steps) do
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
  menu:addButton(_("Start"), 0, 100, 300, startgamebutton)

  menu:run()
end

