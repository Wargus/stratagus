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
--	anim.lua	-	The unit animation definitions.
--
--	(c) Copyright 2000-2005 by Josh Cogliati, Lutz Sammer, Crestez Leonard
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
--        $Id$

---------------------------
-------- Elite Units ------
---------------------------

DefineNewAnimations("animations-apcs", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 0", "move 4", "wait 2", 
        "frame 0", "move 4", "wait 2", "frame 0", "move 4", "wait 2", 
        "frame 0", "move 4", "wait 2", "frame 0", "move 4", "wait 2", 
        "frame 0", "move 4", "wait 2", "frame 0", "move 4", "wait 2", 
        "frame 0", "move 4", "wait 2", "frame 0", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", "frame 5", "sound apcs-attack", "attack", "wait 2", 
        "frame 0", "wait 2", "frame 5", "wait 2", "frame 0", "wait 2", 
        "frame 5", "attack", "wait 2", "frame 0", "wait 2", 
        "frame 0", "wait 2", "frame 0", "wait 2", "frame 0", "wait 2", 
        "frame 0", "wait 2", "frame 0", "wait 2", "frame 0", "wait 2", 
        "frame 0", "wait 2", "frame 0", "wait 2", "frame 0", "wait 2", 
        "frame 0", "wait 2", "frame 0", "wait 2", "frame 0", "wait 2", 
        "frame 0", "wait 2", "frame 0", "unbreakable end", "wait 2", },
    Death = {"unbreakable begin", "frame 0", "wait 5", "frame 0", "wait 5", 
        "frame 0", "wait 5", "frame 0", "unbreakable end", "wait 5", },
    })

DefineNewAnimations("animations-medic", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 4", "wait 2", 
        "frame 5", "move 4", "wait 2", "frame 10", "move 4", "wait 2", 
        "frame 10", "move 4", "wait 2", "frame 15", "move 4", "wait 2", 
        "frame 15", "move 4", "wait 2", "frame 20", "move 4", "wait 2", 
        "frame 20", "move 4", "wait 2", "frame 20", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", "frame 25", "wait 4", 
        "frame 30", "sound medic-attack", "attack", "wait 4", 
        "frame 0", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 40", "wait 2", "frame 45", 
        "wait 2", "frame 50", "wait 2", "frame 55", "unbreakable end", "wait 2", },
    })

DefineNewAnimations("animations-bazoo", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 2", "wait 2", 
        "frame 5", "move 2", "wait 1", "frame 5", "move 2", "wait 2", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 2", 
        "frame 10", "move 2", "wait 1", "frame 10", "move 2", "wait 2", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 2", 
        "frame 15", "move 2", "wait 1", "frame 15", "move 2", "wait 2", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 2", 
        "frame 20", "move 2", "wait 1", "frame 20", "move 2", "wait 2", 
        "frame 20", "move 2", "wait 1", "frame 20", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", "frame 25", "wait 2", 
        "frame 30", "sound bazoo-attack", "attack", "wait 2", 
        "frame 35", "sound bazoo-attack", "wait 2", "frame 0", "wait 150", 
        "frame 0", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 40", "wait 2", "frame 45", "wait 2", 
        "frame 50", "wait 2", "frame 55", "unbreakable end", "wait 2", },
    })

DefineNewAnimations("animations-assault", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 2", "wait 1", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "wait 1", "frame 5", "move 2", "wait 1", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "wait 1", "frame 20", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", 
        "frame 25", "sound assault-attack", "attack", "wait 1", 
        "frame 0", "wait 1", "frame 25", "wait 1", "frame 0", "wait 1", 
        "frame 25", "wait 1", "frame 0", "wait 1", "frame 25", "wait 1", 
        "frame 0", "wait 1", "frame 25", "wait 1", "frame 0", "wait 1", 
        "frame 25", "attack", "wait 1", "frame 0", "wait 24", 
        "frame 0", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 30", "wait 5", "frame 35", "wait 5", 
        "frame 40", "wait 5", "frame 45", "unbreakable end", "wait 5", },
    })

DefineNewAnimations("animations-grenadier", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 2", "wait 1", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "wait 1", "frame 5", "move 2", "wait 1", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "wait 1", "frame 20", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", "frame 25", "wait 3", "frame 30", "wait 3", 
        "frame 35", "sound grenadier-attack", "attack", "wait 3", 
        "frame 40", "wait 3", "frame 0", "wait 150", 
        "frame 0", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 30", "wait 5", "frame 35", "wait 5", 
        "frame 40", "wait 5", "frame 45", "unbreakable end", "wait 5", },
    })

DefineNewAnimations("animations-engineer", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 2", "wait 2", 
        "frame 5", "move 2", "wait 1", "frame 5", "move 2", "wait 2", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 2", 
        "frame 10", "move 2", "wait 1", "frame 10", "move 2", "wait 2", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 2", 
        "frame 15", "move 2", "wait 1", "frame 15", "move 2", "wait 2", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 2", 
        "frame 20", "move 2", "wait 1", "frame 20", "move 2", "wait 2", 
        "frame 20", "move 2", "wait 1", "frame 20", "unbreakable end", "wait 1", },
    Repair = {"unbreakable begin", "frame 25", "wait 8", "frame 30", "wait 2",
        "frame 35", "wait 2", "frame 40", "sound engineer-repair", "wait 8",
        "frame 35", "wait 3", "frame 30", "wait 2", "unbreakable end", "wait 1", },
    Build = {"frame 25", "wait 8", "frame 30", "wait 2", "frame 35", "wait 2", 
        "frame 40", "sound engineer-repair", "wait 8", "frame 35", "wait 3", 
        "frame 30", "wait 3", },
    Harvest_titanium = {"frame 25", "wait 8", "frame 30", "wait 2", 
        "frame 35", "wait 2", "frame 40", "sound engineer-harvest", "wait 8", 
        "frame 35", "wait 3", "frame 30", "wait 3", },
    Harvest_crystal = {"frame 25", "wait 8", "frame 30", "wait 2", 
        "frame 35", "wait 2", "frame 40", "sound engineer-harvest", "wait 8", 
        "frame 35", "wait 3", "frame 30", "wait 3", },
    Death = {"unbreakable begin", "frame 45", "wait 5", "frame 50", "wait 5", 
        "frame 55", "wait 5", "frame 50", "unbreakable end", "wait 5", },
    })

DefineNewAnimations("animations-harvester", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "wait 1", "frame 0", "move 1", "wait 1", 
        "frame 0", "move 1", "unbreakable end", "wait 1", },
    Harvest_crystal = {"frame 5", "sound harvester-harvest", "wait 6", 
        "frame 10", "wait 3", "frame 15", "wait 3", "frame 20", "wait 3", 
        "frame 25", "sound harvester-harvest", "wait 6", "frame 20", "wait 3", 
        "frame 15", "wait 3", "frame 10", "wait 3", },
    Harvest_titanium = {"frame 5", "sound harvester-harvest", "wait 6", 
        "frame 10", "wait 3", "frame 15", "wait 3", "frame 20", "wait 3", 
        "frame 25", "sound harvester-harvest", "wait 6", "frame 20", "wait 3", 
        "frame 15", "wait 3", "frame 10", "wait 3", },
    Death = {"unbreakable begin", "frame 0", "wait 5", "frame 0", "wait 5", 
        "frame 0", "wait 5", "frame 0", "unbreakable end", "wait 5", },
        })

-------------------------------
-------- Elite Buildings ------
-------------------------------

DefineNewAnimations("animations-plate", {
    Still = {"frame 0", "wait 1", },
    })

DefineNewAnimations("animations-msilo", {
    Still = {"frame 0", "wait 1", },
    Attack = {"unbreakable begin", "frame 1", "wait 1", 
        "frame 2", "sound msilo-attack", "attack", "wait 1", 
        "frame 3", "wait 1", "frame 4", "wait 1", "frame 5", "wait 25", 
        "frame 6", "sound msilo-attack", "attack", "wait 25", 
        "frame 6", "unbreakable end", "wait 1", },
    })

DefineNewAnimations("animations-gen", {
    Still = {"frame 0", "wait 2", "frame 1", "wait 2", "frame 2", "wait 2", 
        "frame 3", "wait 2", "frame 4", "wait 2", "frame 5", "wait 2", 
        "frame 6", "wait 2", "frame 7", "wait 2", "frame 8", "wait 2", },
    })

DefineNewAnimations("animations-dev-yard", {
    Still = {"frame 1", "wait 20", "frame 2", "wait 20", "frame 3", "wait 20", 
        "frame 4", "wait 50", "frame 5", "wait 20", "frame 6", "wait 50", },
    })

DefineNewAnimations("animations-rfac", {
    Still = {"frame 0", "wait 3", },
    })

DefineNewAnimations("animations-vault", {
    Still = {"frame 0", "wait 5", "frame 1", "wait 5", "frame 2", "wait 5", 
        "frame 3", "wait 5", "frame 4", "wait 5", "frame 5", "wait 5", 
        "frame 5", "wait 5", "frame 5", "wait 5", "frame 4", "wait 5", 
        "frame 3", "wait 5", "frame 2", "wait 5", "frame 1", "wait 5", 
        "frame 0", "wait 5", "frame 0", "wait 5", },
    Death = {"unbreakable begin", "frame 0", "unbreakable end", "wait 3", },
    })

DefineNewAnimations("animations-camp", {
    Still = {"frame 0", "wait 3", },
    })

DefineNewAnimations("animations-hosp", {
    Still = {"frame 0", "wait 2", "frame 1", "wait 2", "frame 2", "wait 2", 
        "frame 3", "wait 2", },
    })

DefineNewAnimations("animations-vfac", {
    Still = {"frame 0", "wait 3", "frame 1", "wait 3", "frame 2", "wait 3", 
        "frame 3", "wait 3", "frame 4", "wait 3", "frame 5", "wait 3", 
        "frame 6", "wait 3", "frame 7", "wait 3", "frame 8", "wait 3", },
    })

----------------------
-------- Nature ------
----------------------

DefineNewAnimations("animations-elitecorpse1", {
    Death = {"unbreakable begin", "wait 1", "frame 20", "wait 2000", 
        "frame 0", "wait 200", "frame 5", "wait 200", "frame 10", "wait 200", 
        "frame 15", "wait 200", "frame 15", "wait 1", "unbreakable end", "wait 1", },
    }) 

DefineNewAnimations("animations-elitebuild1", {
    Death = {"unbreakable begin", "wait 1", "frame 16", "wait 2000", 
        "frame 16", "wait 200", "frame 16", "wait 200", "frame 17", "wait 200",
        "frame 17", "wait 200", "frame 17", "wait 1", "unbreakable end", "wait 1", },
    }) 

DefineNewAnimations("animations-elitebuild2", {
    Death = {"unbreakable begin", "wait 1", "frame 19", "wait 2000", 
        "frame 19", "wait 200", "frame 19", "wait 200", "frame 20", "wait 200",
        "frame 20", "wait 200", "frame 20", "wait 1", "unbreakable end", "wait 1", },
    })

DefineNewAnimations("animations-elitebuild3", {
    Death = {"unbreakable begin", "wait 1", "frame 15", "wait 2000", 
        "frame 15", "wait 200", "frame 15", "wait 200", "frame 16", "wait 200",
        "frame 16", "wait 200", "frame 16", "wait 1", "unbreakable end", "wait 1", },
    })

DefineNewAnimations("animations-elitebuild4", {
    Death = {"unbreakable begin", "wait 1", "frame 12", "wait 2000", 
        "frame 12", "wait 200", "frame 12", "wait 200", "frame 13", "wait 200",
        "frame 13", "wait 200", "frame 13", "wait 1", "unbreakable end", "wait 1", },
    })

DefineNewAnimations("animations-elitebuild5", {
    Death = {"unbreakable begin", "wait 1", "frame 15", "wait 2000", 
        "frame 15", "wait 200", "frame 15", "wait 200", "frame 16", "wait 200",
        "frame 16", "wait 200", "frame 16", "wait 1", "unbreakable end", "wait 1", },
    })

DefineNewAnimations("animations-elitebuild6", {
    Death = {"unbreakable begin", "wait 1", "frame 30", "wait 2000", 
        "frame 30", "wait 200", "frame 30", "wait 200", "frame 31", "wait 200",
        "frame 31", "wait 200", "frame 31", "wait 1", "unbreakable end", "wait 1", },
    })

DefineNewAnimations("animations-elitebuild7", {
    Death = {"unbreakable begin", "wait 1", "frame 14", "wait 2000", 
        "frame 14", "wait 200", "frame 14", "wait 200", "frame 15", "wait 200",
        "frame 15", "wait 200", "frame 15", "wait 1", "unbreakable end", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field1", {
    Still = {"frame 6", "wait 4", "frame 6", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field2", {
    Still = {"frame 10", "wait 4", "frame 10", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field3", {
    Still = {"frame 12", "wait 4", "frame 12", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field4", {
    Still = {"frame 4", "wait 4", "frame 4", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field5", {
    Still = {"frame 0", "wait 4", "frame 0", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field6", {
    Still = {"frame 1", "wait 4", "frame 1", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field7", {
    Still = {"frame 5", "wait 4", "frame 5", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field8", {
    Still = {"frame 7", "wait 4", "frame 7", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field9", {
    Still = {"frame 2", "wait 4", "frame 2", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field10", {
    Still = {"frame 3", "wait 4", "frame 3", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field11", {
    Still = {"frame 11", "wait 4", "frame 11", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field12", {
    Still = {"frame 8", "wait 4", "frame 8", "wait 1", },
    })

DefineNewAnimations("animations-crystal-field13", {
    Still = {"frame 9", "wait 4", "frame 9", "wait 1", },
    })

----------------------------
-------- Terras Units ------
----------------------------

DefineNewAnimations("animations-dorcoz", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 4", "wait 2", 
        "frame 10", "move 4", "wait 2", "frame 15", "move 4", "wait 2", 
        "frame 20", "move 4", "wait 2", "frame 5", "move 4", "wait 2", 
        "frame 10", "move 4", "wait 2", "frame 15", "move 4", "wait 2", 
        "frame 20", "move 4", "unbreakable end", "wait 2", },
    Attack = {"unbreakable begin", 
        "frame 25", "sound dorcoz-attack", "attack", "wait 1", 
        "frame 0", "unbreakable end", "wait 49", },
    Death = {"unbreakable begin", "frame 30", "wait 5", "frame 35", "wait 5", 
        "frame 40", "wait 5", "frame 45", "unbreakable end", "wait 5", },
    })

---------------------------------------
-- Misc.

DefineNewAnimations("animations-building", {
    Still = {"frame 0", "wait 4", "frame 0", "wait 1"},
    })

---------------------------------------
--        Dead Body

DefineNewAnimations("animations-dead-body", {
    Death = {"unbreakable begin", "frame 5", "wait 200", "frame 10", "wait 200",
        "frame 15", "wait 200", "frame 20", "wait 200", "frame 25", "wait 200",
        "frame 25", "unbreakable end", "wait 1", },
    })

---------------------------------------
--        Destroyed *x* Place

DefineNewAnimations("animations-destroyed-place", {
    Death = {"unbreakable begin", "frame 0", "wait 200", "frame 1", "wait 200",
        "frame 1", "unbreakable end", "wait 1", },
    })


