--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	menus.lua	-	Menus configuration
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
-- load the keystroke helps
--
Load("scripts/keystrokes.lua")

MenuBackground = "screens/menu.png"

Load("scripts/menus/ingame.lua")
Load("scripts/menus/multiplayer.lua")
Load("scripts/menus/editor.lua")
--
-- define the menu graphics
--
DefineMenuGraphics({
  {"file", "general/ui_buttons.png", "size", {300, 144}},
  {"file", "general/ui_buttons.png", "size", {300, 144}}})



--
-- menu-program-start
--
DefineMenu("name", "menu-program-start", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 1, "init", "program-start")
DefineMenuItem("pos", { 0, 0}, "font", "game",
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
    "func", function() ProcessMenu("menu-load-game") end,
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
    "func", function() ProcessMenu("menu-global-options", 1) end,
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
-- menu-select-scenario
--
DefineMenu("name", "menu-select-scenario", "geometry", {144, 64, 352, 352},
  "init", "scen-select-init", "exit", "scen-select-exit",
  "panel", "panel5", "background", MenuBackground, "default", 4)
DefineMenuItem("pos", { 176, 8}, "font", "large",
  "text", {"caption", "Select map", "align", "center"},
  "menu", "menu-select-scenario")
DefineMenuItem("pos", { 24, 140}, "font", "game",
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
    "options", {"Stratagus map (smp)"},
    "default", 0,
    "current", 0},
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
-- menu-global-options
--
DefineMenu("name", "menu-global-options", "geometry", {144, 64, 352, 352},
  "panel", "panel5", "background", MenuBackground, "default", 7, 
  "init", "global-options-init", "exit", "global-options-exit")
DefineMenuItem("pos", { 176, 11}, "font", "large",
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

--[[
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
]]

DefineMenuItem("pos", { 123, 309}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "end-menu",
    "style", "gm-half"},
  "menu", "menu-global-options")

--
-- menu-custom-game (Single player)
--
DefineMenu("name", "menu-custom-game", "geometry", {0, 0, 640, 480},
  "panel", "none", "background", MenuBackground, "default", 3, "init", "game-setup-init")
DefineMenuItem("pos", { 0, 0}, "font", "game", 
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
    "options", {"Default"},
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
    "options", {"Default"},
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
    "func", function() ProcessMenu("menu-restart-confirm") end,
    "style", "gm-full"},
  "menu", "menu-end-scenario")
DefineMenuItem("pos", { 16, 40 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "~!Surrender",
    "hotkey", "s",
    "func", function() ProcessMenu("menu-surrender-confirm") end,
    "style", "gm-full"},
  "menu", "menu-end-scenario")
DefineMenuItem("pos", { 16, 40 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "~!Quit to Menu",
    "hotkey", "q",
    "func", function() ProcessMenu("menu-quit-to-menu-confirm") end,
    "style", "gm-full"},
  "menu", "menu-end-scenario")
DefineMenuItem("pos", { 16, 40 + (36 * 3)}, "font", "large",
  "button", {
    "caption", "E~!xit Program",
    "hotkey", "x",
    "func", function() ProcessMenu("menu-exit-confirm") end,
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
-- menu-replay-game
--
DefineMenu("name", "menu-replay-game", "geometry", {144, 64, 352, 352},
  "panel", "panel5", "background", MenuBackground, "default", 4, 
  "init", "replay-game-init", "exit", "replay-game-exit")
DefineMenuItem("pos", { 352 / 2, 11}, "font", "large",  
  "text", {"caption", "Select game", "align", "center"},
  "menu", "menu-replay-game")
DefineMenuItem("pos", { (352 - 18 - 288) / 2, 11 + 98}, "font", "game", 
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

