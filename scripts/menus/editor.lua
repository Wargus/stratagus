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
--	editor.lua	-	Editor menus configuration
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
--      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id$



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
  "init", "editor-main-load-init",
  "panel", "panel5", "background", MenuBackground, "default", 4)
DefineMenuItem("pos", { 352 / 2, 11}, "font", "large",  
  "text", {"caption", "Select map", "align", "center"},
  "menu", "menu-editor-main-load-map")
DefineMenuItem("pos", { (352 - 18 - 288) / 2, 11 + 98}, "font", "game",
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
  "init", "editor-load-init",
  "panel", "panel5", "default", 4)
DefineMenuItem("pos", { 352 / 2, 11}, "font", "large",  
  "text", {"caption", "Select map", "align", "center"},
  "menu", "menu-editor-load")
DefineMenuItem("pos", { (352 - 18 - 288) / 2, 11 + 98}, "font", "game",
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
    "text", {"caption", ""..i, "align", "left"},
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
DefineMenuItem("pos", { (352 - 18 - 288) / 2, 11 + 98}, "font", "game",
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
  "init", "editor-save-confirm-init",
  "panel", "panel4", "default", 1)
DefineMenuItem("pos", { 288 / 2, 11}, "font", "large",  
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

