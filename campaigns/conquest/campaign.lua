--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      campaign.lua  -  Define the Conquest campaign.
--
--      (c) Copyright 2010 by Jimmy Salmon
--
--      This program is free software; you can redistribute it and/or modify
--      it under the terms of the GNU General Public License as published by
--      the Free Software Foundation; either version 2 of the License, or
--      (at your option) any later version.
--  
--      This program is distributed in the hope that it will be useful,
--      but WITHOUT ANY WARRANTY; without even the implied warranty of
--      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--      GNU General Public License for more details.
--  
--      You should have received a copy of the GNU General Public License
--      along with this program; if not, write to the Free Software
--      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
--

local function transx(x)
  return (Video.Width - 640) / 2 + x
end
local function transy(y)
  return (Video.Height - 480) / 2 + y
end

function ConquestBriefingMenu(data)
  local menu
  local b
  local channel = nil

  SetObjectives(data.Objectives)

  menu = BosMenu(data.Title, data.Background)

  local blankLabel = Label("")
  blankLabel:setFont(Fonts["large"])

  local sw = ScrollingWidget(330, 190)
  sw:setBackgroundColor(dark)
  sw:setSpeed(0.5)
  local xPadding = 5
  local y = sw:getHeight()
  for i,f in ipairs(data.Briefing) do
    local t = MultiLineLabel(f)
    t:setFont(Fonts["large"])
    t:setAlignment(MultiLineLabel.LEFT)
    t:setLineWidth(sw:getWidth() - xPadding * 2)
    t:adjustSize()

    sw:add(t, xPadding, y)
    y = y + t:getHeight()
    sw:add(blankLabel, 0, y)
    y = y + blankLabel:getHeight()
  end
  menu:add(sw, transx(30), transy(80))

  local objContainer = Container()
  objContainer:setSize(225, 110)
  objContainer:setBaseColor(dark)
  local objText = "Objectives:\n"
  for i,f in ipairs(data.Objectives) do
    objText = objText .. "- " .. f .. "\n"
  end
  local objs = MultiLineLabel(objText)
  objs:setFont(Fonts["large"])
  objs:setAlignment(MultiLineLabel.LEFT)
  objs:setLineWidth(objContainer:getWidth() - xPadding * 2)
  objs:adjustSize()
  objContainer:add(objs, xPadding, xPadding)
  menu:add(objContainer, transx(385), transy(245))

  menu:addButton(_("~!Start"), transx(220), transy(380),
    function()
      if (channel ~= nil) then StopChannel(channel) end
      menu:stop()
    end)
  menu:addButton(_("Cancel (~<Esc~>)"), transx(220), transy(415),
    function()
      if (channel ~= nil) then StopChannel(channel) end
      menu:stop(1)
    end)

  if (data.Sound ~= nil) then
    channel = PlaySoundFile(data.Sound, function() channel = nil end)
  end

  return menu:run()
end

function CreateMapStep(data)
  function RunCampaignMap()
    if (ConquestBriefingMenu(data) ~= 0) then
      GameResult = GameQuitToMenu
      return
    end
    Load(data.Map) -- Needed to force the load of the presentation
    RunMap(data.Map, data.Objectives) 
  end
  return RunCampaignMap
end


campaign_steps = {
  CreateMapStep({
    Map = "campaigns/conquest/01/presentation.smp",
    Title = "I. Outpost 31",
    Objectives = {
      "Establish Outpost 31"
    },
    Briefing = {
      "Construction has begun on Outpost 31 near the border of the sinister Talius empire.",
      "This new outpost will serve as a crucial lookout point for monitoring enemy troop movements and gathering intelligence.",
      "Because of its strategic importance, you have been assigned to oversee its completion.",
      "You are to lead a small group of soldiers to the outpost and get it fully operational.",
      "Stay alert, there could be enemy forces in the vicinity.",
    },
    Sound = nil,
    Background = "campaigns/conquest/conquest.png",
  })
}

