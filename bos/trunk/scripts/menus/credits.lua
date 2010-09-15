--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      credits.lua - Credits menu.
--
--      (c) Copyright 2005-2007 by Francois Beerten
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
--      $Id: guichan.lua 304 2005-12-18 13:13:15Z feb $

function RunCreditsMenu(s)
  local menu
  local b

  local credits
  credits = {
     _("Graphics:"),
     "  Tina Petersen Jensen",
     "  Soeren Soendergaard Jensen",
     "  TimberDragon",
     "  Frank Loeffler",
     "  Francois Beerten",
     "",
     _("Scripting:"),
     "  Francois Beerten",
     "  Lois Taulelle",
     "",
     _("Maps and campaigns:"),
     "  Lois Taulelle",
     "  Francois Beerten",
     "",
     _("Sound:"),
     "  Tina Petersen",
     "  Brian Pedersen",
     " ",
     _("Engine Programmers:"),
     "  Andreas 'Ari' Arens",
     "  Lutz 'Johns' Sammer",
     "  Edgar 'Froese' Toernig",
     "  Jimmy Salmon",
     "  Nehal Mistry",
     "  Russell Smith",
     "  Francois Beerten",
     "  Joris Dauphin",
     "  Mark Pazolli",
     "  Valery Shchedrin",
     "  Iftikhar Rathore",
     "  Charles K Hardin",
     "  Fabrice Rossi",
     "  DigiCat",
     "  Josh Cogliati",
     "  Patrick Mullen",
     "  Vladi Belperchinov-Shabanski",
     "  Cris Daniluk",
     "  Patrice Fortier",
     "  FT Rathore",
     "  Trent Piepho",
     "  Jon Gabrielson",
     "  Lukas Hejtmanek",
     "  Steinar Hamre",
     "  Ian Farmer",
     "  Sebastian Drews",
     "  Jarek Sobieszek",
     "  Anthony Towns",
     "  Stefan Dirsch",
     "  Al Koskelin",
     "  George J. Carrette",
     "  Dirk 'Guardian' Richartz",
     "  Michael O'Reilly",
     "  Dan Hensley",
     "  Sean McMillian",
     "  Mike Earl",
     "  Ian Turner",
     "  David Slimp",
     "  Iuri Fiedoruk",
     "  Luke Mauldin",
     "  Nathan Adams",
     "  Stephan Rasenberger",
     "  Dave Reed",
     "  Josef Spillner",
     "  James Dessart",
     "  Jan Uerpmann",
     "  Aaron Berger",
     "  Latimerius",
     "  Antonis Chaniotis",
     "  Samuel Hays",
     "  David Martinez Moreno",
     "  Flavio Silvestrow",
     "  Daniel Burrows",
     "  Dave Turner",
     "  Ben Hines",
     "  Kachalov Anton",
     " ",
     _("Patches"),
     "  Martin Renold",
     "  Martin Hajduch",
     "  Jeff Binder",
     "  Ludovic",
     "  Juan Pablo",
     "  Phil Hannent",
     "  Alexander MacLean",
     "",
     _("Stratagus Media Project Graphics"),
     "  Paolo D'Inca", -- green_cross.png
     "  Rick Elliot", -- cursor arrows
     "",
     "",
     _("The Bos Wars Team thanks"),
     _("everybody who has contributed"),
     _("patches, bug reports, ideas.")
  }

  menu = BosMenu(_("Bos Wars Credits"))

  local sw = ScrollingWidget(400, Video.Height * 12 / 20)
  menu:add(sw, Video.Width / 2 - 200, Video.Height / 20 * 3)
  sw:setBackgroundColor(dark)
  sw:setActionCallback(function() sw:restart() end)
  for i,f in ipairs(credits) do
    sw:add(Label(f), 50, 20 * i + 50)
  end

  menu:addButton(_("Main Menu (~<Esc~>)"), Video.Width / 2 - 100, 
                 Video.Height - 100, function() menu:stop() end)

  menu:run()
end

