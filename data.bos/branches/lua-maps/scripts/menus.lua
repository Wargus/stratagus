--       _________ __                 __                               
--      /   _____//  |_____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--			  T H E   W A R   B E G I N S
--	   Stratagus - A free fantasy real time strategy game engine
--
--	menus.lua	-	Menus configuration
--
--	(c) Copyright 2002-2003 by Kachalov Anton and Jimmy Salmon.
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
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id$

--
-- load the keystroke helps
--
Load("scripts/keystrokes.lua")

MenuBackground = "screens/menu.png"

--
-- define the menu graphics
--
DefineMenuGraphics({
  {"file", "general/ui_buttons.png", "size", {300, 144}},
  {"file", "general/ui_buttons.png", "size", {300, 144}}})

--
-- menu-game
--
DefineMenu("name", "menu-game", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 7)
DefineMenuItem("pos", { 128, 11}, "font", "large", "init", "game-menu-init",
  "text", {"caption", "Game Menu", "align", "center"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16, 40}, "font", "large",
  "button", {
    "caption", "Save (~<F11~>)",
    "hotkey", "f11",
    "func", "save-game-menu",
    "style", "gm-half"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16 + 12 + 106, 40}, "font", "large",
  "button", {
    "caption", "Load (~<F12~>)",
    "hotkey", "f12",
    "func", "load-game-menu",
    "style", "gm-half"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16, 40 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "Options (~<F5~>)",
    "hotkey", "f5",
    "func", "game-options-menu",
    "style", "gm-full"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16, 40 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "Help (~<F1~>)",
    "hotkey", "f1",
    "func", "help-menu",
    "style", "gm-full"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16, 40 + (36 * 3)}, "font", "large",
  "button", {
    "caption", "~!Objectives",
    "hotkey", "o",
    "func", "objectives-menu",
    "style", "gm-full"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16, 40 + (36 * 4)}, "font", "large",
  "button", {
    "caption", "~!End Scenario",
    "hotkey", "e",
    "func", "end-scenario-menu",
    "style", "gm-full"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16, 288 - 40}, "font", "large",
  "button", {
    "caption", "Return to Game (~<Esc~>)",
    "hotkey", "esc",
    "func", "game-menu-return",
    "style", "gm-full"},
  "menu", "menu-game")


--
-- menu-victory
--
DefineMenu("name", "menu-victory", "geometry", {256, 176, 288, 128},
  "panel", "panel4", "default", 2)
DefineMenuItem("pos", { 144, 11}, "font", "large", "init", "victory-init",
  "text", {"caption", "Congratulations!", "align", "center"},
  "menu", "menu-victory")
DefineMenuItem("pos", { 144, 32}, "font", "large",
  "text", {"caption", "You are victorious!", "align", "center"},
  "menu", "menu-victory")
DefineMenuItem("pos", { 32, 56}, "font", "large",
  "button", {
    "caption", "~!Victory",
    "hotkey", "v",
    "func", "game-menu-end",
    "style", "gm-full"},
  "menu", "menu-victory")
DefineMenuItem("pos", { 32, 90}, "font", "large",
  "button", {
    "caption", "Save ~!Replay",
    "hotkey", "r",
    "func", "save-replay",
    "style", "gm-full"},
  "menu", "menu-victory")


--
-- menu-defeated
--
DefineMenu("name", "menu-defeated", "geometry", {256, 176, 288, 128},
  "panel", "panel4", "default", 2)
DefineMenuItem("pos", { 144, 11}, "font", "large", "init", "defeated-init",
  "text", {"caption", "You have failed to", "align", "center"},
  "menu", "menu-defeated")
DefineMenuItem("pos", { 144, 32}, "font", "large",
  "text", {"caption", "achieve victory!", "align", "center"},
  "menu", "menu-defeated")
DefineMenuItem("pos", { 32, 56}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "game-menu-end",
    "style", "gm-full"},
  "menu", "menu-defeated")
DefineMenuItem("pos", { 32, 90}, "font", "large",
  "button", {
    "caption", "Save ~!Replay",
    "hotkey", "r",
    "func", "save-replay",
    "style", "gm-full"},
  "menu", "menu-defeated")

--
-- menu-save-replay
--
DefineMenu("name", "menu-save-replay", "geometry", {256, 176, 288, 128},
  "panel", "panel4", "default", 1)
DefineMenuItem("pos", { 144, 11}, "font", "large", "init", "save-replay-init", "exit", "save-replay-exit",
  "text", {"caption", "Save Replay", "align", "center"},
  "menu", "menu-save-replay")
DefineMenuItem("pos", { 14, 40}, "font", "game",
  "input", {"size", {260, 20},
    "func", "save-replay-enter-action",
    "style", "pulldown"},
  "menu", "menu-save-replay")
DefineMenuItem("pos", { 14, 80}, "font", "large", "flags", {"disabled"},
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "save-replay-ok",
    "style", "gm-half"},
  "menu", "menu-save-replay")
DefineMenuItem("pos", { 162, 80}, "font", "large",
  "button", {
    "caption", "Cancel (~<Esc~>)",
    "hotkey", "esc",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-save-replay")


--
-- menu-select-scenario
--
DefineMenu("name", "menu-select-scenario", "geometry", {144, 64, 352, 352},
  "panel", "panel5", "background", MenuBackground, "default", 4)
DefineMenuItem("pos", { 176, 8}, "font", "large", "init", "scen-select-init",
  "text", {"caption", "Select map", "align", "center"},
  "menu", "menu-select-scenario")
DefineMenuItem("pos", { 24, 140}, "font", "game", "init", "scen-select-lb-init", "exit", "scen-select-lb-exit",
  "listbox", {"size", {288, 108},
    "style", "pulldown",
    "func", "scen-select-lb-action",
    "retopt", "scen-select-lb-retrieve",
    "handler", "scen-select-ok",
    "nlines", 6},
  "menu", "menu-select-scenario")
DefineMenuItem("pos", { 312, 140}, "font", "small",
  "vslider", {"size", {18, 108},
    "func", "scen-select-vs-action",
    "handler", "scen-select-ok"},
  "menu", "menu-select-scenario")
DefineMenuItem("pos", { 48, 318}, "font", "large",
  "button", {
    "caption", "OK",
    "hotkey", "",
    "func", "scen-select-ok",
    "style", "gm-half"},
  "menu", "menu-select-scenario")
DefineMenuItem("pos", { 198, 318}, "font", "large",
  "button", {
    "caption", "Cancel",
    "hotkey", "",
    "func", "scen-select-cancel",
    "style", "gm-half"},
  "menu", "menu-select-scenario")
DefineMenuItem("pos", { 132, 40}, "font", "large",
  "text", {"caption", "Type:", "align", "right"},
  "menu", "menu-select-scenario")
DefineMenuItem("pos", { 140, 40}, "font", "game",
  "pulldown",  {"size", {192, 20},
    "style", "pulldown",
    "func", "scen-select-tpms-action",
    "options", {"Stratagus map (smp)", "Foreign map (pud)" },
    "default", 1,
    "current", 1},
  "menu", "menu-select-scenario")
DefineMenuItem("pos", { 132, 80}, "font", "large",
  "text", {"caption", "Map size:", "align", "right"},
  "menu", "menu-select-scenario")
DefineMenuItem("pos", { 140, 80}, "font", "game",
  "pulldown",  {"size", {192, 20},
    "style", "pulldown",
    "func", "scen-select-tpms-action",
    "options", {"Any size", "32 x 32", "64 x 64", "96 x 96", "128 x 128", "256 x 256", "512 x 512", "1024 x 1024" },
    "default", 0,
    "current", 0},
  "menu", "menu-select-scenario")
DefineMenuItem("pos", { 22, 112}, "font", "game",
  "button", {
    "caption", "",
    "hotkey", "",
    "func", "scen-select-folder",
    "style", "folder"},
  "menu", "menu-select-scenario")


--
-- menu-program-start
--
DefineMenu("name", "menu-program-start", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 1)
DefineMenuItem("pos", { 0, 0}, "font", "game", "init", "program-start",
  "drawfunc", "name-line-draw",
  "menu", "menu-program-start")
DefineMenuItem("pos", { 208, 109 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "~!Single Player Game",
    "hotkey", "s",
    "func", "single-player-game-menu",
    "style", "gm-full"},
  "menu", "menu-program-start")
DefineMenuItem("pos", { 208, 109 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "~!Multi Player Game",
    "hotkey", "m",
    "func", "multi-player-game-menu",
    "style", "gm-full"},
  "menu", "menu-program-start")
DefineMenuItem("pos", { 208, 109 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "~!Campaign Game",
    "hotkey", "c",
    "func", "campaign-game-menu",
    "style", "gm-full"},
  "menu", "menu-program-start")
DefineMenuItem("pos", { 208, 109 + (36 * 3)}, "font", "large",
  "button", {
    "caption", "~!Load Game",
    "hotkey", "l",
    "func", "load-game-menu",
    "style", "gm-full"},
  "menu", "menu-program-start")
DefineMenuItem("pos", { 208, 109 + (36 * 4)}, "font", "large",
  "button", {
    "caption", "~!Replay Game",
    "hotkey", "r",
    "func", "replay-game-menu",
    "style", "gm-full"},
  "menu", "menu-program-start")
DefineMenuItem("pos", { 208, 109 + (36 * 5)}, "font", "large",
  "button", {
    "caption", "~!Options",
    "hotkey", "o",
    "func", "global-options-menu",
    "style", "gm-full"},
  "menu", "menu-program-start")
DefineMenuItem("pos", { 208, 109 + (36 * 6)}, "font", "large",
  "button", {
    "caption", "~!Editor",
    "hotkey", "e",
    "func", "game-start-editor",
    "style", "gm-full"},
  "menu", "menu-program-start")
DefineMenuItem("pos", { 208, 109 + (36 * 7)}, "font", "large",
  "button", {
    "caption", "S~!how Credits",
    "hotkey", "h",
    "func", "game-show-credits",
    "style", "gm-full"},
  "menu", "menu-program-start")
DefineMenuItem("pos", { 208, 109 + (36 * 8)}, "font", "large",
  "button", {
    "caption", "E~!xit Program",
    "hotkey", "x",
    "func", "game-menu-exit",
    "style", "gm-full"},
  "menu", "menu-program-start")

--
-- menu-global-options
--
DefineMenu("name", "menu-global-options", "geometry", {144, 64, 352, 352},
  "panel", "panel5", "background", MenuBackground, "default", 7)
DefineMenuItem("pos", { 176, 11}, "font", "large", "init", "global-options-init", "exit", "global-options-exit",
  "text", {"caption", "Global Options", "align", "center"},
  "menu", "menu-global-options")
DefineMenuItem("pos", { 16, 44}, "font", "game",
  "text", {"caption", "Video Resolution", "align", "left"},
  "menu", "menu-global-options")

DefineMenuItem("pos", { 16, 65},
  "checkbox", {
    "text", "640 x 480",
    "state", "unchecked",
    "func", "global-options-resolution-checkbox",
    "style", "round"},
  "menu", "menu-global-options")

DefineMenuItem("pos", { 16, 91},
  "checkbox", {
    "text", "800 x 600",
    "state", "unchecked",
    "func", "global-options-resolution-checkbox",
    "style", "round"},
  "menu", "menu-global-options")

DefineMenuItem("pos", { 16, 117},
  "checkbox", {
    "text", "1024 x 768",
    "state", "unchecked",
    "func", "global-options-resolution-checkbox",
    "style", "round"},
  "menu", "menu-global-options")

DefineMenuItem("pos", { 16, 143},
  "checkbox", {
    "text", "1280 x 960",
    "state", "unchecked",
    "func", "global-options-resolution-checkbox",
    "style", "round"},
  "menu", "menu-global-options")

DefineMenuItem("pos", { 16, 169},
  "checkbox", {
    "text", "1600 x 1200",
    "state", "unchecked",
    "func", "global-options-resolution-checkbox",
    "style", "round"},
  "menu", "menu-global-options")

DefineMenuItem("pos", { 17, 195},
  "checkbox", {
    "text", "Fullscreen",
    "state", "unchecked",
    "func", "global-options-fullscreen-checkbox",
    "style", "square"},
  "menu", "menu-global-options")

DefineMenuItem("pos", { 192, 44}, "font", "game",
  "text", {"caption", "Shadow Fog", "align", "left"},
  "menu", "menu-global-options")

DefineMenuItem("pos", { 192, 65},
  "checkbox", {
    "text", "Original",
    "state", "unchecked",
    "func", "global-options-fog-original-checkbox",
    "style", "round"},
  "menu", "menu-global-options")

DefineMenuItem("pos", { 192, 91},
  "checkbox", {
    "text", "Alpha",
    "state", "unchecked",
    "func", "global-options-fog-alpha-checkbox",
    "style", "round"},
  "menu", "menu-global-options")

DefineMenuItem("pos", { 123, 309}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-global-options")

--
-- menu-custom-game
--
DefineMenu("name", "menu-custom-game", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 3)
DefineMenuItem("pos", { 0, 0}, "font", "game", "init", "game-setup-init",
  "drawfunc", "game-draw-func",
  "menu", "menu-custom-game")
DefineMenuItem("pos", { (640 / 2) + 12, 192}, "font", "large",
  "text", {"caption", "~<Single Player Game Setup~>", "align", "center"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 640 - 224 - 16, 360 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "S~!elect Map",
    "hotkey", "e",
    "func", "scen-select-menu",
    "style", "gm-full"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 640 - 224 - 16, 360 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "~!Start Game",
    "hotkey", "s",
    "func", "custom-game-start",
    "style", "gm-full"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 640 - 224 - 16, 360 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "~!Cancel Game",
    "hotkey", "c",
    "func", "game-cancel",
    "style", "gm-full"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 40, (10 + 240) - 20}, "font", "game",
  "text", {"caption", "~<Your Race:~>"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 40, 10 + 240}, "font", "game",
  "pulldown", {"size", {152, 20},
    "style", "pulldown",
    "func", "game-rcs-action"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 220, (10 + 240) - 20}, "font", "game",
  "text", {"caption", "~<Resources:~>"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 220, 10 + 240}, "font", "game",
  "pulldown", {"size", {152, 20},
    "style", "pulldown",
    "func", "game-res-action",
    "options", {"Map Default", "Low", "Medium", "High" },
    "default", 0,
    "current", 0},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 640 - 224 - 16, (10 + 240) - 20}, "font", "game",
  "text", {"caption", "~<Units:~>"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 640 - 224 - 16, 10 + 240}, "font", "game",
  "pulldown", {"size", {190, 20},
    "style", "pulldown",
    "func", "game-uns-action",
    "options", {"Map Default", "One Peasant Only" },
    "default", 0,
    "current", 0},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 40, (10 + 300) - 20}, "font", "game",
  "text", {"caption", "~<Opponents:~>"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 40, 10 + 300}, "font", "game",
  "pulldown", {"size", {152, 20},
    "style", "pulldown",
    "func", "custom-game-ops-action",
    "options", {"Map Default", "1 Opponent", "2 Opponents", "3 Opponents", "4 Opponents", "5 Opponents", "6 Opponents", "7 Opponents", "8 Opponents", "9 Opponents", "10 Opponents", "11 Opponents", "12 Opponents", "13 Opponents", "14 Opponents" },
    "default", 0,
    "current", 0},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 220, (10 + 300) - 20}, "font", "game",
  "text", {"caption", "~<Map Tileset:~>"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 220, 10 + 300}, "font", "game",
  "pulldown", {"size", {152, 20},
    "style", "pulldown",
    "func", "game-tss-action"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 640 - 224 - 16, (10 + 300) - 20}, "font", "game",
  "text", {"caption", "~<Game Type:~>"},
  "menu", "menu-custom-game")
DefineMenuItem("pos", { 640 - 224 - 16, 10 + 300}, "font", "game",
  "pulldown", {"size", {190, 20},
    "style", "pulldown",
    "func", "game-gat-action",
    "options", {"Use map settings", "Melee", "Free for all", "Top vs bottom", "Left vs right", "Man vs Machine" },
    "default", 0,
    "current", 0},
  "menu", "menu-custom-game")


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

--[[ disable until internet play is working
--
-- menu-internet-create-join-menu
--
DefineMenu("name", "menu-internet-create-join-menu", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 2)
DefineMenuItem("pos", { 208, 320 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "~!Join Internet Game",
    "hotkey", "j",
    "func", "metaserver-list",
    "style", "gm-full"},
  "menu", "menu-internet-create-join-menu")
DefineMenuItem("pos", { 208, 320 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "~!Create Internet Game",
    "hotkey", "c",
    "func", "net-internet-create-game",
    "style", "gm-full"},
  "menu", "menu-internet-create-join-menu")
DefineMenuItem("pos", { 208, 320 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "~!Previous Menu",
    "hotkey", "p",
    "func", "menu-internet-end-menu",
    "style", "gm-full"},
  "menu", "menu-internet-create-join-menu")
]]

--[[ disable until internet play is working
--
-- menu-multi-net-type-menu
--
DefineMenu("name", "menu-multi-net-type-menu", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 2)
DefineMenuItem("pos", { 208, 320 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "~!LAN/P2P Game",
    "hotkey", "l",
    "func", "net-lan-game",
    "style", "gm-full"},
  "menu", "menu-multi-net-type-menu")
DefineMenuItem("pos", { 208, 320 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "~!Internet Game",
    "hotkey", "i",
    "func", "net-internet-game",
    "style", "gm-full"},
  "menu", "menu-multi-net-type-menu")
DefineMenuItem("pos", { 208, 320 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "~!Previous Menu",
    "hotkey", "p",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-multi-net-type-menu")
]]


--
-- menu-multi-setup
--
DefineMenu("name", "menu-multi-setup", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 3)
DefineMenuItem("pos", { 0, 0}, "font", "game", "init", "multi-game-setup-init", "exit", "multi-game-setup-exit",
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
DefineMenuItem("pos", { 40, 32 + (22 * 0)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 40, 32 + (22 * 1)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 40, 32 + (22 * 2)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 40, 32 + (22 * 3)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 40, 32 + (22 * 4)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 40, 32 + (22 * 5)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 40, 32 + (22 * 6)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 40, 32 + (22 * 7)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 360, 32 + (22 * 0)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 360, 32 + (22 * 1)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 360, 32 + (22 * 2)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 360, 32 + (22 * 3)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 360, 32 + (22 * 4)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 360, 32 + (22 * 5)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
DefineMenuItem("pos", { 360, 32 + (22 * 6)}, "font", "game",
  "pulldown",  {"size", {172, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Available", "Computer", "Closed" },
    "default", 0,
    "current", 0},
  "menu", "menu-multi-setup")
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
  "panel", "none", "background", MenuBackground, "default", 4, "netaction", "terminate-net-connect")
DefineMenuItem("pos", { 0, 0}, "font", "game", "init", "multi-game-client-init", "exit", "multi-game-client-exit",
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
  "panel", "panel4", "background", MenuBackground, "default", 2, "netaction", "terminate-net-connect")
DefineMenuItem("pos", { 144, 11}, "font", "large", "init", "net-connecting-init", "exit", "net-connecting-exit",
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
    "hotkey", "esc",
    "func", "net-connecting-cancel",
    "style", "gm-full"},
  "menu", "menu-net-connecting")


--
-- menu-campaign-select
--
DefineMenu("name", "menu-campaign-select", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 0)
DefineMenuItem("pos", { 208, 212 + (36 * 0)}, "font", "large",
  "button", {
    "caption", nil,
    "hotkey", "a",
    "func", "campaign-1",
    "style", "gm-full"},
  "menu", "menu-campaign-select")
DefineMenuItem("pos", { 208, 212 + (36 * 1)}, "font", "large",
  "button", {
    "caption", nil,
    "hotkey", "m",
    "func", "campaign-2",
    "style", "gm-full"},
  "menu", "menu-campaign-select")
DefineMenuItem("pos", { 208, 212 + (36 * 2)}, "font", "large",
  "button", {
    "caption", nil,
    "hotkey", "l",
    "func", "campaign-3",
    "style", "gm-full"},
  "menu", "menu-campaign-select")
DefineMenuItem("pos", { 208, 212 + (36 * 3)}, "font", "large",
  "button", {
    "caption", nil,
    "hotkey", "y",
    "func", "campaign-4",
    "style", "gm-full"},
  "menu", "menu-campaign-select")
DefineMenuItem("pos", { 208, 212 + (36 * 4)}, "font", "large", "flags", {"disabled"},
  "button", {
    "caption", "~!Select Campaign",
    "hotkey", "s",
    "func", "select-campaign-menu",
    "style", "gm-full"},
  "menu", "menu-campaign-select")
DefineMenuItem("pos", { 208, 212 + (36 * 5)}, "font", "large",
  "button", {
    "caption", "~!Previous Menu",
    "hotkey", "p",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-campaign-select")


--
-- menu-campaign-continue
--
DefineMenu("name", "menu-campaign-continue", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 0)
DefineMenuItem("pos", { 508, 320 + (36 * 3)}, "font", "large",
  "button", {
    "caption", "~!Continue",
    "hotkey", "c",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-campaign-continue")


--
-- menu-objectives
--
DefineMenu("name", "menu-objectives", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 10)
DefineMenuItem("pos", { 128, 11}, "font", "large", "init", "objectives-init",
  "text", {"caption", "Objectives", "align", "center"},
  "menu", "menu-objectives")
DefineMenuItem("pos", { 14, 38 + (21 * 0)}, "font", "large",
  "text", {"caption", nil},
  "menu", "menu-objectives")
DefineMenuItem("pos", { 14, 38 + (21 * 1)}, "font", "large",
  "text", {"caption", nil},
  "menu", "menu-objectives")
DefineMenuItem("pos", { 14, 38 + (21 * 2)}, "font", "large",
  "text", {"caption", nil},
  "menu", "menu-objectives")
DefineMenuItem("pos", { 14, 38 + (21 * 3)}, "font", "large",
  "text", {"caption", nil},
  "menu", "menu-objectives")
DefineMenuItem("pos", { 14, 38 + (21 * 4)}, "font", "large",
  "text", {"caption", nil},
  "menu", "menu-objectives")
DefineMenuItem("pos", { 14, 38 + (21 * 5)}, "font", "large",
  "text", {"caption", nil},
  "menu", "menu-objectives")
DefineMenuItem("pos", { 14, 38 + (21 * 6)}, "font", "large",
  "text", {"caption", nil},
  "menu", "menu-objectives")
DefineMenuItem("pos", { 14, 38 + (21 * 7)}, "font", "large",
  "text", {"caption", nil},
  "menu", "menu-objectives")
DefineMenuItem("pos", { 14, 38 + (21 * 8)}, "font", "large",
  "text", {"caption", nil},
  "menu", "menu-objectives")
DefineMenuItem("pos", { 16, 288 - 40}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-objectives")


--
-- menu-end-scenario
--
DefineMenu("name", "menu-end-scenario", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 5)
DefineMenuItem("pos", { 128, 11}, "font", "large",
  "text", {"caption", "End Scenario", "align", "center"},
  "menu", "menu-end-scenario")
DefineMenuItem("pos", { 16, 40 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "~!Restart Scenario",
    "hotkey", "r",
    "func", "restart-confirm-menu",
    "style", "gm-full"},
  "menu", "menu-end-scenario")
DefineMenuItem("pos", { 16, 40 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "~!Surrender",
    "hotkey", "s",
    "func", "surrender-confirm-menu",
    "style", "gm-full"},
  "menu", "menu-end-scenario")
DefineMenuItem("pos", { 16, 40 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "~!Quit to Menu",
    "hotkey", "q",
    "func", "quit-to-menu-confirm-menu",
    "style", "gm-full"},
  "menu", "menu-end-scenario")
DefineMenuItem("pos", { 16, 40 + (36 * 3)}, "font", "large",
  "button", {
    "caption", "E~!xit Program",
    "hotkey", "x",
    "func", "exit-confirm-menu",
    "style", "gm-full"},
  "menu", "menu-end-scenario")
DefineMenuItem("pos", { 16, 288 - 40}, "font", "large",
  "button", {
    "caption", "Previous (~<Esc~>)",
    "hotkey", "esc",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-end-scenario")


--
-- menu-sound-options
--
DefineMenu("name", "menu-sound-options", "geometry", {224, 64, 352, 352},
  "panel", "panel5", "default", 23)
DefineMenuItem("pos", { 176, 11}, "font", "large", "init", "sound-options-init", "exit", "sound-options-exit",
  "text", {"caption", "Sound Options", "align", "center"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 16, 36 * 1}, "font", "game",
  "text", {"caption", "Master Volume", "align", "left"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 32, 36 * 1.5}, "font", "small",
  "hslider", {"size", {198, 18},
    "func", "master-volume-hs-action",
    "handler", "scen-select-ok"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 44, (36 * 2) + 6}, "font", "small",
  "text", {"caption", "min", "align", "center"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 218, (36 * 2) + 6}, "font", "small",
  "text", {"caption", "max", "align", "center"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 240, 36 * 1.5}, "font", "large",
  "checkbox", {
    "text", "Enabled",
    "state", "unchecked",
    "func", "set-master-power",
    "style", "square"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 16, 36 * 3}, "font", "game",
  "text", {"caption", "Music Volume", "align", "left"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 32, 36 * 3.5}, "font", "small",
  "hslider", {"size", {198, 18},
    "func", "music-volume-hs-action",
    "handler", "scen-select-ok"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 44, (36 * 4) + 6}, "font", "small",
  "text", {"caption", "min", "align", "center"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 218, (36 * 4) + 6}, "font", "small",
  "text", {"caption", "max", "align", "center"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 240, 36 * 3.5}, "font", "large",
  "checkbox", {
    "text", "Enabled",
    "state", "unchecked",
    "func", "set-music-power",
    "style", "square"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 16, 36 * 5}, "font", "game",
  "text", {"caption", "CD Volume", "align", "left"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 32, 36 * 5.5}, "font", "small",
  "hslider", {"size", {198, 18},
    "func", "cd-volume-hs-action",
    "handler", "scen-select-ok"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 44, (36 * 6) + 6}, "font", "small",
  "text", {"caption", "min", "align", "center"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 218, (36 * 6) + 6}, "font", "small",
  "text", {"caption", "max", "align", "center"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 240, 36 * 5.5}, "font", "large",
  "checkbox", {
    "text", "Enabled",
    "state", "unchecked",
    "func", "set-cd-power",
    "style", "square"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 32, 36 * 6.5}, "font", "large",
  "checkbox", {
    "text", "Defined Tracks",
    "state", "unchecked",
    "func", "set-cd-mode-defined",
    "style", "round"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 170, 36 * 6.5}, "font", "large",
  "checkbox", {
    "text", "Random Tracks",
    "state", "unchecked",
    "func", "set-cd-mode-random",
    "style", "round"},
  "menu", "menu-sound-options")
DefineMenuItem("pos", { 176 - (106 / 2), 352 - 11 - 27}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-sound-options")


--
-- menu-preferences
--
DefineMenu("name", "menu-preferences", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 5)
DefineMenuItem("pos", { 128, 11}, "font", "large", "init", "preferences-init", "exit", "preferences-exit",
  "text", {"caption", "Preferences", "align", "center"},
  "menu", "menu-preferences")
DefineMenuItem("pos", { 16, 36 * 1}, "font", "large",
  "checkbox", {
    "text", "Fog of War Enabled",
    "state", "unchecked",
    "func", "set-fog-of-war",
    "style", "square"},
  "menu", "menu-preferences")
DefineMenuItem("pos", { 16, 36 * 2}, "font", "large",
  "checkbox", {
    "text", "Show command key",
    "state", "unchecked",
    "func", "set-command-key",
    "style", "square"},
  "menu", "menu-preferences")
DefineMenuItem("pos", { 128 - (106 / 2), 245}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-preferences")


--
-- menu-diplomacy
--
DefineMenu("name", "menu-diplomacy", "geometry", {222, 76, 352, 352},
  "panel", "panel5", "default", 5)
DefineMenuItem("pos", { 136, 30}, "font", "game", "init", "diplomacy-init", "exit", "diplomacy-exit",
  "text", {"caption", "Allied", "align", "center"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 196, 30}, "font", "game",
  "text", {"caption", "Enemy", "align", "center"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 286, 30}, "font", "game",
  "text", {"caption", "Shared Vision", "align", "center"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 176, 11}, "font", "large",
  "text", {"caption", "Diplomacy", "align", "center"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 1) + 26}, "font", "game",
  "text", {"caption", "Player 1", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 1) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 1) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 1) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 16, (18 * 2) + 26}, "font", "game",
  "text", {"caption", "Player 3", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 2) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 2) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 2) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 3) + 26}, "font", "game",
  "text", {"caption", "Player 4", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 3) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 3) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 3) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 4) + 26}, "font", "game",
  "text", {"caption", "Player 5", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 4) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 4) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 4) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 5) + 26}, "font", "game",
  "text", {"caption", "Player 6", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 5) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 5) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 5) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 6) + 26}, "font", "game",
  "text", {"caption", "Player 7", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 6) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 6) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 6) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 7) + 26}, "font", "game",
  "text", {"caption", "Player 8", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 7) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 7) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 7) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 8) + 26}, "font", "game",
  "text", {"caption", "Player 9", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 8) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 8) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 8) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 9) + 26}, "font", "game",
  "text", {"caption", "Player 10", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 9) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 9) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 9) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 10) + 26}, "font", "game",
  "text", {"caption", "Player 11", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 10) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 10) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 10) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 11) + 26}, "font", "game",
  "text", {"caption", "Player 12", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 11) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 11) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 11) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 12) + 26}, "font", "game",
  "text", {"caption", "Player 13", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 12) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 12) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 12) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 13) + 26}, "font", "game",
  "text", {"caption", "Player 14", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 13) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 13) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 13) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 16, (18 * 14) + 26}, "font", "game",
  "text", {"caption", "Player 15", "align", "left"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 126, (18 * 14) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 186, (18 * 14) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "round"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 276, (18 * 14) + 23}, "font", "large",
  "checkbox", {
    "state", "unchecked",
    "func", "diplomacy-wait",
    "style", "square"},
  "menu", "menu-diplomacy")

DefineMenuItem("pos", { 75, 352 - 40}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "diplomacy-ok",
    "style", "gm-half"},
  "menu", "menu-diplomacy")
DefineMenuItem("pos", { 195, 352 - 40}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-diplomacy")


--
-- menu-speed-options
--
DefineMenu("name", "menu-speed-options", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 13)
DefineMenuItem("pos", { 128, 11}, "font", "large", "init", "speed-options-init", "exit", "speed-options-exit",
  "text", {"caption", "Speed Settings", "align", "center"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 16, 36 * 1}, "font", "game",
  "text", {"caption", "Game Speed", "align", "left"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 32, 36 * 1.5}, "font", "small",
  "hslider", {"size", {198, 18},
    "func", "game-speed-hs-action",
    "handler", "scen-select-ok"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 34, (36 * 2) + 6}, "font", "small",
  "text", {"caption", "slow", "align", "left"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 230, (36 * 2) + 6}, "font", "small",
  "text", {"caption", "fast", "align", "right"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 16, 36 * 3}, "font", "game",
  "text", {"caption", "Mouse Scroll", "align", "left"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 32, 36 * 3.5}, "font", "small",
  "hslider", {"size", {198, 18},
    "func", "mouse-scroll-hs-action",
    "handler", "scen-select-ok"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 34, (36 * 4) + 6}, "font", "small",
  "text", {"caption", "off", "align", "left"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 230, (36 * 4) + 6}, "font", "small",
  "text", {"caption", "fast", "align", "right"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 16, 36 * 5}, "font", "game",
  "text", {"caption", "Keyboard Scroll", "align", "left"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 32, 36 * 5.5}, "font", "small",
  "hslider", {"size", {198, 18},
    "func", "keyboard-scroll-hs-action",
    "handler", "scen-select-ok"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 34, (36 * 6) + 6}, "font", "small",
  "text", {"caption", "off", "align", "left"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 230, (36 * 6) + 6}, "font", "small",
  "text", {"caption", "fast", "align", "right"},
  "menu", "menu-speed-options")
DefineMenuItem("pos", { 128 - (106 / 2), 245}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-speed-options")


--
-- menu-game-options
--
DefineMenu("name", "menu-game-options", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 5)
DefineMenuItem("pos", { 128, 11}, "font", "large",
  "text", {"caption", "Game Options", "align", "center"},
  "menu", "menu-game-options")
DefineMenuItem("pos", { 16, 40 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "Sound (~<F7~>)",
    "hotkey", "f7",
    "func", "sound-options-menu",
    "style", "gm-full"},
  "menu", "menu-game-options")
DefineMenuItem("pos", { 16, 40 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "Speeds (~<F8~>)",
    "hotkey", "f8",
    "func", "speed-options-menu",
    "style", "gm-full"},
  "menu", "menu-game-options")
DefineMenuItem("pos", { 16, 40 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "Preferences (~<F9~>)",
    "hotkey", "f9",
    "func", "preferences-menu",
    "style", "gm-full"},
  "menu", "menu-game-options")
DefineMenuItem("pos", { 16, 40 + (36 * 3)}, "font", "large",
  "button", {
    "caption", "~!Diplomacy",
    "hotkey", "d",
    "func", "diplomacy-menu",
    "style", "gm-full"},
  "menu", "menu-game-options")
DefineMenuItem("pos", { 128 - (224 / 2), 245}, "font", "large",
  "button", {
    "caption", "Previous (~<Esc~>)",
    "hotkey", "esc",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-game-options")


--
-- menu-net-error
--
DefineMenu("name", "menu-net-error", "geometry", {176, 260, 288, 128},
  "panel", "panel4", "default", 2)
DefineMenuItem("pos", { 144, 11}, "font", "large",
  "text", {"caption", "Error:", "align", "center"},
  "menu", "menu-net-error")
DefineMenuItem("pos", { 144, 38}, "font", "large",
  "text", {"caption", nil, "align", "center"},
  "menu", "menu-net-error")
DefineMenuItem("pos", { 92, 80}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-net-error")


--
-- menu-tips
--
DefineMenu("name", "menu-tips", "geometry", {256, 112, 288, 256},
  "panel", "panel2", "default", 4)
DefineMenuItem("pos", { 144, 11}, "font", "large", "init", "tips-init", "exit", "tips-exit",
  "text", {"caption", "Stratagus Tips", "align", "center"},
  "menu", "menu-tips")
DefineMenuItem("pos", { 14, 256 - 75}, "font", "game",
  "checkbox", {
    "text", "Show tips at startup",
    "state", "checked",
    "func", "tips-show-tips-checkbox",
    "style", "square"},
  "menu", "menu-tips")
DefineMenuItem("pos", { 14, 256 - 40}, "font", "large",
  "button", {
    "caption", "~!Next Tip",
    "hotkey", "n",
    "func", "tips-next-tip",
    "style", "gm-half"},
  "menu", "menu-tips")
DefineMenuItem("pos", { 168, 256 - 40}, "font", "large",
  "button", {
    "caption", "~!Close",
    "hotkey", "c",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 0)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 1)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 2)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 3)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 4)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 5)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 6)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 7)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-tips")


--
-- menu-help
--
DefineMenu("name", "menu-help", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 3)
DefineMenuItem("pos", { 128, 11}, "font", "large",
  "text", {"caption", "Help Menu", "align", "center"},
  "menu", "menu-help")
DefineMenuItem("pos", { 16, 40 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "Keystroke ~!Help",
    "hotkey", "h",
    "func", "keystroke-help-menu",
    "style", "gm-full"},
  "menu", "menu-help")
DefineMenuItem("pos", { 16, 40 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "Stratagus ~!Tips",
    "hotkey", "t",
    "func", "tips-menu",
    "style", "gm-full"},
  "menu", "menu-help")
DefineMenuItem("pos", { 128 - (224 / 2), 248}, "font", "large",
  "button", {
    "caption", "Previous (~<Esc~>)",
    "hotkey", "esc",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-help")


--
-- menu-keystroke-help
--
DefineMenu("name", "menu-keystroke-help", "geometry", {224, 64, 352, 352},
  "panel", "panel5", "default", 2)
DefineMenuItem("pos", { 352 / 2, 11}, "font", "large",
  "text", {"caption", "Keystroke Help Menu", "align", "center"},
  "menu", "menu-keystroke-help")
DefineMenuItem("pos", { 352 - 18 - 16, 40 + 20}, "font", "small",
  "vslider", {"size", {18, 216},
    "func", "keystroke-help-vs-action",
    "handler", nil,
    "default", 0},
  "menu", "menu-keystroke-help")
DefineMenuItem("pos", { (352 / 2) - (224 / 2), 352 - 40}, "font", "large",
  "button", {
    "caption", "Previous (~<Esc~>)",
    "hotkey", "esc",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-keystroke-help")
DefineMenuItem("pos", { 16, 40 + 20}, "font", "game",
  "drawfunc", "keystroke-help-draw-func",
  "menu", "menu-keystroke-help")


--
-- menu-save-game
--
DefineMenu("name", "menu-save-game", "geometry", {208, 112, 384, 256},
  "panel", "panel3", "default", 6)
DefineMenuItem("pos", { 384 / 2, 11}, "font", "large",  "init", "save-game-init", "exit", "save-game-exit",
  "text", {"caption", "Save Game", "align", "center"},
  "menu", "menu-save-game")
DefineMenuItem("pos", { (384 - 300 - 18) / 2, 11 + 36}, "font", "game",
  "input", {"size", {300, 20},
    "func", "save-game-enter-action",
    "style", "pulldown"},
  "menu", "menu-save-game")
DefineMenuItem("pos", { (384 - 300 - 18) / 2, 11 + 36 + 22}, "font", "game",  "init", "save-game-lb-init", "exit", "save-game-lb-exit",
  "listbox", {"size", {300, 126},
    "style", "pulldown",
    "func", "save-game-lb-action",
    "retopt", "save-game-lb-retrieve",
    "handler", "save-game-ok",
    "nlines", 7},
  "menu", "menu-save-game")
DefineMenuItem("pos", { ((384 - 300 - 18) / 2) + 300, 11 + 36 + 22}, "font", "small",
  "vslider", {"size", {18, 126},
    "func", "save-game-vs-action",
    "handler", "save-game-ok"},
  "menu", "menu-save-game")
DefineMenuItem("pos", { (1 * (384 / 3)) - 106 - 10, 256 - 16 - 27}, "font", "large",
  "button", {
    "caption", "~!Save",
    "hotkey", "s",
    "func", "save-game-ok",
    "style", "gm-half"},
  "menu", "menu-save-game")
DefineMenuItem("pos", { (2 * (384 / 3)) - 106 - 10, 256 - 16 - 27}, "font", "large",
  "button", {
    "caption", "~!Delete",
    "hotkey", "d",
    "func", "delete-confirm-menu",
    "style", "gm-half"},
  "menu", "menu-save-game")
DefineMenuItem("pos", { (3 * (384 / 3)) - 106 - 10, 256 - 16 - 27}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-save-game")


--
-- menu-load-game
--
DefineMenu("name", "menu-load-game", "geometry", {208, 112, 384, 256},
  "panel", "panel3", "background", MenuBackground, "default", 4)
DefineMenuItem("pos", { 384 / 2, 11}, "font", "large",  "init", "load-game-init", "exit", "load-game-exit",
  "text", {"caption", "Load Game", "align", "center"},
  "menu", "menu-load-game")
DefineMenuItem("pos", { (384 - 300 - 18) / 2, 11 + (36 * 1.5)}, "font", "game",  "init", "load-game-lb-init", "exit", "load-game-lb-exit",
  "listbox", {"size", {300, 126},
    "style", "pulldown",
    "func", "load-game-lb-action",
    "retopt", "load-game-lb-retrieve",
    "handler", "load-game-ok",
    "nlines", 7},
  "menu", "menu-load-game")
DefineMenuItem("pos", { ((384 - 300 - 18) / 2) + 300, 11 + (36 * 1.5)}, "font", "small",
  "vslider", {"size", {18, 126},
    "func", "load-game-vs-action",
    "handler", "load-game-ok"},
  "menu", "menu-load-game")
DefineMenuItem("pos", { (384 - 300 - 18) / 2, 256 - 16 - 27}, "font", "large",
  "button", {
    "caption", "~!Load",
    "hotkey", "l",
    "func", "load-game-ok",
    "style", "gm-half"},
  "menu", "menu-load-game")
DefineMenuItem("pos", { 384 - ((384 - 300 - 18) / 2) - 106, 256 - 16 - 27}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-load-game")


--
-- menu-save-confirm
--
DefineMenu("name", "menu-save-confirm", "geometry", {256, 112, 288, 128},
  "panel", "panel4", "default", 1)
DefineMenuItem("pos", { 288 / 2, 11}, "font", "large",  "init", "save-confirm-init", "exit", "save-confirm-exit",
  "text", {"caption", "Overwrite File", "align", "center"},
  "menu", "menu-save-confirm")
DefineMenuItem("pos", { 16, 11 + (20 * 1.5)}, "font", "game",
  "text", {"caption", "Are you sure you want to overwrite", "align", "left"},
  "menu", "menu-save-confirm")
DefineMenuItem("pos", { 16, 11 + (20 * 2.5)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-save-confirm")
DefineMenuItem("pos", { 16, 128 - (27 * 1.5)}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "save-confirm-ok",
    "style", "gm-half"},
  "menu", "menu-save-confirm")
DefineMenuItem("pos", { 288 - 16 - 106, 128 - (27 * 1.5)}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "save-confirm-cancel",
    "style", "gm-half"},
  "menu", "menu-save-confirm")


--
-- menu-delete-confirm
--
DefineMenu("name", "menu-delete-confirm", "geometry", {256, 112, 288, 128},
  "panel", "panel4", "default", 1)
DefineMenuItem("pos", { 288 / 2, 11}, "font", "large",  "init", "delete-confirm-init", "exit", "delete-confirm-exit",
  "text", {"caption", "Delete File", "align", "center"},
  "menu", "menu-delete-confirm")
DefineMenuItem("pos", { 16, 11 + (20 * 1.5)}, "font", "game",
  "text", {"caption", "Are you sure you want to delete", "align", "left"},
  "menu", "menu-delete-confirm")
DefineMenuItem("pos", { 16, 11 + (20 * 2.5)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-delete-confirm")
DefineMenuItem("pos", { 16, 128 - (27 * 1.5)}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "delete-confirm-ok",
    "style", "gm-half"},
  "menu", "menu-delete-confirm")
DefineMenuItem("pos", { 288 - 16 - 106, 128 - (27 * 1.5)}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "delete-confirm-cancel",
    "style", "gm-half"},
  "menu", "menu-delete-confirm")


--
-- menu-editor-select
--
DefineMenu("name", "menu-editor-select", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 0)
DefineMenuItem("pos", { 0, 0}, "font", "game",
  "drawfunc", "editor-new-draw-func",
  "menu", "menu-editor-select")
DefineMenuItem("pos", { 208, 320 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "~!New Map",
    "hotkey", "n",
    "func", "editor-new-map",
    "style", "gm-full"},
  "menu", "menu-editor-select")
DefineMenuItem("pos", { 208, 320 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "~!Load Map",
    "hotkey", "l",
    "func", "editor-main-load-map",
    "style", "gm-full"},
  "menu", "menu-editor-select")
DefineMenuItem("pos", { 208, 320 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "Cancel (~<Esc~>)",
    "hotkey", "esc",
    "func", "editor-select-cancel",
    "style", "gm-full"},
  "menu", "menu-editor-select")


--
-- menu-editor-new
--
DefineMenu("name", "menu-editor-new", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 0)
DefineMenuItem("pos", { 176 + 16, 112 + (40 + 20)}, "font", "game",
  "drawfunc", "editor-new-draw-func",
  "menu", "menu-editor-new")
DefineMenuItem("pos", { 176 + ((288 - 260) / 2), 112 + (11 + 36)}, "font", "game",
  "text", {"caption", "Map Description:", "align", "left"},
  "menu", "menu-editor-new")
DefineMenuItem("pos", { 176 + ((288 - 260) / 2), 112 + (11 + 36 + 22)}, "font", "game",
  "input", {"size", {260, 20},
    "func", "editor-new-map-description-enter-action",
    "style", "pulldown"},
  "menu", "menu-editor-new")
DefineMenuItem("pos", { 176 + ((288 - 260) / 2), 112 + (11 + (36 * 2) + 22)}, "font", "game",
  "text", {"caption", "Size:", "align", "left"},
  "menu", "menu-editor-new")
DefineMenuItem("pos", { 176 + (288 - ((288 - 260) / 2) - 152), 112 + (11 + (36 * 2) + 22)}, "font", "game",
  "input", {"size", {60, 20},
    "func", "editor-new-map-size-enter-action",
    "style", "pulldown"},
  "menu", "menu-editor-new")
DefineMenuItem("pos", { 176 + (288 - ((288 - 260) / 2) - 60), 112 + (11 + (36 * 2) + 22)}, "font", "game",
  "input", {"size", {60, 20},
    "func", "editor-new-map-size-enter-action",
    "style", "pulldown"},
  "menu", "menu-editor-new")
DefineMenuItem("pos", { 176 + ((288 - 260) / 2), 112 + (11 + (36 * 3) + 22)}, "font", "game",
  "text", {"caption", "Tileset:", "align", "left"},
  "menu", "menu-editor-new")
DefineMenuItem("pos", { 176 + (288 - ((288 - 260) / 2) - 152), 112 + (11 + (36 * 3) + 22)}, "font", "game",
  "pulldown",  {"size", {152, 20},
    "style", "pulldown",
    "func", nil},
  "menu", "menu-editor-new")
DefineMenuItem("pos", { 320 - 106 - 23, 328}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "editor-new-ok",
    "style", "gm-half"},
  "menu", "menu-editor-new")
DefineMenuItem("pos", { 320 + 23, 328}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "editor-new-cancel",
    "style", "gm-half"},
  "menu", "menu-editor-new")


--
-- menu-editor-main-load-map
--
DefineMenu("name", "menu-editor-main-load-map", "geometry", {144, 64, 352, 352},
  "panel", "panel5", "background", MenuBackground, "default", 4)
DefineMenuItem("pos", { 352 / 2, 11}, "font", "large",  "init", "editor-main-load-init",
  "text", {"caption", "Select map", "align", "center"},
  "menu", "menu-editor-main-load-map")
DefineMenuItem("pos", { (352 - 18 - 288) / 2, 11 + 98}, "font", "game",  "init", "editor-main-load-lb-init", "exit", "editor-main-load-lb-exit",
  "listbox", {"size", {288, 108},
    "style", "pulldown",
    "func", "editor-main-load-lb-action",
    "retopt", "editor-main-load-lb-retrieve",
    "handler", "editor-main-load-ok",
    "nlines", 6},
  "menu", "menu-editor-main-load-map")
DefineMenuItem("pos", { ((352 - 18 - 288) / 2) + 288, 11 + 98}, "font", "small",
  "vslider", {"size", {18, 108},
    "func", "editor-main-load-vs-action",
    "handler", "editor-main-load-ok"},
  "menu", "menu-editor-main-load-map")
DefineMenuItem("pos", { 48, 308}, "font", "large",
  "button", {
    "caption", "OK",
    "hotkey", "",
    "func", "editor-main-load-ok",
    "style", "gm-half"},
  "menu", "menu-editor-main-load-map")
DefineMenuItem("pos", { 198, 308}, "font", "large",
  "button", {
    "caption", "Cancel",
    "hotkey", "",
    "func", "editor-main-load-cancel",
    "style", "gm-half"},
  "menu", "menu-editor-main-load-map")
DefineMenuItem("pos", { ((352 - 18 - 288) / 2) - 2, (11 + 98) - 28}, "font", "game",
  "button", {
    "caption", nil,
    "hotkey", "",
    "func", "editor-main-load-folder",
    "style", "folder"},
  "menu", "menu-editor-main-load-map")


--
-- menu-editor-load
--
DefineMenu("name", "menu-editor-load", "geometry", {224, 64, 352, 352},
  "panel", "panel5", "default", 4)
DefineMenuItem("pos", { 352 / 2, 11}, "font", "large",  "init", "editor-load-init",
  "text", {"caption", "Select map", "align", "center"},
  "menu", "menu-editor-load")
DefineMenuItem("pos", { (352 - 18 - 288) / 2, 11 + 98}, "font", "game",  "init", "editor-load-lb-init", "exit", "editor-load-lb-exit",
  "listbox", {"size", {288, 108},
    "style", "pulldown",
    "func", "editor-load-lb-action",
    "retopt", "editor-load-lb-retrieve",
    "handler", "editor-load-ok",
    "nlines", 6},
  "menu", "menu-editor-load")
DefineMenuItem("pos", { ((352 - 18 - 288) / 2) + 288, 11 + 98}, "font", "small",
  "vslider", {"size", {18, 108},
    "func", "editor-load-vs-action",
    "handler", "editor-load-ok"},
  "menu", "menu-editor-load")
DefineMenuItem("pos", { 48, 308}, "font", "large",
  "button", {
    "caption", "OK",
    "hotkey", "",
    "func", "editor-load-ok",
    "style", "gm-half"},
  "menu", "menu-editor-load")
DefineMenuItem("pos", { 198, 308}, "font", "large",
  "button", {
    "caption", "Cancel",
    "hotkey", "",
    "func", "editor-load-cancel",
    "style", "gm-half"},
  "menu", "menu-editor-load")
DefineMenuItem("pos", { ((352 - 18 - 288) / 2) - 2, (11 + 98) - 28}, "font", "game",
  "button", {
    "caption", nil,
    "hotkey", "",
    "func", "editor-load-folder",
    "style", "folder"},
  "menu", "menu-editor-load")


--
-- menu-editor
--
DefineMenu("name", "menu-editor", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 6)
DefineMenuItem("pos", { 128, 11}, "font", "large",
  "text", {"caption", "Editor Menu", "align", "center"},
  "menu", "menu-editor")
DefineMenuItem("pos", { 16, 40}, "font", "large",
  "button", {
    "caption", "Save (~<F11~>)",
    "hotkey", "f11",
    "func", "editor-save-menu",
    "style", "gm-half"},
  "menu", "menu-editor")
DefineMenuItem("pos", { 16 + 12 + 106, 40}, "font", "large",
  "button", {
    "caption", "Load (~<F12~>)",
    "hotkey", "f12",
    "func", "editor-load-menu",
    "style", "gm-half"},
  "menu", "menu-editor")
DefineMenuItem("pos", { 16, 40 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "Map Properties (~<F5~>)",
    "hotkey", "f5",
    "func", "editor-map-properties-menu",
    "style", "gm-full"},
  "menu", "menu-editor")
DefineMenuItem("pos", { 16, 40 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "Player Properties (~<F6~>)",
    "hotkey", "f6",
    "func", "editor-player-properties-menu",
    "style", "gm-full"},
  "menu", "menu-editor")
DefineMenuItem("pos", { 16, 288 - 40 - 36}, "font", "large",
  "button", {
    "caption", "E~!xit to Menu",
    "hotkey", "x",
    "func", "editor-quit-to-menu",
    "style", "gm-full"},
  "menu", "menu-editor")
DefineMenuItem("pos", { 16, 288 - 40}, "font", "large",
  "button", {
    "caption", "Return to Editor (~<Esc~>)",
    "hotkey", "esc",
    "func", "game-menu-return",
    "style", "gm-full"},
  "menu", "menu-editor")


--
-- menu-editor-map-properties
--
DefineMenu("name", "menu-editor-map-properties", "geometry", {256, 112, 288, 256},
  "panel", "panel2", "default", 10)
DefineMenuItem("pos", { 288 / 2, 11}, "font", "large",
  "text", {"caption", "Map Properties", "align", "center"},
  "menu", "menu-editor-map-properties")
DefineMenuItem("pos", { (288 - 260) / 2, 11 + 36}, "font", "game",
  "text", {"caption", "Map Description:", "align", "left"},
  "menu", "menu-editor-map-properties")
DefineMenuItem("pos", { (288 - 260) / 2, 11 + 36 + 22}, "font", "game",
  "input", {"size", {260, 20},
    "func", "editor-map-properties-enter-action",
    "style", "pulldown"},
  "menu", "menu-editor-map-properties")
DefineMenuItem("pos", { (288 - 260) / 2, 11 + (36 * 2) + 22}, "font", "game",
  "text", {"caption", "Size:", "align", "left"},
  "menu", "menu-editor-map-properties")
DefineMenuItem("pos", { 288 - ((288 - 260) / 2) - 152, 11 + (36 * 2) + 22}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-editor-map-properties")
DefineMenuItem("pos", { (288 - 260) / 2, 11 + (36 * 3) + 22}, "font", "game",
  "text", {"caption", "Tileset:", "align", "left"},
  "menu", "menu-editor-map-properties")
DefineMenuItem("pos", { 288 - ((288 - 260) / 2) - 152, 11 + (36 * 3) + 22}, "font", "game",
  "pulldown",  {"size", {152, 20},
    "style", "pulldown",
    "func", nil},
  "menu", "menu-editor-map-properties")
DefineMenuItem("pos", { (288 - 260) / 2, 11 + (36 * 4) + 22}, "font", "game",
  "text", {"caption", "Version:", "align", "left"},
  "menu", "menu-editor-map-properties")
DefineMenuItem("pos", { 288 - ((288 - 260) / 2) - 152, 11 + (36 * 4) + 22}, "font", "game",
  "pulldown",  {"size", {152, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Original", "Expansion", "Stratagus" },
    "default", 0,
    "current", 0},
  "menu", "menu-editor-map-properties")
DefineMenuItem("pos", { (288 - (106 * 2)) / 4, 256 - 11 - 27}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "editor-map-properties-ok",
    "style", "gm-half"},
  "menu", "menu-editor-map-properties")
DefineMenuItem("pos", { 288 - ((288 - (106 * 2)) / 4) - 106, 256 - 11 - 27}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "editor-end-menu",
    "style", "gm-half"},
  "menu", "menu-editor-map-properties")


--
-- menu-editor-player-properties
--
DefineMenu("name", "menu-editor-player-properties", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 0)
DefineMenuItem("pos", { 0, 0}, "font", "game",
  "drawfunc", "editor-player-properties-draw-func",
  "menu", "menu-editor-player-properties")
DefineMenuItem("pos", { 640 / 2, 11}, "font", "large",
  "text", {"caption", "Player Properties", "align", "center"},
  "menu", "menu-editor-player-properties")
DefineMenuItem("pos", { 455, 440}, "font", "large",
  "button", {
    "caption", "OK",
    "hotkey", "o",
    "func", "editor-end-menu",
    "style", "gm-half"},
  "menu", "menu-editor-player-properties")
-- 3

DefineMenuItem("pos", { 12, 40 + (22 * 0)}, "font", "game",
  "text", {"caption", "#", "align", "left"},
  "menu", "menu-editor-player-properties")

for i=0,15 do
  DefineMenuItem("pos", { 12, 40 + (22 * (i+1))}, "font", "game",
    "text", {"caption", i, "align", "left"},
    "menu", "menu-editor-player-properties")
end

-- 20
DefineMenuItem("pos", { 40, 40 + (22 * 0)}, "font", "game",
  "text", {"caption", "Race", "align", "left"},
  "menu", "menu-editor-player-properties")
for i=0,14 do
  DefineMenuItem("pos", { 40, 40 + (22 * (i+1))}, "font", "game",
    "pulldown",  {"size", {80, 20},
      "style", "pulldown",
      "func", nil,
      "options", {"Elite", "Neutral" },
      "default", 0,
      "current", 0},
    "menu", "menu-editor-player-properties")
end
DefineMenuItem("pos", { 40, 40 + (22 * 16)}, "font", "game", "flags", {"disabled"},
  "pulldown",  {"size", {80, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Elite", "Neutral" },
    "default", 0,
    "current", 0},
  "menu", "menu-editor-player-properties")

-- 37
DefineMenuItem("pos", { 130, 40 + (22 * 0)}, "font", "game",
  "text", {"caption", "Type", "align", "left"},
  "menu", "menu-editor-player-properties")
for i=0,14 do
  DefineMenuItem("pos", { 130, 40 + (22 * (i+1))}, "font", "game",
    "pulldown",  {"size", {150, 20},
      "style", "pulldown",
      "func", nil,
      "options", {"Person", "Computer", "Rescue (Passive)", "Rescue (Active)", "Neutral", "Nobody "},
      "default", 0,
      "current", 0},
    "menu", "menu-editor-player-properties")
end
DefineMenuItem("pos", { 130, 40 + (22 * 16)}, "font", "game", "flags", {"disabled"},
  "pulldown",  {"size", {150, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Person", "Computer", "Rescue (Passive)", "Rescue (Active)", "Neutral", "Nobody "},
    "default", 0,
    "current", 0},
  "menu", "menu-editor-player-properties")

-- 54
DefineMenuItem("pos", { 290, 40 + (22 * 0)}, "font", "game",
  "text", {"caption", "AI", "align", "left"},
  "menu", "menu-editor-player-properties")
for i=0,14 do
  DefineMenuItem("pos", { 290, 40 + (22 * (i+1))}, "font", "game",
    "pulldown",  {"size", {120, 20},
      "style", "pulldown",
      "func", nil,
      "options", {"Land Attack", "Passive", "Sea Attack", "Air Attack" },
      "default", 0,
      "current", 0},
    "menu", "menu-editor-player-properties")
end
DefineMenuItem("pos", { 290, 40 + (22 * 16)}, "font", "game", "flags", {"disabled"},
  "pulldown",  {"size", {120, 20},
    "style", "pulldown",
    "func", nil,
    "options", {"Land Attack", "Passive", "Sea Attack", "Air Attack" },
    "default", 0,
    "current", 0},
  "menu", "menu-editor-player-properties")

-- 71
DefineMenuItem("pos", { 420, 40 + (22 * 0)}, "font", "game",
  "text", {"caption", "Titanium", "align", "left"},
  "menu", "menu-editor-player-properties")
for i=0,14 do
  DefineMenuItem("pos", { 420, 40 + (22 * (i+1))}, "font", "game",
    "input", {"size", {60, 20},
      "func", "editor-player-properties-enter-action",
      "style", "pulldown"},
    "menu", "menu-editor-player-properties")
end
DefineMenuItem("pos", { 420, 40 + (22 * 16)}, "font", "game", "flags", {"disabled"},
  "input", {"size", {60, 20},
    "func", "editor-player-properties-enter-action",
    "style", "pulldown"},
  "menu", "menu-editor-player-properties")

-- 88
DefineMenuItem("pos", { 490, 40 + (22 * 0)}, "font", "game",
  "text", {"caption", "Crystal", "align", "left"},
  "menu", "menu-editor-player-properties")
for i=0,14 do
  DefineMenuItem("pos", { 490, 40 + (22 * (i+1))}, "font", "game",
    "input", {"size", {60, 20},
      "func", "editor-player-properties-enter-action",
      "style", "pulldown"},
    "menu", "menu-editor-player-properties")
end
DefineMenuItem("pos", { 490, 40 + (22 * 16)}, "font", "game", "flags", {"disabled"},
  "input", {"size", {60, 20},
    "func", "editor-player-properties-enter-action",
    "style", "pulldown"},
  "menu", "menu-editor-player-properties")

-- 105
DefineMenuItem("pos", { 560, 40 + (22 * 0)}, "font", "game",
  "text", {"caption", "Quality", "align", "left"},
  "menu", "menu-editor-player-properties")
for i=0,14 do
  DefineMenuItem("pos", { 560, 40 + (22 * (i+1))}, "font", "game",
    "input", {"size", {60, 20},
      "func", "editor-player-properties-enter-action",
      "style", "pulldown"},
    "menu", "menu-editor-player-properties")
end
DefineMenuItem("pos", { 560, 40 + (22 * 16)}, "font", "game", "flags", {"disabled"},
  "input", {"size", {60, 20},
    "func", "editor-player-properties-enter-action",
    "style", "pulldown"},
  "menu", "menu-editor-player-properties")

--
-- menu-editor-tips
--
DefineMenu("name", "menu-editor-tips", "geometry", {256, 112, 288, 256},
  "panel", "panel2", "default", 4)
DefineMenuItem("pos", { 144, 11}, "font", "large",  --"init", "init-editor-tips"
  "text", {"caption", "Editor Tips", "align", "center"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 14, (256 - 75)}, "font", "game",
  "checkbox", {
    "state", "checked",
    "func", nil, --set-editor-tips
    "style", "square"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 14 + 22, (256 - 75) + 4}, "font", "game",
  "text", {"caption", "Show tips at startup", "align", "left"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 14, 256 - 40}, "font", "large",
  "button", {
    "caption", "~!Next Tip",
    "hotkey", "n",
    "func", nil, --show-next-editor-tip
    "style", "gm-half"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 168, 256 - 40}, "font", "large",
  "button", {
    "caption", "~!Close",
    "hotkey", "c",
    "func", "editor-end-menu",
    "style", "gm-half"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 0)}, "font", "game",
  "text", {"caption", "Warning:", "align", "left"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 1)}, "font", "game",
  "text", {"caption", "This is the first relase of the editor!", "align", "left"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 2)}, "font", "game",
  "text", {"caption", "Please expect bugs and please report", "align", "left"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 3)}, "font", "game",
  "text", {"caption", "them!", "align", "left"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 4)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 5)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 6)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-editor-tips")
DefineMenuItem("pos", { 14, 35 + (16 * 7)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-editor-tips")


--
-- menu-editor-edit-resource
--
DefineMenu("name", "menu-editor-edit-resource", "geometry", {256, 176, 288, 128},
  "panel", "panel4", "default", 1)
DefineMenuItem("pos", { 288 / 2, 11}, "font", "large",
  "text", {"caption", nil, "align", "center"},
  "menu", "menu-editor-edit-resource")
DefineMenuItem("pos", { 40, 46}, "font", "game",
  "input", {"size", {212, 20},
    "func", "editor-edit-resource-enter-action",
    "style", "pulldown"},
  "menu", "menu-editor-edit-resource")
DefineMenuItem("pos", { 24, 88}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "editor-edit-resource-ok",
    "style", "gm-half"},
  "menu", "menu-editor-edit-resource")
DefineMenuItem("pos", { 154, 88}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "editor-edit-resource-cancel",
    "style", "gm-half"},
  "menu", "menu-editor-edit-resource")


--
-- menu-editor-error
--
DefineMenu("name", "menu-editor-error", "geometry", {256, 176, 288, 128},
  "panel", "panel4", "default", 2)
DefineMenuItem("pos", { 144, 11}, "font", "large",
  "text", {"caption", "Error:", "align", "center"},
  "menu", "menu-editor-error")
DefineMenuItem("pos", { 144, 38}, "font", "large",
  "text", {"caption", nil, "align", "center"},
  "menu", "menu-editor-error")
DefineMenuItem("pos", { 92, 80}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "editor-end-menu",
    "style", "gm-half"},
  "menu", "menu-editor-error")


--
-- menu-editor-edit-ai-properties
--
DefineMenu("name", "menu-editor-edit-ai-properties", "geometry", {256, 176, 288, 128},
  "panel", "panel4", "default", 6)
DefineMenuItem("pos", { 288 / 2, 11}, "font", "large",
  "text", {"caption", "Artificial Intelligence", "align", "center"},
  "menu", "menu-editor-edit-ai-properties")
DefineMenuItem("pos", { 100, 34},
  "checkbox", {
    "state", "unchecked",
    "func", "editor-edit-ai-properties-checkbox",
    "style", "round"},
  "menu", "menu-editor-edit-ai-properties")
DefineMenuItem("pos", { 124, 38}, "font", "game",
  "text", {"caption", "Active", "align", "left"},
  "menu", "menu-editor-edit-ai-properties")
DefineMenuItem("pos", { 100, 56},
  "checkbox", {
    "state", "unchecked",
    "func", "editor-edit-ai-properties-checkbox",
    "style", "round"},
  "menu", "menu-editor-edit-ai-properties")
DefineMenuItem("pos", { 124, 60}, "font", "game",
  "text", {"caption", "Passive", "align", "left"},
  "menu", "menu-editor-edit-ai-properties")
DefineMenuItem("pos", { 24, 88}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "editor-edit-ai-properties-ok",
    "style", "gm-half"},
  "menu", "menu-editor-edit-ai-properties")
DefineMenuItem("pos", { 154, 88}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "editor-edit-ai-properties-cancel",
    "style", "gm-half"},
  "menu", "menu-editor-edit-ai-properties")


--
-- menu-editor-save
--
DefineMenu("name", "menu-editor-save", "geometry", {224, 64, 352, 352},
  "panel", "panel5", "default", 5)
DefineMenuItem("pos", { 352 / 2, 11}, "font", "large",
  "text", {"caption", "Save map", "align", "center"},
  "menu", "menu-editor-save")
DefineMenuItem("pos", { (352 - 18 - 288) / 2, 11 + 98}, "font", "game",  "init", "editor-save-lb-init", "exit", "editor-save-lb-exit",
  "listbox", {"size", {288, 108},
    "style", "pulldown",
    "func", "editor-save-lb-action",
    "retopt", "editor-save-lb-retrieve",
    "handler", "editor-save-ok",
    "nlines", 6},
  "menu", "menu-editor-save")
DefineMenuItem("pos", { ((352 - 18 - 288) / 2) + 288, 11 + 98}, "font", "small",
  "vslider", {"size", {18, 108},
    "func", "editor-save-vs-action",
    "handler", "editor-save-ok"},
  "menu", "menu-editor-save")
DefineMenuItem("pos", { (352 - 18 - 288) / 2, 11 + 98 + 108 + 12}, "font", "game",
  "input", {"size", {288, 20},
    "func", "editor-save-enter-action",
    "style", "pulldown"},
  "menu", "menu-editor-save")
DefineMenuItem("pos", { 48, 308}, "font", "large",
  "button", {
    "caption", "OK",
    "hotkey", "o",
    "func", "editor-save-ok",
    "style", "gm-half"},
  "menu", "menu-editor-save")
DefineMenuItem("pos", { 198, 308}, "font", "large",
  "button", {
    "caption", "Cancel",
    "hotkey", "c",
    "func", "editor-save-cancel",
    "style", "gm-half"},
  "menu", "menu-editor-save")
DefineMenuItem("pos", { ((352 - 18 - 288) / 2) - 2, (11 + 98) - 28}, "font", "game",
  "button", {
    "caption", nil,
    "hotkey", "",
    "func", "editor-save-folder",
    "style", "folder"},
  "menu", "menu-editor-save")


--
-- menu-editor-save-confirm
--
DefineMenu("name", "menu-editor-save-confirm", "geometry", {256, 112, 288, 128},
  "panel", "panel4", "default", 1)
DefineMenuItem("pos", { 288 / 2, 11}, "font", "large",  "init", "editor-save-confirm-init",
  "text", {"caption", "Overwrite File", "align", "center"},
  "menu", "menu-editor-save-confirm")
DefineMenuItem("pos", { 16, 11 + (20 * 1.5)}, "font", "game",
  "text", {"caption", "Are you sure you want to overwrite", "align", "left"},
  "menu", "menu-editor-save-confirm")
DefineMenuItem("pos", { 16, 11 + (20 * 2.5)}, "font", "game",
  "text", {"caption", nil, "align", "left"},
  "menu", "menu-editor-save-confirm")
DefineMenuItem("pos", { 16, 128 - (27 * 1.5)}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "editor-save-confirm-ok",
    "style", "gm-half"},
  "menu", "menu-editor-save-confirm")
DefineMenuItem("pos", { 288 - 16 - 106, 128 - (27 * 1.5)}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "editor-save-confirm-cancel",
    "style", "gm-half"},
  "menu", "menu-editor-save-confirm")


--
-- menu-replay-game
--
DefineMenu("name", "menu-replay-game", "geometry", {144, 64, 352, 352},
  "panel", "panel5", "background", MenuBackground, "default", 4)
DefineMenuItem("pos", { 352 / 2, 11}, "font", "large",  "init", "replay-game-init",
  "text", {"caption", "Select game", "align", "center"},
  "menu", "menu-replay-game")
DefineMenuItem("pos", { (352 - 18 - 288) / 2, 11 + 98}, "font", "game",  "init", "replay-game-lb-init", "exit", "replay-game-lb-exit",
  "listbox", {"size", {288, 108},
    "style", "pulldown",
    "func", "replay-game-lb-action",
    "retopt", "replay-game-lb-retrieve",
    "handler", "replay-game-ok",
    "nlines", 6},
  "menu", "menu-replay-game")
DefineMenuItem("pos", { ((352 - 18 - 288) / 2) + 288, 11 + 98}, "font", "small",
  "vslider", {"size", {18, 108},
    "func", "replay-game-vs-action",
    "handler", "replay-game-ok"},
  "menu", "menu-replay-game")
DefineMenuItem("pos", { 48, 308}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "replay-game-ok",
    "style", "gm-half"},
  "menu", "menu-replay-game")
DefineMenuItem("pos", { 198, 308}, "font", "large",
  "button", {
    "caption", "~!Cancel",
    "hotkey", "c",
    "func", "replay-game-cancel",
    "style", "gm-half"},
  "menu", "menu-replay-game")
DefineMenuItem("pos", { ((352 - 18 - 288) / 2) - 2, (11 + 98) - 28}, "font", "game",
  "button", {
    "caption", nil,
    "hotkey", "",
    "func", "replay-game-folder",
    "style", "folder"},
  "menu", "menu-replay-game")
DefineMenuItem("pos", { 23, 264},
  "checkbox", {
    "text", "Reveal Map",
    "state", "unchecked",
    "func", "replay-game-disable-fog",
    "style", "square"},
  "menu", "menu-replay-game")


--
-- menu-restart-confirm
--
DefineMenu("name", "menu-restart-confirm", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 4)
DefineMenuItem("pos", { 128, 11}, "font", "large",
  "text", {"caption", "Are you sure you", "align", "center"},
  "menu", "menu-restart-confirm")
DefineMenuItem("pos", { 128, 11 + (24 * 1)}, "font", "large",
  "text", {"caption", "want to restart", "align", "center"},
  "menu", "menu-restart-confirm")
DefineMenuItem("pos", { 128, 11 + (24 * 2)}, "font", "large",
  "text", {"caption", "the scenario?", "align", "center"},
  "menu", "menu-restart-confirm")
DefineMenuItem("pos", { 16, 11 + (24 * 3) + 29}, "font", "large",
  "button", {
    "caption", "~!Restart Scenario",
    "hotkey", "r",
    "func", "end-scenario-restart",
    "style", "gm-full"},
  "menu", "menu-restart-confirm")
DefineMenuItem("pos", { 16, 288 - 40}, "font", "large",
  "button", {
    "caption", "Cancel (~<Esc~>)",
    "hotkey", "esc",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-restart-confirm")


--
-- menu-surrender-confirm
--
DefineMenu("name", "menu-surrender-confirm", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 4)
DefineMenuItem("pos", { 128, 11}, "font", "large",
  "text", {"caption", "Are you sure you", "align", "center"},
  "menu", "menu-surrender-confirm")
DefineMenuItem("pos", { 128, 11 + (24 * 1)}, "font", "large",
  "text", {"caption", "want to surrender", "align", "center"},
  "menu", "menu-surrender-confirm")
DefineMenuItem("pos", { 128, 11 + (24 * 2)}, "font", "large",
  "text", {"caption", "to your enemies?", "align", "center"},
  "menu", "menu-surrender-confirm")
DefineMenuItem("pos", { 16, 11 + (24 * 3) + 29}, "font", "large",
  "button", {
    "caption", "~!Surrender",
    "hotkey", "s",
    "func", "end-scenario-surrender",
    "style", "gm-full"},
  "menu", "menu-surrender-confirm")
DefineMenuItem("pos", { 16, 288 - 40}, "font", "large",
  "button", {
    "caption", "Cancel (~<Esc~>)",
    "hotkey", "esc",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-surrender-confirm")


--
-- menu-quit-to-menu-confirm
--
DefineMenu("name", "menu-quit-to-menu-confirm", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 4)
DefineMenuItem("pos", { 128, 11}, "font", "large",
  "text", {"caption", "Are you sure you", "align", "center"},
  "menu", "menu-quit-to-menu-confirm")
DefineMenuItem("pos", { 128, 11 + (24 * 1)}, "font", "large",
  "text", {"caption", "want to quit to", "align", "center"},
  "menu", "menu-quit-to-menu-confirm")
DefineMenuItem("pos", { 128, 11 + (24 * 2)}, "font", "large",
  "text", {"caption", "the main menu?", "align", "center"},
  "menu", "menu-quit-to-menu-confirm")
DefineMenuItem("pos", { 16, 11 + (24 * 3) + 29}, "font", "large",
  "button", {
    "caption", "~!Quit to Menu",
    "hotkey", "q",
    "func", "end-scenario-quit-to-menu",
    "style", "gm-full"},
  "menu", "menu-quit-to-menu-confirm")
DefineMenuItem("pos", { 16, 288 - 40}, "font", "large",
  "button", {
    "caption", "Cancel (~<Esc~>)",
    "hotkey", "esc",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-quit-to-menu-confirm")


--
-- menu-exit-confirm
--
DefineMenu("name", "menu-exit-confirm", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 4)
DefineMenuItem("pos", { 128, 11}, "font", "large",
  "text", {"caption", "Are you sure you", "align", "center"},
  "menu", "menu-exit-confirm")
DefineMenuItem("pos", { 128, 11 + (24 * 1)}, "font", "large",
  "text", {"caption", "want to exit", "align", "center"},
  "menu", "menu-exit-confirm")
DefineMenuItem("pos", { 128, 11 + (24 * 2)}, "font", "large",
  "text", {"caption", "Stratagus?", "align", "center"},
  "menu", "menu-exit-confirm")
DefineMenuItem("pos", { 16, 11 + (24 * 3) + 29}, "font", "large",
  "button", {
    "caption", "E~!xit Program",
    "hotkey", "x",
    "func", "game-menu-exit",
    "style", "gm-full"},
  "menu", "menu-exit-confirm")
DefineMenuItem("pos", { 16, 288 - 40}, "font", "large",
  "button", {
    "caption", "Cancel (~<Esc~>)",
    "hotkey", "esc",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-exit-confirm")

-------------------------------------------------------------------
-- **** Show master server game list
-------------------------------------------------------------------
-- metaserver-list
--
DefineMenu("name", "metaserver-list", "geometry", {0, 0, 640, 480},
  "panel", "none", "default", 3)
DefineMenuItem("pos", { 0, 0}, "font", "game", "init", "metaserver-list-init", "exit", "metaserver-list-exit",
   "menu", "metaserver-list")
DefineMenuItem("pos", { (640 / 2) + 12, 8}, "font", "large",
  "text", {"caption", "~<Online Game List ~>", "align", "center"},
  "menu", "metaserver-list")
DefineMenuItem("pos", { 640 - 224 - 16, 360 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "~!Cancel Game",
    "hotkey", "c",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "metaserver-list")
DefineMenuItem("pos", { 640 - 224 - 16, 360 + (36 * 1)}, "font", "large", 
  "button", {
    "caption", "~!Refresh",
    "hotkey", "r",
    "func", "metaserver-list",	-- refresh game list.
    "style", "gm-full"},
  "menu", "metaserver-list")

--------------------------------------------------- nickname
DefineMenuItem("pos", { 30, 32 + (22 * 1)}, "font", "game",
  "text", {"caption", "~<Nickname~>" },
  "menu", "metaserver-list")
--------------------------------------------------- IP:Port
DefineMenuItem("pos", { 100, 32 + (22 * 1)}, "font", "game",
  "text", {"caption", "~<IP:PORT~>" },
  "menu", "metaserver-list")
--------------------------------------------------- Map
DefineMenuItem("pos", { 250, 32 + (22 * 1)}, "font", "game",
  "text", {"caption", "~<Map~>" },
  "menu", "metaserver-list")
--------------------------------------------------- Players
DefineMenuItem("pos", { 350, 32 + (22 * 1)}, "font", "game",
  "text", {"caption", "~<Players~>" },
  "menu", "metaserver-list")
--------------------------------------------------- FreeSpots
DefineMenuItem("pos", { 450, 32 + (22 * 1)}, "font", "game",
  "text", {"caption", "~<FreeSpots~>" },
  "menu", "metaserver-list")
--------------------------------------------------- checkbox
DefineMenuItem("pos", { 5, 32 + (22 * 1)},
  "checkbox", {
    "state", "unchecked",
    "func", "select-game-server",
    "style", "square"},
  "menu", "metaserver-list")

--------------------------------------------------- nickname
DefineMenuItem("pos", { 30, 32 + (22 * 2)}, "font", "game",
  "text", {"caption", "~<Nickname~>" },
  "menu", "metaserver-list")
--------------------------------------------------- IP:Port
DefineMenuItem("pos", { 100, 32 + (22 * 2)}, "font", "game",
  "text", {"caption", "~<IP:PORT~>" },
  "menu", "metaserver-list")
--------------------------------------------------- Map
DefineMenuItem("pos", { 250, 32 + (22 * 2)}, "font", "game",
  "text", {"caption", "~<Map~>" },
  "menu", "metaserver-list")
--------------------------------------------------- Players
DefineMenuItem("pos", { 350, 32 + (22 * 2)}, "font", "game",
  "text", {"caption", "~<Players~>" },
  "menu", "metaserver-list")
--------------------------------------------------- FreeSpots
DefineMenuItem("pos", { 450, 32 + (22 * 2)}, "font", "game",
  "text", {"caption", "~<FreeSpots~>" },
  "menu", "metaserver-list")
--------------------------------------------------- checkbox
DefineMenuItem("pos", { 5, 32 + (22 * 2)},
  "checkbox", {
    "state", "unchecked",
    "func", "select-game-server",
    "style", "square"},
  "menu", "metaserver-list")
--------------------------------------------------- nickname
DefineMenuItem("pos", { 30, 32 + (22 * 3)}, "font", "game",
  "text", {"caption", "~<Nickname~>" },
  "menu", "metaserver-list")
--------------------------------------------------- IP:Port
DefineMenuItem("pos", { 100, 32 + (22 * 3)}, "font", "game",
  "text", {"caption", "~<IP:PORT~>" },
  "menu", "metaserver-list")
--------------------------------------------------- Map
DefineMenuItem("pos", { 250, 32 + (22 * 3)}, "font", "game",
  "text", {"caption", "~<Map~>" },
  "menu", "metaserver-list")
--------------------------------------------------- Players
DefineMenuItem("pos", { 350, 32 + (22 * 3)}, "font", "game",
  "text", {"caption", "~<Players~>" },
  "menu", "metaserver-list")
--------------------------------------------------- FreeSpots
DefineMenuItem("pos", { 450, 32 + (22 * 3)}, "font", "game",
  "text", {"caption", "~<FreeSpots~>" },
  "menu", "metaserver-list")
--------------------------------------------------- checkbox
DefineMenuItem("pos", { 5, 32 + (22 * 3)},
  "checkbox", {
    "state", "unchecked",
    "func", "select-game-server",
    "style", "square"},
  "menu", "metaserver-list")
--------------------------------------------------- nickname
DefineMenuItem("pos", { 30, 32 + (22 * 4)}, "font", "game",
  "text", {"caption", "~<Nickname~>" },
  "menu", "metaserver-list")
--------------------------------------------------- IP:Port
DefineMenuItem("pos", { 100, 32 + (22 * 4)}, "font", "game",
  "text", {"caption", "~<IP:PORT~>" },
  "menu", "metaserver-list")
--------------------------------------------------- Map
DefineMenuItem("pos", { 250, 32 + (22 * 4)}, "font", "game",
  "text", {"caption", "~<Map~>" },
  "menu", "metaserver-list")
--------------------------------------------------- Players
DefineMenuItem("pos", { 350, 32 + (22 * 4)}, "font", "game",
  "text", {"caption", "~<Players~>" },
  "menu", "metaserver-list")
--------------------------------------------------- FreeSpots
DefineMenuItem("pos", { 450, 32 + (22 * 4)}, "font", "game",
  "text", {"caption", "~<FreeSpots~>" },
  "menu", "metaserver-list")
--------------------------------------------------- checkbox
DefineMenuItem("pos", { 5, 32 + (22 * 4)},
  "checkbox", {
    "state", "unchecked",
    "func", "select-game-server",
    "style", "square"},
  "menu", "metaserver-list")

