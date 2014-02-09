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


function loadgraphic(filename, width, height)
   local g = CGraphic:New(filename, width, height)
   g:Load()
   return g
end

local flash_graphic = loadgraphic("graphics/explosions/flash.png", 240, 194)
local smoke_graphic = loadgraphic("graphics/explosions/smokelight12.png", 12, 12)
local largeflames = {}
local mediumflames = {}
local smallflames = {}
for i = 1, 9 do
  largeflames[i] = loadgraphic("graphics/explosions/large0"..i..".png", 128, 96)
  mediumflames[i] = loadgraphic("graphics/explosions/medium0"..i..".png", 54, 73)
  smallflames[i] = loadgraphic("graphics/explosions/small0"..i..".png", 13, 18)
end

function pickRandom(list) 
    return list[math.random(table.getn(list))] 
end

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

function largeExplosion(x, y)
   addStaticParticle(flash_graphic, 22, x, y)
   addStaticParticle(pickRandom(largeflames), 33, x, y)
   addChunkParticles(16, smoke_graphic, 60, x, y)
end

function bazooExplosion(x, y)
   addStaticParticle(pickRandom(mediumflames), 33, x, y)
end

function nuclearExplosion(x, y)
   addStaticParticle(flash_graphic, 22, x, y)
   addStaticParticle(pickRandom(largeflames), 33, x+50, y+50)
   addStaticParticle(pickRandom(largeflames), 33, x-50, y-47)
   addStaticParticle(pickRandom(largeflames), 33, x-40, y+45)
   addStaticParticle(pickRandom(largeflames), 33, x+47, y-53)
   addChunkParticles(32, smoke_graphic, 60, x, y)
end
