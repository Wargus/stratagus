--       _________ __                 __                               
--      /   _____//  |_____________ _/  |______     ____  __ __  ______
--      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
--      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
--     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
--             \/                  \/          \//_____/            \/ 
--  ______________________                           ______________________
--                        T H E   W A R   B E G I N S
--         Stratagus - A free fantasy real time strategy game engine
--
--      ui.lua - Define the elites user interface
--
--      (c) Copyright 2001-2005 by Lutz Sammer, Jimmy Salmon, Crestez Leonard,
--                                 and François Beerten.
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
--      $Id$

DefineCursor({
	Name = "cursor-point",
	Race = "elites",
	File = "elites/ui/elites_claw.png",
	HotSpot = {1, 1}, 
	Size = {28, 32}})

DefineCursor({
	Name = "cursor-green-hair",
	Race = "elites",
	File = "general/green.png",
	HotSpot = {15, 15},
	Size = {32, 32}})

DefineCursor({
	Name = "cursor-yellow-hair",
	Race = "elites",
	File = "general/yellow.png",
	HotSpot = {15, 15},
	Size = {32, 32}})

DefineCursor({
	Name = "cursor-red-hair",
	Race = "elites",
	File = "general/red.png",
	HotSpot = {15, 15},
	Size = {32, 32}})

------------------------------------------------------------------------------;
--	* Race elites.
------------------------------------------------------------------------------;
function AppendElements(t, elements)
	for k,v in pairs(elements) do
		if(type(k)=="number") then 
			table.insert(t, v)
		else
			table.insert(t, k, v)
		end
	end
end

function AddFiller(ui, graphic, position)
	table.insert(ui, "filler")
	table.insert(ui, {File = graphic, Pos = position})
end

function DefineElitePanels(info_panel_x, info_panel_y) 
   local min_damage = Max(1, Div(ActiveUnitVar("PiercingDamage"), 2))
   local max_damage = Add(ActiveUnitVar("PiercingDamage"), ActiveUnitVar("BasicDamage"))
   local damage_bonus = Sub(ActiveUnitVar("PiercingDamage", "Value", "Type"),
                            ActiveUnitVar("PiercingDamage", "Value", "Initial"))

   DefinePanelContents(
-- Default presentation. ------------------------
  {
  Ident = "panel-general-contents"..info_panel_x,
  Pos = {info_panel_x, info_panel_y}, DefaultFont = "game",
  Contents = {
	{ Pos = {14, 56}, Condition = {ShowOpponent = false, HideNeutral = true},
		More = {"LifeBar", {Variable = "HitPoints", Height = 7, Width = 45}}
	},
	{ Pos = {38, 57}, Condition = {ShowOpponent = false, HideNeutral = true},
		More = {"FormattedText2", {
			Font = "small", Variable = "HitPoints", Format = "%d/%d",
			Component1 = "Value", Component2 = "Max", Centered = true}}
	},
	{ Pos = {114, 25}, More = {"Text", {ShowName = true}} }, -- FIXME:split for long name
	--{ Pos = {10, 158}, More = {"Text", {Variable = "Slot"}},
	--	Condition = {Slot = "only"} }, -- DEBUG ONLY.
-- Ressource Left
	{ Pos = {76, 86}, Condition = {ShowOpponent = false, GiveResource = "only"},
		More = {"FormattedText2", {Format = "%s: %d", Variable = "GiveResource",
					Component1 = "Name", Component2 = "Value", Centered = true}}
	}

  } },
-- Supply Building constructed.----------------
  {
  Ident = "panel-building-contents"..info_panel_x,
  Pos = {info_panel_x, info_panel_y}, DefaultFont = "game",
  Condition = {ShowOpponent = false, HideNeutral = true, Build = "false", Supply = "only", Training = "false", UpgradeTo = "false"},
-- FIXME more condition. not town hall.
  Contents = {
-- Food building
        -- { Pos = {16, 71}, More = {"Text", "Usage"} },
	{ Pos = {16, 86}, More = {"Text", {Text = "Supply : ", Variable = "Supply",
					Component = "Max"}} },
	{ Pos = {16, 102}, More = {"Text", {Text = "Demand : ", Variable = "Demand",
					Component = "Max"}} },
	{ Pos = {16, 102}, More = {"FormattedText", {Format = "Demand : ~<%d~>",
					Variable = "Demand", Component = "Max"}}
-- FIXME COLOR when Demand >= Supply
    }

  } },
-- All own unit -----------------
  {
  Ident = "panel-all-unit-contents"..info_panel_x,
  Pos = {info_panel_x, info_panel_y},
  DefaultFont = "game",
  Condition = {ShowOpponent = false, HideNeutral = true, Build = "false"},
  Contents = {
         { Pos = {16, 97}, Condition = {BasicDamage = "only"},
            More = {"Text", {Text = Concat("Damage: ", String(min_damage), 
                                           "-", String(max_damage),
                                           If(Equal(0, damage_bonus), "",
                                              InverseVideo(Concat("+", String(damage_bonus)))) 
                                          )}}
        },
	{ Pos = {16, 111}, Condition = {AttackRange = "only"},
		More = {"Text", {
			 Text = "Range: ", Variable = "AttackRange" , Stat = true}}
	},
-- Construction
	{ Pos = {12, 153}, Condition = {Build = "only"},
		More = {"CompleteBar", {Variable = "Build", Width = 152, Height = 12}}
	},
	{ Pos = {50, 154}, Condition = {Build = "only"}, More = {"Text", "% Complete"}},
	{ Pos = {107, 78}, Condition = {Build = "only"}, More = {"Icon", {Unit = "Worker"}}},
-- Research
	{ Pos = {14, 141}, Condition = {Research = "only"},
		More = {"CompleteBar", {Variable = "Research", Width = 136, Height = 12}}
	},
	{ Pos = {64, 106}, Condition = {Research = "only"}, More = {"Text", "Researching"}},
	{ Pos = {44, 141}, Condition = {Research = "only"}, More = {"Text", "% Complete"}},
-- Training
	{ Pos = {14, 141}, Condition = {Training = "only"},
		More = {"CompleteBar", {Variable = "Training", Width = 136, Height = 12}}
	},
	{ Pos = {44, 141}, Condition = {Training = "only"}, More = {"Text", "% Complete"}},
-- Upgrading To
	{ Pos = {14, 141}, Condition = {UpgradeTo = "only"},
		More = {"CompleteBar", {Variable = "UpgradeTo", Width = 136, Height = 12}}
	},
	{ Pos = {37,  86}, More = {"Text", "Upgrading:"}, Condition = {UpgradeTo = "only"} },
	{ Pos = {44, 141}, More = {"Text", "% Complete"}, Condition = {UpgradeTo = "only"} },
-- Mana
	{ Pos = {16, 141}, Condition = {Mana = "only"},
		More = {"CompleteBar", {Variable = "Mana", Height = 16, Width = 140, Border = true}}
	},
	{ Pos = {86, 141}, More = {"Text", {Variable = "Mana"}}, Condition = {Mana = "only"} },
-- Ressource Carry
	{ Pos = {16, 138}, Condition = {CarryResource = "only"},
		More = {"FormattedText2", {Format = "Carry: %d %s", Variable = "CarryResource",
				Component1 = "Value", Component2 = "Name"}}
	}

  } },
-- Attack Unit -----------------------------
  {
  Ident = "panel-attack-unit-contents"..info_panel_x,
  Pos = {info_panel_x, info_panel_y},
  DefaultFont = "game",
  Condition = {ShowOpponent = false, HideNeutral = true, Building = "false", Build = "false"},
  Contents = {
-- Unit caracteristics
	{ Pos = {114, 37},
		More = {"FormattedText", {Centered = true, 
			Variable = "Level", Format = "Level ~<%d~>"}}
	},
	{ Pos = {114, 52},
		More = {"FormattedText2", {Centered = true,
			Variable1 = "Xp", Variable2 = "Kill", Format = "XP:~<%d~> Kills:~<%d~>"}}
	},
	{ Pos = {16, 84}, Condition = {Armor = "only"},
		More = {"Text", {Text = "Armor: ", Variable = "Armor", Stat = true}}
	},
	{ Pos = {16, 125}, Condition = {SightRange = "only"},
		More = {"Text", {Text = "Sight: ", Variable = "SightRange", Stat = true}}
	},
	{ Pos = {16, 97}, Condition = {Speed = "only"},
		More = {"Text", {Text = "Speed: ", Variable = "Speed", Stat = true}}
	} } })
end


function DefineEliteScreen(screen_width, screen_height)
	local info_panel_x = screen_width - 200
	local info_panel_y = 160 - 8

	DefineElitePanels(info_panel_x, info_panel_y)
	local ui = {"elites", screen_width, screen_height,
		"normal-font-color", "light-blue",
		"reverse-font-color", "yellow"}
	-- no menu panel ?
	-- minimap
	AddFiller(ui, "elites/ui/ui_minimap.png", {screen_width - 200, 24-8})

	local ui2 = {
		"resources", {
			"titanium", {
				File = "elites/ui/ui_res_icons.png", Frame = 0,
				Pos = {67 +   0, 0}, 
				Size = {14, 14}, 
				TextPos = {85 +   0, 1}},
			"crystal", {
				File = "elites/ui/ui_res_icons.png", Frame = 1,
				Pos = {67 +  75, 0},
				Size = {14, 14},
				TextPos =  {85 +  75, 1}},
			"food", {
				File = "elites/ui/ui_res_icons.png", Frame = 3,
				Pos =  {67 + 150, 0}, 
				Size = {14, 14}, 
				TextPos = {85 + 150, 1}},
			"score", {
				File = "elites/ui/ui_res_icons.png", Frame = 4,
				Pos = {67 + 300, 0}, 
				Size = {14, 14}, 
				TextPos = {85 + 300, 1}}},
		"info-panel", {
			"panel", {
				"file", "elites/ui/ui_info.png",
				"pos", {info_panel_x, info_panel_y},
				"size", {200, 176}},
			"panels", {"panel-general-contents"..info_panel_x, 
				"panel-attack-unit-contents"..info_panel_x,
		                "panel-all-unit-contents"..info_panel_x, 
				"panel-building-contents"..info_panel_x},
			"selected", {
				"single", {
					"icon", {
					   "pos", {info_panel_x + 13, info_panel_y+16}, "style", "icon"}
				},
				"multiple", {
					"icons", {
						{"pos", {info_panel_x + 13, info_panel_y+16}, "style", "icon"},
						{"pos", {info_panel_x + 28, info_panel_y+16}, "style", "icon"},
						{"pos", {info_panel_x + 43, info_panel_y+16}, "style", "icon"},
						{"pos", {info_panel_x + 58, info_panel_y+16}, "style", "icon"},
						{"pos", {info_panel_x + 73, info_panel_y+16}, "style", "icon"},
						{"pos", {info_panel_x + 88, info_panel_y+16}, "style", "icon"},
						{"pos", {info_panel_x + 103, info_panel_y+16}, "style", "icon"},
						{"pos", {info_panel_x + 118, info_panel_y+16}, "style", "icon"},
						{"pos", {info_panel_x + 133, info_panel_y+16}, "style", "icon"}},
					"max-text", {
						"font", "game",
						"pos", {info_panel_x + 10, info_panel_y + 10}}}},
			"training", {
				"single", {
					"text", {"text", "", "font", "game", "pos", {screen_width - 187, 204}},
					"icon", {"pos", {info_panel_x + 13, info_panel_y + 91}, "style", "icon"}},
				"multiple", {
					"icons", {
						{"pos", {info_panel_x + 13, info_panel_y + 91}, "style", "icon"},
						{"pos", {info_panel_x + 28, info_panel_y + 91}, "style", "icon"},
						{"pos", {info_panel_x + 43, info_panel_y + 91}, "style", "icon"},
						{"pos", {info_panel_x + 58, info_panel_y + 91}, "style", "icon"},
						{"pos", {info_panel_x + 73, info_panel_y + 91}, "style", "icon"},
						{"pos", {info_panel_x + 88, info_panel_y + 91}, "style", "icon"}}}},
			"upgrading", {
				"icon", {"pos", {info_panel_x + 13, 243}, "style", "icon"}},
			"researching", {
				"icon", {"pos", {info_panel_x + 13, 243}, "style", "icon"}},
			"transporting", {"icons", {
					{"pos", {info_panel_x + 13, info_panel_y + 91}, "style", "icon"},
					{"pos", {info_panel_x + 28, info_panel_y + 91}, "style", "icon"},
					{"pos", {info_panel_x + 43, info_panel_y + 91}, "style", "icon"},
					{"pos", {info_panel_x + 58, info_panel_y + 91}, "style", "icon"},
					{"pos", {info_panel_x + 73, info_panel_y + 91}, "style", "icon"},
					{"pos", {info_panel_x + 88, info_panel_y + 91}, "style", "icon"}}},
			"completed-bar", {
				"color", {50, 50, 80}}},
		"button-panel", {
			"panel", {
				"file", "elites/ui/ui_" .. screen_width .. "_bpanel.png",
				"pos", {screen_width - 200, 336-8-8}},
			"icons", {
				{"pos", {screen_width - 177+4, 340-12}, "style", "icon"},
				{"pos", {screen_width - 122+4, 340-12}, "style", "icon"},
				{"pos", {screen_width - 67+4, 340-12},  "style", "icon"},
				{"pos", {screen_width - 177+4, 385-12}, "style", "icon"},
				{"pos", {screen_width - 122+4, 385-12}, "style", "icon"},
				{"pos", {screen_width - 67+4, 385-12},  "style", "icon"},
				{"pos", {screen_width - 177+4, 430-12}, "style", "icon"},
				{"pos", {screen_width - 122+4, 430-12}, "style", "icon"},
				{"pos", {screen_width - 67+4, 430-12},  "style", "icon"}},
		        "auto-cast-border-color", {0, 0, 252}},
		"piemenu", {
			"radius", 70,
			"file", "ui/rosace1.png",
			"mouse-button", "middle"},
		"map-area", {
			Pos = {0, 16},
			Size = {
				screen_width - 200,
				screen_height - 32}},
		"menu-panel", {
			"menu-button", {
				Pos = {screen_width - 200, 0},
				Caption = "Menu (~<F10~>)",
				Style = "black"},
			"network-menu-button", {
				Pos = {screen_width - 200, 0},
				Caption = "Menu", 
				Style = "network"},
			"network-diplomacy-button", {
				Pos = {screen_width - 100, 0},
				Caption = "Diplomacy",
				Style = "network"}},
		"minimap", {
                       Pos = {screen_width - 200 + 46, 24 + 17-8},
                       Size = {121, 105}},
		"status-line", {
			TextPos = {2 + 36, screen_height - 14},
			Font = "game",
			Width = screen_width - 200 - 100 },
		"cursors", {
  			 Point = "cursor-point",
     			 Glass = "cursor-glass",
      			 Cross = "cursor-cross",
		         Yellow = "cursor-yellow-hair",
		         Green = "cursor-green-hair",
     			 Red = "cursor-red-hair",
		         Scroll = "cursor-scroll",
		         ArrowE = "cursor-arrow-e",
                         ArrowNE = "cursor-arrow-ne",
                         ArrowN = "cursor-arrow-n",
                         ArrowNW = "cursor-arrow-nw",
                         ArrowW = "cursor-arrow-w",
                         ArrowSW = "cursor-arrow-sw",
                         ArrowS = "cursor-arrow-s",
                         ArrowSE = "cursor-arrow-se"},
		"menu-panels", {
			"panel1", "general/panel_1.png",
			"panel2", "general/panel_2.png",
			"panel3", "general/panel_3.png",
			"panel4", "general/panel_4.png",
			"panel5", "general/panel_5.png"},
		"victory-background", "screens/general.png",
		"defeat-background", "screens/general.png"
	}
	AppendElements(ui,ui2)
	DefineUI(unpack(ui))
end

DefineEliteScreen(640, 480)
DefineEliteScreen(800, 600)
DefineEliteScreen(1024, 768)
DefineEliteScreen(1280, 960)
DefineEliteScreen(1600, 1200)

