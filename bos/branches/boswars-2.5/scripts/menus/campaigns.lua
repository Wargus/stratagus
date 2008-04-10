--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
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


function RunBriefingMenu(objectivestext, briefingtext, briefingsound)
  local menu
  local b
  local channel = nil

  if objectivestext == nil then
    SetObjectives(default_objective)
    return
  end

  SetObjectives(objectivestext)
  menu = BosMenu(_("Briefing"))
  local text = briefingtext .. 
      "\n\n" ..
      _("Objectives:") ..
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

  menu:addButton(_("~!Start"), Video.Width / 2 - 100, Video.Height - 100,
    function()
      if (channel ~= nil) then StopChannel(channel) end
      menu:stop()
    end)

  if (briefingsound ~= nil) then
    channel = PlaySoundFile(briefingsound, function() channel = nil end)
  end

  menu:run()
end

function AddCampaignMessage(when, text)
  AddTrigger(
    function() return (GameCycle() >= when) end,
    function() return AddMessage(text) end
  )
end

function AddCampaignFinalAssault(when, text)
  AddTrigger(
    function() return (GameCycle() >= when) end,
    function() 
      AddMessage(text)
      CreateUnit("unit-vault", 1, {60, 8})
      CreateUnit("unit-tank", 1, {59, 12})
      CreateUnit("unit-tank", 1, {61, 12})
      CreateUnit("unit-tank", 1, {59, 14})
      CreateUnit("unit-tank", 1, {60, 14})
      AiForce(9, {"unit-tank", 4, "unit-rtank", 3})
      AiAttackWithForce(9)
    end
  )
end


function CreateMapStep(map, objectivestext, briefingtext, briefingsound)
   function RunCampaignMap()
     RunBriefingMenu(objectivestext, briefingtext, briefingsound)
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

  ResetMapOptions()
  local lister = CreateFilteringLister("^%a",  ListDirsInDirectory)
  local browser = menu:addBrowser("campaigns/", lister,
                                 Video.Width / 2 - 150, 100, 300, 200)
  function startgamebutton(s)
    print("Starting campaign")
    position = nil
    RunCampaign("campaigns/" .. browser:getSelectedItem() .. "/campaign.lua")
    menu:stop()
  end
  menu:addButton(_("~!Main Menu"), Video.Width / 2 - 250, Video.Height - 100,
                 function() menu:stop() end)
  menu:addButton(_("~!Start"), Video.Width / 2 + 50 ,  Video.Height - 100,
                 startgamebutton)

  menu:run()
end

