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
--	anim.ccl	-	The unit animation definitions.
--
--	(c) Copyright 2000-2004 by Josh Cogliati, Lutz Sammer, Crestez Leonard
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
--	$Id: anim.lua,v 1.6 2004/11/02 03:30:09 mr-russ Exp $

---------------------------
-------- Elite Units ------
---------------------------

DefineAnimations("animations-apcs",
	"still", {{3, 0, 1, 0}},
	"move", {
		{0, 4, 2, 0}, {0, 4, 2, 0}, {0, 4, 2, 0}, {0, 4, 2, 0},
		{0, 4, 2, 0}, {0, 4, 2, 0}, {0, 4, 2, 0}, {0, 4, 2, 0},
		{3, 0, 1, 0}},
	"attack", {
		{4, 0, 2, 5}, {0, 0, 2, 0}, {0, 0, 2, 5}, {0, 0, 2, 0},
		{8, 0, 2, 5}, {0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0},
		{0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0},
		{0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0},
		{0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0}, {3, 0, 2, 0}},
	"die", {
		{0, 0, 5,  0}, {0, 0, 5,  0}, {0, 0, 5,  0}, {3, 0, 5,  0}})

DefineAnimations("animations-medic",
	"still", {{3, 0, 1, 0}},
	"move", {
		{0, 4, 2, 5}, {0, 4, 2, 5}, {0, 4, 2, 10}, {0, 4, 2, 10},
		{0, 4, 2, 15}, {0, 4, 2, 15}, {0, 4, 2, 20}, {0, 4, 2, 20},
		{3, 0, 1, 20}},
	"attack", {
		{0, 0, 4, 25}, {12, 0, 4, 30}, {3, 0, 1, 0}},
	"die", {
		{0, 0, 2, 40}, {0, 0, 2, 45}, {0, 0, 2, 50}, {3, 0, 2, 55}})

DefineAnimations("animations-bazoo",
	"still", {{3, 0, 1, 0}},
	"move", {
		{0, 2, 2, 5}, {0, 2, 1, 5}, {0, 2, 2, 5}, {0, 2, 1, 5},
		{0, 2, 2, 10}, {0, 2, 1, 10}, {0, 2, 2, 10}, {0, 2, 1, 10},
		{0, 2, 2, 15}, {0, 2, 1, 15}, {0, 2, 2, 15}, {0, 2, 1, 15},
		{0, 2, 2, 20}, {0, 2, 1, 20}, {0, 2, 2, 20}, {0, 2, 1, 20},
		{3, 0, 1, 20}},
	"attack", {
		{0, 0, 2, 25}, {12, 0, 2, 30}, {4, 0, 2, 35}, {0, 0, 150, 0},
		{3, 0, 1, 0}},
	"die", { 
		{0, 0, 2, 40}, {0, 0, 2, 45}, {0, 0, 2, 50}, {3, 0, 2, 55}})

DefineAnimations("animations-assault",
	"still", {{3, 0, 1, 0}},
	"move", {
		{0, 2, 1, 5}, {0, 2, 1, 5}, {0, 2, 1, 10}, {0, 2, 1, 10},
		{0, 2, 1, 15}, {0, 2, 1, 15}, {0, 2, 1, 20}, {0, 2, 1, 20},
		{0, 2, 1, 5}, {0, 2, 1, 5}, {0, 2, 1, 10}, {0, 2, 1, 10},
		{0, 2, 1, 15}, {0, 2, 1, 15}, {0, 2, 1, 20}, {0, 2, 1, 20},
		{3, 0, 1, 20}},
	"attack", {
		{4, 0, 1, 25}, {0, 0, 1, 0}, {0, 0, 1, 25}, {0, 0, 1, 0},
		{0, 0, 1, 25}, {0, 0, 1, 0}, {0, 0, 1, 25}, {0, 0, 1, 0},
		{0, 0, 1, 25}, {0, 0, 1, 0}, {8, 0, 1, 25}, {0, 0, 24, 0},
		{3, 0, 1, 0}},
	"die", {
		{0, 0, 5, 30}, {0, 0, 5, 35}, {0, 0, 5, 40}, {3, 0, 5, 45}})

DefineAnimations("animations-grenadier",
	"still", {{3, 0, 1, 0}},
	"move", {
		{0, 2, 1, 5}, {0, 2, 1, 5}, {0, 2, 1, 10}, {0, 2, 1, 10},
		{0, 2, 1, 15}, {0, 2, 1, 15}, {0, 2, 1, 20}, {0, 2, 1, 20},
		{0, 2, 1, 5}, {0, 2, 1, 5}, {0, 2, 1, 10}, {0, 2, 1, 10},
		{0, 2, 1, 15}, {0, 2, 1, 15}, {0, 2, 1, 20}, {0, 2, 1, 20},
		{3, 0, 1, 20}},
	"attack", {
		{0, 0, 3, 25}, {0, 0, 3, 30}, {12, 0, 3, 35}, {0, 0, 3, 40},
		{0, 0, 150, 0}, {3, 0, 1, 0}},
	"die", {
		{0, 0, 5, 30}, {0, 0, 5, 35}, {0, 0, 5, 40}, {3, 0, 5, 45}})

DefineAnimations("animations-engineer",
	"still", {{3, 0, 1, 0}},
	"move", {
		{0, 2, 2, 5}, {0, 2, 1, 5}, {0, 2, 2, 5}, {0, 2, 1, 5},
		{0, 2, 2, 10}, {0, 2, 1, 10}, {0, 2, 2, 10}, {0, 2, 1, 10},
		{0, 2, 2, 15}, {0, 2, 1, 15}, {0, 2, 2, 15}, {0, 2, 1, 15},
		{0, 2, 2, 20}, {0, 2, 1, 20}, {0, 2, 2, 20}, {0, 2, 1, 20},
		{3, 0, 1, 20}},
	"repair", {
		{0, 0, 8, 25}, {0, 0, 2, 30}, {0, 0, 2, 35}, {4, 0, 8, 40},
		{0, 0, 3, 35}, {3, 0, 3, 30}},
	"harvest", "titanium", {
		{0, 0, 8, 25}, {0, 0, 2, 30}, {0, 0, 2, 35}, {4, 0, 8, 40},
		{0, 0, 3, 35}, {3, 0, 3, 30}},
        "harvest", "crystal", {
                {0, 0, 8, 25}, {0, 0, 2, 30}, {0, 0, 2, 35}, {4, 0, 8, 40},
                {0, 0, 3, 35}, {3, 0, 3, 30}},
	"die", {
		{0, 0, 5, 45}, {0, 0, 5, 50}, {0, 0, 5, 55}, {3, 0, 5, 50}})

DefineAnimations("animations-harvester",
	"still", {{3, 0, 1, 0}},
	"move", {
		{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
		{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
		{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
		{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
		{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
		{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
		{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0},
		{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {3, 1, 1, 0}},
	"harvest", "crystal", {
		{4, 0, 6, 5}, {0, 0, 3, 10}, {0, 0, 3, 15}, {0, 0, 3, 20},
		{4, 0, 6, 25}, {0, 0, 3, 20}, {0, 0, 3, 15}, {3, 0, 3, 10}},
        "harvest", "titanium", {
                {4, 0, 6, 5}, {0, 0, 3, 10}, {0, 0, 3, 15}, {0, 0, 3, 20},
                {4, 0, 6, 25}, {0, 0, 3, 20}, {0, 0, 3, 15}, {3, 0, 3, 10}},
	"die", {
		{0, 0, 5, 0}, {0, 0, 5, 0}, {0, 0, 5, 0}, {3, 0, 5, 0}})

-------------------------------
-------- Elite Buildings ------
-------------------------------

DefineAnimations("animations-plate",
	"still", {{3, 0, 1, 0}})

DefineAnimations("animations-msilo",
	"still", {{3, 0, 1, 0}},
	"attack", {
		{0, 0, 1, 1}, {12, 0, 1, 2}, {0, 0, 1, 3}, {0, 0, 1, 4},
		{0, 0, 25, 5}, {12, 0, 25, 6}, {3, 0, 1, 6}})

DefineAnimations("animations-gen",
	"still", {
		{0, 0, 2, 0}, {0, 0, 2, 1}, {0, 0, 2, 2}, {0, 0, 2, 3},
		{0, 0, 2, 4}, {0, 0, 2, 5}, {0, 0, 2, 6}, {0, 0, 2, 7},
		{3, 0, 2, 8}})

DefineAnimations("animations-dev-yard",
	"still", {
		{0, 0, 20, 1}, {0, 0, 20, 2}, {0, 0, 20, 3}, {0, 0, 50, 4},
		{0, 0, 20, 5}, {3, 0, 50, 6}})

DefineAnimations("animations-rfac",
	"still", {{3, 0, 3, 0}})

DefineAnimations("animations-vault",
	"still", {
		{0, 0, 5, 0}, {0, 0, 5, 1}, {0, 0, 5, 2}, {0, 0, 5, 3},
		{0, 0, 5, 4}, {0, 0, 5, 5}, {0, 0, 5, 5}, {0, 0, 5, 5},
		{0, 0, 5, 4}, {0, 0, 5, 3}, {0, 0, 5, 2}, {0, 0, 5, 1},
		{0, 0, 5, 0}, {3, 0, 5, 0}},
	"die", {{3, 0, 3, 0}})

DefineAnimations("animations-camp",
	"still", {{3, 0, 3, 0}})

DefineAnimations("animations-hosp",
	"still", {
		{0, 0, 2, 0}, {0, 0, 2, 1}, {0, 0, 2, 2}, {3, 0, 2, 3}})

DefineAnimations("animations-vfac",
	"still", {
		{0, 0, 3, 0}, {0, 0, 3, 1}, {0, 0, 3, 2}, {0, 0, 3, 3},
		{0, 0, 3, 4}, {0, 0, 3, 5}, {0, 0, 3, 6}, {0, 0, 3, 7},
		{3, 0, 3, 8}}) 

DefineAnimations("animations-gturret",
	"still", {{3, 0, 1, 4}},
	"attack", {
		{0, 0, 1, 0}, {0, 0, 1, 5}, {0, 0, 1, 0}, {0, 0, 1, 5},{0, 0, 1, 0}, {12, 0, 1, 5},
		{3, 0, 1, 0}})

----------------------
-------- Nature ------
----------------------

DefineAnimations("animations-elitecorpse1",
	"die", {
		{0, 0, 2000, 20}, {0, 0, 200, 0}, {0, 0, 200, 5}, {0, 0, 200, 10},
		{0, 0, 200, 15}, {3, 0, 1, 15}})

DefineAnimations("animations-elitebuild1",
	"die", {
		{0, 0, 2000, 16}, {0, 0, 200, 16}, {0, 0, 200, 16}, {0, 0, 200, 17},
		{0, 0, 200, 17}, {3, 0, 1, 17}})

DefineAnimations("animations-elitebuild2",
	"die", {
		{0, 0, 2000, 19}, {0, 0, 200, 19}, {0, 0, 200, 19}, {0, 0, 200, 20},
		{0, 0, 200, 20}, {3, 0, 1, 20}})

DefineAnimations("animations-elitebuild3",
	"die", {
		{0, 0, 2000, 15}, {0, 0, 200, 15}, {0, 0, 200, 15}, {0, 0, 200, 16},
		{0, 0, 200, 16}, {3, 0, 1, 16}})

DefineAnimations("animations-elitebuild4",
	"die", {
		{0, 0, 2000, 12}, {0, 0, 200, 12}, {0, 0, 200, 12}, {0, 0, 200, 13},
		{0, 0, 200, 13}, {3, 0, 1, 13}})

DefineAnimations("animations-elitebuild5",
	"die", {
		{0, 0, 2000, 15}, {0, 0, 200, 15}, {0, 0, 200, 15}, {0, 0, 200, 16},
		{0, 0, 200, 16}, {3, 0, 1, 16}})

DefineAnimations("animations-elitebuild6",
	"die", {
		{0, 0, 2000, 30}, {0, 0, 200, 30}, {0, 0, 200, 30}, {0, 0, 200, 31},
		{0, 0, 200, 31}, {3, 0, 1, 31}})

DefineAnimations("animations-elitebuild7",
	"die", {
		{0, 0, 2000, 14}, {0, 0, 200, 14}, {0, 0, 200, 14}, {0, 0, 200, 15},
		{0, 0, 200, 15}, {3, 0, 1, 15}})

DefineAnimations("animations-crystal-field1",
	"still", {{0, 0, 4, 6}, {3, 0, 1, 6}})
DefineAnimations("animations-crystal-field2",
	"still", {{0, 0, 4, 10}, {3, 0, 1, 10}})
DefineAnimations("animations-crystal-field3",
	"still", {{0, 0, 4, 12}, {3, 0, 1, 12}})
DefineAnimations("animations-crystal-field4",
	"still", {{0, 0, 4, 4}, {3, 0, 1, 4}})
DefineAnimations("animations-crystal-field5",
	"still", {{0, 0, 4, 0}, {3, 0, 1, 0}})
DefineAnimations("animations-crystal-field6",
	"still", {{0, 0, 4, 1}, {3, 0, 1, 1}})
DefineAnimations("animations-crystal-field7",
	"still", {{0, 0, 4, 5}, {3, 0, 1, 5}})
DefineAnimations("animations-crystal-field8",
	"still", {{0, 0, 4, 7}, {3, 0, 1, 7}})
DefineAnimations("animations-crystal-field9",
	"still", {{0, 0, 4, 2}, {3, 0, 1, 2}})
DefineAnimations("animations-crystal-field10",
	"still", {{0, 0, 4, 3}, {3, 0, 1, 3}})
DefineAnimations("animations-crystal-field11",
	"still", {{0, 0, 4, 11}, {3, 0, 1, 11}})
DefineAnimations("animations-crystal-field12",
	"still", {{0, 0, 4, 8}, {3, 0, 1, 8}})
DefineAnimations("animations-crystal-field13",
	"still", {{0, 0, 4, 9}, {3, 0, 1, 9}})

----------------------------
-------- Terras Units ------
----------------------------

DefineAnimations("animations-dorcoz",
	"still", {{3, 0, 1, 0}},
	"move", {
		{0, 4, 2, 5}, {0, 4, 2, 10}, {0, 4, 2, 15}, {0, 4, 2, 20},
		{0, 4, 2, 5}, {0, 4, 2, 10}, {0, 4, 2, 15}, {3, 4, 2, 20}},
	"attack", {
		{12, 0, 1, 25}, {3, 0, 49, 0}, {3, 0, 1, 0}},
	"die", {
		{0, 0, 5, 30}, {0, 0, 5, 35}, {0, 0, 5, 40}, {3, 0, 5, 45}})

-------------------
-------- OLD ------
-------------------

--[[
DefineAnimations("animations-grunt
"still", {	; #5
{0 0   4   0}, {3 0   1   0}}
"move", {	; #16 P32
{0 3   2   0}, {0 3   1   5}, {0 3   2   5}, {0 2   1  10}
{0 3   1  10}, {0 2   1   0}, {0 3   2   0}, {0 3   1  15}
{0 3   2  15}, {0 2   1  20}, {0 3   1  20}, {3 2   1   0}}
"attack", {	; #25
{0 0   3  25}, {0 0   3  30}, {0 0   3  35}, {12 0   5  40}
{0 0  10   0}, {3 0   1   0}}
"die", {	; #107
{0 0   3  45}, {0 0   3  50}, {0 0 100  55}, {3 0   1  55}}}

--------
--	Peasant, Peon
DefineAnimations("animations-peasant
"still", {	; #5
{ 0 0   4   0}, { 3 0   1   0}}
"move", {	; #16 P32
{ 0 3   2   0}, { 0 3   1   5}, { 0 3   2   5}, { 0 2   1  10}
{ 0 3   1  10}, { 0 2   1   0}, { 0 3   2   0}, { 0 3   1  15}
{ 0 3   2  15}, { 0 2   1  20}, { 0 3   1  20}, { 3 2   1   0}}
"attack", {	; #25
{ 0 0   3  25}, { 0 0   3  30}, { 0 0   3  35}, {12 0   5  40}
{ 0 0   3  45}, { 0 0   7  25}, { 3 0   1  25}}
"die", {	; #107
{ 0 0   3  50}, { 0 0   3  55}, { 0 0 100  60}, { 3 0   1  60}}}
]]--

---------------------------------------
-- Misc.
DefineAnimations("animations-building",
	"still", {{0, 0, 4, 0}, {3, 0, 1, 0}})

---------------------------------------
--	Dead Body
DefineAnimations("animations-dead-body",
	"die", {
		--	Corpse:		Orcish
		{0, 0, 200, 5}, {16, 0, 200, 10}, {0, 0, 200, 15}, {0, 0, 200, 20},
		{0, 0, 200, 25}, {3, 0, 1, 25},
		--	Corpse:		Human
		{0, 0, 200, 0}, {16, 0, 200, 10}, {0, 0, 200, 15}, {0, 0, 200, 20},
		{0, 0, 200, 25}, {3, 0, 1, 25},
		--	Corpse:		Ships
		{0, 0, 100, 30}, {16, 0, 100, 30}, {3, 0, 1, 30}})

---------------------------------------
--	Destroyed *x* Place
DefineAnimations("animations-destroyed-place",
	"die", {
		--	Destroyed land site
		{0, 0, 200, 0}, {16, 0, 200, 1}, {3, 0, 1, 1},
		--	Destroyed water site
		{0, 0, 200, 2}, {16, 0, 200, 3}, {3, 0, 1, 3}})
