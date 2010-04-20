--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      buttonstyles.lua - Define the widgets
--
--      (c) Copyright 2004 by Jimmy Salmon
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


DefineButtonStyle("network", {
  Size = {80, 12},
  Font = "game",
  TextNormalColor = "yellow",
  TextReverseColor = "white",
  TextAlign = "Center",
  TextPos = {40, 0},
  Default = {
    Size = {300, 144},
  },
  Hover = {
    TextNormalColor = "white",
  },
  Clicked = {
    Size = {300, 144},
    TextNormalColor = "white",
    TextPos = {42, 2},
  },
})

DefineButtonStyle("icon", {
  Size = {46, 38},
  Font = "game",
  TextNormalColor = "yellow",
  TextReverseColor = "white",
  TextAlign = "Right",
  TextPos = {46, 25},
  Default = {
    Border = {
      Color = {0, 0, 0}, Size = 1,
    },
  },
  Hover = {
    TextNormalColor = "white",
    Border = {
      Color = {128, 128, 128}, Size = 1,
    },
  },
  Clicked = {
    TextNormalColor = "white",
    Border = {
      Color = {128, 128, 128}, Size = 1,
    },
  },
})
