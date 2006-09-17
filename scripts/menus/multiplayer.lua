--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	multiplayer.lua	-	Multiplayer menus configuration
--
--	(c) Copyright 2002-2005 by Kachalov Anton, François Beerten 
--                     and Jimmy Salmon.
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
--	$Id$


--
-- menu-enter-name
--
DefineMenu("name", "menu-enter-name", "geometry", {176, 260, 288, 128},
  "panel", "panel4", "background", MenuBackground, "default", 2)
DefineMenuItem("pos", { 144, 11}, "font", "game",
  "text", {"caption", "Enter your name:", "align", "center"},
  "menu", "menu-enter-name")
DefineMenuItem("pos", { 40, 38}, "font", "game",
  "input", {"size", {212, 20},
    "func", "enter-name-action",
    "style", "pulldown"},
  "menu", "menu-enter-name")
DefineMenuItem("pos", { 24, 80}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-enter-name")
DefineMenuItem("pos", { 154, 80}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "enter-name-cancel",
    "style", "gm-half"},
  "menu", "menu-enter-name")


--
-- menu-multi-setup
--
DefineMenu("name", "menu-multi-setup", "geometry", {0, 0, 640, 480},
  "init", "multi-game-setup-init", "exit", "multi-game-setup-exit",
  "panel", "none", "background", MenuBackground, "default", 3)
DefineMenuItem("pos", { 0, 0}, "font", "game", 
  "drawfunc", "multi-game-draw-func",
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { (640 / 2) + 12, 8}, "font", "large",
  "text", {"caption", "~<Multi Player Setup~>", "align", "center"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 640 - 224 - 16, 360 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "S~!elect Map",
    "hotkey", "e",
    "func", "multi-scen-select",
    "style", "gm-full"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 640 - 224 - 16, 360 + (36 * 1)}, "font", "large", "flags", {"disabled"},
  "button", {
    "caption", "~!Start Game",
    "hotkey", "s",
    "func", "multi-game-start",
    "style", "gm-full"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 640 - 224 - 16, 360 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "~!Cancel Game",
    "hotkey", "c",
    "func", "multi-game-cancel",
    "style", "gm-full"},
  "menu", "menu-multi-setup")
for i = 0, 7 do
  DefineMenuItem("pos", { 40, 32 + (22 * i)}, "font", "game",
    "pulldown",  {"size", {172, 20},
      "style", "pulldown",
      "func", nil,
      "options", {"Available", "Computer", "Closed" },
      "default", 0,
      "current", 0},
    "menu", "menu-multi-setup")
end
for i = 0, 6 do
  DefineMenuItem("pos", { 360, 32 + (22 * i)}, "font", "game",
    "pulldown",  {"size", {172, 20},
      "style", "pulldown",
      "func", nil,
      "options", {"Available", "Computer", "Closed" },
      "default", 0,
      "current", 0},
    "menu", "menu-multi-setup")
end
DefineMenuItem("pos", { 40, (10 + 240) - 20}, "font", "game",
  "text", {"caption", "~<Your Race:~>"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 40, 10 + 240}, "font", "game",
  "pulldown",  {"size", {152, 20},
    "style", "pulldown",
    "func", "game-rcs-action"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 220, (10 + 240) - 20}, "font", "game",
  "text", {"caption", "~<Resources:~>"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 220, 10 + 240}, "font", "game",
  "pulldown",  {"size", {152, 20},
    "style", "pulldown",
    "func", "game-res-action",
    "options", {"Map Default", "Low", "Medium", "High" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 640 - 224 - 16, (10 + 240) - 20}, "font", "game",
  "text", {"caption", "~<Units:~>"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 640 - 224 - 16, 10 + 240}, "font", "game",
  "pulldown",  {"size", {190, 20},
    "style", "pulldown",
    "func", "game-uns-action",
    "options", {"Map Default", "One Peasant Only" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 40, (10 + 300) - 20}, "font", "game",
  "text", {"caption", "~<Fog of War:~>"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 40, 10 + 300}, "font", "game",
  "pulldown",  {"size", {152, 20},
    "style", "pulldown",
    "func", "multi-game-fws-action",
    "options", {"Fog of War", "No Fog of War", "Reveal map", "Reveal map,No fog" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 220, (10 + 300) - 20}, "font", "game",
  "text", {"caption", "~<Map Tileset:~>"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 220, 10 + 300}, "font", "game",
  "pulldown",  {"size", {152, 20},
    "options", {"default"},
    "style", "pulldown",
    "func", "game-tss-action"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 640 - 224 - 16, (10 + 300) - 20}, "font", "game",
  "text", {"caption", "~<Game Type:~>"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 640 - 224 - 16, 10 + 300}, "font", "game",
  "pulldown",  {"size", {190, 20},
    "style", "pulldown",
    "func", "game-gat-action",
    "options", {"Use map settings", "Melee", "Free for all", "Top vs bottom", "Left vs right", "Man vs Machine" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 10, 32 + (22 * 1)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 10, 32 + (22 * 2)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 10, 32 + (22 * 3)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 10, 32 + (22 * 4)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 10, 32 + (22 * 5)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 10, 32 + (22 * 6)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 10, 32 + (22 * 7)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 330, 32 + (22 * 0)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 330, 32 + (22 * 1)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 330, 32 + (22 * 2)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 330, 32 + (22 * 3)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 330, 32 + (22 * 4)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 330, 32 + (22 * 5)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 330, 32 + (22 * 6)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "square"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 218, 32 + (22 * 1)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 218, 32 + (22 * 2)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 218, 32 + (22 * 3)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 218, 32 + (22 * 4)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 218, 32 + (22 * 5)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 218, 32 + (22 * 6)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 218, 32 + (22 * 7)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 538, 32 + (22 * 0)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 538, 32 + (22 * 1)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 538, 32 + (22 * 2)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 538, 32 + (22 * 3)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 538, 32 + (22 * 4)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 538, 32 + (22 * 5)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 538, 32 + (22 * 6)}, "font", "large",
  "checkbox", {
    "state", "passive",
    "func", nil,
    "style", "round"},
  "menu", "menu-multi-setup")

--
-- menu-enter-server
--
DefineMenu("name", "menu-enter-server", "geometry", {176, 260, 288, 128},
  "panel", "panel4", "background", MenuBackground, "default", 3)
DefineMenuItem("pos", { 144, 11}, "font", "game",
  "text", {"caption", "Enter server IP-address:", "align", "center"},
  "menu", "menu-enter-server")
DefineMenuItem("pos", { 40, 38}, "font", "game",
  "input", {"size", {212, 20},
    "func", "enter-server-ip-action",
    "style", "pulldown"},
  "menu", "menu-enter-server")
DefineMenuItem("pos", { 24, 80}, "font", "large", "flags", {"disabled"},
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-enter-server")
DefineMenuItem("pos", { 154, 80}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "enter-server-ip-cancel",
    "style", "gm-half"},
  "menu", "menu-enter-server")


--
-- menu-net-multi-client
--
DefineMenu("name", "menu-net-multi-client", "geometry", {0, 0, 640, 480},
  "init", "multi-game-client-init", "exit", "multi-game-client-exit",
  "panel", "none", "background", MenuBackground, "default", 4, "netaction", "terminate-net-connect")
DefineMenuItem("pos", { 0, 0}, "font", "game", 
  "drawfunc", "multi-client-draw-func",
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { (640 / 2) + 12, 8}, "font", "large",
  "text", {"caption", "~<Multi Player Game~>", "align", "center"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 640 - 224 - 16, 360 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "~!Ready",
    "hotkey", "r",
    "func", "multi-client-ready",
    "style", "gm-full"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 640 - 224 - 16, 360 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "~!Not Ready",
    "hotkey", "n",
    "func", "multi-client-not-ready",
    "style", "gm-full"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 640 - 224 - 16, 360 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "~!Cancel Game",
    "hotkey", "c",
    "func", "multi-client-cancel",
    "style", "gm-full"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, 32 + (22 * 0)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, 32 + (22 * 1)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, 32 + (22 * 2)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, 32 + (22 * 3)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, 32 + (22 * 4)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, 32 + (22 * 5)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, 32 + (22 * 6)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, 32 + (22 * 7)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 360, 32 + (22 * 0)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 360, 32 + (22 * 1)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 360, 32 + (22 * 2)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 360, 32 + (22 * 3)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 360, 32 + (22 * 4)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 360, 32 + (22 * 5)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 360, 32 + (22 * 6)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, (10 + 240) - 20}, "font", "game",
  "text", {"caption", "~<Your Race:~>"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, (10 + 240)}, "font", "game",
  "pulldown",  {"size", {152, 20},
    "style", "pulldown",
    "func", "multi-client-rcs-action"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 220, (10 + 240) - 20}, "font", "game",
  "text", {"caption", "~<Resources:~>"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 220, 10 + 240}, "font", "game",
  "pulldown",  {"size", {152, 20},
    "style", "pulldown",
    "func", "game-res-action",
    "options", {"Map Default", "Low", "Medium", "High" },
    "state", "passive",
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 640 - 224 - 16, (10 + 240) - 20}, "font", "game",
  "text", {"caption", "~<Units:~>"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 640 - 224 - 16, 10 + 240}, "font", "game",
  "pulldown",  {"size", {190, 20},
    "style", "pulldown",
    "func", "game-uns-action",
    "options", {"Map Default", "One Peasant Only" },
    "state", "passive",
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, (10 + 300) - 20}, "font", "game",
  "text", {"caption", "~<Fog of War:~>"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 40, 10 + 300}, "font", "game",
  "pulldown",  {"size", {152, 20},
    "style", "pulldown",
    "func", "multi-game-fws-action",
    "options", {"Fog of War", "No Fog of War", "Reveal map", "Reveal map,No fog" },
    "state", "passive",
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 220, (10 + 300) - 20}, "font", "game",
  "text", {"caption", "~<Map Tileset:~>"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 220, 10 + 300}, "font", "game",
  "pulldown",  {"size", {152, 20},
    "style", "pulldown",
    "options", {"default"},
    "func", "game-tss-action",
    "state", "passive"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 640 - 224 - 16, (10 + 300) - 20}, "font", "game",
  "text", {"caption", "~<Game Type:~>"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 640 - 224 - 16, 10 + 300}, "font", "game",
  "pulldown",  {"size", {190, 20},
    "style", "pulldown",
    "func", "game-gat-action",
    "options", {"Use map settings", "Melee", "Free for all", "Top vs bottom", "Left vs right", "Man vs Machine" },
    "state", "passive",
    "default", 0,
    "current", 0},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 10, 32 + (22 * 1)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 10, 32 + (22 * 2)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 10, 32 + (22 * 3)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 10, 32 + (22 * 4)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 10, 32 + (22 * 5)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 10, 32 + (22 * 6)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 10, 32 + (22 * 7)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 330, 32 + (22 * 0)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 330, 32 + (22 * 1)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 330, 32 + (22 * 2)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 330, 32 + (22 * 3)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 330, 32 + (22 * 4)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 330, 32 + (22 * 5)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")
DefineMenuItem("pos", { 330, 32 + (22 * 6)}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "multi-client-checkbox-action",
    "style", "square"},
  "menu", "menu-net-multi-client")


--
-- menu-net-connecting
--
DefineMenu("name", "menu-net-connecting", "geometry", {176, 260, 288, 128},
  "init", "net-connecting-init", "exit", "net-connecting-exit",
  "panel", "panel4", "background", MenuBackground, "default", 2, "netaction", "terminate-net-connect")
DefineMenuItem("pos", { 144, 11}, "font", "large", 
  "text", {"caption", "Connecting to server", "align", "center"},
  "menu", "menu-net-connecting")
DefineMenuItem("pos", { 144, 32}, "font", "large",
  "text", {"caption", nil, "align", "center"},
  "menu", "menu-net-connecting")
DefineMenuItem("pos", { 144, 53}, "font", "large",
  "text", {"caption", nil, "align", "center"},
  "menu", "menu-net-connecting")
DefineMenuItem("pos", { 32, 90}, "font", "large",
  "button", {
    "caption", "Cancel (~<Esc~>)",
    "hotkey", "escape",
    "func", "net-connecting-cancel",
    "style", "gm-full"},
  "menu", "menu-net-connecting")


--
-- menu-create-join-menu
--
DefineMenu("name", "menu-create-join-menu", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 2)
DefineMenuItem("pos", { 208, 320 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "~!Join LAN Game",
    "hotkey", "j",
    "func", "net-join-game",
    "style", "gm-full"},
  "menu", "menu-create-join-menu")
DefineMenuItem("pos", { 208, 320 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "~!Create LAN Game",
    "hotkey", "c",
    "func", "net-create-game",
    "style", "gm-full"},
  "menu", "menu-create-join-menu")
DefineMenuItem("pos", { 208, 320 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "~!Previous Menu",
    "hotkey", "p",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-create-join-menu")

