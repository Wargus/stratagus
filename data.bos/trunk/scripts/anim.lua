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

DefineAnimations("animations-apcs", {
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

DefineAnimations("animations-bazoo", {
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

DefineAnimations("animations-assault", {
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

DefineAnimations("animations-grenadier", {
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

DefineAnimations("animations-engineer", {
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

DefineAnimations("animations-harvester", {
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

DefineAnimations("animations-rfac", {
    Still = {"frame 0", "wait 3", },
    })

DefineAnimations("animations-vfac", {
    Still = {"frame 0", "wait 3", "frame 1", "wait 3", "frame 2", "wait 3", 
        "frame 3", "wait 3", "frame 4", "wait 3", "frame 5", "wait 3", 
        "frame 6", "wait 3", "frame 7", "wait 3", "frame 8", "wait 3", },
    })

----------------------
-------- Nature ------
----------------------

DefineAnimations("animations-elitecorpse1", {
    Death = {"unbreakable begin", "wait 1", "frame 20", "wait 2000", 
        "frame 0", "wait 200", "frame 5", "wait 200", "frame 10", "wait 200", 
        "frame 15", "wait 200", "frame 15", "wait 1", "unbreakable end", "wait 1", },
    }) 

DefineAnimations("animations-elitebuild1", {
    Death = {"unbreakable begin", "wait 1", "frame 16", "wait 2000", 
        "frame 16", "wait 200", "frame 16", "wait 200", "frame 17", "wait 200",
        "frame 17", "wait 200", "frame 17", "wait 1", "unbreakable end", "wait 1", },
    }) 

DefineAnimations("animations-elitebuild2", {
    Death = {"unbreakable begin", "wait 1", "frame 19", "wait 2000", 
        "frame 19", "wait 200", "frame 19", "wait 200", "frame 20", "wait 200",
        "frame 20", "wait 200", "frame 20", "wait 1", "unbreakable end", "wait 1", },
    })

DefineAnimations("animations-elitebuild4", {
    Death = {"unbreakable begin", "wait 1", "frame 12", "wait 2000", 
        "frame 12", "wait 200", "frame 12", "wait 200", "frame 13", "wait 200",
        "frame 13", "wait 200", "frame 13", "wait 1", "unbreakable end", "wait 1", },
    })

DefineAnimations("animations-elitebuild5", {
    Death = {"unbreakable begin", "wait 1", "frame 15", "wait 2000", 
        "frame 15", "wait 200", "frame 15", "wait 200", "frame 16", "wait 200",
        "frame 16", "wait 200", "frame 16", "wait 1", "unbreakable end", "wait 1", },
    })

DefineAnimations("animations-elitebuild6", {
    Death = {"unbreakable begin", "wait 1", "frame 30", "wait 2000", 
        "frame 30", "wait 200", "frame 30", "wait 200", "frame 31", "wait 200",
        "frame 31", "wait 200", "frame 31", "wait 1", "unbreakable end", "wait 1", },
    })

DefineAnimations("animations-elitebuild7", {
    Death = {"unbreakable begin", "wait 1", "frame 14", "wait 2000", 
        "frame 14", "wait 200", "frame 14", "wait 200", "frame 15", "wait 200",
        "frame 15", "wait 200", "frame 15", "wait 1", "unbreakable end", "wait 1", },
    })

---------------------------------------
-- Misc.

DefineAnimations("animations-building", {
    Still = {"frame 0", "wait 4", "frame 0", "wait 1"},
    })

---------------------------------------
--        Dead Body

DefineAnimations("animations-dead-body", {
    Death = {"unbreakable begin", "frame 5", "wait 200", "frame 10", "wait 200",
        "frame 15", "wait 200", "frame 20", "wait 200", "frame 25", "wait 200",
        "frame 25", "unbreakable end", "wait 1", },
    })

---------------------------------------
--        Destroyed *x* Place

DefineAnimations("animations-destroyed-place", {
    Death = {"unbreakable begin", "frame 0", "wait 200", "frame 1", "wait 200",
        "frame 1", "unbreakable end", "wait 1", },
    })


