--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--      dependency.lua     -       Define the dependencies.
--
--      (c) Copyright 2001 - 2006 by Lutz Sammer, Crestez Leonard, 
--      Francois Beerten and Lois Taulelle
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
--      $Id: upgrade.lua 198 2005-08-19 16:44:05Z gruiick $


DefineDependency("unit-bazoo", {"upgrade-expl2"})
DefineDependency("unit-grenadier", {"upgrade-expl"})
DefineDependency("unit-hosp", {"unit-vault", "unit-camp"})
DefineDependency("unit-vfac", {"unit-vault", "unit-rfac"})
DefineDependency("unit-msilo", {"unit-vault", "unit-rfac", "unit-dev-yard"})

DefineDependency("upgrade-expl2", {"upgrade-expl"})
DefineDependency("upgrade-ddril", {"upgrade-tdril"})
DefineDependency("upgrade-pdril", {"upgrade-ddril"})

