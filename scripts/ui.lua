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
--      ui.lua - Define the widgets
--
--      (c) Copyright 2000 - 2004 by Lutz Sammer and Crestez Leonard.
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
--      $Id$

Load("scripts/widgets.lua")

Load("scripts/elites/ui.lua")

DefineCursor({
	Name = "cursor-glass",
	Race = "any",
    File = "general/magnifying_glass.png",
    HotSpot = {11, 11},
	Size = {34, 35}})

DefineCursor({
	Name = "cursor-cross",
	Race = "any",
    File = "general/small_green_cross.png",
    HotSpot = {8, 8},
	Size = {18, 18}})

DefineCursor({
	Name = "cursor-scroll",
	Race = "any",
    File = "general/cross.png",
	HotSpot = {15, 15},
	Size = {32, 32}})

DefineCursor({
	Name = "cursor-arrow-e",
	Race = "any",
    File = "general/arrow_E.png",
	HotSpot = {22, 10},
	Size = {32, 24}})

DefineCursor({
	Name = "cursor-arrow-ne",
	Race = "any",
    File = "general/arrow_NE.png",
	HotSpot = {20, 2},
	Size = {32, 24}})

DefineCursor({
	Name = "cursor-arrow-n",
	Race = "any",
    File = "general/arrow_N.png",
	HotSpot = {12, 2},
	Size = {32, 24}})

DefineCursor({
	Name = "cursor-arrow-nw",
	Race = "any",
    File = "general/arrow_NW.png",
	HotSpot = {2, 2},
	Size = {32, 24}})

DefineCursor({
	Name = "cursor-arrow-w",
	Race = "any",
    File = "general/arrow_W.png",
	HotSpot = {4, 10},
	Size = {32, 24}})

DefineCursor({
	Name = "cursor-arrow-s",
	Race = "any",
    File = "general/arrow_S.png",
	HotSpot = {12, 22},
	Size = {32, 24}})

DefineCursor({
	Name = "cursor-arrow-sw",
	Race = "any",
    File = "general/arrow_SW.png",
	HotSpot = {2, 18},
	Size = {32, 24}})

DefineCursor({
	Name = "cursor-arrow-se",
	Race = "any",
    File = "general/arrow_SE.png",
	HotSpot = {20, 18},
	Size = {32, 24}})

