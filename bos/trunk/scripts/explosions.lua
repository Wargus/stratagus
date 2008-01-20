--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	explosions.lua	-	Helpers for particle explosions.
--
--	(c) Copyright 2008 by Francois Beerten
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
--	$Id: missiles.lua 9396 2008-01-20 18:15:30Z feb $

local flame_graphic = CGraphic:New("graphics/particle/large01.png", 128, 96)
flame_graphic:Load()
local flash_graphic = CGraphic:New("graphics/particle/flash.png", 240, 194)
flash_graphic:Load()
local smoke_graphic = CGraphic:New("graphics/particle/smokelight12.png", 12, 12)
smoke_graphic:Load()

function addStaticParticle(graphic, ticksperframe, x, y)
   local a = GraphicAnimation(graphic, ticksperframe)
   local e = StaticParticle(CPosition(x,y), a)
   ParticleManager:add(e:clone())
end

function addChunkParticles(amount, smokegraphic, ticksperframe, x, y)
   local smokeanimation = GraphicAnimation(smoke_graphic, ticksperframe)
   for i = 1, amount do
      local chunk = CChunkParticle(CPosition(x, y), smokeanimation)
      ParticleManager:add(chunk:clone())
   end
end

function bazooExplosion(x, y)
   addStaticParticle(flash_graphic, 22, x, y)
   addStaticParticle(flame_graphic, 33, x, y)
   addChunkParticles(8, smoke_graphic, 60, x, y)
end

