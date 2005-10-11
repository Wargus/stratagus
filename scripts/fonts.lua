--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--	fonts.lua	-	Define the used fonts.
--
--	(c) Copyright 2000-2003 by Lutz Sammer, Jimmy Salmon and Crestez Leonard
--
--      This program is free software-- you can redistribute it and/or modify
--      it under the terms of the GNU General Public License as published by
--      the Free Software Foundation-- either version 2 of the License, or
--      (at your option) any later version.
--  
--      This program is distributed in the hope that it will be useful,
--      but WITHOUT ANY WARRANTY-- without even the implied warranty of
--      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--      GNU General Public License for more details.
--  
--      You should have received a copy of the GNU General Public License
--      along with this program-- if not, write to the Free Software
--      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
--
--	$Id$

CFont:New("small", CGraphic:New("general/small.png", 7, 6))
CFont:New("game", CGraphic:New("general/game.png", 14, 14))
CFont:New("large", CGraphic:New("general/large.png", 20, 20))
CFont:New("small-title", CGraphic:New("general/large.png", 20, 20))
CFont:New("large-title", CGraphic:New("general/large.png", 20, 20))

--	FIXME: only yellow, white, and grey are correct.

DefineFontColor("black",
  {    0,   0,   0,	-- 0
      40,  40,  60,	-- 228
      40,  40,  60,	-- 228
      40,  40,  60,	-- 228
      40,  40,  60,	-- 228
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("red",
  {    0,   0,   0,	-- 0
     164,   0,   0,	-- 208
     124,   0,   0,	-- 209
      92,   4,   0,	-- 210
      68,   4,   0,	-- 211
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("green",
  {    0,   0,   0,	-- 0
      44, 180, 148,	-- 216
      44, 180, 148,	-- 216
      44, 180, 148,	-- 216
      44, 180, 148,	-- 216
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("yellow",
  {  252, 248, 240,	-- 246
     244, 224,  32,	-- 200
     208, 192,  28,	-- 199
     168, 140,  16,	-- 197
      92,  48,   0,	-- 192
       0,   0,   0,	-- 239
     108, 108, 108})	-- 104
DefineFontColor("blue",
  {    0,   0,   0,	-- 0
       0, 148, 252,	-- 1
       0, 148, 252,	-- 1
       0, 148, 252,	-- 1
       0, 148, 252,	-- 1
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("magenta",
  {    0,   0,   0,	-- 0
     152,  72, 176,	-- 220
     152,  72, 176,	-- 220
     152,  72, 176,	-- 220
     152,  72, 176,	-- 220
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("cyan",
  {    0,   0,   0,	-- 0
     248, 140, 140,	-- 224
     248, 140, 140,	-- 224
     248, 140, 140,	-- 224
     248, 140, 140,	-- 224
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("white",
  {    0,   0,   0,	-- 0
     168, 168, 168,	-- 246
     252, 248, 240,	-- 246
     252, 248, 240,	-- 246
     108, 108, 108,	-- 104
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("grey",
  {    0,   0,   0,	-- 0
     192, 192, 192,	-- 111
     180, 180, 180,	-- 110
     168, 168, 168,	-- 109
     108, 108, 108,	-- 104
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("light-red",
  {    0,   0,   0,	-- 0
     255,   0,   0,	-- 208
     255,   0,   0,	-- 208
     255,   0,   0,	-- 208
     255,   0,   0,	-- 208
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("light-green",
  {    0,   0,   0,	-- 0
      44, 180, 44,	-- 216
      44, 180, 44,	-- 216
      44, 180, 44,	-- 216
      44, 180, 44,	-- 216
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("light-yellow",
  {  252, 248, 240,	-- 246
     244, 224,  32,	-- 200
     208, 192,  28,	-- 199
     168, 140,  16,	-- 197
      92,  48,   0,	-- 192
       0,   0,   0,	-- 239
     108, 108, 108})	-- 104
DefineFontColor("light-blue",
  {    0,   0,   0,	-- 0
       100, 158, 255,	-- 1
       100, 158, 255,	-- 1
       100, 158, 255,	-- 1
       100, 158, 255,	-- 1
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("light-magenta",
  {    0,   0,   0,	-- 0
     152,  72, 176,	-- 220
     152,  72, 176,	-- 220
     152,  72, 176,	-- 220
     152,  72, 176,	-- 220
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("light-cyan",
  {    0,   0,   0,	-- 0
     248, 140,  20,	-- 224
     248, 140,  20,	-- 224
     248, 140,  20,	-- 224
     248, 140,  20,	-- 224
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("light-grey",
  {    0,   0,   0,	-- 0
     192, 192, 192,	-- 111
     180, 180, 180,	-- 110
     168, 168, 168,	-- 109
     108, 108, 108,	-- 104
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0

DefineFontColor("violet",
  {    0,   0,   0,	-- 0
     152,  72, 176,	-- 220
     152,  72, 176,	-- 220
     152,  72, 176,	-- 220
     152,  72, 176,	-- 220
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0
DefineFontColor("orange",
  {    0,   0,   0,	-- 0
     248, 140,  20,	-- 224
     248, 140,  20,	-- 224
     248, 140,  20,	-- 224
     248, 140,  20,	-- 224
       0,   0,   0,	-- 239
       0,   0,   0})	-- 0

