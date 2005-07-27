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
--	ingame.lua	-	In-game menus configuration
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
-- menu-game
--
DefineMenu("name", "menu-game", "geometry", {272, 96, 256, 288},
  "panel", "panel1", "default", 7, "init", "game-menu-init")
DefineMenuItem("pos", { 128, 11}, "font", "large",
  "text", {"caption", "Game Menu", "align", "center"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16, 40}, "font", "large",
  "button", {
    "caption", "Save (~<F11~>)",
    "hotkey", "f11",
    "func",  function() ProcessMenu("menu-save-game") end,
    "style", "gm-half"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16 + 12 + 106, 40}, "font", "large",
  "button", {
    "caption", "Load (~<F12~>)",
    "hotkey", "f12",
    "func", function() ProcessMenu("menu-load-game") end,
    "style", "gm-half"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16, 40 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "Options (~<F5~>)",
    "hotkey", "f5",
    "func", function() ProcessMenu("menu-game-options") end,
    "style", "gm-full"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16, 40 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "Help (~<F1~>)",
    "hotkey", "f1",
    "func", function() ProcessMenu("menu-keystroke-help") end,
    "style", "gm-full"},
  "menu", "menu-game")
DefineMenuItem("pos", { 16, 40 + (36 * 3)}, "font", "large",
  "button", {
    "caption", "~!Objectives",
    "hotkey", "o",
    "func", function() ProcessMenu("menu-objectives") end,
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
  "init", "victory-init",
  "panel", "panel4", "default", 2)
DefineMenuItem("pos", { 144, 11}, "font", "large", 
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
    "func", function() ProcessMenu("menu-save-replay") end,
    "style", "gm-full"},
  "menu", "menu-victory")


--
-- menu-defeated
--
DefineMenu("name", "menu-defeated", "geometry", {256, 176, 288, 128},
  "init", "defeated-init",
  "panel", "panel4", "default", 2)
DefineMenuItem("pos", { 144, 11}, "font", "large", 
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
  "init", "save-replay-init", "exit", "save-replay-exit",
  "panel", "panel4", "default", 1)
DefineMenuItem("pos", { 144, 11}, "font", "large", 
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
-- menu-objectives
--
DefineMenu("name", "menu-objectives", "geometry", {272, 96, 256, 288}, 
  "panel", "panel1", "default", 10)
DefineMenuItem("pos", { 128, 11}, "font", "large", 
  "text", {"caption", "Objectives", "align", "center"},
  "menu", "menu-objectives")
for i = 0, 8 do
 DefineMenuItem("pos", { 14, 38 + (21 * i)}, "font", "large",
  "text", {"caption", Line(1 + i, GameInfo("Objectives"), 228, "large")},
  "menu", "menu-objectives")
end
DefineMenuItem("pos", { 16, 288 - 40}, "font", "large",
  "button", {
    "caption", "~!OK",
    "hotkey", "o",
    "func", "end-menu",
    "style", "gm-full"},
  "menu", "menu-objectives")

--
-- menu-sound-options
--
DefineMenu("name", "menu-sound-options", "geometry", {224, 64, 352, 352},
  "init", "sound-options-init", "exit", "sound-options-exit",
  "panel", "panel5", "default", 23)
DefineMenuItem("pos", { 176, 11}, "font", "large", 
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
  "init", "preferences-init", "exit", "preferences-exit",
  "panel", "panel1", "default", 5)
DefineMenuItem("pos", { 128, 11}, "font", "large", 
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
  "init", "diplomacy-init", "exit", "diplomacy-exit",
  "panel", "panel5", "default", 5)
DefineMenuItem("pos", { 136, 30}, "font", "game", 
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
  "init", "speed-options-init", "exit", "speed-options-exit",
  "panel", "panel1", "default", 13)
DefineMenuItem("pos", { 128, 11}, "font", "large", 
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
  "panel", "panel1", "default", 5, 
  "init", "speed-options-init", "exit", "speed-options-exit")
DefineMenuItem("pos", { 128, 11}, "font", "large",
  "text", {"caption", "Game Options", "align", "center"},
  "menu", "menu-game-options")
DefineMenuItem("pos", { 16, 40 + (36 * 0)}, "font", "large",
  "button", {
    "caption", "Sound (~<F7~>)",
    "hotkey", "f7",
    "func", function() ProcessMenu("menu-sound-options") end,
    "style", "gm-full"},
  "menu", "menu-game-options")
DefineMenuItem("pos", { 16, 40 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "Speeds (~<F8~>)",
    "hotkey", "f8",
    "func", function() ProcessMenu("menu-speed-options") end,
    "style", "gm-full"},
  "menu", "menu-game-options")
DefineMenuItem("pos", { 16, 40 + (36 * 2)}, "font", "large",
  "button", {
    "caption", "Preferences (~<F9~>)",
    "hotkey", "f9",
    "func", function() ProcessMenu("menu-preferences") end,
    "style", "gm-full"},
  "menu", "menu-game-options")
DefineMenuItem("pos", { 16, 40 + (36 * 3)}, "font", "large",
  "button", {
    "caption", "~!Diplomacy",
    "hotkey", "d",
    "func", function() ProcessMenu("menu-diplomacy") end,
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
-- menu-tips
--
DefineMenu("name", "menu-tips", "geometry", {256, 112, 288, 256},
  "exit", "tips-exit",
  "panel", "panel2", "default", 4)
DefineMenuItem("pos", { 144, 11}, "font", "large", 
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
for i = 0,7 do
 DefineMenuItem("pos", { 14, 35 + (16 * i)}, "font", "game",
  "text", {"caption", Line(1 + i, GameInfo("Tips"), 260, "game"), "align", "left"},
  "menu", "menu-tips")
end


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
    "func", function() ProcessMenu("menu-keystroke-help") end,
    "style", "gm-full"},
  "menu", "menu-help")
DefineMenuItem("pos", { 16, 40 + (36 * 1)}, "font", "large",
  "button", {
    "caption", "Stratagus ~!Tips",
    "hotkey", "t",
    "func", function() ProcessMenu("menu-tips") end,
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
  "init", "save-game-init", "exit", "save-game-exit",
  "panel", "panel3", "default", 6)
DefineMenuItem("pos", { 384 / 2, 11}, "font", "large",  
  "text", {"caption", "Save Game", "align", "center"},
  "menu", "menu-save-game")
DefineMenuItem("pos", { (384 - 300 - 18) / 2, 11 + 36}, "font", "game",
  "input", {"size", {300, 20},
    "func", "save-game-enter-action",
    "style", "pulldown"},
  "menu", "menu-save-game")
DefineMenuItem("pos", { (384 - 300 - 18) / 2, 11 + 36 + 22}, "font", "game",
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
    "func", function() ProcessMenu("menu-delete-confirm") end,
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
  "init", "load-game-init", "exit", "load-game-exit",
  "panel", "panel3", "background", MenuBackground, "default", 4)
DefineMenuItem("pos", { 384 / 2, 11}, "font", "large", 
  "text", {"caption", "Load Game", "align", "center"},
  "menu", "menu-load-game")
DefineMenuItem("pos", { (384 - 300 - 18) / 2, 11 + (36 * 1.5)}, "font", "game",
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
  "init", "save-confirm-init", "exit", "save-confirm-exit",
  "panel", "panel4", "default", 1)
DefineMenuItem("pos", { 288 / 2, 11}, "font", "large",  
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
  "init", "delete-confirm-init", "exit", "delete-confirm-exit",
  "panel", "panel4", "default", 1)
DefineMenuItem("pos", { 288 / 2, 11}, "font", "large",  
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
